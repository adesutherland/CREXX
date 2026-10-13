#!/usr/bin/env python3
"""Compare the same scalar source with compiled Level G and RexxScript 1.
The caller supplies a product bin and an isolated directory for all outputs.
"""
import argparse
import json
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('bin', type=Path)
parser.add_argument('work', type=Path)
args = parser.parse_args()
bin_dir, work = args.bin.resolve(), args.work.resolve()
work.mkdir(parents=True, exist_ok=True)
source = Path(__file__).with_name('scalar-conformance.rexx').resolve()
(work / 'oracle.crexx').write_text('options levelg numeric_common\n' + source.read_text())

def run(*argv):
    result = subprocess.run([str(x) for x in argv], capture_output=True, text=True, timeout=600)
    with (work / 'commands.log').open('a') as log:
        log.write(' '.join(map(str, argv)) + '\n' + result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError(f'{argv[0]} exited {result.returncode}; see {work / "commands.log"}')
    return result.stdout

observations = {}
for mode in ('opt', 'noopt'):
    flags = ['-n'] if mode == 'noopt' else []
    out = work / ('oracle-' + mode)
    run(bin_dir / 'rxc', '-x', '--no-exe-import', *flags, '-i', bin_dir, '-o', out, work / 'oracle.crexx')
    run(bin_dir / 'rxas', *flags, out)
    for vm in ('rxtvm', 'rxbvm'):
        expected = run(bin_dir / vm, out.with_suffix('.rxbin'))
        (work / f'{mode}-{vm}.out').write_text(expected)
        observations[mode, vm] = expected
actual = run(bin_dir / 'rexxscript1', source)
(work / 'rexxscript1.out').write_text(actual)
assert actual == observations['noopt', 'rxtvm'] == observations['noopt', 'rxbvm'], 'runtime scalar mismatch'
assert observations['opt', 'rxtvm'] == observations['opt', 'rxbvm'], 'compiler VM mismatch'
runtime = actual.splitlines()
folded = observations['opt', 'rxtvm'].splitlines()
assert len(runtime) == len(folded) == 28
differences = [{'line': i + 1, 'source': source.read_text().splitlines()[i],
                'runtime': a, 'folded': b}
               for i, (a, b) in enumerate(zip(runtime, folded)) if a != b]
# The develop repair must preserve runtime results under optimization. The old
# RS-F01 discrepancy remains in historical evidence, not in an allowed-failure list.
receipt = {'runtime_overlap': 28, 'compiler_modes': 2, 'compiler_vms': 2,
           'optimizer_disagreements': differences, 'unexpected_failures': 0}
(work / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
assert not differences, differences
print(f'scalar_runtime_overlap=28; optimizer_disagreements={len(differences)}; unexpected_failures=0')
