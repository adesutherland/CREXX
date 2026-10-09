#!/usr/bin/env python3
"""Whole explicit EXIT lifecycle through direct and ordinary linked images."""

import argparse
from pathlib import Path
import subprocess

from levelc_test_io import native_stdout


def run(command, workdir, label, expected_status=0, expected_output=b""):
    result = subprocess.run(command, cwd=workdir, capture_output=True,
                            timeout=120, check=False)
    (workdir / f"{label}.log").write_bytes(result.stdout + result.stderr)
    expected_output = native_stdout(expected_output)
    if result.returncode != expected_status or result.stdout != expected_output:
        raise RuntimeError(
            f"{label}: exit={result.returncode}, stdout={result.stdout!r}, "
            f"stderr={result.stderr!r}; expected {expected_status}, {expected_output!r}")


def main():
    parser = argparse.ArgumentParser()
    for key in ("rxc", "rxas", "rxlink", "rxvm", "bindir", "source", "workdir", "mode"):
        parser.add_argument(f"--{key}", required=True)
    parser.add_argument("--sourcedir", required=True)
    args = parser.parse_args()
    if args.mode not in ("opt", "noopt"):
        raise ValueError(args.mode)
    for key in ("rxc", "rxas", "rxlink", "rxvm", "source"):
        setattr(args, key, str(Path(getattr(args, key)).resolve()))
    sourcedir = Path(args.sourcedir).resolve()
    workdir = Path(args.workdir).resolve() / args.mode
    workdir.mkdir(parents=True, exist_ok=True)
    bindir = Path(args.bindir).resolve()
    assembly = workdir / "levelc_exit_whole.rxas"
    binary = workdir / "levelc_exit_whole.rxbin"
    image = workdir / "levelc_exit_whole_linked.rxbin"
    compile_command = [args.rxc, "-i", str(bindir)]
    if args.mode == "noopt":
        compile_command.append("-n")
    compile_command += ["-o", str(assembly), args.source]
    run(compile_command, workdir, "compile")
    run([args.rxas, "-o", str(binary), str(assembly)], workdir, "assemble")
    run([args.rxlink, "-o", str(image), str(binary),
         str(bindir / "library.rxbin"), str(bindir / "classlib.rxbin"),
         str(bindir / "rxfnsc.rxbin")], workdir, "link")
    cases = {
        "main": (7, b""),
        "bare": (0, b""),
        "local": (8, b"deepest;rc=77\n"),
        "private": (9, b""),
        "function": (10, b""),
        "actual": (10, b""),
        "assign": (10, b""),
        "return_expr": (10, b""),
        "loop": (13, b"loop=1\n"),
        "empty": (0, b""),
        "text": (0, b""),
    }
    for boundary, executable in (("direct", binary), ("linked", image)):
        for mode, (status, expected) in cases.items():
            run([args.rxvm, str(executable), str(bindir / "library"),
                 str(bindir / "classlib"), str(bindir / "rxfnsc"), "-a", mode],
                workdir, f"{boundary}_{mode}", status, expected)

    for stem, expected in (("levelc_exit_eof_main", b"top\n"),
                           ("levelc_exit_eof_local", b"last\nresumed=RESULT\n")):
        eof_assembly = workdir / f"{stem}.rxas"
        eof_binary = workdir / f"{stem}.rxbin"
        eof_image = workdir / f"{stem}_linked.rxbin"
        command = [args.rxc, "-i", str(bindir)]
        if args.mode == "noopt":
            command.append("-n")
        command += ["-o", str(eof_assembly), str(sourcedir / f"{stem}.rexx")]
        run(command, workdir, f"{stem}_compile")
        run([args.rxas, "-o", str(eof_binary), str(eof_assembly)], workdir,
            f"{stem}_assemble")
        run([args.rxlink, "-o", str(eof_image), str(eof_binary),
             str(bindir / "library.rxbin"), str(bindir / "classlib.rxbin"),
             str(bindir / "rxfnsc.rxbin")], workdir, f"{stem}_link")
        for boundary, executable in (("direct", eof_binary),
                                     ("linked", eof_image)):
            run([args.rxvm, str(executable), str(bindir / "library"),
                 str(bindir / "classlib"), str(bindir / "rxfnsc")],
                workdir, f"{stem}_{boundary}_run", 0, expected)


if __name__ == "__main__":
    main()
