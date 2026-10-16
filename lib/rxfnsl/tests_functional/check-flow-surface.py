#!/usr/bin/env python3
"""Explicit properties/flow qualification matrix; not a nested aggregate CTest.

Run BUILD OUTPUT. Retains exact commands, logs and input identities.
"""
import argparse
import hashlib
import json
import shutil
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('build', type=Path)
parser.add_argument('output', type=Path)
args = parser.parse_args()
root = Path(__file__).resolve().parents[3]
binary = args.build.resolve() / 'bin'
output = args.output.resolve()
output.mkdir(parents=True, exist_ok=True)
records = []


def run(key, command, status=0):
    command = list(map(str, command))
    result = subprocess.run(command, capture_output=True, timeout=1200)
    data = result.stdout + result.stderr
    path = output / (key + '.log')
    path.write_bytes(data)
    records.append(dict(key=key, command=command, status=result.returncode,
                        sha256=hashlib.sha256(data).hexdigest()))
    assert result.returncode == status, (key, result.returncode, data[-4000:])
    assert b'Warning in ' not in data, (key, data[-4000:])
    return (result.stdout if status == 0 else data).decode()


base = (root / 'lib/rxfnsl/examples/flow/rules.rxl').read_text()
consumer = root / 'lib/rxfnsl/tests_functional/ts_llflow.crexx'
for backend in ['ll1', 'lr1', 'packrat', 'earley']:
    text = base.replace('backend ll1', 'backend ' + backend)
    if backend == 'packrat':
        text = text.replace('semantics contextfree', 'semantics ordered')
    for route in ['source', 'rxas']:
        for mode in ['opt', 'noopt']:
            key = '-'.join([backend, route, mode])
            work = output / key
            imports = work / 'imports'
            imports.mkdir(parents=True, exist_ok=True)
            for module in ['library', 'rxfnsl']:
                shutil.copyfile(binary / (module + '.rxbin'), imports / (module + '.rxbin'))
            spec = work / 'rules.rxl'
            spec.write_text(text)
            generated = run(key + '-generate', [binary / 'levell', spec] + (['--rxas'] if route == 'rxas' else []))
            source = work / ('flowdemo.rxas' if route == 'rxas' else 'flowdemo.crexx')
            source.write_text(generated)
            flags = ['-n'] if mode == 'noopt' else []
            if route == 'source':
                run(key + '-provider-compile', [binary / 'rxc', '-x', '--no-exe-import', *flags,
                    '-i', imports, '-o', work / 'flowdemo', source])
            run(key + '-provider-assemble', [binary / 'rxas', *flags, work / 'flowdemo'])
            shutil.copyfile(work / 'flowdemo.rxbin', imports / 'flowdemo.rxbin')
            run(key + '-compile', [binary / 'rxc', '-x', '--no-exe-import', *flags,
                '-i', imports, '-o', work / 'consumer', consumer])
            run(key + '-assemble', [binary / 'rxas', *flags, work / 'consumer'])
            run(key + '-link', [binary / 'rxlink', '-s', '-o', work / 'linked', work / 'consumer',
                work / 'flowdemo', binary / 'rxfnsl', binary / 'library'])
            for vm in ['rxtvm', 'rxbvm']:
                result = run(key + '-' + vm, [binary / vm, work / 'linked.rxbin'])
                assert result == 'failures=0\n', (key, vm, result)

# Literal codec and negative defaults must agree in source and packed providers.
default_consumer = output / 'defaults.crexx'
default_consumer.write_text("""options levelb
import flowdemo
import lllanguage
import llast
language = flowdemo..language()
request = .lllanguage..request(language,''x as .binary)
if request.parse() <> 0 then exit 1
tree = request.ast()
draft = .flowdemo..notes(reference tree)
notes = draft.finish()
view = notes.at(tree.root())
say view.getmessage()
say view.getpriority()
say view.hasmessage() view.haspriority() notes.size()
""")
for route in ['source', 'rxas']:
    key = 'literal-defaults-' + route
    work = output / key
    imports = work / 'imports'
    imports.mkdir(parents=True, exist_ok=True)
    for module in ['library', 'rxfnsl']:
        shutil.copyfile(binary / (module + '.rxbin'), imports / (module + '.rxbin'))
    spec = work / 'rules.rxl'
    spec.write_text(base.replace(".string default ''", ".string default 'café O''Reilly\\rules'")
                   .replace('.int default 0', '.int default -9223372036854775808'))
    generated = run(key + '-generate', [binary / 'levell', spec] + (['--rxas'] if route == 'rxas' else []))
    artifact = work / ('flowdemo.rxas' if route == 'rxas' else 'flowdemo.crexx')
    artifact.write_text(generated)
    if route == 'source':
        run(key + '-provider-compile', [binary / 'rxc', '-x', '--no-exe-import', '-i', imports, '-o', work / 'flowdemo', artifact])
    run(key + '-provider-assemble', [binary / 'rxas', work / 'flowdemo'])
    shutil.copyfile(work / 'flowdemo.rxbin', imports / 'flowdemo.rxbin')
    run(key + '-compile', [binary / 'rxc', '-x', '--no-exe-import', '-i', imports, '-o', work / 'consumer', default_consumer])
    run(key + '-assemble', [binary / 'rxas', work / 'consumer'])
    run(key + '-link', [binary / 'rxlink', '-s', '-o', work / 'linked', work / 'consumer', work / 'flowdemo', binary / 'rxfnsl', binary / 'library'])
    for vm in ['rxtvm', 'rxbvm']:
        observed = run(key + '-' + vm, [binary / vm, work / 'linked.rxbin'])
        assert observed == "café O'Reilly\\rules\n-9223372036854775808\n0 0 0\n", observed

