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
for name in ['library', 'rxfnsc']:
    shutil.copyfile(build / 'bin' / (name + '.rxbin'), runtime / (name + '.rxbin'))
shutil.copyfile(backend / 'llctarget.rxbin', runtime / 'llctarget.rxbin')
ext = '.exe' if os.name == 'nt' else ''
tool = lambda name: str(build / 'bin' / (name + ext))
env = dict(os.environ, PATH=str(work / 'no-native-compiler'))
records = []


def run(label, command, expected=0, isolated=False):
    before = time.monotonic()
    command = [str(x) for x in command]
    result = subprocess.run(command, cwd=work, capture_output=True, timeout=300,
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
               [tool(args.vm), backend / ('llrxc-' + level + '-linked.rxbin'), '-a',
                *(['-n'] if ir == 'noopt' else []), '-i', runtime, '-o', assembly, source],
               expected, isolated=True)


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
                modules += [runtime / 'llctarget.rxbin', runtime / 'rxfnsc.rxbin']
            run(label + '-link', [tool('rxlink'), '-o', linked, *modules])
            result = run(label + '-run', [tool(args.vm), str(linked) + '.rxbin'], status)
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
    print('SAY PASS: ' + str(len(records)) + ' commands', flush=True)
finally:
    run_directory.cleanup()
