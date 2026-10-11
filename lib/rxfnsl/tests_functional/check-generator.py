#!/usr/bin/env python3
"""Deterministic product artifacts and clear, atomic build failures."""
import argparse
from pathlib import Path
import hashlib
import json
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('bin', type=Path)
parser.add_argument('work', type=Path)
args = parser.parse_args()
bin_dir, work = args.bin.resolve(), args.work.resolve()
work.mkdir(parents=True, exist_ok=True)
root = Path(__file__).resolve().parents[3]
fixtures = Path(__file__).with_name('grammars')
results = []

def invoke(spec, *options):
    result = subprocess.run([str(bin_dir / 'levell'), str(spec), *options], capture_output=True, timeout=600)
    return result

for spec in [root / 'lib/rxfnsl/tools/example.levell', root / 'rexxscript/grammar/rexxscript.levell']:
    first, second = invoke(spec, '--rxas'), invoke(spec, '--rxas')
    assert first.returncode == second.returncode == 0, first.stdout[-4000:]
    assert first.stdout == second.stdout, 'nondeterministic artifact'
    (work / (spec.stem + '.rxas')).write_bytes(first.stdout)
    results.append({'case': spec.name, 'sha256': hashlib.sha256(first.stdout).hexdigest(), 'deterministic': True})

for name, message in [('nullable','LL_NULLABLE_TOKEN'), ('recursive-lexer','LL_LEX_RECURSION'), ('conflict','LL_LR_CONFLICT'), ('ordered','LL_BACKEND_CAPABILITY')]:
    spec = fixtures / (name + '.levell')
    result = invoke(spec, '--rxas')
    assert result.returncode and message.encode() in result.stdout, (name, result.stdout, result.stderr)
    (work / (name + '.out')).write_bytes(result.stdout + result.stderr)
    artifact = work / (name + '.rxas')
    artifact.write_bytes(b'previous valid artifact\n')
    atomic = subprocess.run(['cmake', f'-DLEVEL_L={bin_dir / "levell"}', f'-DSPEC={spec}', f'-DOUTPUT={artifact}', '-DFORMAT=rxas', '-P', str(root / 'lib/rxfnsl/tools/generate.cmake')], capture_output=True, timeout=600)
    assert atomic.returncode and artifact.read_bytes() == b'previous valid artifact\n'
    assert not Path(str(artifact) + '.tmp').exists()
    results.append({'case':name, 'diagnostic':message, 'nonzero_exit':True, 'previous_artifact_preserved':True})

limit_spec = work / 'limit.levell'
limit_spec.write_text('options levell\nnamespace limit\na: token = 1\nscan: lexer input bytes\n  huge: rule = "' + 'a' * 5000 + '" then emit a\nparse: parser semantics contextfree input tokens start root\n  root: rule = a\n')
limit = invoke(limit_spec, '--rxas')
assert limit.returncode and (b'limit' in limit.stdout.lower() or b'budget' in limit.stdout.lower()), limit.stdout
(work / 'limit.out').write_bytes(limit.stdout + limit.stderr)
results.append({'case':'construction exhaustion', 'nonzero_exit':True, 'message':limit.stdout.decode().strip()})
invalid = work / 'invalid-utf8.levell'; invalid.write_bytes(b'\xff')
for name, spec, message in [('UTF8', invalid, b'LL_UTF8'), ('missing file', work / 'absent.levell', b'LL_SOURCE_IO')]:
    result = invoke(spec)
    assert result.returncode and message in result.stdout, result.stdout
    results.append({'case':name, 'nonzero_exit':True})
(work / 'receipt.json').write_text(json.dumps(results, indent=2) + '\n')
print(f'generator_cases={len(results)}; failures=0')
