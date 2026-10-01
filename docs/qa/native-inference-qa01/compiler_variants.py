from pathlib import Path
import subprocess,shlex,json,shutil
root=Path('/Users/adrian/CLionProjects/CREXX'); build=root/'cmake-build-debug'; work=Path('/tmp/ni-s4-qa01')
commands=json.loads((build/'compile_commands.json').read_text()); lines=(work/'compiler-commands.txt').read_text().splitlines(); link=shlex.split(lines[-1]); assert link[:2]==[':','&&']; link=link[2:link.index('&&',2)]
assert 'bin/librxclib.a' in link
for name in ('rxcp_ast_core','rxcpsymb'):
 folder=work/name; folder.mkdir(exist_ok=True)
 src=folder/(name+'.c'); src.write_bytes(subprocess.check_output(['git','show','c2cf28a4f:compiler/'+name+'.c'],cwd=root))
 entry=next(x for x in commands if x['file']==str(root/'compiler'/src.name) and 'rxclib.dir' in x.get('command',''))
 argv=shlex.split(entry['command']); argv[argv.index('-o')+1]=str(folder/(name+'.c.o')); argv[argv.index('-c')+1]=str(src)
 for option in ('-MF','-MT'):
  if option in argv: del argv[argv.index(option):argv.index(option)+2]
 if '-MD' in argv: argv.remove('-MD')
 logs=[]
 def run(cmd,cwd):
  r=subprocess.run(cmd,cwd=cwd,capture_output=True,text=True,timeout=60); logs.append(repr(cmd)+'\n'+r.stdout+r.stderr); assert r.returncode==0,logs[-1]
 run(argv,build)
 archive=folder/'librxclib.a'; shutil.copy2(build/'bin/librxclib.a',archive)
 run(['/usr/bin/ar','r',str(archive),str(folder/(name+'.c.o'))],build)
 args=list(link); args[args.index('bin/librxclib.a')]=str(archive); args[args.index('-o')+1]=str(folder/'rxc')
 run(args,build)
 # Object archive members retain the .c.o basename; replacement must use that name.
 (folder/'build.log').write_text('\n'.join(logs))
 r=subprocess.run([str(folder/'rxc'),'--no-exe-import','-n','-x','-i',str(build/'bin'),'-o',str(folder/'http'),str(work/'minimal_http.crexx')],capture_output=True,text=True,timeout=60)
 (folder/'repro.log').write_text(r.stdout+r.stderr); print(name,r.returncode,r.stderr[-220:],flush=True)
