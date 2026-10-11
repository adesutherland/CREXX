#!/usr/bin/env python3
"""Product surface, named results, diagnostics and artifact-route parity.

This is a focused manual panel, not a newly registered nested CTest aggregate.
The caller supplies an already built product; every command and result is kept.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
p.add_argument('--backends', nargs='+', default=['lr1', 'll1', 'packrat', 'earley'])
p.add_argument('--negative-only', action='store_true', help='Retain progress.json and rerun only specification checks')
a = p.parse_args()
root = Path(__file__).resolve().parents[3]
build = a.build.resolve()
bin_dir = build / 'bin'
out = a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
examples = root / 'lib/rxfnsl/examples/settings'
commands = []
records = []
if a.negative_only:
    retained = json.loads((out / 'progress.json').read_text())
    records = retained['records']
    commands = retained['commands']


def run(name, args, expected=0):
    args = list(map(str, args))
    result = subprocess.run(args, capture_output=True, timeout=1200)
    log = result.stdout + result.stderr
    (out / (name + '.log')).write_bytes(log)
    commands.append(dict(name=name, argv=args, returncode=result.returncode))
    assert result.returncode == expected, (name, result.returncode, log[-2500:])
    assert b'Warning in ' not in log, (name, log[-2500:])
    return result.stdout


for backend in ([] if a.negative_only else a.backends):
    for lesson in ['01-basic', '02-messages', '03-unicode']:
        work = out / (backend + '-' + lesson)
        work.mkdir(exist_ok=True)
        spec = (examples / (lesson + '.levell')).read_text()
        spec = spec.replace('parser start settings', 'parser start settings backend ' + backend
                            + (' semantics ordered' if backend == 'packrat' else ''))
        path = work / 'settings.levell'
        path.write_text(spec)
        for route in ['rxas', 'source']:
            name = backend + '-' + lesson + '-' + route
            args = [bin_dir / 'levell', path]
            if route == 'rxas':
                args.append('--rxas')
            generated = run(name + '-generate', args)
            generated_path = work / ('levell_settings.' + ('rxas' if route == 'rxas' else 'crexx'))
            generated_path.write_bytes(generated)
            for mode in ['opt', 'noopt']:
                cell = name + '-' + mode
                flags = ['-n'] if mode == 'noopt' else []
                if route == 'source':
                    run(cell + '-language-compile', [bin_dir / 'rxc', *flags, '-x', '--no-exe-import',
                        '-i', f'{work};{bin_dir}', '-o', work / 'levell_settings', generated_path])
                # Source compilation overwrites RXAS; each route is generated independently.
                run(cell + '-language-assemble', [bin_dir / 'rxas', *flags, work / 'levell_settings'])
                run(cell + '-reader-compile', [bin_dir / 'rxc', *flags, '-x', '--no-exe-import',
                    '-i', f'{work};{bin_dir}', '-o', work / 'reader',
                    root / 'lib/rxfnsl/examples/settings-reader.crexx'])
                run(cell + '-reader-assemble', [bin_dir / 'rxas', *flags, work / 'reader'])
                run(cell + '-link', [bin_dir / 'rxlink', '-s', '-o', work / 'linked',
                    work / 'reader.rxbin', work / 'levell_settings.rxbin',
                    bin_dir / 'rxfnsl.rxbin', bin_dir / 'library.rxbin'])
                cases = {'good': (b'workers = 4\ngreeting = "Hello, \xce\xba\xcf\x8c\xcf\x83\xce\xbc\xce\xb5 \xf0\x9f\x98\x80"\n', 0),
                         'bad-value': (b'workers = many\n', 1),
                         'missing-value': (b'workers =', 1),
                         'after-value': (b'workers = 4 many', 1),
                         'bad-key': (b'4 = 4', 1)}
                if lesson == '03-unicode':
                    cases['unicode-names'] = ((examples / 'unicode.settings').read_bytes(), 0)
                for vm in ['rxtvm', 'rxbvm']:
                    for case, (source, status) in cases.items():
                        target = work / (case + '.settings')
                        target.write_bytes(source)
                        observed = run(cell + '-' + vm + '-' + case,
                            [bin_dir / vm, work / 'linked.rxbin', '-a', target], status)
                        if case == 'good':
                            assert observed == 'workers -> 4\ngreeting -> "Hello, κόσμε 😀"\n'.encode(), observed
                        if case == 'unicode-names':
                            assert observed == 'café -> 1\ncafe\u0301 -> 2\n温度 -> 23\nΔ -> 4\n𐐀 -> "😀"\n'.encode(), observed
                        if case in ['bad-value', 'missing-value']:
                            expected = b'Expected a number or quoted string; found ' if lesson != '01-basic' else b'expected number, string'
                            assert expected in observed, (cell, case, observed)
                        if case in ['after-value', 'bad-key']:
                            assert b'value.error' not in observed, (cell, case, observed)
                        records.append(dict(cell=cell, vm=vm, case=case, status=status,
                            source_sha256=hashlib.sha256(source).hexdigest(),
                            result_sha256=hashlib.sha256(observed).hexdigest()))
                        (out / 'progress.json').write_text(json.dumps(dict(records=records, commands=commands), indent=2) + '\n')

base = (examples / '02-messages.levell').read_text()
negative = {
    'unknown-field': (base.replace('{found}', '{invented}'), b'unknown diagnostic field'),
    'unclosed-field': (base.replace('{found}', '{found'), b'unclosed diagnostic field'),
    'unknown-rule': (base.replace('diagnostic rule value', 'diagnostic rule missing'), b'unknown diagnostic rule'),
    'token-literal': (base.replace('name.value equals value', "name.value '=' value"), b'quoted spelling'),
    'unknown-backend': (base.replace('parser start settings', 'parser start settings backend mystery'), b'backend'),
    'profile-mismatch': (base.replace('parser start settings', 'parser start settings backend packrat'), b'CFG alternatives require a CFG engine'),
    'duplicate-option': (base.replace('parser start settings', 'parser start settings input tokens input tokens'), b'duplicate'),
}
for case, (text, expected) in negative.items():
    path = out / (case + '.levell')
    path.write_text(text)
    for stage in range(3):
        tool = build / f'lib/rxfnsl/tools/bootstrap/L{stage}/levell-linked.rxbin'
        result = run(case + f'-L{stage}', [bin_dir / 'rxtvm', tool, '-a', path, '--rxas'], 1)
        assert expected in result, (case, stage, result)

stages = build / 'lib/rxfnsl/tools/bootstrap'
assert (stages / 'L1/llsyntax.rxas').read_bytes() == (stages / 'L2/llsyntax.rxas').read_bytes()
(out / 'receipt.json').write_text(json.dumps(dict(records=records, commands=commands,
    boundary='Native host; optimized product runtime. Opt/noopt generated artifacts and consumers; both VMs.',
    fixed_point_sha256=hashlib.sha256((stages / 'L2/llsyntax.rxas').read_bytes()).hexdigest()), indent=2) + '\n')
print(f'surface_cells={len(records)}; negative_stage_cases={len(negative)*3}; failures=0')
