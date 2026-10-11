#!/usr/bin/env python3
"""Current bootstrap tutorial/diagnostic checks; preserve the earlier audit."""
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[3]
BIN = ROOT / 'cmake-build-levell-product/bin'
WORK = ROOT / 'research/level-l/build/user-guide'
OUT = ROOT / 'research/level-l/evidence/bootstrap/user-guide'
EXAMPLES = ROOT / 'lib/rxfnsl/examples'
WORK.mkdir(parents=True, exist_ok=True)
OUT.mkdir(parents=True, exist_ok=True)
records = []


def run(name, command, status=0, contains=()):
    proc = subprocess.run([str(x) for x in command], cwd=ROOT,
                          capture_output=True, timeout=60)
    output = proc.stdout + proc.stderr
    (OUT / (name + '.out')).write_bytes(output)
    assert proc.returncode == status, (name, proc.returncode, output[-3000:])
    for part in contains:
        assert part.encode() in output, (name, part, output[-3000:])
    records.append(dict(case=name, exit=proc.returncode,
                        command=[str(x) for x in command],
                        output_sha256=hashlib.sha256(output).hexdigest()))
    return proc.stdout


def generate(name, source, status=0, contains=()):
    path = WORK / (name + '.levell')
    path.write_bytes(source if isinstance(source, bytes) else source.encode())
    return run(name, [BIN / 'levell', path, '--rxas'], status, contains)


def build_reader(name, artifact):
    (WORK / 'levell_settings.rxas').write_bytes(artifact)
    run(name + '-assemble-grammar', [BIN / 'rxas', WORK / 'levell_settings'])
    run(name + '-compile', [BIN / 'rxc', '-x', '--no-exe-import', '-i',
        f'{BIN};{WORK}', '-o', WORK / 'settings-reader', EXAMPLES / 'settings-reader.crexx'])
    run(name + '-assemble-reader', [BIN / 'rxas', WORK / 'settings-reader'])
    image = WORK / name
    run(name + '-link', [BIN / 'rxlink', '-o', image,
        WORK / 'settings-reader.rxbin', WORK / 'levell_settings.rxbin',
        BIN / 'rxfnsl.rxbin', BIN / 'library.rxbin'])
    return image.with_suffix('.rxbin')


spec = (EXAMPLES / 'settings.levell').read_text()
artifact = run('tutorial-generate', [BIN / 'levell', EXAMPLES / 'settings.levell', '--rxas'])
image = build_reader('settings-reader-linked', artifact)
expected = 'retries -> 3\ngreeting -> "café"\nnote -> "say ""hello"""\n'.encode()
for vm in ['rxtvm', 'rxbvm']:
    assert run('tutorial-' + vm, [BIN / vm, image, '-a', EXAMPLES / 'settings.txt']) == expected

for name, source, message in [
    ('grammar-undefined', spec.replace('value: rule = number | string', 'value: rule = absent_one'), 'undefined symbol absent_one'),
    ('grammar-two-errors', spec.replace('name.value: rule = identifier', 'name.value: rule = absent_one').replace('value: rule = number | string', 'value: rule = absent_two'), 'undefined symbol absent_one'),
    ('grammar-duplicate', spec + 'identifier: token = 9\n', 'duplicate declaration identifier'),
    ('grammar-unclosed-group', spec.replace('value: rule = number | string', 'value: rule = (number | string'), 'LL_SYNTAX'),
    ('grammar-unclosed-quote', spec + "bad: diagnostic = 'unfinished\n", 'LL_LEXICAL'),
    ('grammar-cr-lines', spec.replace('value: rule = number | string', 'value: rule = absent_one').replace('\n', '\r'),
     f"line {spec[:spec.index('value: rule = number | string')].count(chr(10)) + 1},"),
]:
    output = generate(name, source, 1, [message])
    if name == 'grammar-two-errors':
        assert b'absent_two' not in output and output.count(b'LL_GRAMMAR:') == 1

for name, message in [('nullable', 'LL_NULLABLE_TOKEN'), ('recursive-lexer', 'LL_LEX_RECURSION'),
                      ('conflict', 'LL_LR_CONFLICT'), ('ordered', 'LL_BACKEND_CAPABILITY')]:
    run('grammar-' + name, [BIN / 'levell', ROOT / 'lib/rxfnsl/tests_functional/grammars' / (name + '.levell'), '--rxas'], 1, [message])

