#!/usr/bin/env python3
"""Build the real rxbuild demo rule with private import producers and real tools."""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile
import time


def main():
    started = time.monotonic()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--bin', type=Path, required=True)
    parser.add_argument('--cmake', required=True)
    parser.add_argument('--generator', required=True)
    parser.add_argument('--make-program', required=True)
    args = parser.parse_args()
    source, binary = args.source.resolve(), args.bin.resolve()

    def run(command):
        result = subprocess.run([str(x) for x in command], capture_output=True,
                                text=True, timeout=120)
        assert result.returncode == 0, (command, result.stdout[-4000:], result.stderr[-4000:])
        return result.stdout

    with tempfile.TemporaryDirectory(prefix='rxbuild demo ') as temp:
        root = Path(temp)
        fixture, build = root/'source', root/'build'
        (fixture/'library').mkdir(parents=True)
        for name in ['library', 'rxcexits']:
            shutil.copyfile(binary/(name+'.rxbin'), fixture/'library'/(name+'.rxbin'))
        # Keep the producer in another directory, like the real library. Both
        # target ordering and file freshness must survive that boundary.
        (fixture/'library/CMakeLists.txt').write_text('''
foreach(name library rxcexits)
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/${name}.rxbin"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${name}.rxbin"
    "${CMAKE_BINARY_DIR}/bin/${name}.rxbin"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${name}.rxbin" VERBATIM)
endforeach()
add_custom_target(library DEPENDS "${CMAKE_BINARY_DIR}/bin/library.rxbin")
add_custom_target(compiler_exit_bin DEPENDS "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin")
''')
        declarations = []
        for name in ['rxc', 'rxas', 'rxlink', 'rxcpack', 'rxlang']:
            # Only rxc/rxas execute for rxbuild_example. Other imported tools
            # allow the unchanged component declarations to configure.
            declarations += [f'add_executable({name} IMPORTED GLOBAL)',
                             f'set_target_properties({name} PROPERTIES IMPORTED_LOCATION "{(binary/name).as_posix()}")']
        (fixture/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.24)
project(rxbuild_demo_order C)
set(BUILD_TESTING OFF)
function(configure_linker_for_static_lib)
endfunction()
function(crexx_install_targets)
endfunction()
foreach(target rxfnsl fs fs_static _platform _platform_static _hash _hash_static)
  add_custom_target(${target})
endforeach()
add_library(rxvml INTERFACE)
add_library(rxpashim INTERFACE)
add_subdirectory(library)
''' + '\n'.join(declarations) + f'''
add_subdirectory("{source.as_posix()}/tools/rxbuild" rxbuild)
''')
        run([args.cmake, '-S', fixture, '-B', build, '-G', args.generator,
             '-DCMAKE_MAKE_PROGRAM='+args.make_program])
        command = [args.cmake, '--build', build, '--target', 'rxbuild_example', '--parallel', '8']
        products = [build/'bin/library.rxbin', build/'bin/rxcexits.rxbin', build/'build/build_demo.rxas',
                    build/'build/build_demo.rxbin']

        def snapshot():
            return [p.stat().st_mtime_ns for p in products]

        def execute():
            output = run([binary/'rxbvm', products[3], products[0], '-a', 'smoke'])
            assert output.splitlines() == ['RXBUILD DEMO: generated program executed',
                                           'RXBUILD DEMO ARGUMENT: smoke'], output

        run(command)  # No other target is built first to mask a missing edge.
        execute()
        initial = snapshot()
        run(command)
        assert snapshot() == initial, 'unchanged inputs regenerated the demo'

        for index, name in enumerate(['library', 'rxcexits']):
            for change in ['missing', 'changed']:
                previous = snapshot()
                time.sleep(1.05)  # Make/filesystems may use one-second mtimes.
                if change == 'missing':
                    products[index].unlink()
                else:
                    (fixture/'library'/(name+'.rxbin')).touch()
                run(command)
                current = snapshot()
                assert all(current[i] > previous[i] for i in [index, 2, 3]), (name, change, 'demo not rebuilt')
                assert current[1-index] == previous[1-index], (name, change, 'unrelated import rebuilt')
                execute()
        final = snapshot()
        run(command)
        assert snapshot() == final, 'final no-op regenerated the demo'
    print('RXBUILD_DEMO_BUILD|PASS|clean,missing-imports,changed-imports,noops,execution')
    print(f'Fixture elapsed: {time.monotonic() - started:.2f}s (scheduling observation only)')


if __name__ == '__main__':
    main()
