#!/usr/bin/env python3
"""Complete calculator and backend-choice lessons, using isolated current imports.

This is a focused explicit panel, not an added nested aggregate CTest.
Pass the complete no-opt surface build too to vary the underlying Level L core.
"""
import argparse
import hashlib
import json
import re
import shutil
import subprocess
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
p.add_argument('--part', choices=['all', 'calculator', 'backends'], default='all')
a = p.parse_args()
root = Path(__file__).resolve().parents[3]
binary, out = a.build.resolve() / 'bin', a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
commands, observations = [], []


def identity(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(name, argv, status=0):
    argv = list(map(str, argv))
    result = subprocess.run(argv, capture_output=True, timeout=1200)
    data = result.stdout + result.stderr
    (out / (name + '.log')).write_bytes(data)
    commands.append(dict(name=name, argv=argv, status=result.returncode,
                         log_sha256=hashlib.sha256(data).hexdigest()))
    assert result.returncode == status, (name, result.returncode, data[-2500:])
    assert not re.search(rb'Warning in |\bWARNING\b', data), (name, data[-2500:])
    return result.stdout.decode()


def generate(key, text, namespace, route='rxas', refusal=None, language_mode='opt'):
    work = out / key
    work.mkdir(exist_ok=True)
    path = work / 'input.levell'
    path.write_text(text)
    generated = run(key + '-generate', [binary / 'levell', path] +
                    (['--rxas'] if route == 'rxas' else []), 1 if refusal else 0)
    if refusal:
        assert all(value in generated for value in refusal), (key, generated)
        assert '.globals=' not in generated and 'namespace ' + namespace not in generated
        observations.append(dict(key=key, kind='build-refusal', required=refusal))
        return None
    imports = work / 'imports'
    imports.mkdir(exist_ok=True)
    for name in ['rxfnsl', 'library']:
        shutil.copy2(binary / (name + '.rxbin'), imports / (name + '.rxbin'))
    generated_path = work / (namespace + ('.rxas' if route == 'rxas' else '.crexx'))
    generated_path.write_text(generated)
    if route == 'source':
        run(key + '-compile-language', [binary / 'rxc', '-x', '--no-exe-import'] +
            (['--no-opt'] if language_mode == 'noopt' else []) + [
            '-i', imports, '-o', work / namespace, generated_path])
    run(key + '-assemble-language', [binary / 'rxas', work / namespace])
    shutil.copy2(work / (namespace + '.rxbin'), imports / (namespace + '.rxbin'))
    return work


def consumer(key, work, namespace, source, mode):
    output = work / ('consumer-' + mode)
    run(key + '-compile-' + mode, [binary / 'rxc', '-x', '--no-exe-import'] +
        (['--no-opt'] if mode == 'noopt' else []) +
        ['-i', work / 'imports', '-o', output, source])
    run(key + '-assemble-' + mode, [binary / 'rxas', output])
    linked = work / ('linked-' + mode)
    run(key + '-link-' + mode, [binary / 'rxlink', '-s', '-o', linked,
        output.with_suffix('.rxbin'), work / (namespace + '.rxbin'),
        binary / 'rxfnsl.rxbin', binary / 'library.rxbin'])
    return linked.with_suffix('.rxbin')


def observe(key, work, image, mode, cases):
    for name, text, status, required, forbidden in cases:
        path = work / (name + '.txt')
        path.write_bytes(text.encode())
        for vm in ['rxtvm', 'rxbvm']:
            data = run(key + '-' + mode + '-' + vm + '-' + name,
                       [binary / vm, image, '-a', path], status)
            assert all(value in data for value in required), (key, name, data)
            assert all(value not in data for value in forbidden), (key, name, data)
            observations.append(dict(key=key, mode=mode, vm=vm, case=name,
                                     status=status, input_sha256=identity(path)))


calculator = root / 'lib/rxfnsl/examples/calculator'
if a.part in ['all', 'calculator']:
    cases = []
    for name, text, value in [
        ('precedence', '2+3*4', '14'),
        ('grouping', '(2+3)*4', '20'),
        ('left-subtraction', '10-3-2', '5'),
        ('left-division', '8/2/2', '2'),
        ('prefix', '-2*3', '-6'),
        ('nested-prefix', '--3', '3'),
        ('decimal', '0.1+0.2', '0.3'),
        ('decimal-division', '7/2', '3.5'),
        ('nested-comments', '2(* outer (* inner *) end *)+3', '5'),
        ('multiline-comments', '2(*\nnotes\n*)+3', '5'),
        ('unicode-comments', '2(* café 日本 😀 *)+3', '5'),
        ('crlf-whitespace', '\t2 +\r\n3 ', '5'),
        ('documented', (calculator / 'example.calc').read_text(), '11'),
    ]:
        cases.append((name, text, 0, ['RESULT|' + value + '\n'], ['CALC_PARSE', 'CALC_VALUE']))
    for name, text, fragment, location in [
        ('bad-operator', '2+*3', "unexpected '*'", 'column 3 (byte 2)'),
        ('missing-close', '(2+3', 'closing delimiter for byte 0', 'column 5 (byte 4)'),
        ('trailing-input', '2 3', "unexpected '3'", 'column 3 (byte 2)'),
        ('empty', '', 'end of input', 'column 1 (byte 0)'),
        ('unclosed-comment', '2(* never closed', 'unclosed nested comment', 'column 2 (byte 1)'),
        ('lexical', '2+$', 'LL_LEXICAL', 'column 3 (byte 2)'),
    ]:
        cases.append((name, text, 1, ['CALC_PARSE', fragment, location], ['RESULT|']))
    cases.extend([
        ('zero', '7/0', 3, ['CALC_DIVIDE_BY_ZERO', 'column 3 (byte 2)'], ['RESULT|']),
        ('computed-zero', '7/(3-3)', 3, ['CALC_DIVIDE_BY_ZERO', 'column 3 (byte 2)'], ['RESULT|']),
        # Whole-input parsing wins over an earlier possible evaluation failure.
        ('parse-before-evaluate', '7/0 +', 1, ['CALC_PARSE', 'end of input'], ['RESULT|', 'CALC_DIVIDE_BY_ZERO']),
    ])
    for route in ['rxas', 'source']:
        for mode in ['opt', 'noopt']:
            key = 'calculator-' + route + '-' + mode
            work = generate(key, (calculator / 'calculator.levell').read_text(),
                            'levell_calculator', route, language_mode=mode)
            image = consumer(key, work, 'levell_calculator', calculator / 'calculator.crexx', mode)
            observe(key, work, image, mode, cases)


lessons = root / 'lib/rxfnsl/examples/backends'
if a.part in ['all', 'backends']:
    def select(text, backend):
        text = re.sub(r'backend (ll1|lr1|packrat|earley)', 'backend ' + backend, text)
        text = text.replace('semantics ordered ', '')
        if backend == 'packrat':
            text = text.replace('parser ', 'parser semantics ordered ', 1)
        return text

    fixtures = [
        ('predictive', ['ll1', 'lr1', 'packrat', 'earley'], [
            ('read', 'read port', 0, ['STATUS|0', 'NODE|command|0|9'], []),
            ('write', 'write port 3', 0, ['STATUS|0', 'NODE|command|0|12'], []),
            ('bad-start', 'port', 1, ['Expected read NAME or write NAME NUMBER', "found 'port'", 'column 1 (byte 0)'], []),
            ('missing-number', 'write port', 1, ['end of input', 'column 11 (byte 10)'], []),
        ]),
        ('line-comments', ['ll1', 'lr1', 'packrat', 'earley'], [
            ('multiple-styles', '1 # notes\r\n2 // notes\n3', 0,
             ['STATUS|0', 'NODE|number|0|1', 'NODE|number|11|12', 'NODE|number|22|23'], []),
            ('opaque-string', '"# // /* (*"', 0, ['STATUS|0', 'NODE|string|0|12'], []),
            ('doubled-quotes', '"a""#b" // café 😀', 0, ['STATUS|0', 'NODE|string|0|7'], []),
            ('line-eof', '1# no newline', 0, ['STATUS|0', 'NODE|number|0|1'], []),
            ('after-comments-error', '1# hidden $\n$', 1,
             ['LL_LEXICAL', 'line 2, column 1 (byte 12)'], []),
        ]),
        ('left-recursive-list', ['lr1', 'earley'], [
            ('one', 'a', 0, ['STATUS|0', 'NODE|list|0|1'], []),
            ('three', 'a,b,c', 0, ['STATUS|0', 'NODE|list|0|5'], []),
            ('eof', 'a,', 1, ['end of input', 'identifier', 'column 3 (byte 2)'], []),
        ]),
        ('palindrome', ['earley'], [
            ('empty', '', 0, ['STATUS|0', 'NODE|palindrome|0|0'], []),
            ('aa', 'aa', 0, ['STATUS|0', 'NODE|palindrome|0|2'], []),
            ('abba', 'abba', 0, ['STATUS|0', 'NODE|palindrome|0|4'], []),
            ('bad', 'abab', 1, ['LESSON_PARSE', 'unexpected'], []),
        ]),
        ('ambiguous-arithmetic', ['earley'], [
            ('one', '2', 0, ['STATUS|0', 'NODE|expression|0|1'], []),
            ('two-trees', '2+3*4', 5, ['STATUS|5', 'DERIVATIONS_CAPPED_AT_2|2', 'FOREST_VALID|1', 'LL_FOREST'], ['LESSON_PARSE', 'NODE|']),
            ('broken', '2+*4', 1, ["unexpected '*'", 'number', 'column 3 (byte 2)'], ['LL_FOREST']),
        ]),
    ]
    for fixture, backends, cases in fixtures:
        text = (lessons / (fixture + '.levell')).read_text()
        for backend in backends:
            key = fixture + '-' + backend
            work = generate(key, select(text, backend), 'levell_lesson')
            for mode in ['opt', 'noopt']:
                image = consumer(key, work, 'levell_lesson', lessons / 'inspect.crexx', mode)
                observe(key, work, image, mode, cases)

    prefix = (lessons / 'ordered-prefix.levell').read_text()
    for backend in ['lr1', 'earley', 'packrat']:
        for reverse in [False, True] if backend == 'packrat' else [False]:
            key = 'prefix-' + backend + ('-long-first' if reverse else '')
            text = select(prefix, backend)
            if reverse:
                text = text.replace('fragment: rule = a | a b', 'fragment: rule = a b | a')
            cases = [('short', 'ac', 0, ['STATUS|0'], [])]
            if backend == 'packrat' and not reverse:
                cases += [('long', 'abc', 1, ["unexpected 'b'", 'stop', 'column 2 (byte 1)'], [])]
            else:
                cases += [('long', 'abc', 0, ['STATUS|0'], [])]
            work = generate(key, text, 'levell_lesson')
            for mode in ['opt', 'noopt']:
                image = consumer(key, work, 'levell_lesson', lessons / 'inspect.crexx', mode)
                observe(key, work, image, mode, cases)

    for fixture in ['left-recursive-list', 'palindrome', 'ambiguous-arithmetic']:
        text = (lessons / (fixture + '.levell')).read_text()
        generate(fixture + '-refuse-ll1', select(text, 'll1'), 'levell_lesson',
                 refusal=['LL_BACKEND_ADMISSION:', 'll1'])
        if fixture != 'palindrome':
            generate(fixture + '-refuse-packrat', select(text, 'packrat'), 'levell_lesson',
                     refusal=['LL_BACKEND_ADMISSION:', 'packrat', 'left-corner cycle'])
        if fixture != 'left-recursive-list':
            generate(fixture + '-refuse-lr1', select(text, 'lr1'), 'levell_lesson',
                     refusal=['LL_LR_ADMISSION:', 'conflict'])
    generate('prefix-refuse-ll1', select(prefix, 'll1'), 'levell_lesson',
             refusal=['LL_BACKEND_ADMISSION:', 'll1', 'alternatives compete'])

sources = list(calculator.glob('*')) + list(lessons.glob('*')) + [Path(__file__).resolve()]
receipt = dict(commands=commands, observations=observations,
    source_sha256={str(path.relative_to(root)): identity(path) for path in sources if path.is_file()},
    product_sha256={str(path): identity(path) for path in [binary / (name + '.rxbin')
                   for name in ['rxfnsl', 'library']]},
    scope='Existing Level L syntax; native both-VM optimized/unoptimized consumers; no general forest resolver or generalized-comment implementation claimed.')
(out / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(f'observations={len(observations)}; commands={len(commands)}; failures=0')
