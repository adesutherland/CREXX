#!/usr/bin/env python3
"""Focused assignment source and supplied-context integration; no product rebuild."""
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
    parser.add_argument('--vm', action='append', choices=('rxbvm', 'rxtvm'))
    parser.add_argument('--phase', choices=('all', 'context', 'source', 'failures', 'fatal'), default='all',
                        help='allow remaining components without repeating qualified ones')
    parser.add_argument('--inspect', type=Path, help='temporary RXAS for independent review')
    args = parser.parse_args()
    build = args.build.resolve(); tools = build / 'bin'
    suffix = '.exe' if os.name == 'nt' else ''
    vms = args.vm or ('rxbvm', 'rxtvm')
    fixtures = Path(__file__).resolve().parent / 'assignment'
    expected = (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n')
    args.work.mkdir(parents=True, exist_ok=True)
    commands = 0; started = time.monotonic()
    env = dict(os.environ); env.pop('CREXX_PROVIDER_PATH', None)
    with tempfile.TemporaryDirectory(prefix='assignment-', dir=args.work) as temporary:
        work = Path(temporary); runtime = work / 'runtime'; runtime.mkdir()
        libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
        for name in libraries[1:]:
            shutil.copyfile(tools / (name + '.rxbin'), runtime / (name + '.rxbin'))

        def tool(name):
            return tools / (name + suffix)

        def run(label, command, *, failure=False):
            nonlocal commands
            result = subprocess.run([str(part) for part in command], cwd=runtime,
                                    env=env, capture_output=True, timeout=300)
            commands += 1; diagnostic = result.stdout + result.stderr
            assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|'
                                 rb'SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-3000:])
            assert (result.returncode != 0 if failure else result.returncode == 0), (label, result.returncode, diagnostic[-5000:])
            return result

        def inspect(path):
            if args.inspect:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(path, args.inspect / path.name)

        def compile_c(mode, source, output, ir='on', failure=False):
            host = build / 'research/level-l/compilers/backend' / mode / 'llrxc-c-linked.rxbin'
            return run(output.stem + '-compile', [tool('rxbvm'), '--no-autoload', host,
                '-a', *(['-n'] if ir == 'off' else []), '-i', runtime, '-o', output, source], failure=failure)

        def link_c(stem, image):
            output = work / (stem + '-linked')
            run(stem + '-link', [tool('rxlink'), '-o', output, image,
                *[runtime / (name + '.rxbin') for name in libraries]])
            return output.with_suffix('.rxbin')

        def execute(stem, image, output, error=None):
            for vm in vms:
                result = run(stem + '-' + vm, [tool(vm), '--no-autoload', image], failure=error is not None)
                assert result.stdout.replace(b'\r\n', b'\n') == output, (stem, vm, result.stdout, output)
                if error:
                    assert error in result.stderr and b'unreachable' not in result.stdout, (stem, result.stderr)
                else:
                    assert not result.stderr, (stem, result.stderr)

        for mode in ('opt', 'noopt'):
            backend = build / 'research/level-l/compilers/backend' / mode
            shutil.copyfile(backend / 'llctarget.rxbin', runtime / 'llctarget.rxbin')
            if args.phase in ('all', 'context'):
                provider = work / (mode + '-provider.rxas')
                for vm in vms:
                    result = run(mode + '-contracts-' + vm, [tool(vm), '--no-autoload',
                        backend / 'assignment-contracts-linked.rxbin', '-a', runtime, provider])
                    assert re.fullmatch(rb'ASSIGN_CONTRACTS\|checks=\d+\|failures=0\r?\n', result.stdout), result.stdout
                    assert not result.stderr, result.stderr
                check_provider_shape(provider.read_text(encoding='utf-8'))
                inspect(provider)
                run(mode + '-provider-assemble', [tool('rxas'), '-n', provider])
                imports = work / (mode + '-imports'); imports.mkdir()
                for file in runtime.glob('*.rxbin'): shutil.copyfile(file, imports / file.name)
                shutil.copyfile(provider.with_suffix('.rxbin'), imports / 'assignfixture.rxbin')
                host = work / (mode + '-host')
                run(mode + '-host-compile', [tool('rxc'), *(['-n'] if mode == 'noopt' else []),
                    '-x', '--no-exe-import', '-i', imports, '-o', host, fixtures / 'host.crexx'])
                run(mode + '-host-assemble', [tool('rxas'), '-n', host])
                linked = work / (mode + '-host-linked'); control = work / (mode + '-host.ctl')
                files = [host.with_suffix('.rxbin'), provider.with_suffix('.rxbin'),
                         *[runtime / (name + '.rxbin') for name in libraries]]
                control.write_text(''.join('INPUT ' + str(path) + '\n' for path in files)
                    + 'ROOT ' + host.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
                run(mode + '-host-link', [tool('rxlink'), '-c', control])
                execute(mode + '-host', linked.with_suffix('.rxbin'), b'ASSIGN_HOST|checks=20|failures=0\n')
            if args.phase in ('all', 'source'):
                for ir in ('on', 'off'):
                    stem = mode + '-ir-' + ir; output = work / (stem + '.rxas')
                    compile_c(mode, fixtures / 'source-c.crexx', output, ir)
                    text = output.read_text(encoding='utf-8')
                    assert str(build) not in text and str(work) not in text
                    check_source_shape(text)
                    inspect(output)
                    run(stem + '-assemble', [tool('rxas'), '-n', output])
                    execute(stem, link_c(stem, output.with_suffix('.rxbin')), expected)
        if args.phase in ('all', 'failures', 'fatal'):
            # Distinct syntax obligations run once; dead malformed RHS runs in
            # both IR modes. Unchanged DROP name/schema/ref controls are reused.
            negatives = [('invalid-explicit', '1bad = 1', ('on',)),
                         ('invalid-empty', '1bad =', ('on',)),
                         ('malformed-rhs', 'value = (1 + 2', ('on',)),
                         ('dead-malformed', 'if 0 then value = (1 + 2', ('on', 'off'))]
            if args.phase == 'fatal': negatives = []
            for name, source, modes in negatives:
                path = work / (name + '.crexx'); path.write_text('options levelc\n' + source + '\n', encoding='utf-8')
                for ir in modes:
                    output = work / (name + '-' + ir + '.rxas'); output.write_bytes(b'previous output\n')
                    result = compile_c('opt', path, output, ir, failure=True)
                    diagnostic = result.stdout + result.stderr
                    assert b'LL_COMPILER_SYNTAX at' in diagnostic and path.name.encode() + b':' in diagnostic, diagnostic
                    assert all(word in diagnostic for word in (b'line ', b'column ', b'^')), diagnostic
                    assert output.read_bytes() == b'previous output\n', name
            for ir in ('on', 'off'):
                stem = 'fatal-' + ir; output = work / (stem + '.rxas')
                compile_c('opt', fixtures / 'fatal-c.crexx', output, ir)
                text = output.read_text(encoding='utf-8'); check_fatal_shape(text); inspect(output)
                run(stem + '-assemble', [tool('rxas'), '-n', output])
                execute(stem, link_c(stem, output.with_suffix('.rxbin')), b'before\n', b'RXC-LC-42.3')
    print(f'ASSIGN|phase={args.phase}|commands={commands}|failures=0|seconds={time.monotonic() - started:.4f}')


def normal_body(text):
    found = re.search(r'^llf1\(\) \.locals=[\s\S]*?(?=^llerror|\Z)', text, re.M)
    assert found is not None
    return found.group()


def check_provider_shape(text):
    body = normal_body(text)
    assert '.expose=assignfixture.run' in body
    assert 'context=reference .rexxclassiccontext..rexxclassiccontext,entry=.int' in body
    assert re.findall(r'^llf\d+\(\)', text, re.M) == ['llf1()'], 'one supplied-context unit'
    read = 'rexxclassiccontext.rexxclassiccontext.readsymbol'
    assert len(re.findall(r'^call [^\n]*,' + re.escape(read) + r'\(\),', body, re.M)) == 3
    assert ',llctarget.poolread(),' not in body, 'final reads retain live NOVALUE policy'
    assert len(re.findall(r'^call [^\n]*,llctarget.poolwrite\(\),', body, re.M)) == 8
    assert not re.search(r'^deref |asstring|resolve|split', body, re.M | re.I)
    assert re.findall(r'^beq llprocedure1_\d+_match,[^,\n]+,(\d+)$', body, re.M) == ['0'], 'checked exported entry zero'
    blocks = re.findall(r'^llb\d+:\n([\s\S]*?)(?=^llb\d+:|\Z)', body, re.M)
    semantic = [block for block in blocks if ',' + read + '(),' in block]
    assert len(semantic) == 1, 'one straight-line assignment semantic block'
    # Exclude only the terminal forward guard that restores condition policy;
    # reads and writes themselves must still introduce no branch.
    effects = re.sub(r'^brf (llpolicies\d+_\d+_done),(r\d+)\n'
                     r'(?:sigpop "[A-Z_]+"\n)+load \2,0\n\1:\n(?=ret r\d+\n?$)',
                     '', semantic[0], flags=re.M)
    assert not re.search(r'^br(?:t|f)? |^b(?:eq|ne|lt|le|gt|ge) ', effects, re.M), 'assignment effects introduce no branch'
    assert ',llctarget.poolnew(),' not in body and '.bindpool(),' not in body, 'supplied shared entry adds no private pool'
    assert body.index('"key"') < body.index('"items.key"'), 'tail mutation precedes raw compound store'


def check_source_shape(text):
    # Private marker is the only source call. Argument/result ownership may
    # surround it; substitution and final write follow it, never precede it.
    body = normal_body(text)
    assert re.findall(r'^llf\d+\(\)', text, re.M) == ['llf1()'], 'one source-unit body'
    calls = list(re.finditer(r'^call [^,\n]+,llf1\(\),(r\d+)$', body, re.M))
    assert len(calls) == 1, 'one marker invocation'
    entry_slot = 'r' + str(int(calls[0][1][1:]) + 2)
    entries = re.findall(r'^load '+entry_slot+r',([^\n]+)$', body[:calls[0].start()], re.M)
    assert entries[-1] == '1', 'marker uses the first positive source entry'
    following = body[calls[0].end():]
    first_write = re.search(r'^call [^\n]*,llctarget.poolwrite\(\),', following, re.M)
    assert first_write and '"items.key"' in following[:first_write.end()]
    assert not re.search(r'^call [^\n]*,(?:llctarget\.poolread|rexxclassiccontext\.rexxclassiccontext\.readsymbol)\(\),',
                         following[:first_write.end()], re.M)


def check_fatal_shape(text):
    body = normal_body(text)
    rhs = re.search(r'^call ([^,]+),llctarget.binaryop\(\),', body, re.M)
    write = re.search(r'^call [^\n]*,llctarget.poolwrite\(\),', body, re.M)
    assert rhs and write and rhs.end() < write.start()
    assert re.search(r'^sigbrv llerror1_2,[^\n]*"DIVISION_BY_ZERO"$', body[:rhs.start()], re.M), 'own-frame handler is installed before RHS'
    handlers = text[text.index('llerror1_1:'):]
    assert 'sighalt "DIVISION_BY_ZERO"' in handlers and 'signalorigin "DIVISION_BY_ZERO"' in handlers
    assert not re.search(r'^call [^\n]*,llctarget.poolwrite\(\),', handlers, re.M), 'fatal handler cannot enter the store'


if __name__ == '__main__':
    main()
