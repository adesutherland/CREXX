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
                assert 'context=reference .rexxclassiccontext..rexxclassiccontext,entry=.int' in text
                run(stem + '-assemble', [tool('rxas'), '-n', output])
                execute(stem, link_c(stem, output.with_suffix('.rxbin')), expected)
                if args.inspect:
                    args.inspect.mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(output, args.inspect / output.name)
                if mode == 'opt' and ir == 'on':
                    run(stem + '-asopt-assemble', [tool('rxas'), output])
                    execute(stem + '-asopt', link_c(stem + '-asopt', output.with_suffix('.rxbin')), expected)
        if not args.refusals_only:
            # Classic retains micro-sign identity; Greek mu uppercases to Mu.
            # Distinct outputs prove that boundary and dotted local lookup.
            path = work / 'routine-names.crexx'
            path.write_text("options levelc\nsay µ()\nsay μ()\nsay Μ()\nsay p.q()\nexit 0\nµ: procedure\nreturn 'micro'\nμ: procedure\nreturn 'greek'\np.q: procedure\nreturn 'dotted'\n", encoding='utf-8')
            output = work / 'routine-names.rxas'
            compile_c('opt', path, output)
            text = output.read_text(encoding='utf-8')
            assert re.findall(r'^llf\d+\(\)', text, re.M) == ['llf1()'], 'all routine names share one unit'
            assert len(re.findall(r'^call [^,\n]+,llf1\(\),', text, re.M)) == 5, 'root plus four checked source calls'
            run('routine-names-assemble', [tool('rxas'), '-n', output])
            execute('routine-names', link_c('routine-names', output.with_suffix('.rxbin')), b'micro\ngreek\ngreek\ndotted\n')
        # Compile refusal is independent of optimizer cleanup. Representative
        # malformed unreachable source runs in both IR modes; other distinct
        # boundaries run once rather than multiplying a syntax-only matrix.
        # Unresolved Unicode/dotted calls are reached43.1 controls in tests/call;
        # declared-name casing and lookup remain exercised above.
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
    assert 'context=reference .rexxclassiccontext..rexxclassiccontext,entry=.int' in text
    # Source records remain authored; no compiler-side word/tail loop is emitted.
    assert 'drop-provider.crexx' in text and 'drop items. (names)' in text
    assert not any('word' in target or 'split' in target or 'resolve' in target for target in methods)
    # Checked entry dispatch is separate from the two DROP items. Their
    # semantic block retains the original no-list-control-loop property.
    body = re.search(r'^llf1\(\) \.locals=[\s\S]*?(?=^llerror|\Z)', text, re.M)
    assert body is not None
    assert re.findall(r'^beq llprocedure1_\d+_match,[^,\n]+,(\d+)$', body.group(), re.M) == ['0'], 'checked exported entry zero'
    blocks = re.findall(r'^llb\d+:\n([\s\S]*?)(?=^llb\d+:|\Z)', body.group(), re.M)
    semantic = [block for block in blocks if ','+direct+'(),' in block]
    assert len(semantic) == 1, 'one straight-line DROP semantic block'
    assert not re.search(r'^br(?:t|f)? |^b(?:eq|ne|lt|le|gt|ge) ', semantic[0], re.M), 'unexpected normal DROP list loop'
    assert ',llctarget.poolnew(),' not in body.group() and '.bindpool(),' not in body.group(), 'supplied shared entry adds no private pool'


if __name__ == '__main__':
    main()
