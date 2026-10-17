#!/usr/bin/env python3
"""Compare recursive and repetition-first settings lists on every CFG/PEG engine.

Explicit panel: supply BUILD OUTPUT. Diagnostic outputs are temporary and deleted on exit.
The expected grammar is the original recursive definition, kept here so changes
in accepted input, assignment order or source spans cannot pass unnoticed.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
from _scratch import scratch
import subprocess

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[2]
binary = a.build.resolve() / 'bin'
out = scratch()
out.mkdir(parents=True, exist_ok=True)
commands = []
observations = []


def tool(name):
    return binary / (name + ('.exe' if os.name == 'nt' else ''))


def run(label, argv, status=0):
    result = subprocess.run(list(map(str, argv)), capture_output=True, timeout=1200)
    data = result.stdout + result.stderr
    (out / (label + '.log')).write_bytes(data)
    commands.append(dict(label=label, argv=list(map(str, argv)), status=result.returncode,
                         sha256=hashlib.sha256(data).hexdigest()))
    assert result.returncode == status, (label, data[-3000:])
    assert b'Warning in ' not in data, (label, data[-3000:])
    return result.stdout


cases = {
    'empty': ('', 0), 'one': ('port=1', 0),
    'many': ('a=1;b=2;c=3', 0), 'blank': ('\n\n\n', 0),
    'repeated': ('\n;a=1;;\n\nb=2;\n', 0), 'trailing': ('a=1;', 0),
    'no-final': ('a=1\nb=2', 0), 'unicode-crlf': ('a="café 😀"\r\nb=2\r\n', 0),
    'missing-separator': ('a=1 b=2', 1), 'missing-value': ('a=', 1),
    'bad-item': ('a=1; b=many; c=3', 1), 'bad-key': ('1=2', 1),
}
base = (root / 'langtools/examples/settings/01-basic.crexx').read_text(encoding='utf-8')
new_rule = 'settings: rule = separator* (assignment (separator assignment?)*)?'
old_rule = "settings: rule = separator settings | assignment tail | ''\n  tail: rule = separator settings | ''"
assert new_rule in base

for backend in ['ll1', 'lr1', 'packrat', 'earley']:
    results = {}
    for shape in ['recursive', 'repeated']:
        work = out / (backend + '-' + shape)
        work.mkdir(exist_ok=True)
        text = base.replace('parser start settings', 'parser start settings backend ' + backend
                            + (' semantics ordered' if backend == 'packrat' else ''))
        if shape == 'recursive':
            text = text.replace(new_rule, old_rule)
        source = work / 'settings.crexx'
        source.write_text(text, encoding='utf-8')
        label = backend + '-' + shape
        generated = run(label + '-generate', [tool('rxlang'), source, '--rxas'])
        (work / 'levell_settings.rxas').write_bytes(generated)
        run(label + '-assemble', [tool('rxas'), work / 'levell_settings'])
        for mode in ['opt', 'noopt']:
            flags = ['-n'] if mode == 'noopt' else []
            run(label + '-' + mode + '-compile', [tool('rxc'), *flags, '-x', '--no-exe-import',
                '-i', f'{work};{binary}', '-o', work / 'probe',
                root / 'langtools/tests/surface-probe.crexx'])
            run(label + '-' + mode + '-assemble', [tool('rxas'), *flags, work / 'probe'])
            run(label + '-' + mode + '-link', [tool('rxlink'), '-s', '-o', work / 'linked',
                work / 'probe.rxbin', work / 'levell_settings.rxbin', binary / 'rxfnsl.rxbin', binary / 'library.rxbin'])
            for vm in ['rxtvm', 'rxbvm']:
                if not tool(vm).exists():
                    continue
                for case, (content, status) in cases.items():
                    target = work / (case + '.txt')
                    target.write_bytes(content.encode())
                    output = run('-'.join([label, mode, vm, case]),
                                 [tool(vm), work / 'linked.rxbin', '-a', target], status).decode()
                    lines = output.splitlines()
                    assert lines[0] == f'STATUS|{status}', output
                    if status:
                        # Generic expectations may reflect a different authored
                        # rule structure; the failed input position must agree.
                        value = [line.split('|')[:2] for line in lines if line.startswith('ERROR|')]
                    else:
                        rows = [line.split('|') for line in lines if line.startswith('NODE|')]
                        if shape == 'repeated':
                            assert not any(row[2] == 'tail' for row in rows), output
                            assert sum(row[2] == 'settings' for row in rows) == 1, output
                        value = [row[2:5] for row in rows if row[2] not in ['settings', 'tail']]
                    key = (mode, vm, case)
                    if shape == 'recursive':
                        results[key] = value
                    else:
                        assert value == results[key], (backend, key, results[key], value)
                    observations.append(dict(backend=backend, shape=shape, mode=mode, vm=vm,
                        case=case, status=status, source_sha256=hashlib.sha256(content.encode()).hexdigest()))

(out / 'receipt.json').write_text(json.dumps(dict(commands=commands, observations=observations), indent=2) + '\n')
print(f'list_observations={len(observations)}; failures=0')
