"""Standalone Classic compiler/import/assembly/link/runtime preservation."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import time


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--bindir', type=Path, required=True)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    args = parser.parse_args()
    bindir, source, work = args.bindir.resolve(), args.source.resolve(), args.work.resolve()
    work.mkdir(parents=True, exist_ok=True)
    providers = work / 'providers É'
    providers.mkdir(exist_ok=True)
    fixtures = work / 'sources É'
    fixtures.mkdir(exist_ok=True)
    extension = '.exe' if os.name == 'nt' else ''
    records = []

    def run(label, argv, cwd, expected=0):
        started = time.monotonic()
        result = subprocess.run(list(map(str, argv)), cwd=cwd, stdin=subprocess.DEVNULL,
                                capture_output=True, timeout=600)
        log = cwd / (label + '.log')
        log.write_bytes(result.stdout + result.stderr)
        records.append(dict(label=label, argv=list(map(str, argv)), rc=result.returncode,
                            elapsed=time.monotonic()-started, log=str(log)))
        if result.returncode != expected:
            raise RuntimeError(f'{label}: expected {expected}, got {result.returncode}; {log}\n'
                               + (result.stdout+result.stderr)[-1800:].decode(errors='replace'))
        return result.stdout.replace(b'\r\n', b'\n')

    def tool(name):
        return bindir / (name + extension)

    provider = providers / 'alias.crexx'
    shutil.copyfile(source / 'compiler/tests/rexx_src/levelc_call_alias_provider.crexx', provider)
    run('provider-compile', [tool('rxc'), '-i', bindir, '-o', providers/'alias', provider], providers)
    run('provider-assemble', [tool('rxas'), providers/'alias.rxas'], providers)
    g_providers = work / 'G providers É'
    g_providers.mkdir(exist_ok=True)
    g_provider = g_providers / 'alias.crexx'
    g_provider.write_text(provider.read_text(encoding='utf-8').replace('options levelb', 'options levelg'), encoding='utf-8')
    run('g-provider-compile', [tool('rxc'), '-i', bindir, '-o', g_providers/'alias', g_provider], g_providers)
    run('g-provider-assemble', [tool('rxas'), g_providers/'alias.rxas'], g_providers)
    (fixtures/'exact.rexx').write_bytes(b"options levelc\nx=1\ns.='base'\ns.x='v'\nsay c2x('a\0b') || ':' || s.x\n")
    (fixtures/'typed.rexx').write_text("options levelc\ncall typedentry\nsay result\n", encoding='utf-8')
    shutil.copyfile(fixtures/'typed.rexx', fixtures/'typed_g.rexx')
    for name in ('levelc_procedure_unicode', 'levelc_expression_precedence'):
        shutil.copyfile(source/f'compiler/tests/rexx_src/{name}.rexx', fixtures/f'{name}.rexx')

    for noopt in (False, True):
        mode = 'noopt' if noopt else 'opt'
        flags = ['-n'] if noopt else []
        for name in ('exact', 'typed', 'typed_g', 'levelc_procedure_unicode', 'levelc_expression_precedence'):
            outputs = []
            images = []
            for compiler in ('rxc', 'rxc-c'):
                lane = work / mode / name / compiler
                lane.mkdir(parents=True, exist_ok=True)
                provider_root = g_providers if name == 'typed_g' else providers
                extra = ['--import', 'levelc_call_alias', '-s', provider_root] if name.startswith('typed') else []
                run('compile', [tool(compiler), *flags, '-i', bindir, *extra,
                                '-o', lane/'program', fixtures/f'{name}.rexx'], lane)
                run('assemble', [tool('rxas'), *flags, lane/'program.rxas'], lane)
                linked = lane/'linked.rxbin'
                dependencies = [provider_root/'alias.rxbin'] if name.startswith('typed') else []
                run('link', [tool('rxlink'), '-o', linked, lane/'program.rxbin', *dependencies,
                             bindir/'library.rxbin', bindir/'classlib.rxbin', bindir/'rxfnsc.rxbin'], lane)
                outputs.append(run('run', [tool('rxvm'), linked], lane))
                images.append(lane/'program.rxas')
                if compiler == 'rxc-c' and (tool('rxtvm')).exists():
                    threaded = run('threaded', [tool('rxtvm'), linked], lane)
                    if threaded != outputs[-1]:
                        raise RuntimeError(f'{mode}/{name}: portable/threaded output mismatch')
            if images[0].read_bytes() != images[1].read_bytes():
                raise RuntimeError(f'{mode}/{name}: assembly mismatch')
            if outputs[0] != outputs[1]:
                raise RuntimeError(f'{mode}/{name}: runtime output mismatch')
            expected = {'exact': b'610062:v\n', 'typed': b'ALIAS-RESULT\n', 'typed_g': b'ALIAS-RESULT\n',
                        'levelc_procedure_unicode': b'unicode=after\n'}.get(name)
            if expected is not None and outputs[1] != expected:
                raise RuntimeError(f'{mode}/{name}: incorrect output {outputs[1]!r}')

    # A source-only RXAS dependency must keep the parent's opt-in discovery
    # policy when rxc-c asks the owning G compiler to materialize contracts.
    rxas_providers = work/'RXAS providers É'
    rxas_providers.mkdir(exist_ok=True)
    helper = rxas_providers/'extrahelper.rxas'
    helper.write_text('.globals=0\nhelper() .locals=0 .expose=extrahelper.helper\n'
                      ' .meta "extrahelper.helper"="b" ".string" helper() ""\n'
                      ' ret "RXAS-PROVIDER"\n', encoding='utf-8')
    helper.with_suffix('.rxbin').unlink(missing_ok=True)
    extra_provider = rxas_providers/'alias.crexx'
    extra_provider.write_text(g_provider.read_text(encoding='utf-8').replace(
        'import rexxvalue', 'import rexxvalue\nimport extrahelper').replace(
        "'ALIAS-RESULT'", 'helper()'), encoding='utf-8')
    flags = ['--no-exe-import', '--import-rxas', '-x', '-i', bindir,
             '-i', rxas_providers, '-s', rxas_providers]
    run('rxas-provider-compile', [tool('rxc'), *flags, '-o', rxas_providers/'alias', extra_provider], rxas_providers)
    compiled = []
    for compiler in ('rxc', 'rxc-c'):
        lane = work/'rxas-policy'/compiler
        lane.mkdir(parents=True, exist_ok=True)
        run('compile', [tool(compiler), *flags, '--import', 'levelc_call_alias',
                        '-o', lane/'program', fixtures/'typed_g.rexx'], lane)
        compiled.append(lane)
    run('helper-assemble', [tool('rxas'), helper], rxas_providers)
    run('rxas-provider-assemble', [tool('rxas'), rxas_providers/'alias.rxas'], rxas_providers)
    for lane in compiled:
        run('assemble', [tool('rxas'), lane/'program.rxas'], lane)
        run('link', [tool('rxlink'), '-o', lane/'linked.rxbin', lane/'program.rxbin',
                     rxas_providers/'alias.rxbin', helper.with_suffix('.rxbin'),
                     bindir/'library.rxbin', bindir/'classlib.rxbin', bindir/'rxfnsc.rxbin'], lane)
        if run('run', [tool('rxvm'), lane/'linked.rxbin'], lane) != b'RXAS-PROVIDER\n':
            raise RuntimeError('Typed source broker lost parent RXAS import policy')

    bad = fixtures/'bad.rexx'
    for text in ("options levelc\nif 1 then\n", "options levelc\ninterpret 'say 1'\n"):
        bad.write_text(text, encoding='utf-8')
        diagnostics = []
        for compiler in ('rxc', 'rxc-c'):
            lane = work/compiler
            lane.mkdir(exist_ok=True)
            run('invalid', [tool(compiler), '--diagnostics', 'raw', '-i', bindir, bad], lane, 2)
            diagnostics.append((lane/'invalid.log').read_bytes())
        if diagnostics[0] != diagnostics[1]:
            raise RuntimeError('Classic source diagnostics differ between drivers')
    (work/'results.json').write_text(json.dumps(records, indent=2)+'\n')
    print(f'Classic standalone parity passes: {len(records)} tool commands; logs at {work}')


if __name__ == '__main__':
    main()