target_cases = [
    ('missing-value', b'first =\nsecond = 2\n', 'expected number, string'),
    ('missing-equals', b'first 3\nsecond = 2\n', 'expected equals'),
    ('missing-separator', b'first = 3 second = 2\n', 'separator'),
    ('unknown-byte', b'first = @\nsecond = 2\n', 'LL_LEXICAL'),
    ('unclosed-string', b'first = "unfinished\nsecond = 2\n', 'LL_LEXICAL'),
    ('unclosed-comment', b'first = 1\n/* unfinished\nsecond = 2\n', 'LL_COMMENT'),
    ('invalid-utf8', b'first = "\xff"\nsecond = 2\n', 'source bytes:'),
    ('two-errors', b'first =\nsecond =\nthird = 3\n', 'expected number, string'),
    ('lexical-preempts-syntax', b'first =\nsecond = @\nthird = 3\n', 'line 2, column 10'),
    ('cr-locations', b'first = 1\rsecond =\rthird = 3\r', 'line 2, column 9'),
]
for name, source, message in target_cases:
    path = WORK / (name + '.txt')
    path.write_bytes(source)
    for vm in ['rxtvm', 'rxbvm']:
        output = run('target-' + name + '-' + vm, [BIN / vm, image, '-a', path], 1, [message, 'SETTINGS_PARSE'])
        assert b' -> ' not in output, (name, output)
        assert output.count(b'SETTINGS_PARSE at') == 1, (name, output)

# Lexical messages are now emitted and used; tree annotations remain a gap.
custom = spec.replace('scan: lexer', "bad.character: diagnostic = 'CUSTOM_LEXICAL_MESSAGE'\nscan: lexer")
custom = custom.replace('parse: parser', 'bad.rule: rule = otherwise then error bad.character\nparse: parser')
custom_artifact = generate('grammar-custom-diagnostic', custom)
assert b'CUSTOM_LEXICAL_MESSAGE' in custom_artifact
custom_image = build_reader('settings-custom-linked', custom_artifact)
output = run('target-custom-diagnostic', [BIN / 'rxtvm', custom_image, '-a', WORK / 'unknown-byte.txt'], 1, ['bad.character: CUSTOM_LEXICAL_MESSAGE'])
tree_artifact = generate('grammar-tree-declaration', spec + 'custom.tree: tree rule name.value kind 42 arity 1\n')
assert artifact == tree_artifact, 'tree declaration now affects emission; revise the audit'

# Real first consumer: later statements must not execute following parse failure.
rs = WORK / 'two-errors.rexx'
rs.write_text('x = ;\ny = ;\nsay "EXECUTED_LATER_STATEMENT"\n')
output = run('rexxscript-two-errors', [BIN / 'rexxscript1', rs], 1, ['RS_PARSE'])
assert b'EXECUTED_LATER_STATEMENT' not in output

# Restore the tutorial artifact for the documented commands.
(WORK / 'levell_settings.rxas').write_bytes(artifact)
run('restore-tutorial-grammar', [BIN / 'rxas', WORK / 'levell_settings'])
hashes = {}
for path in [*sorted(EXAMPLES.glob('*')), Path(__file__),
             *[BIN / x for x in ['levell', 'rexxscript1', 'rxc', 'rxas', 'rxlink', 'rxtvm', 'rxbvm', 'rxfnsl.rxbin', 'library.rxbin']],
             *sorted((ROOT / 'lib/rxfnsl/rexx').glob('ll*.crexx')),
             ROOT / 'lib/rxfnsl/tools/levell.crexx']:
    hashes[str(path.relative_to(ROOT))] = hashlib.sha256(path.read_bytes()).hexdigest()
(OUT / 'receipt.json').write_text(json.dumps({'scope': 'local tutorial and current diagnostic audit; no recovery implemented',
    'records': records, 'sha256': hashes}, indent=2) + '\n')
print(f'user_guide_commands={len(records)}; failures=0; evidence={OUT}')
