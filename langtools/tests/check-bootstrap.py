#!/usr/bin/env python3
"""Independent scalar/regex oracles, stage agreement and exact-byte VM controls."""
import argparse, hashlib, json, random, re, struct, subprocess
from pathlib import Path
from _scratch import scratch
p=argparse.ArgumentParser();p.add_argument('build',type=Path);p.add_argument('output',type=Path);p.add_argument('--tools',type=Path);p.add_argument('--only',nargs='*');p.add_argument('--source',action='store_true');a=p.parse_args()
root=Path(__file__).resolve().parents[2];build=a.build.resolve();out=scratch();out.mkdir(parents=True,exist_ok=True)
bin=a.tools.resolve() if a.tools else build/'bin';runtime=build/'bin';stages=build/'langtools/bootstrap'
records=[]
def run(name,args):
 r=subprocess.run([str(x) for x in args],capture_output=True,timeout=1200)
 (out/(name+'.log')).write_bytes(r.stdout+r.stderr)
 assert r.returncode==0,(name,r.returncode,(r.stdout+r.stderr)[-2500:])
 assert b'Warning in ' not in r.stdout+r.stderr,(name,'unexpected compiler warning')
 return r.stdout

def qualify(name,spec,cases,oracle):
 if a.only and name not in a.only:return
 records[:]=[r for r in records if r['case']!=name]
 work=out/name;work.mkdir(exist_ok=True);path=work/'probe.levell';path.write_text(spec)
 artifacts=[]
 for stage in range(3):
  artifacts.append(run(name+'-L'+str(stage),[bin/'rxtvm',stages/f'L{stage}/rxlang-linked.rxbin','-a',path,'--rxas']))
 assert artifacts[0]==artifacts[1]==artifacts[2],(name,'stage disagreement')
 (work/'probe_language.rxas').write_bytes(artifacts[2])
 if a.source:
  (work/'probe_language.crexx').write_bytes(run(name+'-source',[bin/'rxtvm',stages/'L2/rxlang-linked.rxbin','-a',path]))
 data=struct.pack('<I',len(cases))+b''.join(struct.pack('<I',len(x))+x for x in cases)
 (work/'cases.bin').write_bytes(data)
 outputs=[]
 for mode in ['opt','noopt']:
  flags=['-n'] if mode=='noopt' else []
  if a.source:
   run(name+'-'+mode+'-source-compile',[bin/'rxc',*flags,'-x','--no-exe-import','-i',f'{runtime};{bin}','-o',work/'probe_language',work/'probe_language.crexx'])
  run(name+'-'+mode+'-grammar',[bin/'rxas',*flags,'-o',work/'probe_language',work/'probe_language.rxas'])
  run(name+'-'+mode+'-compile',[bin/'rxc',*flags,'-x','--no-exe-import','-i',f'{work};{runtime};{bin}','-o',work/'probe',root/'langtools/tests/bootstrap-probe.crexx'])
  run(name+'-'+mode+'-assemble',[bin/'rxas',*flags,work/'probe'])
  run(name+'-'+mode+'-link',[bin/'rxlink','-s','-o',work/'linked',work/'probe.rxbin',work/'probe_language.rxbin',runtime/'rxfnsl.rxbin',runtime/'library.rxbin'])
  for vm in ['rxtvm','rxbvm']:
   text=run(name+'-'+mode+'-'+vm,[bin/vm,work/'linked.rxbin','-a',work/'cases.bin']).decode()
   outputs.append(text);actual=[]
   for line in text.splitlines():
    fields=line.split('|')
    if fields[0]=='CASE':actual.append({'status':int(fields[2]),'offset':int(fields[3]),'error':'|'.join(fields[4:]),'tokens':[]})
    elif fields[0]=='TOKEN':actual[-1]['tokens'].append(tuple(map(int,fields[1:])))
    else:raise AssertionError((name,line))
   assert len(actual)==len(cases)
   for i,(source,result) in enumerate(zip(cases,actual)):
    expected=oracle(source)
    assert (result['status']==0)==(expected is not None),(name,i,source,result,expected)
    if expected is not None:assert result['tokens']==expected,(name,i,source,result,expected)
    else:assert 'bad_' in result['error'],(name,i,result)
 assert len(set(outputs))==1,(name,'VM/optimizer disagreement')
 records.append({'case':name,'inputs':len(cases),'cells':4,'stage_artifact_sha256':hashlib.sha256(artifacts[2]).hexdigest(),'output_sha256':hashlib.sha256(outputs[0].encode()).hexdigest()})

