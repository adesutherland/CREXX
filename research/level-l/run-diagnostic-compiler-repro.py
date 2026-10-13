#!/usr/bin/env python3
"""Retain compiler mode disagreement; this does not repair the compiler."""
from pathlib import Path
import subprocess, json, hashlib
ROOT=Path(__file__).resolve().parent
TOOLS=ROOT/'build/frozen-tools-20261010'
OUT=ROOT/'evidence/recovery-diagnostic-opt-failure'
SOURCE=ROOT/'cases/diagnostic-call-repro.crexx'
h=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
record={'source':{str(SOURCE.relative_to(ROOT)):h(SOURCE)}, 'tools':{}, 'modules':{}, 'outputs':{}}
OUT.mkdir(exist_ok=True)
with (OUT/'reproducer-build.log').open('w') as log:
    for mode in ('opt','noopt'):
        target=OUT/f'reproducer-{mode}'
        flags=['-n'] if mode=='noopt' else []
        build=ROOT/f'build/recovery-{mode}'
        modules=[build/(name+'.rxbin') for name in ('llcore','llcheck','lllabel')]
        for m in modules: record['modules'][str(m.relative_to(ROOT))]=h(m)
        for command in ([str(TOOLS/'rxc'),'--no-exe-import','-x',*flags,'-i',str(build/'imports'),'-o',str(target),str(SOURCE)],
                        [str(TOOLS/'rxas'),*flags,str(target)],
                        [str(TOOLS/'rxlink'),'-o',str(target)+'-linked',str(target)+'.rxbin',*[str(m) for m in modules],str(TOOLS/'library.rxbin')]):
            subprocess.run(command,stdout=log,stderr=log,check=True)
        for vm in ('rxtvm','rxbvm'):
            result=subprocess.run([str(TOOLS/vm),str(target)+'-linked.rxbin'],capture_output=True,text=True,check=True)
            f=OUT/f'reproducer-{mode}-{vm}.out';f.write_text(result.stdout+result.stderr)
            record['outputs'][f.name]=h(f)
for name in ('rxc','rxas','rxlink','rxtvm','rxbvm','library.rxbin'): record['tools'][name]=h(TOOLS/name)
a=(OUT/'reproducer-opt-rxtvm.out').read_text();b=(OUT/'reproducer-noopt-rxtvm.out').read_text()
record['mode_disagreement']=a!=b
record['status']='open frozen-tool finding; no production repair or current-develop reproduction claimed'
(OUT/'provenance.json').write_text(json.dumps(record,indent=2)+'\n')
print('Frozen-tool diagnostic mode disagreement:',a!=b)
