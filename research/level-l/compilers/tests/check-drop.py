#!/usr/bin/env python3
"""Focused DROP orchestration; source admission and semantics stay in B/L."""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--vm', action='append', choices=('rxbvm', 'rxtvm'),
                        help='available target engines; default: both')
    parser.add_argument('--refusals-only', action='store_true',
                        help='check source refusals without repeating the positive matrix')
    parser.add_argument('--inspect', type=Path,
                        help='temporary readable RXAS for independent review')
    args = parser.parse_args()
    build = args.build.resolve()
    args.work.mkdir(parents=True, exist_ok=True)
    suffix = '.exe' if os.name == 'nt' else ''
    tools = build / 'bin'
    vms = args.vm or ('rxbvm', 'rxtvm')
    fixtures = Path(__file__).resolve().parent / 'drop'
    expected = (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n')
    commands = 0
    started = time.monotonic()
    with tempfile.TemporaryDirectory(prefix='drop-', dir=args.work) as temporary:
        work = Path(temporary)
        runtime = work / 'runtime'
        runtime.mkdir()
        for name in ('library', 'rxfnsc', 'classlib', 'rxunicode-core'):
            shutil.copyfile(tools / (name + '.rxbin'), runtime / (name + '.rxbin'))
        shutil.copyfile(build / 'research/level-l/compilers/backend/opt/llctarget.rxbin',
                        runtime / 'llctarget.rxbin')
        env = dict(os.environ)
        env.pop('CREXX_PROVIDER_PATH', None)

        def run(label, command, *, failure=False, cwd=runtime):
            nonlocal commands
            result = subprocess.run([str(part) for part in command], cwd=cwd,
                                    env=env, capture_output=True, timeout=300)
            commands += 1
            diagnostic = result.stdout + result.stderr
            assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|'
                                 rb'SUMMARY: .*Sanitizer|runtime error:', diagnostic), (label, diagnostic[-3000:])
            assert not re.search(rb'Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-3000:])
            assert (result.returncode != 0 if failure else result.returncode == 0), (
                label, result.returncode, diagnostic[-5000:])
            return result

        def tool(name):
            return tools / (name + suffix)

        def compile_c(mode, source, output, ir='on', failure=False):
            host = build / 'research/level-l/compilers/backend' / mode / 'llrxc-c-linked.rxbin'
            return run(output.stem + '-compile', [tool('rxbvm'), '--no-autoload', host,
                       '-a', *(['-n'] if ir == 'off' else []), '-i', runtime,
                       '-o', output, source], failure=failure)

        def link_c(stem, image):
            output = work / (stem + '-linked')
            run(stem + '-link', [tool('rxlink'), '-o', output, image,
                *[runtime / (name + '.rxbin') for name in
                  ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')]])
            return output.with_suffix('.rxbin')

        def execute(stem, image, expected_output):
            for vm in vms:
                result = run(stem + '-' + vm, [tool(vm), '--no-autoload', image])
                assert result.stdout.replace(b'\r\n', b'\n') == expected_output, (
                    stem, vm, result.stdout, expected_output)
                assert not result.stderr, (stem, vm, result.stderr)

        # Both native B host compiler modes and C IR modes; each emitted target
        # executes in both engines. One extra assembler-on execution distinguishes
        # assembler optimization from the compiler/IR choices.
        for mode in (() if args.refusals_only else ('opt', 'noopt')):
            backend = build / 'research/level-l/compilers/backend' / mode
            shutil.copyfile(backend / 'llctarget.rxbin', runtime / 'llctarget.rxbin')
            provider = work / (mode + '-provider.rxas')
            for vm in vms:
                result = run(mode + '-contracts-' + vm, [tool(vm), '--no-autoload',
                    backend / 'drop-contracts-linked.rxbin', '-a', runtime, provider])
                assert re.fullmatch(rb'DROP_CONTRACTS\|checks=\d+\|failures=0\r?\n', result.stdout), result.stdout
                assert not result.stderr, result.stderr
            provider_text = provider.read_text(encoding='utf-8')
            check_provider_shape(provider_text)
            if args.inspect:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(provider, args.inspect / provider.name)
            run(mode + '-provider-assemble', [tool('rxas'), '-n', provider])
            imports = work / (mode + '-imports')
            imports.mkdir()
            for file in runtime.glob('*.rxbin'):
                shutil.copyfile(file, imports / file.name)
            shutil.copyfile(provider.with_suffix('.rxbin'), imports / 'dropfixture.rxbin')
            host = work / (mode + '-host')
            run(mode + '-host-compile', [tool('rxc'), *(['-n'] if mode == 'noopt' else []),
                '-x', '--no-exe-import', '-i', imports, '-o', host, fixtures / 'host.crexx'])
            run(mode + '-host-assemble', [tool('rxas'), '-n', host])
            # Both files have roots. Explicit B ROOT proves the generated C root
            # is never entered; the only C entry is the ordinary test export.
            control = work / (mode + '-host.ctl')
            linked = work / (mode + '-host-linked')
            files = [host.with_suffix('.rxbin'), provider.with_suffix('.rxbin'),
                     *[runtime / (name + '.rxbin') for name in
                       ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')]]
            control.write_text(''.join('INPUT ' + str(path) + '\n' for path in files)
                               + 'ROOT ' + host.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
            run(mode + '-host-link', [tool('rxlink'), '-c', control])
            execute(mode + '-host', linked.with_suffix('.rxbin'),
                    b'DROP_PROVIDER\nDROP_HOST|checks=9|failures=0\n')
            for ir in ('on', 'off'):
                stem = mode + '-ir-' + ir
                output = work / (stem + '.rxas')
                compile_c(mode, fixtures / 'source-c.crexx', output, ir)
                text = output.read_text(encoding='utf-8')
                assert 'llctarget2' not in text and 'contextnew' not in text
                assert str(build) not in text and str(work) not in text
                assert re.search(r'context=reference \.rexxclassiccontext\.\.rexxclassiccontext', text)
                run(stem + '-assemble', [tool('rxas'), '-n', output])
                execute(stem, link_c(stem, output.with_suffix('.rxbin')), expected)
                if args.inspect:
                    args.inspect.mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(output, args.inspect / output.name)
                if mode == 'opt' and ir == 'on':
                    run(stem + '-asopt-assemble', [tool('rxas'), output])
                    execute(stem + '-asopt', link_c(stem + '-asopt', output.with_suffix('.rxbin')), expected)
        # Compile refusal is independent of optimizer cleanup. Representative
        # malformed unreachable source runs in both IR modes; other distinct
        # boundaries run once rather than multiplying a syntax-only matrix.
        negatives = [
            ('empty', 'drop', b'LL_COMPILER_SYNTAX'),
            ('empty-reference', 'drop ()', b'LL_COMPILER_SYNTAX'),
            ('expression', 'drop (a+1)', b'LL_COMPILER_SYNTAX'),
            ('two-reference-names', 'drop (a b)', b'LL_COMPILER_SYNTAX'),
            ('unclosed-reference', 'drop (a', b'LL_COMPILER_SYNTAX'),
            ('constant-name', 'drop 1bad', b'LL_COMPILER_SYNTAX'),
            ('dot-name', 'drop .ordinary', b'LL_COMPILER_SYNTAX'),
            ('reserved-prefix', 'drop .RCx', b'LL_COMPILER_SYNTAX'),
            ('emoji-name', 'drop 🙂', b'LL_COMPILER_SYNTAX'),
            ('combining-name', 'drop é', b'LL_COMPILER_SYNTAX'),
            ('dead-malformed', 'if 0 then drop (a+1)', b'LL_COMPILER_SYNTAX'),
            ('unicode-routine', 'exit; µ: procedure; return', b'LL_COMPILER_PROFILE'),
            ('unicode-call', 'say μ()', b'LL_COMPILER_PROFILE'),
            ('dotted-call', 'say p.q()', b'LL_COMPILER_PROFILE'),
        ]
        for name, source, diagnostic in negatives:
            path = work / (name + '.crexx')
            path.write_text('options levelc\n' + source + '\n', encoding='utf-8')
            for ir in (('on', 'off') if name == 'dead-malformed' else ('on',)):
                output = work / (name + '-' + ir + '.rxas')
                output.write_bytes(b'preserve previous output\n')
                result = compile_c('opt', path, output, ir, failure=True)
                message = result.stdout + result.stderr
                assert re.search(re.escape(diagnostic) + rb'(?: at|:)', message), (name, message)
                if diagnostic == b'LL_COMPILER_PROFILE':
                    assert b'routine name is outside the ASCII-undotted profile' in message, (name, message)
                assert path.name.encode() + b':' in message and b'line ' in message and b'column ' in message and b'^' in message, (name, message)
                assert output.read_bytes() == b'preserve previous output\n', name
    label = 'DROP_REFUSALS' if args.refusals_only else 'DROP'
    print(f'{label}|commands={commands}|failures=0|seconds={time.monotonic() - started:.4f}')


def check_provider_shape(text):
    """Two operations use existing pool methods, with one indirect capture."""
    calls = re.findall(r'^call (r\d+),([^,]+)\(\),(r\d+)$', text, re.M)
    methods = [target for _, target, _ in calls]
    direct = 'rexxpool.rexxvariablepool.dropsymbol'
    indirect = 'rexxpool.rexxvariablepool.dropindirectlist'
    capture = 'rexxpool.rexxvariablepool.symbolvalue'
    string = 'rexxvalue.rexxvalue.asstring'
    for target in (direct, indirect, capture, string):
        assert methods.count(target) == 1, (target, methods)
    assert methods.index(direct) < methods.index(capture) < methods.index(string) < methods.index(indirect)
    assert methods.count('rexxclassicconfig.rexxclassicconfig.§factory') == 1
    assert methods.count('rexxclassiccontext.rexxclassiccontext.config') >= 1
    assert methods.count('rexxclassiccontext.rexxclassiccontext.pool') >= 2
    assert not re.search(r'^deref ', text, re.M), 'copied authoritative owner'
    assert '.expose=dropfixture.run' in text
    assert 'context=reference .rexxclassiccontext..rexxclassiccontext' in text
    # Source records remain authored; no compiler-side word/tail loop is emitted.
    assert 'drop-provider.crexx' in text and 'drop items. (names)' in text
    assert not any('word' in target or 'split' in target or 'resolve' in target for target in methods)
    # Fatal dispatch has its own branch ladder. The normal body containing
    # these two straight-line DROP items has no list-control loop.
    body = re.search(r'^llf1\(\) \.locals=[\s\S]*?(?=^llerror|\Z)', text, re.M)
    assert body is not None
    assert not re.search(r'^br(?:t|f)? ', body.group(), re.M), 'unexpected normal DROP list loop'


if __name__ == '__main__':
    main()
