#!/usr/bin/env python3
"""One capture surface across CFG engines and ordered PEG."""
import argparse
import json
import subprocess
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
p.add_argument('--keyword-fields', action='store_true')
a = p.parse_args()
root = Path(__file__).resolve().parents[3]
binary = a.build.resolve() / 'bin'
out = a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
records = []

def run(name, command, status=0):
    result = subprocess.run(list(map(str, command)), capture_output=True, timeout=1200)
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
parse: parser start root BACKEND
root: rule = (identifier as item)* (equals number as contents)?
root.error: diagnostic rule root = 'LIST_MESSAGE {found}'
'''
cases = [
    (b'a b = 42', 0, [b'FIELDS|2|1', b'ITEM|a|0|1', b'ITEM|b|2|3', b'CONTENTS|42|6|8']),
    (b'a b', 0, [b'FIELDS|2|0']),
    (b'= 42', 0, [b'FIELDS|0|1', b'CONTENTS|42|2|4']),
    (b'', 0, [b'FIELDS|0|0']),
    (b'a =', 1, [b'LIST_MESSAGE', b'end of input']),
]
fieldargs = []
if a.keyword_fields:
    base = base.replace('as item', 'as rule').replace('as contents', 'as as')
    fieldargs = ['rule', 'as']
for backend in ['lr1', 'll1', 'packrat', 'earley']:
    work = out / backend
    work.mkdir(exist_ok=True)
    spec = base.replace('BACKEND', 'backend ' + backend + (' semantics ordered' if backend == 'packrat' else ''))
    path = work / 'input.levell'
    path.write_text(spec)
    for route in ['rxas', 'source']:
        cell = backend + '-' + route
        generated = work / ('levell_settings.' + ('rxas' if route == 'rxas' else 'crexx'))
        generated.write_bytes(run(cell + '-generate', [binary / 'levell', path, *(['--rxas'] if route == 'rxas' else [])]))
        if route == 'source':
            run(cell + '-compile-language', [binary / 'rxc', '-x', '--no-exe-import', '-i', f'{work};{binary}', '-o', work / 'levell_settings', generated])
        run(cell + '-assemble-language', [binary / 'rxas', work / 'levell_settings'])
        run(cell + '-compile-probe', [binary / 'rxc', '-x', '--no-exe-import', '-i', f'{work};{binary}', '-o', work / 'probe', root / 'lib/rxfnsl/tests_functional/capture-probe.crexx'])
        run(cell + '-assemble-probe', [binary / 'rxas', work / 'probe'])
        run(cell + '-link', [binary / 'rxlink', '-s', '-o', work / 'linked', work / 'probe.rxbin', work / 'levell_settings.rxbin', binary / 'rxfnsl.rxbin', binary / 'library.rxbin'])
        for vm in ['rxtvm', 'rxbvm']:
            for i, (source, status, expected) in enumerate(cases):
                target = work / 'input.txt'
                target.write_bytes(source)
                data = run(cell + '-' + vm + '-' + str(i), [binary / vm, work / 'linked.rxbin', '-a', target, *fieldargs], status)
                assert all(part in data for part in expected), (cell, i, data, expected)
                records.append(dict(backend=backend, route=route, vm=vm, case=i))
                (out / 'progress.json').write_text(json.dumps(dict(records=records), indent=2) + '\n')

bad = out / 'lexical-capture.levell'
bad.write_text(base.replace('BACKEND', '').replace('letters+ then emit', 'letters+ as invalid then emit'))
for stage in range(3):
    tool = a.build.resolve() / f'lib/rxfnsl/tools/bootstrap/L{stage}/levell-linked.rxbin'
    result = run('lexical-capture-L' + str(stage), [binary / 'rxtvm', tool, '-a', bad, '--rxas'], 1)
    assert b'named fields belong to parser rules' in result, result

# Run the actual final tutorial: captures must survive the Pratt/LR region
# boundary and the reader must recover original spelling and source order.
work = out / 'worked-example'
work.mkdir(exist_ok=True)
examples = root / 'lib/rxfnsl/examples'
for route in ['rxas', 'source']:
    key = 'worked-' + route
    generated = work / ('levell_settings.' + ('rxas' if route == 'rxas' else 'crexx'))
    generated.write_bytes(run(key + '-generate', [binary / 'levell', examples / 'settings/06-fields.levell',
                                               *(['--rxas'] if route == 'rxas' else [])]))
    if route == 'source':
        run(key + '-compile-language', [binary / 'rxc', '-x', '--no-exe-import', '-i', f'{work};{binary}',
                                      '-o', work / 'levell_settings', generated])
    run(key + '-assemble-language', [binary / 'rxas', work / 'levell_settings'])
    run(key + '-compile-reader', [binary / 'rxc', '-x', '--no-exe-import', '-i', f'{work};{binary}',
                                '-o', work / 'reader', examples / 'settings-fields.crexx'])
    run(key + '-assemble-reader', [binary / 'rxas', work / 'reader'])
    run(key + '-link', [binary / 'rxlink', '-s', '-o', work / 'linked', work / 'reader.rxbin',
                       work / 'levell_settings.rxbin', binary / 'rxfnsl.rxbin', binary / 'library.rxbin'])
    for vm in ['rxtvm', 'rxbvm']:
        data = run(key + '-' + vm, [binary / vm, work / 'linked.rxbin', '-a', examples / 'settings/expressions.settings'])
        assert data == b'workers -> [2 + 3 * 4]\ngreeting -> "Hello"\nlimit -> [-(2 + 3) * 4]\n', data
(out / 'receipt.json').write_text(json.dumps(dict(records=records, lexical_capture_rejections=3,
                                                worked_example_cases=4), indent=2) + '\n')
print(f'capture_cases={len(records)}; lexical_rejections=3; worked_example_cases=4; failures=0')

