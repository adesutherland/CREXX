#!/usr/bin/env python3
"""Check prepared SDK/compiler package boundaries without a nested build.

Real link controls, exact staged imports and owning source imports establish
selection/freshness boundaries. Independent executable fixtures establish the
packages' semantics; this check does not claim installed or platform delivery.
"""
import argparse
import hashlib
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]
OPTIONAL = {'llirgraph', 'llireffects', 'llrxascatalog', 'llrxas', 'llirexecute'}


def digest(path):
    result = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            result.update(block)
    return result.hexdigest()


def control(path):
    rows = path.read_text().splitlines()
    inputs = [Path(row[6:]) for row in rows if row.startswith('INPUT ')]
    selectors = [row[8:] for row in rows if row.startswith('INCLUDE ')]
    outputs = [Path(row[7:]) for row in rows if row.startswith('OUTPUT ')]
    assert inputs and selectors and len(outputs) == 1, (path, 'incomplete control')
    assert len(inputs) == len(set(inputs)), (path, 'duplicate input producer')
    assert len(selectors) == len(set(selectors)), (path, 'duplicate selector')
    assert all(item.is_file() for item in inputs), (path, 'unprepared input')
    assert outputs[0].with_suffix('.rxbin').is_file(), (path, 'unprepared image')
    assert 'PRESERVE INLINE' in rows, (path, 'lost SDK metadata')
    return inputs, set(selectors), outputs[0]


def imports(directory, originals):
    expected = {path.name: path for path in originals}
    actual = {path.name: path for path in directory.iterdir()}
    assert actual.keys() == expected.keys(), (directory, sorted(actual), sorted(expected))
    for name, original in expected.items():
        assert digest(actual[name]) == digest(original), (directory, name, 'stale staged import')


def source_imports(paths, allowed):
    for path in paths:
        names = set(re.findall(r'^\s*import\s+([A-Za-z0-9_]+)', path.read_text(), re.M | re.I))
        rejected = {name for name in names if name.lower() not in allowed}
        assert not rejected, (path, 'foreign source dependency', sorted(rejected))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--config', required=True)
    parser.add_argument('--language', choices=('c', 'g'), action='append', default=[])
    args = parser.parse_args()
    build = args.build.resolve()
    sdk = build / 'lib/rxfnsl'
    base = build / 'bin/library.rxbin'
    core_image = build / 'bin/rxfnsl.rxbin'
    analysis_image = build / 'bin/rxfnsl-analysis.rxbin'

    core_inputs, core, output = control(sdk / 'rexx' / f'rxfnsl_link_{args.config}.ctl')
    assert output == core_image.with_suffix('')
    assert core == {path.stem for path in core_inputs} and not core & OPTIONAL
    assert all(path.parent == sdk / 'rexx' for path in core_inputs)
    core_sources = list((ROOT / 'lib/rxfnsl/rexx').glob('*.crexx'))
    source_imports(core_sources, core | {'rxfnsb'})
    imports(sdk / 'rexx/imports', [base])

    packages = (
        ('analysis', 'analysis.ctl', 'rxfnsl-analysis', {'llirgraph', 'llireffects'},
         core | {'llirgraph', 'llireffects', 'rxfnsb'}, [base, core_image]),
        ('rxas', 'rxas.ctl', 'rxfnsl-rxas', {'llrxascatalog', 'llrxas'},
         {'llrxascatalog', 'llrxas', 'rxfnsb'}, [base]),
        ('execute', 'execute.ctl', 'rxfnsl-execute', {'llirexecute'},
         core | {'llirgraph', 'llireffects', 'llirexecute', 'rxfnsb'},
         [base, core_image, analysis_image]),
    )
    for directory, filename, image, members, allowed, dependencies in packages:
        inputs, selectors, output = control(sdk / directory / filename)
        assert selectors == members == {path.stem for path in inputs}, (image, selectors)
        assert all(path.parent == sdk / directory for path in inputs), image
        assert output == build / 'bin' / image
        sources = list((ROOT / 'lib/rxfnsl' / directory).glob('*.crexx'))
        if directory == 'rxas':
            sources.append(sdk / 'rxas/llrxascatalog.crexx')
        source_imports(sources, allowed)
        imports(sdk / directory / 'imports', dependencies)

    # The prepared view must name the current canonical native owner inputs.
    catalog_inputs = ('binutils/include/rxops.h', 'binutils/include/rxopeffects.h',
                      'binutils/include/rxopsignals.h', 'binutils/include/rxdefs.h',
                      'binutils/rxopmeta.c', 'binutils/include/opdata.c')
    fingerprint = hashlib.sha256(''.join(digest(ROOT / name) for name in catalog_inputs).encode()).hexdigest()
    assert fingerprint in (sdk / 'rxas/llrxascatalog.crexx').read_text(), 'stale canonical RXAS view'

    imports(build / 'langtools/bootstrap/imports', [base, core_image])
    tools = list((ROOT / 'langtools/rexx').glob('*.crexx'))
    tools += list((ROOT / 'langtools/common').glob('*.crexx')) + [ROOT / 'langtools/rxlang.crexx']
    source_imports(tools, core | {'rxfnsb', 'rxfs', 'llfront', 'llsyntax', 'llgenerate'})
    for helper in ('llrxastext', 'llbinmeta', 'llfileoutput'):
        dependencies = [base, core_image]
        if helper == 'llfileoutput':
            dependencies.append(build / 'bin/rxfs.rxplugin')
        imports(build / 'langtools/common' / (helper + '-imports'), dependencies)

    compilers = build / 'research/level-l/compilers'
    for language in args.language:
        owner = 'levelc' if language == 'c' else 'typed'
        inputs, selectors, output = control(compilers / owner / 'opt/service.ctl')
        assert output == compilers / 'products' / f'll{language}compiler'
        required = {'llirgraph', 'llireffects', 'llrxascatalog', 'llrxas'}
        assert required <= selectors and 'llirexecute' not in selectors
        assert {analysis_image, build / 'bin/rxfnsl-rxas.rxbin'} <= set(inputs)
        assert build / 'bin/rxfnsl-execute.rxbin' not in inputs
        assert not selectors & {'rxfnsl-analysis', 'rxfnsl-rxas', 'rxfnsl-execute'}
        foreign = {'typed', 'levelg'} if language == 'c' else {'levelc'}
        assert all(not foreign.intersection(path.relative_to(compilers).parts)
                   for path in inputs if path.is_relative_to(compilers)), (language, 'foreign frontend')
        assert f'll{"g" if language == "c" else "c"}grammar' not in selectors
    if 'c' in args.language:
        imports(compilers / 'backend/opt/imports-llctarget', [base, build / 'bin/rxfnsc.rxbin'])
        runtime_sources = list((ROOT / 'lib/rxfnsc').rglob('*.crexx'))
        runtime_sources += list((ROOT / 'research/level-l/compilers/levelc/runtime').glob('*.crexx'))
        for path in runtime_sources:
            names = re.findall(r'^\s*import\s+(\w+)', path.read_text(), re.M | re.I)
            assert not any(name.lower().startswith('ll') for name in names), (path, 'compiler/SDK runtime dependency')
    print('LEVEL_L_PACKAGE_CLOSURES|PASS')


if __name__ == '__main__':
    main()
