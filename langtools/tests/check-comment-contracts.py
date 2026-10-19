#!/usr/bin/env python3
"""Native literal/legacy/frozen-policy assertions on both consumer optimizers."""
import argparse
import hashlib
import json
import shutil
import subprocess
from pathlib import Path
from _scratch import scratch

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[2]
binary, out = a.build.resolve() / 'bin', scratch()
(out / 'imports').mkdir(parents=True, exist_ok=True)
records = []
for name in ['library', 'rxfnsl']:
    shutil.copy2(binary / (name + '.rxbin'), out / 'imports' / (name + '.rxbin'))
for mode in ['opt', 'noopt']:
    probe, image = out / ('probe-' + mode), out / ('linked-' + mode)
    commands = [
        ('compile', [binary / 'rxc', '-x', '--no-exe-import'] + (['-n'] if mode == 'noopt' else []) +
         ['-i', out / 'imports', '-o', probe, root / 'langtools/tests/comment-contract-probe.crexx']),
        ('assemble', [binary / 'rxas', probe]),
        ('link', [binary / 'rxlink', '-s', '-o', image, probe.with_suffix('.rxbin'), binary / 'rxfnsl.rxbin', binary / 'library.rxbin']),
    ]
    commands += [(vm, [binary / vm, image.with_suffix('.rxbin')]) for vm in ['rxtvm', 'rxbvm']]
    for name, argv in commands:
        argv = list(map(str, argv))
        result = subprocess.run(argv, capture_output=True, timeout=1200)
        data = result.stdout + result.stderr
        (out / (mode + '-' + name + '.log')).write_bytes(data)
        assert result.returncode == 0 and b'Warning in ' not in data, (name, data[-2000:])
        if name in ['rxtvm', 'rxbvm']:
            assert b'comment_contract_checks=19; failures=0' in data
        records.append(dict(mode=mode, name=name, argv=argv, sha256=hashlib.sha256(data).hexdigest()))
(out / 'receipt.json').write_text(json.dumps(dict(records=records, assertions=19, cells=4), indent=2) + '\n')
print('contract_assertions=19; cells=4; failures=0')
