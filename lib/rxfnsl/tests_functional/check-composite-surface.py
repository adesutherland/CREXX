#!/usr/bin/env python3
"""Authored Pratt/region trees, unsafe composition and diagnostic controls."""
import argparse
import json
from pathlib import Path
import subprocess

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[3]
build = a.build.resolve()
bin_dir = build / 'bin'
out = a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
commands = []
records = []


def run(name, args, expected=0):
    args = list(map(str, args))
    r = subprocess.run(args, capture_output=True, timeout=1200)
    (out / (name + '.log')).write_bytes(r.stdout + r.stderr)
    commands.append(dict(name=name, argv=args, status=r.returncode))
    assert r.returncode == expected, (name, r.returncode, r.stdout[-2500:], r.stderr[-1000:])
    assert b'Warning in ' not in r.stdout + r.stderr, (name, r.stdout, r.stderr)
    return r.stdout


base = (root / 'lib/rxfnsl/examples/settings/05-composite.levell').read_text()
lexical = base[:base.index('parse: parser')]
ops = base[base.index('arithmetic: operators'):base.index('math.region:')]
pratt = lexical + 'parse: parser start arithmetic backend pratt\n' + ops
simple = (root / 'lib/rxfnsl/examples/settings/01-basic.levell').read_text()
simple_lex = simple[:simple.index('parse: parser')]
variants = {
    'regional': (base, [(b'workers = [2+3*4]', 0, ['sum', 'number', 'product', 'number', 'number']),
                         (b'workers = [-(2+3)*4]', 0, ['product', 'negative', 'group', 'sum', 'number', 'number', 'number']),
                         (b'workers = [2+*3]', 1, b'embedded expression opened at byte 10'),
                         (b'workers = [2+3', 1, b'missing region closer'),
                         (b'workers = many', 1, b'value.error')]),
    'pratt': (pratt, [(b'2+3*4', 0, ['arithmetic', 'sum', 'number', 'product', 'number', 'number']),
                       (b'-(2+3)*4', 0, ['arithmetic', 'product', 'negative', 'group', 'sum', 'number', 'number', 'number']),
                       (b'2+*3', 1, b'expected atom'), (b'(2+3', 1, b'closing delimiter for byte 0')]),
    'forest': (simple_lex + 'parse: parser start root backend earley\nroot: rule = one | two\none: rule = number\ntwo: rule = number\n',
               [(b'1', 5, b'DERIVATIONS|2')]),
    'shared-context': (simple_lex + '''parse: parser start root
root: rule = first string | second separator
first: rule = number equals
second: rule = number equals
first.error: diagnostic rule first = 'ONLY_FIRST {found}'
''', [(b'1 many', 1, b'expected equals')]),
    'predicate': (simple_lex + '''parse: parser start root semantics ordered backend packrat
root: rule = !forbidden number
forbidden: rule = string
hidden.error: diagnostic rule forbidden = 'HIDDEN_PREDICATE {found}'
''', [(b'many', 1, b'expected number')]),
}

for variant, (spec, cases) in variants.items():
    work = out / variant
    work.mkdir(exist_ok=True)
    path = work / 'input.levell'
    path.write_text(spec)
    for route in ['rxas', 'source']:
        key = variant + '-' + route
        generated = work / ('levell_settings.' + ('rxas' if route == 'rxas' else 'crexx'))
        generated.write_bytes(run(key + '-generate', [bin_dir / 'levell', path, *(['--rxas'] if route == 'rxas' else [])]))
        for mode in ['opt', 'noopt']:
            flags = ['-n'] if mode == 'noopt' else []
            cell = key + '-' + mode
            if route == 'source':
                run(cell + '-compile-language', [bin_dir / 'rxc', *flags, '-x', '--no-exe-import', '-i', f'{work};{bin_dir}',
                    '-o', work / 'levell_settings', generated])
            run(cell + '-assemble-language', [bin_dir / 'rxas', *flags, work / 'levell_settings'])
            run(cell + '-compile-probe', [bin_dir / 'rxc', *flags, '-x', '--no-exe-import', '-i', f'{work};{bin_dir}',
                '-o', work / 'probe', root / 'lib/rxfnsl/tests_functional/surface-probe.crexx'])
            run(cell + '-assemble-probe', [bin_dir / 'rxas', *flags, work / 'probe'])
            run(cell + '-link', [bin_dir / 'rxlink', '-s', '-o', work / 'linked', work / 'probe.rxbin',
                work / 'levell_settings.rxbin', bin_dir / 'rxfnsl.rxbin', bin_dir / 'library.rxbin'])
            for vm in ['rxtvm', 'rxbvm']:
                for i, (source, status, expected) in enumerate(cases):
                    target = work / (str(i) + '.txt')
                    target.write_bytes(source)
                    observed = run(cell + '-' + vm + '-' + str(i), [bin_dir / vm, work / 'linked.rxbin', '-a', target], status)
                    if isinstance(expected, list):
                        names = [line.split('|')[2] for line in observed.decode().splitlines() if line.startswith('NODE|')]
                        if variant == 'regional':
                            names = names[names.index('expression') + 1:-1]  # expression subtree, then empty tail
                        assert names == expected, (cell, source, names, expected, observed)
                    else:
                        assert expected in observed, (cell, source, observed)
                    assert b'ONLY_FIRST' not in observed and b'HIDDEN_PREDICATE' not in observed, (cell, observed)
                    records.append(dict(variant=variant, route=route, mode=mode, vm=vm, case=i, status=status))
                    (out / 'progress.json').write_text(json.dumps(dict(records=records, commands=commands), indent=2) + '\n')

negative = {
    'unsafe-inner-boundary': (base.replace('open lbracket close rbracket', 'open lparen close rparen'), b'boundary/virtual token belongs to inner grammar'),
    'unsafe-outer-boundary': (base.replace('value: rule = number | string | expression', 'value: rule = number | string | expression | lbracket'), b'boundary token also belongs to outer grammar'),
    'raw-virtual': (base.replace("'[' then emit lbracket", "'[' then emit expression"), b'virtual region token must not be emitted'),
    'unknown-operator-component': (base.replace('using arithmetic', 'using unknown'), b'unknown operators component'),
    'pratt-arbitrary-grammar': (simple.replace('parser start settings', 'parser start settings backend pratt'), b'start must name an operators declaration'),
    'mixed-associativity': (base.replace('times precedence 20 left', 'times precedence 10 right'), b'mixed associativity'),
}
for name, (spec, expected) in negative.items():
    path = out / (name + '.levell')
    path.write_text(spec)
    observed = run(name, [bin_dir / 'levell', path, '--rxas'], 1)
    assert expected in observed, (name, observed)

(out / 'receipt.json').write_text(json.dumps(dict(records=records, commands=commands), indent=2) + '\n')
print(f'composite_cases={len(records)}; admission_negatives={len(negative)}; failures=0')
