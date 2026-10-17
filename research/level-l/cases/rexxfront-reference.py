#!/usr/bin/env python3
"""Frozen B/G/C initial trees; actual compiler diagnostics remain evidence."""
from pathlib import Path
import hashlib,json,os,re,subprocess,tempfile,atexit,sys
R=Path(__file__).resolve().parents[1];B=Path(os.environ.get('CREXX_BIN_DIR',R.parents[1]/'cmake-build-debug/bin'))
owned=tempfile.TemporaryDirectory(prefix='crexx-rexxfront-');atexit.register(owned.cleanup)
E=Path(owned.name);C=E/'sources';C.mkdir()
cases={
 'c-soft-assign':('c',"if=3;then=4;else=5;say if then else\n"),
 'c-soft-label':('c',"if:;say 'label'\n"),
 'c-concat':('c',"say 'a''b';say 'a' 'b';say 'a'/* one /* two */ end */'b'\n"),
 'c-decimal-abut':('c',"say '['1.5']'\n"),
 'c-continuation':('c',"say 'a',\n'b'\n"),
 'c-if':('c',"flag=1\nif flag then say 'yes'\nelse say 'bad'\n"),
 'c-nested-if':('c',"if 1 then if 0 then say 'a';else say 'b';else say 'c'\n"),
 'c-parse':('c',"source='alpha beta gamma'\nparse upper var source first . rest\n"),
 'c-parse-dynamic':('c',"delimiter=','\nsource='left,right'\nif 1 then parse var source first (delimiter) second\n"),
 'c-parse-dot':('c',"s='a b'\nparse var s . .\n"),
 'c-bad-if':('c',"if 1 say 'bad'\n"),
 'c-bad-parse':('c',"parse upper unknown target\n"),
 'c-bad-comment':('c',"say 1 /* open\n"),
 'c-bad-quote':('c',"say 'open\n"),
 'b-continuation':('b',"x=1 + ,\n2\nreturn x\n"),
 'g-continuation':('g',"x=1 + ,\n2\nreturn x\n"),
 'b-comments':('b',"x='a'/* one /* two */ end */'b'\nreturn x\n"),
 'g-comments':('g',"x='a'/* one /* two */ end */'b'\nreturn x\n"),
 'b-final-clause':('b',"x=1+2*3"),
 'g-final-clause':('g',"x=1+2*3"),
}
line_re=re.compile(r'^( *)([A-Z_]+) : "(.*)" \((-?\d+):(-?\d+)\) \[Type: .*\]$')
def stage(log,name):
 if f'--- {name} ---' not in log:return None
 text=log.split(f'--- {name} ---',1)[1].split('\n--- ',1)[0];roots=[];stack=[]
 for line in text.splitlines():
  m=line_re.match(line)
  if not m:continue
  indent,kind,value,ln,col=m.groups();node={'kind':kind,'value':value,'children':[],'line':int(ln),'column':int(col)}
  depth=len(indent)//2
  while len(stack)>depth:stack.pop()
  if stack:stack[-1]['children'].append(node)
  else:roots.append(node)
  stack.append(node)
 return roots
manifest=[]
for name,(level,body) in cases.items():
 source=f'options level{level}\n'+body;p=C/(name+'.rexx');p.write_text(source)
 fd,tmp=tempfile.mkstemp(dir=E,suffix='.log');os.close(fd)
 cmd=[str(B/'rxc'),'-x','-d1','-dp','-n','--no-exe-import','-i',str(B),'-o',str(E/name),str(p)]
 with open(tmp,'wb') as f:run=subprocess.run(cmd,stdout=f,stderr=f)
 raw=Path(tmp).read_bytes();(E/(name+'.log')).write_bytes(raw);text=raw.decode(errors='replace')
 tree=stage(text,'STAGE_RAW');(E/(name+'.json')).write_text(json.dumps(tree,indent=2)+'\n')
 errors=[ln for ln in text.splitlines() if 'Error in ' in ln or 'error(s)' in ln]
 manifest.append({'id':name,'level':level,'source':source,'source_sha256':hashlib.sha256(source.encode()).hexdigest(),'compiler_exit':run.returncode,'errors':errors,'raw_present':tree is not None,'command':cmd})
 print(name,'exit='+str(run.returncode),'raw='+str(tree is not None))
(E/'manifest.json').write_text(json.dumps({'rxc_sha256':hashlib.sha256((B/'rxc').read_bytes()).hexdigest(),'scope':'Raw initial AST and diagnostics only; -dp stops before full compilation, exits disabled. Not execution qualification.','cases':manifest},indent=2)+'\n')
subprocess.run([sys.executable,str(R/'cases/rexxfront-projection.py'),str(E),*(['--update'] if '--update' in sys.argv else [])],check=True)
