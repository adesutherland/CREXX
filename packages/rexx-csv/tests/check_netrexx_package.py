#!/usr/bin/env python3
"""Check a fresh NetRexx source install and its runtime-only file consumer."""
import argparse
from pathlib import Path
import runpy
import sys
import tarfile
import tempfile

sys.dont_write_bytecode = True
from netrexx_support import ROOT, Toolchain


def check(tools):
    build = runpy.run_path(str(ROOT / 'packaging/build_netrexx.py'))['build']
    with tempfile.TemporaryDirectory(prefix='rexx csv NetRexx install ') as temp:
        temp = Path(temp)
        artifact, repeat = temp / 'netrexx.rxpkg', temp / 'repeat.rxpkg'
        build(artifact)
        build(repeat)
        assert artifact.read_bytes() == repeat.read_bytes(), 'artifact is nondeterministic'
        installed = temp / 'Installed NetRexx package Ω with spaces'
        installed.mkdir()
        with tarfile.open(artifact) as archive:
            names = [m.name for m in archive.getmembers()]
            assert set(names) == {'rexxcsv/Csv.nrx', 'rexxcsv/CsvRow.nrx',
                                  'rexxcsv/CsvDocument.nrx', 'rexxcsv/CsvOutput.nrx',
                                  'NetRexxDemo.nrx', 'rexx-csv-README.md',
                                  'rexx-csv-SPEC.md', 'rexx-csv-LICENSE.txt'}
            assert all(m.isfile() and m.mode == 0o644 for m in archive.getmembers())
            archive.extractall(installed, filter='data')
        sources = [Path(name) for name in names if name.endswith('.nrx')]
        readme = (installed / 'rexx-csv-README.md').read_text(encoding='utf-8')
        snippet = readme.split('```netrexx\n', 1)[1].split('```', 1)[0]
        (installed / 'Usage.nrx').write_text(snippet, encoding='utf-8')
        tools.compile(installed, sources + [Path('Usage.nrx')])
        usage = tools.run(installed, 'Usage')
        assert usage.strip() == 'name,town', usage
        # Re-run the exact SPEC consumer snippet too; no checkout imports or test classes.
        spec = (installed / 'rexx-csv-SPEC.md').read_text(encoding='utf-8')
        assert spec.split('```netrexx\n', 1)[1].split('```', 1)[0] == snippet
        root_readme = (ROOT / 'README.md').read_text(encoding='utf-8')
        assert root_readme.split('```netrexx\n', 1)[1].split('```', 1)[0] == snippet
        assert '2 rows' in tools.run(installed, 'NetRexxDemo')
        source, target = installed / 'input Ω file.csv', installed / 'output Ω file.csv'
        original = 'name,city\r\n"Ada, Jr.","Montréal"\n'.encode('utf-8')
        expected = 'name,city\r\n"Ada, Jr.",Montréal\r\n'.encode('utf-8')
        source.write_bytes(original)
        target.write_bytes(b'old contents' * 100)
        tools.run(installed, 'NetRexxDemo', source, target)
        assert target.read_bytes() == expected, 'exact bytes/truncation'
        source.write_bytes('a,"x\r\ny",🙂\nz,,"q""uote"'.encode('utf-8'))
        tools.run(installed, 'NetRexxDemo', source, target)
        assert target.read_bytes() == 'a,"x\r\ny",🙂\r\nz,"","q""uote"\r\n'.encode('utf-8')
        expected = target.read_bytes()
        for invalid, code in [(b'\xff', 'Invalid UTF-8'), (b'"unfinished', 'E_UNCLOSED_QUOTE'),
                              (b'a\x00', 'E_NUL')]:
            source.write_bytes(invalid)
            try:
                tools.run(installed, 'NetRexxDemo', source, target)
            except RuntimeError as error:
                assert code in str(error), error
            else:
                raise AssertionError('invalid input accepted')
            assert target.read_bytes() == expected, 'invalid input replaced output'
        source.write_bytes(b'x"' + b'a' * 16777215)
        try:
            tools.run(installed, 'NetRexxDemo', source, target)
        except RuntimeError as error:
            assert 'E_LIMIT input_bytes' in str(error), error
        else:
            raise AssertionError('oversized file accepted')
        assert target.read_bytes() == expected, 'oversized input replaced output'
        source.write_bytes(b'')
        tools.run(installed, 'NetRexxDemo', source, target)
        assert target.read_bytes() == b'', 'empty document did not truncate output'
        source.write_bytes(original)
        tools.run(installed, 'NetRexxDemo', source)
        tools.run(installed, 'NetRexxDemo', source, source)
        assert source.read_bytes() == 'name,city\r\n"Ada, Jr.",Montréal\r\n'.encode('utf-8')
        try:
            tools.run(installed, 'NetRexxDemo', installed / 'missing.csv', target)
        except RuntimeError as error:
            assert 'CSV file I/O failed' in str(error), error
        else:
            raise AssertionError('missing input accepted')
        assert target.read_bytes() == b'', 'open failure changed output'
        classes = list(installed.rglob('*.class'))
        assert classes and all(int.from_bytes(p.read_bytes()[6:8], 'big') == 52 for p in classes)
        # A precompiled install contains neither source nor the test orchestrator.
        for source_file in installed.rglob('*.nrx'):
            source_file.unlink()
        for name in ('rexx-csv-README.md', 'rexx-csv-SPEC.md', 'rexx-csv-LICENSE.txt', 'command.log'):
            (installed / name).unlink(missing_ok=True)
        assert '2 rows' in tools.run(installed, 'NetRexxDemo')
    print('PASS: deterministic NetRexx archive; fresh compile/installed consumer; runtime-only closure; UTF-8/CRLF and spaced paths; failure preserves output')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', required=True, type=Path)
    parser.add_argument('--java-home', type=Path)
    args = parser.parse_args()
    check(Toolchain(args.sdk, args.java_home))
