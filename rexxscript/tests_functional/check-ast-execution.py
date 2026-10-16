#!/usr/bin/env python3
"""Compare a retained interpreter with the named-AST implementation.

Explicit qualification panel; not a nested CTest. Commands and exact output are
retained. Run CONTROL CANDIDATE OUTPUT; use --control-only before rebuilding.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

p = argparse.ArgumentParser()
p.add_argument('control', type=Path)
p.add_argument('candidate', type=Path)
p.add_argument('output', type=Path)
p.add_argument('--control-only', action='store_true')
a = p.parse_args()
a.output.mkdir(parents=True, exist_ok=True)
cases = {
    'precedence': 'x=2+3*4; say x; say -2**2; say 2**3**2',
    'typed': "say 5/2; say 5.0/2; say '2'/3; say 0.1+0.2=0.3; say 0.1d+0.2d=0.3d",
    'overflow': 'say 9223372036854775807+(2-1)',
    'minimum': 'say -9223372036854775808',
    'division': 'say 1/(2-2)',
    'nested-error': 'say 5/(1+1-2)',
    'prefix-error': "say -('a'||'bc')",
    'unselected': 'if 0 then say 1/0 else say 7',
    'nested-if': 'if 1 then if 0 then say 1 else say 2 else say 3',
    'continued-if': 'if 0 then say 1 else if 0 then say 2 else say 3',
    'and': "x=0; say 0 & value('x',1); say x",
    'or': 'say 1 | unknown',
    'order': "x=1; say value('x',2)+value('x',3); say x",
    'args': "say substr('abcd',2,,'.'); say reverse(upper('ab'))",
    'bif-error': 'say substr()',
    'host': "say linein('somefile')",
    'control': 'say 1; leave',
    'controlled': 'x=0; do i=1 to 5; if i=3 then iterate; x=x+i; end; say x',
    'descending': 'x=0; do i=3 to 1 by -1; x=x+i; end; say x',
    'until': 'x=0; do until x=3; x=x+1; end; say x',
    'while': 'x=0; do while x<3; x=x+1; end; say x',
    'leave': 'do 2; do forever; say 7; leave; end; end',
    'return': 'if 1 then return 42 else say 1/0; say unknown',
    'comments': 'say 1 /* outer /* nested */ */ + 2',
    'continuation': 'say 1 + ,\n2',
    'unicode-error': "say 'café'; say 1/(2-2)",
    'parse': 'say 1; x=(2+',
    'empty': '',
}
records = []
for name, source in cases.items():
    path = a.output / (name + '.rexx')
    path.write_text(source, encoding='utf-8')
    results = []
    for label, tool in [('control', a.control), ('candidate', a.candidate)]:
        if a.control_only and label == 'candidate':
            continue
        log = a.output / (name + '-' + label + '.log')
        result = subprocess.run([str(tool.resolve()), str(path.resolve())], capture_output=True, timeout=600)
        data = result.stdout + result.stderr
        log.write_bytes(data)
        results.append((result.returncode, data))
        records.append(dict(case=name, implementation=label, status=result.returncode,
                            output=data.decode('utf-8', errors='replace'),
                            source_sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
    if not a.control_only:
        assert results[0] == results[1], (name, results)
receipt = {'observations': records, 'control_sha256': hashlib.sha256(a.control.read_bytes()).hexdigest()}
if not a.control_only:
    receipt['candidate_sha256'] = hashlib.sha256(a.candidate.read_bytes()).hexdigest()
(a.output / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(f'PASS: {len(cases)} cases; {len(records)} observations')
