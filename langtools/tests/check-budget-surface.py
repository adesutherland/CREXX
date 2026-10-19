#!/usr/bin/env python3
"""Host-budget overrides, default invariance and explicit resource outcomes."""
import argparse
import hashlib
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
commands = []

def run(name, command, status=0):
    command = list(map(str, command))
    result = subprocess.run(command, capture_output=True, timeout=1200)
    data = result.stdout + result.stderr
    (out / (name + '.log')).write_bytes(data)
    commands.append(dict(name=name, argv=command, status=result.returncode,
                         sha256=hashlib.sha256(data).hexdigest()))
    assert result.returncode == status and b'Warning in ' not in data, (name, result.returncode, data[-3000:])
    return data

base = (root / 'langtools/tests/grammars/budget.levell').read_text()
composite = (root / 'langtools/examples/settings/05-composite.levell').read_text()
lexical = composite[:composite.index('parse: parser')]
operators = composite[composite.index('arithmetic: operators'):composite.index('math.region:')]
for backend in ['lr1', 'll1', 'packrat', 'earley', 'pratt', 'regional']:
    work = out / backend
    work.mkdir(exist_ok=True)
    spec = base.replace('parser start assignment', 'parser start assignment backend ' + backend
                        + (' semantics ordered' if backend == 'packrat' else ''))
    source = b'port = 42'
    if backend == 'pratt':
        spec = lexical + 'parse: parser start arithmetic backend pratt\n' + operators
        source = b'2+3*4'
    elif backend == 'regional':
        spec = composite
        source = b'workers = [2+3*4]'
    path = work / 'input.levell'
    path.write_text(spec)
    generated = run(backend + '-generate', [binary / 'rxlang', path, '--rxas'])
    (work / 'levell_settings.rxas').write_bytes(generated)
    run(backend + '-assemble-language', [binary / 'rxas', work / 'levell_settings'])
    run(backend + '-compile-probe', [binary / 'rxc', '-x', '--no-exe-import', '-i', f'{work};{binary}',
        '-o', work / 'probe', root / 'langtools/tests/budget-probe.crexx'])
    run(backend + '-assemble-probe', [binary / 'rxas', work / 'probe'])
    run(backend + '-link', [binary / 'rxlink', '-s', '-o', work / 'linked', work / 'probe.rxbin',
        work / 'levell_settings.rxbin', binary / 'rxfnsl.rxbin', binary / 'library.rxbin'])
    target = work / 'input.txt'
    target.write_bytes(source)
    cases = [('default', [], 0, b'PARSE|0'),
             ('exact-source-boundary', ['source.bytes', str(len(source))], 0, b'PARSE|0'),
             ('larger-work-budget', ['parse.work', '2000000'], 0, b'PARSE|0'),
             ('source', ['source.bytes','1'], 2, f'source.bytes budget 1, required at least {len(source)}'.encode()),
             ('scanner', ['scan.work','1'], 2, b'scan.work budget 1, required at least 2'),
             ('parser', ['parse.work','1'], 2, b'parse.work budget 1, required at least '),
             ('unknown', ['wrong.name','1'], 3, b'unknown resource budget'),
             ('nonpositive', ['parse.work','0'], 3, b'positive count')]
    if backend not in ['pratt', 'regional']:
        cases.append(('projection', ['tree.entries','1','project'], 2,
                      b'PROJECT|2|LL_RESOURCE_LIMIT: tree.entries budget 1, required at least 2'))
    if backend == 'earley':
        cases.append(('chart', ['earley.cells','1'], 2, b'earley.cells budget 1, more required'))
    if backend == 'regional':
        cases.extend([
            ('recovery-default', ['parse.work', '2000000', 'recover'], 0, b'RECOVER|0'),
            ('recovery-source', ['source.bytes', '1', 'recover'], 2, b'source.bytes budget 1'),
            ('recovery-scan', ['scan.work', '1', 'recover'], 2, b'scan.work budget 1'),
            ('recovery-parse', ['parse.work', '1', 'recover'], 2, b'parse.work budget 1'),
        ])
    for vm in ['rxtvm', 'rxbvm']:
        for name, extra, status, expected in cases:
            data = run(backend + '-' + vm + '-' + name, [binary / vm, work / 'linked.rxbin', '-a', target, *extra], status)
            assert expected in data, (backend, name, data)
            records.append(dict(backend=backend, vm=vm, case=name))
    if backend == 'lr1':
        overridden = run('build-budget-high', [binary / 'rxlang', path, '--rxas', '--limit', 'lr.states=100000'])
        assert generated == overridden, 'A host budget changed a successful artifact'
        exhausted = run('build-budget-low', [binary / 'rxlang', path, '--limit', 'lr.states=1', '--rxas'], 1)
        assert b'lr.states budget 1, required at least 2' in exhausted and b'.globals=' not in exhausted, exhausted
        assert b'LL_LR_ADMISSION:' not in exhausted, 'Resource exhaustion was labelled an admission refusal'
        for name, setting in [('unknown','wrong.name=1'), ('zero','lr.states=0'), ('bad','lr.states=x'),
                              ('overflow','lr.states=9999999999999999999999999999999999999999')]:
            run('build-invalid-' + name, [binary / 'rxlang', path, '--limit', setting], 2)

conflict = out / 'conflict.levell'
conflict.write_text(base[:base.index('parse: parser')] + '''parse: parser start choice
choice: rule = first | second
first: rule = number
second: rule = number
''')
refused = run('large-budget-does-not-change-admission',
              [binary / 'rxlang', conflict, '--rxas', '--limit', 'lr.states=100000'], 1)
assert b'LL_LR_CONFLICT' in refused and b'LL_RESOURCE_LIMIT' not in refused, refused

(out / 'receipt.json').write_text(json.dumps(dict(records=records, commands=commands), indent=2) + '\n')
print(f'budget_runtime_cases={len(records)}; build_controls=7; failures=0')

