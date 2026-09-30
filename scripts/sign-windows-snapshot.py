#!/usr/bin/env python3
"""Sign and publish matching Windows core and llama snapshot ZIPs and installers."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

from importlib.util import module_from_spec, spec_from_file_location

SCRIPTS = Path(__file__).resolve().parent


def module(name):
    spec = spec_from_file_location(name, SCRIPTS / (name + '.py'))
    value = module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


assets = module('windows-release-assets')
paired = module('sign-windows-packages')


def capture_inputs(repo, tag, selected=None):
    info = assets.release(repo, tag)
    core_name = f'CREXX-{tag}-windows-x64.zip'
    plugins = sorted(a['name'] for a in info['assets']
                     if a['name'] in [f'llama.rexx-{tag}-windows-x64-{b}.zip'
                                      for b in ('vulkan', 'cuda')])
    if selected and selected != core_name:
        if selected not in plugins:
            raise ValueError('Select an unsigned Windows core or llama ZIP from this snapshot')
        plugins = [selected]
    if not plugins:
        raise ValueError('Snapshot has no Windows llama plugin ZIP')
    state = assets.capture(repo, tag, core_name)
    state['dependencies'] = [assets.capture(repo, tag, name) for name in plugins]
    assets.verify(state)
    return state


def manager_artifact(repo, commit):
    name = f'llama-manager-{commit}-windows-x64'
    pages = assets.api(f'repos/{repo}/actions/artifacts?name={name}&per_page=100',
                       '--paginate', '--slurp')
    for item in sorted((a for page in pages for a in page['artifacts']),
                       key=lambda a: a['id'], reverse=True):
        if (item['name'] != name or item['expired'] or
                item.get('workflow_run', {}).get('head_sha') != commit):
            continue
        run = assets.api(f"repos/{repo}/actions/runs/{item['workflow_run']['id']}")
        if (run['head_sha'] == commit and run['status'] == 'completed' and
                run['conclusion'] == 'success' and run['path'] == '.github/workflows/build.yml' and
                run['repository']['full_name'] == repo and
                run['head_repository']['full_name'] == repo):
            if not (item.get('digest') or '').startswith('sha256:'):
                raise ValueError('Manager artifact has no GitHub SHA-256 digest')
            return item
    raise ValueError('No retained manager from a successful matching Build: ' + name)


def download_manager(repo, item, work):
    archive = work / 'manager.zip'
    with archive.open('wb') as stream:
        subprocess.run(['gh', 'api', f"repos/{repo}/actions/artifacts/{item['id']}/zip"],
                       stdout=stream, check=True)
    if assets.digest(archive) != item['digest']:
        raise ValueError('Manager download does not match GitHub SHA-256')
    destination = work / 'manager'
    paired.package.extract(archive, destination)
    return destination


def sign_snapshot(state, item, work):
    sources = [state, *state['dependencies']]
    archives = []
    for index, source in enumerate(sources):
        path = work / source['asset']['name']
        assets.download(source, path)
        root, _ = paired.unpack(path, work / f'check-{index}',
                                'core-package.json' if index == 0 else 'llama-package.json')
        assets.verify_payload(source, root)
        archives.append(path)
    manager = download_manager(state['repo'], item, work)
    # The paired signer validates core/plugin/toolchain/manager and bootstrap
    # hashes before any key operation, and refreshes manifests after signing.
    output = work / 'signed'
    command = [sys.executable, str(SCRIPTS / 'sign-windows-packages.py'),
               '--core', str(archives[0]), '--manager', str(manager), '--output', str(output)]
    for archive in archives[1:]:
        command += ['--plugin', str(archive)]
    assets.verify(state)
    subprocess.run(command, check=True)
    record = json.loads((output / 'signed-delivery.json').read_text())
    expected = {name for archive in archives for name in
                (archive.stem + '-signed.zip', archive.stem + '-signed-setup.exe')}
    if record['commit'] != state['commit'] or set(record['files']) != expected:
        raise ValueError('Incomplete or mismatched signed delivery')
    paths = [output / name for name in sorted(expected)]
    for path in paths:
        if paired.package.digest(path) != record['files'][path.name]:
            raise ValueError('Signed output hash mismatch: ' + path.name)
    print('Publishing signed core and llama ZIPs/setups for ' + state['commit'], flush=True)
    assets.publish(state, paths)
    print('Done. Signed core and llama ZIPs/setups published; unsigned downloads retained.', flush=True)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('-R', '--repo', help='Defaults to the repository containing this script')
    p.add_argument('-t', '--tag', default='dev-snapshot')
    p.add_argument('-a', '--asset', help='Optionally select one unsigned plugin ZIP; matching core is always included')
    p.add_argument('--keep-unsigned', action='store_true', help='Compatibility option; unsigned inputs are always retained')
    p.add_argument('--delete-unsigned', action='store_true', help=argparse.SUPPRESS)
    p.add_argument('--dry-run', action='store_true', help='Inspect inputs without downloading, signing or publishing')
    p.add_argument('--keep-work', action='store_true')
    p.add_argument('--work-dir', type=Path, help='New or empty work directory; retained after exit')
    args = p.parse_args()
    if args.delete_unsigned:
        p.error('Snapshot signing retains unsigned assets; omit --delete-unsigned')
    repo = args.repo or subprocess.check_output(
        ['gh', 'repo', 'view', '--json', 'nameWithOwner', '--jq', '.nameWithOwner'],
        cwd=SCRIPTS.parent, text=True).strip()
    state = capture_inputs(repo, args.tag, args.asset)
    item = manager_artifact(repo, state['commit'])
    for source in [state, *state['dependencies']]:
        print('Source: ' + source['asset']['name'] + ' @ ' + source['commit'], flush=True)
    print(f"Manager: artifact {item['id']} / Build {item['workflow_run']['id']}", flush=True)
    if args.dry_run:
        print('Would sign and publish core and selected plugin ZIPs, payloads, uninstallers and setups.')
        return
    for tool in ('gh', 'jsign', 'osslsigncode', 'makensis', 'unzip', 'zip', 'file'):
        if not shutil.which(tool):
            p.error('Missing command: ' + tool)
    provider = Path(os.environ.get('PROVIDER', SCRIPTS / 'provider.macos.cfg'))
    if not provider.is_file():
        p.error('Provider configuration not found: ' + str(provider))
    if args.work_dir:
        work = args.work_dir.resolve()
        if work.exists() and any(work.iterdir()):
            p.error('--work-dir must be new or empty')
        work.mkdir(parents=True, exist_ok=True)
    else:
        work = Path(tempfile.mkdtemp(prefix='crexx-snapshot-sign-'))
    print('Working directory: ' + str(work), flush=True)
    try:
        (work / 'source.json').write_text(json.dumps(state, indent=2) + '\n')
        (work / 'manager-source.json').write_text(json.dumps(item, indent=2) + '\n')
        sign_snapshot(state, item, work)
    finally:
        if not (args.keep_work or args.work_dir):
            shutil.rmtree(work)


if __name__ == '__main__':
    main()
