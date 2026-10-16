#!/usr/bin/env python3
"""Focused paired compiler-mode orchestration; all compiler logic is Level B."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time

parser = argparse.ArgumentParser()
parser.add_argument('--build', type=Path, required=True)
parser.add_argument('--work', type=Path, required=True)
parser.add_argument('--mode', choices=['opt', 'noopt'], default='noopt')
parser.add_argument('--vm', default='rxbvm')
args = parser.parse_args()
build = args.build.resolve()
work = args.work.resolve()
work.mkdir(parents=True, exist_ok=True)
backend = build/'lib/rxfnsl/compilers/backend'/args.mode
root = Path(__file__).resolve().parent
runtime = work/'runtime'
runtime.mkdir(exist_ok=True)
for name in ['library', 'rxfnsc', 'classlib']:
    shutil.copyfile(build/'bin'/f'{name}.rxbin', runtime/f'{name}.rxbin')
shutil.copyfile(backend/'llctarget.rxbin', runtime/'llctarget.rxbin')
suffix = '.exe' if os.name == 'nt' else ''
tool = lambda name: build/'bin'/(name+suffix)
records = []
started = time.monotonic()


def run(label, argv):
    before = time.monotonic()
    command = [str(x) for x in argv]
    p = subprocess.run(command, cwd=runtime, capture_output=True, timeout=300)
    (work/(label+'.stdout')).write_bytes(p.stdout)
    (work/(label+'.stderr')).write_bytes(p.stderr)
    records.append(dict(label=label, command=command, status=p.returncode,
                        seconds=round(time.monotonic()-before, 3)))
    assert p.returncode == 0, (label, p.returncode, p.stdout[-2000:], p.stderr[-2000:])
    return p


try:
    p = run('admission', [tool(args.vm), backend/'backend-optimise-linked.rxbin', '-a', runtime])
    assert b'failures=0' in p.stdout
    for level in ['c', 'g']:
        for name, source, expected in [
            ('branch', root/'../examples/step05'/f'branch-{level}.crexx', b'yes\n'),
            ('cleanup', root/'backend'/f'cleanup-{level}.crexx',
             b'\nkept\n0.3333\n11\n' if level == 'c' else b'\n\n11\ncalled\n'),
        ]:
            assembly_text = {}
            for ir_mode in ['off', 'on']:
                label = f'{name}-{level}-{ir_mode}'
                assembly = work/(label+'.rxas')
                run(label+'-compile', [tool(args.vm), backend/f'llrxc-{level}-linked.rxbin',
                    '-a', *(['-n'] if ir_mode == 'off' else []), '-i', runtime, '-o', assembly, source])
                assembly_text[ir_mode] = assembly.read_text()
                assert str(source.resolve().parent) not in assembly_text[ir_mode]
                # Assembler no-opt isolates the compiler IR transformations.
                run(label+'-assemble', [tool('rxas'), '-n', assembly])
                linked = work/(label+'-linked')
                modules = [assembly.with_suffix('.rxbin'), runtime/'library.rxbin']
                if level == 'c':
                    modules += [runtime/'llctarget.rxbin', runtime/'rxfnsc.rxbin']
                run(label+'-link', [tool('rxlink'), '-o', linked, *modules])
                p = run(label+'-run', [tool(args.vm), str(linked)+'.rxbin'])
                assert p.stdout.replace(b'\r\n', b'\n') == expected and not p.stderr, (label, p.stdout, p.stderr)
            if name == 'branch':
                assert len(assembly_text['on']) < len(assembly_text['off'])
                assert '"no"' not in assembly_text['on'] and '"yes"' in assembly_text['on']
                assert source.name in assembly_text['on']
    print(f'OPTIMISE_E2E|commands={len(records)}|failures=0')
finally:
    inputs = [Path(__file__), root/'backend-optimise.crexx',
              *sorted((root/'../examples/step05').glob('*.crexx')),
              *sorted((root/'backend').glob('cleanup-*.crexx'))]
    (work/'receipt.json').write_text(json.dumps(dict(mode=args.mode, vm=args.vm,
        seconds=round(time.monotonic()-started, 3), commands=records,
        inputs={str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}), indent=2)+'\n')
