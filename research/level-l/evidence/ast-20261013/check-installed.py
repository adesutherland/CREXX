from pathlib import Path
import hashlib,json,os,subprocess
root=Path('/Users/adrian/CLionProjects/CREXX-level-l-inline-storage')
base=root/'research/level-l/build/ast-20261013'
binary=base/'install/bin'
out=base/'installed-check';out.mkdir(exist_ok=True)
records=[]
def run(name,args,required=()):
 r=subprocess.run(list(map(str,args)),capture_output=True,timeout=1200,env=dict(os.environ,CREXX_HOME=str(binary.parent)))
 data=r.stdout+r.stderr;(out/(name+'.log')).write_bytes(data)
 assert r.returncode==0 and b'Warning in ' not in data,(name,r.returncode,data[-3500:])
 assert all(x.encode() in data for x in required),(name,data,required)
 records.append(dict(name=name,command=list(map(str,args)),status=r.returncode,sha256=hashlib.sha256(data).hexdigest()))
examples=root/'lib/rxfnsl/examples'
for name,src,grammar,sample,required in [
 ('calculator',examples/'calculator/calculator.crexx',examples/'calculator/calculator.rxl',examples/'calculator/example.calc',['RESULT|11']),
 ('fold',examples/'calculator/fold.crexx',examples/'calculator/calculator.rxl',examples/'calculator/example.calc',['RESULT|11','FOLDED|Number|derived']),
 ('settings',examples/'settings-ast.crexx',examples/'settings/09-ast.rxl',examples/'settings/expressions.settings',[' -> '])]:
 run(name+'-build',[binary/'crexx','--program',out/name,src,grammar])
 for vm in ['rxtvm','rxbvm']:
  run(name+'-'+vm,[binary/vm,(out/name).with_suffix('.rxbin'),'-a',sample],required)
 if name=='calculator':
  run(name+'-noop',[binary/'crexx','--program',out/name,src,grammar],['SKIP: project current'])
library=out/'libraries/calc'
run('library-build',[binary/'crexx','--library',library,examples/'calculator/calculator.rxl'])
run('library-consumer',[binary/'crexx','--program',out/'library-calculator',examples/'calculator/calculator.crexx','-i',library.parent,'-l',library])
for vm in ['rxtvm','rxbvm']:
 run('library-'+vm,[binary/vm,out/'library-calculator.rxbin','-a',examples/'calculator/example.calc'],['RESULT|11'])
(out/'receipt.json').write_text(json.dumps(dict(commands=records),indent=2)+'\n')
print('PASS:',len(records),'installed commands;',out/'receipt.json')
