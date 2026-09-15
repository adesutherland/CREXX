from pathlib import Path
import hashlib,json,os,subprocess,time
root=Path(__file__).parent
config=json.loads((root/'replay.json').read_text())
env=dict(os.environ)
for key in ('CREXX_HOME','CREXX_LLAMA_GLUE_PROBES','DYLD_LIBRARY_PATH','DYLD_FALLBACK_LIBRARY_PATH','LD_LIBRARY_PATH','GGML_BACKEND_PATH','CREXX_PROVIDER_PATH'):env.pop(key,None)
start=time.monotonic()
with (root/'replay.log').open('w') as log:
 result=subprocess.run(config['argv'],cwd=config['cwd'],env=env,stdout=log,stderr=log,timeout=300)
config.update(returncode=result.returncode,elapsed_seconds=time.monotonic()-start,asan_options=env.get('ASAN_OPTIONS'))
(root/'result.json').write_text(json.dumps(config,indent=2)+'\n')
print(json.dumps(config),flush=True)
print((root/'replay.log').read_text()[-3000:])
raise SystemExit(result.returncode)
