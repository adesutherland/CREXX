"""Run one explicit STEP-06 correctness suite in a fresh retained directory.

This dispatches existing workloads without changing their assertions/counts.
Use the CMake qualification targets through tools/asan-run.sh for ASan. It is
not a CTest registration or a performance comparison runner.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time

parser = argparse.ArgumentParser()
parser.add_argument("--build", type=Path, required=True)
parser.add_argument("--source", type=Path, required=True)
parser.add_argument("--models", type=Path, required=True)
parser.add_argument("--output-root", type=Path, required=True)
parser.add_argument("--modes", default="cpu")
parser.add_argument("--suite", choices=("typed", "embedding-legacy", "generation", "generation-closeout",
                                       "package-typed", "package-generation"), required=True)
args = parser.parse_args()
build, source, models, output_root = [p.resolve() for p in
                                     (args.build, args.source, args.models, args.output_root)]
assert args.modes and all(m in ("cpu", "required-gpu", "auto") for m in args.modes.split(","))
output_root.mkdir(parents=True, exist_ok=True)
record = Path(tempfile.mkdtemp(prefix=args.suite + "-", dir=output_root))
print("QUALIFICATION_RECORD=" + str(record), flush=True)
started = time.monotonic()
commands = []


def run(label, command):
    command = list(map(str, command))
    with (record / (label + ".log")).open("w") as log:
        log.write("argv=" + repr(command) + "\n")
        log.flush()
        result = subprocess.run(command, cwd=source, stdout=log, stderr=log, timeout=7200)
    commands.append({"label": label, "argv": command, "returncode": result.returncode})
    if result.returncode:
        print((record / (label + ".log")).read_text()[-5000:], flush=True)
        raise RuntimeError(label + " failed: " + str(result.returncode))


driver = None
try:
    if args.suite == "embedding-legacy":
        driver = "step04_toolchain.cmake"
        for program, marker, modes in (
            ("embedding_text_boundary", "PASS: complete text is validated", ["cpu"]),
            ("embedding_acceptance", "PASS: rxllama persistent embedding acceptance", args.modes.split(",")),
        ):
            for opt in ("opt", "noopt"):
                for mode in modes:
                    label = program + "-" + opt + "-" + mode
                    run(label, ["cmake", f"-DBUILD={build}", f"-DSOURCE={source}",
                        f"-DOUTPUT={record / 'work' / label}", f"-DMODE={mode}",
                        f"-DMODEL={models / 'bge-small-en-v1.5-f16.gguf'}",
                        "-DHASH=f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999",
                        f"-DPROGRAM={program}", f"-DMARKER={marker}",
                        "-DNO_OPT=" + ("ON" if opt == "noopt" else "OFF"),
                        "-P", source / "tests/native-inference" / driver])
    elif args.suite.startswith("package-"):
        prefix = record / "install"
        run("install", ["cmake", "--install", build, "--prefix", prefix])
        driver = "typed_package_consumer.py" if args.suite == "package-typed" else "generation_package_consumer.py"
        command = [sys.executable, source / "tests/native-inference" / driver,
                   prefix, source, models, record / "work", args.modes]
    else:
        driver = "typed_toolchain.py" if args.suite == "typed" else "generation_toolchain.py"
        command = [sys.executable, source / "tests/native-inference" / driver,
                   build, source, models, record / "work", args.modes]
        if args.suite == "generation-closeout":
            command.append("--closeout")
    if args.suite != "embedding-legacy":
        run("suite", command)
    print("PASS: STEP-06 " + args.suite + " modes=" + args.modes, flush=True)
finally:
    identity = {"suite": args.suite, "modes": args.modes,
                "elapsed_seconds": round(time.monotonic() - started, 3),
                "build": str(build), "source": str(source), "models": str(models),
                "asan_options": os.environ.get("ASAN_OPTIONS", ""),
                "commands": commands}
    if driver:
        path = source / "tests/native-inference" / driver
        identity["driver_sha256"] = hashlib.sha256(path.read_bytes()).hexdigest()
    (record / "dispatch.json").write_text(json.dumps(identity, indent=2) + "\n")
