#!/usr/bin/env python3
import argparse,hashlib,json,struct,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('bin',type=Path);p.add_argument('output',type=Path);a=p.parse_args()
root=Path(__file__).resolve().parents[3];bin=a.bin.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
cases=['','Unicode 17.0.0\n','0041..005A:230\n','00C0=0041 0300\n','0061>0041\n','0041*\n','# café 😀 Ελληνικά\r\n0041=0042 #tail\n','Unicode 17.0.0\n0041:0\n00C0=0041 0300\n','0041=\n','110000=0041\n','D800=0041\n','0041..0040:230\n','0041:256\n','0041=0042','0041 @\n','#comment\x00','0041\r']
data=struct.pack('<I',len(cases))+b''.join(struct.pack('<I',len(s.encode()))+s.encode() for s in cases);(out/'cases.bin').write_bytes(data)
outputs=[];logs=[]
def run(name,args):
 r=subprocess.run([str(x) for x in args],capture_output=True,timeout=600);(out/(name+'.log')).write_bytes(r.stdout+r.stderr)
 assert r.returncode==0,(name,(r.stdout+r.stderr)[-2400:]);assert b'Warning in ' not in r.stdout+r.stderr,name
 return r.stdout
for implementation in ['reference','native']:
 for mode in ['opt','noopt']:
  work=out/(implementation+'-'+mode);work.mkdir(exist_ok=True);flags=['-n'] if mode=='noopt' else []
  lexer=(root/'research/level-l/evidence/bootstrap/reference-gennorm2.crexx') if implementation=='reference' else root/'lib/rxfnsg/unicode/tools/level_l_gennorm2.crexx'
  modules=[]
  for name,src in [('level_l_gennorm2',lexer),('unicode_gennorm2',root/'lib/rxfnsg/unicode/tools/unicode_gennorm2.crexx'),('probe',root/'lib/rxfnsl/tests_functional/gennorm2-record-probe.crexx')]:
   run(implementation+'-'+mode+'-'+name+'-compile',[bin/'rxc',*flags,'-x','--no-exe-import','-i',f'{work};{bin}','-o',work/name,src])
   run(implementation+'-'+mode+'-'+name+'-assemble',[bin/'rxas',*flags,work/name]);modules.append(work/(name+'.rxbin'))
  run(implementation+'-'+mode+'-link',[bin/'rxlink','-s','-o',work/'linked',modules[-1],*modules[:-1],bin/'gennorm2_language.rxbin',bin/'rxfnsl.rxbin',bin/'library.rxbin'])
  for vm in ['rxtvm','rxbvm']:
   output=run(implementation+'-'+mode+'-'+vm,[bin/vm,work/'linked.rxbin','-a',out/'cases.bin']);outputs.append(output)
def without_nul_control(data):
 blocks=data.split(b'CASE|')
 return b'CASE|'.join(block for block in blocks if not block.startswith(b'16|'))
assert len(set(without_nul_control(output) for output in outputs))==1,'Generated lexer changed owning parser observations'
assert all(b'CASE|16|0|0' in output for output in outputs[:4]),'Legacy NUL discrepancy changed'
assert all(b'CASE|16|2|8' in output for output in outputs[4:]),'Embedded NUL must be rejected at its true byte position' 
(out/'receipt.json').write_text(json.dumps({'cases':len(cases),'cells':8,'identical_record_token_error_observations_except_documented_NUL_correction':True,'NUL_correction':'Legacy accepted truncated comment; native rejects NUL at byte 8','output_sha256':hashlib.sha256(outputs[0]).hexdigest()},indent=2)+'\n');print('gennorm2_compat_cases='+str(len(cases))+'; cells=8; failures=0')
