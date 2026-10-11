#!/usr/bin/env python3
"""Generated frontend grammar/line controls, stage admission and fixed point."""
import argparse,hashlib,json,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('build',type=Path);p.add_argument('output',type=Path);a=p.parse_args()
root=Path(__file__).resolve().parents[3];build=a.build.resolve();bin=build/'bin';stages=build/'lib/rxfnsl/tools/bootstrap';out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);records=[]
source=(root/'lib/rxfnsl/examples/settings.levell').read_text()
valid={'settings':source,'settings-unicode-comment':source+'\n/* 😀 café */\n','lexer-only':(root/'lib/rxfnsl/grammar/gennorm2.levell').read_text(),'parser-only':"options levell\nnamespace probe\nitem: token=1\nparse: parser semantics contextfree input tokens start root\nroot: rule = item\n"}
invalid={'undefined':source.replace('value: rule = number | string','value: rule = missing'), 'duplicate':source+'\nidentifier: token = 17\n','unclosed-group':source.replace('value: rule = number | string','value: rule = (number | string'), 'unclosed-quote':source+"\nbad: diagnostic = 'missing\n",'bad-codepoint':source.replace('U+0030','U+110000'),'charset-cycle':source.replace('digit: charset = U+0030 to U+0039','digit: charset = digit'), 'inverted-range':source.replace('U+0030 to U+0039','U+0039 to U+0030'),'duplicate-id':source.replace('number: token = 2','number: token = 1'),'empty-lexical':source.replace('space+ then skip','space* then skip'),'reserved-eof':source.replace('number.rule: rule = digit+ then emit number','number.rule: rule = eof then skip')}
for name,text in {**valid,**invalid}.items():
 path=out/(name+'.levell');path.write_text(text);results=[]
 for stage in range(3):
  mode='--graph' if name=='parser-only' else '--rxas'
  r=subprocess.run([str(bin/'rxtvm'),str(stages/f'L{stage}/levell-linked.rxbin'),'-a',str(path),mode],capture_output=True,timeout=600)
  (out/f'{name}-L{stage}.log').write_bytes(r.stdout+r.stderr)
  assert (r.returncode==0)==(name in valid),(name,stage,r.returncode,r.stdout[-1200:])
  results.append(r.stdout)
 if name in valid:assert len(set(results))==1,(name,'stage artifact mismatch')
 records.append({'case':name,'stages':3,'accepted':name in valid})
for ending in ['\n','\r','\r\n']:
 text=invalid['undefined'].replace('\n',ending);path=out/('lines-'+str(len(records))+'.levell');path.write_bytes(text.encode())
 expected_line=1+invalid['undefined'][:invalid['undefined'].index('value: rule = missing')].count('\n')
 for stage in range(3):
  r=subprocess.run([str(bin/'rxtvm'),str(stages/f'L{stage}/levell-linked.rxbin'),'-a',str(path),'--rxas'],capture_output=True,timeout=600)
  assert r.returncode and f'line {expected_line},'.encode() in r.stdout,(repr(ending),stage,r.stdout)
  (out/f'lines-{len(records)}-L{stage}.log').write_bytes(r.stdout+r.stderr)
 records.append({'case':'line-endings-'+repr(ending),'stages':3,'line':expected_line})
# Same generated text across optimized/unoptimized full tools, not just within one bootstrap.
noopt=root/'research/level-l/build/product-noopt/bootstrap-build/tools/bootstrap/L2/llsyntax.rxas'
assert noopt.read_bytes()==(stages/'L2/llsyntax.rxas').read_bytes(),'Optimization changed self-description artifact'
(out/'receipt.json').write_text(json.dumps({'cases':records,'self_description_sha256':hashlib.sha256(noopt.read_bytes()).hexdigest(),'opt_noopt_fixed_point_agrees':True},indent=2)+'\n')
print('frontend_cases='+str(len(records))+'; stages=3; failures=0')
