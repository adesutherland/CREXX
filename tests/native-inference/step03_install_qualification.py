"""Explicit isolated installation/package measurement; no model downloads."""
from pathlib import Path
import subprocess
import sys
import tempfile
import time

build, source, models, record = [Path(p).resolve() for p in sys.argv[1:]]
record.mkdir(parents=True, exist_ok=True)
root = Path(tempfile.mkdtemp(prefix="crexx-llama-qualified-Å-"))
prefix = root / "installed Ω"
(record / "workspace.txt").write_text(str(root))
started = time.monotonic()
commands = [
    ["cmake", "--install", str(build), "--prefix", str(prefix)],
    [sys.executable, str(source / "tests/native-inference/package_acceptance.py"),
     str(prefix / "bin/crexx-provider-package"), str(prefix / "bin/providers/rxllama.native.json")],
    [sys.executable, str(source / "tests/native-inference/step03_package_consumer.py"),
     str(prefix), str(source), str(models), str(root / "checks")],
]
for index, command in enumerate(commands):
    with (record / f"{index:02}.log").open("w") as log:
        result = subprocess.run(command, stdout=log, stderr=log, timeout=7200)
    if result.returncode:
        print((record / f"{index:02}.log").read_text()[-4000:])
        raise SystemExit(result.returncode)
elapsed = time.monotonic() - started
(record / "elapsed-seconds.txt").write_text(f"{elapsed:.3f}\n")
print(f"PASS: scratch install, relocated CPU/GPU consumers, cache and dynamic/native workers; seconds={elapsed:.3f}; evidence={record}")
