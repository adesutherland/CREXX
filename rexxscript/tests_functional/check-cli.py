#!/usr/bin/env python3
"""Exercise the built standalone runner, its shared providers and failure exits."""
import argparse
import json
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('bin', type=Path)
parser.add_argument('work', type=Path)
args = parser.parse_args()
binary = args.bin.resolve() / 'rexxscript1'
work = args.work.resolve()
work.mkdir(parents=True, exist_ok=True)
root = Path(__file__).resolve().parents[2]
cases = [
    ('example', root / 'rexxscript/examples/modern.rexx', 0, 'total: 46\ncafé\nok\n'),
    ('missing-file', work / 'absent.rexx', 1, 'RS_SOURCE_IO'),
]
for name, source, message in [
    ('parse', 'say "must not execute"; x=(1+', 'RS_PARSE'),
    ('capability', 'say linein("absent")', 'RS_CAPABILITY'),
    ('undefined', 'say missing', 'RS_UNDEFINED'),
]:
    path = work / (name + '.rexx')
    path.write_text(source)
    cases.append((name, path, 1, message))
bad_utf8 = work / 'utf8.rexx'
bad_utf8.write_bytes(b'say "\xff"')
cases.append(('utf8', bad_utf8, 1, 'source bytes:'))
results = []
for name, path, expected_status, expected in cases:
    run = subprocess.run([str(binary), str(path)], capture_output=True, text=True, timeout=600)
    text = run.stdout + run.stderr
    (work / (name + '.out')).write_text(text)
    assert run.returncode == expected_status, (name, run.returncode, text)
    assert expected in text, (name, text)
    if name == 'example':
        assert text == expected
    if name == 'parse':
        # A diagnostic may quote the source, but no SAY line can be executed.
        assert 'must not execute' not in text.splitlines()
        assert 'line 1, column' in text and '^' in text
    results.append({'case': name, 'exit': run.returncode, 'pass': True})
(work / 'receipt.json').write_text(json.dumps(results, indent=2) + '\n')
print(f'cli_cases={len(results)}; failures=0')