header='''options levell
namespace probe_language
eof: token = 0
value: token
bad_encoding: diagnostic = 'malformed bytes'
bad_character: diagnostic = 'unmatched scalar'
scan: lexer input utf8
'''
footer='''  end.input: rule = eof then emit eof
  malformed.input: rule = malformed then error bad_encoding
  unmatched.input: rule = otherwise then error bad_character
'''
random.seed(20261011)
points=set(range(0,0x110000,997))|{0,1,0x40,0x41,0x5a,0x5b,0x7f,0x80,0x7ff,0x800,0xd7ff,0xe000,0xffff,0x10000,0x1ffff,0x20000,0x10ffff}
points={x for x in points if not 0xd800<=x<=0xdfff}
cases=[b'']+[chr(x).encode() for x in sorted(points)]
cases += [b'\xff',b'\x80',b'\xc0\x80',b'\xe0\x80\x80',b'\xed\xa0\x80',b'\xf4\x90\x80\x80',b'\xf0\x90',b'\xc2x']
def scalar_oracle(source,allowed):
 try:text=source.decode('utf8')
 except UnicodeDecodeError:return None
 if any(not allowed(ord(c)) for c in text):return None
 tokens=[];at=0
 for c in text:
  size=len(c.encode());tokens.append((1,at,size));at+=size
 return tokens+[(0,len(source),0)]
qualify('all-scalars',header+'''  all: charset = scalar
  value.rule: rule = all then emit value
'''+footer,cases,lambda s:scalar_oracle(s,lambda x:True))
qualify('subtracted-scalars',header+'''  letters: charset = U+0041 to U+005A
  selected: charset = (scalar except letters) except (U+10000 to U+1FFFF)
  value.rule: rule = selected then emit value
'''+footer,cases,lambda s:scalar_oracle(s,lambda x:not(0x41<=x<=0x5a or 0x10000<=x<=0x1ffff)))
def bounded_oracle(source):
 pattern=re.compile(rb'(?:\xce\xb1){2,3}(?:\xf0\x9f\x98\x80)?')
 tokens=[];at=0
 while at<len(source):
  match=pattern.match(source,at)
  if not match:return None
  tokens.append((1,at,match.end()-at));at=match.end()
 return tokens+[(0,len(source),0)]
qualify('bounded-unicode',header+"""  pair: pattern = U+03B1{2,3}
  value.rule: rule = pair U+1F600? then emit value
"""+footer,[s.encode() for s in ['', 'α','αα','ααα','αααα','αα😀','ααα😀','αα😀αα','😀','ααβ']],bounded_oracle)
# The owning ICU rule-file oracle operates on decoded scalars and re-encodes spans.
gennorm=(root/'lib/rxfnsg/unicode/gennorm2.levell').read_text().replace('namespace gennorm2_language','namespace probe_language').replace('invalid_utf8','bad_utf8').replace('invalid_character','bad_character')
def gennorm_oracle(source):
 try:text=source.decode('utf8')
 except UnicodeDecodeError:return None
 pats=[(-1,r'[\t ]+'),(-1,r'#[^\x00\r\n]*'),(7,r'Unicode'),(8,r'[0-9]+(?:\.[0-9]+)+'),(1,r'[0-9A-F]+'),(2,r'\.\.'),(3,':'),(4,'='),(5,'>'),(6,r'\*'),(9,r'\r?\n')]
 at=0;byte=0;tokens=[]
 while at<len(text):
  matched=[(m.end(),-i,kind) for i,(kind,pat) in enumerate(pats) if (m:=re.compile(pat).match(text,at))]
  if not matched:return None
  end,_,kind=max(matched);size=len(text[at:end].encode())
  if kind>=0:tokens.append((kind,byte,size))
  at=end;byte+=size
 return tokens+[(0,len(source),0)]
gcases=[b'',b'Unicode 17.0.0\n0041..005A:230\n00C0=0041 0300\n',b'0061>0041\n*\n', '# café 😀\r\n0041 = 0042 # Ελληνικά\n'.encode(),b'\x00',b'#ok\x00',b'\r',b'ABC.',b'\xff',b'Unicode 17.0\n#tail']
for _ in range(70):
 c=random.choice(['# 😀 comment\n','0041=0042\n','00C0>0041 0300\n','Unicode 17.0.0\n','0041..005A:230\n'])
 gcases.append(c.encode());gcases.append(c[:-1].encode())
qualify('gennorm2',gennorm,gcases,gennorm_oracle)
tiny=(root/'langtools/examples/tinyexpr/tinyexpr.levell').read_text().replace('namespace level_l_tinyexpr','namespace probe_language').replace('invalid_utf8','bad_utf8').replace('invalid_character','bad_character')
def tiny_oracle(source):
 try:source.decode('utf8')
 except UnicodeDecodeError:return None
 patterns=[(-1,rb'[ \t\r\n]+'),(1,rb'[0-9]+'),(2,rb'[A-Za-z_][A-Za-z_0-9]*')]+[(i+3,re.escape(bytes([c]))) for i,c in enumerate(b'+-*/()')]
 at=0;tokens=[]
 while at<len(source):
  hits=[(m.end(),-i,kind) for i,(kind,pat) in enumerate(patterns) if (m:=re.compile(pat).match(source,at))]
  if not hits:return None
  end,_,kind=max(hits)
  if kind>=0:tokens.append((kind,at,end-at))
  at=end
 return tokens+[(0,len(source),0)]
qualify('tinyexpr',tiny,[b'',b'12 + alpha_2*(9-3)/4']+[bytes([i]) for i in range(256)],tiny_oracle)
(out/'receipt.json').write_text(json.dumps(records,indent=2)+'\n')
print('bootstrap_panels='+str(len(records))+'; cases='+str(sum(r['inputs'] for r in records))+'; cells=4; failures=0')
