#!/usr/bin/env python3
"""Compile and link the fixed external Classic CALL ABI without special link rules."""

import argparse
from pathlib import Path
import re
import subprocess

from levelc_call_delayed_injection import inject_boundaries


def run(command, workdir, label, success=True):
    result = subprocess.run(command, cwd=workdir, capture_output=True,
                            timeout=120, check=False)
    (workdir / f"{label}.log").write_bytes(result.stdout + result.stderr)
    if (result.returncode == 0) != success:
        raise RuntimeError(f"{label}: exit {result.returncode}; see {label}.log")
    return result.stdout + result.stderr


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
    workdir = Path(args.workdir).resolve() / args.mode
    workdir.mkdir(parents=True, exist_ok=True)
    bindir = Path(args.bindir).resolve()
    sourcedir = Path(args.sourcedir).resolve()
    binaries = []
    providers = []
    cases = (
        ("levelc_call_external_leaf", ".rexx", True),
        ("levelc_call_external_middle", ".rexx", True),
        ("levelc_call_external_handler", ".rexx", True),
        ("levelc_call_external_bg", ".crexx", False),
        ("levelc_call_external_consumer", ".rexx", False),
    )
    for stem, suffix, provider in cases:
        source = sourcedir / f"{stem}{suffix}"
        assembly = workdir / f"{stem}.rxas"
        binary = workdir / f"{stem}.rxbin"
        command = [args.rxc, "-s", str(sourcedir), "-i", str(bindir)]
        if args.mode == "noopt":
            command.append("-n")
        if provider:
            command.append("--levelc-routine")
        if stem == "levelc_call_external_consumer":
            command += ["--import", "levelc_call_external_bg"]
        command += ["-o", str(assembly), str(source)]
        run(command, workdir, f"{stem}_compile")
        run([args.rxas, "-o", str(binary), str(assembly)], workdir,
            f"{stem}_assemble")
        binaries.append(str(binary))
        if provider or stem == "levelc_call_external_bg":
            providers.append(str(binary))
        if provider:
            text = assembly.read_text()
            if ".expose=" not in text or "__rxcp_levelc_body()" not in text:
                raise RuntimeError(f"{stem}: missing exposed one-body provider")
            if re.search(r"(?m)^main\(\) \.locals=", text):
                raise RuntimeError(f"{stem}: provider exported an extra main")
    image = workdir / "levelc_call_external_image.rxbin"
    run([args.rxlink, "-o", str(image), *binaries,
         str(bindir / "library.rxbin"), str(bindir / "classlib.rxbin"),
         str(bindir / "rxfnsc.rxbin")], workdir, "link")
    output = run([args.rxvm, str(image)], workdir, "run")
    expected = (
        "leaf-args=3|1|0|1\n"
        "leaf-data=410042|3|2|É🙂\n"
        "leaf-pool=LIT\n"
        "result=410042|410042\n"
        "caller-pool=caller\n"
        "leaf-args=0|0|0|0\n"
        "leaf-data=|0|0|\n"
        "leaf-pool=LIT\n"
        "void=LIT|LIT\n"
        "local=LOCAL\n"
        "bg=BG-RESULT|BG-RESULT\n"
    ).encode()
    if output != expected:
        raise RuntimeError(f"unexpected output: {output!r}; expected {expected!r}")

    binary_only = workdir / "binary_only"
    binary_only.mkdir(exist_ok=True)
    binary_source = binary_only / "levelc_call_external_consumer.rexx"
    binary_source.write_bytes((sourcedir / binary_source.name).read_bytes())
    binary_assembly = binary_only / "levelc_call_external_consumer.rxas"
    binary_consumer = binary_only / "levelc_call_external_consumer.rxbin"
    binary_image = binary_only / "levelc_call_external_image.rxbin"
    binary_command = [args.rxc, "-i", str(workdir), "-i", str(bindir),
                      "--import", "levelc_call_external_bg"]
    if args.mode == "noopt":
        binary_command.append("-n")
    binary_command += ["-o", str(binary_assembly), str(binary_source)]
    run(binary_command, binary_only, "binary_only_compile")
    run([args.rxas, "-o", str(binary_consumer), str(binary_assembly)],
        binary_only, "binary_only_assemble")
    run([args.rxlink, "-o", str(binary_image), str(binary_consumer),
         *providers, str(bindir / "library.rxbin"),
         str(bindir / "classlib.rxbin"), str(bindir / "rxfnsc.rxbin")],
        binary_only, "binary_only_link")
    binary_output = run([args.rxvm, str(binary_image)], binary_only,
                        "binary_only_run")
    if binary_output != expected:
        raise RuntimeError(f"binary-only provider output: {binary_output!r}")

    trap_source = sourcedir / "levelc_call_external_trap.rexx"
    trap_assembly = workdir / "levelc_call_external_trap.rxas"
    trap_injected = workdir / "levelc_call_external_trap_injected.rxas"
    trap_binary = workdir / "levelc_call_external_trap.rxbin"
    trap_command = [args.rxc, "-s", str(sourcedir), "-i", str(bindir)]
    if args.mode == "noopt":
        trap_command.append("-n")
    trap_command += ["-o", str(trap_assembly), str(trap_source)]
    run(trap_command, workdir, "trap_compile")
    trap_injected.write_text(
        inject_boundaries(trap_assembly.read_text(), ((4, "external"),)),
        encoding="utf-8")
    run([args.rxas, "-o", str(trap_binary), str(trap_injected)],
        workdir, "trap_assemble")
    trap_image = workdir / "levelc_call_external_trap_image.rxbin"
    run([args.rxlink, "-o", str(trap_image), str(trap_binary), *providers,
         str(bindir / "library.rxbin"), str(bindir / "classlib.rxbin"),
         str(bindir / "rxfnsc.rxbin")], workdir, "trap_link")
    trap_output = run([args.rxvm, str(trap_image)], workdir, "trap_run")
    if trap_output != b"trap=ERROR|CALL|DELAY|0\nafter=prior|prior\n":
        raise RuntimeError(f"unexpected external trap output: {trap_output!r}")

    invalid_entries = {
        "badarg": (".void", "frame = .string"),
        "badreturn": (".string", "frame = .RexxActivationArguments"),
        "badarity": (".void", "frame = .RexxActivationArguments, extra = .string"),
        "badref": (".void", "frame = reference .RexxActivationArguments"),
    }
    for stem, (return_type, formals) in invalid_entries.items():
        (workdir / f"{stem}.crexx").write_text(
            "options levelb\n"
            f"namespace {stem} expose {stem}\n"
            "import rexxactivation\n"
            f"{stem}: procedure = {return_type}\n"
            f"  arg {formals}\n"
            + ("  return 'wrong'\n" if return_type == ".string"
               else "  return\n"), encoding="utf-8")
        consumer = workdir / f"{stem}_consumer.rexx"
        consumer.write_text(f"options levelc\ncall {stem}\n", encoding="utf-8")
        diagnostic = run(
            [args.rxc, "-s", str(workdir), "-i", str(bindir),
             "--import", stem, "-o", str(workdir / stem), str(consumer)],
            workdir, f"{stem}_negative", success=False)
        if b"#LEVELC_CALL_SIGNATURE" not in diagnostic:
            raise RuntimeError(f"{stem}: wrong compiler diagnostic {diagnostic!r}")


if __name__ == "__main__":
    main()
