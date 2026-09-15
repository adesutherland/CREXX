from pathlib import Path
import hashlib,json,os,subprocess,sys,time
root=Path('/Users/adrian/CLionProjects/CREXX')
here=Path(__file__).parent
build=root/'cmake-build-debugasan'
prefix=build/'lib/plugins/llama/tests/qualification/package-generation-m6osa3qr/install'
models=Path('/Users/adrian/Library/Caches/crexx/native-inference')
commands=[['cmake','--install',str(build),'--component','llama-docs','--prefix',str(prefix)],[sys.executable,str(root/'tests/native-inference/generation_package_consumer.py'),str(prefix),str(root),str(models),str(here/'work'),'cpu,required-gpu','--start-case','opt-shared_generation']]
record={'asan_options':os.environ.get('ASAN_OPTIONS'),'commands':[],'reuses':'Three completed opt cases from package-generation-m6osa3qr; previous stricter hang limits passed.'}
start=time.monotonic()
try:
 for i,command in enumerate(commands):
  with (here/('command-'+str(i)+'.log')).open('w') as log:
   result=subprocess.run(command,cwd=root,stdout=log,stderr=log,timeout=7200)
  record['commands'].append({'argv':command,'returncode':result.returncode})
  if result.returncode:
   print((here/('command-'+str(i)+'.log')).read_text()[-4000:]);raise SystemExit(result.returncode)
 print('PASS: generation package continuation from opt-shared_generation',flush=True)
finally:
 record['elapsed_seconds']=time.monotonic()-start
 (here/'result.json').write_text(json.dumps(record,indent=2)+'\n')
