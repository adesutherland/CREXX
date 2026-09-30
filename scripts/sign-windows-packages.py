#!/usr/bin/env python3
"""Sign retained Windows core/plugins and build matching installers locally.

No build, release mutation or upload. Supply explicit archives and the native
manager artifact from their Build run. Output hashes describe final signed bytes.
"""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

SCRIPTS = Path(__file__).resolve().parent


def module(name):
    spec = importlib.util.spec_from_file_location(name, SCRIPTS / (name + '.py'))
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


package = module('package-core-release')
installer = module('package-llama-installer')
refresh = module('refresh-package-manifests')


def unpack(archive, destination, manifest):
    package.extract(archive, destination)
    matches = list(destination.glob('*/' + manifest))
    if len(matches) != 1:
        raise ValueError('Expected one ' + manifest + ' in ' + str(archive))
    root = matches[0].parent
    return root, installer.verify_manifest(root, manifest)


def validate_pair(core, addon):
    if core.get('component') != 'core' or core.get('llama_enabled') is not False:
        raise ValueError('Expected llama-free core')
    if addon.get('component') != 'llama.rexx' or addon.get('backend') not in ('vulkan', 'cuda'):
        raise ValueError('Expected Windows Vulkan/CUDA plugin')
    for key in ('commit', 'platform', 'toolchain'):
        if core[key] != addon[key]:
            raise ValueError('Core/plugin identity mismatch: ' + key)
    if core['platform'] != 'windows-x64' or core['toolchain'] != 'msvc':
        raise ValueError('Expected Windows MSVC delivery')
    if set(core['files']) & set(addon['files']):
        raise ValueError('Plugin overlaps a core file')


def sign(root):
    subprocess.run(['bash', '-c',
        'set -euo pipefail; source "$1"; sign_windows_payload "$2" "$3" "$4" "$5"',
        'sign-windows-package', str(SCRIPTS / 'windows-signing-common.sh'), str(root),
        os.environ.get('PROVIDER', str(SCRIPTS / 'provider.macos.cfg')),
        os.environ.get('CERTUM_ALIAS', '7DDC0FE9C4D43C9D1D900B39548410F1'),
        os.environ.get('TSA_URL', 'http://time.certum.pl')], check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--core', type=Path, required=True)
    parser.add_argument('--plugin', type=Path, action='append', required=True)
    parser.add_argument('--manager', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        parser.error('Output must be a new directory')
    args.output.mkdir(parents=True)
    outputs = []
    with tempfile.TemporaryDirectory(prefix='crexx-signed-delivery-') as tmp:
        work = Path(tmp)
        core, base = unpack(args.core, work / 'core', 'core-package.json')
        plugins = [(*unpack(path, work / ('plugin-' + str(i)), 'llama-package.json'), path)
                   for i, path in enumerate(args.plugin)]
        for _, addon, _ in plugins:
            validate_pair(base, addon)
        if len({addon['backend'] for _, addon, _ in plugins}) != len(plugins):
            raise ValueError('Duplicate plugin backend')
        manager_data = installer.verify_manifest(args.manager, 'manager.json')
        refresh.update(args.manager, verify=True)
        if manager_data.get('core_commit') != base['commit']:
            raise ValueError('Manager/core identity mismatch')
        for name, expected in manager_data.get('bootstrap_files', {}).items():
            if base['files'].get(name) != expected:
                raise ValueError('Manager bootstrap differs from core: ' + name)
        manager = work / 'manager'
        shutil.copytree(args.manager, manager)
        sign(core)
        # Bootstrap runtime bytes are core-owned. Reuse the signed bytes rather
        # than independently signing the manager copy with a different timestamp.
        for name in manager_data.get('bootstrap_files', {}):
            if name not in base['files']:
                raise ValueError('Manager bootstrap is not core-owned: ' + name)
            shutil.copy2(core / name, manager / name)
        refresh.update(manager)
        sign(manager)
        for root, _, _ in plugins:
            sign(root)
        for root, source in [(core, args.core), *[(r, p) for r, _, p in plugins]]:
            target = args.output / (source.stem + '-signed.zip')
            package.archive(root, target)
            outputs.append(target)
        core_setup = outputs[0].with_name(outputs[0].stem + '-setup.exe')
        subprocess.run(['bash', str(SCRIPTS / 'package-latest-windows-installer.sh'),
                        '--zip', str(outputs[0]), '--sign', '--no-upload',
                        '--output', str(core_setup)], check=True)
        outputs.append(core_setup)
        version = (core / 'VERSION').read_text().strip().removeprefix('crexx-')
        for root, _, source in plugins:
            setup = args.output / (source.stem + '-signed-setup.exe')
            subprocess.run([sys.executable, str(SCRIPTS / 'package-llama-installer.py'),
                '--core', str(core), '--plugin', str(root), '--manager', str(manager),
                '--version', version, '--output', str(setup),
                '--sign-helper', str(SCRIPTS / 'sign-windows-file.sh')], check=True)
            outputs.append(setup)
        record = dict(commit=base['commit'], platform=base['platform'],
                      inputs={str(p.resolve()): package.digest(p) for p in [args.core, *args.plugin]},
                      manager_input=manager_data,
                      manager_signed=json.loads((manager / 'manager.json').read_text()), files={p.name: package.digest(p) for p in outputs})
        (args.output / 'signed-delivery.json').write_text(json.dumps(record, indent=2) + '\n')
    print('Signed archives/installers prepared and verified locally; nothing published.')


if __name__ == '__main__':
    main()
