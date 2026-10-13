from pathlib import Path
import re, subprocess, shutil, json
root=Path.cwd(); work=root/'research/level-l/build/manual-review/projection'; work.mkdir(parents=True,exist_ok=True)
binary=root/'cmake-build-levell-product/bin'; imports=work/'imports'; imports.mkdir(exist_ok=True)
for name in ['rxfnsl','library']: shutil.copy2(binary/(name+'.rxbin'),imports/(name+'.rxbin'))
spec=work/'projection.levell'
spec.write_text('''options levell
namespace manual_projection
number: token
scan: lexer
  digit: charset = '0' to '9'
  number.lex: rule = digit+ then emit number
parse: parser start value
  value: rule = number
keep.value: tree rule value kind 10 arity 0
''')
consumer=work/'read.crexx'
consumer.write_text('''options levelb
import llcore
import lllanguage
import llproject
import manual_projection
main: procedure = .int
  definition = manual_projection..language()
  source = '12' as .binary
  request = .lllanguage..request(definition,source)
  status = request.parse()
  if status <> 0 then do
    say request.error(); return status
  end
  projection = request.projection()
  status = projection.run()
  if status <> 0 then do
    say projection.error(); return status
  end
  result = projection.tree()
  root = projection.root()
  if result.kind(root) <> 10 | result.label(root) <> 'value' | result.left(root) <> 0 | result.right(root) <> 0 then return 3
  if result.start(root) <> 0 | result.finish(root) <> 2 | projection.origin(root) < 1 then return 3
  say 'PROJECTION|value|10|0|2'
  return 0
''')
records=[]
def run(name,argv):
    r=subprocess.run(list(map(str,argv)),capture_output=True,cwd=root,timeout=300)
    (work/(name+'.log')).write_bytes(r.stdout+r.stderr)
    assert r.returncode==0,(name,r.returncode,r.stdout[-2000:],r.stderr[-2000:])
    assert not re.search(rb'Warning in |\bWARNING\b',r.stdout+r.stderr)
    records.append(dict(name=name,argv=list(map(str,argv)),status=r.returncode,stdout=r.stdout.decode(),stderr=r.stderr.decode()))
    return r.stdout
run('generate',['cmake','-DLEVEL_L='+str(binary/'levell'),'-DSPEC='+str(spec),'-DOUTPUT='+str(work/'manual_projection.rxas'),'-DFORMAT=rxas','-P',root/'lib/rxfnsl/tools/generate.cmake'])
run('assemble-grammar',[binary/'rxas',work/'manual_projection'])
shutil.copy2(work/'manual_projection.rxbin',imports/'manual_projection.rxbin')
run('compile',[binary/'rxc','-x','--no-exe-import','-i',imports,'-o',work/'read',consumer])
run('assemble-reader',[binary/'rxas',work/'read'])
run('link',[binary/'rxlink','-s','-o',work/'linked',work/'read.rxbin',work/'manual_projection.rxbin',binary/'rxfnsl.rxbin',binary/'library.rxbin'])
for vm in ['rxtvm','rxbvm']: assert run(vm,[binary/vm,work/'linked.rxbin'])==b'PROJECTION|value|10|0|2\n'
(work/'receipt.json').write_text(json.dumps(records,indent=2)+'\n')
print('projection_commands='+str(len(records))+'; failures=0')
