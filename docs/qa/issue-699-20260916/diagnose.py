#!/usr/bin/env python3
"""Record #699's defective-baseline behavior; this is not a passing product test.

Usage: python3 diagnose.py /absolute/path/to/rxc [provider]
All compilation and modified controls live in a new temporary directory.
"""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time

compiler = Path(sys.argv[1]).resolve()
provider = len(sys.argv) > 2 and sys.argv[2] == "provider"
fixtures = Path(__file__).resolve().parent / ("provider-reproducer" if provider else "reproducer")
primary = "main" if provider else "model"
work = Path(tempfile.mkdtemp(prefix="crexx-699-controls-"))
results = []
for variant in ("closefile", "lineout", "no-extension"):
    for optimize in (True, False):
        case = work / (variant + ("-opt" if optimize else "-noopt"))
        shutil.copytree(fixtures, case)
        if variant == "lineout":
            source = case / "cleanup.crexx"
            source.write_text(source.read_text().replace("call closefile", "call lineout"))
        elif variant == "no-extension":
            (case / "extension.crexx").unlink()
        command = [str(compiler)] + ([] if optimize else ["-n"])
        command += ["-s", str(case), "-o", str(case / primary), str(case / (primary + ".crexx"))]
        start = time.monotonic()
        with (case / "compile.log").open("w") as log:
            try:
                rc = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, timeout=120).returncode
            except subprocess.TimeoutExpired:
                rc = "timeout"
        output = (case / "compile.log").read_text()
        expected = (2 if provider else 255) if variant == "closefile" else 0
        diagnostic = '#TYPE_MISMATCH: Type mismatch., "provider"' if provider else \
            "INTERNAL_CONVERGENCE_ERROR: Loop failed to converge. Active flags: 0x0002"
        matched = rc == expected and (expected == 0 or
            diagnostic in output)
        results.append(dict(case=case.name, command=command, rc=rc,
                            expected_defective_baseline_rc=expected,
                            matches_defective_baseline=matched,
                            seconds=time.monotonic() - start))
        print(case.name, "rc=" + str(rc), "baseline_match=" + str(matched))
report = dict(compiler=str(compiler),
              compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),
              source_sha256={f.name: hashlib.sha256(f.read_bytes()).hexdigest()
                             for f in sorted(fixtures.glob("*.crexx"))},
              results=results)
(work / "results.json").write_text(json.dumps(report, indent=2) + "\n")
print(work / "results.json")
sys.exit(0 if all(result["matches_defective_baseline"] for result in results) else 1)
