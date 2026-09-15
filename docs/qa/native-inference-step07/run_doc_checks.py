from pathlib import Path
import subprocess, os, json, hashlib, re, shutil, time
work=Path(__file__).parent
prefix=work/'install'
models=Path('/Users/adrian/Library/Caches/crexx/native-inference')
repo=Path('/Users/adrian/CLionProjects/CREXX')
env=dict(os.environ, CREXX_HOME=str(prefix), S7_MODEL_DIR=str(models))
for key in ['CREXX_LLAMA_GLUE_PROBES','GGML_BACKEND_PATH','CREXX_PROVIDER_PATH','DYLD_LIBRARY_PATH','DYLD_FALLBACK_LIBRARY_PATH','LD_LIBRARY_PATH']:
 env.pop(key,None)
records=[]
def run(label,argv,cwd=None,marker=None):
 start=time.monotonic()
 p=subprocess.run(argv,cwd=cwd or work,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=300)
 output=p.stdout
 (work/(label+'.log')).write_text('argv='+repr(argv)+'\ncwd='+str(cwd or work)+'\nrc='+str(p.returncode)+'\n'+output)
 record=dict(label=label,argv=argv,rc=p.returncode,seconds=round(time.monotonic()-start,3),marker=marker)
 records.append(record)
 (work/'recipe-results.json').write_text(json.dumps(records,indent=2)+'\n')
 assert p.returncode==0,(label,p.returncode,output[-2000:])
 assert not any(s in output for s in ['FAIL:','ERROR:','PANIC:']),(label,output[-2000:])
 if marker: assert marker in output,(label,output[-2000:])
 print('PASS '+label,flush=True)
model_doc=(prefix/'share/crexx/llama/models.md').read_text()
recipe=re.search(r'```sh\n(.*?)```',model_doc,re.S).group(1)
recipe=recipe.replace('model_dir="$HOME/crexx-models"','model_dir="$S7_MODEL_DIR"')
(work/'verify-models.sh').write_text(recipe)
run('model-cached-verification',['sh',str(work/'verify-models.sh')],marker='Verified:')
# Exercise the download branch on BGE; reuse the unchanged larger Smol artifact.
fresh=work/'downloaded-models'; fresh.mkdir()
(fresh/'smollm2-360m-instruct-q8_0.gguf').symlink_to(models/'smollm2-360m-instruct-q8_0.gguf')
env['S7_MODEL_DIR']=str(fresh)
run('model-fresh-bge-download',['sh',str(work/'verify-models.sh')],marker='Verified:')
inputs=[('embeddings','bge-small-en-v1.5-f16.gguf','f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999','PASS: persistent typed embedding example'),('generation','smollm2-360m-instruct-q8_0.gguf','48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201','PASS: persistent typed generation example')]
app=work/'llama-example-work'; app.mkdir()
for name,model,digest,marker in inputs:
 source=prefix/'share/crexx/llama/examples'/('persistent_'+name+'.crexx')
 run(name+'-native-build',[str(prefix/'bin/crexx'),'--program',str(app/name),str(source),'--jobs','1','--native'],app,'PUBLISHED: native program')
 run(name+'-native-auto',[str(app/name),'auto',str(models/model),digest],app,marker)
# Execute the four-tool route exactly with resolved prefix/model variables.
run('vm-compile',[str(prefix/'bin/rxc'),'--no-exe-import','-i',str(prefix/'bin'),'-o','embeddings_vm',str(prefix/'share/crexx/llama/examples/persistent_embeddings.crexx')],app)
run('vm-assemble',[str(prefix/'bin/rxas'),'-o','embeddings_vm','embeddings_vm'],app)
run('vm-link',[str(prefix/'bin/rxlink'),'-o','embeddings_linked','embeddings_vm',str(prefix/'bin/library'),str(prefix/'bin/classlib'),str(prefix/'bin/rxfnsg')],app)
run('vm-auto',[str(prefix/'bin/rxvm'),'embeddings_linked','-a','auto',str(models/inputs[0][1]),inputs[0][2]],app,inputs[0][3])
# Verbatim guide source identity, plus all runtime package bytes used here.
ident={}
for path in list((prefix/'share/crexx/llama').rglob('*'))+list(app.glob('*.native.json')):
 if path.is_file():ident[str(path.relative_to(work))]=hashlib.sha256(path.read_bytes()).hexdigest()
(work/'recipe-identities.json').write_text(json.dumps(ident,indent=2)+'\n')
