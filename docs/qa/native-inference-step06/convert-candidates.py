"""S6-D01 candidate conversion; does not modify provider pins or approved models.

Run with the locked conversion interpreter. Arguments: LLAMA_SOURCE CHECKPOINTS
FRESH_OUTPUT. The source must be extracted from the recorded verified archive.
"""
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import time

source, inputs, output = [Path(s).resolve() for s in sys.argv[1:4]]
converter = source / "convert_hf_to_gguf.py"
assert hashlib.sha256(converter.read_bytes()).hexdigest() == "e38975e1c68d98ac1664dfd530616eb35c72294382a4dd873d4746b23f27779f"
for entry in json.loads((inputs / "inputs.json").read_text()):
    path = inputs / entry["model"] / entry["path"]
    assert hashlib.sha256(path.read_bytes()).hexdigest() == entry["sha256"], str(path)
output.mkdir(parents=True, exist_ok=False)
environment = dict(os.environ, HF_HUB_OFFLINE="1", TRANSFORMERS_OFFLINE="1",
                   PYTHONHASHSEED="0", OMP_NUM_THREADS="2", MKL_NUM_THREADS="2")
records = []
try:
    for repeat in (1, 2):
        dest = output / f"repeat-{repeat}"
        dest.mkdir()
        for name, outtype, filename in (
            ("bge", "f16", "bge-small-en-v1.5-f16.gguf"),
            ("smol", "q8_0", "smollm2-360m-instruct-q8_0.gguf"),
        ):
            target = dest / filename
            command = [sys.executable, str(converter), str(inputs / name),
                       "--outtype", outtype, "--outfile", str(target)]
            started = time.monotonic()
            with (dest / (name + ".log")).open("w") as log:
                result = subprocess.run(command, env=environment, cwd=source,
                                        stdout=log, stderr=log, timeout=900)
            row = {"repeat": repeat, "model": name, "argv": command,
                   "returncode": result.returncode, "elapsed_seconds": time.monotonic() - started}
            records.append(row)
            assert result.returncode == 0, str(dest / (name + ".log"))
            row.update(sha256=hashlib.sha256(target.read_bytes()).hexdigest(), size=target.stat().st_size)
            print("Converted:", repeat, name, row["sha256"], row["size"], flush=True)
    for name in ("bge", "smol"):
        rows = [row for row in records if row["model"] == name]
        assert rows[0]["sha256"] == rows[1]["sha256"], name + " conversion was not reproducible"
    print("PASS: both candidate GGUFs reproduce byte-for-byte", flush=True)
finally:
    record = {"python": sys.version, "executable": sys.executable,
              "executable_sha256": hashlib.sha256(Path(sys.executable).read_bytes()).hexdigest(),
              "platform": platform.platform(), "llama_commit": "5266f24da75dc449bd56cbed7addb9c8e4a6a73e",
              "source": str(source), "inputs": str(inputs),
              "environment": {key: environment[key] for key in (
                  "HF_HUB_OFFLINE", "TRANSFORMERS_OFFLINE", "PYTHONHASHSEED", "OMP_NUM_THREADS", "MKL_NUM_THREADS")},
              "conversions": records}
    (output / "conversion.json").write_text(json.dumps(record, indent=2) + "\n")
