#!/usr/bin/env python3
"""New comment surface agrees through seed/proper stages and fails atomically."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from _scratch import scratch

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[2]
build, out = a.build.resolve(), scratch()
binary, stages = build / 'bin', build / 'langtools/bootstrap'
out.mkdir(parents=True, exist_ok=True)
records = []
text = '''options levell
namespace comments_stages
word: token
hash: comments line '#' outside quoted.text
plain: comments block '/*' '*/' outside quoted.text newlines keep
nested: comments nested '(*' '*)' outside quoted.text
scan: lexer input utf8
  body: charset = scalar except '"'
  quoted.text: pattern = '"' body* '"'
  word.lex: rule = quoted.text then emit word
  whitespace: rule = (' ' | U+000A | U+000D)+ then skip
parse: parser start root
  root: rule = word*
'''
spec = out / 'valid.levell'
spec.write_text(text)
observed = []
for stage in range(3):
    for vm in ['rxtvm', 'rxbvm']:
        argv = [binary / vm, stages / f'L{stage}/rxlang-linked.rxbin', '-a', spec, '--rxas']
        result = subprocess.run(list(map(str, argv)), capture_output=True, timeout=1200)
        data = result.stdout + result.stderr
        key = f'L{stage}-{vm}'
        (out / (key + '.log')).write_bytes(data)
        assert result.returncode == 0 and b'Warning in ' not in data, (key, data[-2000:])
        observed.append(result.stdout)
        records.append(dict(key=key, argv=list(map(str, argv)), sha256=hashlib.sha256(data).hexdigest()))
assert len(set(observed)) == 1, 'Stage/VM changed comment metadata or images'
output = out / 'last-good.rxas'
output.write_bytes(observed[0])
before = hashlib.sha256(output.read_bytes()).hexdigest()
invalid = out / 'duplicate.levell'
invalid.write_text(text + "\nduplicate: comments nested '#' '!'\n")
argv = ['cmake', '-DLEVEL_L=' + str(binary / 'rxlang'), '-DSPEC=' + str(invalid),
        '-DOUTPUT=' + str(output), '-DFORMAT=rxas', '-P', root / 'langtools/cmake/generate.cmake']
result = subprocess.run(list(map(str, argv)), capture_output=True, timeout=1200)
data = result.stdout + result.stderr
(out / 'atomic-failure.log').write_bytes(data)
assert result.returncode != 0 and b'LL_COMMENT_CONTRACT' in data and b'duplicate opener' in data
assert hashlib.sha256(output.read_bytes()).hexdigest() == before
assert not output.with_suffix(output.suffix + '.tmp').exists()
(out / 'receipt.json').write_text(json.dumps(dict(records=records, stage_vm_cells=6,
    atomic_preservation=True, artifact_sha256=before), indent=2) + '\n')
print('comment_stage_cells=6; atomic_preservation=true; failures=0')
