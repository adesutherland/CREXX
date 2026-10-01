from pathlib import Path
import hashlib,json,subprocess,sys,time
root=Path('/Users/adrian/CLionProjects/CREXX')
scratch=Path(__file__).parent
kind=sys.argv[1]
build=root/('cmake-build-debug' if kind=='normal' else 'cmake-build-debugasan')
output=scratch/(kind+'-wide-backstop');output.mkdir()
command=['cmake','-DBUILD='+str(build),'-DSOURCE='+str(root),'-DOUTPUT='+str(output),'-DMODE=cpu','-DMODEL=/Users/adrian/Library/Caches/crexx/native-inference/smollm2-360m-instruct-q8_0.gguf','-DHASH=48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201','-DPROGRAM=shared_generation','-DCONSUMER='+str(root/'lib/plugins/llama/examples/shared_generation.crexx'),'-DMARKER=PASS: four typed generation workers','-P',str(root/'tests/native-inference/step04_toolchain.cmake')]
start=time.monotonic()
with (output/'driver.log').open('w') as log: result=subprocess.run(command,stdout=log,stderr=log,timeout=7200)
record={'argv':command,'returncode':result.returncode,'elapsed_seconds':time.monotonic()-start,'source_sha256':hashlib.sha256((root/'lib/plugins/llama/examples/shared_generation.crexx').read_bytes()).hexdigest()}
(output/'result.json').write_text(json.dumps(record,indent=2)+'\n')
print(json.dumps(record),flush=True)
print((output/'driver.log').read_text()[-4000:])
raise SystemExit(result.returncode)
