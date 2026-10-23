#!/usr/bin/env python3
"""Run the common literal oracle and Classic contracts on one explicit runtime."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]


def literal(value):
    return "'" + value.encode('utf-8').hex() + "'x" if value else "''"


HELPERS = r'''
check: procedure
  parse arg label, actual, expected
  if actual \== expected then do
    say 'FAIL' label 'expected' c2x(expected) 'got' c2x(actual)
    exit 1
  end
  return
'''


def common_source(cases):
    lines = ['numeric digits 20']

    def check(label, actual, expected):
        lines.append(f"call check {literal(label)}, {actual}, {literal(str(expected))}")

    def table(label, obj, rows):
        check(label + ' rows', f"rexx_csv('ROW_COUNT', {obj})", len(rows))
        for r, row in enumerate(rows, 1):
            check(label + f' width {r}', f"rexx_csv('FIELD_COUNT', {obj}, {r})", len(row))
            for c, value in enumerate(row, 1):
                lines.append(f"cell = rexx_csv('FIELD', {obj}, {r}, {c})")
                check(label + f' field status {r},{c}', "rexx_csv('OK', cell)", 1)
                check(label + f' cell {r},{c}', "rexx_csv('TEXT', cell)", value)

    for case in cases:
        ident, expected = case['id'], case['expected']
        if case['operation'] == 'parse':
            lines.append(f"value = rexx_csv('PARSE', {literal(case['input'])})")
        else:
            operands = []
            for r, row in enumerate(case['rows'], 1):
                args = ''.join(', ' + literal(v) for v in row)
                lines.append(f"row{r} = rexx_csv('ROW'{args})")
                operands.append(f'row{r}')
            args = ''.join(', ' + row for row in operands)
            lines += [f"original = rexx_csv('TABLE'{args})", "value = rexx_csv('WRITE', original)"]
        check(ident + ' status', "rexx_csv('OK', value)", int(expected['ok']))
        if expected['ok']:
            for operation in ('ERROR_CODE', 'ERROR', 'ERROR_LIMIT'):
                check(ident + ' ' + operation, f"rexx_csv('{operation}', value)", '')
            for operation in ('ERROR_RECORD', 'ERROR_FIELD'):
                check(ident + ' ' + operation, f"rexx_csv('{operation}', value)", 0)
            if case['operation'] == 'parse':
                table(ident, 'value', expected['rows'])
                lines.append("written = rexx_csv('WRITE', value)")
                check(ident + ' writer', "rexx_csv('OK', written)", 1)
                lines.append("again = rexx_csv('PARSE', rexx_csv('TEXT', written))")
                rows = expected['rows']
            else:
                check(ident + ' canonical bytes', "rexx_csv('TEXT', value)", expected['text'])
                lines.append("again = rexx_csv('PARSE', rexx_csv('TEXT', value))")
                rows = case['rows']
            check(ident + ' round trip', "rexx_csv('OK', again)", 1)
            table(ident + ' round trip', 'again', rows)
        else:
            error = expected['error']
            for operation, key, default in (('ERROR_CODE', 'code', ''), ('ERROR_RECORD', 'record', 0),
                                             ('ERROR_FIELD', 'field', 0), ('ERROR_LIMIT', 'limit', '')):
                check(ident + ' ' + operation, f"rexx_csv('{operation}', value)", error.get(key, default))
            check(ident + ' message', "length(rexx_csv('ERROR', value)) > 0", 1)
            if case['operation'] == 'parse':
                check(ident + ' atomic rows', "rexx_csv('ROW_COUNT', value)", 0)
                check(ident + ' invalid count', "rexx_csv('FIELD_COUNT', value, 1)", -1)
            check(ident + ' atomic text', "rexx_csv('TEXT', value)", '')
    lines += [f"say 'PASS: {len(cases)} common fixtures and valid round trips'", 'exit 0', HELPERS]
    return ('\n'.join(lines) + '\n').replace('rexx_csv(', "'rexx_csv'(")


def constructor_source():
    lines = ['numeric digits 20']
    for count in (1024, 1025):
        # Kept in one clause: Rexx's comma continuation can consume an argument
        # delimiter. These bounded calls are proven on each actual runtime.
        values = ', '.join(["'0012'"] * count)
        lines += [f"row = rexx_csv('ROW', {values})",
                  f"call check 'constructor width', rexx_csv('FIELD_COUNT', row, 1), '{count}'",
                  "written = rexx_csv('WRITE', row)"]
        if count == 1024:
            lines += ["call check 'constructor accepted', rexx_csv('OK', written), '1'",
                      "cell = rexx_csv('FIELD', row, 1, 1024)",
                      "call check 'constructor last', rexx_csv('TEXT', cell), '0012'"]
        else:
            for op, expected in [('OK', '0'), ('ERROR_CODE', 'E_LIMIT'),
                                 ('ERROR_RECORD', '1'), ('ERROR_FIELD', '1025'),
                                 ('ERROR_LIMIT', 'fields_per_record'), ('TEXT', '')]:
                lines.append(f"call check 'constructor excess', rexx_csv('{op}', written), '{expected}'")
    return ('\n'.join(lines + ["say 'PASS: 1,024-field ROW and first excess writer coordinate'", 'exit 0', HELPERS]) + '\n').replace('rexx_csv(', "'rexx_csv'(")


def run(runtime, source, directory, label, timeout=600, arguments=()):
    log = directory / 'command.log'
    started = time.monotonic()
    print(f'START: {label}', flush=True)
    timed_out = False
    with log.open('wb') as out, log.open('rb') as progress:
        process = subprocess.Popen([str(runtime), str(source), *arguments], cwd=directory,
                                   stdout=out, stderr=subprocess.STDOUT)
        try:
            while process.poll() is None:
                for line in progress.readlines():
                    print(line.decode('utf-8', errors='replace').rstrip(), flush=True)
                if time.monotonic() - started > timeout:
                    timed_out = True
                    process.kill()
                    break
                time.sleep(0.2)
            process.wait()
        finally:
            if process.poll() is None:
                process.kill()
                process.wait()
        for line in progress.readlines():
            print(line.decode('utf-8', errors='replace').rstrip(), flush=True)
    output = log.read_text(encoding='utf-8', errors='replace')
    elapsed = time.monotonic() - started
    if timed_out or process.returncode or 'FAIL' in output or 'PASS:' not in output:
        reason = 'timeout' if timed_out else f'exit {process.returncode}'
        raise RuntimeError(f'{label}: {reason} after {elapsed:.1f}s\n' + output[-6500:])
    print(f'PASS: {label} ({elapsed:.1f}s)', flush=True)
    return output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime', required=True, type=Path)
    parser.add_argument('--common-only', action='store_true')
    parser.add_argument('--groups', nargs='+', choices=('ownership', 'fields', 'counts', 'large', 'quotes'),
                        default=['ownership', 'fields', 'counts', 'large', 'quotes'])
    args = parser.parse_args()
    runtime = args.runtime.expanduser().resolve(strict=True)
    manifest = json.loads((ROOT / 'tests/fixtures/cases.json').read_text(encoding='utf-8'))
    assert manifest['schema_version'] == 1 and manifest['profile'] == 'rexx-csv-0.1'
    assert len({c['id'] for c in manifest['cases']}) == len(manifest['cases']) == 71
    with tempfile.TemporaryDirectory(prefix='rexx csv classic ') as temp:
        directory = Path(temp)
        shutil.copy2(ROOT / 'classic/rexx_csv.rexx', directory)
        version = directory / 'version.rexx'
        version.write_text("parse version runtime\nsay 'PASS: runtime' runtime\n", encoding='ascii')
        identity = run(runtime, version, directory, 'runtime identity')
        if not any(name in identity for name in ('REXX-Regina_', 'REXX-ooRexx_')):
            raise RuntimeError('Expected an actual Regina or ooRexx runtime')
        source = directory / 'common.rexx'
        source.write_text(common_source(manifest['cases']), encoding='ascii')
        run(runtime, source, directory, 'common fixtures')
        if not args.common_only:
            source = directory / 'constructors.rexx'
            source.write_text(constructor_source(), encoding='ascii')
            run(runtime, source, directory, 'variadic constructors')
            source = directory / 'contracts.rexx'
            shutil.copy2(ROOT / 'classic/tests/contracts.rexx', source)
            for group in args.groups:
                run(runtime, source, directory, 'Classic contracts ' + group,
                    timeout=1800, arguments=[group])


if __name__ == '__main__':
    main()
