#!/usr/bin/env python3
"""Independent preparation oracle plus generated-backend comment regressions."""
import argparse
import hashlib
import itertools
import json
import re
import shutil
import subprocess
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
p.add_argument('--part', choices=['all', 'preparation', 'backends', 'admission'], default='all')
a = p.parse_args()
root = Path(__file__).resolve().parents[3]
binary, out = a.build.resolve() / 'bin', a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
commands, observations = [], []

def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

product_paths = [binary / name for name in ['levell', 'rxc', 'rxas', 'rxlink', 'rxtvm', 'rxbvm', 'library.rxbin', 'rxfnsl.rxbin']]
bootstrap = a.build.resolve() / 'lib/rxfnsl/tools/bootstrap/L2/levell-linked.rxbin'
if bootstrap.exists():
    product_paths.append(bootstrap)
product_before = {str(path): sha256(path) for path in product_paths}


def run(name, argv, expected=0):
    argv = list(map(str, argv))
    result = subprocess.run(argv, capture_output=True, timeout=1200)
    data = result.stdout + result.stderr
    (out / (name + '.log')).write_bytes(data)
    commands.append(dict(name=name, argv=argv, status=result.returncode,
                         log_sha256=hashlib.sha256(data).hexdigest()))
    assert result.returncode == expected, (name, result.returncode, data[-2500:])
    assert b'Warning in ' not in data and b'WARNING' not in data, (name, data[-2500:])
    return result.stdout


def compile_probe(key, work, source, extra=(), mode='opt', core='rxfnsl'):
    imports = work / 'imports'
    imports.mkdir(exist_ok=True)
    for path in [binary / 'library.rxbin', binary / (core + '.rxbin'), *extra]:
        shutil.copy2(path, imports / path.name)
    base = work / ('probe-' + mode)
    run(key + '-compile-' + mode, [binary / 'rxc', '-x', '--no-exe-import'] +
        (['--no-opt'] if mode == 'noopt' else []) +
        ['-i', imports, '-o', base, source])
    run(key + '-assemble-' + mode, [binary / 'rxas', base])
    image = work / ('linked-' + mode)
    run(key + '-link-' + mode, [binary / 'rxlink', '-s', '-o', image, base.with_suffix('.rxbin'),
        *extra, binary / (core + '.rxbin'), binary / 'library.rxbin'])
    return image.with_suffix('.rxbin')


def oracle(source, keep):
    forms = [(b'#', b'', 'line', 0), (b'//', b'', 'line', 0),
             (b'/*', b'*/', 'block', 0), (b'(*', b'*)', 'nested', 0),
             (b'<!--', b'-->', 'block', keep), (b'##', b'$$', 'nested', 0)]
    result = bytearray(source)
    at = 0
    while at < len(source):
        if source[at] in (34, 39):
            quote = source[at]
            at += 1
            while at < len(source):
                if source[at] == quote:
                    at += 1
                    if at < len(source) and source[at] == quote:
                        at += 1
                        continue
                    break
                at += 1
            continue
        matches = [form for form in forms if source.startswith(form[0], at)]
        if not matches:
            at += 1
            continue
        opening, closing, kind, retain = max(matches, key=lambda form: len(form[0]))
        start = at
        at += len(opening)
        if kind == 'line':
            while at < len(source) and source[at] not in (10, 13):
                at += 1
        else:
            depth = 1
            while at < len(source) and depth:
                if source.startswith(closing, at):
                    depth -= 1
                    at += len(closing)
                elif kind == 'nested' and source.startswith(opening, at):
                    depth += 1
                    at += len(opening)
                else:
                    at += 1
            if depth:
                return 1, start, None, kind
        for index in range(start, at):
            if not retain or source[index] not in (10, 13):
                result[index] = 32
    return 0, 0, bytes(result), None


