#!/usr/bin/env python3
"""Compare portable table bytes in Level L's RXAS and source emission paths."""
import argparse
import hashlib
from pathlib import Path
import re
import struct

parser = argparse.ArgumentParser()
parser.add_argument('rxas', type=Path)
parser.add_argument('source', type=Path)
parser.add_argument('receipt', type=Path)
args = parser.parse_args()
assembly = args.rxas.read_text()
source = args.source.read_text()
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
