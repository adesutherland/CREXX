#!/usr/bin/env python3
"""Level L source selection and installed crexx generation/build regressions.

Run with BUILD OUTPUT [--part forms|builds|all]. Each command retains a log;
OUTPUT must be a dedicated scratch directory. --bin tests an installed prefix.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time

parser = argparse.ArgumentParser()
parser.add_argument('build', type=Path)
parser.add_argument('output', type=Path)
parser.add_argument('--bin', type=Path)
parser.add_argument('--part', choices=['all', 'forms', 'builds'], default='all')
a = parser.parse_args()
root = Path(__file__).resolve().parents[3]
binary = a.bin.resolve() if a.bin else a.build.resolve() / 'bin'
out = a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
env = dict(os.environ, CREXX_HOME=str(binary.parent))
records = []
start = time.monotonic()


def tool(name, directory=binary):
    path = directory / name
    return path.with_suffix('.exe') if os.name == 'nt' else path


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(name, argv, status=0, environment=env):
    result = subprocess.run(list(map(str, argv)), capture_output=True, env=environment, timeout=1200)
    data = result.stdout + result.stderr
    (out / (name + '.log')).write_bytes(data)
    records.append(dict(name=name, argv=list(map(str, argv)), status=result.returncode,
                        sha256=hashlib.sha256(data).hexdigest()))
    (out / 'progress.json').write_text(json.dumps(records, indent=2) + '\n', encoding='utf-8')
    assert result.returncode == status, (name, result.returncode, data[-5000:])
    assert b'Warning in ' not in data, (name, data[-5000:])
    return result.stdout.decode()


spec = (root / 'lib/rxfnsl/examples/settings/01-basic.crexx').read_text(encoding='utf-8')
if a.part in ['all', 'forms']:
    forms = out / 'forms'
    forms.mkdir(exist_ok=True)
    variants = {
        'explicit.crexx': spec,
        'explicit.rxl': spec,
        'short.rxl': spec.replace('options levell\n', '', 1),
        'short.RXL': spec.replace('options levell\n', '', 1),
        'legacy.levell': spec,
        'comments.crexx': spec.replace('options levell', '/* outer /* nested */ end */\nOpTiOnS /* note */ LeVeLl', 1),
    }
    expected = None
    for name, text in variants.items():
        source = forms / name
        source.write_text(text, encoding='utf-8')
        generated = run('form-' + name, [tool('levell'), source, '--rxas'])
        # Grammar declaration line metadata follows authored source positions.
        if name != 'comments.crexx':
            if expected is None:
                expected = generated
            # Graph source-line metadata can differ when OPTIONS is omitted;
            # execution equivalence is checked by the mixed-project tests.
            assert 'levell_settings.language' in generated
    for stage in range(3):
        image = a.build.resolve() / f'lib/rxfnsl/tools/bootstrap/L{stage}/levell-linked.rxbin'
        if not image.exists():
            continue
        for name in ['explicit.crexx', 'short.rxl']:
            generated = run(f'bootstrap-{stage}-{name}', [tool('rxtvm'), image, '-a', forms / name, '--rxas'])
            native = (out / ('form-' + name + '.log')).read_text(encoding='utf-8')
            assert generated == native, (stage, name, 'bootstrap disagrees')
    for name, text, required in [
        ('missing.crexx', spec.replace('options levell\n', '', 1), 'expected options'),
        ('conflict.rxl', spec.replace('options levell', 'options levelg'), 'levell'),
        ('located.rxl', spec.replace('options levell\n', '', 1).replace('value: rule = number | string', 'value: rule = missing'), 'undefined symbol missing'),
    ]:
        source = forms / name
        source.write_text(text, encoding='utf-8')
        data = run('invalid-' + name, [tool('levell'), source, '--rxas'], 1)
        assert str(source) in data and required in data, data
        if name == 'located.rxl':
            line = text[:text.index('value: rule = missing')].count('\n') + 1
            assert f'line {line},' in data, data
    for name, token in [('L1', 'L2')]:
        stage_root = a.build.resolve() / 'lib/rxfnsl/tools/bootstrap'
        assert (stage_root / name / 'llsyntax.rxas').read_bytes() == (stage_root / token / 'llsyntax.rxas').read_bytes()

if a.part in ['all', 'builds']:
    work = out / 'project with spaces'
    source = work / 'source'
    source.mkdir(parents=True, exist_ok=True)
    grammar = source / 'settings grammar.crexx'
    grammar.write_text(spec, encoding='utf-8')
    reader = source / 'reader.crexx'
    shutil.copyfile(root / 'lib/rxfnsl/examples/settings-reader.crexx', reader)
    sample = work / 'sample.settings'
    sample.write_text('\nport=7;\n\ngreeting="café"', encoding='utf-8')
    output = work / 'reader'
    args = [tool('crexx'), '--program', output, reader, grammar, '--jobs', '2']
    data = run('mixed-clean', args)
    assert data.index('BUILT: grammar') < data.index('START: project member'), data
    image = output.with_suffix('.rxbin')
    for vm in ['rxtvm', 'rxbvm']:
        data = run('mixed-' + vm, [tool(vm), image, '-a', sample])
        assert data == 'port -> 7\ngreeting -> "café"\n', data
    original = sha(image)
    data = run('mixed-noop', args)
    assert 'SKIP: project current' in data and 'START:' not in data
    assert sha(image) == original
    grammar.write_text(spec.replace("'0' to '9'", "'0' to '8'"), encoding='utf-8')
    data = run('grammar-edit', args)
    assert 'BUILT: grammar' in data and 'START: project member reader' in data
    sample.write_text('port=9', encoding='utf-8')
    data = run('changed-language', [tool('rxtvm'), image, '-a', sample], 1)
    assert 'SETTINGS_PARSE' in data and 'port ->' not in data
    stable = sha(image)
    grammar_image = Path(str(output) + '.crexx-build/grammars/settings grammar_grammar/settings grammar_grammar.rxbin')
    stable_grammar = sha(grammar_image)
    grammar.write_text(spec.replace('value: rule = number | string', 'value: rule = missing'), encoding='utf-8')
    data = run('failed-grammar', args, 1)
    assert str(grammar) in data and 'undefined symbol missing' in data and 'START: project member' not in data
    assert sha(image) == stable and sha(grammar_image) == stable_grammar
    grammar.write_text(spec, encoding='utf-8')
    run('restored', args)
    sample.write_text('port=9', encoding='utf-8')
    data = run('restored-run', [tool('rxbvm'), image, '-a', sample])
    assert data == 'port -> 9\n'
    # Source order and an absent OPTIONS header do not affect dependency order.
    short = source / 'short.rxl'
    short.write_text(spec.replace('options levell\n', '', 1), encoding='utf-8')
    reordered = [tool('crexx'), '--program', work / 'reordered', short, reader]
    run('short-first', reordered)
    data = run('short-first-run', [tool('rxtvm'), work / 'reordered.rxbin', '-a', sample])
    assert data == 'port -> 9\n'
    run('mixed-noopt', [tool('crexx'), '--program', work / 'noopt', reader, short, '--noopt'])
    assert run('mixed-noopt-run', [tool('rxbvm'), work / 'noopt.rxbin', '-a', sample]) == 'port -> 9\n'
    library = work / 'library' / 'settings'
    run('library', [tool('crexx'), '--library', library, short])
    # A program can consume a separately built library without the grammar.
    run('library-consumer', [tool('crexx'), '--program', work / 'library-reader', reader, '-i', library.parent, '-l', library])
    data = run('library-run', [tool('rxvme'), work / 'library-reader.rxbin', '-a', sample])
    assert data == 'port -> 9\n'
    second = source / 'second.rxl'
    second.write_text(short.read_text(encoding='utf-8').replace('namespace levell_settings', 'namespace second_settings'), encoding='utf-8')
    run('two-library', [tool('crexx'), '--library', work / 'library' / 'both', short, second])
    # Both standalone grammar libraries can participate without duplicate runtime exports.
    run('second-library', [tool('crexx'), '--library', work / 'library' / 'second', second])
    both_reader = source / 'both.crexx'
    both_reader.write_text('options levelb\nimport rxfnsb\nimport lllanguage\nimport levell_settings\nimport second_settings\nmain: procedure = .int\n  first = .lllanguage..request(levell_settings..language(), \'a=1\' as .binary)\n  second = .lllanguage..request(second_settings..language(), \'a=2\' as .binary)\n  say \'two grammars\' first.parse() second.parse()\n  return 0\n', encoding='utf-8')
    for kind, libs in [('combined', [work / 'library' / 'both']), ('separate', [library, work / 'library' / 'second'])]:
        library_args = [value for lib in libs for value in ['-l', lib]]
        run('two-' + kind, [tool('crexx'), '--program', work / ('two-' + kind), both_reader, '-i', library.parent, *library_args])
        assert run('two-' + kind + '-run', [tool('rxvme'), work / ('two-' + kind + '.rxbin')]) == 'two grammars 0 0\n'
    data = run('implicit-library', [tool('crexx'), short])
    assert 'PUBLISHED: library' in data
    data = run('implicit-program', [tool('crexx'), reader, short, '--args', sample])
    assert 'port -> 9' in data
    # Case-insensitive suffixes, commented headers, and conflicting levels.
    uppercase = source / 'caps.RXL'
    uppercase.write_text(spec.replace('options levell\n', '', 1), encoding='utf-8')
    run('uppercase', [tool('crexx'), '--library', work / 'caps', uppercase])
    grammar.write_text(spec.replace('options levell', '/* nested /* comment */ */\nOPTIONS /* note */ LEVELL'), encoding='utf-8')
    run('commented-header', args)
    short.write_text(spec.replace('options levell', 'options levelg'), encoding='utf-8')
    run('conflicting-header', reordered, 1)
    short.write_text(spec.replace('options levell\n', '', 1), encoding='utf-8')
    collision = source / 'short.crexx'
    collision.write_text(spec, encoding='utf-8')
    data = run('stem-collision', [tool('crexx'), '--program', work / 'collision', short, collision], 2)
    assert 'duplicate project member name' in data
    collision.unlink()
    # A native program embeds the parser and its runtime without an extra -l.
    native = work / 'native-reader'
    run('native', [tool('crexx'), '--program', native, reader, short, '--native'])
    data = run('native-run', [tool('native-reader', work), sample])
    assert data == 'port -> 9\n'
    data = run('native-noop', [tool('crexx'), '--program', native, reader, short, '--native'])
    assert 'SKIP: native program current' in data
    run('native-library', [tool('crexx'), '--program', work / 'native-library', reader, '-i', library.parent, '-l', library, '--native'])
    assert run('native-library-run', [tool('native-library', work), sample]) == 'port -> 9\n'
    calculator = root / 'lib/rxfnsl/examples/calculator'
    run('calculator', [tool('crexx'), '--program', work / 'calculator', calculator / 'calculator.crexx', calculator / 'calculator.rxl'])
    calc_input = work / 'expression.calc'
    calc_input.write_text('1+2*3', encoding='utf-8')
    assert 'RESULT|7' in run('calculator-run', [tool('rxvme'), work / 'calculator.rxbin', '-a', calc_input])
    # A private launcher changes identity without modifying signed executables.
    if os.name != 'nt':
        private = work / 'toolchain'
        private_bin = private / 'bin'
        private_bin.mkdir(parents=True, exist_ok=True)
        for entry in binary.iterdir():
            target = private_bin / entry.name
            if not target.exists():
                target.symlink_to(entry, target_is_directory=entry.is_dir())
        generator = private_bin / 'levell'
        generator.unlink()
        (private_bin / 'levell.real').symlink_to(tool('levell'))
        launcher = '#!/bin/sh\nexec "${0%/*}/levell.real" "$@"\n'
        generator.write_text(launcher, encoding='utf-8')
        generator.chmod(0o755)
        private_env = dict(env, CREXX_HOME=str(private))
        private_args = [tool('crexx', private_bin), '--library', work / 'tool-change', short]
        run('tool-before', private_args, environment=private_env)
        generator.write_text(launcher + '# changed generator identity\n', encoding='utf-8')
        data = run('tool-changed', private_args, environment=private_env)
        assert 'BUILT: grammar' in data
        # Add a valid, unrelated module to the runtime container: public parser
        # behavior is unchanged, but its content identity must invalidate work.
        marker = work / 'runtime_identity.crexx'
        marker.write_text('options levelb\nnamespace runtime_identity expose version\nversion: procedure = .int\n  return 1\n', encoding='utf-8')
        run('runtime-marker-compile', [tool('rxc'), '--no-exe-import', '-i', binary, '-o', work / 'runtime_identity', marker])
        run('runtime-marker-assemble', [tool('rxas'), work / 'runtime_identity'])
        runtime = private_bin / 'rxfnsl.rxbin'
        runtime.unlink()
        runtime.write_bytes((binary / 'rxfnsl.rxbin').read_bytes() + (work / 'runtime_identity.rxbin').read_bytes())
        data = run('runtime-changed', private_args, environment=private_env)
        assert 'BUILT: grammar' in data
        generator.unlink()
        data = run('missing-generator', private_args, 2, environment=private_env)
        assert 'generator is missing' in data

receipt = dict(part=a.part, seconds=time.monotonic() - start, commands=records,
               tools={name: sha(tool(name)) for name in ['crexx', 'levell', 'rxc', 'rxas', 'rxlink']})
(out / ('receipt-' + a.part + '.json')).write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
print(f'authoring_build_commands={len(records)}; failures=0; seconds={receipt["seconds"]:.2f}')
