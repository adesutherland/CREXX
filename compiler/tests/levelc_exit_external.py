#!/usr/bin/env python3
"""Prove EXIT returns from a separate signed Level C program boundary."""

import argparse
from pathlib import Path
import subprocess


def run(command, workdir, label, expected=b""):
    result = subprocess.run(command, cwd=workdir, capture_output=True,
                            timeout=120, check=False)
    (workdir / f"{label}.log").write_bytes(result.stdout + result.stderr)
    if result.returncode != 0 or result.stdout != expected:
        raise RuntimeError(
            f"{label}: exit={result.returncode}, stdout={result.stdout!r}, "
            f"stderr={result.stderr!r}; expected {expected!r}")


def main():
    parser = argparse.ArgumentParser()
    for key in ("rxc", "rxas", "rxlink", "rxvm", "bindir", "sourcedir", "workdir", "mode"):
        parser.add_argument(f"--{key}", required=True)
    args = parser.parse_args()
    if args.mode not in ("opt", "noopt"):
        raise ValueError(args.mode)
    for key in ("rxc", "rxas", "rxlink", "rxvm"):
        setattr(args, key, str(Path(getattr(args, key)).resolve()))
    source = Path(args.sourcedir).resolve()
    workdir = Path(args.workdir).resolve() / args.mode
    workdir.mkdir(parents=True, exist_ok=True)
    bindir = Path(args.bindir).resolve()
    binaries = []
    for stem, provider in (("levelc_exit_provider", True),
                           ("levelc_exit_eof_provider", True),
                           ("levelc_exit_consumer", False)):
        assembly = workdir / f"{stem}.rxas"
        binary = workdir / f"{stem}.rxbin"
        command = [args.rxc, "-s", str(source), "-i", str(bindir)]
        if args.mode == "noopt":
            command.append("-n")
        if provider:
            command.append("--levelc-routine")
        command += ["-o", str(assembly), str(source / f"{stem}.rexx")]
        run(command, workdir, f"{stem}_compile")
        run([args.rxas, "-o", str(binary), str(assembly)], workdir,
            f"{stem}_assemble")
        binaries.append(str(binary))
    image = workdir / "levelc_exit_image.rxbin"
    run([args.rxlink, "-o", str(image), *binaries,
         str(bindir / "library.rxbin"), str(bindir / "classlib.rxbin"),
         str(bindir / "rxfnsc.rxbin")], workdir, "link")
    run([args.rxvm, str(image)], workdir, "run",
        "value=雪|00|2|77\nevaluated-once\nonce=once|77\n"
        "empty=VAR|0|77\nbare=LIT|77\n"
        "provider-end\nprovider-resumed\neof=LIT|77\n".encode())


if __name__ == "__main__":
    main()
