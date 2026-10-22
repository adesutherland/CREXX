#!/usr/bin/env python3
"""Narrow STEP06 gap checks. Compiler algorithms and view consumer are Level B."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import time
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('--build', type=Path, required=True)
parser.add_argument('--work', type=Path, required=True)
parser.add_argument('--mode', choices=['opt', 'noopt'], default='opt')
parser.add_argument('--vm', default='rxbvm')
args = parser.parse_args()
build = args.build.resolve()
args.work.mkdir(parents=True, exist_ok=True)
run_directory = tempfile.TemporaryDirectory(prefix='check-',dir=args.work)
work = Path(run_directory.name)
root = Path(__file__).resolve().parent
examples = root.parent/'examples/step06'
backend = build/'research/level-l/compilers/backend'/args.mode
runtime = work/'runtime'
runtime.mkdir(exist_ok=True)
for name in ['library', 'rxfnsc', 'classlib', 'rxunicode-core']:
    shutil.copyfile(build/'bin'/f'{name}.rxbin', runtime/f'{name}.rxbin')
shutil.copyfile(backend/'llctarget.rxbin', runtime/'llctarget.rxbin')
suffix = '.exe' if os.name == 'nt' else ''
tool = lambda name: build/'bin'/(name+suffix)
environment = dict(os.environ, PATH=str(work/'empty-path'))
records, shapes = [], {}
started = time.monotonic()


def run(label, command, status=0):
    command = [str(x) for x in command]
    before = time.monotonic()
    p = subprocess.run(command, cwd=runtime, env=environment,
                       capture_output=True, timeout=300)
    (work/(label+'.stdout')).write_bytes(p.stdout)
    (work/(label+'.stderr')).write_bytes(p.stderr)
    records.append(dict(label=label, command=command, status=p.returncode,
                        seconds=round(time.monotonic()-before, 3)))
    assert (p.returncode == status if status is not None else p.returncode != 0), (
        label, p.returncode, p.stdout[-2000:], p.stderr[-2000:])
    return p


def compile_source(level, source, assembly, mode, label, status=0):
    return run(label, [tool(args.vm), backend/f'llrxc-{level}-linked.rxbin', '-a',
        *(['-n'] if mode == 'before' else []), '-i', runtime, '-o', assembly, source], status)


def execute(level, assembly, label, output, failure=False):
    # IR optimisation is the variable; assembler -n isolates its result.
    run(label+'-assemble', [tool('rxas'), '-n', assembly])
    linked = work/(label+'-linked')
    modules = [assembly.with_suffix('.rxbin'), runtime/'library.rxbin']
    if level == 'c':
        modules += [runtime/'llctarget.rxbin', runtime/'rxfnsc.rxbin',
                    runtime/'classlib.rxbin', runtime/'rxunicode-core.rxbin']
    run(label+'-link', [tool('rxlink'), '-o', linked, *modules])
    p = run(label+'-run', [tool(args.vm), str(linked)+'.rxbin'],
            (28 if level == 'c' else 5) if failure else 0)
    assert p.stdout.replace(b'\r\n', b'\n') == output, (label, p.stdout, output)
    if failure:
        error = b'RXC-LC-42.3' if level == 'c' else b'DIVISION_BY_ZERO'
        assert error in p.stderr and f'reached-{level}.crexx:4:10:'.encode() in p.stderr, (label, p.stderr)
    else:
        assert not p.stderr, (label, p.stderr)


def shape(text):
    # Count complete emitted modules, including wrapper and cold handlers.
    lines = [line.strip() for line in text.splitlines()]
    code = [line for line in lines if line and not line.startswith(('.', '*', '#', ';'))
            and not line.endswith(':') and not re.match(r'[^\s()]+\(\)', line)]
    return dict(instructions=len(code), calls=sum(x.startswith('call ') for x in code),
                source_records=sum(x.startswith('.srcstep ') for x in lines),
                frames=[int(x) for x in re.findall(r'\.locals=(\d+)', text)],
                bytes=len(text.encode()))


try:
    for level in ['c', 'g']:
        source = examples/f'walk-{level}.crexx'
        prefix = work/f'walk-{level}'
        p = run(f'views-{level}', [tool(args.vm), backend/'backend-poc-linked.rxbin',
            '-a', level, source, runtime, prefix])
        assert b'failures=0' in p.stdout, p.stdout
        expected = b'01\nok\n' if level == 'c' else b'1\nok\n'
        reached = work/f'reached-{level}.crexx'
        reached.write_bytes(source.read_bytes().replace(b'if 1', b'if 0'))
        broken = work/f'broken-{level}.crexx'
        broken.write_bytes(source.read_bytes().replace(b' then', b''))
        for mode in ['before', 'after']:
            label = f'walk-{level}-{mode}'
            assembly = work/(label+'.rxas')
            compile_source(level, source, assembly, mode, label+'-compile')
            # The public command and service must produce exactly the same output.
            assert assembly.read_bytes() == Path(str(prefix)+f'.{mode}.rxas').read_bytes()
            text = assembly.read_text()
            assert str(source.resolve().parent) not in text and 'generated Classic entry' not in text
            shapes[label] = shape(text)
            execute(level, assembly, label, expected)
            bad_assembly = work/f'reached-{level}-{mode}.rxas'
            compile_source(level, reached, bad_assembly, mode, f'reached-{level}-{mode}-compile')
            execute(level, bad_assembly, f'reached-{level}-{mode}', expected.split(b'\n')[0]+b'\n', True)
            saved = assembly.read_bytes()
            p = compile_source(level, broken, assembly, mode, f'syntax-{level}-{mode}', None)
            assert b'LL_COMPILER_SYNTAX' in p.stdout and b'THEN' in p.stdout, p.stdout
            # The expression remains viable until ELSE on the next line.
            assert b'line 4, column 1' in p.stdout and b"unexpected 'else'" in p.stdout, p.stdout
            assert assembly.read_bytes() == saved and not Path(str(assembly)+'.llrxc-stage').exists()
            control = root/'poc'/f'control-{level}.crexx'
            control_assembly = work/f'control-{level}-{mode}.rxas'
            compile_source(level, control, control_assembly, mode, f'control-{level}-{mode}-compile')
            expected_control = (b'rhs\n0\nrhs\n1\n1\nsetting 4\nsetting 1\n4\n1\n0\n9\n'
                                if level == 'c' else b'0\n1\n1\n')
            execute(level, control_assembly, f'control-{level}-{mode}', expected_control)
        assert shapes[f'walk-{level}-after']['instructions'] < shapes[f'walk-{level}-before']['instructions']
    print(f'POC_E2E|commands={len(records)}|failures=0')
finally:
    run_directory.cleanup()
