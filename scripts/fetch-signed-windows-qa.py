"""Fetch private draft QA artifacts, pin their commit/hashes; never publish."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess

spec = importlib.util.spec_from_file_location('core_package', Path(__file__).with_name('package-core-release.py'))
package = importlib.util.module_from_spec(spec)
spec.loader.exec_module(package)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--release-id', type=int, required=True)
    parser.add_argument('--commit', required=True)
    parser.add_argument('--work', type=Path, required=True)
    args = parser.parse_args()
    repo = os.environ['GITHUB_REPOSITORY']
    info = json.loads(subprocess.check_output(['gh', 'api', f'repos/{repo}/releases/{args.release_id}']))
    if not info['draft'] or info['target_commitish'] != args.commit or info['tag_name'] != 'qa-llama-signing-' + args.commit:
        raise RuntimeError('Not the private draft for this qualified candidate')
    assets = {a['name']: a for a in info['assets']}
    output = args.work / 'signed'
    output.mkdir()

    def download(name):
        if Path(name).name != name or '/' in name or '\\' in name:
            raise ValueError('Nonlocal asset name')
        target = output / name
        with target.open('wb') as stream:
            subprocess.run(['gh', 'api', f"repos/{repo}/releases/assets/{assets[name]['id']}",
                            '-H', 'Accept: application/octet-stream'], stdout=stream, check=True)
        if assets[name].get('digest') != 'sha256:' + package.digest(target):
            raise RuntimeError('GitHub asset digest mismatch: ' + name)
        return target

    record = json.loads(download('signed-delivery.json').read_text())
    if record['commit'] != args.commit or record['platform'] != 'windows-x64':
        raise RuntimeError('Signed delivery identity mismatch')
    for source, expected in record['inputs'].items():
        name = Path(source).name
        matches = list((args.work / 'retained').glob('*/' + name))
        if len(matches) != 1 or package.digest(matches[0]) != expected:
            raise RuntimeError('Signed input differs from qualified Build artifact: ' + name)
    for name, expected in record['files'].items():
        if package.digest(download(name)) != expected:
            raise RuntimeError('Signed output digest mismatch: ' + name)
    core = next(output.glob('CREXX-*-signed.zip'))
    package.extract(core, args.work / 'core')
    for backend in ('vulkan', 'cuda'):
        archives = list(output.glob('llama.rexx-*-' + backend + '-signed.zip'))
        if not archives:
            continue
        destination = args.work / 'retained' / backend
        for previous in destination.glob('*.zip'):
            previous.unlink()
        shutil.copy2(archives[0], destination / archives[0].name)
    (args.work / 'proof/signed-source.json').write_text(json.dumps(dict(release_id=info['id'],
        draft=True, commit=args.commit, delivery=record), indent=2) + '\n')


if __name__ == '__main__':
    main()
