#!/usr/bin/env python3
"""Freeze D01 before engine comparison; independent recognizer + edit BFS."""
from collections import deque
from pathlib import Path
import hashlib, json

ROOT = Path(__file__).resolve().parents[1]
ALPHABET = '[a],;'

def accepts(s):
    def array(p):
        if p == len(s) or s[p] != '[':
            raise ValueError
        p += 1
        if p < len(s) and s[p] == ']':
            return p + 1
        while True:
            if p < len(s) and s[p] == 'a':
                p += 1
            else:
                p = array(p)
            if p < len(s) and s[p] == ']':
                return p + 1
            if p == len(s) or s[p] != ',':
                raise ValueError
            p += 1
    try:
        p = array(0)
        while p < len(s) and s[p] == ';':
            p = array(p + 1)
        return p == len(s)
    except (ValueError, RecursionError):
        return False

def distance(s, bound=2):
    current = {s}
    seen = set(current)
    for cost in range(bound + 1):
        good = sorted(x for x in current if accepts(x))
        if good:
            return cost, good[0]
        following = set()
        if cost < bound:
            for x in current:
                for p in range(len(x)):
                    following.add(x[:p] + x[p+1:])
                for p in range(len(x) + 1):
                    for c in ALPHABET:
                        following.add(x[:p] + c + x[p:])
        current = following - seen
        seen.update(current)
    return -1, ''

def main():
    rows = []
    def add(source, mutation, parent, later=-1):
        identity = f'{parent}|{mutation}|{source}'
        split = 'heldout' if hashlib.sha256(identity.encode()).digest()[0] % 3 == 0 else 'development'
        cost, target = distance(source)
        rows.append(dict(id=f'D{len(rows)+1:03}', split=split, source=source,
                         mutation=mutation, parent=parent, clean=accepts(source),
                         exact_insert_delete_cost=cost, oracle_bound=2,
                         oracle_target=target, untouched_later_start=later))
    for base in ('[a];[a]', '[[a],a];[a]'):
        boundary = base.rfind(';')
        add(base, 'clean', base, boundary+1)
        for p in range(boundary):
            add(base[:p]+base[p+1:], f'delete@{p}', base, boundary)
            add(base[:p]+'x'+base[p+1:], f'substitute@{p}', base, boundary+1)
        for p in (0, boundary//2, boundary):
            add(base[:p]+'x'+base[p:], f'insert@{p}', base, boundary+2)
        for p in (1, boundary, len(base)-1):
            add(base[:p], f'truncate@{p}', base)
    for x in ('[a,];[a]', '[aa];[a]', '[];[a]', '[a];[x]', '[a];[a]\n', '[a];[a]\x00'):
        add(x, 'fixed-panel', x, 3 if x.startswith('[];') else -1)
    meta = dict(revision=1, selection='frozen before comparing new recovery engines',
                language='nested arrays of a, comma-separated; document arrays separated by semicolon; no whitespace',
                oracle='independent recursive descent plus exhaustive BFS insert/delete cost 1; substitution cost 2',
                bound=2, rubric=['clean input unchanged','full recovered vs partial/rejected/resource status',
                    'located code/expectation/caret/related opener','explicit repair cost model',
                    'selected clean later array retains exact source interval','bad recovery retained'],
                intent_warning='Minimum edit cost does not establish intended syntax; heldout labels are fixed before engine results.',
                cases=rows)
    target = ROOT/'evidence/recovery-corpus.json'
    text = json.dumps(meta, indent=2)+'\n'
    if target.exists() and target.read_text() != text:
        raise SystemExit('Frozen corpus differs: do not silently replace it')
    target.write_text(text)
    lines = []
    for r in rows:
        value = r['source'].encode().hex().upper() or '-'
        lines.append(f"{r['id']} {r['split']} {value} {int(r['clean'])} {r['exact_insert_delete_cost']} {r['untouched_later_start']}")
    (ROOT/'evidence/recovery-corpus.txt').write_text('\n'.join(lines)+'\n')
    print('Frozen recovery cases:', len(rows), 'heldout:',sum(x['split']=='heldout' for x in rows))

if __name__ == '__main__':
    main()