if a.part in ('all', 'preparation'):
    work = out / 'preparation'
    work.mkdir(exist_ok=True)
    cases = [
        b'1 // (* not a block\n2 /* (* not nested */ 3 (*a(*b*)c*)',
        b'## complete $$1 # line\r\n2',
        b'/* first /* text */ rest */',
        b'"# // /* (* <!-- ##"', b"'a''(*b' # after\n2",
        b'"a""/* b" // after', b'"broken (* should remain quoted',
        b'1<!--\r\nnotes\rmore\n-->2', b'1//at EOF', b'1#at EOF',
        b'(* nested (* still open *)', b'/* open', b'<!--open',
        '1(* café 日本 😀 *)2'.encode(), b'1/*\0*/2', b'1(*\0*)2',
    ]
    # Bounded combinations distinguish neighboring markers, quotes and lines.
    atoms = [b'1', b'#x\n', b'//x\r\n', b'/*x*/', b'(*x(*y*)z*)', b'"/*x*/"', b'##x$$']
    cases += [b''.join(pair) for pair in itertools.product(atoms, repeat=2)]
    for mode in ('opt', 'noopt'):
        image = compile_probe('preparation', work,
            root / 'lib/rxfnsl/tests_functional/comment-preparation-probe.crexx', mode=mode)
        for keep in (0, 1):
            for index, source in enumerate(cases):
                path = work / f'input-{index}.txt'
                path.write_bytes(source)
                status, offset, masked, kind = oracle(source, keep)
                for vm in ('rxtvm', 'rxbvm'):
                    key = f'preparation-{mode}-{vm}-{keep}-{index}'
                    data = run(key, [binary / vm, image, '-a', path, keep], status)
                    assert f'STATUS|{status}|BYTE|{offset}\n'.encode() in data, (key, data)
                    assert b'ORIGINAL|' + source.hex().upper().encode() + b'\n' in data, (key, data)
                    if masked is not None:
                        assert b'MASK|' + masked.hex().upper().encode() + b'\n' in data, (key, data)
                    else:
                        assert f'unclosed {kind} comment'.encode() in data
                        assert f'opening bytes [{offset},'.encode() in data
                    observations.append(dict(key=key, status=status))
        for vm in ('rxtvm', 'rxbvm'):
            path = work / 'tiny.txt'
            path.write_bytes(b'1(* many bytes *)2')
            data = run('preparation-tiny-' + mode + '-' + vm,
                       [binary / vm, image, '-a', path, 0, 2], 2)
            assert b'LL_RESOURCE_LIMIT: scan.work budget 2, required at least 3' in data
            path.write_bytes(b'1(*\xff*)2')
            data = run('preparation-utf8-' + mode + '-' + vm, [binary / vm, image, '-a', path], 1)
            assert b'LL_UTF8:' in data and b'unclosed' not in data
            data = run('preparation-bytes-' + mode + '-' + vm,
                       [binary / vm, image, '-a', path, 0, 10000000, 'bytes'])
            assert b'MASK|' + oracle(path.read_bytes(), 0)[2].hex().upper().encode() + b'\n' in data
            observations += [dict(key='preparation-extra-' + mode + '-' + vm, checks=3)]


base = (root / 'lib/rxfnsl/examples/settings/03-unicode.levell').read_text()
declarations = '''hash.notes: comments line '#'
slash.notes: comments line '//'
c.notes: comments block '/*' '*/'
rexx.notes: comments nested '(*' '*)'
angle.notes: comments block '<!--' '-->' newlines keep
long.hash: comments nested '##' '$$'
'''


def specification(backend, comments=declarations, custom=False):
    spec = base.replace('scan: lexer', comments + 'scan: lexer', 1)
    spec = spec.replace('parser start settings', 'parser start settings backend ' + backend +
                        (' semantics ordered' if backend == 'packrat' else ''))
    spec += '\nread.errors: recovery rule settings sync separator\n'
    if custom:
        spec = re.sub(r"(comments (?:line|block|nested) [^\n]+)", r'\1 outside quoted.text', spec)
        start = spec.index('  string.lex: rule =')
        end = spec.index('\n', start)
        spec = spec[:start] + '''  any.character: charset = scalar
  escaped.body: charset = scalar except ('"' | U+005C | U+000A | U+000D)
  quoted.text: pattern = '"' (escaped.body | U+005C any.character)* '"'
  string.lex: rule = quoted.text then emit string''' + spec[end:]
    return spec