# Pratt uses the same services after operator-tree construction.
pratt = (root / 'lib/rxfnsl/examples/calculator/calculator.rxl').read_text()
pratt += """
review: properties
  message: property .string default ''
execution: flow
  Number: value
  Add: evaluate left right
  Subtract: evaluate left right
  Multiply: evaluate left right
  Divide: evaluate left right
  Negate: evaluate value
"""
pratt_consumer = output / 'pratt.crexx'
pratt_consumer.write_text("""options levelb
import levell_calculator
import lllanguage
import llast
import llflow
language = levell_calculator..language()
request = .lllanguage..request(language,'1 + 2 * -3' as .binary)
if request.parse() <> 0 then exit 1
tree = request.ast()
graph = levell_calculator..execution(reference tree)
result = graph.definiteassignment('host')
if result.error() <> '' then exit 2
if result.count() <> 0 then exit 3
if result.before(tree.root()).assigned('host') <> 1 then exit 4
spellings = ''; point = graph.start()
do while point > 0
  node = graph.node(point)
  if node.kind() = 'Number' then spellings = spellings || node.text('spelling')
  edge = graph.firstedge(point)
  if edge = 0 then leave
  point = graph.destination(edge)
end
if spellings <> '123' then exit 5
draft = .levell_calculator..review(reference tree)
call draft.setmessage(tree.root(),'checked')
review = draft.finish()
say tree.root().kind() review.at(tree.root()).getmessage()
""")
for route in ['source', 'rxas']:
    key = 'pratt-' + route
    work = output / key
    imports = work / 'imports'
    imports.mkdir(parents=True, exist_ok=True)
    for module in ['library', 'rxfnsl']:
        shutil.copyfile(binary / (module + '.rxbin'), imports / (module + '.rxbin'))
    spec = work / 'rules.rxl'
    spec.write_text(pratt)
    generated = run(key + '-generate', [binary / 'levell', spec] + (['--rxas'] if route == 'rxas' else []))
    artifact = work / ('levell_calculator.rxas' if route == 'rxas' else 'levell_calculator.crexx')
    artifact.write_text(generated)
    if route == 'source':
        run(key + '-provider-compile', [binary / 'rxc', '-x', '--no-exe-import', '-i', imports, '-o', work / 'levell_calculator', artifact])
    run(key + '-provider-assemble', [binary / 'rxas', work / 'levell_calculator'])
    shutil.copyfile(work / 'levell_calculator.rxbin', imports / 'levell_calculator.rxbin')
    run(key + '-compile', [binary / 'rxc', '-x', '--no-exe-import', '-i', imports, '-o', work / 'consumer', pratt_consumer])
    run(key + '-assemble', [binary / 'rxas', work / 'consumer'])
    run(key + '-link', [binary / 'rxlink', '-s', '-o', work / 'linked', work / 'consumer', work / 'levell_calculator', binary / 'rxfnsl', binary / 'library'])
    for vm in ['rxtvm', 'rxbvm']:
        observed = run(key + '-' + vm, [binary / vm, work / 'linked.rxbin'])
        assert observed == 'Add checked\n', observed

