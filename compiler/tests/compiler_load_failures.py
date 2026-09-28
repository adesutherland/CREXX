#!/usr/bin/env python3
"""Real isolated RXC: required exit library failures cannot erase statements."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import time

started = time.monotonic()

p = argparse.ArgumentParser()
for arg in ('rxc', 'rxas', 'vm', 'library', 'exits', 'work'):
    p.add_argument('--' + arg, required=True)
a = p.parse_args()
w = Path(a.work).resolve()
w.mkdir(parents=True, exist_ok=True)
exe = w / Path(a.rxc).name
shutil.copy2(a.rxc, exe)
source = w / 'main.crexx'
source.write_text('options levelb\nsay "before"\nparse "hello" word\nsay word\nsay "after"\n')
log = []
def compile(name, expected, diagnostic=None, optional=None):
    env = dict(os.environ)
    env.pop('RXCP_EXIT_MODULE', None)
    if optional:
        env['RXCP_EXIT_MODULE'] = optional
    command = [str(exe), '-i', str(w), '-o', str(w / name), str(source)]
    r = subprocess.run(command, cwd=w, env=env, capture_output=True, text=True)
    log.append('$ ' + ' '.join(command) + '\n' + r.stdout + r.stderr + f'status={r.returncode}\n')
    (w/'commands.log').write_text('\n'.join(log))
    assert (r.returncode == 0) == expected, log[-1]
    if diagnostic:
        assert diagnostic in r.stderr, log[-1]
    if not expected:
        assert not (w / (name + '.rxas')).exists(), 'failed compilation emitted output'
    return r
for member in ('library.rxbin', 'rxcexits.rxbin', 'broken.rxbin', 'broken.rxas',
               'missing_library.rxas', 'invalid_library.rxas', 'missing_exits.rxas'):
    (w/member).unlink(missing_ok=True)
compile('missing_library', False, 'EXIT_MODULE_LOAD_ERROR')
(w/'library.rxbin').write_bytes(b'not an RXBIN file')
compile('invalid_library', False, 'EXIT_MODULE_LOAD_ERROR')
shutil.copy2(a.library, w/'library.rxbin')
compile('missing_exits', False, 'Failed to load certified exit')
shutil.copy2(a.exits, w/'rxcexits.rxbin')
compile('working', True)
compile('optional_missing', True, 'continuing with certified exits only', 'absent_optional')
for name in ('working', 'optional_missing'):
    subprocess.run([a.rxas, '-o', str(w/name), str(w/(name+'.rxas'))], cwd=w, check=True)
    r = subprocess.run([a.vm, str(w/(name+'.rxbin'))], cwd=w, capture_output=True, text=True)
    assert r.returncode == 0 and r.stdout.splitlines() == ['before', 'hello', 'after'], (r.stdout, r.stderr)
source.write_text('options levelb\nimport broken\nsay unknown()\n')
(w/'broken.rxbin').write_bytes(b'bad import bytes')
r = subprocess.run([str(exe), '-x', '-i', str(w), '-o', str(w/'broken'), str(source)], cwd=w, capture_output=True, text=True)
assert r.returncode != 0 and 'RXBIN_IMPORT_READ_ERROR' in r.stderr, (r.returncode,r.stdout,r.stderr)
assert not (w/'broken.rxas').exists()
log.append(f'broken-import status={r.returncode}\n'+r.stderr)
(w/'commands.log').write_text('\n'.join(log))
print(f'PASS mandatory/optional compiler exit loads, real PARSE execution and malformed import status ({time.monotonic() - started:.2f}s)')
