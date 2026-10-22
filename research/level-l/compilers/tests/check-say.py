#!/usr/bin/env python3
"""SAY instruction regressions; orchestration only, compiler algorithms are B/L."""
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
parser.add_argument('--mode', choices=['opt', 'noopt'], default='opt')
parser.add_argument('--vm', choices=['rxbvm', 'rxtvm'], default='rxbvm')
parser.add_argument('--c-only', action='store_true',
                    help='check C in a staged compiler host and compiler-free target')
args = parser.parse_args()
build = args.build.resolve()
work_parent = args.work.resolve()
work_parent.mkdir(parents=True, exist_ok=True)
run_directory = tempfile.TemporaryDirectory(prefix='say-', dir=work_parent)
work = Path(run_directory.name)
sources = Path(__file__).resolve().parent / 'say'
backend = build / 'research/level-l/compilers/backend' / args.mode
runtime = work / 'runtime'
runtime.mkdir(exist_ok=True)
for name in ['library', 'rxfnsc', 'classlib', 'rxunicode-core']:
    shutil.copyfile(build / 'bin' / (name + '.rxbin'), runtime / (name + '.rxbin'))
shutil.copyfile(backend / 'llctarget.rxbin', runtime / 'llctarget.rxbin')
ext = '.exe' if os.name == 'nt' else ''
runtime_dlls = tuple(sorted((build / 'bin').glob('*.dll'))) if os.name == 'nt' else ()
dll_names = {path.name for path in runtime_dlls}
tool = lambda name: str(build / 'bin' / (name + ext))
env = dict(os.environ, PATH=str(work / 'no-native-compiler'))
records = []
compiler_vm = build / 'bin' / (args.vm + ext)
compiler_image = backend / 'llrxc-c-linked.rxbin'
compiler_directory = work
target_vm = compiler_vm
target_directory = work
vm_options = []


def stage_vm(directory, providers):
    directory.mkdir()
    shutil.copy2(build / 'bin' / (args.vm + ext), directory / (args.vm + ext))
    # Installed MSVC executables use the same app-local CRT closure as the
    # native-package relocation tests, independently of the caller's PATH.
    for path in runtime_dlls:
        shutil.copy2(path, directory / path.name)
    provider_directory = directory / 'providers'
    provider_directory.mkdir()
    for provider in providers:
        name = provider + '.rxplugin'
        shutil.copy2(build / 'bin' / name, provider_directory / name)
    assert {entry.name for entry in provider_directory.iterdir()} == {
        provider + '.rxplugin' for provider in providers}
    return directory / (args.vm + ext)


if args.c_only:
    # The VM also searches beside itself and the application, so an empty PATH
    # alone does not isolate a compiler running from the shared build bin/.
    # Native RXAS/RXLINK remain explicit tools outside these execution roots.
    (work / 'no-native-compiler').mkdir()
    env.pop('CREXX_PROVIDER_PATH', None)
    compiler_directory = work / 'compiler host'
    compiler_vm = stage_vm(compiler_directory, ['rxfs'])
    compiler_image = compiler_directory / 'llrxc-c-linked.rxbin'
    shutil.copyfile(backend / compiler_image.name, compiler_image)
    assert {entry.name for entry in compiler_directory.iterdir()} == {
        args.vm + ext, compiler_image.name, 'providers', *dll_names}
    target_directory = work / 'static target'
    target_vm = stage_vm(target_directory, ['rxfs', 'rxcstream'])
    assert {entry.name for entry in target_directory.iterdir()} == {
        args.vm + ext, 'providers', *dll_names}
    assert {entry.name for entry in runtime.iterdir()} == {
        'library.rxbin', 'rxfnsc.rxbin', 'classlib.rxbin', 'rxunicode-core.rxbin',
        'llctarget.rxbin'}
    for compiler in ['rxc', 'rxc-c']:
        assert shutil.which(compiler, path=env['PATH']) is None
    # Exact linked inputs must suffice: no bytecode package may be found by
    # autoload in either stage. Runtime imports above are compiler metadata and
    # explicit link inputs; they are never loaded by the compiler host VM.
    vm_options = ['--no-autoload']


