"""Explicit typed installed/native correctness checks; no performance verdict.

In STEP-06, run instrumented builds through the maintained sanitizer runner.

Arguments: INSTALL_PREFIX SOURCE_ROOT MODEL_DIRECTORY FRESH_WORK_DIRECTORY MODES.
MODES is a comma-separated explicit selection, e.g. cpu,required-gpu on Metal.
Optional: --start-case opt-NAME|noopt-NAME resumes at a whole case in a fresh
work directory. Earlier successful cases require separately retained evidence.
All writes stay in the supplied disposable work directory. Models are read-only.
"""
from pathlib import Path
import hashlib
import json
import os
import shutil
import subprocess
import sys

prefix, source, models, work = [Path(p).resolve() for p in sys.argv[1:5]]
modes = sys.argv[5].split(",")
assert modes and all(mode in ("cpu", "required-gpu", "auto") for mode in modes)
assert len(sys.argv) == 6 or (len(sys.argv) == 8 and sys.argv[6] == "--start-case")
start_case = sys.argv[7] if len(sys.argv) == 8 else None
work.mkdir(parents=True, exist_ok=False)
env = dict(os.environ, CREXX_HOME=str(prefix))
env.pop("CREXX_LLAMA_GLUE_PROBES", None)
clean = dict(env)
for key in ("CREXX_HOME", "DYLD_LIBRARY_PATH", "DYLD_FALLBACK_LIBRARY_PATH",
            "LD_LIBRARY_PATH", "GGML_BACKEND_PATH", "CREXX_PROVIDER_PATH"):
    clean.pop(key, None)
number = 0
records = []
suffix = ".exe" if os.name == "nt" else ""
assert (prefix / ("bin/rxbvm" + suffix)).is_file(), "installed portable VM is missing"


def run(label, argv, marker=None, environment=None, cwd=None):
    global number
    number += 1
    result = subprocess.run(list(map(str, argv)), cwd=cwd or work, env=environment or env,
                            capture_output=True, text=True, timeout=1800)
    output = result.stdout + result.stderr
    (work / f"{number:02}-{label}.log").write_text(
        f"argv={argv!r}\ncwd={cwd or work}\nrc={result.returncode}\n{output}")
    assert result.returncode == 0, (label, result.returncode, output[-3000:])
    assert not any(word in output for word in ("FAIL:", "PANIC:", "ERROR:")), (label, output[-3000:])
    if marker:
        assert marker in output, (label, output[-3000:])
    print(f"PASS: {label}", flush=True)


model = models / "smollm2-360m-instruct-q8_0.gguf"
digest = "48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201"
assert hashlib.sha256(model.read_bytes()).hexdigest() == digest
cases = (
    ("typed_generation", source / "tests/native-inference/typed_generation.crexx", "PASS: typed llama generation"),
    ("generation_overhead", source / "tests/native-inference/generation_overhead.crexx", "PASS: NI-S5 fixed generation work"),
    ("persistent_generation", prefix / "share/crexx/llama/examples/persistent_generation.crexx", "PASS: persistent typed generation example"),
    ("shared_generation", prefix / "share/crexx/llama/examples/shared_generation.crexx", "PASS: four typed generation workers"),
)
assert start_case is None or start_case in {
    opt + "-" + name for opt in ("opt", "noopt") for name, _, _ in cases
}, "unknown continuation case"
started = start_case is None
for opt in ("opt", "noopt"):
  for name, original_source, marker in cases:
    test = opt + "-" + name
    if test == start_case:
        started = True
    if not started:
        continue
    test_modes = modes
    extra = ["4", "1"] if name == "generation_overhead" else []
    # Source is copied outside the checkout; installed compilation uses no
    # checkout import roots. The fully linked worker image has explicit libraries.
    test_source = work / (test + ".crexx")
    shutil.copy2(original_source, test_source)
    program = work / (test + "-dynamic")
    run(test + "-compile", [prefix / ("bin/rxc" + suffix), "--no-exe-import", *(["-n"] if opt == "noopt" else []), "-i", prefix / "bin",
                             "-o", program, test_source])
    run(test + "-assemble", [prefix / ("bin/rxas" + suffix), "-o", program, program])
    linked = str(program) + "-linked"
    run(test + "-link", [prefix / ("bin/rxlink" + suffix), "-o", linked, program,
                          prefix / "bin/library", prefix / "bin/classlib", prefix / "bin/rxfnsg"])
    for mode in test_modes:
        for vm in ("rxbvm", "rxtvm"):
            if (prefix / "bin" / (vm + suffix)).exists():
                run(f"{test}-installed-{vm}-{mode}",
                    [prefix / "bin" / (vm + suffix), linked, "-a", mode, model, digest, *extra], marker,
                    environment=clean)
    native = work / (test + " native Ω") / "persistent"
    native.parent.mkdir()
    run(test + "-native-build", [prefix / ("bin/crexx" + suffix), "--program", native,
        test_source, "--jobs", "1", "--native", *(["--nooptimize"] if opt == "noopt" else [])], "PUBLISHED: native program")
    executable = native.with_suffix(".exe") if os.name == "nt" else native
    relocated = work / (test + " relocated Å")
    relocated.mkdir()
    shutil.copy2(executable, relocated / executable.name)
    manifests = list(native.parent.glob("*.native.json"))
    assert any(p.name == "rxllama.native.json" for p in manifests), "native provider selection missing"
    for manifest_path in manifests:
        shutil.copy2(manifest_path, relocated / manifest_path.name)
        manifest = json.loads(manifest_path.read_text())
        for entry in manifest["runtime_files"]:
            path = Path(entry["path"])
            assert not path.is_absolute() and ".." not in path.parts
            target = relocated / path
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(native.parent / path, target)
    for mode in test_modes:
        run(f"{test}-relocated-native-{mode}",
            [relocated / executable.name, mode, model, digest, *extra], marker,
            environment=clean, cwd=relocated)
    records.append({"test": test, "relocated": str(relocated), "files": {
        str(p.relative_to(relocated)): hashlib.sha256(p.read_bytes()).hexdigest()
        for p in sorted(relocated.rglob("*")) if p.is_file()}})
(work / "packages.json").write_text(json.dumps(records, indent=2) + "\n")
print("PASS: typed STEP-05 installed and relocated native consumers, opt/noopt" +
      ("; continuation from " + start_case if start_case else "; complete matrix"))
