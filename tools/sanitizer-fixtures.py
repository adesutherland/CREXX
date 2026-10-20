#!/usr/bin/env python3
"""Ordinary fixture ownership and native instrumentation for sanitizer QA.

The seal is disposable build state, never release/SAN assurance. Native files
are never copied from the donor. Original producer rules own full qualification.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import shlex
import subprocess
import sys
import tempfile

SCHEMA = 1
MANIFEST = 'qa-fixtures.json'
CONFIG = 'qa-fixture-config.json'
ALLOWED = {'.rxbin', '.rxas', '.crexx', '.rexx', '.c'}


def digest(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def json_read(path):
    return json.loads(Path(path).read_text(encoding='utf-8'))


def json_write(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(mode='w', dir=path.parent, delete=False,
                                     encoding='utf-8') as stream:
        temporary = Path(stream.name)
        json.dump(value, stream, sort_keys=True, indent=2)
        stream.write('\n')
    try:
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def source_identity(root, include_docs=True):
    root = Path(root).resolve()
    tracked = subprocess.check_output(
        ['git', '-C', str(root), 'ls-files', '--cached', '--recurse-submodules', '-z'])
    untracked = subprocess.check_output(
        ['git', '-C', str(root), 'ls-files', '--others', '--exclude-standard', '-z'])
    h = hashlib.sha256()
    for name in sorted(set((tracked + untracked).decode().split('\0')) - {''}):
        path = root / name
        # Documentation/history-only updates do not invalidate generated code.
        if (not path.is_file() or
                (not include_docs and (name.startswith('docs/') or path.suffix.lower() in {'.md', '.tex', '.pdf'})) or
                path.suffix.lower() == '.pyc' or
                '__pycache__' in path.parts):
            continue
        h.update(name.encode() + b'\0' + digest(path).encode() + b'\n')
    return h.hexdigest()


def dependency_identity(config):
    result = {}
    for name in ('DSLSH_SOURCE_DIR',):
        if config.get(name):
            root = Path(config[name])
            result[name] = {
                'contents': source_identity(root),
                'commit': subprocess.check_output(
                    ['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip(),
            }
    return result


def normalized_config(build, config):
    result = {}
    for name, value in config.items():
        if name in {'CREXX_QA_FIXTURE_TREE', 'DSLSH_SOURCE_DIR'}:
            continue
        if name.startswith(('CMAKE_C_FLAGS', 'CMAKE_CXX_FLAGS', 'CMAKE_EXE_LINKER_FLAGS',
                            'CMAKE_SHARED_LINKER_FLAGS', 'CMAKE_MODULE_LINKER_FLAGS')):
            # This is the only admitted difference between the two builds.
            value = re.sub(r'(?<!\S)-fsanitize=address(?!\S)|(?<!\S)-fno-omit-frame-pointer(?!\S)', '', value)
            value = ' '.join(value.split())
        result[name] = value.replace(config.get('CMAKE_BINARY_DIR', str(build)), '<BUILD>').replace(str(build), '<BUILD>')
    return result


def identity(build):
    build = Path(build).resolve()
    config = json_read(build / CONFIG)
    return {
        'source': source_identity(config['CMAKE_HOME_DIRECTORY'], include_docs=False),
        'configuration': normalized_config(build, config),
        'dependencies': dependency_identity(config),
    }


def fixture_files(build):
    return (Path(build) / 'qa-fixture-files.txt').read_text(encoding='utf-8').splitlines()


def producer_paths(build):
    suffix = '.exe' if os.name == 'nt' else ''
    return [Path(build) / 'bin' / (name + suffix) for name in ('rxc', 'rxas', 'rxlink')]


def seal(build):
    build = Path(build).resolve()
    config = json_read(build / CONFIG)
    if config.get('CREXX_QA_FIXTURE_TREE') or (build / '.crexx-external-fixtures-used').exists():
        raise ValueError('a fixture donor must use its original producers')
    effective = config.get('CMAKE_BUILD_TYPE', '').upper()
    active_flags = ' '.join(config.get(key, '') for key in (
        'CMAKE_C_FLAGS', 'CMAKE_CXX_FLAGS', 'CMAKE_C_FLAGS_' + effective,
        'CMAKE_CXX_FLAGS_' + effective))
    if '-fsanitize=' in active_flags:
        raise ValueError('fixture donor must be an ordinary, uninstrumented build')
    prepared = {}
    for name in fixture_files(build):
        path = build / name
        if path.is_file():
            if path.suffix not in ALLOWED:
                raise ValueError(f'unaudited fixture type: {name}')
            prepared[name] = digest(path)
    value = {
        'schema': SCHEMA, 'identity': identity(build), 'artifacts': prepared,
        'producers': {str(path.relative_to(build)): digest(path) for path in producer_paths(build)},
    }
    json_write(build / MANIFEST, value)
    print(f'Sealed {len(prepared)} ordinary generated fixture files; native files stay separate.')


def donor(build):
    build = Path(build).resolve()
    config = json_read(build / CONFIG)
    if not config.get('CREXX_QA_FIXTURE_TREE'):
        return None, None
    root = Path(config['CREXX_QA_FIXTURE_TREE']).resolve()
    if root == build or root in build.parents or build in root.parents:
        raise ValueError('ordinary donor and instrumented build must be disjoint trees')
    manifest = json_read(root / MANIFEST)
    if manifest.get('schema') != SCHEMA:
        raise ValueError('unsupported ordinary fixture seal')
    return root, manifest


def validate(build, artifacts=False):
    build = Path(build).resolve()
    root, manifest = donor(build)
    if root is None:
        return
    expected = manifest['identity']
    for candidate in (root, build):
        actual = identity(candidate)
        # Contents and actual checkout identity both matter for external code.
        if actual != expected:
            raise ValueError(f'ordinary fixture source/configuration/dependency mismatch: {candidate}')
    for name, checksum in manifest['producers'].items():
        if digest(root / name) != checksum:
            raise ValueError(f'ordinary fixture producer changed after sealing: {name}')
    json_write(build / '.qa-validated-donor.json', {'seal': digest(root / MANIFEST)})
    if artifacts:
        covered = set()
        for receipt in (build / '.qa-fixture-owners').glob('*.json'):
            owner = json_read(receipt)
            if owner['seal'] != digest(root / MANIFEST):
                raise ValueError('copied fixture owner belongs to a different seal')
            for name in owner['files']:
                path = build / name
                source = root / name
                if (not path.is_file() or not source.is_file() or
                        digest(path) != manifest['artifacts'].get(name) or digest(source) != manifest['artifacts'].get(name)):
                    raise ValueError(f'missing or changed copied fixture owner member: {name}')
                covered.add(name)
        for name in fixture_files(build):
            path = build / name
            if path.exists() and name not in covered:
                raise ValueError(f'fixture lacks an installed owner; use a fresh external-fixture build: {name}')


def copy_files(build, names):
    build = Path(build).resolve()
    root, manifest = donor(build)
    if root is None:
        raise ValueError('copy requires an external ordinary fixture tree')
    for name in names:
        relative = Path(name)
        if relative.is_absolute() or '..' in relative.parts or relative.suffix not in ALLOWED:
            raise ValueError(f'unaudited fixture name: {name}')
    destinations = [build / name for name in names]
    owner_key = hashlib.sha256(json.dumps(names).encode()).hexdigest()
    receipt = build / '.qa-fixture-owners' / (owner_key + '.json')
    try:
        if json_read(build / '.qa-validated-donor.json').get('seal') != digest(root / MANIFEST):
            raise ValueError('ordinary fixture seal changed during the build')
        sources = []
        for name in names:
            relative = Path(name)
            source = root / relative
            checksum = manifest['artifacts'].get(name)
            if checksum is None or not source.is_file() or digest(source) != checksum:
                raise ValueError(f'missing, partial or changed ordinary fixture: {name}')
            sources.append(source)
        # Validate the entire owner set before publishing any output.
        for source, destination in zip(sources, destinations):
            destination.parent.mkdir(parents=True, exist_ok=True)
            with tempfile.NamedTemporaryFile(dir=destination.parent, delete=False) as stream:
                temporary = Path(stream.name)
            try:
                shutil.copyfile(source, temporary)
                os.replace(temporary, destination)
            finally:
                temporary.unlink(missing_ok=True)
        json_write(receipt, {'seal': digest(root / MANIFEST), 'files': names})
    except BaseException:
        receipt.unlink(missing_ok=True)
        for path in destinations:
            path.unlink(missing_ok=True)
        raise


def native_format(header):
    archive = header.startswith((b'!<arch>\n', b'!<thin>\n'))
    elf = (header.startswith(b'\x7fELF') and len(header) >= 18 and
           int.from_bytes(header[16:18], 'little' if header[5] == 1 else 'big') in (2, 3))
    macho = (len(header) >= 16 and header[:4] in (b'\xcf\xfa\xed\xfe', b'\xfe\xed\xfa\xcf', b'\xce\xfa\xed\xfe', b'\xfe\xed\xfa\xce') and
             int.from_bytes(header[12:16], 'little' if header[0] in (0xcf, 0xce) else 'big') in (2, 6, 8))
    fat = header[:4] in (b'\xca\xfe\xba\xbe', b'\xca\xfe\xba\xbf')
    return archive or elf or macho or fat


def owned_native_files(build):
    """Linked native outputs/copies, including CTest-created child products.

    CMake's private compiler probes are configuration machinery, not first-party
    providers or test hosts. Relocatable object files have separate flag proof.
    """
    found = set()
    for directory, children, names in os.walk(build):
        relative = Path(directory).relative_to(build)
        if any(part in {'CompilerIdC', 'CompilerIdCXX', 'CMakeScratch', 'CMakeTmp'} for part in relative.parts):
            children[:] = []
            continue
        children[:] = [name for name in children if name not in {'.git', '__pycache__', 'asan-logs'}]
        for name in names:
            path = Path(directory) / name
            if name.startswith('CMakeDetermineCompilerABI_') or not path.is_file():
                continue
            with path.open('rb') as stream:
                header = stream.read(32)
            if native_format(header):
                found.add(path)
    return found


def native_closure(build, if_sanitized=False):
    build = Path(build).resolve()
    config = json_read(build / CONFIG)
    mode = config.get('CMAKE_BUILD_TYPE', '').upper()
    flags = ' '.join(config.get(name, '') for name in (
        'CMAKE_C_FLAGS', 'CMAKE_CXX_FLAGS', 'CMAKE_C_FLAGS_' + mode, 'CMAKE_CXX_FLAGS_' + mode))
    if '-fsanitize=address' not in flags:
        if if_sanitized:
            print('Ordinary native build: no sanitizer instrumentation claim.')
            return
        raise ValueError('native instrumentation verification requires an ASan build')
    if config.get('ENABLE_LLAMA') == 'ON':
        raise ValueError('this closure audit does not qualify the optional native-inference configuration')
    # Link-time symbols alone cannot prove that a provider's own source was
    # instrumented. Verify each existing object's effective compile command,
    # including the separately configured first-party external SDK consumer.
    databases = [build / 'compile_commands.json']
    databases.extend(path for path in build.rglob('compile_commands.json')
                     if path != databases[0] and '_deps' not in path.parts)
    objects = set()
    for database in databases:
        if not database.is_file():
            raise ValueError(f'native source instrumentation inventory missing: {database}')
        for entry in json_read(database):
            args = entry.get('arguments') or shlex.split(entry['command'])
            output = entry.get('output')
            if not output and '-o' in args:
                output = args[args.index('-o') + 1]
            if not output:
                raise ValueError(f'compile command has no owned object: {entry["file"]}')
            obj = Path(entry['directory']) / output
            if not obj.is_file():
                continue
            if ('-fsanitize=address' not in args or
                    any(arg in args for arg in ('-fno-sanitize=address', '-fno-sanitize=all'))):
                raise ValueError(f'first-party/native source lacks effective ASan compile flags: {entry["file"]}')
            objects.add(obj.resolve())
    if not objects:
        raise ValueError('empty native source instrumentation inventory')
    required = [build / 'bin' / name for name in ('rxc', 'rxas', 'rxlink', 'rxvm', 'rxbvm', 'rxtvm', 'crexx')]
    if config.get('CMAKE_SYSTEM_NAME') not in {'Linux', 'Darwin'}:
        raise ValueError('native closure audit is qualified only on Linux and Apple')
    for path in required:
        if not path.is_file():
            raise ValueError(f'missing required instrumented native product: {path}')
    if digest(build / 'bin/rxvm') not in {digest(build / 'bin/rxbvm'), digest(build / 'bin/rxtvm')}:
        raise ValueError('product rxvm is not the configured instrumented concrete engine')
    files = {Path(line).resolve() for line in (build / 'qa-native-files.txt').read_text().splitlines() if line}
    # Includes provider copies staged in imports, packages and nested SDK builds.
    files.update(build.rglob('*.rxplugin'))
    files.update(owned_native_files(build))
    for name in ('crexx_native_cflags', 'crexx_native_ldflags'):
        if '-fsanitize=address' not in (build / 'bin' / name).read_text():
            raise ValueError(f'driver native child configuration lacks ASan: {name}')
    files = {path.resolve() for path in files}
    checked = 0
    for path in sorted(files):
        if not path.is_file():
            continue  # unbuilt opt-in native targets are not a pass
        with path.open('rb') as stream:
            magic = stream.read(32)
        if not native_format(magic):
            continue  # intentionally malformed provider/container fixtures
        result = subprocess.run(['nm', '-a', str(path)], stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, check=False)
        if result.returncode or b'asan_init' not in result.stdout:
            raise ValueError(f'native executable/library/provider lacks ASan instrumentation: {path}')
        checked += 1
    if not checked:
        raise ValueError('empty native instrumentation audit')
    print(f'Verified ASan flags for {len(objects)} built native source objects and symbols in {checked} products/provider copies; optional unbuilt targets are unqualified.')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('command', choices=('prepare', 'seal', 'validate', 'copy', 'native'))
    p.add_argument('--build-dir', type=Path, required=True)
    p.add_argument('--build-jobs', type=int, default=4)
    p.add_argument('--build-leaks', choices=('on', 'off'), default='on')
    p.add_argument('--files', nargs='+')
    p.add_argument('--artifacts', action='store_true')
    p.add_argument('--if-sanitized', action='store_true')
    a = p.parse_args()
    if a.command == 'prepare':
        # Public preparation always completes the owning ordinary build graph
        # before sealing. seal is also exposed for independent fixture tests.
        runner = Path(__file__).with_name('asan-run.sh')
        base = [str(runner), '--build-dir', str(a.build_dir.resolve()), '--phase', 'build',
                '--build-jobs', str(a.build_jobs), '--build-leaks', a.build_leaks, '--no-live-tail']
        subprocess.run(base, check=True)
        subprocess.run([*base, '--build-target', 'qa-prep'], check=True)
        seal(a.build_dir)
    elif a.command == 'seal':
        seal(a.build_dir)
    elif a.command == 'validate':
        validate(a.build_dir, a.artifacts)
    elif a.command == 'copy':
        if not a.files:
            p.error('copy requires --files')
        copy_files(a.build_dir, a.files)
    else:
        native_closure(a.build_dir, a.if_sanitized)


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print(f'sanitizer-fixtures: {error}', file=sys.stderr)
        raise SystemExit(1)
