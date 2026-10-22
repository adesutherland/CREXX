#!/usr/bin/env python3
"""Qualify a disposable installed ANSI Lotto package using an explicit core."""
import argparse
import importlib.util
from pathlib import Path
import subprocess
import sys

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[3]
spec = importlib.util.spec_from_file_location(
    "console_pty_checks", ROOT / "lib/plugins/console/tests/pty_checks.py")
pty_checks = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pty_checks)


def check(directory: Path, binary: Path) -> None:
    output = directory / "Lotto-Tipps-cREXX.txt"
    model_output = directory / "lotto-test-output.txt"
    assert not output.exists() and not model_output.exists(), "use a fresh disposable install"

    def run(tool, *args):
        result = subprocess.run([str(binary / tool), *args], cwd=directory,
                                capture_output=True, text=True, timeout=120)
        assert result.returncode == 0, (tool, result.stdout, result.stderr)
        assert not result.stderr and "Warning" not in result.stdout, result.stdout + result.stderr
        return result.stdout

    print("Core:", run("rxc", "-v").strip(), flush=True)
    for name in ("lotto_logic", "lotto", "lotto_tests"):
        run("rxc", "-o", name, name + ".crexx")
        run("rxas", name)
    run("rxlink", "-o", "lotto-app", "lotto", "lotto_logic")
    run("rxlink", "-o", "lotto-checks", "lotto_tests", "lotto_logic")
    assert "PASS: cREXX Lotto (3825 tips; validation, save, replace, failure)" in run(
        "rxvm", "lotto-checks.rxbin")
    model_output.unlink()
    print("PASS clean compiler/assembler/linker/VM and 3825-tip model checks", flush=True)

    rejected = subprocess.run([str(binary / "rxvm"), "lotto-app.rxbin"], cwd=directory,
                              capture_output=True, text=True, timeout=30)
    assert rejected.returncode != 0 and "Open a real terminal" in rejected.stdout
    print("PASS redirected terminal is rejected clearly", flush=True)

    terminal = pty_checks.Terminal([str(binary / "rxvm"), "lotto-app.rxbin"], directory)

    def click(column, row):
        terminal.send(f"\x1b[<0;{column};{row}M\x1b[<0;{column};{row}m".encode())

    try:
        terminal.expect(b"Choose a game, enter 1-50 tips")
        assert not output.exists()
        terminal.send(b"et50\r")
        terminal.expect(b"Generated 50 tips")
        terminal.expect(b"Euro-Lotto: 5 from 50 + 2 from 12 - tips 1-8 of 50")
        assert not output.exists(), "Generate must not save"
        click(43, 10)
        terminal.expect(b"tips 9-16 of 50")
        terminal.send(b"\x1b[6~")
        terminal.expect(b"tips 17-24 of 50")
        terminal.send(b"\x1b[5~")
        terminal.expect(b"tips 9-16 of 50")
        terminal.send(b"t0\r")
        terminal.expect(b"Enter a whole number from 1 to 50. Results kept.")
        terminal.expect(b"tips 9-16 of 50")
        click(3, 4)
        terminal.expect(b"Selected Austria")
        click(18, 8)
        terminal.send(b"2")
        terminal.expect(b"Tips: enter a whole number from 1 to 50.")
        click(4, 10)
        terminal.expect(b"Generated 2 tips")
        terminal.expect(b"Austria: 6 from 45 - tips 1-2 of 2")
        assert not output.exists()
        print("PASS keyboard/mouse game, editor, Generate, paging and invalid input", flush=True)

        output.mkdir()  # A directory at the output name forces a real Save failure.
        click(18, 10)
        terminal.expect(b"S again: replace Lotto-Tipps-cREXX.txt")
        click(18, 10)
        terminal.expect(b"Save failed. Check folder permissions; results kept.")
        output.rmdir()
        terminal.send(b"s")
        terminal.expect(b"S again: replace Lotto-Tipps-cREXX.txt")
        assert not output.exists(), "first Save must only ask for confirmation"
        terminal.send(b"s")
        terminal.expect(b"Saved Lotto-Tipps-cREXX.txt")
        lines = output.read_text().splitlines()
        assert len(lines) == 3 and "Austria: 6 from 45" in lines[0]
        for line in lines[1:]:
            numbers = list(map(int, line.split(": ", 1)[1].split()))
            assert len(numbers) == 6 and numbers == sorted(set(numbers))
            assert all(1 <= number <= 45 for number in numbers)
        saved = output.read_bytes()
        terminal.send(b"d")
        terminal.expect(b"Selected Germany")
        terminal.send(b"s")
        terminal.expect(b"S again: replace Lotto-Tipps-cREXX.txt")
        terminal.send(b"s")
        terminal.expect(b"Saved Lotto-Tipps-cREXX.txt")
        assert output.read_bytes() == saved, "selection must not relabel the generated draw"
        terminal.resize(60, 20)
        terminal.expect(b"Resize to at least 80 columns x 24 rows")
        terminal.resize(80, 24)
        terminal.expect(b"tips 1-2 of 2")
        click(56, 10)
        terminal.finish()
        print("PASS Save confirmation/failure/recovery, generated identity, resize, mouse Quit and restoration", flush=True)
    finally:
        terminal.close()

    terminal = pty_checks.Terminal([str(binary / "rxvm"), "lotto-app.rxbin"], directory)
    try:
        terminal.expect(b"Choose a game, enter 1-50 tips")
        terminal.send(b"\x03")
        terminal.finish()
        assert output.read_bytes() == saved
        print("PASS Ctrl-C restores the terminal and preserves the saved file", flush=True)
    finally:
        terminal.close()
    output.unlink()

    # Project mode builds a linked program; launching it is a separate command.
    run("crexx", "-program", "lotto-driver-app", "lotto.crexx", "lotto_logic.crexx")
    terminal = pty_checks.Terminal([str(binary / "rxvm"), "lotto-driver-app.rxbin"], directory)
    try:
        terminal.expect(b"Choose a game, enter 1-50 tips")
        terminal.send(b"q")
        terminal.finish()
        assert not output.exists()
        print("PASS documented crexx -program build and foreground rxvm launch", flush=True)
    finally:
        terminal.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--install", required=True, type=Path)
    parser.add_argument("--bin", required=True, type=Path)
    args = parser.parse_args()
    check(args.install.resolve(), args.bin.resolve())
