"""Explicit STEP-04 installed/native functional checks; no performance or ASan.

Arguments: INSTALL_PREFIX SOURCE_ROOT MODEL_DIRECTORY FRESH_WORK_DIRECTORY MODES.
MODES is a comma-separated explicit selection, e.g. cpu,required-gpu on Metal.
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
work.mkdir(parents=True, exist_ok=False)
env = dict(os.environ, CREXX_HOME=str(prefix))
env.pop("CREXX_LLAMA_GLUE_PROBES", None)
clean = dict(env)
for key in ("CREXX_HOME", "DYLD_LIBRARY_PATH", "DYLD_FALLBACK_LIBRARY_PATH",
            "LD_LIBRARY_PATH", "GGML_BACKEND_PATH", "CREXX_PROVIDER_PATH"):
    clean.pop(key, None)
number = 0
records = []


def run(label, argv, marker=None, environment=None, cwd=None):
    global number
    number += 1
    result = subprocess.run(list(map(str, argv)), cwd=cwd or work, env=environment or env,
                            capture_output=True, text=True, timeout=300)
    output = result.stdout + result.stderr
    (work / f"{number:02}-{label}.log").write_text(
        f"argv={argv!r}\ncwd={cwd or work}\nrc={result.returncode}\n{output}")
    assert result.returncode == 0, (label, result.returncode, output[-3000:])
    assert not any(word in output for word in ("FAIL:", "PANIC:", "ERROR:")), (label, output[-3000:])
    if marker:
        assert marker in output, (label, output[-3000:])
    print(f"PASS: {label}", flush=True)


model = models / "bge-small-en-v1.5-f16.gguf"
digest = "f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999"
assert hashlib.sha256(model.read_bytes()).hexdigest() == digest
for test, marker in (
    ("embedding_acceptance", "PASS: rxllama persistent embedding acceptance"),
    ("shared_worker_embeddings", "PASS: four embedding workers"),
):
    # Source is copied outside the checkout; installed compilation uses no
    # checkout import roots. The fully linked worker image has explicit libraries.
    test_source = work / (test + ".crexx")
    shutil.copy2(source / "tests/native-inference" / test_source.name, test_source)
    program = work / (test + "-dynamic")
    run(test + "-compile", [prefix / "bin/rxc", "--no-exe-import", "-i", prefix / "bin",
                             "-o", program, test_source])
    run(test + "-assemble", [prefix / "bin/rxas", "-o", program, program])
    linked = str(program) + "-linked"
    run(test + "-link", [prefix / "bin/rxlink", "-o", linked, program,
                          prefix / "bin/library", prefix / "bin/classlib", prefix / "bin/rxfnsg"])
    for mode in modes:
        for vm in ("rxbvm", "rxtvm"):
            if (prefix / "bin" / vm).exists():
                run(f"{test}-installed-{vm}-{mode}",
                    [prefix / "bin" / vm, linked, "-a", mode, model, digest], marker,
                    environment=clean)
    native = work / (test + " native Ω") / "persistent"
    native.parent.mkdir()
    run(test + "-native-build", [prefix / "bin/crexx", "--program", native,
        test_source, "--jobs", "1", "--native"], "PUBLISHED: native program")
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
    for mode in modes:
        run(f"{test}-relocated-native-{mode}",
            [relocated / executable.name, mode, model, digest], marker,
            environment=clean, cwd=relocated)
    records.append({"test": test, "relocated": str(relocated), "files": {
        str(p.relative_to(relocated)): hashlib.sha256(p.read_bytes()).hexdigest()
        for p in sorted(relocated.rglob("*")) if p.is_file()}})
(work / "packages.json").write_text(json.dumps(records, indent=2) + "\n")
print("PASS: STEP-04 installed and relocated native embedding/worker consumers")
