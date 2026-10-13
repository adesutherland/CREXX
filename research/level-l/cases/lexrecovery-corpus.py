#!/usr/bin/env python3
"""Freeze lexical-damage comparison with a separate regular-language edit oracle."""
from pathlib import Path
import re,json,hashlib
R=Path(__file__).resolve().parents[1]
ALPHABET='x="ab;/*'
# The independent recognizer supports arbitrarily many records, not just a panel pair.
PATTERN=re.compile(r'x="[ab]*"(?:/\*a*\*/)?(?:;x="[ab]*"(?:/\*a*\*/)?)*')
def accepts(s):return PATTERN.fullmatch(s) is not None
def distance(s,bound=2):
    seen={s};current={s}
    for cost in range(bound+1):
        good=sorted(x for x in current if accepts(x))
        if good:return cost,good[0]
        nxt=set()
        if cost<bound:
            for x in current:
                for p in range(len(x)):nxt.add(x[:p]+x[p+1:])
                for p in range(len(x)+1):
                    for c in ALPHABET:nxt.add(x[:p]+c+x[p:])
        current=nxt-seen;seen.update(current)
    return -1,''
records=[]
def add(s,mutation,parent,later=-1):
    ident=f'{parent}|{mutation}|{s}'
    split='heldout' if hashlib.sha256(ident.encode()).digest()[0]%3==0 else 'development'
    cost,target=distance(s.encode().decode('latin1'))
    records.append(dict(id=f'Q{len(records)+1:03}',source=s,split=split,clean=accepts(s),mutation=mutation,parent=parent,
                        exact_insert_delete_cost=cost,oracle_target=target,untouched_later_start=later))
for base in ('x="a";x="b"','x="a"/*a*/;x="b"'):
    boundary=base.rfind(';');add(base,'clean',base,boundary+1)
    for p in range(boundary):
        if base[p] in '"/*=':
            add(base[:p]+base[p+1:],f'delete@{p}',base,boundary)
            add(base[:p]+'?'+base[p+1:],f'substitute@{p}',base,boundary+1)
    for p in [2,boundary,len(base)-1]:add(base[:p],f'truncate@{p}',base)
for s in ['x=""','x="ab"/**/;x="b"','x="a";x="b"\0','x="a"/*a; x="b"','x="a\nx="b"','x="a";x="b"é']:
    add(s,'fixed',s)
meta={'scope':'Quoted values and optional nonnested block comments; exact stated byte language, not full Rexx or a complete config format.',
      'frozen_before_engine_comparison':True,'oracle':'independent regex recognizer; exhaustive insertion/deletion BFS over bytes, unit cost; substitution costs two',
      'bound':2,'later_extent':5,'lexing':'separate scanner is an admission gate; all recovery engines consume original bytes in this panel',
      'rubric':['valid-input invariance','status and full/partial/resource distinction','stable located expectations and opening quote/comment',
                'explicit cost model','untouched later record with exact source interval','bad recovery retained; no intended-repair oracle'],
      'cases':records}
p=R/'evidence/lexrecovery-corpus.json';text=json.dumps(meta,indent=2)+'\n'
if p.exists() and p.read_text()!=text:raise SystemExit('Frozen lexical corpus differs')
p.write_text(text)
(R/'evidence/lexrecovery-corpus.txt').write_text(''.join(f"{x['id']} {x['split']} {x['source'].encode().hex().upper() or '-'} {int(x['clean'])} {x['exact_insert_delete_cost']} {x['untouched_later_start']}\n" for x in records))
print('Frozen lexical recovery cases:',len(records),'heldout:',sum(x['split']=='heldout' for x in records))
