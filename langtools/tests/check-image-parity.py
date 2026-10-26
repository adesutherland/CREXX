#!/usr/bin/env python3
"""Compare portable table bytes in Level L's RXAS and source emission paths."""
import argparse
import hashlib
from pathlib import Path
import re
import struct

def prepared_call_shape(text):
    """The raw generated entry must reserve the whole optional-formal window."""
    entry = re.search(r'^language\(\) \.locals=(\d+) \.expose=\S+\n(.*?)(?=^\S+\(\).*\.expose=|\Z)',
                      text, re.M | re.S)
    assert entry, 'prepared language entry'
    locals_count = int(entry[1])
    ops = [tuple(part.strip() for part in match[2].split(','))
           for match in re.finditer(r'^\s+(load|settp|call|ret)\s+(.+)$', entry[2], re.M)]
    names = [match[1] for match in re.finditer(r'^\s+([a-z][a-z0-9]*)\s+(.+)$', entry[2], re.M)]
    assert names == ['load', 'settp', 'settp', 'settp', 'load', 'call', 'ret'], 'prepared call instructions'
    assert len(ops[5]) == 3 and re.fullmatch(r'r[0-9]+', ops[5][2]), 'prepared call count register'
    result, target, count_reg = ops[5]
    count = int(count_reg[1:])
    image_reg, policy_reg, bytes_reg = (f'r{count + offset}' for offset in (1, 2, 3))
    assert target.lstrip('§') == 'lllanguage.prepared()', 'prepared call target'
    assert re.fullmatch(r'r[0-9]+', result) and int(result[1:]) < locals_count, 'prepared result frame'
    assert locals_count > count + 3, 'prepared physical optional window exceeds frame'
    assert len(ops[0]) == 2 and ops[0][0] == image_reg and re.fullmatch(r'0x[0-9a-fA-F]+', ops[0][1]), 'prepared image argument'
    assert ops[1:5] == [(image_reg, '256'), (policy_reg, '512'), (bytes_reg, '0'), (count_reg, '3')], 'prepared physical count and explicit presence flags'
    assert ops[6] == (result,), 'prepared owned return'
    image = bytes.fromhex(ops[0][1][2:])
    assert image.startswith(b'LDP1'), 'prepared image magic'
    return image


def compiled_call_shape(text):
    """Trusted entry has one supplied image formal and no runtime hydration."""
    entry = re.search(r'^language\(\) \.locals=(\d+) \.expose=\S+\n(.*?)(?=^\S+\(\).*\.expose=|\Z)',
                      text, re.M | re.S)
    assert entry, 'compiled language entry'
    locals_count = int(entry[1])
    ops = [(match[1], tuple(part.strip() for part in match[2].split(',')))
           for match in re.finditer(r'^\s+([a-z][a-z0-9]*)\s+(.+)$', entry[2], re.M)]
    assert [op for op, _ in ops] == ['load', 'settp', 'load', 'call', 'ret'], 'compiled wrapper instructions'
    result, target, count_reg = ops[3][1]
    assert target.lstrip('§') == 'lllanguage.compiled()', 'compiled target'
    assert re.fullmatch(r'r[0-9]+', count_reg), 'compiled count register'
    count = int(count_reg[1:]); image_reg = f'r{count + 1}'
    assert locals_count > count + 1, 'compiled image window outside frame'
    assert re.fullmatch(r'r[0-9]+', result) and int(result[1:]) < locals_count, 'compiled return frame'
    assert ops[0][1][0] == image_reg and re.fullmatch(r'0x[0-9a-fA-F]+', ops[0][1][1]), 'compiled image operand'
    assert ops[1][1] == (image_reg, '256') and ops[2][1] == (count_reg, '1'), 'compiled formal presence/count'
    assert ops[4][1] == (result,), 'compiled owned return'
    image = bytes.fromhex(ops[0][1][1][2:])
    assert image.startswith(b'LFP1'), 'compiled image magic'
    return image


def compiled_window_controls():
    valid = ('language() .locals=8 .expose=control.language\n'
             '  load r7,0x4c465031\n  settp r7,256\n'
             '  load r6,1\n  call r4,§lllanguage.compiled(),r6\n  ret r4\n')
    assert compiled_call_shape(valid) == b'LFP1'
    for old, new in (('.locals=8', '.locals=7'), ('load r6,1', 'load r6,3'),
                     ('settp r7,256', 'settp r7,0'), ('load r7,0x', 'load r5,0x'),
                     ('lllanguage.compiled()', 'lllanguage.prepared()'), ('ret r4', 'ret r3')):
        try:
            compiled_call_shape(valid.replace(old, new))
        except AssertionError:
            pass
        else:
            raise AssertionError('compiled window negative: ' + old)


