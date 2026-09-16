#!/usr/bin/env python3
"""Disposable Windows runner: native core/plugin setup with retained binaries."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time
import winreg
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def run(*args, **kwargs):
    print('RUN:', *map(str, args), flush=True)
    subprocess.run(list(map(str, args)), check=True, timeout=1800, **kwargs)


def setup(executable, destination=None):
    # NSIS requires /D= to be the unquoted remainder of its command line. The
    # existing core QA uses Start-Process -Wait so any bootstrap child also
    # finishes. This is runner orchestration, never shipped switcher logic.
    arguments = '/S' + (' /D=' + str(destination) if destination else '')
    env = dict(os.environ, LLAMA_QA_SETUP=str(executable), LLAMA_QA_SETUP_ARGS=arguments)
    try:
        run('powershell.exe', '-NoProfile', '-NonInteractive', '-Command',
            '$p = Start-Process -FilePath $env:LLAMA_QA_SETUP -ArgumentList $env:LLAMA_QA_SETUP_ARGS -Wait -PassThru; exit $p.ExitCode', env=env)
    finally:
        for log in Path(tempfile.gettempdir()).glob('crexx-llama-*-installer.log'):
            print(log.read_text(errors='replace'), flush=True)
            shutil.copy2(log, Path(os.environ['RUNNER_TEMP']) / 'proof' / log.name)


def wait_removed(path):
    deadline = time.monotonic() + 180
    while path.exists() and time.monotonic() < deadline:
        time.sleep(.2)
    if path.exists():
        print('Remaining files:', *(str(p.relative_to(path)) for p in path.rglob('*')), sep='\n', flush=True)
        raise RuntimeError('Uninstaller did not remove ' + str(path))


def machine_env():
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,
                        r'SYSTEM\CurrentControlSet\Control\Session Manager\Environment') as key:
        values = {}
        for name in ('Path', 'CREXX_HOME', 'REXX_HOME'):
            try:
                values[name] = winreg.QueryValueEx(key, name)
            except FileNotFoundError:
                values[name] = None
        return values


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--work', type=Path, required=True)
    p.add_argument('--makensis', required=True)
    args = p.parse_args()
    if os.environ.get('GITHUB_ACTIONS') != 'true':
        raise RuntimeError('Disposable GitHub runner only')
    work = args.work.resolve()
    core = work / 'core/CREXX-windows-x64'
    installed = work / 'cREXX installed with spaces'
    if installed.exists():
        raise RuntimeError('QA prefix already exists')
    before = machine_env()
    nsis = args.makensis
    core_setup = work / 'core-qa-setup.exe'
    run(nsis, '/V2', '/DCREXX_PAYLOAD_DIR=' + str(core), '/DCREXX_OUTFILE=' + str(core_setup),
        ROOT / 'packaging/windows/crexx.nsi')
    setup(core_setup, installed)
    if not (installed / 'core-package.json').is_file():
        raise RuntimeError('Native core installation failed')
    core_environment = machine_env()
    backends = ['vulkan', 'cuda'] if os.environ.get('INCLUDE_CUDA') == '1' else ['vulkan']
    installers = {}
    for backend in backends:
        archives = list((work / 'retained' / backend).glob('*.zip'))
        if len(archives) != 1:
            raise RuntimeError('Expected one plugin archive: ' + backend)
        unpacked = work / ('plugin-' + backend)
        with zipfile.ZipFile(archives[0]) as archive:
            archive.extractall(unpacked)
        plugin = unpacked / 'CREXX-windows-x64'
        output = work / ('llama-' + backend + '-qa-setup.exe')
        run(sys.executable, ROOT / 'scripts/package-llama-installer.py', '--core', core,
            '--plugin', plugin, '--manager', work / 'manager', '--unsigned-qa',
            '--makensis', nsis, '--output', output)
        installers[backend] = output
        # No /D: exercise discovery of the core's registered installation.
        setup(output)
        manager = installed / 'bin/crexx-llama.exe'
        status = subprocess.check_output([str(manager), 'status'], text=True)
        print(status)
        if 'Active backend: vulkan' not in status:
            raise RuntimeError('Second installer changed the active backend')
    model = installed / 'user-model.gguf'
    model.write_bytes(b'user-owned data')
    sdk_free = dict(os.environ, PATH=os.pathsep.join([str(Path(os.environ['SystemRoot']) / 'System32'), os.environ['SystemRoot']]))
    for backend in backends:
        run(manager, 'use', backend, env=sdk_free)
        run(sys.executable, ROOT / 'scripts/test-llama-installed.py', '--prefix', installed,
            '--logs', work / ('proof/consumer-' + backend))
    run(manager, 'use', 'vulkan', env=sdk_free)
    setup(installers['vulkan'], installed)
    # Check the full projection after commit/cleanup, including manifests larger
    # than the shared tool's. A successful installer exit is not sufficient.
    manifest = json.loads((installed / 'llama-package.json').read_text())
    for name, expected in manifest['files'].items():
        with (installed / name).open('rb') as stream:
            if hashlib.file_digest(stream, 'sha256').hexdigest() != expected:
                raise RuntimeError('Reinstall changed declared file: ' + name)
    for backend in reversed(backends):
        store = installed / '.llama-backends' / backend
        setup(store / 'uninstall.exe')
        wait_removed(store)
    if manager.exists() or (installed / 'bin/rxllama.rxplugin').exists():
        raise RuntimeError('Plugin files survived last removal')
    if model.read_bytes() != b'user-owned data' or machine_env() != core_environment:
        raise RuntimeError('Plugin lifecycle changed models or core environment')
    consumer = work / 'core consumer'
    consumer.mkdir()
    shutil.copy2(installed / 'examples/hello.crexx', consumer / 'hello.crexx')
    output = subprocess.check_output([str(installed / 'bin/crexx.exe'), str(consumer / 'hello.crexx')], text=True)
    if 'hello CREXX world!' not in output:
        raise RuntimeError('Core smoke failed after plugin removal')
    (work / 'proof/core-after-removal.log').write_text(output)
    # Reinstall one variant to exercise core uninstall registration cleanup.
    setup(installers['vulkan'])
    registration = (installed / '.llama-installer/registration-id').read_text()
    setup(installed / 'Uninstall.exe')
    wait_removed(installed)
    if machine_env() != before:
        raise RuntimeError('Core uninstaller did not restore environment')
    for backend in backends:
        key = r'Software\Microsoft\Windows\CurrentVersion\Uninstall\CREXX.llama.' + registration + '.' + backend
        try:
            handle = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, key, 0, winreg.KEY_READ | winreg.KEY_WOW64_64KEY)
        except FileNotFoundError:
            continue
        winreg.CloseKey(handle)
        raise RuntimeError('Stale backend uninstall registration: ' + key)
    print('PASS: native Rexx switcher and Windows installer lifecycle; signing is a separate gate.')


if __name__ == '__main__':
    main()
