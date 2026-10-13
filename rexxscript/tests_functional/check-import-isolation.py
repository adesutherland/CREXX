#!/usr/bin/env python3
"""A stale diagnostic prototype in the output directory must not become an import."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
build = a.build.resolve()
binary = build / 'bin'
out = a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
modern = build / 'rexxscript/modern'
shadow = modern / '__levell_import_regression.rxbin'
assert not shadow.exists(), 'Do not overwrite an existing fixture'
records = []


def run(name, command):
    command = list(map(str, command))
    r = subprocess.run(command, capture_output=True, timeout=1200)
    data = r.stdout + r.stderr
    (out / (name + '.log')).write_bytes(data)
    records.append(dict(name=name, argv=command, status=r.returncode,
                        sha256=hashlib.sha256(data).hexdigest()))
    assert r.returncode == 0 and b'Warning in ' not in data, (name, r.returncode, data[-2000:])
    return r.stdout


source = out / 'stale-prototype.crexx'
source.write_text('''options levelb
namespace llsource expose diagnostic
diagnostic: procedure = .string
  arg source = .binary, offset = .int, code = .string, message = .string
  return 'STALE_PROTOTYPE ' || code || message || offset || <blen>(source)
''')
# Supply only Level B imports when creating the intentionally obsolete interface.
imports = out / 'imports'
imports.mkdir(exist_ok=True)
link = imports / 'library.rxbin'
if link.is_symlink():
    link.unlink()
link.symlink_to(binary / 'library.rxbin')
run('compile-shadow', [binary / 'rxc', '-x', '--no-exe-import', '-i', imports,
                       '-o', out / 'shadow', source])
run('assemble-shadow', [binary / 'rxas', out / 'shadow'])
try:
    shadow.write_bytes((out / 'shadow.rxbin').read_bytes())
    # Force a real consumer compile while the stale interface remains present.
    (modern / 'RexxScriptProgram.rxbin').unlink()
    run('build-with-shadow', ['cmake', '--build', build, '--target',
        'rexxscript1_cli', 'testrexxscript1', '--parallel', '4'])
    for vm in ['rxtvm', 'rxbvm']:
        data = run(vm, [binary / vm, modern / 'testRexxScript1-linked.rxbin'])
        assert b'rexxscript1_failures=0' in data
finally:
    shadow.unlink(missing_ok=True)
(out / 'receipt.json').write_text(json.dumps(dict(records=records,
    scope='Obsolete four-argument llsource.diagnostic export beside linked drivers; native packing and both interpreter consumers use current private imports.'), indent=2) + '\n')
print('stale_import_build=1; interpreter_cells=2; failures=0')
