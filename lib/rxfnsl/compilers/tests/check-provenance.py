#!/usr/bin/env python3
"""Focused toolchain proof of source anchors and the existing fatal handler.

Compiler logic remains Level B. The injected RXAS faults exercise each admitted
VM signal in the generated handler without inventing extra C/G language syntax.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
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
backend = build / 'lib/rxfnsl/compilers/backend' / args.mode
sources = Path(__file__).resolve().parent / 'provenance'
runtime = work / 'runtime'
runtime.mkdir(exist_ok=True)
for name in ['library', 'rxfnsc']:
    shutil.copyfile(build/'bin'/f'{name}.rxbin', runtime/f'{name}.rxbin')
shutil.copyfile(backend/'llctarget.rxbin', runtime/'llctarget.rxbin')
suffix = '.exe' if os.name == 'nt' else ''
tool = lambda name: str(build/'bin'/(name+suffix))
records = []
started = time.monotonic()


def run(label, command, expected=0):
    before = time.monotonic()
    result = subprocess.run([str(x) for x in command], cwd=work,
                            capture_output=True, timeout=180)
    (work/(label+'.stdout')).write_bytes(result.stdout)
    (work/(label+'.stderr')).write_bytes(result.stderr)
    records.append(dict(label=label, command=[str(x) for x in command],
                        status=result.returncode, seconds=round(time.monotonic()-before, 3)))
    assert result.returncode == expected, (label, result.returncode, result.stdout[-2000:], result.stderr[-2000:])
    return result


def compile_source(level, source, name):
    assembly = work/(name+'.rxas')
    run(name+'-compile', [tool(args.vm), backend/f'llrxc-{level}-linked.rxbin',
                         '-a', '-i', runtime, '-o', assembly, source])
    text = assembly.read_text()
    assert str(source.parent) not in text, (name, 'physical source path leaked')
    assert 'generated Classic entry' not in text
    assert 'llctarget.failure()' not in text
    if level == 'c':
        assert 'signalorigin "CLASSIC_SYNTAX"' in text and 'entering $MAIN' in text
    # Retain distinct anchors and control-flow boundaries, while rejecting
    # repeated unchanged metadata/context within one emitted straight path.
    last_source = last_location = None
    location_register = None
    for row in text.splitlines():
        if '.locals=' in row:
            location_register = None
        if row.endswith(':') or '.locals=' in row:
            last_source = last_location = None
        if row.startswith('.srcstep '):
            key = row.split(' ', 3)[3]
            assert key != last_source, (name, 'redundant source record', row)
            last_source = key
        entry = re.match(r'load (r[0-9]+),".*: entering ', row)
        if entry:
            location_register = entry.group(1)
        if location_register and row.startswith('load '+location_register+','):
            value = row.split(',', 1)[1]
            assert value != last_location, (name, 'redundant location update', row)
            last_location = value
    # Records carry honest unknown clause IDs; ranges/flags are checked below
    # and in the B service tests. IDs and register numbers are not golden text.
    for row in text.splitlines():
        if row.startswith('.srcstep '):
            assert row.split()[2] == '0', row
    return assembly


def execute(level, assembly, label, mode, expected, strip=False):
    run(label+'-assemble', [tool('rxas')] + (['-n'] if mode == 'noopt' else []) + [assembly])
    linked = work/(label+'-linked')
    modules = [assembly.with_suffix('.rxbin'), runtime/'library.rxbin']
    if level == 'c':
        modules += [runtime/'llctarget.rxbin', runtime/'rxfnsc.rxbin']
    run(label+'-link', [tool('rxlink')] + (['-s'] if strip else []) + ['-o', linked] + modules)
    return run(label+'-run', [tool(args.vm), str(linked)+'.rxbin'], expected)


try:
    result = run('contracts', [tool(args.vm), backend/'backend-tests-linked.rxbin', '-a', runtime])
    assert b'failures=0' in result.stdout
    for level in ['c', 'g']:
        for name, output, line, column in [
            ('nested', '', 7, 9),
            ('loop', '', 6 if level == 'g' else 5, 20),
            ('skip', '2\n', 0, 0),
            ('unicode', 'αβ\n', 3, 5),
        ]:
            stem = name+'-'+level
            source = sources/(stem+'.crexx')
            assembly = compile_source(level, source, stem)
            text = assembly.read_text()
            if name == 'unicode':
                assert re.search(r'\.srcstep \d+ 0 17 "unicode-'+level+r'\.crexx" 3 5 10 "say 1 / 0"', text), text
            for mode in ['noopt', 'opt']:
                result = execute(level, assembly, stem+'-'+mode, mode, (28 if level == 'c' else 5) if line else 0)
                assert result.stdout.replace(b'\r\n', b'\n') == output.encode(), (stem, result.stdout)
                error = result.stderr.decode(errors='replace')
                if line:
                    assert f'{source.name}:{line}:{column}' in error, (stem, error)
                    if level == 'c':
                        assert 'RXC-LC-42.3' in error, error
                        fault = error.split('  called from:', 1)[0]
                        assert re.search(r'at module [0-9]+ \(RexxClassicBifs\) address [0-9]+', fault), error
                        # Some maintained runtime packages strip source records.
                        # Fault identity must survive even without source text.
                        if '  source:' in fault:
                            assert 'source: RexxClassicBifs.crexx:' in fault, error
                        assert 'source: llctarget.crexx:' not in fault, error
                    else:
                        assert 'DIVISION_BY_ZERO' in error, error
                else:
                    assert not error, error
            # Link-time stripping must not invent a source for the fault.
            if name == 'unicode':
                result = execute(level, assembly, stem+'-stripped', 'opt', 28 if level == 'c' else 5, True)
                assert b'at module ' in result.stderr and b'  source:' not in result.stderr

    # Same logical CLI basename under two physical roots: byte-identical RXAS.
    outputs = []
    for root in ['root-one', 'root-two']:
        directory = work/root
        directory.mkdir(exist_ok=True)
        source = directory/'same.crexx'
        source.write_text('options levelg\nsay 1\n')
        outputs.append(compile_source('g', source, root).read_bytes())
    assert outputs[0] == outputs[1]

    # Exercise all four dispatch arms, including an error before the first
    # authored operation, and assert the original fault anchor survives. This
    # does not claim general condition-object payload or handler semantics.
    source = work/'entry.crexx'
    source.write_text('options levelc\nsay 1\n')
    original = compile_source('c', source, 'entry').read_text()
    for signal, status in [('CLASSIC_SYNTAX', 28), ('DIVISION_BY_ZERO', 5),
                           ('CONVERSION_ERROR', 6), ('INVALID_ARGUMENTS', 7)]:
        assembly = work/(signal+'.rxas')
        marker = '.srcstep 900 0 17 "bound-fault.rexx" 7 3 8 "  fault"\n'
        injected = 'llb1:\n'+marker+'signal "'+signal+'","original-message"\n'
        assert original.count('llb1:\n') == 1
        assembly.write_text(original.replace('llb1:\n', injected))
        for mode in ['noopt', 'opt']:
            result = execute('c', assembly, signal+'-'+mode, mode, status)
            assert not result.stdout
            error = result.stderr.decode(errors='replace')
            assert 'entry.crexx: entering $MAIN: original-message' in error, error
            assert f'(SIGNAL {signal})' in error, error
            assert 'source: bound-fault.rexx:7:3:' in error, error
            assert 'source: llctarget.crexx:' not in error, error
    print('PROVENANCE_E2E|failures=0')
finally:
    inputs = [Path(__file__)] + sorted(sources.glob('*.crexx'))
    (work/'receipt.json').write_text(json.dumps(dict(mode=args.mode, vm=args.vm,
        seconds=round(time.monotonic()-started, 3), commands=records,
        inputs={str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}), indent=2)+'\n')
