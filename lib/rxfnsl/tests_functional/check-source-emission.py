#!/usr/bin/env python3
"""Verify source emission against the independently checked RXAS observations."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('build', type=Path)
parser.add_argument('controls', type=Path)
parser.add_argument('output', type=Path)
args = parser.parse_args()
root = Path(__file__).resolve().parents[3]
build = args.build.resolve()
controls = args.controls.resolve()
output = args.output.resolve()
output.mkdir(parents=True, exist_ok=True)
bin_dir = build / 'bin'
stage = build / 'lib/rxfnsl/tools/bootstrap/L2/levell-linked.rxbin'
records = []
commands = []


def run(name, command):
    command = [str(x) for x in command]
    result = subprocess.run(command, capture_output=True, timeout=1200)
    (output / (name + '.log')).write_bytes(result.stdout + result.stderr)
    assert result.returncode == 0, (name, result.returncode, result.stderr[-2000:])
    assert b'Warning in ' not in result.stdout + result.stderr, name
    commands.append(command)
    return result.stdout


reference = {row['case']: row for row in json.loads((controls / 'receipt.json').read_text())}
for name in ['bounded-unicode', 'gennorm2']:
    work = output / name
    work.mkdir(exist_ok=True)
    source = run(name + '-emit', [bin_dir / 'rxtvm', stage, '-a', controls / name / 'probe.levell'])
    (work / 'probe_language.crexx').write_bytes(source)
    for mode in ['opt', 'noopt']:
        flags = ['-n'] if mode == 'noopt' else []
        for module, path in [('probe_language', work / 'probe_language.crexx'),
                             ('probe', root / 'lib/rxfnsl/tests_functional/bootstrap-probe.crexx')]:
            run(name + '-' + mode + '-' + module + '-compile', [bin_dir / 'rxc', *flags,
                '-x', '--no-exe-import', '-i', f'{work};{bin_dir}', '-o', work / module, path])
            run(name + '-' + mode + '-' + module + '-assemble', [bin_dir / 'rxas', *flags, work / module])
        run(name + '-' + mode + '-link', [bin_dir / 'rxlink', '-s', '-o', work / 'linked',
            work / 'probe.rxbin', work / 'probe_language.rxbin', bin_dir / 'rxfnsl.rxbin', bin_dir / 'library.rxbin'])
        for vm in ['rxtvm', 'rxbvm']:
            observed = run(name + '-' + mode + '-' + vm,
                [bin_dir / vm, work / 'linked.rxbin', '-a', controls / name / 'cases.bin'])
            digest = hashlib.sha256(observed).hexdigest()
            assert digest == reference[name]['output_sha256'], (name, mode, vm, digest)
    records.append(dict(case=name, inputs=reference[name]['inputs'], cells=4,
        source_sha256=hashlib.sha256(source).hexdigest(),
        output_sha256=reference[name]['output_sha256']))
(output / 'receipt.json').write_text(json.dumps(dict(records=records, commands=commands), indent=2) + '\n')
print(f'source_emission_panels={len(records)}; cases={sum(x["inputs"] for x in records)}; cells=4; failures=0')
