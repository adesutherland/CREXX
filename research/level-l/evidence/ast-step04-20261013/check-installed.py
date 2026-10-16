"""Explicit AST-STEP04 installed-tool panel: BIN OUTPUT."""
from pathlib import Path
import argparse, hashlib, json, os, re, subprocess
p = argparse.ArgumentParser(); p.add_argument('binary', type=Path); p.add_argument('output', type=Path)
a = p.parse_args(); binary = a.binary.resolve(); out = a.output.resolve(); out.mkdir(parents=True, exist_ok=True)
root = Path(__file__).resolve().parents[4]; records = []
def run(name, args, required=()):
    result = subprocess.run(list(map(str, args)), capture_output=True, timeout=1200,
                            env=dict(os.environ, CREXX_HOME=str(binary.parent)))
    data = result.stdout + result.stderr; (out / (name + '.log')).write_bytes(data)
    assert result.returncode == 0 and not re.search(rb'Warning in |\bWARNING\b', data), (name, result.returncode, data[-3000:])
    assert all(x.encode() in data for x in required), (name, data, required)
    records.append(dict(name=name, command=list(map(str, args)), status=result.returncode,
                        output=data.decode(errors='replace'), sha256=hashlib.sha256(data).hexdigest()))
example = root / 'rexxscript/examples/ast-analysis.crexx'
run('analysis-build', [binary/'crexx','--program',out/'ast-analysis',example])
for vm in ['rxtvm','rxbvm']:
    run('analysis-'+vm, [binary/vm,out/'ast-analysis.rxbin'], ['Constant expression: 1 value: 14','Variables read: X','Replacement: Number 14','Original source: 2+3*4','\n14\n'])
run('analysis-noop', [binary/'crexx','--program',out/'ast-analysis',example], ['SKIP: project current'])
source = out / 'script.rexx'; source.write_text('x=2+3*4; if x>10 then say x else say 1/0\n')
run('native-script', [binary/'rexxscript1',source], ['14\n'])
examples = root/'lib/rxfnsl/examples/calculator'
run('calculator-build', [binary/'crexx','--program',out/'calculator',examples/'calculator.crexx',examples/'calculator.rxl'])
for vm in ['rxtvm','rxbvm']:
    run('calculator-'+vm, [binary/vm,out/'calculator.rxbin','-a',examples/'example.calc'], ['RESULT|11'])
(out/'receipt.json').write_text(json.dumps(dict(commands=records),indent=2)+'\n')
print('PASS:',len(records),'installed commands')
