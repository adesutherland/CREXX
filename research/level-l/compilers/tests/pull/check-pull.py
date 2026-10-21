#!/usr/bin/env python3
"""Standalone PULL checks over prepared products; this runner never builds.

One C compilation serves stdin/EOF, queue/borrow controls and readable RXAS.
Shared PARSE admission/engine tests remain their owners. Configured NOTREADY,
host callbacks, TRACE and declared platforms remain STEP07/08 obligations.
"""
import argparse
from collections import Counter
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import tempfile


def called(row, symbol):
    return re.search(r'^\s*(?:settp)?(?:swap)?call[1-4]?\s+[^,\n]+,§?' +
                     re.escape(symbol) + r'\(\)(?:,|\s*$)', row)


def unit(text):
    assert re.findall(r'^llf\d+\(\)', text, re.M) == ['llf1()'], 'one shared C source body'
    start = re.search(r'^llf1\(\)\s+\.locals=', text, re.M)
    assert start, 'generated C function header'
    body = text[start.start():text.index('llerror1_1:')]
    assert 'context=reference .rexxclassiccontext..rexxclassiccontext,entry=.int' in body
    return body


def source_position(data, needle):
    offset = data.index(needle)
    line = data[:offset].count(b'\n') + 1
    start = data.rfind(b'\n', 0, offset) + 1
    column = len(data[start:offset].decode('utf-8')) + 1
    return offset, line, column


