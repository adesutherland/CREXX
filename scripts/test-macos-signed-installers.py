#!/usr/bin/env python3
"""Disposable hosted Mac only: install stapled packages with networking down.

The independent watchdog and finally block restore previously active interfaces.
Unlike a process sandbox, this also blocks Gatekeeper's system daemon network.
"""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--core-pkg', type=Path, required=True)
    parser.add_argument('--plugin-pkg', type=Path, required=True)
    parser.add_argument('--logs', type=Path, required=True)
    args = parser.parse_args()
    if sys.platform != 'darwin' or os.geteuid() != 0 or os.environ.get('GITHUB_ACTIONS') != 'true':
        parser.error('Requires sudo on a disposable GitHub-hosted Mac')
    prefix = Path('/usr/local/crexx')
    if prefix.exists():
        raise RuntimeError('Refusing to overwrite an existing core')
    args.logs.mkdir(parents=True, exist_ok=True)
    records = []

    def run(label, command, expect=0, timeout=300):
        with (args.logs / (label + '.log')).open('w') as log:
            result = subprocess.run(list(map(str, command)), stdout=log, stderr=log, timeout=timeout)
        records.append(dict(label=label, returncode=result.returncode))
        if (expect == 0 and result.returncode != 0) or (expect != 0 and result.returncode == 0):
            raise RuntimeError('Unexpected result: ' + label)

    curl = ['/usr/bin/curl', '--silent', '--show-error', '--fail', '--head',
            '--connect-timeout', '5', '--max-time', '15', 'https://www.apple.com']
    run('online-positive-control', curl)
    for kind, pkg in (('core', args.core_pkg), ('plugin', args.plugin_pkg)):
        run(kind + '-download-quarantine', ['/usr/bin/xattr', '-w', 'com.apple.quarantine',
                                          f'0081;{int(time.time()):x};cREXX QA;', pkg])
    inventory = subprocess.check_output(['/sbin/ifconfig', '-a'], text=True)
    interfaces = [name for name, flags in re.findall(r'^(\w+): flags=[0-9a-fA-F]+<([^>]+)>', inventory, re.M)
                  if name != 'lo0' and 'UP' in flags.split(',')]
    if not interfaces:
        raise RuntimeError('No active network interfaces to isolate')
    (args.logs / 'interfaces.json').write_text(json.dumps(interfaces) + '\n')
    # A killed parent must not strand the Actions agent offline. This watchdog
    # only restores interface state; it cannot turn an interrupted test green.
    watchdog_code = ('import json,subprocess,sys,time; time.sleep(600); '
                     '[subprocess.run(["/sbin/ifconfig",n,"up"]) for n in json.loads(sys.argv[1])]')
    watchdog_log = (args.logs / 'network-watchdog.log').open('w')
    watchdog = subprocess.Popen([sys.executable, '-c', watchdog_code, json.dumps(interfaces)],
                                stdout=watchdog_log, stderr=watchdog_log, start_new_session=True)
    outcome = 'failed'
    try:
        for name in interfaces:
            run('network-down-' + name, ['/sbin/ifconfig', name, 'down'])
        run('offline-negative-control', curl, expect=1)
        for kind, pkg in (('core', args.core_pkg), ('plugin', args.plugin_pkg)):
            run(kind + '-signature', ['/usr/sbin/pkgutil', '--check-signature', pkg])
            run(kind + '-ticket', ['/usr/bin/xcrun', 'stapler', 'validate', pkg])
            run(kind + '-gatekeeper', ['/usr/sbin/spctl', '--assess', '--type', 'install', '--verbose=4', pkg])
            run(kind + '-install', ['/usr/sbin/installer', '-pkg', pkg, '-target', '/'])
        run('installed-consumer', [sys.executable, Path(__file__).with_name('test-llama-installed.py'),
                                  '--prefix', prefix, '--logs', args.logs / 'consumer'])
        run('still-offline-control', curl, expect=1)
        outcome = 'passed'
    finally:
        restored = True
        for name in interfaces:
            restored &= subprocess.run(['/sbin/ifconfig', name, 'up'], capture_output=True).returncode == 0
        if restored:
            watchdog.terminate()
            watchdog.wait(timeout=10)
        watchdog_log.close()
        (args.logs / 'summary.json').write_text(json.dumps(dict(outcome=outcome,
            interfaces_restored=restored, checks=records), indent=2) + '\n')
        if not restored:
            raise RuntimeError('Interface restoration incomplete; watchdog retained')
    print('PASS: signed/stapled core and plugin install and consumer with networking disabled')


if __name__ == '__main__':
    main()