def run(label, command, expected=0, isolated=False, cwd=None):
    before = time.monotonic()
    command = [str(x) for x in command]
    result = subprocess.run(command, cwd=cwd or work, capture_output=True, timeout=300,
                            env=env if isolated else None)
    (work / (label + '.stdout')).write_bytes(result.stdout)
    (work / (label + '.stderr')).write_bytes(result.stderr)
    records.append(dict(label=label, command=command, status=result.returncode,
                        seconds=round(time.monotonic() - before, 3)))
    assert result.returncode == expected, (label, result.returncode,
                                          result.stdout[-2000:], result.stderr[-2000:])
    return result


def compile_source(level, source, assembly, ir, expected=0):
    return run(assembly.stem + '-compile',
               [compiler_vm, *vm_options,
                compiler_image if args.c_only else backend / ('llrxc-' + level + '-linked.rxbin'), '-a',
                *(['-n'] if ir == 'noopt' else []), '-i', runtime, '-o', assembly, source],
               expected, isolated=True, cwd=compiler_directory)


try:
    forms = ('\n\n01\n a  b \nα漢🙂\u0080ÿ\nA\0B\u0080ÿ\n\u0080ÿ\nÃ©\n'
             '\x0fÿ\n\x1f\n\n\n41 tail\n41XYZ\nyes\ngroup\ninner\n'
             'outer=done\n0.333\n10.0E+6\n').encode()
    for ir in ['opt', 'noopt']:
        for name, level, expected, status in [
            ('forms', 'c', forms, 0),
            ('error', 'c', b'before\n', 28),
            ('literal', 'c', b'A\0B\xc2\x80\xc3\xbf\n', 0),
            ('forms', 'g', '\n01\nα漢🙂\n5\n'.encode(), 0),
        ]:
            if args.c_only and level != 'c':
                continue
            label = name + '-' + level + '-' + ir
            source = sources / (name + '-' + level + '.crexx')
            assembly = work / (label + '.rxas')
            compile_source(level, source, assembly, ir)
            text = assembly.read_text()
            assert str(source.parent) not in text, (label, 'physical source path')
            assert 'generated Classic entry' not in text
            if name == 'literal':
                assert len(re.findall(r'^call .*llctarget.sayvalue\(\)', text, re.M)) == 1
                assert 'llctarget.poolread' not in text and 'llctarget.binaryop' not in text
                assert 'x2c' not in text.lower() and 'b2x' not in text.lower()
                assert r'\x00' in text and r'\xC2\x80\xC3\xBF' in text
                assert re.search(r'\.srcstep \d+ 0 17 "literal-c.crexx" 2 5 ', text)
            run(label + '-assemble', [tool('rxas'), '-n', assembly])
            linked = work / (label + '-linked')
            modules = [assembly.with_suffix('.rxbin'), runtime / 'library.rxbin']
            if level == 'c':
                modules += [runtime / 'llctarget.rxbin', runtime / 'rxfnsc.rxbin',
                            runtime / 'classlib.rxbin', runtime / 'rxunicode-core.rxbin']
            run(label + '-link', [tool('rxlink'), '-o', linked, *modules])
            image = Path(str(linked) + '.rxbin')
            if args.c_only:
                # The deployment root contains only the VM, native transports
                # and linked static images, with no compiler or L SDK package.
                staged = target_directory / image.name
                shutil.copyfile(image, staged)
                image = staged
            result = run(label + '-run', [target_vm, *vm_options, image], status,
                         isolated=args.c_only, cwd=target_directory)
            assert result.stdout.replace(b'\r\n', b'\n') == expected, (label, result.stdout)
            if status:
                assert b'RXC-LC-42.3' in result.stderr and b'error-c.crexx:3:5' in result.stderr
            else:
                assert not result.stderr, (label, result.stderr)
        for index, content in enumerate(["say 'GG'x", "say '4 1'x", "say '1 1'b", 'say 1 +']):
            label = 'invalid-' + str(index) + '-' + ir
            source = work / (label + '.crexx')
            source.write_text(content)
            assembly = work / (label + '.rxas')
            assembly.write_bytes(b'previous user output')
            result = compile_source('c', source, assembly, ir, 1)
            message = result.stdout + result.stderr
            assert b'LL_COMPILER_SYNTAX' in message and source.name.encode() in message, message
            assert assembly.read_bytes() == b'previous user output'
    verdict = 'SAY C-ONLY PASS: ' if args.c_only else 'SAY PASS: '
    print(verdict + str(len(records)) + ' commands', flush=True)
finally:
    run_directory.cleanup()
