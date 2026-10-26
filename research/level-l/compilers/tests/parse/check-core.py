#!/usr/bin/env python3
"""Execute independently built PARSE IR and inspect its complete caller shape."""

import argparse
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import tempfile
import time

commands = 0


def run(command, success=True, silent=False, diagnostics=False):
    global commands
    commands += 1
    completed = subprocess.run(command, encoding='utf-8', capture_output=True, timeout=300)
    if (completed.returncode == 0) != success:
        raise RuntimeError(f"{command[0]} failed ({completed.returncode}):\n"
                           f"{completed.stdout}{completed.stderr}")
    silent = silent or Path(command[0]).stem in {'rxc', 'rxas', 'rxlink'}
    if success and (completed.stderr or (silent and completed.stdout)):
        raise RuntimeError(f"{command[0]} emitted unexpected diagnostics:\n"
                           f"{completed.stdout}{completed.stderr}")
    output = completed.stdout + (completed.stderr if diagnostics else '')
    return output.replace("\r\n", "\n")


def catalog_source(paths, target, variant='valid'):
    """Metadata-only RXAS: validate imports without running fabricated bodies."""
    declarations = {}
    classes = set()
    signature = re.compile(r'^\s*\.meta "([^"]+)"="b" "([^"]+)" \S+\(\) "([^"]*)"$')
    for path in paths:
        for row in path.read_text(encoding='utf-8').splitlines():
            if re.match(r'^\s*\.meta .* (?:\.class|\.attr \d+)$', row):
                classes.add(row.strip())
            match = signature.fullmatch(row)
            if match:
                name, result, arguments = match.groups()
                declarations[name] = (result, arguments)
    assert target in declarations, f'missing real producer metadata: {target}'
    output = ['.globals=0', *sorted(classes)]
    for index, (name, (result, arguments)) in enumerate(sorted(declarations.items())):
        language = 'b'
        if name == target:
            if variant == 'missing':
                continue
            if variant == 'language':
                language = 'g'
            if variant == 'result':
                result = '.string'
            parts = arguments.split(',')
            if variant == 'arity':
                parts.pop()
            if variant == 'type':
                parts[-1] = parts[-1].split('=')[0] + '=.string'
            if variant == 'optional':
                parts[-1] = '?' + parts[-1]
            arguments = ','.join(parts)
        label = f'metafixture{index}'
        output.extend((f'{label}() .locals=0 .expose={name}',
                       f' .meta "{name}"="{language}" "{result}" {label}() "{arguments}"', ' ret'))
    return '\n'.join(output) + '\n'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--bin", type=Path, required=True)
    parser.add_argument("--host", type=Path, required=True)
    parser.add_argument("--artifacts", type=Path, required=True)
    parser.add_argument("--vm", type=Path, required=True)
    parser.add_argument('--phase', choices=('all', 'checkpoint'), default='all')
    args = parser.parse_args()
    args.bin = args.bin.resolve()
    args.host = args.host.resolve()
    args.artifacts = args.artifacts.resolve()
    args.vm = args.vm.resolve()
    extension = '.exe' if os.name == 'nt' else ''
    tool = lambda name: str(args.bin / (name + extension))
    started = time.monotonic()
    with tempfile.TemporaryDirectory(prefix="parse-core-") as scratch:
        adapter_source = args.host.parent / 'llctarget.rxas'
        host = Path(scratch) / 'core-host.rxbin'
        shutil.copy2(args.host, host)
        args.host = host
        contracts = Path(scratch) / 'contracts'
        contracts.mkdir()
        for name in ('library', 'rxfnsc', 'llctarget'):
            shutil.copy2(args.artifacts / (name + '.rxbin'), contracts / (name + '.rxbin'))
        args.artifacts = contracts
        base = Path(scratch) / "parsecore"
        assembly = base.with_suffix(".rxas")
        if args.phase == 'checkpoint':
            run([str(args.vm), '--no-autoload', str(args.host), '-a',
                 str(args.artifacts), str(assembly), 'LINEIN'], silent=True)
            run([tool('rxas'), '-n', str(base)])
            checkpoint_run(args, scratch, base, tool)
            print(f"PARSE_CORE_CHECKPOINT|commands={commands}|failures=0|seconds={time.monotonic()-started:.3f}")
            return
        run([str(args.vm), "--no-autoload", str(args.host), "-a",
             str(args.artifacts), str(assembly)], silent=True)
        source = assembly.read_text(encoding="utf-8")
        assert "rexxclassicparse.execute()" in source
        assert "llctarget.parsesource()" in source
        assert re.search(r"\bload r\d+,0x4[Cc]435031", source), "binary LCP1 load missing"
        assert not re.search(r"\bparseplan\b|rxparse\.cursor", source), "caller duplicates cursor work"
        assert '"CLASSIC_CONDITION"' in source, "typed source checkpoint missing"
        current = None
        call_origins = {}
        for row in source.splitlines():
            if row.lstrip().startswith('.srcstep '):
                fields = shlex.split(row)
                current = tuple(fields[1:])
            for target in ('llctarget.parsesource', 'rexxclassicparse.execute'):
                if re.search(r'\bcall\b.*\b' + re.escape(target) + r'\(\)', row):
                    call_origins[target] = current
        assert set(call_origins) == {'llctarget.parsesource', 'rexxclassicparse.execute'}
        assert call_origins['llctarget.parsesource'] == call_origins['rexxclassicparse.execute'], \
            'source and execute must retain the same actual PARSE clause and span'
        assert call_origins['llctarget.parsesource'][1] == '1', 'owning PARSE byte start determines clause'
        finite = {'CLASSIC_SYNTAX', 'DIVISION_BY_ZERO', 'CONVERSION_ERROR',
                  'INVALID_ARGUMENTS', 'OBJECT_NOT_INITIALIZED', 'REFERENCE_INVALID',
                  'UNICODE_ERROR', 'OUT_OF_RANGE', 'FAILURE'}
        assert set(re.findall(r'\bsigbrv\s+[^\n]*,"([A-Z_]+)"', source)) == finite
        assert set(re.findall(r'\bsignalorigin\s+"([A-Z_]+)"', source)) == finite
        assert str(base) not in source, "temporary host identity leaked into RXAS"
        run([tool("rxas"), "-n", str(base)])
        run([tool("rxlink"), "-o", str(base.with_name("linked")),
             str(base.with_suffix(".rxbin")), str(args.artifacts / "llctarget.rxbin"),
             str(args.bin / "rxfnsc.rxbin"), str(args.bin / "library.rxbin"),
             str(args.bin / "classlib.rxbin"), str(args.bin / "rxunicode-core.rxbin")])
        result = run([str(args.vm), "--no-autoload",
                      str(base.with_name("linked").with_suffix(".rxbin")), "-a", "MiXeD"])
        assert result == "MiXeD\n", f"raw PARSE ARG was changed: {result!r}"
        metadata = Path(scratch) / 'metadata'
        metadata.mkdir()
        for name in ('library', 'rxfnsc', 'llctarget'):
            shutil.copy2(args.artifacts / (name + '.rxbin'), metadata / (name + '.rxbin'))
        members = args.bin.parent / 'lib/rxfnsc/members'
        producers = [('rxfnsc', 'rexxclassicparse.execute', sorted(members.glob('*/*.rxas'))),
                     ('llctarget', 'llctarget.parsesource', [adapter_source])]
        for package, target, producer in producers:
            for variant in ('valid', 'missing', 'language', 'result', 'arity', 'type', 'optional'):
                catalog = metadata / package
                catalog.with_suffix('.rxas').write_text(catalog_source(producer, target, variant), encoding='utf-8')
                run([tool('rxas'), '-n', str(catalog)])
                sentinel = Path(scratch) / 'previous.rxas'
                sentinel.write_text('previous output', encoding='utf-8')
                result = run([str(args.vm), '--no-autoload', str(args.host), '-a',
                              str(metadata), str(sentinel)], variant == 'valid',
                             silent=variant == 'valid')
                if variant == 'valid':
                    assert sentinel.read_text(encoding='utf-8') != 'previous output'
                else:
                    assert target in result and 'IMPORT:' in result, (target, variant, result)
                    assert sentinel.read_text(encoding='utf-8') == 'previous output', (target, variant)
            shutil.copy2(args.artifacts / (package + '.rxbin'), metadata / (package + '.rxbin'))
        for kind in ('SOURCE', 'LINEIN'):
            run([str(args.vm), '--no-autoload', str(args.host), '-a',
                 str(args.artifacts), str(assembly), kind], silent=True)
            run([tool('rxas'), '-n', str(base)])
            if kind == 'SOURCE':
                run([tool('rxlink'), '-o', str(base.with_name('linked')),
                     str(base.with_suffix('.rxbin')), str(args.artifacts / 'llctarget.rxbin'),
                     *(str(args.bin / (name + '.rxbin')) for name in
                       ('rxfnsc', 'library', 'classlib', 'rxunicode-core'))])
                result = run([str(args.vm), '--no-autoload',
                              str(base.with_name('linked').with_suffix('.rxbin'))])
                assert 'COMMAND' in result and 'core-parse-c' in result, \
                    f'SOURCE lost caller mode/logical identity: {result!r}'
                assert 'llctarget' not in result and scratch not in result
            else:
                checkpoint_run(args, scratch, base, tool)
    print(f"PARSE_CORE|checks=43|commands={commands}|failures=0|seconds={time.monotonic()-started:.3f}")


