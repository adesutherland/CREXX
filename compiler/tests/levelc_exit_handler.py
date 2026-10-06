#!/usr/bin/env python3
"""A delayed local CALL handler's EXIT ends the interrupted program."""

import argparse
from pathlib import Path
import subprocess
import sys

sys.dont_write_bytecode = True
from levelc_call_delayed_injection import inject_boundaries


def run(command, workdir, label, status=0, output=b""):
    result = subprocess.run(command, cwd=workdir, capture_output=True,
                            timeout=120, check=False)
    (workdir / f"{label}.log").write_bytes(result.stdout + result.stderr)
    if result.returncode != status or result.stdout != output:
        raise RuntimeError(
            f"{label}: exit={result.returncode}, stdout={result.stdout!r}, "
            f"stderr={result.stderr!r}; expected {status}, {output!r}")


def main():
    parser = argparse.ArgumentParser()
    for key in ("rxc", "rxas", "rxlink", "rxvm", "bindir", "source", "workdir", "mode"):
        parser.add_argument(f"--{key}", required=True)
    args = parser.parse_args()
    if args.mode not in ("opt", "noopt"):
        raise ValueError(args.mode)
    for key in ("rxc", "rxas", "rxlink", "rxvm", "source"):
        setattr(args, key, str(Path(getattr(args, key)).resolve()))
    bindir = Path(args.bindir).resolve()
    workdir = Path(args.workdir).resolve() / args.mode
    workdir.mkdir(parents=True, exist_ok=True)
    assembly = workdir / "levelc_exit_handler.rxas"
    injected = workdir / "levelc_exit_handler_injected.rxas"
    binary = workdir / "levelc_exit_handler.rxbin"
    image = workdir / "levelc_exit_handler_linked.rxbin"
    compile_command = [args.rxc, "-i", str(bindir)]
    if args.mode == "noopt":
        compile_command.append("-n")
    compile_command += ["-o", str(assembly), args.source]
    run(compile_command, workdir, "compile")
    injected.write_text(inject_boundaries(assembly.read_text(), ((3, "exit"),)),
                        encoding="utf-8")
    run([args.rxas, "-o", str(binary), str(injected)], workdir, "assemble")
    run([args.rxlink, "-o", str(image), str(binary),
         str(bindir / "library.rxbin"), str(bindir / "classlib.rxbin"),
         str(bindir / "rxfnsc.rxbin")], workdir, "link")
    for boundary, executable in (("direct", binary), ("linked", image)):
        run([args.rxvm, str(executable), str(bindir / "library"),
             str(bindir / "classlib"), str(bindir / "rxfnsc")],
            workdir, f"{boundary}_run", 12, b"handler\n")


if __name__ == "__main__":
    main()
