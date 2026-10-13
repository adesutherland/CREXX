#!/usr/bin/env python3
"""Isolated cold/incremental/failure graph proof using unchanged host prerequisites."""
import argparse,hashlib,json,os,shutil,subprocess,time
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('tools',type=Path);p.add_argument('work',type=Path);p.add_argument('evidence',type=Path);a=p.parse_args()
root=Path(__file__).resolve().parents[3];tools=a.tools.resolve();work=a.work.resolve();out=a.evidence.resolve()
assert not work.exists(), 'Use a new work directory for the cold-build proof'
work.mkdir(parents=True);out.mkdir(parents=True,exist_ok=True)
source=work/'source';build=work/'build';blocked=work/'no-re2c';blocked.mkdir()
shutil.copytree(root/'lib/rxfnsl',source/'lib/rxfnsl')
(source/'cmake').mkdir();shutil.copy2(root/'cmake/CrexxImportRoots.cmake',source/'cmake/CrexxImportRoots.cmake')
(blocked/'re2c').write_text('#!/bin/sh\necho forbidden-re2c-invocation >> "'+str(out/'forbidden-re2c.log')+'"\nexit 97\n');(blocked/'re2c').chmod(0o755)
env=dict(os.environ,PATH=str(blocked)+os.pathsep+os.environ['PATH']);records=[]
def run(name,args,success=True):
 start=time.monotonic();r=subprocess.run([str(x) for x in args],capture_output=True,env=env,timeout=1800)
 data=r.stdout+r.stderr;(out/(name+'.log')).write_bytes(data)
 assert (r.returncode==0)==success,(name,r.returncode,data[-2500:])
 if success:assert b'Warning in ' not in data,(name,'compiler warning')
 records.append({'case':name,'command':[str(x) for x in args],'exit':r.returncode,'elapsed_seconds':round(time.monotonic()-start,3),'log_sha256':hashlib.sha256(data).hexdigest()})
 return data

def snapshot():
 return {str(p.relative_to(build)):(p.stat().st_mtime_ns,hashlib.sha256(p.read_bytes()).hexdigest()) for p in build.rglob('*') if p.suffix in ['.rxbin','.rxas','.stamp']}
cmd=['cmake','--build',build,'--target','rxfnsl','levell_unicode_lexer','--parallel','8','--verbose']
run('configure',['cmake','-S',source/'lib/rxfnsl/bootstrap','-B',build,f'-DCREXX_HOST_BIN={tools}','-DCMAKE_BUILD_TYPE=Debug'])
run('cold',cmd);before=snapshot();run('noop',cmd);after=snapshot()
assert before==after,'No-op build rewrote generated artifacts'
# Reproduce the incremental metadata hazard: each stage directory still holds
# a linked image with the old constructor while its new frontend is compiled.
# The driver now calls the extended constructor. Importing the broad stage
# directory can select the embedded old API; the private declared inputs must
# let all three stages rebuild without manually deleting the linked images.
frontend=source/'lib/rxfnsl/rexx/llfront.crexx'
text=frontend.read_text()
signature='arg source = .string, limit = 0, ?limits = .lllimits..policy'
assert signature in text
text=text.replace(signature,signature+', revision = 0',1)
text=text.replace('_source = source; _bytes = source as .binary; _limit = limit',
                  '_source = source; _bytes = source as .binary; _limit = limit + revision',1)
frontend.write_text(text)
driver=source/'lib/rxfnsl/tools/levell.crexx';text=driver.read_text()
call='.llfront..document(source as .string,0,limits)'
assert call in text
driver.write_text(text.replace(call,'.llfront..document(source as .string,0,limits,0)',1))
run('frontend-api-edit',cmd);after_api=snapshot()
for stage in range(3):
 name=f'tools/bootstrap/L{stage}/levell-linked.rxbin'
 assert after_api[name][0]!=after[name][0],(stage,'API change did not rebuild stage')
run('frontend-api-noop',cmd)
assert snapshot()==after_api,'API rebuild did not settle to a no-op'
after=after_api
grammar=source/'lib/rxfnsl/grammar/levell.levell';original=grammar.read_text();grammar.write_text(original+'\n/* regeneration lifecycle probe */\n')
run('grammar-edit',cmd);changed=snapshot()
assert changed['tools/bootstrap/L0/levell-linked.rxbin']==after['tools/bootstrap/L0/levell-linked.rxbin']
assert changed['tools/bootstrap/L1/llsyntax.rxas'][0]!=after['tools/bootstrap/L1/llsyntax.rxas'][0]
assert changed['tools/bootstrap/L2/llsyntax.rxas'][0]!=after['tools/bootstrap/L2/llsyntax.rxas'][0]
runtime=source/'lib/rxfnsl/rexx/llcharset.crexx';runtime.write_text(runtime.read_text().replace("'byte range'","'byte range lifecycle probe'"))
run('runtime-edit',cmd);after_runtime=snapshot()
assert after_runtime['rexx/llcharset.rxbin'][0]!=changed['rexx/llcharset.rxbin'][0]
assert after_runtime['tools/bootstrap/L0/levell-linked.rxbin'][0]!=changed['tools/bootstrap/L0/levell-linked.rxbin'][0]
last_good={name:after_runtime[name][1] for name in ['bin/rxfnsl.rxbin','tools/bootstrap/L1/llsyntax.rxas','bin/gennorm2_language.rxbin']}
grammar.write_text(original+'\nbad: rule = (\n')
run('invalid-grammar',cmd,False)
for name,digest in last_good.items():assert hashlib.sha256((build/name).read_bytes()).hexdigest()==digest,(name,'failed generation replaced valid artifact')
assert not list(build.rglob('*.tmp')),'Failed generation left a temporary output'
grammar.write_text(original);run('restored',cmd)
assert not (out/'forbidden-re2c.log').exists(),'Level L invoked re2c'
assert not (source/'lib/rxfnsg').exists() and not (source/'re2c').exists()
# All output paths/tool identities belong to this isolated graph.
receipt={'cases':records,'host_tools':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in tools.iterdir() if p.name in ['rxc','rxas','rxlink','rxtvm','rxbvm','library.rxbin']},'no_op_unchanged':True,'re2c_invocations':0,'level_g_present':False,'generated':{n:v[1] for n,v in snapshot().items()}}
(out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n');print('bootstrap_build_cases='+str(len(records))+'; failures=0')
