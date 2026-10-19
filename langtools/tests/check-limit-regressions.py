#!/usr/bin/env python3
"""Exact resource diagnostics, raised-budget artifacts and loader independence."""
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


def run(name, command, status=0, contains=None):
    command = list(map(str, command))
    r = subprocess.run(command, capture_output=True, timeout=1200)
    data = r.stdout + r.stderr
    (out / (name + '.log')).write_bytes(data)
    records.append(dict(name=name, argv=command, status=r.returncode,
                        sha256=hashlib.sha256(data).hexdigest()))
    assert r.returncode == status and b'Warning in ' not in data, (name, r.returncode, data[-2000:])
    if contains is not None:
        assert contains in data, (name, data[-2000:])
    return r.stdout


base = (root / 'langtools/tests/grammars/budget.levell').read_text()
spec = out / 'input.levell'
spec.write_text(base)
image = run('baseline', [binary / 'rxlang', spec, '--rxas'])
settings = run('discover', [binary / 'rxlang', '--limits'])
for key in ['grammar.clauses', 'spec.tokens', 'cfg.productions', 'nfa.states',
            'dfa.states', 'lr.items', 'lr.rhs', 'recovery.errors',
            'recovery.declarations', 'stage.rules', 'grammar.repeat', 'lexer.clauses']:
    assert (key + '=').encode() in settings
    raised = run('raised-' + key, [binary / 'rxlang', spec, '--rxas', '--limit', key + '=100000'])
    assert raised == image, (key, 'budget changed successful artifact')

for key in ['grammar.clauses', 'spec.tokens', 'cfg.productions', 'nfa.states', 'dfa.states', 'lr.items', 'lexer.clauses']:
    failed = run('exhausted-' + key, [binary / 'rxlang', spec, '--rxas', '--limit', key + '=1'],
                 1, ('LL_RESOURCE_LIMIT: ' + key + ' budget 1, required at least ').encode())
    assert b'LL_LR_ADMISSION' not in failed and b'LL_BACKEND_ADMISSION' not in failed
    assert b'.globals=' not in failed

for key, count in [('lr.states', '999999'), ('lr.items', '134217727'), ('lr.rhs', '4194304'),
                   ('source.bytes', '2147483648')]:
    run('ceiling-' + key, [binary / 'rxlang', spec, '--limit', key + '=' + count],
        2, b'representation ceiling')

repeated = out / 'repeated.levell'
repeated.write_text(base.replace('assignment: rule =', 'assignment: rule = number{257} |'))
run('repeat-default', [binary / 'rxlang', repeated, '--rxas'], 1,
    b'LL_RESOURCE_LIMIT: grammar.repeat budget 256, required at least 257')
run('repeat-raised', [binary / 'rxlang', repeated, '--rxas', '--limit', 'grammar.repeat=300',
                     '--limit', 'grammar.clauses=2000', '--limit', 'lr.states=12000'])

# More than the old fixed graph limit, with an intentionally tiny active language.
# Loading must validate the graph/image without reapplying build defaults.
large = out / 'large.levell'
large.write_text(base + ''.join(f'extra.{i}: rule = number\n' for i in range(520)))
run('large-default', [binary / 'rxlang', large, '--rxas'], 1, b'grammar.clauses budget 512')
generated = run('large-raised', [binary / 'rxlang', large, '--rxas', '--limit', 'grammar.clauses=2048'])
(out / 'levell_settings.rxas').write_bytes(generated)
run('assemble-language', [binary / 'rxas', out / 'levell_settings'])
run('compile-probe', [binary / 'rxc', '-x', '--no-exe-import', '-i', f'{out};{binary}',
                     '-o', out / 'probe', root / 'langtools/tests/limit-regression-probe.crexx'])
run('assemble-probe', [binary / 'rxas', out / 'probe'])
run('link', [binary / 'rxlink', '-s', '-o', out / 'linked', out / 'probe.rxbin',
             out / 'levell_settings.rxbin', binary / 'rxfnsl.rxbin', binary / 'library.rxbin'])
for vm in ['rxtvm', 'rxbvm']:
    run(vm, [binary / vm, out / 'linked.rxbin'], 0, b'loader_and_boundary_checks=12; failures=0')

(out / 'receipt.json').write_text(json.dumps(dict(records=records,
    scope='Construction resource categories, successful-artifact invariance, packed loader independence and boundary checks.'), indent=2) + '\n')
print(f'limit_regression_commands={len(records)}; loader_checks=24; failures=0')
