from pathlib import Path
import hashlib,json,re,shutil,subprocess
r=Path(__file__).resolve().parent
source=r/'donor-qualification/crexx/application/ragimprove.crexx'
original=source.read_bytes()
shim=r/'install-measured-qualified/bin/rxc'
original_shim=shim.read_bytes()
base=(r/'run-release-qualified.sh').read_text()
results=[]
def run(label,extra=''):
    script=base.replace('release-qualified-wave','qualified-'+label)
    if extra: script=script.replace(' --jobs auto ', ' '+extra+' --jobs auto ')
    p=r/('run-qualified-'+label+'.sh');p.write_text(script)
    with (r/'evidence'/('qualified-'+label+'-driver.log')).open('w') as out:
        done=subprocess.run(['bash',str(p)],stdout=out,stderr=subprocess.STDOUT)
    e=r/'evidence'/('qualified-'+label)
    log=(e/'wave.log').read_text()
    item={'label':label,'rc':done.returncode,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'selected':re.findall(r'^START: project member (.+)$',log,re.M),'wave':re.findall(r'WAVE:.*',log),'time':(e/'wave.time').read_text(),'extra_options':extra}
    results.append(item)
    (r/'evidence/qualified-incremental-matrix.json').write_text(json.dumps({'timing_note':'Qualification with concurrent ASan QA; not comparable performance evidence.','cells':results},indent=2)+'\n')
    if done.returncode: raise RuntimeError(label+' failed; inspect '+str(e/'wave.log'))
try:
    shutil.copytree(r/'wave-qualified',r/'wave-qualified-clean')
    run('unchanged')
    old=b'  output = ""; status = sqlitecolumntext(prepared, 0, output); call sqlitefinalize prepared'
    assert original.count(old)==1
    source.write_bytes(original.replace(old,old.replace(b'output = ""',b'output = "scaling-private-probe"')))
    run('private')
    addition=b'  scaling_contract_probe: method = .int\n    return 1\n'
    marker=b'  schema: method = .string\n'
    assert marker in original
    source.write_bytes(original.replace(marker,addition+marker,1))
    run('contract')
    source.write_bytes(original)
    run('restore')
    run('options','--diagnostic-locale en_US')
    run('options-unchanged','--diagnostic-locale en_US')
    shim.write_bytes(original_shim+b'\n# tool identity qualification probe\n')
    run('toolchain','--diagnostic-locale en_US')
    run('toolchain-unchanged','--diagnostic-locale en_US')
finally:
    source.write_bytes(original)
    shim.write_bytes(original_shim)
