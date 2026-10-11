#!/usr/bin/env python3
"""Frozen compiler raw/fixup snapshots -> explicit consumer adapter fixtures.

The compact projection excludes OPTIONS, synthetic empty ARGS, synthetic CLASS
and type facts; declared CLASS nodes remain.
Raw anchors are independently recovered from source lines for this closed corpus;
raw compiler dumps carry (-1,-1). This is deliberately not a general AST format.
"""
from pathlib import Path
import hashlib,json,os,re,subprocess,tempfile
r=Path(__file__).resolve().parents[1]; bins=Path(os.environ.get('CREXX_BIN_DIR',r/'build/frozen-tools-20261010'))
e=r/'evidence/migration';e.mkdir(exist_ok=True)
kinds={'PROCEDURE':10,'CLASS':11,'ASSIGN':20,'RETURN':21,'VAR_TARGET':22,'VAR_SYMBOL':23,'INTEGER':24,'OP_ADD':25,'OP_MULT':26,'INSTRUCTIONS':30}
line_re=re.compile(r'^( *)([A-Z_]+) : "(.*)" \((-?\d+):(-?\d+)\) \[Type: .*\]$')
def stage(log,name):
 text=log.split(f'--- {name} ---',1)[1].split('\n--- ',1)[0]
 roots=[];stack=[]
 for line in text.splitlines():
  m=line_re.match(line)
  if not m:continue
  indent,kind,value,ln,col=m.groups();node={'kind':kind,'value':value,'children':[],'line':int(ln),'column':int(col)}
  depth=len(indent)//2
  while len(stack)>depth:stack.pop()
  if stack:stack[-1]['children'].append(node)
  else:roots.append(node)
  stack.append(node)
 return roots[0]
def descend(n,kind):
 if n['kind']==kind:return n
 for c in n['children']:
  found=descend(c,kind)
  if found:return found
 return None
def canonical(n):
 if n['kind'] in ['REXX_UNIVERSE','PROGRAM_FILE']:return ''.join(canonical(c) for c in n['children'])
 if n['kind'] in ['REXX_OPTIONS','ARGS']:return ''
 if n['kind']=='CLASS' and n['line']<0:return ''
 return '('+n['kind']+':'+n['value']+''.join(canonical(c) for c in n['children'])+')'
# Explicit B/G pair and an implicit-main variant; no exit-driven syntax.
sources={
 'procedures-b':'options levelb\nmain: procedure = .int\n  x = 1 + 2 * 3\n  return x\nother: procedure = .int\n  return 4\n',
 'procedures-g':'options levelg\nmain: procedure = .int\n  x = 1 + 2 * 3\n  return x\nother: procedure = .int\n  return 4\n',
 'implicit-b':'options levelb\nx = 1 + 2 * 3\nreturn x\nother: procedure = .int\n  return 4\n',
}
records=[]
for name,source in sources.items():
 path=r/f'cases/migration/{name}.crexx';path.write_text(source)
 fd,tmp=tempfile.mkstemp(prefix='crexx-levell-migration-',suffix='.log');os.close(fd)
 command=[str(bins/'rxc'),'-x','-d1','-dp','-n','--no-exe-import','-i',str(bins),'-o',str(r/f'build/{name}'),str(path)]
 with open(tmp,'wb') as log:run=subprocess.run(command,stdout=log,stderr=log)
 log=Path(tmp).read_text();(e/f'{name}.log').write_text(log)
 if run.returncode:raise SystemExit(f'{name}: compiler {run.returncode}; inspect {tmp}')
 raw=stage(log,'STAGE_RAW');fixed=stage(log,'STAGE_FIXUP')
 (e/f'{name}.expected').write_text(canonical(fixed)+'\n')
 flat=descend(raw,'INSTRUCTIONS')['children'];rows=[];linepos=1
 starts=[];offset=0
 for line in source.splitlines(True):starts.append(offset);offset+=len(line)
 # Recursively anchor token-bearing subtrees in each authored source line.
 def place(n,lo,hi):
  global cursor
  kind=n['kind']; value=n['value']; text=source[lo:hi]
  children=n['children']; spans=[]
  if kind in ['ASSIGN','OP_ADD','OP_MULT']:
   symbol=value; at=source.index(symbol,lo,hi)
   spans=[place(children[0],lo,at),place(children[1],at+len(symbol),hi)]
  elif kind=='RETURN':
   at=source.index('return',lo,hi);spans=[place(c,at+6,hi) for c in children]
  elif kind=='PROCEDURE':
   at=source.index(value,lo,hi);spans=[place(c,at+len(value),hi) for c in children]
  else:at=source.index(value,lo,hi)
  start=min([at]+[s[1] for s in spans]);finish=max([at+len(value)]+[s[2] for s in spans])
  ids=[s[0] for s in spans]
  if len(ids)>2:raise AssertionError('adapter requires binary fixture shape')
  rows.append((kinds[kind],kind+':'+value,start,finish,ids[0] if ids else 0,ids[1] if len(ids)>1 else 0))
  return len(rows),start,finish
 root=0;first=0;finish=0
 lines=source.splitlines(True)
 for n in flat:
  while linepos<len(lines) and not lines[linepos].strip():linepos+=1
  lo=starts[linepos];hi=lo+len(lines[linepos].rstrip('\n'));node,begin,finish=place(n,lo,hi);linepos+=1
  if root:
   rows.append((99,'list',first,finish,root,node));root=len(rows)
  else:root=node;first=begin
 out=f'{source.encode().hex()}\n{root}\n'+''.join('|'.join(map(str,row))+'\n' for row in rows)
 (e/f'{name}.tree').write_text(out)
 records.append({'name':name,'command':command,'returncode':run.returncode,'source_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'log_sha256':hashlib.sha256(log.encode()).hexdigest()})
(e/'reference.json').write_text(json.dumps({'tool_sha256':hashlib.sha256((bins/'rxc').read_bytes()).hexdigest(),'cases':records,'scope':'B/G structural grouping with compiler exits disabled; no full AST/service equivalence'},indent=2)+'\n')
print('Retained',len(records),'raw/fixup compiler references.')
