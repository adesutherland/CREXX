"""Verify published same-revision SDK and llama archives for a downstream build."""

import argparse
import importlib.util
import json
from pathlib import Path
import shutil

spec = importlib.util.spec_from_file_location(
    'sdk_package', Path(__file__).with_name('package-sdk-release.py'))
sdk = importlib.util.module_from_spec(spec)
spec.loader.exec_module(sdk)


def check_files(prefix, document, manifest_name):
    files = document['files']
    actual = {p.relative_to(prefix).as_posix() for p in prefix.rglob('*')
              if p.is_file() and p.name != manifest_name}
    if actual != set(files):
        raise ValueError('Archive inventory differs from ' + manifest_name)
    for name, expected in files.items():
        relative = Path(name)
        if relative.is_absolute() or '..' in relative.parts:
            raise ValueError('Nonlocal manifest entry: ' + name)
        if sdk.digest(prefix / relative) != expected:
            raise ValueError('Manifest hash mismatch: ' + name)


def release_digest(assets, name):
    matches = [entry for entry in assets['assets'] if entry['name'] == name]
    if len(matches) != 1 or not matches[0].get('digest', '').startswith('sha256:'):
        raise ValueError('Release SHA-256 is unavailable: ' + name)
    return matches[0]['digest'].split(':', 1)[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--llama', type=Path, required=True)
    parser.add_argument('--assets-json', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--commit', required=True)
    parser.add_argument('--platform', required=True)
    parser.add_argument('--toolchain', required=True)
    parser.add_argument('--backend', required=True)
    args = parser.parse_args()
    assets = json.loads(args.assets_json.read_text())
    sdk_name = 'CREXX-sdk-' + args.commit + '-' + args.platform + '.zip'
    llama_name = 'llama.rexx-dev-snapshot-' + args.platform + '-' + args.backend + '.zip'
    if args.sdk.name != sdk_name or args.llama.name != llama_name:
        raise ValueError('Archive name does not match the requested platform/revision')
    for path in (args.sdk, args.llama):
        if sdk.digest(path) != release_digest(assets, path.name):
            raise ValueError('Published archive SHA-256 mismatch: ' + path.name)

    output = args.output.resolve()
    if output.exists():
        shutil.rmtree(output)
    sdk.extract(args.sdk, output / 'sdk')
    sdk.extract(args.llama, output / 'llama')
    prefix = output / 'sdk' / ('CREXX-sdk-' + args.platform)
    plugin = output / 'llama' / ('CREXX-' + args.platform)
    sdk_manifest = json.loads((prefix / 'sdk-package.json').read_text())
    llama_manifest = json.loads((plugin / 'llama-package.json').read_text())
    for document, component in ((sdk_manifest, 'sdk'), (llama_manifest, 'llama.rexx')):
        for key, expected in dict(component=component, commit=args.commit,
                                  platform=args.platform, toolchain=args.toolchain).items():
            if document.get(key) != expected:
                raise ValueError(component + ' identity mismatch: ' + key)
    if llama_manifest['backend'] != args.backend:
        raise ValueError('Llama backend mismatch')
    if sdk_manifest['core_manifest_sha256'] != sdk.digest(prefix / 'core-package.json'):
        raise ValueError('SDK/core manifest mismatch')
    if ('commit=' + args.commit) not in (prefix / 'BUILDINFO').read_text().splitlines():
        raise ValueError('SDK BUILDINFO mismatch')
    check_files(prefix, sdk_manifest, 'sdk-package.json')
    check_files(plugin, llama_manifest, 'llama-package.json')
    sdk.required_inventory(prefix, args.platform)
    overlap = set(sdk_manifest['files']) & set(llama_manifest['files'])
    if overlap:
        raise ValueError('Llama would overwrite SDK files: ' + ', '.join(sorted(overlap)))
    sdk.overlay_core(plugin, prefix)
    report = dict(commit=args.commit, platform=args.platform,
                  toolchain=args.toolchain, backend=args.backend,
                  sdk_sha256=sdk.digest(args.sdk), llama_sha256=sdk.digest(args.llama),
                  sdk_prefix=str(prefix), verified=True)
    (output / 'receipt.json').write_text(json.dumps(report, indent=2) + '\n')
    print('PASS: published SDK and matching llama overlay ' + str(prefix))


if __name__ == '__main__':
    main()
