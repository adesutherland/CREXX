#!/usr/bin/env python3
"""Located authored messages, exact EOF/Unicode coordinates and PEG speculation."""
import argparse
import hashlib
import json
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


def run(name, command, status=0):
    command = list(map(str, command))
    r = subprocess.run(command, capture_output=True, timeout=1200)
    data = r.stdout + r.stderr
    (out / (name + '.log')).write_bytes(data)
    assert r.returncode == status and b'Warning in ' not in data, (name, r.returncode, data[-2000:])
    records.append(dict(name=name, argv=command, status=r.returncode,
                        sha256=hashlib.sha256(data).hexdigest()))
    return r.stdout.decode()


def build(name, spec):
    work = out / name
    work.mkdir(exist_ok=True)
    path = work / 'input.levell'
    path.write_text(spec)
    generated = run(name + '-generate', [binary / 'rxlang', path, '--rxas'])
    (work / 'levell_settings.rxas').write_text(generated)
    run(name + '-assemble-language', [binary / 'rxas', work / 'levell_settings'])
    run(name + '-compile', [binary / 'rxc', '-x', '--no-exe-import', '-i', f'{work};{binary}',
                            '-o', work / 'probe', root / 'langtools/tests/diagnostic-probe.crexx'])
    run(name + '-assemble', [binary / 'rxas', work / 'probe'])
    run(name + '-link', [binary / 'rxlink', '-s', '-o', work / 'linked', work / 'probe.rxbin',
                         work / 'levell_settings.rxbin', binary / 'rxfnsl.rxbin', binary / 'library.rxbin'])
    return work


base = (root / 'langtools/examples/settings/03-unicode.levell').read_text()
cases = [
    ('ordinary', 'port = many', 7, 1, 8, 'port = many', 7),
    ('tab', '\tport = many', 8, 1, 9, r'\tport = many', 9),
    ('composed', 'café = many', 8, 1, 8, r'caf\u{E9} = many', 12),
    ('combining', 'cafe\u0301 = many', 9, 1, 9, r'cafe\u{301} = many', 14),
    ('supplementary', '𐐀 = many', 7, 1, 5, r'\u{10400} = many', 12),
    ('crlf', 'port = 4\r\nnext = many', 17, 2, 8, 'next = many', 7),
    ('cr', 'port = 4\rnext = many', 16, 2, 8, 'next = many', 7),
    ('eof', 'port =', 6, 1, 7, 'port =', 6),
]
target_count = 0
for backend in ['lr1', 'll1', 'packrat', 'earley']:
    spec = base.replace('parser start settings', 'parser start settings backend ' + backend
                        + (' semantics ordered' if backend == 'packrat' else ''))
    work = build(backend, spec)
    for name, source, offset, line, column, excerpt, marker in cases:
        target = work / (name + '.txt')
        target.write_bytes(source.encode())
        found = 'end of input' if name == 'eof' else "'many'"
        for vm in ['rxtvm', 'rxbvm']:
            data = run(backend + '-' + vm + '-' + name, [binary / vm, work / 'linked.rxbin', '-a', target], 1)
            assert f'STATUS|1|BYTE|{offset}\n' in data, (backend, name, data)
            assert f'Expected a number or quoted string; found {found}.' in data, (backend, name, data)
            assert f'LL_TARGET at line {line}, column {column} (byte {offset})' in data
            assert f'\n  {excerpt}\n  ' + ' ' * marker + '^\n' in data, (backend, name, data)
            target_count += 1
    target = work / 'wrong-context.txt'
    target.write_text('port = 4 many')
    for vm in ['rxtvm', 'rxbvm']:
        data = run(backend + '-' + vm + '-wrong-context', [binary / vm, work / 'linked.rxbin', '-a', target], 1)
        assert 'value.error' not in data and "unexpected 'many'" in data
        target_count += 1

# The expression providers also report the actual unexpected lexeme and opener.
composite = (root / 'langtools/examples/settings/05-composite.levell').read_text()
lexical = composite[:composite.index('parse: parser')]
operators = composite[composite.index('arithmetic: operators'):composite.index('math.region:')]
for backend, spec, source, offset in [
    ('pratt', lexical + 'parse: parser start arithmetic backend pratt\n' + operators, '2+*3', 2),
    ('regional', composite, 'workers = [2+*3]', 13),
]:
    work = build(backend, spec)
    target = work / 'bad.txt'
    target.write_text(source)
    for vm in ['rxtvm', 'rxbvm']:
        data = run(backend + '-' + vm, [binary / vm, work / 'linked.rxbin', '-a', target], 1)
        assert f'STATUS|1|BYTE|{offset}' in data and "unexpected '*'" in data, data
        if backend == 'regional':
            assert 'byte 10' in data, ('region opener', data)
        target_count += 1

# A successful negative predicate must suppress the child's speculative message.
guard = '''options levell
namespace levell_settings
item: token = 1
forbidden: token = 2
scan: lexer
item.lex: rule = 'a' then emit item
forbidden.lex: rule = 'b' then emit forbidden
parse: parser start root backend packrat semantics ordered
root: rule = !guard item
guard: rule = forbidden
guard.error: diagnostic rule guard = 'Speculative guard message must stay hidden.'
'''
work = build('predicate', guard)
target = work / 'input.txt'
target.write_text('aa')
for vm in ['rxtvm', 'rxbvm']:
    data = run('predicate-' + vm, [binary / vm, work / 'linked.rxbin', '-a', target], 1)
    assert 'Speculative guard' not in data and "unexpected 'a'" in data, data
    target_count += 1

(out / 'receipt.json').write_text(json.dumps(dict(records=records, target_cases=target_count,
    scope='Four grammar engines plus Pratt/regions; exact byte/scalar/escaped-marker positions, EOF, context refusal and PEG predicates.'), indent=2) + '\n')
print(f'diagnostic_target_cases={target_count}; backends=6; failures=0')
