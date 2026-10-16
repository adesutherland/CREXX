"""Explicit noopt-bootstrap and installed-wrapper checks. Run from repo root."""
from pathlib import Path
import hashlib, json, os, shutil, subprocess, sys
section = sys.argv[1] if len(sys.argv) > 1 else "all"
assert section in ["all", "noopt", "installed"]
root=Path.cwd(); build=root/'cmake-build-levell-integration'; binary=build/'bin'
base=root/'research/level-l/build/ast5'; noopt=base/'noopt'; output=base/'delivery'
output.mkdir(parents=True,exist_ok=True); records=[]
def run(key, command, required=(), installed=False):
    command=list(map(str,command))
    result=subprocess.run(command,capture_output=True,timeout=1200,
        env=dict(os.environ,CREXX_HOME=str(base/'install')) if installed else None)
    data=result.stdout+result.stderr; (output/(key+'.log')).write_bytes(data)
    records.append(dict(key=key,command=command,status=result.returncode,output=data.decode(errors='replace')))
    assert result.returncode==0 and b'Warning in ' not in data,(key,result.returncode,data[-4000:])
    assert all(s.encode() in data for s in required),(key,data,required)
    return result.stdout
spec=root/'lib/rxfnsl/examples/flow/rules.rxl'
consumer=root/'lib/rxfnsl/tests_functional/ts_llflow.crexx'
if section != 'installed':
    for route in ['source','rxas']:
        options=['--rxas'] if route=='rxas' else []
        expected=run(route+'-optimized-generate',[binary/'levell',spec,*options])
        for stage in range(3):
            actual=run(route+f'-noopt-L{stage}',[binary/'rxtvm',noopt/f'tools/bootstrap/L{stage}/levell-linked.rxbin','-a',spec,*options])
            assert actual==expected,(route,stage)
        for mode,flags in [('opt',[]),('noopt',['-n'])]:
            key=route+'-'+mode; work=output/key; imports=work/'imports'; imports.mkdir(parents=True,exist_ok=True)
            for module in ['rxfnsl','library']:
                shutil.copyfile(noopt/'bin'/f'{module}.rxbin',imports/f'{module}.rxbin')
            artifact=work/('flowdemo.rxas' if route=='rxas' else 'flowdemo.crexx'); artifact.write_bytes(actual)
            if route=='source': run(key+'-provider-compile',[binary/'rxc','-x','--no-exe-import',*flags,'-i',imports,'-o',work/'flowdemo',artifact])
            run(key+'-provider-assemble',[binary/'rxas',*flags,work/'flowdemo'])
            shutil.copyfile(work/'flowdemo.rxbin',imports/'flowdemo.rxbin')
            run(key+'-compile',[binary/'rxc','-x','--no-exe-import',*flags,'-i',imports,'-o',work/'consumer',consumer])
            run(key+'-assemble',[binary/'rxas',*flags,work/'consumer'])
            run(key+'-link',[binary/'rxlink','-s','-o',work/'linked',work/'consumer',work/'flowdemo',imports/'rxfnsl',imports/'library'])
            for vm in ['rxtvm','rxbvm']: run(key+'-'+vm,[binary/vm,work/'linked.rxbin'],['failures=0'])
installed=base/'install/bin'; example=root/'lib/rxfnsl/examples/flow/check.crexx'
required=['UNASSIGNED at line 4, column 5 (byte 32)', 'false branch of If on line 1','Stored notes: 2','Root priority: 0','Root has explicit priority: 0']
if section != 'noopt':
    for extension in ['rxl','crexx']:
        work=output/('installed-'+extension); work.mkdir(exist_ok=True)
        grammar=work/('rules.'+extension)
        grammar.write_text(('options levell\n' if extension == 'crexx' else '') + spec.read_text())
        target=work/'flow'
        command=[installed/'crexx','--program',target,grammar,example]
        run(extension+'-combined',command,installed=True)
        for vm in ['rxtvm','rxbvm']: run(extension+'-'+vm,[installed/vm,target.with_suffix('.rxbin')],required,True)
        run(extension+'-repeat',command,['SKIP: project current'],True)
    library=output/'libraries/flowdemo'
    run('library-build',[installed/'crexx','--library',library,spec],installed=True)
    run('separate-consumer',[installed/'crexx','--program',output/'separate',example,'-i',library.parent,'-l',library],installed=True)
    for vm in ['rxtvm','rxbvm']: run('separate-'+vm,[installed/vm,output/'separate.rxbin'],required,True)
(output/(section+'-receipt.json')).write_text(json.dumps(dict(commands=records,images={str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [binary/'rxfnsl.rxbin',noopt/'bin/rxfnsl.rxbin',installed/'rxfnsl.rxbin'] if p.exists()}),indent=2)+'\n')
print('PASS:',section,len(records),'delivery commands')
