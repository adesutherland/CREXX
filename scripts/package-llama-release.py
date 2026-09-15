"""Package an optional provider against a previously qualified, unchanged core."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess

_spec = importlib.util.spec_from_file_location('core_package', Path(__file__).with_name('package-core-release.py'))
core = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(core)


def verify_core(prefix, commit, platform, toolchain):
    manifest = json.loads((prefix / 'core-package.json').read_text())
    for name, value in dict(component='core', commit=commit, platform=platform,
                            toolchain=toolchain, llama_enabled=False).items():
        if manifest.get(name) != value:
            raise ValueError('Core identity mismatch: ' + name)
    for name, expected in manifest['files'].items():
        relative = Path(name)
        if relative.is_absolute() or '..' in relative.parts:
            raise ValueError('Nonlocal core file: ' + name)
        if core.digest(prefix / relative) != expected:
            raise ValueError('Core file changed: ' + name)
    return manifest


def add_file(source, relative, plugin, base):
    target = plugin / relative
    existing = base / relative
    if existing.exists():
        if core.digest(existing) != core.digest(source):
            raise ValueError('Plugin would replace a qualified core file: ' + str(relative))
        return
    target.parent.mkdir(parents=True, exist_ok=True)
    if source.is_symlink():
        link = os.readlink(source)
        if Path(link).is_absolute() or '..' in Path(link).parts:
            raise ValueError('Nonlocal provider link: ' + link)
        target.symlink_to(link)
    else:
        shutil.copy2(source, target)


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--core-archive', type=Path, required=True)
    p.add_argument('--build', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--commit', required=True)
    p.add_argument('--platform', required=True)
    p.add_argument('--toolchain', required=True)
    p.add_argument('--backend', choices=['vulkan', 'cuda', 'metal'], required=True)
    args = p.parse_args()
    output, build = args.output.resolve(), args.build.resolve()
    base_name = 'CREXX-' + args.platform
    core.extract(args.core_archive, output / 'base')
    base = output / 'base' / base_name
    manifest = verify_core(base, args.commit, args.platform, args.toolchain)
    plugin = output / 'plugin-stage' / base_name
    plugin.mkdir(parents=True)
    providers = build / 'bin/providers'
    native = json.loads((providers / 'rxllama.native.json').read_text())
    runtime = json.loads((providers / 'rxllama.runtime.json').read_text())
    for entry in native['link_libraries'] + native['runtime_files']:
        relative = Path(entry['path'])
        if relative.is_absolute() or '..' in relative.parts:
            raise ValueError('Nonlocal provider file: ' + str(relative))
        if core.digest(providers / relative) != entry['sha256']:
            raise ValueError('Provider hash mismatch: ' + str(relative))
    backends = [entry['backend'] for entry in runtime['backends']]
    for required in ['cpu', args.backend]:
        if not any(name == required or name.startswith(required + '-') for name in backends):
            raise ValueError('Missing packaged backend: ' + required)
    for file in sorted(providers.rglob('*')):
        if file.is_file() or file.is_symlink():
            add_file(file, Path('bin/providers') / file.relative_to(providers), plugin, base)
    extension = '.exe' if args.platform.startswith('windows-') else ''
    for name in ['rxllama.rxplugin', 'crexx-provider-package' + extension]:
        add_file(build / 'bin' / name, Path('bin') / name, plugin, base)
    if extension:
        inventory = build / 'lib/plugins/llama/tests/release-smoke-bootstrap-files.txt'
        for name in inventory.read_text().splitlines():
            if Path(name).name != name:
                raise ValueError('Nonlocal bootstrap file: ' + name)
            add_file(build / 'bin' / name, Path('bin') / name, plugin, base)
    subprocess.run(['cmake', '--install', str(build), '--prefix', str(plugin),
                    '--component', 'llama-docs'], check=True, stdout=subprocess.DEVNULL)
    if list(plugin.rglob('*.gguf')):
        raise ValueError('Model leaked into plugin archive')
    identity = dict(schema=1, component='llama.rexx', commit=args.commit,
        platform=args.platform, toolchain=args.toolchain, backend=args.backend,
        core_archive=args.core_archive.name, core_archive_sha256=core.digest(args.core_archive),
        core_binaries_rebuilt=False, files={file.relative_to(plugin).as_posix(): core.digest(file)
            for file in sorted(plugin.rglob('*')) if file.is_file()})
    (plugin / 'llama-package.json').write_text(json.dumps(identity, indent=2) + '\n')
    archive = output / 'assets' / ('llama.rexx-user-test-' + args.commit + '-' +
                                 args.platform + '-' + args.backend + '.zip')
    core.archive(plugin, archive)
    combined = output / 'combined'
    core.extract(args.core_archive, combined)
    core.extract(archive, combined)
    verify_core(combined / base_name, args.commit, args.platform, args.toolchain)
    for name, expected in identity['files'].items():
        if core.digest(combined / base_name / name) != expected:
            raise ValueError('Extracted plugin file changed: ' + name)
    qa = output / 'qa'; qa.mkdir()
    (qa / 'archive.json').write_text(json.dumps(dict(name=archive.name,
        bytes=archive.stat().st_size, sha256=core.digest(archive),
        **{k:v for k,v in identity.items() if k != 'files'}), indent=2) + '\n')
    print('PACKAGED: ' + str(archive))
    print('CONSUMER_PAYLOAD: ' + str(combined / base_name))


if __name__ == '__main__':
    main()
