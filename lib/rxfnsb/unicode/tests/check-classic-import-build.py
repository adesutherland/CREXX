#!/usr/bin/env python3
"""Exercise the maintained Classic import-staging rule without building cREXX."""
import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile
import time


parser = argparse.ArgumentParser()
parser.add_argument('--cmake', default='cmake')
parser.add_argument('--generator', choices=('Unix Makefiles', 'Ninja'),
                    default='Unix Makefiles')
args = parser.parse_args()
source_root = Path(__file__).resolve().parents[4]
owner = (source_root / 'lib/rxfnsc/CMakeLists.txt').read_text(encoding='utf-8')
start = owner.index('set(RXFNSC_IMPORT_ROOT ')
end = owner.index('set(RXFNSC_SELECTORS ', start)
staging_rule = owner[start:end]
notice = re.search(r'COMMENT "([^"]+)"', staging_rule).group(1)
files = ('library.rxbin', 'classlib.rxbin', 'rxcexits.rxbin',
         'rxunicode-core.rxbin', 'rxcstream.rxplugin', 'rxfs.rxplugin')


with tempfile.TemporaryDirectory(prefix='crexx-classic-import-') as temporary:
    work = Path(temporary)
    source = work / 'source with spaces'
    build = work / 'build with spaces'
    source.mkdir()
    inputs = build / 'bin'
    inputs.mkdir(parents=True)
    for name in files:
        (inputs / name).write_bytes(('stand-in input: ' + name + '\n').encode('ascii'))
    imports = build / 'imports'
    imports.mkdir()
    stale = imports / 'rxfnsg.rxbin'
    stale.write_bytes(b'obsolete G metadata\n')
    # The real staging declarations supply every output, command and dependency.
    # Only their existing producer targets are replaced by fixed stand-in files.
    (source / 'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.24)
project(ClassicImportStaging NONE)
foreach(provider IN ITEMS cstream fs)
  add_library(${provider} UNKNOWN IMPORTED GLOBAL)
  set_target_properties(${provider} PROPERTIES
    IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/bin/rx${provider}.rxplugin")
endforeach()
add_custom_target(library DEPENDS "${CMAKE_BINARY_DIR}/bin/library.rxbin")
add_custom_target(classlib DEPENDS "${CMAKE_BINARY_DIR}/bin/classlib.rxbin")
add_custom_target(compiler_exit_bin DEPENDS "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin")
add_custom_target(rxunicode_core DEPENDS "${CMAKE_BINARY_DIR}/bin/rxunicode-core.rxbin")
''' + staging_rule, encoding='utf-8')

    def run(argv):
        result = subprocess.run([str(value) for value in argv], capture_output=True,
                                text=True, timeout=600)
        assert result.returncode == 0, (argv, result.returncode,
                                       result.stdout[-4000:], result.stderr[-4000:])
        return result.stdout + result.stderr

    def snapshot():
        return {name: (imports / name).stat().st_mtime_ns for name in files}

    def stage(label, refreshed):
        output = run([args.cmake, '--build', build, '--target', 'rxfnsc_imports',
                      '--parallel', '1'])
        assert (notice in output) == refreshed, (label, refreshed, output)
        assert {path.name for path in imports.iterdir()} == set(files), label
        for name in files:
            assert (imports / name).read_bytes() == (inputs / name).read_bytes(), (label, name)

    def noop(label):
        before = snapshot()
        stage(label, False)
        assert snapshot() == before, (label, 'unchanged inputs changed staged timestamps')

    run([args.cmake, '-S', source, '-B', build, '-G', args.generator])
    stage('cold staging', True)
    assert not stale.exists()
    noop('cold no-op')
    for name in files:
        (imports / name).unlink()
        stage('missing ' + name, True)
        noop('recovered ' + name + ' no-op')

    # A completed core refresh can preserve its bytes while advancing its date.
    # The staging recipe must consume that freshness once, then remain a no-op.
    core = inputs / 'rxunicode-core.rxbin'
    previous = snapshot()
    # Older Unix Make implementations compare whole seconds.
    next_second = max(previous.values()) // 1_000_000_000 + 1
    time.sleep(max(0, next_second - time.time()) + 0.02)
    os.utime(core, None)
    assert core.stat().st_mtime_ns > max(previous.values())
    stale.write_bytes(b'obsolete G metadata\n')
    stage('same-byte core refresh', True)
    noop('same-byte core refresh no-op')

    previous = snapshot()
    next_second = max(previous.values()) // 1_000_000_000 + 1
    time.sleep(max(0, next_second - time.time()) + 0.02)
    core.write_bytes(b'changed core input\n')
    assert core.stat().st_mtime_ns > max(previous.values())
    stage('changed-byte core refresh', True)
    noop('changed-byte core refresh no-op')
    print(f'PASS: Classic import staging ({args.generator}; cold, six missing outputs, same-byte and changed-byte refreshes, no-ops)')
