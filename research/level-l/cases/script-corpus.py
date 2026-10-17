#!/usr/bin/env python3
"""Frozen expression/statement observations from the actual RexxScript consumer."""
from pathlib import Path
from fixture_support import write_fixture
import json
r=Path(__file__).resolve().parents[1]
expressions=['1+2*3','10-3-2','8/3','-8/3','--2','1--2',"'a' 'b'", "'a''b'", "'a' || 'b'",'x+3',"'total ' || x",'12x',
             "length('abc')", "substr('abc',2)", "substr('abc',2,)", "substr('abc',,2)", "substr('abc',2,1)",
             "length(substr('abcdef',2,3))", "'12'+3", '1+', '(1+2)', '1**2', "'open", 'unknown', '1/0',
             "'a'+2",'length()',"substr('abc',0)", '1\n+2', '1e2']
seed='1+2*3'
for i in range(len(seed)):
    expressions += [seed[:i]+seed[i+1:],seed[:i]+'@'+seed[i+1:]]
records=[]
for i,expression in enumerate(dict.fromkeys(expressions)):
    records.append({'id':f'E{i:03}','kind':'expression','expression':expression,'source':'result='+expression,'split':'heldout' if i%3==1 else 'development'})
statements=["a=1;say 'starting';say a;a=a+2;result=a", "result=x+3;say result", "say 'before';result='after';say result",
            "if 0 then do; say 'bad'; end; say 'good'", "say 'before'; signal done; say 'bad'; done:; say 'after'",
            "result=1+;say 'later'", "result=substr('abc',0);say 'later'", "result=1/*comment*/+2", "result='/*quoted*/'", "say 'a;b';result=3"]
for i,source in enumerate(statements):records.append({'id':f'S{i:03}','kind':'statement','source':source,'split':'heldout' if i%3==1 else 'development'})
p=r/'cases/fixtures/script-corpus.json';text=json.dumps(records,indent=2)+'\n'
if p.exists() and p.read_text()!=text:raise SystemExit('Frozen RexxScript corpus changed')
write_fixture(p,text)
write_fixture((r/'cases/fixtures/script-input.txt'),''.join(x['id']+'|'+x['source'].encode().hex().upper()+'\n' for x in records))
print('Frozen RexxScript observations:',len(records))
