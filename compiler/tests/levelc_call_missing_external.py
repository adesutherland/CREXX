#!/usr/bin/env python3
"""Missing Classic CALL targets fail at the reached clause after ordinary linking."""

import argparse
from pathlib import Path
import subprocess


def run(command, workdir, name, expect_success=True):
    completed = subprocess.run(command, cwd=workdir, capture_output=True,
                               timeout=120, check=False)
    output = completed.stdout + completed.stderr
    (workdir / f"{name}.log").write_bytes(output)
    if (completed.returncode == 0) != expect_success:
        raise RuntimeError(f"{name} exited {completed.returncode}; see {name}.log")
    return output


def main():
    parser = argparse.ArgumentParser()
    for key in ("rxc", "rxas", "rxlink", "rxvm", "bindir", "sourcedir",
                "workdir", "mode"):
        parser.add_argument(f"--{key}", required=True)
    args = parser.parse_args()
    if args.mode not in ("opt", "noopt"):
        raise ValueError(args.mode)
    for key in ("rxc", "rxas", "rxlink", "rxvm"):
        setattr(args, key, str(Path(getattr(args, key)).resolve()))
    bindir = Path(args.bindir).resolve()
    sourcedir = Path(args.sourcedir).resolve()
    workdir = Path(args.workdir).resolve() / args.mode
    workdir.mkdir(parents=True, exist_ok=True)
    for kind in ("unreachable", "reached"):
        stem = f"levelc_call_missing_{kind}"
        assembly = workdir / f"{stem}.rxas"
        binary = workdir / f"{stem}.rxbin"
        image = workdir / f"{stem}_image.rxbin"
        command = [args.rxc, "-s", str(sourcedir), "-i", str(bindir)]
        if args.mode == "noopt":
            command.append("-n")
        command += ["-o", str(assembly), str(sourcedir / f"{stem}.rexx")]
        run(command, workdir, f"{stem}_compile")
        text = assembly.read_text()
        if "RXC-LC-43.1" not in text or "missing_provider()" in text:
            raise RuntimeError(f"{stem}: missing target emitted a static import")
        run([args.rxas, "-o", str(binary), str(assembly)], workdir,
            f"{stem}_assemble")
        run([args.rxlink, "-o", str(image), str(binary),
             str(bindir / "library.rxbin"), str(bindir / "classlib.rxbin"),
             str(bindir / "rxfnsc.rxbin")], workdir, f"{stem}_link")
        output = run([args.rxvm, str(image)], workdir, f"{stem}_run",
                     expect_success=kind == "unreachable")
        if kind == "unreachable":
            if output != b"unreached-safe\n":
                raise RuntimeError(f"unreachable CALL output: {output!r}")
        elif (b"RXC-LC-43.1: Could not find routine MISSING_PROVIDER" not in output
              or b"source: levelc_call_missing_reached.rexx:2:1: call missing_provider" not in output
              or b"after\n" in output):
            raise RuntimeError(f"reached CALL diagnostic: {output!r}")

    provider_assembly = workdir / "levelc_call_alias_provider.rxas"
    provider_binary = workdir / "levelc_call_alias_provider.rxbin"
    provider_command = [args.rxc, "-s", str(sourcedir), "-i", str(bindir)]
    if args.mode == "noopt":
        provider_command.append("-n")
    provider_command += ["-o", str(provider_assembly),
                         str(sourcedir / "levelc_call_alias_provider.crexx")]
    run(provider_command, workdir, "alias_provider_compile")
    run([args.rxas, "-o", str(provider_binary), str(provider_assembly)],
        workdir, "alias_provider_assemble")
    consumer_assembly = workdir / "levelc_call_alias_consumer.rxas"
    consumer_binary = workdir / "levelc_call_alias_consumer.rxbin"
    consumer_image = workdir / "levelc_call_alias_consumer_image.rxbin"
    consumer_command = [args.rxc, "-s", str(sourcedir), "-i", str(bindir)]
    if args.mode == "noopt":
        consumer_command.append("-n")
    consumer_command += ["-o", str(consumer_assembly),
                         str(sourcedir / "levelc_call_alias_consumer.rexx")]
    run(consumer_command, workdir, "alias_consumer_compile")
    assembly_text = consumer_assembly.read_text()
    if ("levelc_call_alias.typedentry()" not in assembly_text or
            "levelc_call_alias_provider.typedentry()" in assembly_text):
        raise RuntimeError("source namespace did not determine CALL import")
    run([args.rxas, "-o", str(consumer_binary), str(consumer_assembly)],
        workdir, "alias_consumer_assemble")
    run([args.rxlink, "-o", str(consumer_image), str(consumer_binary),
         str(provider_binary), str(bindir / "library.rxbin"),
         str(bindir / "classlib.rxbin"), str(bindir / "rxfnsc.rxbin")],
        workdir, "alias_consumer_link")
    output = run([args.rxvm, str(consumer_image)], workdir,
                 "alias_consumer_run")
    if output != b"alias=ALIAS-RESULT\ndone\n":
        raise RuntimeError(f"alias consumer output: {output!r}")


if __name__ == "__main__":
    main()
