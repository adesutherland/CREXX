#!/usr/bin/env python3
"""Qualify one CSV runtime on a disposable Linux/Windows x64 hosted runner."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import stat
import subprocess
import sys
import tempfile
import time
import urllib.request
import zipfile

ROOT = Path(__file__).resolve().parents[1]
LOCK = json.loads((ROOT / 'tests/hosted-runtimes.json').read_text(encoding='utf-8'))


def download(asset, directory):
    """Never execute an unverified download, including an HTML mirror error."""
    target = directory / asset['url'].rsplit('/', 1)[1]
    for attempt in range(3):
        try:
            with urllib.request.urlopen(asset['url'], timeout=90) as source, target.open('wb') as out:
                shutil.copyfileobj(source, out)
            with target.open('rb') as stream:
                actual = hashlib.file_digest(stream, 'sha256').hexdigest()
            if actual != asset['sha256']:
                raise ValueError(f'Download checksum mismatch: {target.name}: {actual}')
            print(f'Verified {target.name}: SHA-256 {actual}', flush=True)
            return target
        except (OSError, ValueError) as error:
            target.unlink(missing_ok=True)
            if attempt == 2:
                raise
            print(f'Download retry: {error}', flush=True)
            time.sleep(3)


def extract_zip(source, destination):
    """Preserve Unix execute bits and SDK relative symlinks."""
    with zipfile.ZipFile(source) as archive:
        for item in archive.infolist():
            relative = Path(item.filename)
            if relative.is_absolute() or '..' in relative.parts:
                raise ValueError('Nonlocal archive entry: ' + item.filename)
            target = destination / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            mode = item.external_attr >> 16
            if item.is_dir():
                target.mkdir(exist_ok=True)
            elif stat.S_ISLNK(mode):
                link = archive.read(item).decode('utf-8')
                if Path(link).is_absolute() or '..' in Path(link).parts:
                    raise ValueError('Nonlocal archive link: ' + link)
                target.symlink_to(link)
            else:
                target.write_bytes(archive.read(item))
                if os.name != 'nt' and stat.S_IMODE(mode):
                    target.chmod(stat.S_IMODE(mode))


def run(*args):
    print('+ ' + subprocess.list2cmdline(list(map(str, args))), flush=True)
    subprocess.run(list(map(str, args)), check=True)


def identify(command, expected):
    result = subprocess.run(list(map(str, command)), stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, timeout=60)
    text = result.stdout.decode('utf-8', errors='replace')
    if result.returncode or expected not in text:
        raise RuntimeError('Unexpected runtime identity: ' + text)
    print('\n'.join(text.splitlines()[:6]), flush=True)
    return '\n'.join(text.splitlines()[:6])


def qualify(runtime, directory):
    system = platform.system().lower()
    if system not in ('linux', 'windows') or platform.machine().lower() not in ('amd64', 'x86_64'):
        raise RuntimeError('This bootstrap supports Linux/Windows x64 hosted runners only')
    key = runtime + '-' + system if runtime != 'netrexx' else 'netrexx'
    entry = LOCK[key]
    assets = [download(asset, directory) for asset in entry['assets']]
    sdk = directory / 'sdk'
    sdk.mkdir()
    for asset in assets:
        if asset.suffix == '.zip':
            extract_zip(asset, sdk)
        else:
            # Official matching Ubuntu 24.04 packages; only in disposable CI VMs.
            run('sudo', 'dpkg', '-i', asset)
    if system == 'linux' and runtime in ('regina', 'oorexx'):
        run('sudo', 'ldconfig')
    executable = sdk / entry['executable'] if 'executable' in entry else None
    identity = ''
    if runtime == 'crexx':
        binary = executable.parent
        identity = identify([executable, '-v'], 'crexx-1.0.0-beta.3')
        vms = ['rxbvm'] if system == 'windows' else ['rxtvm', 'rxbvm']
        run(sys.executable, ROOT / 'tests/check_crexx.py', '--bin', binary, '--vms', *vms)
        run(sys.executable, ROOT / 'tests/check_package.py', '--bin', binary)
    elif runtime in ('regina', 'oorexx'):
        identity = identify([executable, '-v'], entry['version'])
        run(sys.executable, ROOT / 'tests/check_classic.py', '--runtime', executable)
        run(sys.executable, ROOT / 'tests/check_classic_package.py', '--runtime', executable)
    else:
        java_home = Path(os.environ['JAVA_HOME'])
        java = java_home / ('bin/java.exe' if system == 'windows' else 'bin/java')
        identity = identify([java, '-version'], '26.0.1')
        identity += '\n' + identify([java, '-cp', sdk / 'lib/NetRexxF.jar',
                                    'org.netrexx.process.NetRexxC', '-help'], '5.10')
        run(sys.executable, ROOT / 'tests/check_netrexx.py', '--sdk', sdk, '--java-home', java_home)
        run(sys.executable, ROOT / 'tests/check_netrexx_package.py', '--sdk', sdk, '--java-home', java_home)
    summary = os.environ.get('GITHUB_STEP_SUMMARY')
    if summary:
        with Path(summary).open('a', encoding='utf-8') as out:
            out.write(f'## PASS: {runtime} / {system} x64\n\n')
            out.write(f'Revision: `{os.environ.get("GITHUB_SHA", "local")}`\n\n')
            out.write(f'```text\n{identity}\n```\n\n')
            out.write('71 shared fixtures and round trips, full boundary/lifecycle contracts, '
                      'and a fresh source-package consumer passed.\n')
    print(f'PASS: hosted qualification {runtime}/{system}', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime', choices=('crexx', 'regina', 'oorexx', 'netrexx'), required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='csv-qualification-') as temp:
        qualify(args.runtime, Path(temp))
