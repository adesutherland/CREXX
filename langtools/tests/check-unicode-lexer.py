#!/usr/bin/env python3
"""Large Unicode property DFA checked against independently read UCD membership."""
import argparse
import bisect
import hashlib
import json
import random
import struct
import subprocess
from pathlib import Path
from _scratch import scratch

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[2]
binary = a.build.resolve() / 'bin'
out = scratch()
out.mkdir(parents=True, exist_ok=True)
records = []
sets = {'XID_Start': [], 'XID_Continue': []}
raw = root / 'lib/rxfnsl/unicode/data/unicode-17.0.0/DerivedCoreProperties.txt'
for line in raw.read_text().splitlines():
    body = line.split('#', 1)[0].strip()
    if not body:
        continue
    points, name, *_ = map(str.strip, body.split(';'))
    if name in sets:
        limits = points.split('..')
        sets[name].append((int(limits[0], 16), int(limits[-1], 16)))


def contains(name, point):
    ranges = sets[name]
    i = bisect.bisect_right(ranges, (point, 0x110000)) - 1
    return i >= 0 and point <= ranges[i][1]


def run(name, command, status=0):
    command = list(map(str, command))
    r = subprocess.run(command, capture_output=True, timeout=1800)
    data = r.stdout + r.stderr
    (out / (name + '.log')).write_bytes(data)
    records.append(dict(name=name, argv=command, status=r.returncode,
                        sha256=hashlib.sha256(data).hexdigest()))
    assert r.returncode == status and b'Warning in ' not in data, (name, r.returncode, data[-2000:])
    return r.stdout


spec = '''options levell
namespace probe_language
identifier: token = 1
other: token = 2
scan: lexer
first: charset = unicode.XID_Start | '_'
rest: charset = unicode.XID_Continue | '_'
all.characters: charset = scalar
name.lex: rule = first rest* then emit identifier
other.lex: rule = all.characters then emit other
'''
path = out / 'property.levell'
path.write_text(spec)
generated = run('generate', [binary / 'rxlang', path, '--rxas'])
assert b'Unicode property data: 17.0.0' in generated
(out / 'probe_language.rxas').write_bytes(generated)

# Every interval edge, its neighbors, scalar encoding boundaries and fixed random probes.
points = {0, 0x7F, 0x80, 0x7FF, 0x800, 0xD7FF, 0xE000, 0xFFFF, 0x10000, 0x10FFFF}
for ranges in sets.values():
    for low, high in ranges:
        points.update([low - 1, low, low + 1, high - 1, high, high + 1])
randomizer = random.Random(170000)
points.update(randomizer.randrange(0x110000) for _ in range(1000))
points = sorted(x for x in points if 0 <= x <= 0x10FFFF and not 0xD800 <= x <= 0xDFFF)
cases = [(b'', [])]
for point in points:
    suffix = chr(point).encode()
    starts = contains('XID_Start', point) or point == 95
    continues = contains('XID_Continue', point) or point == 95
    cases.append((suffix, [(1 if starts else 2, 0, len(suffix))]))
    cases.append((b'a' + suffix, [(1, 0, 1 + len(suffix))] if continues else [(1, 0, 1), (2, 1, len(suffix))]))
data = struct.pack('<I', len(cases)) + b''.join(struct.pack('<I', len(s)) + s for s, _ in cases)
(out / 'cases.bin').write_bytes(data)

for mode in ['opt', 'noopt']:
    flags = ['-n'] if mode == 'noopt' else []
    run(mode + '-assemble-language', [binary / 'rxas', *flags, out / 'probe_language'])
    run(mode + '-compile', [binary / 'rxc', *flags, '-x', '--no-exe-import', '-i', f'{out};{binary}',
                            '-o', out / 'probe', root / 'langtools/tests/bootstrap-probe.crexx'])
    run(mode + '-assemble', [binary / 'rxas', *flags, out / 'probe'])
    run(mode + '-link', [binary / 'rxlink', '-s', '-o', out / 'linked', out / 'probe.rxbin',
                         out / 'probe_language.rxbin', binary / 'rxfnsl.rxbin', binary / 'library.rxbin'])
    for vm in ['rxtvm', 'rxbvm']:
        result = run(mode + '-' + vm, [binary / vm, out / 'linked.rxbin', '-a', out / 'cases.bin'])
        observed = []
        for line in result.decode().splitlines():
            fields = line.split('|')
            if fields[0] == 'CASE':
                assert fields[2] == '0', line
                observed.append([])
            else:
                assert fields[0] == 'TOKEN', line
                observed[-1].append(tuple(map(int, fields[1:])))
        assert observed == [tokens for _, tokens in cases], ('membership/span mismatch', vm, mode)

# Wrong units and unknown properties are build errors, rather than ASCII fallback.
for name, text, message in [
    ('byte-policy', spec.replace('scan: lexer', 'scan: lexer input bytes'), b'properties require lexer input utf8'),
    ('unknown', spec.replace('unicode.XID_Start', 'unicode.Invented'), b'unknown Unicode 17.0.0 property'),
]:
    path = out / (name + '.levell')
    path.write_text(text)
    refusal = run(name, [binary / 'rxlang', path, '--rxas'], 1)
    assert message in refusal

(out / 'receipt.json').write_text(json.dumps(dict(records=records, inputs=len(cases), cells=4,
    ucd_sha256=hashlib.sha256(raw.read_bytes()).hexdigest(),
    scope='Single-scalar XID starts and prefixed XID continuations, every range edge/neighbor plus deterministic random scalars; original byte spans.'), indent=2) + '\n')
print(f'unicode_lexer_inputs={len(cases)}; cells=4; refusals=2; failures=0')
