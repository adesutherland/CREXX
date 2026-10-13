#!/usr/bin/env python3
"""Independent raw-UCD interval oracle, native table parity and checksum failure."""
import argparse
import hashlib
import json
import shutil
import struct
import subprocess
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[3]
binary = a.build.resolve() / 'bin'
out = a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
raw = root / 'lib/rxfnsl/unicode/data/unicode-17.0.0'
expected = {}
records = []


def merge(ranges):
    result = []
    for low, high in sorted(ranges):
        if result and low <= result[-1][1] + 1:
            result[-1] = (result[-1][0], max(result[-1][1], high))
        else:
            result.append((low, high))
    return result


for filename in ['DerivedCoreProperties.txt', 'PropList.txt']:
    for line in (raw / filename).read_text().splitlines():
        body = line.split('#', 1)[0].strip()
        if not body:
            continue
        points, name, *_ = map(str.strip, body.split(';'))
        limits = points.split('..')
        expected.setdefault(name, []).append((int(limits[0], 16), int(limits[-1], 16)))

pending = None
assigned = []
for line in (raw / 'UnicodeData.txt').read_text().splitlines():
    fields = line.split(';')
    point = int(fields[0], 16)
    if fields[1].endswith(', First>'):
        pending = point
        continue
    low = pending if fields[1].endswith(', Last>') else point
    pending = None
    assigned.append((low, point))
    if fields[2] == 'Cs':
        continue
    for name in ['gc.' + fields[2], 'gc.' + fields[2][0]]:
        expected.setdefault(name, []).append((low, point))
end = 0
unassigned = []
for low, high in merge(assigned):
    if end < low:
        unassigned.append((end, low - 1))
    end = high + 1
if end <= 0x10FFFF:
    unassigned.append((end, 0x10FFFF))
expected['gc.Cn'] = unassigned
expected['gc.Cs'] = []
expected.setdefault('gc.C', []).extend(unassigned)


def run(name, command, status=0):
    command = list(map(str, command))
    r = subprocess.run(command, capture_output=True, timeout=1200)
    data = r.stdout + r.stderr
    (out / (name + '.log')).write_bytes(data)
    records.append(dict(name=name, argv=command, status=r.returncode,
                        sha256=hashlib.sha256(data).hexdigest()))
    assert r.returncode == status and b'Warning in ' not in data, (name, r.returncode, data[-2000:])
    return r.stdout


for mode in ['opt', 'noopt']:
    flags = ['-n'] if mode == 'noopt' else []
    run(mode + '-compile', [binary / 'rxc', *flags, '-x', '--no-exe-import', '-i', binary,
                           '-o', out / 'probe', root / 'lib/rxfnsl/tests_functional/property-probe.crexx'])
    run(mode + '-assemble', [binary / 'rxas', *flags, out / 'probe'])
    run(mode + '-link', [binary / 'rxlink', '-s', '-o', out / 'linked', out / 'probe.rxbin',
                        binary / 'rxfnsl.rxbin', binary / 'library.rxbin'])
    for vm in ['rxtvm', 'rxbvm']:
        observed = run(mode + '-' + vm, [binary / vm, out / 'linked.rxbin']).decode().splitlines()
        assert observed[0] == 'VERSION|17.0.0'
        for record in observed[1:]:
            _, name, data = record.split('|')
            actual = list(struct.iter_unpack('<ii', bytes.fromhex(data)))
            assert actual == merge(expected[name]), (name, 'native interval mismatch')
        assert len(observed) == 46, ('property count', len(observed))

# The checksum gate fails before invoking the compiler and keeps a good output.
damaged = out / 'damaged-data'
shutil.copytree(raw, damaged, dirs_exist_ok=True)
with (damaged / 'PropList.txt').open('ab') as file:
    file.write(b'\n# deliberate corruption\n')
preserved = out / 'preserved.crexx'
preserved.write_bytes(b'last good artifact\n')
generator = a.build.resolve() / 'lib/rxfnsl/rexx/properties/generator-linked.rxbin'
run('checksum-refusal', ['cmake', '-DVM=' + str(binary / 'rxtvm'), '-DGENERATOR=' + str(generator),
                        '-DDATA=' + str(damaged), '-DOUTPUT=' + str(preserved), '-P',
                        root / 'lib/rxfnsl/unicode/generate-properties.cmake'], 1)
assert preserved.read_bytes() == b'last good artifact\n'
assert not Path(str(preserved) + '.tmp').exists()
(out / 'receipt.json').write_text(json.dumps(dict(records=records, properties=45, cells=4,
    raw_sha256={p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in raw.glob('*.txt')}), indent=2) + '\n')
print('properties=45; native_cells=4; checksum_preservation=1; failures=0')