# All stages share located semantic checks; compare exact messages after the path.
negatives = [
    ('service-name', base.replace('notes: properties', 'language: properties'), 'LL_AST_SERVICE: group name'),
    ('view-collision', base.replace('execution: flow', 'notes_entry: flow'), 'LL_AST_SERVICE: name conflicts'),
    ('property-name', base.replace('message: property', 'message.text: property'), 'LL_PROPERTY: property name'),
    ('duplicate-property', base.replace('  priority: property', "  message: property .string default ''\n  priority: property"), 'LL_PROPERTY: duplicate property message'),
    ('unknown-type', base.replace('property .int', 'property .float'), 'LL_PROPERTY: supported property types'),
    ('string-default', base.replace(".string default ''", '.string default 0'), 'LL_PROPERTY: .string default requires a quoted literal'),
    ('int-default', base.replace('.int default 0', ".int default 'zero'"), 'LL_PROPERTY: .int default requires an integer literal'),
    ('overflow', base.replace('.int default 0', '.int default 9223372036854775808'), 'LL_PROPERTY: priority default must be a representable integer'),
    ('missing-kind', base.replace('  Number: value\n', ''), 'LL_FLOW: missing mapping for AST kind Number'),
    ('unknown-kind', base.replace('  Number: value', '  Numer: value'), 'LL_FLOW: unknown AST kind Numer'),
    ('field-name', base.replace('branch condition then', 'branch test then'), 'LL_FLOW: If has no field test; available fields: condition yes no'),
    ('field-type', base.replace('Assignment: assign name value', 'Assignment: assign value name'), 'LL_FLOW: Assignment.value must be text'),
    ('ignored-child', base.replace('Say: evaluate value', 'Say: value'), 'LL_FLOW: Say.value has no execution role'),
    ('duplicate-role', base.replace('Say: evaluate value', 'Say: evaluate value value'), 'LL_FLOW: Say.value occurs twice'),
    ('optional-condition', base.replace('branch condition then yes else no', 'branch no then yes else condition'), 'LL_FLOW: If.no must contain one child'),
    ('duplicate-kind', base.replace('  Number: value', '  Number: value\n  Number: value'), 'LL_FLOW: duplicate mapping for Number'),
]
anchors = {
    'service-name': 'language: properties', 'view-collision': 'notes_entry: flow',
    'property-name': 'message.text: property', 'duplicate-property': 'message: property',
    'unknown-type': '.float', 'string-default': '0', 'int-default': "'zero'",
    'overflow': 'priority: property', 'missing-kind': 'execution: flow',
    'unknown-kind': 'Numer: value', 'field-name': 'If: branch',
    'field-type': 'Assignment: assign', 'ignored-child': 'Say: value',
    'duplicate-role': 'Say: evaluate', 'optional-condition': 'If: branch',
    'duplicate-kind': 'Number: value',
}
bootstrap = args.build.resolve() / 'lib/rxfnsl/tools/bootstrap'
for name, text, expected in negatives:
    spec = output / (name + '.rxl')
    spec.write_text(text)
    anchor = anchors[name]
    if name == 'string-default':
        offset = text.index('.string default 0') + len('.string default ')
    else:
        offset = text.rindex(anchor)
    location = f"line {text[:offset].count(chr(10))+1}, byte {len(text[:offset].encode())}:"
    messages = []
    for stage in range(3):
        text = run(name + '-L' + str(stage), [binary / 'rxtvm', bootstrap / f'L{stage}/levell-linked.rxbin', '-a', spec, '--rxas'], 1)
        assert expected in text and location in text, (name, stage, location, text)
        assert '.globals=' not in text and 'options levelb' not in text, (name, text)
        messages.append(text)
    assert len(set(messages)) == 1, (name, messages)

# Misspelled generated APIs are ordinary cREXX compile errors.
imports = output / 'll1-rxas-opt/imports'
for label, text, required in [
    ('misspelled-getter', 'answer = view.getmesage()', 'getmesage'),
    ('wrong-setter-type', 'answer = group.setpriority(node,view)', '#TYPE_MISMATCH'),
]:
    source = output / (label + '.crexx')
    source.write_text('options levelb\nimport flowdemo\nimport llast\nprobe: procedure = .int\n'
        '  arg group = .flowdemo..notes, view = .flowdemo..notes_entry, node = .llast..node\n'
        '  ' + text + '\n  return 0\n')
    data = run(label, [binary / 'rxc', '-x', '--no-exe-import', '-i', imports,
                      '-o', output / label, source], 2)
    assert required in data, data

inputs = [root / 'lib/rxfnsl/rexx/llannotation.crexx', root / 'lib/rxfnsl/rexx/llflow.crexx',
          root / 'lib/rxfnsl/rexx/llast.crexx', root / 'lib/rxfnsl/rexx/llfront.crexx',
          root / 'lib/rxfnsl/tools/levell.crexx', root / 'lib/rxfnsl/grammar/levell.levell',
          root / 'lib/rxfnsl/bootstrap/seed-reader.inc', consumer,
          root / 'lib/rxfnsl/examples/flow/rules.rxl', Path(__file__).resolve()]
(output / 'receipt.json').write_text(json.dumps(dict(commands=records, inputs={
    str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest() for path in inputs}), indent=2) + '\n')
print(f'PASS: {len(records)} commands; 32 backend/format/mode/VM runs; {len(negatives)*3} located grammar refusals')
