#!/usr/bin/env python3
"""Build and exercise an unpublished source artifact in a clean installation."""
import argparse
import os
from pathlib import Path
import runpy
import subprocess
import tarfile
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def check(binary):
    binary = binary.resolve()
    def tool(name):
        return binary / (name + (".exe" if os.name == "nt" else ""))
    build = runpy.run_path(str(ROOT / "packaging/build_crexx.py"))["build"]
    with tempfile.TemporaryDirectory(prefix="rexx-csv package ") as temp:
        temp = Path(temp)
        artifact = temp / "rexx-csv.rxpkg"
        repeat = temp / "repeat.rxpkg"
        build(artifact)
        build(repeat)
        assert artifact.read_bytes() == repeat.read_bytes(), "artifact is nondeterministic"
        installed = temp / "Installed CSV package with spaces"
        installed.mkdir()
        with tarfile.open(artifact) as archive:
            assert len(archive.getmembers()) == 5
            assert all(m.isfile() and "/" not in m.name and m.mode == 0o644
                       for m in archive.getmembers())
            archive.extractall(installed, filter="data")
        env = os.environ.copy()
        env["PATH"] = str(binary) + os.pathsep + env.get("PATH", "")

        def run(*args, success=True):
            log = temp / "command.log"
            with log.open("wb") as output:
                result = subprocess.run(list(map(str, args)), cwd=installed, env=env,
                                        stdout=output, stderr=subprocess.STDOUT, timeout=180)
            text = log.read_text(encoding="utf-8", errors="replace")
            if success:
                assert result.returncode == 0 and "Warning" not in text, text[-3500:]
            return result.returncode, text

        run(tool("crexx"), "-program", "csv-demo", "crexx_demo.crexx", "rexx_csv.crexx")
        _, text = run(tool("rxvm"), "csv-demo.rxbin")
        assert "PASS: CSV example; 2 rows; Ada, Jr." in text, text
        # Beta 3's Windows CLI uses narrow argv. Probe this separately from
        # CSV's Unicode text contract; the portable file example uses ASCII paths.
        if os.name == 'nt':
            source, target = installed / 'probe Ω input.csv', installed / 'probe Ω output.csv'
            source.write_bytes(b'name,city\nAda,London\n')
            target.write_bytes(b'keep existing output')
            status, text = run(tool('rxvm'), 'csv-demo.rxbin', '-a', source, target, success=False)
            if status:
                assert target.read_bytes() == b'keep existing output', text
                assert 'READBINARY cannot open' in text, text
                print('LIMIT: Windows Beta 3 CLI cannot transport Ω filenames; existing output preserved')
            else:
                assert target.read_bytes() == b'name,city\r\nAda,London\r\n'
                print('PASS: Windows Unicode filename probe')
        tag = '' if os.name == 'nt' else 'Ω '
        source = installed / ('input ' + tag + 'file.csv')
        target = installed / ('output ' + tag + 'file.csv')
        source.write_bytes('name,city\r\n"Ada, Jr.","Montréal"\n'.encode("utf-8"))
        _, text = run(tool("rxvm"), "csv-demo.rxbin", "-a", source, target)
        expected = 'name,city\r\n"Ada, Jr.",Montréal\r\n'.encode("utf-8")
        assert target.read_bytes() == expected
        source.write_bytes(b"\xff")
        status, text = run(tool("rxvm"), "csv-demo.rxbin", "-a", source, target, success=False)
        assert status != 0 and "UNICODE" in text.upper(), text
        assert target.read_bytes() == expected, "invalid input replaced the output"
        source.write_bytes(b'"unfinished')
        status, text = run(tool("rxvm"), "csv-demo.rxbin", "-a", source, target, success=False)
        assert status != 0 and "E_UNCLOSED_QUOTE" in text, text
        assert target.read_bytes() == expected, "invalid CSV replaced the output"
    print("PASS: deterministic source artifact; clean driver/VM installation; UTF-8/CRLF files; failure preserves output")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bin", type=Path, required=True, help="complete explicit SDK bin directory")
    args = parser.parse_args()
    check(args.bin)
