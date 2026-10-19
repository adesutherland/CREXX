#!/usr/bin/env python3
"""Closed-corpus raw compiler AST projection, separate from source recognition."""
from pathlib import Path
import json,re,sys
from fixture_support import write_fixture
r=Path(__file__).resolve().parents[1];e=Path(sys.argv[1])
def canonical(n):
    kind=n['kind'];value=n['value']
    if kind=='REXX_OPTIONS':return ''
    if kind in ('REXX_UNIVERSE','PROGRAM_FILE','INSTRUCTIONS'):
        return ''.join(canonical(c) for c in n['children'])
    # The debug renderer quotes quotes repeatedly; this closed fixture set has
    # no literal backslashes. Do not use this adapter for arbitrary AST strings.
    if kind=='STRING':value=re.sub(r'\\+([\'\"])',r'\1',value)
    return '('+kind+':'+value.encode().hex().upper()+''.join(canonical(c) for c in n['children'])+')'
def bad(n):return n['kind']=='ERROR' or any(bad(c) for c in n['children'])
out=[]
for c in json.loads((e/'manifest.json').read_text())['cases']:
    roots=json.loads((e/(c['id']+'.json')).read_text());assert roots
    invalid=any(bad(n) for n in roots)
    selected='' if invalid else ''.join(canonical(n) for n in roots)
    out.append('|'.join([c['id'],c['level'],c['source'].encode().hex().upper(),str(int(invalid)),selected])+'\n')
write_fixture(r/'cases/fixtures/rexxfront/expected.txt',''.join(out));print('Projected initial compiler trees:',len(out))