def generate(key, text, route='rxas', refusal=None):
    work = out / key
    work.mkdir(exist_ok=True)
    spec = work / 'input.levell'
    spec.write_text(text)
    data = run(key + '-generate', [binary / 'levell', spec] + (['--rxas'] if route == 'rxas' else []),
               1 if refusal else 0)
    if refusal:
        assert all(part.encode() in data for part in refusal), (key, data)
        assert b'.globals=' not in data and b'namespace levell_settings' not in data
        observations.append(dict(key=key, refusal=refusal))
        return None
    generated = work / ('levell_settings.rxas' if route == 'rxas' else 'levell_settings.crexx')
    generated.write_bytes(data)
    if route == 'source':
        imports = work / 'imports'
        imports.mkdir(exist_ok=True)
        for name in ['library.rxbin', 'rxfnsl.rxbin']:
            shutil.copy2(binary / name, imports / name)
        run(key + '-compile-language', [binary / 'rxc', '-x', '--no-exe-import', '-i', imports,
            '-o', work / 'levell_settings', generated])
    run(key + '-assemble-language', [binary / 'rxas', work / 'levell_settings'])
    return work


if a.part in ('all', 'backends'):
    cases = [
        ('line-then-nested', 'port=1 // (* not opened\nnext=2', 0, []),
        ('block-not-nested', 'port=1 /* /* ordinary body */\nnext=2', 0, []),
        ('nested', 'port=1 (*outer(*inner*)tail*)\nnext=2', 0, []),
        ('quote-markers', 'port="# // /* (* <!-- ##"\nnext=2', 0, []),
        ('doubled-quotes', 'port="a""(*b"\nnext=2', 0, []),
        ('longest-opener', '## note $$port=1\nnext=2', 0, []),
        ('keep-newline', 'port=1<!-- notes\r\n-->next=2', 0, []),
        ('hide-newline', 'port=1/* notes\n*/next=2', 1, ['value.error' + '!ABSENT']),
        ('eof-line', 'port=1#EOF', 0, []),
        ('unicode', 'café=1(* café 日本 😀 *)\nnext=2', 0, []),
        ('nul-byte', 'port=1/*\0*/\nnext=2', 0, []),
        ('wrong-inner-style', 'port=1/* (* */\nnext=2', 0, []),
        ('open-block', 'port=1\nnext=2 /*oops', 1, ['unclosed block comment c.notes', 'line 2, column 8 (byte 14)']),
        ('open-nested', 'port=1\nnext=2 (*oops', 1, ['unclosed nested comment rexx.notes', 'line 2, column 8 (byte 14)']),
        ('later-error', 'port=1#ignored\r\nnext=many', 1, ["found 'many'", 'line 2, column 6 (byte 21)']),
    ]
    for backend in ('ll1', 'lr1', 'packrat', 'earley'):
        for custom in (False, True):
            for route in ('rxas', 'source'):
                key = backend + ('-custom' if custom else '-default') + '-' + route
                work = generate(key, specification(backend, custom=custom), route)
                image = compile_probe(key, work, root / 'lib/rxfnsl/tests_functional/comment-language-probe.crexx',
                                      [work / 'levell_settings.rxbin'])
                selected = cases if not custom else [case for case in cases if case[0] != 'doubled-quotes'] + [
                    ('backslash-quote', 'port="a\\" (* still quoted"\nnext=2', 0, []),
                ]
                for name, text, status, required in selected:
                    path = work / (name + '.txt')
                    path.write_bytes(text.encode())
                    for vm in ('rxtvm', 'rxbvm'):
                        label = key + '-' + vm + '-' + name
                        data = run(label, [binary / vm, image, '-a', path], status)
                        assert f'STATUS|{status}|'.encode() in data
                        assert f'COMMENTS|6|SHIELDED|{6 if custom else 0}\n'.encode() in data
                        assert b'POLICY|angle.notes|KEEP|1\n' in data
                        for part in required:
                            if part.endswith('!ABSENT'):
                                assert part[:-7].encode() not in data, (label, data)
                            else:
                                assert part.encode() in data, (label, data)
                        if status == 0:
                            assert b'TREE_VALID|1' in data
                            # Every token's spelling/span comes from the original bytes.
                            for line in data.decode().splitlines():
                                if line.startswith('TOKEN|'):
                                    _, kind, first, last, spelling = line.split('|')
                                    assert text.encode()[int(first):int(last)].hex().upper() == spelling
                        observations.append(dict(key=label, status=status))
                # Recovery separators inside line/block/string regions stay opaque.
                for name, text, status, required in [
                    ('later-valid', 'first=many // fake;name=4\nsecond=(*x*)\nthird=3', 1,
                     ['ERRORS|2', 'ASSIGNMENT|third', 'DIAGNOSTIC|6|']),
                    ('unclosed', 'first=1\nsecond=(* never closes', 4,
                     ['unclosed nested comment rexx.notes', 'DIAGNOSTIC|15|']),
                ]:
                    path = work / (name + '.txt')
                    path.write_text(text)
                    for vm in ('rxtvm', 'rxbvm'):
                        label = key + '-' + vm + '-recovery-' + name
                        data = run(label, [binary / vm, image, '-a', path, 'recover'], status)
                        assert all(part.encode() in data for part in required), (label, data)
                        observations.append(dict(key=label, status=status))
                path = work / 'low-budget.txt'
                path.write_text('port=1(* many bytes *)')
                for vm in ('rxtvm', 'rxbvm'):
                    data = run(key + '-' + vm + '-tiny', [binary / vm, image, '-a', path, 'strict', 2], 2)
                    assert b'LL_RESOURCE_LIMIT: scan.work budget 2, required at least 3' in data
                    assert b'LL_COMMENT: unclosed' not in data and b'value.error' not in data

    # The expression components receive exactly the same prepared byte policy.
    pratt = (root / 'lib/rxfnsl/examples/calculator/calculator.levell').read_text()
    pratt = pratt.replace('namespace levell_calculator', 'namespace levell_settings')
    pratt = pratt.replace("notes: comments nested '(*' '*)'", declarations)
    regional = (root / 'lib/rxfnsl/examples/settings/05-composite.levell').read_text()
    regional = regional.replace('scan: lexer', declarations + 'scan: lexer', 1)
    for backend, spec, cases in [
        ('pratt', pratt, [
            ('clean', '2 // (* ignored\n+3*4', 0, ['TREE_VALID|1']),
            ('nested', '2(*outer(*inner*)tail*)+3', 0, ['TREE_VALID|1']),
            ('block', '2/* /* body */+3', 0, ['TREE_VALID|1']),
            ('bad', '2/* masked */+*3', 1, ["unexpected '*'", 'byte 14']),
            ('unclosed', '2 (* never closed', 1, ['unclosed nested comment rexx.notes', 'byte 2']),
        ]),
        ('regional', regional, [
            ('clean', 'port=[2/* ] is ignored */+3*4]', 0, ['TREE_VALID|1']),
            ('visible-line-ending', 'port=[2 // ] is ignored\n+3*4]', 1,
             ['embedded expression opened at byte 5', 'byte 23', r'\x0A']),
            ('nested', 'port=[2(* inner [ ] *)+3]', 0, ['TREE_VALID|1']),
            ('bad', 'port=[2/* body */+*3]', 1, ["unexpected '*'", 'embedded expression opened at byte 5', 'byte 18']),
            ('unclosed', 'port=[2 (* never closed', 1, ['unclosed nested comment rexx.notes', 'byte 8']),
        ]),
    ]:
        for route in ('rxas', 'source'):
            key = backend + '-' + route
            work = generate(key, spec, route)
            image = compile_probe(key, work, root / 'lib/rxfnsl/tests_functional/comment-language-probe.crexx',
                                  [work / 'levell_settings.rxbin'])
            for name, source, status, required in cases:
                path = work / (name + '.txt')
                path.write_text(source)
                for vm in ('rxtvm', 'rxbvm'):
                    label = key + '-' + vm + '-' + name
                    data = run(label, [binary / vm, image, '-a', path], status)
                    assert all(part.encode() in data for part in required), (label, data)
                    observations.append(dict(key=label, status=status))

    # A regular but unproductive long prefix cannot exceed the host work budget.
    spec = specification('lr1', "notes: comments nested '(*' '*)' outside slow\n")
    spec = spec.replace('scan: lexer', "scan: lexer\n  slow: pattern = 'a'* 'b'", 1)
    work = generate('bounded-shielding', spec)
    image = compile_probe('bounded-shielding', work,
        root / 'lib/rxfnsl/tests_functional/comment-language-probe.crexx', [work / 'levell_settings.rxbin'])
    path = work / 'adversary.txt'
    path.write_text('a' * 1000)
    for vm in ('rxtvm', 'rxbvm'):
        data = run('bounded-shielding-' + vm, [binary / vm, image, '-a', path, 'strict', 100], 2)
        assert b'LL_RESOURCE_LIMIT: scan.work budget 100, required at least 101' in data
        observations.append(dict(key='bounded-shielding-' + vm, status=2))

