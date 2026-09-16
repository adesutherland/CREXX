#!/usr/bin/env python3
"""Small installed-provider smoke using the existing public fixture control."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--prefix', type=Path, required=True)
    p.add_argument('--logs', type=Path, required=True)
    args = p.parse_args()
    root = Path(__file__).resolve().parents[1]
    args.logs.mkdir(parents=True, exist_ok=True)
    fixture_info = json.loads((root / 'tests/native-inference/fixtures/manifest.json').read_text())
    fixture = root / 'tests/native-inference/fixtures' / fixture_info['filename']
    with fixture.open('rb') as stream:
        assert hashlib.file_digest(stream, 'sha256').hexdigest() == fixture_info['sha256']
    extension = '.exe' if os.name == 'nt' else ''
    binary = args.prefix.resolve() / 'bin'
    env = dict(os.environ)
    for key in ('CREXX_HOME', 'CREXX_PROVIDER_PATH', 'GGML_BACKEND_PATH',
                'LD_LIBRARY_PATH', 'DYLD_LIBRARY_PATH', 'DYLD_FALLBACK_LIBRARY_PATH'):
        env.pop(key, None)
    if os.name == 'nt':
        system = Path(os.environ['SystemRoot'])
        env['PATH'] = os.pathsep.join(map(str, (system / 'System32', system)))
    with tempfile.TemporaryDirectory(prefix='llama installed consumer ') as temporary:
        work = Path(temporary)
        source = work / 'consumer.crexx'
        shutil.copy2(root / 'tests/native-inference/release_provider_smoke.crexx', source)
        program = work / 'consumer'
        linked = work / 'linked'
        commands = [
            ('compile', [binary / ('rxc' + extension), '--no-exe-import', '-i', binary, '-o', program, source]),
            ('assemble', [binary / ('rxas' + extension), '-o', program, program]),
            ('link', [binary / ('rxlink' + extension), '-o', linked, program,
                      binary / 'library', binary / 'classlib', binary / 'rxfnsg']),
            ('run', [binary / ('rxvm' + extension), linked, '-a', fixture, fixture_info['sha256']])]
        for name, command in commands:
            with (args.logs / (name + '.log')).open('w') as log:
                result = subprocess.run(list(map(str, command)), cwd=work, env=env,
                                        stdout=log, stderr=subprocess.STDOUT, timeout=1800)
            output = (args.logs / (name + '.log')).read_text(errors='replace')
            if result.returncode or 'FAIL:' in output or 'PANIC:' in output:
                raise RuntimeError(name + ': ' + output[-2000:])
            if name == 'run' and 'PASS: public llama release provider' not in output:
                raise RuntimeError('Provider success marker missing: ' + output[-2000:])
            print('PASS: installed ' + name)


if __name__ == '__main__':
    main()
