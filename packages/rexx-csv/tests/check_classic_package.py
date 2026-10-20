#!/usr/bin/env python3
"""Check an unpublished Classic archive as a consumer on one actual runtime."""
import argparse
from pathlib import Path
import runpy
import subprocess
import tarfile
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def check(runtime):
    runtime = runtime.expanduser().resolve(strict=True)
    build = runpy.run_path(str(ROOT / 'packaging/build_classic.py'))['build']
    with tempfile.TemporaryDirectory(prefix='rexx csv installed ') as temp:
        temp = Path(temp)
        artifact, repeat = temp / 'classic.rxpkg', temp / 'repeat.rxpkg'
        build(artifact)
        build(repeat)
        assert artifact.read_bytes() == repeat.read_bytes(), 'artifact is nondeterministic'
        installed = temp / 'Installed CSV package Ω with spaces'
        installed.mkdir()
        with tarfile.open(artifact) as archive:
            assert len(archive.getmembers()) == 5
            assert all(m.isfile() and '/' not in m.name and m.mode == 0o644
                       for m in archive.getmembers())
            archive.extractall(installed, filter='data')

        def run(command_string=None, success=True, script='classic_demo.rexx', marker='PASS: CSV example;'):
            args = [str(runtime), script]
            if command_string is not None:
                args.append(command_string)
            log = temp / 'command.log'
            with log.open('wb') as out:
                result = subprocess.run(args, cwd=installed, stdout=out,
                                        stderr=subprocess.STDOUT, timeout=180)
            text = log.read_text(encoding='utf-8', errors='replace')
            if success:
                assert result.returncode == 0 and marker in text, text[-2500:]
            return result.returncode, text

        run()
        readme = (installed / 'rexx-csv-README.md').read_text(encoding='utf-8')
        snippet = readme.split('```rexx\n', 1)[1].split('```', 1)[0]
        (installed / 'usage.rexx').write_text(snippet, encoding='utf-8')
        _, text = run(script='usage.rexx', marker='Grace,London')
        assert text.startswith('Ada\n'), text
        source, target = installed / 'input Ω file.csv', installed / 'output Ω file.csv'
        paths = f'"{source}" "{target}"'
        original = 'name,city\r\n"Ada, Jr.","Montréal"\n'.encode('utf-8')
        expected = 'name,city\r\n"Ada, Jr.",Montréal\r\n'.encode('utf-8')
        source.write_bytes(original)
        target.write_bytes(b'old contents' * 100)
        run(paths)
        assert target.read_bytes() == expected, 'exact UTF-8/CRLF/truncation'
        source.write_bytes('a,"x\r\ny",🙂\nz,,"q""uote"'.encode('utf-8'))
        run(paths)
        assert target.read_bytes() == 'a,"x\r\ny",🙂\r\nz,"","q""uote"\r\n'.encode('utf-8')
        expected = target.read_bytes()
        for invalid, code in [(b'\xff', 'E_ARGUMENT'), (b'"unfinished', 'E_UNCLOSED_QUOTE'),
                              (b'a\x00', 'E_NUL')]:
            source.write_bytes(invalid)
            status, text = run(paths, success=False)
            assert status != 0 and code in text, text
            assert target.read_bytes() == expected, 'invalid input replaced output'
        source.write_bytes(b'')
        run(paths)
        assert target.read_bytes() == b'', 'empty document truncates old output'
        source.write_bytes(original)
        run(f'"{source}"')
        # The source is fully read and closed before replacing the same file.
        run(f'"{source}" "{source}"')
        assert source.read_bytes() == 'name,city\r\n"Ada, Jr.",Montréal\r\n'.encode('utf-8')
        status, text = run(f'"{installed / "missing.csv"}" "{target}"', success=False)
        assert status != 0 and 'Cannot open input' in text, text
        assert target.read_bytes() == b'', 'open failure replaced output'
    print('PASS: deterministic Classic source archive; fresh consumer; spaced/Unicode paths; exact UTF-8/CRLF; failure preserves output')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime', type=Path, required=True)
    args = parser.parse_args()
    check(args.runtime)
