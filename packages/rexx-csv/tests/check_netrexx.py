#!/usr/bin/env python3
"""Compile and execute the common CSV oracle and NetRexx contracts."""
import argparse
import json
from pathlib import Path
import shutil
import sys
import tempfile

sys.dont_write_bytecode = True
from netrexx_support import ROOT, Toolchain


def literal(value):
    return "CsvChecks.text('" + value.encode('utf-8').hex() + "')"


def common_source(cases):
    lines = ['options binary', 'import rexxcsv.Csv', 'import rexxcsv.CsvRow',
             'class CsvCommonTests public final', '  method main(args = String[]) public static']
    lines += [f'    case_{i}()' for i in range(len(cases))]
    lines += [f"    say 'PASS: {len(cases)} common fixtures and every valid round trip'"]

    def number(label, expression, expected):
        lines.append(f"    CsvChecks.number('{label}', {expression}, {expected})")

    def string(label, expression, expected):
        lines.append(f"    CsvChecks.equal('{label}', {expression}, {literal(expected)})")

    def flag(label, expression, expected):
        lines.append(f"    CsvChecks.flag('{label}', {expression}, {int(expected)})")

    def table(label, name, rows):
        number(label + ' rows', f'{name}.rowCount()', len(rows))
        for r, row in enumerate(rows, 1):
            number(label + f' width {r}', f'{name}.fieldCount({r})', len(row))
            for c, value in enumerate(row, 1):
                string(label + f' ({r},{c})', f'{name}.field({r}, {c})', value)

    for index, case in enumerate(cases):
        identifier, expected = case['id'], case['expected']
        lines.append(f'  method case_{index}() private static')
        if case['operation'] == 'parse':
            obj = 'document'
            lines.append(f"    document = Csv.parse({literal(case['input'])})")
        else:
            obj = 'output'
            lines.append(f"    rows = CsvRow[{len(case['rows'])}]")
            for r, row in enumerate(case['rows']):
                lines.append(f'    fields = String[{len(row)}]')
                for c, value in enumerate(row):
                    lines.append(f'    fields[{c}] = {literal(value)}')
                lines.append(f'    rows[{r}] = Csv.row(fields)')
            lines.append('    output = Csv.write(rows)')
        flag(identifier + ' status', obj + '.ok()', expected['ok'])
        if expected['ok']:
            for method in ('errorCode', 'errorLimit', 'error'):
                string(identifier + ' ' + method, f'{obj}.{method}()', '')
            for method in ('errorRecord', 'errorField'):
                number(identifier + ' ' + method, f'{obj}.{method}()', 0)
            if obj == 'document':
                table(identifier, obj, expected['rows'])
                lines.append('    output = Csv.write(document.rows())')
                flag(identifier + ' writer', 'output.ok()', True)
                rows = expected['rows']
            else:
                string(identifier + ' canonical bytes', 'output.text()', expected['text'])
                rows = case['rows']
            lines.append('    again = Csv.parse(output.text())')
            flag(identifier + ' round trip', 'again.ok()', True)
            table(identifier + ' round trip', 'again', rows)
        else:
            error = expected['error']
            lines.append(f"    CsvChecks.diagnostic('{identifier}', {obj}, '{error['code']}', "
                         f"{error['record']}, {error['field']}, '{error.get('limit', '')}')")
    return '\n'.join(lines) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', required=True, type=Path, help='complete NetRexx SDK directory')
    parser.add_argument('--java-home', type=Path, help='explicit JDK/JRE home; defaults to PATH java')
    parser.add_argument('--common-only', action='store_true')
    args = parser.parse_args()
    tools = Toolchain(args.sdk, args.java_home)
    manifest = json.loads((ROOT / 'tests/fixtures/cases.json').read_text(encoding='utf-8'))
    assert manifest['schema_version'] == 1 and manifest['profile'] == 'rexx-csv-0.1'
    assert len({c['id'] for c in manifest['cases']}) == len(manifest['cases']) == 71
    with tempfile.TemporaryDirectory(prefix='rexx csv NetRexx ') as temp:
        temp = Path(temp)
        sources = tools.copy_library(temp)
        shutil.copy2(ROOT / 'netrexx/tests/CsvChecks.nrx', temp)
        source = temp / 'CsvCommonTests.nrx'
        source.write_text(common_source(manifest['cases']), encoding='ascii')
        sources += [Path('CsvChecks.nrx'), Path('CsvCommonTests.nrx')]
        if not args.common_only:
            shutil.copy2(ROOT / 'netrexx/tests/CsvContracts.nrx', temp)
            sources.append(Path('CsvContracts.nrx'))
        print(tools.command([tools.java, '-version'], temp).strip(), flush=True)
        tools.compile(temp, sources)
        print(tools.run(temp, 'CsvCommonTests').strip(), flush=True)
        if not args.common_only:
            print(tools.run(temp, 'CsvContracts').strip(), flush=True)
        classes = list(temp.rglob('*.class'))
        assert classes and all(int.from_bytes(p.read_bytes()[6:8], 'big') == 52 for p in classes)
        print('PASS: fresh compiled classes; runtime-only JAR closure; class-file target 52', flush=True)


if __name__ == '__main__':
    main()
