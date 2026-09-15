"""Package and smoke a llama-free release core, using its actual extracted ZIP."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import stat
import subprocess
import tempfile
import time
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


def smoke(prefix, source, preferred_vm, logs):
    work = Path(tempfile.mkdtemp(prefix='crexx-core-consumer-'))
    extension = '.exe' if os.name == 'nt' else ''
    env = dict(os.environ)
    for key in ('CREXX_HOME', 'CREXX_PROVIDER_PATH', 'LD_LIBRARY_PATH',
                'DYLD_LIBRARY_PATH', 'DYLD_FALLBACK_LIBRARY_PATH', 'GGML_BACKEND_PATH'):
        env.pop(key, None)
    execution = dict(env)
    if os.name == 'nt':
        system = Path(env['SystemRoot'])
        execution['PATH'] = os.pathsep.join(map(str, (system / 'System32', system)))
    records = []
    started = time.monotonic()
    outcome = 'failed'

    def run(label, args, marker=None, native_build=False):
        args = list(map(str, args))
        with (logs / (label + '.log')).open('w') as log:
            log.write('argv=' + repr(args) + '\n'); log.flush()
            result = subprocess.run(args, cwd=work,
                env=dict(env, CREXX_HOME=str(prefix)) if native_build else execution,
                stdout=log, stderr=log, timeout=1800)
        output = (logs / (label + '.log')).read_text(errors='replace')
        records.append(dict(label=label, returncode=result.returncode))
        if result.returncode or any(x in output for x in ('PANIC:', 'FAIL:', 'ERROR: AddressSanitizer')):
            raise RuntimeError(label + ': ' + output[-2500:])
        if marker and marker not in output:
            raise RuntimeError(label + ': missing expected output ' + repr(marker))

    try:
        assert not list(prefix.rglob('*llama*')), 'llama component leaked into core'
        assert not list(prefix.rglob('*.gguf')), 'model leaked into core'
        entry = prefix / 'bin' / ('rxvm' + extension)
        selected = prefix / 'bin' / (preferred_vm + extension)
        assert selected.is_file() and entry.is_file()
        if os.name == 'nt':
            assert digest(entry) == digest(selected)
        else:
            assert entry.is_symlink() and os.readlink(entry) == preferred_vm
        program_source = work / 'hello.crexx'
        shutil.copy2(source / 'examples/hello.crexx', program_source)
        for mode in ('opt', 'noopt'):
            program = work / mode
            run(mode + '-compile', [prefix / ('bin/rxc' + extension), '--no-exe-import',
                *(['-n'] if mode == 'noopt' else []), '-i', prefix / 'bin',
                '-o', program, program_source])
            run(mode + '-assemble', [prefix / ('bin/rxas' + extension), '-o', program, program])
            linked = work / (mode + '-linked')
            run(mode + '-link', [prefix / ('bin/rxlink' + extension), '-o', linked,
                program, prefix / 'bin/library'])
            for vm in ('rxvm', *[x for x in ('rxbvm', 'rxtvm') if x != preferred_vm]):
                binary = prefix / 'bin' / (vm + extension)
                if binary.exists():
                    run(mode + '-' + vm, [binary, linked], 'hello CREXX world!')
                    assert '4711' in (logs / (mode + '-' + vm + '.log')).read_text()
        native = work / 'native' / 'hello'
        native.parent.mkdir()
        run('native-build', [prefix / ('bin/crexx' + extension), '--program', native,
            program_source, '--jobs', '1', '--native'], 'PUBLISHED: native program', True)
        relocated = work / 'relocated'
        relocated.mkdir()
        executable = native.with_suffix(extension) if extension else native
        shutil.copy2(executable, relocated / executable.name)
        # Core MSVC applications use the same app-local CRT as the base.
        if os.name == 'nt':
            for path in (prefix / 'bin').glob('*.dll'):
                shutil.copy2(path, relocated / path.name)
        shutil.rmtree(native.parent)
        run('relocated-native', [relocated / executable.name], 'hello CREXX world!')
        outcome = 'passed'
    finally:
        (logs / 'summary.json').write_text(json.dumps(dict(outcome=outcome,
            elapsed_seconds=round(time.monotonic() - started, 3), commands=records,
            preferred_vm=preferred_vm, llama_enabled=False, work=str(work)), indent=2) + '\n')
        if outcome == 'passed':
            shutil.rmtree(work)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--platform', required=True)
    parser.add_argument('--toolchain', required=True)
    parser.add_argument('--preferred-vm', choices=['rxbvm', 'rxtvm'], required=True)
    parser.add_argument('--commit', required=True)
    args = parser.parse_args()
    source, build, output = args.source.resolve(), args.build.resolve(), args.output.resolve()
    cache = (build / 'CMakeCache.txt').read_text()
    assert 'ENABLE_LLAMA:BOOL=OFF' in cache, 'core must be configured without llama'
    payload = output / 'stage' / ('CREXX-' + args.platform)
    payload.mkdir(parents=True)
    shutil.copytree(build / 'bin', payload / 'bin', symlinks=True)
    shutil.copytree(source / 'examples', payload / 'examples', symlinks=True)
    shutil.copytree(build / 'example-artifacts', payload / 'examples', dirs_exist_ok=True)
    for name in ('LICENSE', 'README.md', 'SECURITY.md', 'INSTALL-RUN.md'):
        shutil.copy2(source / name, payload / name)
    for name in ('VERSION', 'BUILDINFO'):
        shutil.copy2(build / 'generated' / name, payload / name)
    if args.toolchain == 'msvc':
        redist = Path(os.environ['VCToolsRedistDir']) / 'x64'
        runtimes = list(redist.glob('Microsoft.VC*.CRT/*.dll'))
        assert runtimes, 'MSVC redistributable runtime is missing'
        for path in runtimes:
            shutil.copy2(path, payload / 'bin' / path.name)
    manifest = dict(schema=1, component='core', commit=args.commit, platform=args.platform,
        toolchain=args.toolchain, preferred_vm=args.preferred_vm, llama_enabled=False,
        files={p.relative_to(payload).as_posix(): digest(p)
               for p in sorted(payload.rglob('*')) if p.is_file()})
    (payload / 'core-package.json').write_text(json.dumps(manifest, indent=2) + '\n')
    asset = output / 'assets' / ('CREXX-user-test-' + args.commit + '-' + args.platform + '.zip')
    archive(payload, asset)
    extracted = output / 'extracted'
    extract(asset, extracted)
    installed = extracted / payload.name
    for name, expected in manifest['files'].items():
        assert digest(installed / name) == expected, name
    logs = output / 'qa'
    logs.mkdir()
    smoke(installed, source, args.preferred_vm, logs)
    (logs / 'archive.json').write_text(json.dumps(dict(name=asset.name,
        bytes=asset.stat().st_size, sha256=digest(asset), **{k:v for k,v in manifest.items() if k != 'files'}), indent=2) + '\n')
    print('PASS: extracted llama-free core, both optimization modes, applicable VMs and relocated native consumer')


if __name__ == '__main__':
    main()