def shapes(text, source, helper_text):
    body = unit(text)
    critical = {i for i, row in enumerate(source.read_text().splitlines(), 1)
                if row.strip().rstrip(';').rstrip() in ('pull', 'pull , LATER', 'pull ,,,', 'pull FIRST REST,, LATER')}
    reads = Counter(); executes = Counter(); current = None; packet = None
    for row in body.splitlines():
        if row.lstrip().startswith('.srcstep '):
            fields = shlex.split(row)
            current = int(fields[5]) if fields[4] == 'source-c.crexx' else None
        literal = re.match(r'^\s*load\s+r\d+,0x([0-9a-fA-F]+)\s*$', row)
        if literal:
            value = bytes.fromhex(literal.group(1))
            if value.startswith(b'LCP1'):
                packet = value
        if called(row, 'llctarget.parsesource') and current in critical:
            reads[current] += 1
        if called(row, 'rexxclassicparse.execute') and current in critical:
            executes[current] += 1
            assert packet and int.from_bytes(packet[6:8], 'little') & 1, ('PULL packet retains UPPER', current)
    assert critical and all(reads[line] == 1 for line in critical), ('one acquisition for bare/empty/comma PULL', critical, reads)
    assert all(executes[line] >= 1 for line in critical), ('empty PULL still uses the shared conversion/UPPER executor', executes)
    assert 'rxparse.cursor' not in body and not any(called(row, 'rxfnsb.pull') for row in body.splitlines()), 'caller does not duplicate queue/cursor algorithms'
    helper = re.search(r'^parsesource\(\).*\.expose=llctarget\.parsesource\s*$([\s\S]*?)(?=^\S+\(.*\.expose=|\Z)', helper_text, re.M)
    assert helper and any(called(row, 'rexxclassicconfig.rexxclassicconfig.pulltext') for row in helper.group().splitlines()), 'existing config producer owns selected queue/input acquisition'
    data = source.read_bytes()
    _, line, column = source_position(data, b"pull HEAD '00'x TAIL")
    ending = len(data.splitlines()[line - 1].decode('utf-8')) + 1
    assert re.search(r'^\s*\.srcstep\s+\d+\s+\d+\s+(?:17|65)\s+"source-c\.crexx"\s+' +
                     rf'{line}\s+{column}\s+{ending}\s+"', body, re.M), 'PULL after Unicode retains original scalar columns'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', required=True, type=Path)
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--mode', choices=('opt', 'noopt'), default='opt')
    parser.add_argument('--vm', choices=('rxbvm', 'rxtvm'), default='rxbvm')
    parser.add_argument('--ir', choices=('on', 'off'), default='on')
    parser.add_argument('--phase', choices=('all', 'source', 'host', 'fatal', 'refusals', 'shapes'), default='all')
    parser.add_argument('--inspect', type=Path)
    args = parser.parse_args()
    build = args.build.resolve(); fixtures = Path(__file__).resolve().parent
    backend = build / 'research/level-l/compilers/backend' / args.mode
    suffix = '.exe' if os.name == 'nt' else ''
    libraries = ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core')
    args.work.mkdir(parents=True, exist_ok=True)
    commands = 0
    with tempfile.TemporaryDirectory(prefix='pull-', dir=args.work) as temporary:
        work = Path(temporary); runtime = work / 'runtime'; runtime.mkdir()
        for name in libraries:
            root = backend if name == 'llctarget' else build / 'bin'
            shutil.copy2(root / (name + '.rxbin'), runtime / (name + '.rxbin'))
        dlls = tuple((build / 'bin').glob('*.dll')) if os.name == 'nt' else ()

        def tool(name):
            return build / 'bin' / (name + suffix)

        def stage(name, plugins):
            directory = work / name; directory.mkdir()
            vm = directory / (args.vm + suffix); shutil.copy2(tool(args.vm), vm)
            for dll in dlls:
                shutil.copy2(dll, directory / dll.name)
            providers = directory / 'providers'; providers.mkdir()
            for plugin in plugins:
                shutil.copy2(build / 'bin' / (plugin + '.rxplugin'), providers / (plugin + '.rxplugin'))
            return directory, vm

        host, host_vm = stage('C compiler host', ('rxfs',))
        application, target_vm = stage('static application', ('rxfs', 'rxcstream'))
        compiler = host / 'llrxc-c-linked.rxbin'; shutil.copy2(backend / compiler.name, compiler)
        empty_path = work / 'empty PATH'; empty_path.mkdir()
        env = dict(os.environ, PATH=str(empty_path)); env.pop('CREXX_PROVIDER_PATH', None)
        assert not any(shutil.which(name, path=env['PATH']) for name in ('rxc', 'rxc-c'))

        def run(label, command, *, failure=False, silent=False, data=b'', cwd=work):
            nonlocal commands
            completed = subprocess.run([str(item) for item in command], cwd=cwd, env=env,
                                       input=data, capture_output=True, timeout=900)
            commands += 1
            diagnostic = completed.stdout + completed.stderr
            assert not re.search(rb'ERROR: (?:AddressSanitizer|LeakSanitizer)|SUMMARY: .*Sanitizer|runtime error:|Warning in |warning:|#WARNING', diagnostic), (label, diagnostic[-4000:])
            assert (completed.returncode != 0 if failure else completed.returncode == 0), (label, completed.returncode, diagnostic[-6000:])
            if silent:
                assert not diagnostic, (label, diagnostic[-4000:])
            return completed

        def compile_c(source, output, *, failure=False):
            return run('compile ' + source.name,
                       [host_vm, '--no-autoload', compiler, '-a', *(['-n'] if args.ir == 'off' else []),
                        '-i', runtime, '-o', output, source], failure=failure, silent=not failure, cwd=host)

        def inspect(path, name):
            if args.inspect:
                args.inspect.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, args.inspect / f'{name}-{args.mode}-{args.ir}-{args.vm}.rxas')

        if args.phase in ('all', 'source', 'host', 'fatal', 'shapes'):
            source = work / 'source-c.crexx'
            data = (fixtures / source.name).read_bytes()
            source.write_bytes(data.replace(b'\n', b'\r\n') if args.mode == 'noopt' else data)
            assembly = application / 'source.rxas'; compile_c(source, assembly)
            inspect(assembly, 'pull-source'); text = assembly.read_text()
            assert str(work) not in text and str(build) not in text, 'stable logical source identity'
            shapes(text, source, (backend / 'llctarget.rxas').read_text())
            if args.phase in ('all', 'source'):
                run('source assemble', [tool('rxas'), *(['-n'] if args.mode == 'noopt' else []), assembly], silent=True)
                linked = application / 'source-linked'
                run('source link', [tool('rxlink'), '-o', linked, assembly.with_suffix('.rxbin'),
                                    *(runtime / (name + '.rxbin') for name in libraries)], silent=True)
                incoming = 'aéµμ🙂'.encode('utf-8') + b'\x00b rest\n'
                completed = run('default input then EOF', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], data=incoming, cwd=application)
                expected = (fixtures / 'source-c.expected').read_bytes().replace(b'\r\n', b'\n')
                assert completed.stdout.replace(b'\r\n', b'\n') == expected and not completed.stderr, (completed.stdout, completed.stderr, expected)
            if args.phase in ('all', 'host', 'fatal'):
                exported = application / 'source-exported.rxas'
                text, headers = re.subn(r'^(llf1\(\) \.locals=\d+)(\s*)$', r'\1 .expose=pullfixture.run\2', text, count=1, flags=re.M)
                text, metadata = re.subn(r'^(\s*\.meta )"llprogram\.\$MAIN"(?==)', r'\1"pullfixture.run"', text, count=1, flags=re.M)
                assert headers == metadata == 1, 'one test-only exported entry0'
                exported.write_text(text, encoding='utf-8'); inspect(exported, 'pull-exported')
                run('exported assemble', [tool('rxas'), '-n', exported], silent=True)
                imports = work / 'B caller imports'; imports.mkdir()
                for provider in runtime.glob('*.rxbin'):
                    shutil.copy2(provider, imports / provider.name)
                shutil.copy2(exported.with_suffix('.rxbin'), imports / 'pullfixture.rxbin')
                caller = application / 'pull-host'
                run('B host compile', [tool('rxc'), *(['-n'] if args.mode == 'noopt' else []), '-x', '--no-exe-import',
                                       '-i', imports, '-o', caller, fixtures / 'host.crexx'], silent=True)
                run('B host assemble', [tool('rxas'), '-n', caller], silent=True)
                linked = application / 'pull-host-linked'; control = work / 'host.ctl'
                inputs = [caller.with_suffix('.rxbin'), exported.with_suffix('.rxbin'), *(runtime / (name + '.rxbin') for name in libraries)]
                control.write_text(''.join('INPUT ' + str(path) + '\n' for path in inputs) + 'ROOT ' + caller.stem + '\nOUTPUT ' + str(linked) + '\n', encoding='utf-8')
                run('B host link', [tool('rxlink'), '-c', control], silent=True)
                if args.phase in ('all', 'host'):
                    completed = run('selected queue/template/borrow controls', [target_vm, '--no-autoload', linked.with_suffix('.rxbin')], data=b'UNEXPECTED FALLBACK\n', cwd=application)
                    assert re.fullmatch(rb'PULL_HOST\|checks=\d+\|failures=0\r?\n', completed.stdout) and not completed.stderr, (completed.stdout, completed.stderr)
                if args.phase in ('all', 'fatal'):
                    rows = source.read_text().splitlines()
                    cases = (('late-bare', b'17.1', rows.index('LATEBARE:') + 2),
                             ('late-empty', b'17.1', rows.index('LATEEMPTY:') + 2),
                             ('bad-position', b'26.4', rows.index('  pull EARLY 2 CURRENT =(POSITION) LATER') + 1))
                    for mode, code, line in cases:
                        completed = run('reached ' + mode, [target_vm, '--no-autoload', linked.with_suffix('.rxbin'), '-a', mode], failure=True, cwd=application)
                        assert completed.stdout.replace(b'\r\n', b'\n') == b'PULL_FATAL|before\n', (mode, completed.stdout)
                        assert code in completed.stderr and b'CLASSIC_SYNTAX' in completed.stderr, (mode, completed.stderr)
                        assert f'source-c.crexx:{line}:'.encode() in completed.stderr, (mode, line, completed.stderr)
                        if mode == 'bad-position':
                            assert b'template=1' in completed.stderr and b'input#1' in completed.stderr, completed.stderr
                        assert b'UNREACHED' not in completed.stdout and b'Assertion' not in completed.stderr

        if args.phase in ('all', 'refusals'):
            cases = (('position-name', 'pull X +NAME\nreturn 0\n', 'NAME', b'38.2'),
                     ('pattern-expression', 'pull X (SEP + 1)\nreturn 0\n', '+', b'38.1'),
                     ('dead-symbol', 'return 0; pull X 1foo\n', '1foo', b'38.1'),
                     ('dead-radix', "if 0 then pull X 'GG'x\nreturn 0\n", "'GG'x", b'invalid digit'))
            output = application / 'prior.rxas'; prior = b'unchanged prior output\n'; output.write_bytes(prior)
            for name, content, token, code in cases:
                source = work / (name + '.crexx'); data = ('options levelc\n' + content).encode('utf-8'); source.write_bytes(data)
                completed = compile_c(source, output, failure=True)
                diagnostic = completed.stdout + completed.stderr
                offset, line, column = source_position(data, token.encode())
                assert code in diagnostic and f'at line {line}, column {column} (byte {offset})'.encode() in diagnostic, (name, diagnostic)
                assert output.read_bytes() == prior and not Path(str(output) + '.llrxc-stage').exists(), (name, 'atomic failed output')
    print(f'PULL_CHECK|mode={args.mode}|vm={args.vm}|ir={args.ir}|phase={args.phase}|commands={commands}|PASS')


if __name__ == '__main__':
    main()
