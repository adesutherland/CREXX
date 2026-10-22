#!/usr/bin/env python3
"""Focused C PARSE integration and caller RXAS checks; orchestration only."""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time

parser = argparse.ArgumentParser()
parser.add_argument('--build', type=Path, required=True)
parser.add_argument('--work', type=Path, required=True)
parser.add_argument('--mode', choices=('opt', 'noopt'), default='opt')
parser.add_argument('--vm', choices=('rxbvm', 'rxtvm'), default='rxbvm')
parser.add_argument('--ir', choices=('on', 'off'), default='on')
parser.add_argument('--inspect', type=Path, help='temporary readable RXAS review destination')
args = parser.parse_args()
build = args.build.resolve()
backend = build / 'research/level-l/compilers/backend' / args.mode
sources = Path(__file__).resolve().parent / 'parse'
args.work.mkdir(parents=True, exist_ok=True)
started = time.monotonic()
ext = '.exe' if os.name == 'nt' else ''
tool = lambda name: str(build / 'bin' / (name + ext))
expected = (
    'words [one] [two ]\n'
    'back aa|:bb cc|a:bb cc\n'
    'live N|left|right\n'
    'upper ÀßA\n'
    'alias [abc] []\n'
    'commas one|two|||\n'
    'empty []\n'
    'pattern abc|\n'
    'scalar α|:|🙂 z\n'
    'effect once\n'
    'input pull line|linein line\n'
    'source COMMAND\n'
    'version 1\n'
    'args MiXeD|third||0\n'
).encode()

