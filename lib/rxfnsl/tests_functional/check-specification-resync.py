#!/usr/bin/env python3
"""Declaration candidates after quote/comment/group damage; failed builds stay failed."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('build', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[3]
build = a.build.resolve()
binary = build / 'bin'
out = a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
records = []


def run(name, command, status=0):
    command = list(map(str, command))
    r = subprocess.run(command, capture_output=True, timeout=1200)
    data = r.stdout + r.stderr
    (out / (name + '.log')).write_bytes(data)
    records.append(dict(name=name, argv=command, status=r.returncode,
                        sha256=hashlib.sha256(data).hexdigest()))
    assert r.returncode == status and b'Warning in ' not in data, (name, r.returncode, data[-2000:])
    return r.stdout.decode()


prefix = 'options levell\nnamespace broken\nitem: token = 1\nparse: parser start last.good\n'
suffix = 'second: rule = )\nlast.good: rule = item\n'
cases = {
    'group': prefix + 'first: rule = (item\n' + suffix,
    'lexical': prefix + 'first: rule = $ item\n' + suffix,
    'quote': prefix + "first: diagnostic = 'unclosed\n" + suffix,
    'comment': prefix + 'first: rule = item /* unclosed\n' + suffix,
    'quoted-header': prefix + "first: diagnostic = 'fake: rule = )'\n" + suffix,
    'commented-header': prefix + 'first: rule = (item /* fake: rule = ) */\n' + suffix,
}
count = 0
for stage in [1, 2]:
    work = out / ('L' + str(stage))
    work.mkdir(exist_ok=True)
    directory = build / f'lib/rxfnsl/tools/bootstrap/L{stage}'
    run(f'L{stage}-compile', [binary / 'rxc', '-x', '--no-exe-import', '-i',
        f'{directory / "driver-imports"};{binary}', '-o', work / 'probe',
        root / 'lib/rxfnsl/tests_functional/specification-probe.crexx'])
    run(f'L{stage}-assemble', [binary / 'rxas', work / 'probe'])
    run(f'L{stage}-link', [binary / 'rxlink', '-s', '-o', work / 'linked', work / 'probe.rxbin',
        directory / 'llfront.rxbin', directory / 'llsyntax.rxbin',
        binary / 'rxfnsl-base.rxbin', binary / 'library.rxbin'])
    for name, source in cases.items():
        path = work / (name + '.levell')
        path.write_text(source)
        for vm in ['rxtvm', 'rxbvm']:
            data = run(f'L{stage}-{vm}-{name}', [binary / vm, work / 'linked.rxbin', '-a', path], 3)
            assert 'CANDIDATE|last.good\n' in data, (stage, name, data)
            assert 'CANDIDATE|second\n' not in data and 'CANDIDATE|fake\n' not in data
            if name not in ['quoted-header']:
                assert 'CANDIDATE|first\n' not in data
            assert 'LL_SYNTAX at line' in data
            if name in ['lexical', 'quote', 'comment']:
                assert 'LL_LEXICAL at line' in data
            count += 1
        if name == 'group':
            data = run(f'L{stage}-bounded', [binary / 'rxtvm', work / 'linked.rxbin', '-a', path, '1'], 4)
            assert 'LL_RESOURCE_LIMIT: recovery.errors budget 1, required at least 2' in data

        preserved = work / 'preserved.rxas'
        preserved.write_bytes(b'last good output\n')
        run(f'L{stage}-preserve-{name}', ['cmake', '-DLEVEL_L_VM=' + str(binary / 'rxtvm'),
            '-DLEVEL_L_BYTECODE=' + str(directory / 'levell-linked.rxbin'), '-DSPEC=' + str(path),
            '-DOUTPUT=' + str(preserved), '-DFORMAT=rxas', '-P',
            root / 'lib/rxfnsl/tools/generate.cmake'], 1)
        assert preserved.read_bytes() == b'last good output\n'
        assert not Path(str(preserved) + '.tmp').exists()

(out / 'receipt.json').write_text(json.dumps(dict(records=records, candidate_cases=count,
    scope='L1/L2, both VMs; isolated candidates after lexical damage remain failed and never become generated code.'), indent=2) + '\n')
print(f'specification_candidate_cases={count}; preservation_cases=12; failures=0')
