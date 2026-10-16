#!/usr/bin/env python3
"""Explicit structured-flow surface checks; CTests run prebuilt runtime controls.

Usage: check-structured-flow.py BUILD OUTPUT. Keeps commands and exact inputs.
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
parser.add_argument('--malformed-only', action='store_true', help='Run only the small syntax-refusal fixtures')
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
    (output / (key + '.log')).write_bytes(data)
    records.append(dict(key=key, command=command, status=result.returncode,
                        sha256=hashlib.sha256(data).hexdigest()))
    assert result.returncode == status, (key, result.returncode, data[-4000:])
    assert b'Warning in ' not in data, (key, data[-4000:])
    return (result.stdout if status == 0 else data).decode()

spec = root / 'lib/rxfnsl/examples/flow/loops.rxl'
base = spec.read_text()
consumer = root / 'lib/rxfnsl/tests_functional/ts_llstructuredflow.crexx'
bootstrap = args.build.resolve() / 'lib/rxfnsl/tools/bootstrap'
if not args.malformed_only:
    # Source generation complements the permanent packed-provider CTests.
    for backend in ['ll1', 'lr1', 'packrat', 'earley']:
        work = output / backend
        imports = work / 'imports'
        imports.mkdir(parents=True, exist_ok=True)
        for module in ['library', 'rxfnsl']:
            shutil.copyfile(binary / (module + '.rxbin'), imports / (module + '.rxbin'))
        grammar = work / 'rules.rxl'
        text = base.replace('backend ll1', 'backend ' + backend)
        if backend == 'packrat':
            text = text.replace('semantics contextfree', 'semantics ordered')
        grammar.write_text(text)
        source = work / 'flowloops.crexx'
        source.write_text(run(backend + '-generate', [binary / 'levell', grammar]))
        for mode, flags in [('opt', []), ('noopt', ['-n'])]:
            key = backend + '-' + mode
            run(key + '-provider-compile', [binary / 'rxc', '-x', '--no-exe-import', *flags,
                '-i', imports, '-o', work / 'flowloops', source])
            run(key + '-provider-assemble', [binary / 'rxas', *flags, work / 'flowloops'])
            shutil.copyfile(work / 'flowloops.rxbin', imports / 'flowloops.rxbin')
            run(key + '-compile', [binary / 'rxc', '-x', '--no-exe-import', *flags,
                '-i', imports, '-o', work / 'consumer', consumer])
            run(key + '-assemble', [binary / 'rxas', *flags, work / 'consumer'])
            run(key + '-link', [binary / 'rxlink', '-s', '-o', work / 'linked', work / 'consumer',
                work / 'flowloops', binary / 'rxfnsl', binary / 'library'])
            for vm in ['rxtvm', 'rxbvm']:
                assert run(key + '-' + vm, [binary / vm, work / 'linked.rxbin']) == 'failures=0\n'

    # A short-circuit expression can itself be the analysis root. Joining its
    # outcomes is the builder's final allocation boundary, covered here explicitly.
    work = output / 'expression-root'
    work.mkdir(exist_ok=True)
    grammar = work / 'rules.rxl'
    grammar.write_text(base.replace('start program', 'start expression'))
    (work / 'flowloops.rxas').write_text(run('root-generate', [binary / 'levell', grammar, '--rxas']))
    run('root-provider', [binary / 'rxas', work / 'flowloops'])
    probe = work / 'probe.crexx'
    probe.write_text("""options levelb
    import flowloops
    import lllanguage
    import llast
    import llflow
    import rxfnsb
    language = flowloops..language()
    request = .lllanguage..request(language,'both(flag,set(x,1))' as .binary)
    if request.parse() <> 0 then exit 1
    tree = request.ast(); graph = flowloops..execution(reference tree)
    result = graph.definiteassignment('flag')
    if result.error() <> '' | result.count() <> 0 then exit 2
    if result.before(tree.root()).reachable() <> 1 then exit 3
    required = graph.entries()
    do limit = 0 to required - 1
      small = flowloops..execution(reference tree,limit)
      if small.start() <> 0 | small.points() <> 0 | pos('flow.entries',small.error()) = 0 then exit 4
    end
    exact = flowloops..execution(reference tree,required)
    if exact.error() <> '' | exact.entries() <> required then exit 5
    say 'root=ok'
    """)
    imports = work / 'imports'
    imports.mkdir(exist_ok=True)
    for module, directory in [('flowloops', work), ('rxfnsl', binary), ('library', binary)]:
        shutil.copyfile(directory / (module + '.rxbin'), imports / (module + '.rxbin'))
    run('root-compile', [binary / 'rxc', '-x', '--no-exe-import', '-i', imports, '-o', work / 'probe', probe])
    run('root-assemble', [binary / 'rxas', work / 'probe'])
    run('root-link', [binary / 'rxlink', '-s', '-o', work / 'linked', work / 'probe', work / 'flowloops', binary / 'rxfnsl', binary / 'library'])
    for vm in ['rxtvm', 'rxbvm']:
        assert run('root-' + vm, [binary / vm, work / 'linked.rxbin']) == 'root=ok\n'

    for route in ['source', 'rxas']:
        options = ['--rxas'] if route == 'rxas' else []
        stages = [run(f'valid-{route}-L{stage}', [binary / 'rxtvm', bootstrap / f'L{stage}/levell-linked.rxbin', '-a', spec, *options]) for stage in range(3)]
        assert stages[0] == stages[1] == stages[2], route

    # These are valid syntax with invalid AST field contracts, so all three readers
    # must produce exactly the same located semantic diagnostic.
    negatives = [
        ('while-name', 'While: loop while condition do body', 'While: loop while test do body', 'While has no field test'),
        ('until-name', 'Until: loop do body until condition', 'Until: loop do body until test', 'Until has no field test'),
        ('body-name', 'Forever: loop do body', 'Forever: loop do items', 'Forever has no field items'),
        ('duplicate-role', 'While: loop while condition do body', 'While: loop while condition do condition', 'occurs twice in its flow mapping'),
        ('return-text', 'Set: assign name value', 'Set: return name', 'Set.name must be a child field'),
        ('return-list', 'Program: sequence items', 'Program: return items', 'Program.items must contain one optional or required child'),
        ('loop-list', 'Program: sequence items', 'Program: loop do items', 'Program.items must contain exactly one child'),
        ('optional-loop', 'Return: return value', 'Return: loop do value', 'Return.value must contain exactly one child'),
        ('optional-operand', 'Return: return value', 'Return: shortcircuit value and value', 'Return.value must contain exactly one child'),
        ('ignored-child', 'Return: return value', 'Return: return', 'Return.value has no execution role'),
        ('and-name', 'Both: shortcircuit left and right', 'Both: shortcircuit left and other', 'Both has no field other'),
        ('or-text', 'Set: assign name value', 'Set: shortcircuit name or value', 'Set.name must be a child field'),
        ('break-child', 'Return: return value', 'Return: break', 'Return.value has no execution role'),
    ]
    for name, original, replacement, required in negatives:
        text = base.replace(original, replacement)
        grammar = output / (name + '.rxl')
        grammar.write_text(text)
        offset = text.index(replacement)
        location = f"line {text[:offset].count(chr(10))+1}, byte {len(text[:offset].encode())}:"
        results = []
        for stage in range(3):
            message = run(f'{name}-L{stage}', [binary / 'rxtvm', bootstrap / f'L{stage}/levell-linked.rxbin', '-a', grammar, '--rxas'], 1)
            assert required in message and location in message, (name, stage, location, message)
            assert '.globals=' not in message
            results.append(message)
        assert len(set(results)) == 1, (name, results)

# Misplaced/unknown grammar words must fail without generating a partial module.
for name, original, replacement in [
    ('while-missing-do', 'loop while condition do body', 'loop while condition body'),
    ('until-missing-test', 'loop do body until condition', 'loop do body until'),
    ('unknown-shortcircuit', 'shortcircuit left and right', 'shortcircuit left xor right'),
    ('break-qualifier', 'Break: break', 'Break: break body'),
]:
    grammar = output / (name + '.rxl')
    mapping = replacement.split(': ', 1)[-1]
    grammar.write_text('namespace invalidflow\nitem: token\n'
        'parse: parser start root backend ll1\nroot: rule = item then tree Root()\n'
        'execution: flow\n  Root: ' + mapping + '\n')
    for stage in range(3):
        message = run(f'{name}-L{stage}', [binary / 'rxtvm', bootstrap / f'L{stage}/levell-linked.rxbin', '-a', grammar, '--rxas'], 1)
        assert 'line ' in message and 'line 0' not in message and '.globals=' not in message, (name, stage, message)
        if name == 'until-missing-test':
            end = len(grammar.read_bytes())
            assert 'line 7' in message and f'byte {end}' in message, (name, stage, message)
        if name == 'unknown-shortcircuit' and stage == 0:
            assert 'expected AND or OR between shortcircuit fields' in message, message

inputs = [spec, consumer, Path(__file__).resolve(), root / 'lib/rxfnsl/grammar/levell.levell',
          root / 'lib/rxfnsl/rexx/llfront.crexx', root / 'lib/rxfnsl/rexx/llflow.crexx']
(output / ('malformed-receipt.json' if args.malformed_only else 'receipt.json')).write_text(json.dumps(dict(commands=records, inputs={
    str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest() for path in inputs}), indent=2) + '\n')
print(f'PASS: {len(records)} commands; ' + ('small malformed-grammar refusals at L0/L1/L2' if args.malformed_only else 'source providers, both modes/VMs, stage parity and located refusals'))
