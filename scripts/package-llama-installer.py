#!/usr/bin/env python3
"""Wrap already qualified core/plugin payloads; never build the product/engine.

Python is a packaging dependency only. Recipients use the native installer and
the OS shell/PowerShell. Input manifests must describe final (post-signing) bytes.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def local_name(name):
    path = PurePosixPath(name)
    if (not name or path.is_absolute() or str(path) != name or
            any(part in ('..', '.') for part in path.parts) or
            any(c in name for c in '\r\n\t\\:*?"<>|') or
            any(part.endswith((' ', '.')) for part in path.parts)):
        raise ValueError('Unsafe package path: ' + name)
    return path


def verify_manifest(root, filename):
    manifest = json.loads((root / filename).read_text())
    files = manifest['files']
    folded = set()
    for name, expected in files.items():
        local_name(name)
        if name.casefold() in folded or not re.fullmatch('[0-9a-f]{64}', expected):
            raise ValueError('Invalid/duplicate manifest entry: ' + name)
        folded.add(name.casefold())
        target = root / name
        if not target.resolve().is_relative_to(root.resolve()) or digest(target) != expected:
            raise ValueError('Package file changed or escapes payload: ' + name)
    return manifest


def prepare(core, plugin, work):
    base = verify_manifest(core, 'core-package.json')
    addon = verify_manifest(plugin, 'llama-package.json')
    if base.get('component') != 'core' or base.get('llama_enabled') is not False:
        raise ValueError('Expected a llama-free core')
    if addon.get('component') != 'llama.rexx':
        raise ValueError('Expected a llama.rexx plugin')
    for key in ('commit', 'platform', 'toolchain'):
        if base[key] != addon[key]:
            raise ValueError('Core/plugin identity mismatch: ' + key)
    platform, backend = addon['platform'], addon['backend']
    if (platform, backend) not in {('windows-x64', 'vulkan'), ('windows-x64', 'cuda'),
                                  ('macos-arm64', 'metal'), ('macos-x86_64', 'cpu')}:
        raise ValueError('Unsupported plugin installer platform/backend')
    base_files = dict(base['files'], **{'core-package.json': digest(core / 'core-package.json')})
    plugin_files = dict(addon['files'], **{'llama-package.json': digest(plugin / 'llama-package.json')})
    if {n.casefold() for n in base_files} & {n.casefold() for n in plugin_files}:
        raise ValueError('Plugin overlaps a core file')
    actual = {p.relative_to(plugin).as_posix() for p in plugin.rglob('*') if p.is_file()}
    if actual != set(plugin_files) or any(n.endswith('.gguf') for n in actual):
        raise ValueError('Unlisted files or model weights in plugin payload')
    # Materialize only verified entries; preserve mode and follow only validated
    # intra-payload symlinks. Installers do not create destination symlinks.
    for name in plugin_files:
        target = work / 'payload' / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(plugin / name, target)
    metadata = dict(schema=1, commit=base['commit'], platform=platform,
                    toolchain=base['toolchain'], backend=backend,
                    core_files=base_files, plugin_files=plugin_files)
    (work / 'installer.json').write_text(json.dumps(metadata, indent=2) + '\n')
    for kind, files in [('core', base_files), ('plugin', plugin_files)]:
        (work / (kind + '.sha256')).write_text(''.join(
            value + '  ' + name + '\n' for name, value in sorted(files.items())))
    (work / 'backend').write_text(backend + '\n')
    return metadata


def verify_signatures(core, plugin, windows):
    """Fail closed for a signed installer wrapping unsigned native payloads."""
    for prefix in (core, plugin):
        for file in sorted(prefix.rglob('*')):
            if not file.is_file() or file.is_symlink():
                continue
            with file.open('rb') as stream:
                magic = stream.read(4)
            if windows and magic[:2] == b'MZ':
                # Maintainer signing is on macOS using the same verifier as
                # windows-signing-common.sh. Unsigned hosted QA skips this.
                subprocess.run(['osslsigncode', 'verify', '-in', str(file)],
                               check=True, stdout=subprocess.DEVNULL)
            elif not windows and magic in (b'\xcf\xfa\xed\xfe', b'\xce\xfa\xed\xfe',
                                           b'\xfe\xed\xfa\xcf', b'\xfe\xed\xfa\xce',
                                           b'\xca\xfe\xba\xbe', b'\xbe\xba\xfe\xca'):
                subprocess.run(['codesign', '--verify', '--strict', '-R',
                                'anchor apple generic', str(file)], check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--core', type=Path, required=True)
    parser.add_argument('--plugin', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--version', default='1.0.0')
    parser.add_argument('--prepare-only', action='store_true')
    parser.add_argument('--unsigned-qa', action='store_true')
    parser.add_argument('--sign-identity')
    parser.add_argument('--keychain')
    parser.add_argument('--sign-helper', type=Path)
    parser.add_argument('--makensis', default='makensis')
    args = parser.parse_args()
    args.output = args.output.resolve()
    if not re.fullmatch('[A-Za-z0-9.+~-]+', args.version):
        parser.error('Invalid package version')
    if not args.prepare_only and not (args.unsigned_qa or args.sign_identity or args.sign_helper):
        parser.error('Choose a signing identity/helper or explicitly --unsigned-qa')
    with tempfile.TemporaryDirectory(prefix='crexx-llama-installer-') as temporary:
        work = Path(temporary)
        meta = prepare(args.core, args.plugin, work)
        windows = meta['platform'].startswith('windows-')
        if windows and not args.prepare_only and not (ROOT / 'packaging/llama/llama.nsi').exists():
            parser.error('Windows installer wrapper awaits the CI-D04 backend coexistence decision')
        if not args.unsigned_qa and not args.prepare_only:
            verify_signatures(args.core, args.plugin, windows)
        extension = 'ps1' if windows else 'sh'
        shutil.copy2(ROOT / 'packaging/llama' / ('manage.' + extension), work / ('manage.' + extension))
        if args.prepare_only:
            shutil.copytree(work, args.output)
            print('Prepared installer inputs: ' + str(args.output))
            return
        args.output.parent.mkdir(parents=True, exist_ok=True)
        if windows:
            if not args.unsigned_qa and not args.sign_helper:
                parser.error('Windows requires --sign-helper')
            command = [args.makensis, '-V2', '-DLLAMA_STAGE=' + str(work),
                       '-DLLAMA_OUT=' + str(args.output), '-DLLAMA_BACKEND=' + meta['backend'],
                       '-DLLAMA_VERSION=' + args.version]
            if args.sign_helper:
                command += ['-DCREXX_SIGN_HELPER=' + str(args.sign_helper.resolve()),
                            '-DCREXX_SIGNED_PLUGIN_DIR=' + str(work / 'signed-nsis-plugins')]
            # NSIS on Windows uses / switches; Unix makensis uses - switches.
            if os.name == 'nt':
                command = [command[0]] + ['/' + arg[1:] for arg in command[1:]]
            subprocess.run(command + [str(ROOT / 'packaging/llama/llama.nsi')], check=True)
            if args.sign_helper:
                subprocess.run([str(args.sign_helper.resolve()), str(args.output)], check=True)
        else:
            if not args.unsigned_qa and not args.sign_identity:
                parser.error('Mac requires --sign-identity')
            for script in ('preinstall', 'postinstall'):
                shutil.copy2(ROOT / 'packaging/llama' / script, work / script)
                (work / script).chmod(0o755)
            command = ['pkgbuild', '--nopayload', '--scripts', str(work),
                       '--identifier', 'org.crexx.llama.' + meta['platform'] + '.' + meta['backend'],
                       '--version', args.version, '--install-location', '/']
            if args.sign_identity:
                command += ['--sign', args.sign_identity, '--timestamp']
                if args.keychain:
                    command += ['--keychain', args.keychain]
            subprocess.run(command + [str(args.output)], check=True)
    print('Packaged optional plugin installer: ' + str(args.output))


if __name__ == '__main__':
    main()
