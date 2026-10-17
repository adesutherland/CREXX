#!/usr/bin/env python3
"""Generated tree annotations act on owned trees and refuse invalid projections."""
import argparse
import json
import subprocess
from pathlib import Path
from _scratch import scratch

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[2]
binary = a.build.resolve() / 'bin'
out = scratch()
out.mkdir(parents=True, exist_ok=True)
records = []

def run(name, command, status=0):
    command = list(map(str, command))
    result = subprocess.run(command, capture_output=True, timeout=1200)
    data = result.stdout + result.stderr
    (out / (name + '.log')).write_bytes(data)
    assert result.returncode == status and b'Warning in ' not in data, (name, result.returncode, data[-3000:])
    return data

base = '''options levell
namespace levell_settings
identifier: token
number: token
equals: token
scan: lexer
letters: charset = 'a' to 'z'
digits: charset = '0' to '9'
name.lex: rule = letters+ then emit identifier
number.lex: rule = digits+ then emit number
equal.lex: rule = '=' then emit equals
space.lex: rule = ' '+ then skip
parse: parser start assignment BACKEND
assignment: rule = name.value equals value
name.value: rule = identifier
value: rule = number
assignment.tree: tree rule assignment kind 10 arity 2
name.tree: tree rule name.value kind 11 arity 0
value.tree: tree rule value kind 12 arity 0
'''
for backend in ['lr1', 'll1', 'packrat', 'earley']:
    for bad_arity in [False, True]:
        key = backend + ('-bad-arity' if bad_arity else '')
        work = out / key
        work.mkdir(exist_ok=True)
        spec = base.replace('BACKEND', 'backend ' + backend + (' semantics ordered' if backend == 'packrat' else ''))
        if bad_arity:
            spec = spec.replace('kind 10 arity 2', 'kind 10 arity 1')
        path = work / 'input.levell'
        path.write_text(spec)
        for route in ['rxas', 'source']:
            cell = key + '-' + route
            generated = work / ('levell_settings.' + ('rxas' if route == 'rxas' else 'crexx'))
            generated.write_bytes(run(cell + '-generate', [binary / 'rxlang', path, *(['--rxas'] if route == 'rxas' else [])]))
            if route == 'source':
                run(cell + '-compile-language', [binary / 'rxc', '-x', '--no-exe-import', '-i', f'{work};{binary}', '-o', work / 'levell_settings', generated])
            run(cell + '-assemble-language', [binary / 'rxas', work / 'levell_settings'])
            run(cell + '-compile-probe', [binary / 'rxc', '-x', '--no-exe-import', '-i', f'{work};{binary}', '-o', work / 'probe', root / 'langtools/tests/projection-probe.crexx'])
            run(cell + '-assemble-probe', [binary / 'rxas', work / 'probe'])
            run(cell + '-link', [binary / 'rxlink', '-s', '-o', work / 'linked', work / 'probe.rxbin', work / 'levell_settings.rxbin', binary / 'rxfnsl.rxbin', binary / 'library.rxbin'])
            for vm in ['rxtvm', 'rxbvm']:
                for source in [b'port = 42', b'port =']:
                    target = work / 'input.txt'
                    target.write_bytes(source)
                    invalid = bad_arity or source == b'port ='
                    data = run(cell + '-' + vm + ('-invalid' if source == b'port =' else '-valid'), [binary / vm, work / 'linked.rxbin', '-a', target], 3 if invalid else 0)
                    if not invalid:
                        assert b'NODE|name.value|11|0|4' in data and b'NODE|value|12|7|9' in data and b'NODE|assignment|10|0|9' in data, data
                    elif source == b'port =':
                        assert b'PARSE|1|PROJECT|3' in data and b'NODE|' not in data, data
                    else:
                        assert b'projection arity for assignment: expected 1, found 2' in data, data
                    records.append(dict(backend=backend, route=route, vm=vm, bad_arity=bad_arity, input=source.decode()))

(out / 'receipt.json').write_text(json.dumps(dict(records=records), indent=2) + '\n')
print(f'tree_cases={len(records)}; failures=0')

