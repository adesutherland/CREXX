#!/usr/bin/env python3
"""Focused Level L AST surface panel. No nested aggregate CTest registration.

Run BUILD OUTPUT; retains command logs, observations and input identities.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
p.add_argument('--diagnostics-only', action='store_true')
a = p.parse_args()
root = Path(__file__).resolve().parents[3]
binary = a.build.resolve() / 'bin'
out = a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
records, observations = [], []
equivalent_outputs = {}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(key, args, status=0):
    result = subprocess.run(list(map(str, args)), capture_output=True, timeout=1200)
    data = result.stdout + result.stderr
    path = out / (key + '.log')
    path.write_bytes(data)
    records.append(dict(key=key, args=list(map(str, args)), status=result.returncode,
                        sha256=sha(path)))
    assert result.returncode == status, (key, result.returncode, data[-3000:])
    assert not re.search(rb'Warning in |\bWARNING\b', data), (key, data[-3000:])
    return result.stdout.decode('utf-8')


def build(key, text, namespace, source, route='rxas', mode='opt'):
    work = out / key
    work.mkdir(exist_ok=True)
    spec = work / 'grammar.rxl'
    spec.write_text(text, encoding='utf-8')
    data = run(key + '-generate', [binary / 'levell', spec] + (['--rxas'] if route == 'rxas' else []))
    generated = work / (namespace + ('.rxas' if route == 'rxas' else '.crexx'))
    generated.write_text(data, encoding='utf-8')
    imports = work / 'imports'
    imports.mkdir(exist_ok=True)
    for name in ['rxfnsl', 'library']:
        shutil.copyfile(binary / (name + '.rxbin'), imports / (name + '.rxbin'))
    flags = ['-n'] if mode == 'noopt' else []
    if route == 'source':
        run(key + '-grammar-compile', [binary / 'rxc', '-x', '--no-exe-import', *flags,
                                      '-i', imports, '-o', work / namespace, generated])
    run(key + '-grammar-assemble', [binary / 'rxas', *flags, work / namespace])
    shutil.copyfile(work / (namespace + '.rxbin'), imports / (namespace + '.rxbin'))
    run(key + '-compile', [binary / 'rxc', '-x', '--no-exe-import', *flags, '-i', imports,
                          '-o', work / 'consumer', source])
    run(key + '-assemble', [binary / 'rxas', *flags, work / 'consumer'])
    run(key + '-link', [binary / 'rxlink', '-s', '-o', work / 'linked', work / 'consumer.rxbin',
                       work / (namespace + '.rxbin'), binary / 'rxfnsl.rxbin', binary / 'library.rxbin'])
    return work


def observe(key, work, cases, equivalent=False):
    for name, text, required, *status in cases:
        path = work / (name + '.input')
        path.write_bytes(text.encode())
        for vm in ['rxtvm', 'rxbvm']:
            data = run(key + '-' + name + '-' + vm, [binary / vm, work / 'linked.rxbin', '-a', path], status[0] if status else 0)
            assert all(value in data for value in required), (key, name, data, required)
            if equivalent and name != 'invalid':
                expected = equivalent_outputs.setdefault(name, data)
                assert data == expected, (key, name, 'backend/route/mode disagreement', data, expected)
            observations.append(dict(key=key, case=name, vm=vm, sha256=sha(path), output=data))


fixtures = root / 'lib/rxfnsl/tests_functional'
base = (fixtures / 'ast-fixture.rxl').read_text(encoding='utf-8')
cases = [
    ('empty', '', ['PARSE|0', 'AST|0|', 'ITEMS|0', 'COPY|0|program']),
    ('list', 'a=1;b=;c=23;', ['ITEMS|3', 'ENTRY|a|1', 'ENTRY|b|0', 'ENTRY|c|1', 'NUMBER|23', 'COPY|0|program']),
    ('unicode', 'é=1; /* β */β=2;', ['NODE|Entry|0|5', 'ENTRY|é|1', 'ENTRY|β|1', 'COPY|0|program']),
    ('syntax', 'a=;', ['PARSE|0', 'AST|0|', 'ENTRY|a|0']),
    ('invalid', 'a=1', ['PARSE|1', 'AST|3|LL_AST_PARSE:']),
]
calculator = root / 'lib/rxfnsl/examples/calculator'
settings = root / 'lib/rxfnsl/examples/settings'
if not a.diagnostics_only:
    for backend in ['ll1', 'lr1', 'packrat', 'earley']:
        text = base.replace('backend lr1', 'backend ' + backend)
        if backend == 'packrat':
            text = text.replace('start program backend', 'start program semantics ordered backend')
        for route in ['rxas', 'source']:
            for mode in ['opt', 'noopt']:
                key = '-'.join([backend, route, mode])
                work = build(key, text, 'ast_fixture', fixtures / 'ast-probe.crexx', route, mode)
                observe(key, work, cases, equivalent=True)

    for backend in ['ll1', 'lr1', 'packrat', 'earley']:
        text = base.replace('backend lr1', 'backend ' + backend)
        if backend == 'packrat':
            text = text.replace('start program backend', 'start program semantics ordered backend')
        text = text.replace('(statement as item)*', '(statement as item)+').replace('items = item*', 'items = item+')
        key = backend + '-nonempty'
        work = build(key, text, 'ast_fixture', fixtures / 'ast-probe.crexx')
        observe(key, work, [('one', 'a=1;', ['ITEMS|1', 'COPY|0|program']),
                            ('empty', '', ['PARSE|1', 'AST|3|LL_AST_PARSE:'])])

    # A forest is never resolved as a side effect of asking for an AST.
    text = base[:base.index('parse: parser')] + '''parse: parser start program backend earley
    program: rule = pair pair
    pair: rule = number | number number
    '''
    work = build('forest', text, 'ast_fixture', fixtures / 'ast-probe.crexx')
    observe('forest', work, [('ambiguous', '1 2 3', ['PARSE|5', 'AST|3|LL_AST_PARSE:'])])

    # Compare ordinary operator lowering and dedicated Pratt, including grouping.
    calculator = root / 'lib/rxfnsl/examples/calculator'
    text = (calculator / 'calculator.rxl').read_text(encoding='utf-8')
    for backend in ['pratt', 'lr1', 'earley']:
        for program in ['calculator', 'fold']:
            key = backend + '-' + program
            work = build(key, text.replace('backend pratt', 'backend ' + backend),
                         'levell_calculator', calculator / (program + '.crexx'))
            observe(key, work, [('expression', '2+3*(4-1)', ['RESULT|11']),
                                ('prefix', '-(2+3)*4', ['RESULT|-20']),
                                ('divide-zero', '7/(3-3)', ['CALC_DIVIDE_BY_ZERO', 'column 3 (byte 2)'], 3)])

    # The progressive settings example combines fields, Unicode, comments and Pratt.
    settings = root / 'lib/rxfnsl/examples/settings'
    work = build('settings', (settings / '09-ast.rxl').read_text(encoding='utf-8'), 'levell_settings',
                 settings.parent / 'settings-ast.crexx')
    observe('settings', work, [('unicode-composite', 'é=2;\nβ=[2+3*4] /* note */', ['é -> 2', 'β -> [2+3*4]'])])

# Admission errors must be located and must not publish a generated module.
for name, text, code, detail in [
    ('unknown', base.replace('value = value?', 'value = absent?'), 'LL_AST_CAPTURE', 'unknown capture absent'),
    ('required', base.replace('value = value?', 'value = value'), 'LL_AST_CARDINALITY', 'may be absent'),
    ('list-required', base.replace('items = item*', 'items = item'), 'LL_AST_CARDINALITY', 'may match more than once'),
    ('nonempty', base.replace('items = item*', 'items = item+'), 'LL_AST_CARDINALITY', 'but capture item may be absent'),
    ('nested-scope', base.replace('statement: rule = name as key equals (number as value)? semi', 'wrapped: rule = name as key equals (number as value)? semi\nstatement: rule = wrapped'), 'LL_AST_CAPTURE', 'unknown capture key'),
    ('duplicate', base.replace('items = item*', 'items = item*, items = item*'), 'LL_AST_SCHEMA', 'duplicate field'),
    ('text-list', base.replace('name = text(key)', 'name = text(key)*'), 'LL_AST_SCHEMA', 'requires one source'),
    ('conflicting-kind', base.replace('tree Entry(', 'tree Number('), 'LL_AST_SCHEMA', 'conflicting'),
    ('regional-wrapper', (settings / '09-ast.rxl').read_text(encoding='utf-8').replace('arithmetic: operators atom number group lparen rparen', 'arithmetic: operators atom number group lparen rparen then tree Wrapped(value = inner)'), 'LL_AST_REGION', 'enclosing parser rule'),
]:
    path = out / (name + '.rxl')
    path.write_text(text, encoding='utf-8')
    for stage in ['L0', 'L1', 'L2']:
        driver = a.build.resolve() / 'lib/rxfnsl/tools/bootstrap' / stage / 'levell-linked.rxbin'
        data = run(name + '-' + stage, [binary / 'rxtvm', driver, '-a', path, '--rxas'], 1)
        location = re.search(r'line (\d+), byte (\d+):', data)
        assert code in data and detail in data and location, (name, stage, data)
        declaration = 'program:' if name in ['list-required', 'nonempty', 'duplicate'] else 'statement:'
        if name == 'regional-wrapper':
            declaration = 'arithmetic:'
        start = text.index('then tree', text.index(declaration))
        expected_line = text[:start].count('\n') + 1
        line_start = text.rfind('\n', 0, start) + 1
        line_end = text.find('\n', start)
        assert int(location[1]) == expected_line and line_start <= int(location[2]) <= line_end, (name, stage, data, expected_line)
        assert '.globals=' not in data, (name, stage, data)
        observations.append(dict(key=name, stage=stage, output=data))

receipt = dict(commands=records, observations=observations,
               sources={str(path.relative_to(root)): sha(path) for path in [
                   fixtures / 'ast-fixture.rxl', fixtures / 'ast-probe.crexx', Path(__file__).resolve(),
                   calculator / 'calculator.rxl', calculator / 'calculator.crexx', calculator / 'fold.crexx',
                   settings / '09-ast.rxl', settings.parent / 'settings-ast.crexx']},
               tools={name: sha(binary / name) for name in ['levell', 'rxfnsl.rxbin', 'rxc', 'rxas', 'rxlink', 'rxtvm', 'rxbvm']})
(out / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
print(f'PASS: {len(observations)} AST observations; {len(records)} commands; {out / "receipt.json"}')
