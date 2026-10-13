#!/usr/bin/env python3
"""Reconstruct the read-only RexxScript probe; refuse changed binary imports."""
from pathlib import Path
import hashlib,json
r=Path(__file__).resolve().parents[1];root=r.parents[1]
d=r/'build/rexxscript-imports';d.mkdir(exist_ok=True)
record=json.loads((r/'evidence/rexxscript-import.json').read_text())
for name,entry in record['dependencies'].items():
    target=d/name
    source=target if target.exists() else Path(entry['source'])
    data=source.read_bytes()
    if hashlib.sha256(data).hexdigest()!=entry['sha256']:
        raise SystemExit(f'{name}: recorded dependency differs; supply the recorded binary or explicitly requalify a new reference')
    target.write_bytes(data)
s=(root/'rexxscript/RexxScriptEvaluator.crexx').read_text()
marker='\n_modify_var_symbol: procedure'
assert s.count(marker)==1
extra='''
  research_code: method = .string
    return error_code
  research_column: method = .int
    return error_column
  research_statement: method = .int
    return error_statement
'''
p=d/'RexxScriptProbeCore.crexx';text=s.replace(marker,extra+marker)
if p.exists() and p.read_text()!=text:raise SystemExit('Owning source changed; requalification required')
p.write_text(text)
print('Prepared unchanged evaluator plus three read-only diagnostic getters.')
