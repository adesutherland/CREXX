#!/usr/bin/env python3
"""Exercise the real Level L build declarations with instrumented tool stand-ins.

This checks scheduling/freshness only. Real-tool byte identity and execution are
qualified separately. No native compilation or compiler semantics live here.
"""
import argparse
from collections import Counter
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import time


def tool(name, argv):
    if name == 'rxtvm':
        output = Path(argv[0]).parent/'llsyntax.generated'
    elif '-o' in argv:
        output = Path(argv[argv.index('-o') + 1])
        output = output.with_suffix('.rxas' if name == 'rxc' else '.rxbin')
    elif name == 'rxlink':
        control = Path(argv[argv.index('-c') + 1]).read_text()
        output = Path(next(x[7:] for x in control.splitlines() if x.startswith('OUTPUT '))).with_suffix('.rxbin')
    else:
        output = Path(argv[-1]).with_suffix('.rxbin')
    output = output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    lock = Path(str(output)+'.busy')
    try:
        lock.mkdir()
    except FileExistsError:
        sys.exit(f'DUPLICATE PRODUCER: {name} {output}')
    try:
        with open(os.environ['LL_BUILD_EVENTS'], 'a') as stream:
            stream.write(json.dumps([name, str(output)])+'\n')
        # Widen the overlap window; completion is still bounded and deterministic.
        time.sleep(0.2)
        if name == 'rxtvm':
            print('generated syntax')
        else:
            output.write_text(f'{name}: {output.name}\n')
    finally:
        lock.rmdir()


def check():
    parser = argparse.ArgumentParser()
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--cmake', required=True)
    parser.add_argument('--make', required=True)
    args = parser.parse_args()
    source, work = args.source.resolve(), args.work.resolve()
    work.mkdir(parents=True, exist_ok=True)
    # Only this test's private generated fixture is replaced on subsequent runs.
    fixture, build = work/'source', work/'build'
    for path in [fixture, build]:
        if path.exists():
            shutil.rmtree(path)
    tools = fixture/'lib/rxfnsl/tools'
    tools.mkdir(parents=True)
    for name in ['CMakeLists.txt', 'generate.cmake']:
        shutil.copyfile(source/'lib/rxfnsl/tools'/name, tools/name)
    for directory in ['grammar', 'rexx', 'bootstrap']:
        (tools.parent/directory).mkdir()
    (tools/'levell.crexx').write_text('fixture driver\n')
    (tools.parent/'rexx/llfront.crexx').write_text('/* BEGIN FRONTEND READER */\n/* END FRONTEND READER */\n')
    (tools.parent/'bootstrap/seed-reader.inc').write_text('fixture seed\n')
    shutil.copyfile(source/'lib/rxfnsl/bootstrap/prepare-front.cmake', tools.parent/'bootstrap/prepare-front.cmake')
    for name in ['levell.levell', 'gennorm2.levell']:
        (tools.parent/'grammar'/name).write_text('fixture grammar\n')
    declarations = []
    for name in ['rxc', 'rxas', 'rxlink', 'rxtvm']:
        wrapper = fixture/name
        wrapper.write_text('#!/bin/sh\nexec '+shlex.quote(sys.executable)+' '+shlex.quote(str(Path(__file__).resolve()))+' --tool '+name+' "$@"\n')
        wrapper.chmod(0o755)
        declarations += [f'add_executable({name} IMPORTED GLOBAL)',
                         f'set_target_properties({name} PROPERTIES IMPORTED_LOCATION "{wrapper}")']
    (fixture/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.24)
project(level_l_build_order NONE)
set(CREXX_LEVEL_L_STANDALONE ON)
include("'''+source.as_posix()+'''/cmake/CrexxImportRoots.cmake")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
file(WRITE "${CMAKE_BINARY_DIR}/bin/library.rxbin" "base library")
file(WRITE "${CMAKE_BINARY_DIR}/bin/rxfnsl-base.rxbin" "base Level L")
add_custom_target(library)
add_custom_target(rxfnsl_base)
'''+ '\n'.join(declarations)+'''
add_subdirectory(lib/rxfnsl/tools)
add_custom_target(consume ALL)
add_dependencies(consume rxfnsl levell_unicode_lexer)
''')
    events = work/'events.jsonl'
    environment = dict(os.environ, LL_BUILD_EVENTS=str(events))
    records = []

    def run(label, command):
        started = time.monotonic()
        with (work/(label+'.log')).open('w') as log:
            result = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, env=environment, timeout=120)
        records.append(dict(label=label, status=result.returncode, seconds=round(time.monotonic()-started, 3)))
        if result.returncode:
            raise AssertionError(f'{label} failed: {work/(label+".log")}\n'+(work/(label+'.log')).read_text()[-4000:])

    def build_once(label, expected_stages):
        events.write_text('')
        run(label, [args.cmake, '--build', str(build), '--target', 'consume', '--parallel', '8'])
        calls = Counter(tuple(json.loads(line)) for line in events.read_text().splitlines())
        assert all(n == 1 for n in calls.values()), (label, calls)
        drivers = {Path(path).parent.name for (name, path) in calls if name == 'rxc' and Path(path).name == 'levell.rxas'}
        assert drivers == set(expected_stages), (label, drivers)
        if not expected_stages:
            assert not calls, (label, calls)
        records[-1]['tool_calls'] = sum(calls.values())
        records[-1]['driver_stages'] = sorted(drivers)

    run('configure', [args.cmake, '-S', str(fixture), '-B', str(build), '-G', 'Unix Makefiles', '-DCMAKE_MAKE_PROGRAM='+args.make])
    build_once('clean', ['L0', 'L1', 'L2'])
    build_once('noop', [])
    (build/'lib/rxfnsl/tools/bootstrap/L2/levell-linked.rxbin').unlink()
    build_once('missing-l2', ['L2'])
    time.sleep(1.05)  # Accommodate Make/filesystems with one-second mtime resolution.
    with (tools/'levell.crexx').open('a') as stream:
        stream.write('changed fixture driver\n')
    build_once('changed-input', ['L0', 'L1', 'L2'])
    build_once('final-noop', [])
    (work/'receipt.json').write_text(json.dumps(records, indent=2)+'\n')
    print('LEVEL_L_BUILD_ORDER|PASS|clean,missing-l2,changed-input,noops')


if __name__ == '__main__':
    if sys.argv[1:2] == ['--tool']:
        tool(sys.argv[2], sys.argv[3:])
    else:
        check()
