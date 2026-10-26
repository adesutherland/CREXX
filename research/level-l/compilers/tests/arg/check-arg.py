#!/usr/bin/env python3
"""Focused independent standalone ARG source, refusal and RXAS checks.

Use already prepared compiler/runtime images. This runner performs no builds
and retains no command logs. --inspect is only for disposable readable RXAS.
"""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time


def executable_shape(text, logical):
    """Keep executable loads, labels and frame sizes; abstract source payloads."""
    result = []
    for raw in text.splitlines():
        line = raw.strip()
        if not line or line.startswith('.'):
            continue
        if logical + ':' in line and '"' in line:
            line = line[:line.index('"')] + '"<diagnostic location>"'
        result.append(line)
    return '\n'.join(result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--mode', choices=('opt', 'noopt'), default='opt')
    parser.add_argument('--vm', choices=('rxbvm', 'rxtvm'), default='rxbvm')
    parser.add_argument('--ir', choices=('on', 'off'), default='on')
    parser.add_argument('--phase', choices=('all', 'source', 'shapes', 'refusals', 'fatal'),
                        default='all', help='reuse already qualified unchanged components')
    parser.add_argument('--inspect', type=Path, help='disposable RXAS review destination')
    args = parser.parse_args()
    build = args.build.resolve()
    backend = build / 'research/level-l/compilers/backend' / args.mode
    fixtures = Path(__file__).resolve().parent
    expected = (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n')
    args.work.mkdir(parents=True, exist_ok=True)
    suffix = '.exe' if os.name == 'nt' else ''
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    commands = 0
    started = time.monotonic()

    with tempfile.TemporaryDirectory(prefix='arg-', dir=args.work) as temporary:
        work = Path(temporary)
        runtime = work / 'runtime'
        runtime.mkdir()
        for name in libraries[1:]:
            shutil.copy2(build / 'bin' / (name + '.rxbin'), runtime / (name + '.rxbin'))
        shutil.copy2(backend / 'llctarget.rxbin', runtime / 'llctarget.rxbin')
        dlls = tuple((build / 'bin').glob('*.dll')) if os.name == 'nt' else ()

        def tool(name):
            return build / 'bin' / (name + suffix)

        def stage(directory, providers):
            directory.mkdir()
            vm = directory / (args.vm + suffix)
            shutil.copy2(tool(args.vm), vm)
            for dll in dlls:
                shutil.copy2(dll, directory / dll.name)
            provider_dir = directory / 'providers'
            provider_dir.mkdir()
            for provider in providers:
                name = provider + '.rxplugin'
                source = build / 'bin' / name
                if not source.exists():
                    source = build / 'bin' / 'providers' / name
                shutil.copy2(source, provider_dir / name)
            assert {path.name for path in provider_dir.iterdir()} == {
                provider + '.rxplugin' for provider in providers}
            return vm

        host = work / 'C compiler host'
        host_vm = stage(host, ('rxfs', 'rxcontract'))
        image = host / 'llrxc-c-linked.rxbin'
        shutil.copy2(backend / image.name, image)
        target = work / 'static application'
        target_vm = stage(target, ('rxfs', 'rxcstream'))
        assert not (target / 'providers' / 'rxcontract.rxplugin').exists(), (
            'static application excludes the compiler metadata provider')
        empty_path = work / 'empty PATH'
        empty_path.mkdir()
        env = dict(os.environ, PATH=str(empty_path))
        env.pop('CREXX_PROVIDER_PATH', None)
        assert not any(shutil.which(name, path=env['PATH']) for name in ('rxc', 'rxc-c'))

        def run(label, command, *, success=True, silent=False, cwd=work):
            nonlocal commands
            result = subprocess.run([str(part) for part in command], cwd=cwd, env=env,
                                    capture_output=True, timeout=300)
            commands += 1
            assert (result.returncode == 0) == success, (
                label, result.returncode, result.stdout[-4000:], result.stderr[-4000:])
            if silent:
                assert not result.stdout and not result.stderr, (
                    label, result.stdout[-4000:], result.stderr[-4000:])
            return result

        def compile_source(source, output, *, success=True):
            return run('compile ' + source.name,
                       [host_vm, '--no-autoload', image, '-a',
                        *(['-n'] if args.ir == 'off' else []), '-i', runtime,
                        '-o', output, source], success=success, silent=success, cwd=host)

        def inspect(assembly, stem):
            if args.inspect:
                args.inspect.mkdir(parents=True, exist_ok=True)
                destination = args.inspect / f'{stem}-{args.mode}-{args.ir}-{args.vm}.rxas'
                shutil.copy2(assembly, destination)

        def run_image(assembly, stem, *, assembler='noopt', actuals=(), failure=False):
            run(stem + ' assemble ' + assembler,
                [tool('rxas'), *(['-n'] if assembler == 'noopt' else []), assembly], silent=True)
            linked = target / (stem + '-' + assembler + '-linked')
            run(stem + ' link ' + assembler,
                [tool('rxlink'), '-o', linked, assembly.with_suffix('.rxbin'),
                 *(runtime / (name + '.rxbin') for name in libraries)], silent=True)
            return run(stem + ' execute ' + assembler,
                       [target_vm, '--no-autoload', linked.with_suffix('.rxbin'),
                        *(['-a', *actuals] if actuals else [])], success=not failure, cwd=target)

        if args.phase in ('all', 'source'):
            source = fixtures / 'source-c.crexx'
            assembly = target / 'arg-source.rxas'
            compile_source(source, assembly)
            text = assembly.read_text(encoding='utf-8')
            assert 'rexxclassicparse.execute' in text
            assert not re.search(r'^\s*call\s+[^\n]*llctarget\.readarg\(\)', text, re.M)
            assert not re.search(r'^\s*(?:parseplan|parseword|parsepos|parseeos)\s', text, re.M)
            assert str(build) not in text and str(work) not in text, 'logical source identities'
            anchors = {(int(clause), int(line), int(column)) for clause, line, column in
                       re.findall(r'\.srcstep \d+ (\d+) 17 "source-c\.crexx" (\d+) (\d+) ', text)}
            offset = 0
            for number, line in enumerate(source.read_bytes().decode('utf-8').splitlines(keepends=True), 1):
                if line.lstrip().lower().startswith('arg '):
                    column = len(line) - len(line.lstrip()) + 1
                    # Empty statements produce no executable operation anchor.
                    if line.strip().lower() not in ('arg ,,,',):
                        assert (offset + column, number, column) in anchors, (
                            'authored ARG clause/line/column', number, column)
                offset += len(line.encode('utf-8'))
            packets = [bytes.fromhex(payload) for payload in
                       re.findall(r'^\s*load\s+r\d+,\s*0x(4[cC]435031[0-9a-fA-F]*)', text, re.M)]
            assert any(int.from_bytes(packet[12:16], 'little') == 258 and
                       int.from_bytes(packet[6:8], 'little') == 1 for packet in packets), (
                           'sparse ARG packet retains ordinal 258 and UPPER')
            inspect(assembly, 'source')
            for assembler in ('noopt', 'opt'):
                result = run_image(assembly, 'source', assembler=assembler, actuals=('  main MiXeD  ',))
                assert not result.stderr, (assembler, result.stderr)
                assert result.stdout.replace(b'\r\n', b'\n') == expected, (
                    assembler, result.stdout, expected)

        if args.phase in ('all', 'shapes'):
            pairs = (
                ('main', "options levelc\nsay 'before'; arg; arg ,,,; say 'after'\nexit 0\n"),
                ('if', 'options levelc\nif 1 then do; arg; arg ,,,; end; else do; arg; end\nexit 0\n'),
                ('exit', 'options levelc\nexit 0; arg; arg ,,,\n'),
                ('return', 'options levelc\ncall P\nexit 0\nP: procedure\nreturn 1; arg; arg ,,,\n'),
            )
            source = work / 'arg-shape.crexx'
            for name, content in pairs:
                source.write_text(content, encoding='utf-8')
                assembly = target / ('shape-' + name + '-arg.rxas')
                compile_source(source, assembly)
                text = assembly.read_text(encoding='utf-8')
                # Match complete statement spellings, including comma-only tails.
                control = content.replace('arg ,,,', '       ').replace('arg', '   ')
                source.write_text(control, encoding='utf-8')
                blank = target / ('shape-' + name + '-blank.rxas')
                compile_source(source, blank)
                blank_text = blank.read_text(encoding='utf-8')
                assert executable_shape(text, source.name) == executable_shape(blank_text, source.name), (
                    'empty ARG adds executable/frame/dead-return work', name)
                assert not re.search(r'^\s*call\s+[^\n]*(?:llctarget\.parsesource|rexxclassicparse\.execute)\(\)',
                                     text, re.M), name
                inspect(assembly, 'shape-' + name + '-arg')
                inspect(blank, 'shape-' + name + '-blank')

            simple = work / 'arg-simple.crexx'
            simple.write_text("options levelc\narg , FIRST REST,,, . . , +0\nsay '[' || FIRST || ']|[' || REST || ']'\nexit 0\n",
                              encoding='utf-8')
            assembly = target / 'arg-simple.rxas'
            compile_source(simple, assembly)
            text = assembly.read_text(encoding='utf-8')
            for symbol in ('llctarget.parsesource', 'rexxclassicparse.execute'):
                assert len(re.findall(r'^\s*call\s+[^\n]*' + re.escape(symbol) + r'\(\)', text, re.M)) == 3, symbol
            assert len(re.findall(r'^\s*load\s+r\d+,\s*0x4[cC]435031', text, re.M)) == 3
            assert not re.search(r'^\s*call\s+[^\n]*(?:llctarget\.(?:readarg|poolwrite)|rexxclassicbif_upper)\(\)',
                                 text, re.M), 'no caller scalar writer/UPPER splitter'
            assert not re.search(r'^\s*(?:parseplan|parseword|parsepos|parseeos)\s', text, re.M)
            inspect(assembly, 'simple')
            result = run_image(assembly, 'simple', actuals=('ignored', 'one two', 'unused', 'unused', 'drop me', 'abc'))
            assert not result.stderr and result.stdout.replace(b'\r\n', b'\n') == b'[ONE]|[TWO]\n', result

        if args.phase in ('all', 'refusals'):
            negatives = (
                # The unclosed parenthesis shields its trailing newline: only
                # that case reports the next-line EOF rather than a token.
                ('pattern-close', 'arg X (SEP', b'38.1', 3, 1, 26),
                ('position-name', 'arg X +NAME', b'38.2', 2, 8, 22),
                ('pattern-expression', 'arg X (SEP + 1)', b'38.1', 2, 12, 26),
                ('position-prefix', 'arg X =', b'38.1', 2, 8, 22),
                ('dead-symbol', 'exit 0; arg X 1foo', b'38.1', 2, 15, 29),
                ('dead-radix', "if 0 then arg X 'GG'x", b'invalid digit', 2, 17, 31),
            )
            for name, content, code, line, column, byte in negatives:
                source = work / (name + '.crexx')
                source.write_text('options levelc\n' + content + '\n', encoding='utf-8')
                destination = target / (name + '.rxas')
                destination.write_bytes(b'previous output\n')
                result = compile_source(source, destination, success=False)
                diagnostic = result.stdout + result.stderr
                prefix = (f'{source.name}: LL_COMPILER_SYNTAX at line {line}, '
                          f'column {column} (byte {byte}):').encode('utf-8')
                assert diagnostic.startswith(prefix) and code in diagnostic, (name, diagnostic, prefix)
                assert b'^' in diagnostic, (name, diagnostic)
                assert destination.read_bytes() == b'previous output\n', name
                assert not Path(str(destination) + '.llrxc-stage').exists(), name

        if args.phase in ('all', 'fatal'):
            assembly = target / 'arg-fatal.rxas'
            compile_source(fixtures / 'fatal-c.crexx', assembly)
            inspect(assembly, 'fatal')
            result = run_image(assembly, 'fatal', failure=True)
            assert result.stdout.replace(b'\r\n', b'\n') == b'before\n', result.stdout
            assert b'RXC-LC-26.4' in result.stderr, result.stderr
            assert b'fatal-c.crexx:9:' in result.stderr, result.stderr
            assert b'template=1' in result.stderr and b'input#1' in result.stderr, result.stderr
            assert b'Assertion' not in result.stderr and b'unreachable' not in result.stdout, result.stderr

    print(f'ARG|phase={args.phase}|mode={args.mode}|vm={args.vm}|ir={args.ir}'
          f'|commands={commands}|failures=0|seconds={time.monotonic() - started:.3f}')


if __name__ == '__main__':
    main()
