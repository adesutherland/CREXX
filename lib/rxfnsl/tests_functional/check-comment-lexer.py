#!/usr/bin/env python3
"""Standalone lexer metadata/policy parity, without an implicit parser."""
import argparse
import hashlib
import json
import shutil
import subprocess
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[3]
binary, out = a.build.resolve() / 'bin', a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
records = []
products = [binary / name for name in ['levell', 'rxc', 'rxas', 'rxlink', 'rxtvm', 'rxbvm', 'library.rxbin', 'rxfnsl.rxbin']]
product_before = {str(path): hashlib.sha256(path.read_bytes()).hexdigest() for path in products}


def run(name, argv, status=0):
    argv = list(map(str, argv))
    result = subprocess.run(argv, capture_output=True, timeout=1200)
    data = result.stdout + result.stderr
    (out / (name + '.log')).write_bytes(data)
    assert result.returncode == status and b'Warning in ' not in data, (name, data[-2000:])
    records.append(dict(name=name, argv=argv, status=status, sha256=hashlib.sha256(data).hexdigest()))
    return result.stdout


spec = '''options levell
namespace levell_comments
number: token
newline: token
hash: comments line '#'
plain: comments block '/*' '*/'
nested: comments nested '(*' '*)'
scan: lexer input utf8
  digit: charset = '0' to '9'
  number.lex: rule = digit+ then emit number
  newline.lex: rule = U+000D U+000A | U+000D | U+000A then emit newline
  whitespace: rule = (' ' | U+0009)+ then skip
'''
for route in ['source', 'rxas']:
    work = out / route
    (work / 'imports').mkdir(parents=True, exist_ok=True)
    path = work / 'input.levell'
    path.write_text(spec)
    generated = run(route + '-generate', [binary / 'levell', path] + (['--rxas'] if route == 'rxas' else []))
    target = work / ('levell_comments.' + ('crexx' if route == 'source' else 'rxas'))
    target.write_bytes(generated)
    for name in ['rxfnsl', 'library']:
        shutil.copy2(binary / (name + '.rxbin'), work / 'imports' / (name + '.rxbin'))
    if route == 'source':
        run(route + '-compile-language', [binary / 'rxc', '-x', '--no-exe-import', '-i', work / 'imports', '-o', work / 'levell_comments', target])
    run(route + '-assemble-language', [binary / 'rxas', work / 'levell_comments'])
    shutil.copy2(work / 'levell_comments.rxbin', work / 'imports/levell_comments.rxbin')
    for mode in ['opt', 'noopt']:
        probe = work / ('probe-' + mode)
        run(route + '-compile-' + mode, [binary / 'rxc', '-x', '--no-exe-import'] + (['-n'] if mode == 'noopt' else []) +
            ['-i', work / 'imports', '-o', probe, root / 'lib/rxfnsl/tests_functional/comment-lexer-probe.crexx'])
        run(route + '-assemble-' + mode, [binary / 'rxas', probe])
        image = work / ('linked-' + mode)
        run(route + '-link-' + mode, [binary / 'rxlink', '-s', '-o', image, probe.with_suffix('.rxbin'), work / 'levell_comments.rxbin', binary / 'rxfnsl.rxbin', binary / 'library.rxbin'])
        for name, source, status, required in [
            ('coherent', b'1#(* is text\r\n2/* body */3(*x(*y*)z*)', 0,
             [b'TOKEN|number|0|1', b'TOKEN|newline|12|14', b'TOKEN|number|14|15', b'TOKEN|number|25|26']),
            ('unclosed', b'1/* broken', 1, [b'LL_COMMENT: unclosed block comment plain', b'BYTE|1']),
            ('eof', b'1#at EOF', 0, [b'TOKEN|number|0|1']),
            ('utf8', b'1/*\xff*/', 1, [b'LL_UTF8', b'BYTE|3']),
        ]:
            path = work / (name + '.txt')
            path.write_bytes(source)
            for vm in ['rxtvm', 'rxbvm']:
                data = run(route + '-' + mode + '-' + vm + '-' + name, [binary / vm, image.with_suffix('.rxbin'), '-a', path], status)
                assert all(value in data for value in required), (name, data)

# Literal identity/UTF-8/control transport through both emitters.
for label, opening, closing, payload, first in [
    ('numeric', '01', '1', '01body12', 7),
    ('unicode', '«', '»', '«body»2', 8),
    ('nul', '|\0', '\0|', '|\0body\0|2', 8),
    ('control', '|\x01', '\x01|', '|\x01body\x01|2', 8),
]:
    for route in ['source', 'rxas']:
        key = label + '-' + route
        work = out / key
        (work / 'imports').mkdir(parents=True, exist_ok=True)
        text = '\n'.join(line for line in spec.splitlines() if ': comments ' not in line)
        text = text.replace('scan: lexer', "notes: comments block '" + opening + "' '" + closing + "'\nscan: lexer", 1)
        path = work / 'input.levell'
        path.write_text(text)
        data = run(key + '-generate', [binary / 'levell', path] + (['--rxas'] if route == 'rxas' else []))
        generated = work / ('levell_comments.' + ('crexx' if route == 'source' else 'rxas'))
        generated.write_bytes(data)
        for name in ['rxfnsl', 'library']:
            shutil.copy2(binary / (name + '.rxbin'), work / 'imports' / (name + '.rxbin'))
        if route == 'source':
            run(key + '-compile-language', [binary / 'rxc', '-x', '--no-exe-import', '-i', work / 'imports', '-o', work / 'levell_comments', generated])
        run(key + '-assemble-language', [binary / 'rxas', work / 'levell_comments'])
        shutil.copy2(work / 'levell_comments.rxbin', work / 'imports/levell_comments.rxbin')
        run(key + '-compile', [binary / 'rxc', '-x', '--no-exe-import', '-i', work / 'imports', '-o', work / 'probe', root / 'lib/rxfnsl/tests_functional/comment-lexer-probe.crexx'])
        run(key + '-assemble', [binary / 'rxas', work / 'probe'])
        run(key + '-link', [binary / 'rxlink', '-s', '-o', work / 'linked', work / 'probe.rxbin', work / 'levell_comments.rxbin', binary / 'rxfnsl.rxbin', binary / 'library.rxbin'])
        path = work / 'input.txt'
        path.write_bytes(payload.encode())
        for vm in ['rxtvm', 'rxbvm']:
            data = run(key + '-' + vm, [binary / vm, work / 'linked.rxbin', '-a', path])
            assert f'TOKEN|number|{first}|{first+1}\n'.encode() in data, (label, data)
            assert data.count(b'TOKEN|') == 1, (label, data)

assert product_before == {str(path): hashlib.sha256(path.read_bytes()).hexdigest() for path in products}, 'Product changed during qualification'
(out / 'receipt.json').write_text(json.dumps(dict(records=records, runtime_cases=48, products=product_before), indent=2) + '\n')
print('lexer_cases=48; failures=0')
