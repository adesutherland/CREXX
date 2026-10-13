#!/usr/bin/env python3
"""Independent canonical LR admission of the exact common token-CFG graph."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
p = argparse.ArgumentParser()
p.add_argument('levell', type=Path)
p.add_argument('bison', type=Path)
p.add_argument('spec', type=Path)
p.add_argument('work', type=Path)
a = p.parse_args(); a.work.mkdir(parents=True, exist_ok=True)
result = subprocess.run([str(a.levell.resolve()), str(a.spec.resolve()), '--graph'], capture_output=True, check=True, timeout=600)
(a.work / 'graph.txt').write_bytes(result.stdout)
root = None; nodes = {}
for line in result.stdout.decode().splitlines():
    fields = line.split('|')
    if fields[0] == 'ROOT': root = int(fields[1])
    elif fields[0] == 'NODE': nodes[int(fields[1])] = tuple(map(int, fields[2:5]))
assert root in nodes
terminals = sorted({left for op, left, right in nodes.values() if op == 1})
lines = ['%define lr.type canonical-lr', '%token ' + ' '.join(f'T{x}' for x in terminals), f'%start N{root}', '%%']
for node, (op, left, right) in nodes.items():
    if op == 1: rhs = f'T{left}'
    elif op == 2: rhs = f'N{left} N{right}'
    elif op == 4: rhs = f'N{left} | N{right}'
    elif op == 5: rhs = f'N{left}'
    elif op == 6: rhs = '%empty'
    elif op == 7: rhs = f'N{left} N{node} | %empty'
    elif op == 14: rhs = f'N{left} N{node} | N{left}'
    elif op == 15: rhs = f'N{left} | %empty'
    else: raise ValueError(f'unsupported independent-control clause {op}')
    lines.append(f'N{node}: {rhs};')
grammar = a.work / 'grammar.y'; grammar.write_text('\n'.join(lines) + '\n')
run = subprocess.run([str(a.bison.resolve()), '--report=state', '-Werror=conflicts-sr', '-Werror=conflicts-rr', '-o', str(a.work / 'grammar.c'), str(grammar)], capture_output=True, timeout=600)
(a.work / 'bison.log').write_bytes(run.stdout + run.stderr)
run.check_returncode()
report = (a.work / 'grammar.output').read_text()
receipt = {'graph_sha256': hashlib.sha256(result.stdout).hexdigest(), 'bison_sha256': hashlib.sha256(a.bison.read_bytes()).hexdigest(), 'states': len(re.findall(r'^State \d+$', report, re.M)), 'conflicts': 0, 'scope': 'Exact common CFG; Bison adds its EOF final state. Independent admission, not runtime tree equivalence.'}
(a.work / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