def checkpoint_run(args, scratch, base, tool):
    checkpoint = Path(scratch) / 'source-checkpoint'
    run([tool('rxc'), '--no-exe-import', '-n',
         '-i', scratch, '-i', str(args.bin), '-o', str(checkpoint),
         str(Path(__file__).with_name('source-checkpoint.crexx'))])
    run([tool('rxas'), '-n', str(checkpoint)])
    linked = Path(scratch) / 'checkpoint-linked'
    run([tool('rxlink'), '-o', str(linked),
         str(checkpoint.with_suffix('.rxbin')), str(base.with_suffix('.rxbin')),
         str(args.artifacts / 'llctarget.rxbin'),
         *(str(args.bin / (name + '.rxbin')) for name in
           ('rxfnsc', 'library', 'classlib', 'rxunicode-core'))])
    result = run([str(args.vm), '--no-autoload', str(linked.with_suffix('.rxbin'))])
    assert re.search(r'PARSE_C_CHECKPOINT\|checks=\d+\|failures=0', result), result
    # C's finite bridge deliberately halts even with an outer B handler.
    # Observe this original trigger separately from typed-condition transport.
    result = run([str(args.vm), '--no-autoload', str(linked.with_suffix('.rxbin')),
                  '-a', 'reference'], success=False, diagnostics=True)
    assert 'PARSE_C_REFERENCE_INVALID|before' in result, result
    assert re.findall(r'^PANIC:.*$', result, re.MULTILINE) == [
        'PANIC: Classic pool reference is not live (SIGNAL REFERENCE_INVALID)'], result
    assert re.search(r'at module \d+ \(RexxClassicContext\) address [1-9]\d*', result), result
    assert re.search(r'source: RexxClassicContext\.crexx:\d+:\d+:', result), result
    # Generic VM diagnostics preserve the payload and render authored callers separately.
    assert re.search(r'^  called from:\n'
                     r'  at module \d+ \([^\n]+\) address [1-9]\d* \(0x[0-9a-f]+\)\n'
                     r'  source: core-parse-c:1:1: parse linein x$', result, re.MULTILINE), result
    assert 'UNREACHED' not in result and 'Assertion failed' not in result, result


if __name__ == "__main__":
    main()