if a.part in ('all', 'admission'):
    failures = [
        ('duplicate', declarations + "again: comments nested '#' '!'\n", ['duplicate opener', 'hash.notes', 'line ', 'byte ']),
        ('empty', "bad: comments line ''\n", ['LL_COMMENT_CONTRACT', 'nonempty']),
        ('same', "bad: comments nested '!!' '!!'\n", ['LL_COMMENT_CONTRACT', 'distinct']),
        ('line-close', "bad: comments line '#' '!'\n", ['LL_COMMENT_CONTRACT']),
        ('unknown-form', "bad: comments recursive '/*' '*/'\n", ['LL_COMMENT_CONTRACT', 'line, block or nested']),
        ('unknown-shield', "bad: comments block '/*' '*/' outside missing\n", ['unknown outside pattern']),
        ('wrong-shield', "bad: comments block '/*' '*/' outside identifier\n", ['named lexical pattern']),
        ('bad-policy', "bad: comments block '/*' '*/' newlines hide\n", ['LL_COMMENT_CONTRACT', 'keep']),
        ('repeat-policy', "bad: comments block '/*' '*/' newlines keep newlines keep\n", ['duplicate comment qualifier']),
        ('cross-nesting', "bad: comments nested '/*' '*/' mixed styles\n", ['unsupported comment qualifier']),
        ('quote-opener', "bad: comments line '" + '"' + "'\n", ['LL_COMMENT_CONTRACT', 'overlaps default quote']),
    ]
    for name, comments, required in failures:
        generate('refuse-' + name, specification('lr1', comments), refusal=required)
    for name, pattern, required in [
        ('nullable', "''", ['LL_NULLABLE_TOKEN']),
        ('recursive', 'quoted.text', ['LL_LEX_RECURSION']),
        ('overlap', "'/'", ['LL_COMMENT_CONTRACT', 'overlaps opener']),
    ]:
        text = specification('lr1', "bad: comments block '/*' '*/' outside quoted.text\n")
        text = text.replace('scan: lexer', 'scan: lexer\n  quoted.text: pattern = ' + pattern, 1)
        generate('refuse-shield-' + name, text, refusal=required)

assert product_before == {str(path): sha256(path) for path in product_paths}, 'Product artifacts changed during qualification'

(out / 'receipt.json').write_text(json.dumps(dict(commands=commands, observations=observations,
    products=product_before,
    scope='Independent source-byte oracle; four grammar engines, default/custom shielding, source/RXAS; separate strict/recovery/UTF8/resource outcomes.',
    sources={str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
             for path in [Path(__file__).resolve(), root / 'lib/rxfnsl/tests_functional/comment-preparation-probe.crexx',
                          root / 'lib/rxfnsl/tests_functional/comment-language-probe.crexx']}), indent=2) + '\n')
print(f'observations={len(observations)}; commands={len(commands)}; failures=0')
