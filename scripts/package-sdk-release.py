"""Package and qualify a relocatable installed SDK from a qualified core."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import stat
import subprocess
import zipfile


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def archive(payload, target):
    target.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as out:
        for path in sorted(payload.rglob('*')):
            name = path.relative_to(payload.parent).as_posix()
            if path.is_symlink():
                info = zipfile.ZipInfo(name)
                info.create_system = 3
                info.external_attr = (stat.S_IFLNK | 0o777) << 16
                out.writestr(info, os.readlink(path))
            elif path.is_file():
                out.write(path, name)


def extract(archive_path, destination):
    with zipfile.ZipFile(archive_path) as source:
        links = []
        for item in source.infolist():
            relative = Path(item.filename)
            if relative.is_absolute() or '..' in relative.parts:
                raise ValueError('Nonlocal archive entry: ' + item.filename)
            target = destination / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            mode = item.external_attr >> 16
            if stat.S_ISLNK(mode):
                link = source.read(item).decode('utf-8')
                if Path(link).is_absolute() or '..' in Path(link).parts:
                    raise ValueError('Nonlocal archive link: ' + link)
                links.append((target, link))
            elif not item.is_dir():
                target.write_bytes(source.read(item))
                if os.name != 'nt':
                    target.chmod(stat.S_IMODE(mode))
        for target, link in links:
            target.symlink_to(link)


def overlay_core(core, sdk):
    for path in sorted(core.rglob('*')):
        target = sdk / path.relative_to(core)
        if path.is_dir() and not path.is_symlink():
            target.mkdir(parents=True, exist_ok=True)
            continue
        target.parent.mkdir(parents=True, exist_ok=True)
        if target.exists() or target.is_symlink():
            target.unlink()
        if path.is_symlink():
            target.symlink_to(os.readlink(path))
        else:
            shutil.copy2(path, target)


def required_inventory(prefix, platform):
    suffix = '.lib' if platform.startswith('windows-') else '.a'
    required = (
        'BUILDINFO', 'VERSION', 'LICENSE', 'README.md', 'SECURITY.md',
        'INSTALL-RUN.md', 'core-package.json',
        'include/crexx_version.h', 'include/rxpa/crexxpa.h',
        'include/rxpa/crexx_version.h', 'include/platform/rxinteger.h',
        'lib/cmake/CREXX/CREXXConfig.cmake',
        'lib/cmake/CREXX/CREXXConfigVersion.cmake',
        'lib/cmake/CREXX/CREXXTargets.cmake',
        'lib/cmake/CREXX/RXPluginFunction.cmake',
        'lib/cmake/CREXX/WriteProviderPackage.cmake',
        'lib/cmake/CREXX/CrexxOperationContract.cmake',
        'bin/providers/rxsqlite.rxplugin',
        'bin/providers/rxsqlite' + suffix,
        'bin/providers/rxvector.rxplugin',
        'bin/providers/rxvector' + suffix,
        'bin/crexx' + ('.exe' if platform.startswith('windows-') else ''),
    )
    for name in required:
        if not (prefix / name).is_file():
            raise ValueError('SDK is missing ' + name)
    return required


def check_relocatable_cmake(prefix, build):
    cache = (build / 'CMakeCache.txt').read_text(errors='replace')
    source = next((line.split('=', 1)[1] for line in cache.splitlines()
                   if line.startswith('CMAKE_HOME_DIRECTORY:INTERNAL=')), None)
    if not source:
        raise ValueError('Cannot identify producer source tree')
    package_dir = prefix / 'lib' / 'cmake' / 'CREXX'
    for path in package_dir.glob('*.cmake'):
        contents = path.read_text(errors='replace')
        for producer in (str(build), source):
            if producer in contents:
                raise ValueError('Producer path leaked into ' + str(path))


def run_probe(prefix, logs):
    logs.mkdir(parents=True, exist_ok=True)
    work = prefix.parent / 'consumer with spaces'
    work.mkdir()
    cmake_source = work / 'probe'
    cmake_source.mkdir()
    (cmake_source / 'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.21)
project(CREXXSDKProbe NONE)
find_package(CREXX CONFIG REQUIRED PATHS "${SDK_PREFIX}" NO_DEFAULT_PATH)
if(NOT TARGET CREXX::RXPA)
  message(FATAL_ERROR "Missing installed CREXX::RXPA export")
endif()
foreach(name IN ITEMS rxc rxas rxlink rxvm crexx)
  if(NOT EXISTS "${CREXX_${name}_EXECUTABLE}")
    message(FATAL_ERROR "Missing installed ${name}")
  endif()
endforeach()
''')
    env = dict(os.environ)
    for name in ('CREXX_HOME', 'CREXX_PROVIDER_PATH', 'CREXX_DIR',
                 'CMAKE_PREFIX_PATH', 'LD_LIBRARY_PATH', 'DYLD_LIBRARY_PATH',
                 'DYLD_FALLBACK_LIBRARY_PATH'):
        env.pop(name, None)

    def run(label, argv, *, native=False):
        with (logs / (label + '.log')).open('w') as log:
            log.write('argv=' + repr(list(map(str, argv))) + '\n')
            log.flush()
            result = subprocess.run(list(map(str, argv)), cwd=work,
                                    env=dict(env, CREXX_HOME=str(prefix)) if native else env,
                                    stdout=log, stderr=log, timeout=1800)
        output = (logs / (label + '.log')).read_text(errors='replace')
        if result.returncode:
            raise RuntimeError(label + ' failed: ' + output[-3000:])
        return output

    run('find-package', ['cmake', '-S', cmake_source, '-B', work / 'probe-build',
                         '-DSDK_PREFIX=' + str(prefix)])
    consumer = work / 'native-consumer.crexx'
    consumer.write_text('''options levelb floats_binary
import rxfnsb
import rxfnsg
import rxsqlite
import rxvector
main: procedure = .int
  db = .binary
  if sqliteopen(":memory:", db) <> 0 then return 1
  if sqliteclose(db) <> 0 then return 2
  values = .packedfloat(1)
  call values.set(0, 1.0)
  bytes = rxvector..encodef32le(values)
  if binlength(bytes) <> 4 then return 3
  say "PASS: installed SQLite and vector native providers"
  return 0
''')
    output = work / 'native' / 'consumer'
    output.parent.mkdir()
    run('native-build', [prefix / 'bin' / ('crexx.exe' if os.name == 'nt' else 'crexx'),
                         '--program', output, consumer, '--jobs', '1', '--native'],
        native=True)
    executable = output.with_suffix('.exe') if os.name == 'nt' else output
    if not executable.is_file():
        raise ValueError('Native consumer was not produced')
    result = run('native-run', [executable])
    if 'PASS: installed SQLite and vector native providers' not in result:
        raise ValueError('Native consumer did not exercise installed providers')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--core', type=Path, required=True,
                        help='Extracted and qualified same-revision core prefix')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--platform', required=True)
    parser.add_argument('--toolchain', required=True)
    parser.add_argument('--commit', required=True)
    args = parser.parse_args()
    build, core, output = args.build.resolve(), args.core.resolve(), args.output.resolve()
    core_manifest = json.loads((core / 'core-package.json').read_text())
    if (core_manifest['component'] != 'core' or
            core_manifest['commit'] != args.commit or
            core_manifest['platform'] != args.platform or
            core_manifest['toolchain'] != args.toolchain):
        raise ValueError('Core revision/platform/toolchain mismatch')
    for name, expected in core_manifest['files'].items():
        if digest(core / name) != expected:
            raise ValueError('Core hash mismatch: ' + name)
    if ('commit=' + args.commit) not in (core / 'BUILDINFO').read_text().splitlines():
        raise ValueError('Core BUILDINFO source revision mismatch')

    payload = output / 'stage' / ('CREXX-sdk-' + args.platform)
    if payload.exists():
        shutil.rmtree(payload)
    payload.parent.mkdir(parents=True, exist_ok=True)
    logs = output / 'qa'
    logs.mkdir(parents=True, exist_ok=True)
    with (logs / 'install.log').open('w') as log:
        result = subprocess.run(['cmake', '--install', str(build), '--prefix', str(payload)],
                                stdout=log, stderr=log, timeout=1800)
    if result.returncode:
        raise RuntimeError('SDK install failed: ' + (logs / 'install.log').read_text()[-3000:])
    overlay_core(core, payload)
    required_inventory(payload, args.platform)
    check_relocatable_cmake(payload, build)
    if ('commit=' + args.commit) not in (payload / 'BUILDINFO').read_text().splitlines():
        raise ValueError('SDK BUILDINFO source revision mismatch')
    manifest = dict(schema=1, component='sdk', commit=args.commit,
                    platform=args.platform, toolchain=args.toolchain,
                    core_manifest_sha256=digest(core / 'core-package.json'),
                    files={p.relative_to(payload).as_posix(): digest(p)
                           for p in sorted(payload.rglob('*')) if p.is_file()})
    (payload / 'sdk-package.json').write_text(json.dumps(manifest, indent=2) + '\n')
    asset = output / 'assets' / ('CREXX-sdk-' + args.commit + '-' + args.platform + '.zip')
    archive(payload, asset)

    relocated = output / 'extracted with spaces'
    if relocated.exists():
        shutil.rmtree(relocated)
    extract(asset, relocated)
    installed = relocated / payload.name
    for name, expected in manifest['files'].items():
        if digest(installed / name) != expected:
            raise ValueError('SDK extracted hash mismatch: ' + name)
    required_inventory(installed, args.platform)
    check_relocatable_cmake(installed, build)
    run_probe(installed, logs)
    (logs / 'archive.json').write_text(json.dumps(dict(name=asset.name,
        bytes=asset.stat().st_size, sha256=digest(asset), commit=args.commit,
        platform=args.platform, toolchain=args.toolchain), indent=2) + '\n')
    print('PASS: installed SDK inventory, relocated CMake and static-provider native consumer')


if __name__ == '__main__':
    main()
