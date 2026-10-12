from pathlib import Path
import re, urllib.parse, json, hashlib, subprocess
root=Path.cwd(); p=root/'lib/rxfnsl/doc/user-guide.md'; s=p.read_text()
work=root/'research/level-l/build/manual-review'; binary=root/'cmake-build-levell-product/bin'
def anchors(text):
    values=set(); counts={}
    text=re.sub(r'```.*?```','',text,flags=re.S)
    for title in re.findall(r'^#{1,6}\s+(.+?)\s*$',text,re.M):
        a=re.sub(r'[^\w\- ]','',title.replace('`','').lower()).replace(' ','-')
        n=counts.get(a,0); counts[a]=n+1
        values.add(a if n==0 else a+'-'+str(n))
    return values
links=re.findall(r'(?<!!)\[[^\]]+\]\(([^)]+)\)',s)
errors=[]
for link in links:
    if '://' in link: continue
    name,sep,fragment=urllib.parse.unquote(link).partition('#')
    target=(p.parent/name).resolve() if name else p
    if not target.exists(): errors.append('missing '+link)
    elif sep and target.suffix=='.md' and fragment not in anchors(target.read_text()): errors.append('anchor '+link)
assert s.count('```')%2==0
blocks=re.findall(r'```rexx\n(.*?)\n```',s,re.S)
full=[b for b in blocks if b.startswith('options levell') and 'scan: lexer' in b]
base=(root/'lib/rxfnsl/examples/settings/01-basic.levell').read_text()
base=re.sub(r'/\*.*?\*/\s*','',base,flags=re.S).strip()
assert full == [base], 'basic example differs from checked-in source'
assert not errors, errors
result=dict(links=len(links),broken=errors,code_fences=s.count('```')//2,full_specification_matches_example=True)
records=[]
def run(name,argv,status=0,expected=None):
    r=subprocess.run([str(x) for x in argv],capture_output=True,cwd=root,timeout=300)
    data=r.stdout+r.stderr; (work/(name+'.log')).write_bytes(data)
    assert r.returncode==status,(name,r.returncode,data[-2500:])
    assert not re.search(rb'Warning in |\bWARNING\b',data),(name,data)
    if expected is not None: assert r.stdout.decode()==expected,(name,r.stdout)
    records.append(dict(name=name,argv=list(map(str,argv)),status=r.returncode,stdout=r.stdout.decode(),stderr=r.stderr.decode()))
    return r.stdout.decode()
for name,image,inputfile,expected in [
    ('settings','settings/settings-reader-linked',root/'lib/rxfnsl/examples/settings.txt','retries -> 3\ngreeting -> "café"\nnote -> "say ""hello"""\n'),
    ('calculator','calculator/calculator-linked',root/'lib/rxfnsl/examples/calculator/example.calc','RESULT|11\n'),
    ('backend','backend/inspect-linked',root/'cmake-build-levell-product/examples/backend/input.txt','STATUS|0\nNODE|value|0|4\nNODE|boolean|0|4\nNODE|trueword|0|4\n')]:
    run(name+'-rxbvm',[binary/'rxbvm',root/'cmake-build-levell-product/examples'/(image+'.rxbin'),'-a',inputfile],expected=expected)
# Confirm that the printed limits cover every name/default in the manual.
limits=run('limits',[binary/'levell','--limits'])
rows=dict(re.findall(r'^\| `([^`]+)` \| ([\d,]+) \|',s,re.M))
for name,value in re.findall(r'^(\S+?)=(\d+) ',limits,re.M):
    assert rows[name].replace(',','')==value,(name,rows.get(name),value)
result['limit_defaults_checked']=len(limits.splitlines())
# Check multiline strings and missing-value diagnostics using the documented reader.
source=work/'multiline.settings'; source.write_text('note = "first\nsecond"\n')
run('settings-multiline',[binary/'rxtvm',root/'cmake-build-levell-product/examples/settings/settings-reader-linked.rxbin','-a',source],expected='note -> "first\nsecond"\n')
source=work/'missing-value.settings'; source.write_text('first =')
output=run('settings-missing-value',[binary/'rxtvm',root/'cmake-build-levell-product/examples/settings/settings-reader-linked.rxbin','-a',source],1)
assert 'line 1, column 8 (byte 7)' in output and 'expected number, string' in output
result['runtime_checks']=records
(work/'document-check.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps({k:v for k,v in result.items() if k!='runtime_checks'}))
print('runtime_checks='+str(len(records))+'; failures=0')
