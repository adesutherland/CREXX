#!/usr/bin/env python3
"""Reconstruct the read-only RexxScript probe; use explicitly selected current runtime inputs."""
from pathlib import Path
import hashlib,json
r=Path(__file__).resolve().parents[1];root=r.parents[1]
import argparse
p=argparse.ArgumentParser();p.add_argument('--bin',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
d=a.output;d.mkdir(parents=True,exist_ok=True)
for name in ['rxfnsc.rxbin','classlib.rxbin']:
    (d/name).write_bytes((a.bin/name).read_bytes())
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
