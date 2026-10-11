#!/usr/bin/env python3
"""Flat-record recovery and proper-reader declaration resynchronization."""
import argparse
import json
import subprocess
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
p.add_argument('--backends', nargs='+', default=['lr1', 'll1', 'packrat', 'earley'])
a = p.parse_args()
root = Path(__file__).resolve().parents[3]
build = a.build.resolve()
bin_dir = build / 'bin'
out = a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
commands = []
records = []


def run(name, args, expected=0):
    args = list(map(str, args))
    r = subprocess.run(args, capture_output=True, timeout=1200)
    (out / (name + '.log')).write_bytes(r.stdout + r.stderr)
    commands.append(dict(name=name, argv=args, status=r.returncode))
    assert r.returncode == expected, (name, r.returncode, r.stdout[-2500:], r.stderr[-1000:])
    assert b'Warning in ' not in r.stdout + r.stderr, (name, r.stdout, r.stderr)
    return r.stdout


for backend in a.backends:
    work = out / backend
    work.mkdir(exist_ok=True)
    spec = (root / 'lib/rxfnsl/examples/settings/04-recovery.levell').read_text()
    spec = spec.replace('parser start settings', 'parser start settings backend ' + backend
                        + (' semantics ordered' if backend == 'packrat' else ''))
    path = work / 'input.levell'
    path.write_text(spec)
    for route in ['rxas', 'source']:
        key = backend + '-' + route
        args = [bin_dir / 'levell', path] + (['--rxas'] if route == 'rxas' else [])
        emitted = run(key + '-generate', args)
        generated = work / ('levell_settings.' + ('rxas' if route == 'rxas' else 'crexx'))
        generated.write_bytes(emitted)
        if route == 'source':
            run(key + '-compile-language', [bin_dir / 'rxc', '-x', '--no-exe-import', '-i', f'{work};{bin_dir}',
                '-o', work / 'levell_settings', generated])
        run(key + '-assemble-language', [bin_dir / 'rxas', work / 'levell_settings'])
        run(key + '-compile-reader', [bin_dir / 'rxc', '-x', '--no-exe-import', '-i', f'{work};{bin_dir}',
            '-o', work / 'reader', root / 'lib/rxfnsl/examples/settings-recover.crexx'])
        run(key + '-assemble-reader', [bin_dir / 'rxas', work / 'reader'])
        run(key + '-link', [bin_dir / 'rxlink', '-s', '-o', work / 'linked', work / 'reader.rxbin',
            work / 'levell_settings.rxbin', bin_dir / 'rxfnsl.rxbin', bin_dir / 'library.rxbin'])
        cases = {
            'clean': (b'first = 1\nthird = 3\n', 0, b'errors: 0', []),
            'two-errors': (b'first = many\nsecond =\nthird = 3\n', 1, b'errors: 2', []),
            'lexical-damage': (b'first = $\nsecond = many\nthird = 3\n', 1, b'errors: 2', []),
            'quoted-sync': (b'first = "a;b"\nsecond = many\nthird = 3\n', 1, b'first -> "a;b"', []),
            'eof': (b'third = 3\nlast =', 1, b'end of input', []),
            'bounded': (b'first = many\nsecond =\nthird = 3\n', 2, b'LL_RECOVERY_LIMIT', ['1']),
            'utf8-damage': (b'first = \xff\nthird = 3', 4, b'LL_UTF8', []),
        }
        for vm in ['rxtvm', 'rxbvm']:
            for case, (source, status, expected, extra) in cases.items():
                target = work / (case + '.settings')
                target.write_bytes(source)
                observed = run(key + '-' + vm + '-' + case,
                    [bin_dir / vm, work / 'linked.rxbin', '-a', target, *extra], status)
                assert expected in observed, (key, case, observed)
                if status < 2:
                    assert b'third -> 3' in observed, (key, case, observed)
                if case == 'two-errors':
                    assert observed.count(b'value.error:') == 2, (key, case, observed)
                    assert b'first ->' not in observed and b'second ->' not in observed
                records.append(dict(backend=backend, route=route, vm=vm, case=case, status=status))
                (out / 'progress.json').write_text(json.dumps(dict(records=records, commands=commands), indent=2) + '\n')

# The same damaged specification yields multiple explanations in proper readers.
broken = '''options levell
namespace broken
scan: lexer
item: token
parse: parser start first
first: rule = (item
second: rule = )
last.good: rule = item
'''
path = out / 'broken.levell'
path.write_text(broken)
for stage in [1, 2]:
    tool = build / f'lib/rxfnsl/tools/bootstrap/L{stage}/levell-linked.rxbin'
    result = run('spec-errors-L' + str(stage), [bin_dir / 'rxtvm', tool, '-a', path, '--rxas'], 1)
    assert result.count(b'LL_SYNTAX at line') == 2, result
    assert b'.globals=' not in result, result

(out / 'receipt.json').write_text(json.dumps(dict(records=records, commands=commands,
    boundary='Flat records; strict UTF-8/comment preparation; no nested or token-stage recovery claim.'), indent=2) + '\n')
print(f'recovery_cases={len(records)}; specification_resync=2; failures=0')