def prepared_window_controls():
    valid = ('language() .locals=10 .expose=control.language\n'
             '  load r7,0x4c445031\n  settp r7,256\n  settp r8,512\n'
             '  settp r9,0\n  load r6,3\n  call r4,§lllanguage.prepared(),r6\n  ret r4\n')
    assert prepared_call_shape(valid) == b'LDP1'
    for old, new in (('.locals=10', '.locals=9'), ('load r6,3', 'load r6,1'),
                     ('settp r8,512', 'settp r8,256'), ('settp r9,0', 'settp r9,256'),
                     ('  settp r8,512\n', ''), ('lllanguage.prepared()', 'foreign.prepared()')):
        try:
            prepared_call_shape(valid.replace(old, new))
        except AssertionError:
            pass
        else:
            raise AssertionError('prepared optional-window negative: ' + old)


prepared_window_controls()
compiled_window_controls()

parser = argparse.ArgumentParser()
parser.add_argument('rxas', type=Path)
parser.add_argument('source', type=Path)
parser.add_argument('receipt', type=Path)
args = parser.parse_args()
assembly = args.rxas.read_text(encoding="utf-8")
source = args.source.read_text(encoding="utf-8")
if re.search(r'call r[0-9]+,§?lllanguage\.compiled\(\),r[0-9]+', assembly):
    image = compiled_call_shape(assembly)
    constant = re.search(r"^  constant (\w+) = '([0-9A-Fa-f]+)'x as \.binary$", source, re.M)
    assert constant and bytes.fromhex(constant[2]) == image, 'frozen source/RXAS image parity'
    assert re.search(r'^  return lllanguage\.\.compiled\(' + re.escape(constant[1]) + r'\)$', source, re.M), 'trusted source entry'
    assert len(image) >= 20 and struct.unpack_from('<I', image, 4)[0] == 1
    assert struct.unpack_from('<I', image, 8)[0] == len(image)
    profile = struct.unpack_from('<I', image, 12)[0]
    assert profile in (0, 1)
    print(f'emission_image_pairs={1 + profile}; byte_identical=true')
    raise SystemExit(0)

if re.search(r'call r[0-9]+,§?lllanguage\.prepared\(\),r[0-9]+', assembly):
    image = prepared_call_shape(assembly)
    constant = re.search(r"^  constant (\w+) = '([0-9A-Fa-f]+)'x as \.binary$", source, re.M)
    assert constant and bytes.fromhex(constant[2]) == image, 'prepared source/RXAS image parity'
    assert re.search(r'^  return lllanguage\.\.prepared\(' + re.escape(constant[1]) + r'\)$', source, re.M), 'prepared source hydrator'
    # Whole-image parity also covers every metadata section, not just tables.
    assert len(image) >= 20 and struct.unpack_from('<I', image, 4)[0] == 1
    assert struct.unpack_from('<I', image, 8)[0] == len(image)
    profile = struct.unpack_from('<I', image, 12)[0]
    assert profile in (0, 1)
    print(f'emission_image_pairs={1 + profile}; byte_identical=true')
    raise SystemExit(0)

assembly_images = {}
for match in re.finditer(r'load r\d+,0x([0-9A-Fa-f]+)', assembly):
    image = bytes.fromhex(match.group(1))
    if len(image) >= 12:
        magic = struct.unpack_from('<I', image)[0]
        if magic in (1279414833, 1279542833):
            assert magic not in assembly_images
            assembly_images[magic] = image
results = {}
for name, magic in [('lexerdata', 1279542833), ('parserdata', 1279414833)]:
    body = source.split(name + ': procedure = .binary\n', 1)[1]
    body = body.split('\n  return result', 1)[0]
    image = b''.join(bytes.fromhex(x) for x in re.findall(r"chunk = '([0-9A-Fa-f ]+)'x as .binary", body))
    assert image and image == assembly_images[magic], name
    results[name] = {'bytes': len(image), 'sha256': hashlib.sha256(image).hexdigest(),
                     'states': struct.unpack_from('<I', image, 4)[0]}
assert '|root=' in source and '|root=' in assembly
# Results are reported, not retained as a per-run receipt.
print('emission_image_pairs=2; byte_identical=true')