with tempfile.TemporaryDirectory(prefix='parse-', dir=args.work) as temporary:
    work = Path(temporary)
    runtime = work / 'contracts'
    runtime.mkdir()
    for name in ('library', 'rxfnsc', 'classlib', 'rxunicode-core'):
        shutil.copy2(build / 'bin' / (name + '.rxbin'), runtime / (name + '.rxbin'))
    shutil.copy2(backend / 'llctarget.rxbin', runtime / 'llctarget.rxbin')
    dlls = tuple((build / 'bin').glob('*.dll')) if os.name == 'nt' else ()

    def stage(directory, providers):
        directory.mkdir()
        vm = directory / (args.vm + ext)
        shutil.copy2(build / 'bin' / vm.name, vm)
        for dll in dlls:
            shutil.copy2(dll, directory / dll.name)
        target = directory / 'providers'
        target.mkdir()
        for provider in providers:
            shutil.copy2(build / 'bin' / (provider + '.rxplugin'), target / (provider + '.rxplugin'))
        return vm

    host = work / 'C compiler host'
    host_vm = stage(host, ('rxfs',))
    image = host / 'llrxc-c-linked.rxbin'
    shutil.copy2(backend / image.name, image)
    target = work / 'static application'
    target_vm = stage(target, ('rxfs', 'rxcstream'))
    empty_path = work / 'empty PATH'
    empty_path.mkdir()
    env = dict(os.environ, PATH=str(empty_path))
    env.pop('CREXX_PROVIDER_PATH', None)
    assert not any(shutil.which(name, path=env['PATH']) for name in ('rxc', 'rxc-c'))
    commands = 0

    def run(label, command, success=True, input_bytes=None, cwd=work, silent=False):
        global commands
        commands += 1
        result = subprocess.run([str(part) for part in command], cwd=cwd, env=env,
                                input=input_bytes, capture_output=True, timeout=300)
        assert (result.returncode == 0) == success, (
            label, result.returncode, result.stdout[-4000:], result.stderr[-4000:])
        assert not result.stderr, (label, result.stderr[-4000:])
        if silent:
            assert not result.stdout, (label, result.stdout[-4000:])
        return result

    def compile_source(source, destination, success=True):
        return run('compile ' + source.name,
                   [host_vm, '--no-autoload', image, '-a',
                    *(['-n'] if args.ir == 'off' else []), '-i', runtime,
                    '-o', destination, source], success, cwd=host, silent=success)

    assembly = target / 'parse.rxas'
    compile_source(sources / 'frontend-c.crexx', assembly)
    text = assembly.read_text(encoding='utf-8')
    assert re.search(r'load\s+r\d+,\s*0x4[cC]435031', text), 'LCP1 is embedded as binary'
    assert 'rexxclassicparse.execute' in text, 'one shared executor ABI'
    assert not re.search(r'^\s*(?:parseplan|parseword|parsepos|parseeos)\s', text, re.M), 'no legacy template replay'
    assert str(Path(__file__).resolve().parents[4]) not in text, 'logical caller source names'
    anchors = {(int(clause), int(line), int(column)) for clause, line, column in
               re.findall(r'\.srcstep \d+ (\d+) 17 "frontend-c\.crexx" (\d+) (\d+) ', text)}
    byte_offset = 0
    original_source = (sources / 'frontend-c.crexx').read_bytes().decode('utf-8')
    for line_number, line in enumerate(original_source.splitlines(keepends=True), 1):
        if line.lstrip().lower().startswith('parse '):
            column = len(line) - len(line.lstrip()) + 1
            assert (byte_offset + column, line_number, column) in anchors, ('PARSE authored clause/column', line_number)
        byte_offset += len(line.encode())
    assert re.search(r'^\s*call\s+[^\n]*rxfnsb\.sourceinfo\(\)', text, re.M), 'SOURCEINFO belongs to generated caller'
    if args.inspect:
        args.inspect.mkdir(parents=True, exist_ok=True)
        shutil.copy2(assembly, args.inspect / f'parse-{args.mode}-{args.ir}-{args.vm}.rxas')
    for assembler in ('noopt', 'opt'):
        run('assemble ' + assembler, [tool('rxas'), *(['-n'] if assembler == 'noopt' else []), assembly], silent=True)
        linked = target / ('parse-' + assembler + '-linked')
        run('link ' + assembler, [tool('rxlink'), '-o', linked, assembly.with_suffix('.rxbin'),
                                 *(runtime / (name + '.rxbin') for name in
                                   ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core'))], silent=True)
        result = run('execute ' + assembler, [target_vm, '--no-autoload', linked.with_suffix('.rxbin')],
                     input_bytes=b'pull line\nlinein line\n', cwd=target)
        assert result.stdout.replace(b'\r\n', b'\n') == expected, (assembler, result.stdout, expected)
    # Small caller where duplicated source/target machinery is conspicuous.
    simple = work / 'simple-c.crexx'
    simple.write_text("options levelc\nparse value 'one two three' with FIRST REST\nsay FIRST || '|' || REST\n", encoding='utf-8')
    simple_assembly = target / 'simple.rxas'
    compile_source(simple, simple_assembly)
    simple_text = simple_assembly.read_text(encoding='utf-8')
    assert len(re.findall(r'^\s*call\s+[^\n]*rexxclassicparse\.execute\(\)', simple_text, re.M)) == 1
    assert not re.search(r'^\s*call\s+[^\n]*llctarget\.(?:parsesource|poolwrite)\(\)', simple_text, re.M)
    assert len(re.findall(r'^\s*load\s+r\d+,\s*0x4[cC]435031', simple_text, re.M)) == 1
    if args.inspect:
        shutil.copy2(simple_assembly, args.inspect / f'parse-simple-{args.mode}-{args.ir}-{args.vm}.rxas')
    run('simple assemble', [tool('rxas'), '-n', simple_assembly], silent=True)
    simple_linked = target / 'simple-linked'
    run('simple link', [tool('rxlink'), '-o', simple_linked, simple_assembly.with_suffix('.rxbin'),
                        *(runtime / (name + '.rxbin') for name in
                          ('llctarget', 'library', 'rxfnsc', 'classlib', 'rxunicode-core'))], silent=True)
    result = run('simple execute', [target_vm, '--no-autoload', simple_linked.with_suffix('.rxbin')], cwd=target)
    assert result.stdout.replace(b'\r\n', b'\n') == b'one|two three\n', result.stdout
    sentinel = work / 'previous.rxas'
    for name, code in (('source', b'25.13'), ('position', b'38.2'), ('pattern', b'38.1')):
        sentinel.write_bytes(b'previous output')
        result = compile_source(sources / ('frontend-invalid-' + name + '.crexx'), sentinel, False)
        assert code in result.stdout and b'LL_COMPILER_SYNTAX' in result.stdout, result.stdout
        assert sentinel.read_bytes() == b'previous output', name
        assert not Path(str(sentinel) + '.llrxc-stage').exists(), name
    print(f'PARSE_FRONTEND|commands={commands}|failures=0|seconds={time.monotonic()-started:.3f}')
