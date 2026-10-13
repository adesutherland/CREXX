#!/usr/bin/env python3
"""Recount retained recovery records, explicitly excluding clean controls."""
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EV = ROOT / 'evidence'
files = [EV / 'recovery-corpus.json', EV / 'recovery-summary.json',
         EV / 'lexrecovery-corpus.json', EV / 'lexrecovery-summary.json',
         EV / 'tranche-16/lexrecovery-opt-rxtvm.out', EV / 'tranche-16/provenance.json']
engines = ['label-recovery', 'lr-local', 'pika-islands', 'c14']
arrays = json.loads(files[1].read_text())['records']
lexical = []
for line in files[4].read_text().splitlines():
    if not line.startswith('REC|'):
        continue
    _, ident, split, engine, status, whole, model, cost, later, offset, work, objects = line.split('|')
    lexical.append(dict(id=ident, engine=engine, status=int(status),
                        whole=int(whole), later=int(later)))
retained_hash = json.loads(files[5].read_text())['outputs'][files[4].name]
assert hashlib.sha256(files[4].read_bytes()).hexdigest() == retained_hash
result = {'scope': 'Retained research results; damaged-only denominator audit, no engine rerun', 'panels': {}}
for name, corpus_path, records in [('arrays', files[0], arrays), ('quotes-comments', files[2], lexical)]:
    corpus = {x['id']: x for x in json.loads(corpus_path.read_text())['cases']}
    panel = dict(cases=len(corpus), damaged=sum(not c['clean'] for c in corpus.values()),
                 heldout=sum(c['split'] == 'heldout' for c in corpus.values()),
                 eligible_clean=sum(c['clean'] and c['untouched_later_start'] >= 0 for c in corpus.values()),
                 engines={})
    for engine in engines:
        selected = [r for r in records if r['engine'] == engine and not corpus[r['id']]['clean']]
        eligible = [r for r in selected if corpus[r['id']]['untouched_later_start'] >= 0]
        assert len(selected) == panel['damaged']
        assert all(r['later'] >= 0 for r in eligible)
        panel['engines'][engine] = dict(whole_on_damaged=sum(r['whole'] for r in selected),
            later_preserved=sum(r['later'] == 1 for r in eligible), later_eligible=len(eligible))
    result['panels'][name] = panel
result['input_sha256'] = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in files}
out = EV / 'user-guide/recovery-counts.json'
out.write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result['panels'], indent=2))
