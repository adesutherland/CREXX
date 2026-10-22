#!/usr/bin/env python3
"""Check Classic Unicode provider selection using existing semantic fixtures."""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


parser = argparse.ArgumentParser()
parser.add_argument('--bin-root', type=Path, required=True,
                    help='build or installed bin directory; inputs are copied into exact roots')
parser.add_argument('--vm', choices=('rxbvm', 'rxtvm'), default='rxbvm')
parser.add_argument('--mode', choices=('opt', 'noopt'), default='opt')
args = parser.parse_args()
bin_root = args.bin_root.resolve()
source_root = Path(__file__).resolve().parents[4]
extension = '.exe' if os.name == 'nt' else ''
flags = ['-n'] if args.mode == 'noopt' else []
base_libraries = ('library', 'classlib', 'rxfnsc')
normalization_procedures = {
    'tonfd', 'tonfc', 'tonfkd', 'tonfkc', 'isnfd', 'isnfc', 'isnfkd', 'isnfkc',
}
runtime_dlls = tuple(sorted(bin_root.glob('*.dll'))) if os.name == 'nt' else ()
dll_names = {path.name for path in runtime_dlls}
commands = 0


with tempfile.TemporaryDirectory(prefix='crexx-classic-provider-') as temporary:
    work = Path(temporary)
    no_tools = work / 'no-tools'
    no_tools.mkdir()
    environment = dict(os.environ, PATH=str(no_tools))
    environment.pop('CREXX_PROVIDER_PATH', None)

    def run(label, argv, cwd, failure=False):
        global commands
        commands += 1
        try:
            result = subprocess.run([str(value) for value in argv], cwd=cwd,
                                    env=environment, capture_output=True, timeout=600)
        except subprocess.TimeoutExpired as problem:
            raise AssertionError((label, 'timeout', problem.stdout, problem.stderr)) from problem
        if (result.returncode != 0) != failure:
            raise AssertionError((label, result.returncode,
                                  result.stdout[-4000:], result.stderr[-4000:]))
        return result

    def copy_libraries(directory, libraries):
        for library in libraries:
            shutil.copy2(bin_root / (library + '.rxbin'),
                         directory / (library + '.rxbin'))

    def copy_runtime_dlls(directory):
        # Match the existing relocated native-package tests: installed MSVC
        # executables use app-local CRT DLLs rather than the caller's PATH.
        for path in runtime_dlls:
            shutil.copy2(path, directory / path.name)

    def stage_runtime(name, libraries):
        directory = work / name
        directory.mkdir()
        shutil.copy2(bin_root / (args.vm + extension), directory / (args.vm + extension))
        copy_runtime_dlls(directory)
        copy_libraries(directory, libraries)
        providers = directory / 'providers'
        providers.mkdir()
        for provider in ('rxfs', 'rxcstream'):
            filename = provider + '.rxplugin'
            source = bin_root / 'providers' / filename
            if not source.is_file():
                source = bin_root / filename
            shutil.copy2(source, providers / filename)
        assert {path.name for path in directory.iterdir()} == {
            args.vm + extension, 'providers',
            *(library + '.rxbin' for library in libraries), *dll_names}
        assert {path.name for path in providers.iterdir()} == {
            'rxfs.rxplugin', 'rxcstream.rxplugin'}
        return directory

    tools = work / 'tools'
    tools.mkdir()
    for tool in ('rxc-c', 'rxas', 'rxlink', 'rxdas'):
        shutil.copy2(bin_root / (tool + extension), tools / (tool + extension))
    copy_runtime_dlls(tools)
    assert {path.name for path in tools.iterdir()} == {
        *(name + extension for name in ('rxc-c', 'rxas', 'rxlink', 'rxdas')), *dll_names}
    tool = lambda name: tools / (name + extension)
    assert not (tools / ('rxc' + extension)).exists()
    imports = work / 'imports'
    imports.mkdir()
    copy_libraries(imports, (*base_libraries, 'rxunicode-core'))
    assert {path.name for path in imports.iterdir()} == {
        library + '.rxbin' for library in (*base_libraries, 'rxunicode-core')}

    # Check the wrapper image itself: the top-level C call imports rxfnsc;
    # public calls may inline directly to private helpers. All remaining Unicode
    # hints must select the B core, including the eight exposed normalization
    # procedures whose current call ABI retains ordinary calls.
    assembly = run('Classic wrapper metadata', [tool('rxdas'), imports / 'rxfnsc.rxbin'],
                   tools).stdout.decode('utf-8')
    hints = re.findall(r'^\s*\.meta\s+"(_?rxunicode\.[^"]+)"\s*=\s*"\.autoload"\s+"([^"]+)"',
                       assembly, re.MULTILINE | re.IGNORECASE)
    assert hints, 'Classic wrappers have no Unicode autoload hints'
    assert {'rxunicode.' + procedure for procedure in normalization_procedures} <= {
        symbol.lower() for symbol, _ in hints}, hints
    assert all(artifact == 'rxunicode-core' for _, artifact in hints), hints

    core = stage_runtime('core autoload', (*base_libraries, 'rxunicode-core'))
    combined = stage_runtime('G preloaded', (*base_libraries, 'rxfnsg'))
    static = stage_runtime('static target', ())
    assert not (core / 'rxfnsg.rxbin').exists()
    assert not (combined / 'rxunicode-core.rxbin').exists()
    for directory in (core, combined, static):
        assert not any((directory / (name + extension)).exists()
                       for name in ('rxc', 'rxc-c'))

    fixture_root = source_root / 'compiler/tests/rexx_src'
    fixtures = [
        (fixture_root / 'levelc_unicode_services.rexx',
         (fixture_root / 'levelc_unicode_services.expected').read_bytes().replace(b'\r\n', b'\n'), False),
        (source_root / 'tests/rxpa/classic_stream_package.rexx',
         b'PASS: packaged Classic encoded stream\n', True),
    ]
    for source, expected, stream in fixtures:
        # Keeping only the selected fixture beside the compiler also excludes
        # source-import fallback through the maintained fixture directories.
        case = tools / source.stem
        case.mkdir()
        staged_source = case / source.name
        shutil.copy2(source, staged_source)
        output = case / 'program'
        run(source.stem + ' compile',
            [tool('rxc-c'), *flags, '--no-exe-import', '-i', imports,
             '-o', output, staged_source], case)
        run(source.stem + ' assemble', [tool('rxas'), *flags, output], case)
        program = output.with_suffix('.rxbin')
        linked = static / source.stem
        run(source.stem + ' static link',
            [tool('rxlink'), '-o', linked, program,
             *(imports / (library + '.rxbin') for library in base_libraries),
             imports / 'rxunicode-core.rxbin'], case)

        for directory, provider in ((core, 'rxunicode-core'), (combined, 'rxfnsg'),
                                    (static, None)):
            if provider:
                staged_program = directory / (source.stem + '.rxbin')
                shutil.copy2(program, staged_program)
                libraries = [directory / (library + '.rxbin') for library in base_libraries]
                # Core is intentionally discovered from the migrated wrapper's
                # hint. G is explicitly loaded before any hints are considered.
                if provider == 'rxfnsg':
                    libraries.append(directory / 'rxfnsg.rxbin')
                argv = [directory / (args.vm + extension), staged_program, *libraries]
            else:
                argv = [directory / (args.vm + extension), '--no-autoload',
                        linked.with_suffix('.rxbin')]
            result = run(source.stem + ' ' + directory.name, argv, directory)
            assert result.stdout.replace(b'\r\n', b'\n') == expected, (
                source.stem, directory.name, result.stdout, expected)
            assert not result.stderr, (source.stem, directory.name, result.stderr)
            if stream:
                assert (directory / 'classic-stream.bin').read_bytes() == b'\x80\xe9'

        if not stream:
            duplicate = work / 'duplicate'
            result = run('duplicate Unicode providers',
                         [tool('rxlink'), '-o', duplicate, program,
                          *(imports / (library + '.rxbin') for library in base_libraries),
                          imports / 'rxunicode-core.rxbin', combined / 'rxfnsg.rxbin'],
                         case, failure=True)
            assert re.search(rb'Duplicate (providers|exported symbol)',
                             result.stdout + result.stderr), result.stderr
            assert not duplicate.with_suffix('.rxbin').exists()

    print(f'PASS: Classic Unicode providers ({commands} commands; {args.vm} {args.mode})')
