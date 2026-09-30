#!/usr/bin/env python3
"""Compile the Rexx management tool against an existing core; no engine build."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def build(core, output):
    manifest = json.loads((core / 'core-package.json').read_text())
    for name, expected in manifest['files'].items():
        if digest(core / name) != expected:
            raise ValueError('Changed retained core: ' + name)
    # Use the SDK headers belonging to this retained binary revision. They are
    # build inputs only; releases do not need to contain a C development kit.
    commit = manifest['commit']
    windows = os.name == 'nt'
    with tempfile.TemporaryDirectory(prefix='crexx-llama-native-') as temporary:
        work = Path(temporary)
        private = work / 'core'
        def clone(source, dest):
            try:
                os.link(source, dest)
            except OSError:
                shutil.copy2(source, dest)
        shutil.copytree(core / 'bin', private / 'bin', copy_function=clone)
        headers = work / 'sdk'
        for name in ('rxpa/crexxpa.h', 'platform/rxinteger.h'):
            path = headers / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(subprocess.check_output(['git', 'show', commit + ':' + name], cwd=ROOT))
        (headers / 'rxpa/crexx_version.h').write_text('#define rxpa_version "llama-manager-1"\n')
        native = ROOT / 'packaging/llama/native'
        providers = private / 'bin/providers'
        providers.mkdir(exist_ok=True)
        provider = 'rxfs'
        source = str(ROOT / 'lib/plugins/fs/rxfs.c')
        for path in providers.glob('rxfs*'):
            path.unlink()
        (private / 'bin/rxfs.rxplugin').unlink(missing_ok=True)
        include = str(headers / 'rxpa')
        if windows:
            common = ['cl', '/nologo', '/O2', '/MD', '/I' + include, '/DPLUGIN_ID=' + provider]
            subprocess.run(common + ['/LD', '/DBUILD_DLL', source, '/Fe:' + str(providers / (provider + '.rxplugin'))], cwd=work, check=True)
            subprocess.run(common + ['/c', source, '/Fo:' + str(work / 'io.obj')], cwd=work, check=True)
            subprocess.run(['lib', '/nologo', '/OUT:' + str(providers / (provider + '.lib')), str(work / 'io.obj')], check=True)
        else:
            cc = os.environ.get('CC', 'cc')
            common = [cc, '-O2', '-fPIC', '-I' + include, '-DPLUGIN_ID=' + provider]
            subprocess.run(common + ['-shared', '-DBUILD_DLL', source, '-o', str(providers / (provider + '.rxplugin'))], check=True)
            subprocess.run(common + ['-c', source, '-o', str(work / 'io.o')], check=True)
            subprocess.run(['ar', 'rcs', str(providers / (provider + '.a')), str(work / 'io.o')], check=True)
        suffix_archive = '.lib' if windows else '.a'
        shutil.copy2(providers / (provider + suffix_archive), providers / (provider + '_static' + suffix_archive))
        shutil.copy2(providers / (provider + '.rxplugin'), private / 'bin/rxfs.rxplugin')
        program = work / 'crexx_llama.crexx'
        shutil.copy2(native / 'crexx-llama.crexx', program)
        env = dict(os.environ, CREXX_HOME=str(private))
        suffix = '.exe' if windows else ''
        command = [str(private / ('bin/crexx' + suffix)), str(program), '--native', '--noexec',
                   '--nocolor', '-i', str(providers)]
        subprocess.run(command, cwd=work, env=env, check=True)
        built = work / ('crexx_llama' + suffix)
        subprocess.run([str(built), '--help'], check=True)
        contract = work / 'fs_contract.crexx'
        shutil.copy2(ROOT / 'lib/plugins/fs/rxfs_test.crexx', contract)
        subprocess.run([command[0], str(contract)] + command[2:], cwd=work, env=env, check=True)
        subprocess.run([str(work / ('fs_contract' + suffix)), 'native-manager'], cwd=work, check=True)
        (output / 'bin').mkdir(parents=True, exist_ok=True)
        shutil.copy2(built, output / ('bin/crexx-llama' + suffix))
        source_dir = output / 'share/llama-management'
        source_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(native / 'crexx-llama.crexx', source_dir / 'crexx-llama.crexx')
        for name in ('rxfs.c', 'rxfs_ops.h'):
            shutil.copy2(ROOT / 'lib/plugins/fs' / name, source_dir / name)
        files = {p.relative_to(output).as_posix(): digest(p) for p in sorted(output.rglob('*')) if p.is_file()}
        bootstrap = {}
        if windows:
            # NSIS runs the manager from its temporary directory before it can
            # validate the core. Carry the core's app-local CRT there too. These
            # bytes are already core-owned and are never installed a second time.
            for name, expected in manifest['files'].items():
                if re.fullmatch(r'bin/(?:vcruntime|msvcp|vccorlib|concrt)[^/]*\.dll', name, re.I):
                    shutil.copy2(core / name, output / name)
                    bootstrap[name] = expected
        metadata = dict(schema=1, core_commit=commit, files=files, bootstrap_files=bootstrap)
        (output / 'manager.json').write_text(json.dumps(metadata, indent=2) + '\n')
        print('Built cREXX native manager: ' + str(output))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--core', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        parser.error('Output must be a new directory')
    build(args.core.resolve(), args.output.resolve())
