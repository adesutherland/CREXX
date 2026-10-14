from pathlib import Path
import subprocess,shutil,json,re,hashlib
r=Path.cwd(); b=r/'cmake-build-levell-integration'; n=r/'research/level-l/build/ast-20261013/noopt'; w=r/'research/level-l/build/ast-step04/noopt-runtime'; w.mkdir(parents=True,exist_ok=True)
i=w/'imports'; i.mkdir(exist_ok=True); records=[]
for name in ['library','classlib','rxfnsc','rxfnsg']:
    shutil.copyfile(b/'bin'/f'{name}.rxbin',i/f'{name}.rxbin')
shutil.copyfile(n/'bin/rxfnsl.rxbin',i/'rxfnsl.rxbin')
def run(key,cmd):
    p=subprocess.run(list(map(str,cmd)),capture_output=True,timeout=600); data=p.stdout+p.stderr
    (w/(key+'.log')).write_bytes(data)
    records.append(dict(key=key,command=list(map(str,cmd)),status=p.returncode,output=data.decode(errors='replace')))
    assert p.returncode == 0 and not re.search(rb'Warning in |\bWARNING\b',data),(key,p.returncode,data[-3000:])
    if 'run' in key: print(key,data.decode(),flush=True)
def compile(name,path,flags):
    source=w/'source'/name; source.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,source/path.name)
    run(name+'-compile',[b/'bin/rxc','-x','--no-exe-import',*flags,'-i',i,'-o',w/name,source/path.name]);run(name+'-assemble',[b/'bin/rxas',*flags,w/name])
for name in ['ts_llanalysis','ts_llast']:
    for mode,flags in [('opt',[]),('noopt',['-n'])]:
        key=name+'-'+mode
        compile(key,r/'lib/rxfnsl/tests_functional'/f'{name}.crexx',flags)
        run(key+'-link',[b/'bin/rxlink','-s','-o',w/(key+'-linked'),w/key,i/'rxfnsl',i/'library'])
        for vm in ['rxtvm','rxbvm']: run(key+'-run-'+vm,[b/'bin'/vm,w/(key+'-linked.rxbin')])
for name in ['rexxscriptgrammar','RexxScriptBifs']:
    shutil.copyfile(b/'rexxscript/modern'/f'{name}.rxbin',i/f'{name}.rxbin')
for name in ['RexxScriptOperators','RexxScriptAnalysis','RexxScriptProgram','RexxScriptExecution']:
    compile(name,r/'rexxscript'/f'{name}.crexx',['-n'])
    shutil.copyfile(w/f'{name}.rxbin',i/f'{name}.rxbin')
name='testRexxScriptAST'; compile(name,r/'rexxscript/tests_functional'/f'{name}.crexx',['-n'])
run(name+'-link',[b/'bin/rxlink','-s','-o',w/(name+'-linked'),w/name,*sorted(i.glob('*.rxbin'))])
for vm in ['rxtvm','rxbvm']: run(name+'-run-'+vm,[b/'bin'/vm,w/(name+'-linked.rxbin')])
(w/'receipt.json').write_text(json.dumps(dict(commands=records,imports={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in i.glob('*.rxbin')}),indent=2)+'\n')
print('PASS:',len(records),'commands')
