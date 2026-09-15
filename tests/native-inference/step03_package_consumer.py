"""Local lifecycle/package qualification, using an existing scratch install.

Arguments: INSTALL_PREFIX SOURCE_ROOT MODEL_DIRECTORY WORK_DIRECTORY.
All mutations are confined to that disposable install/work directory. Model
files remain read-only. Retains bounded command logs; not a performance test.
"""
from pathlib import Path
import hashlib
import json
import os
import shutil
import subprocess
import sys

prefix, source, models, work = [Path(p).resolve() for p in sys.argv[1:]]
work.mkdir(parents=True, exist_ok=True)
env = dict(os.environ, CREXX_HOME=str(prefix))
number = 0


def run(label, argv, marker=None, success=True, cwd=None, environment=None):
    global number
    number += 1
    result = subprocess.run(list(map(str, argv)), cwd=cwd or work, env=environment or env,
                            capture_output=True, text=True, timeout=180)
    output = result.stdout + result.stderr
    (work / f"{number:02}-{label}.log").write_text(
        f"argv={argv!r}\ncwd={cwd or work}\nrc={result.returncode}\n{output}")
    assert (result.returncode == 0) == success, (label, result.returncode, output[-3000:])
    if marker:
        assert marker in output, (label, output[-3000:])
    if success:
        assert not any(word in output for word in ("FAIL:", "PANIC:", "ERROR:")), (label, output[-3000:])
    print(f"PASS: {label}", flush=True)
    return output


provider = prefix / "bin/providers/rxllama.native.json"
original = provider.read_bytes()
manifest = json.loads(original)
program = work / "project Ω/persistent"
program.parent.mkdir()
driver = prefix / "bin/crexx"
command = [driver, "--program", program, source / "tests/native-inference/lifecycle_acceptance.crexx",
           "--jobs", "1", "--native"]
run("project-build", command, "PUBLISHED: native program")
run("project-no-op", command, "SKIP: native program current")
try:
    provider.write_text(json.dumps(dict(manifest, engine=manifest["engine"] + "-metadata-change")))
    run("selected-metadata-invalidation", command, "PUBLISHED: native program")
finally:
    provider.write_bytes(original)
run("restored-package", command, "PUBLISHED: native program")
runtime = prefix / "bin/providers" / manifest["runtime_files"][0]["path"]
runtime_bytes = runtime.read_bytes()
executable = program.with_suffix(".exe") if os.name == "nt" else program
exe_hash = hashlib.sha256(executable.read_bytes()).hexdigest()
try:
    runtime.write_bytes(b"changed dependency with stale metadata")
    run("stale-dependency-rejected", command, success=False)
    assert hashlib.sha256(executable.read_bytes()).hexdigest() == exe_hash
finally:
    runtime.write_bytes(runtime_bytes)
unrelated = provider.with_name("unused_llama_test.native.json")
assert not unrelated.exists()
try:
    unrelated.write_text("deliberately invalid unrelated provider metadata")
    run("unrelated-metadata-ignored", command, "SKIP: native program current")
finally:
    unrelated.unlink()

relocated = work / "relocated Å"
relocated.mkdir()
shutil.copy2(executable, relocated / executable.name)
for entry in manifest["runtime_files"]:
    shutil.copy2(program.parent / entry["path"], relocated / entry["path"])
shutil.copy2(program.parent / provider.name, relocated / provider.name)
clean = dict(os.environ)
for key in ("CREXX_HOME", "DYLD_LIBRARY_PATH", "DYLD_FALLBACK_LIBRARY_PATH", "LD_LIBRARY_PATH", "GGML_BACKEND_PATH"):
    clean.pop(key, None)
pins = [
    ("bge-small-en-v1.5-f16.gguf", "f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999", "bge-small-en-v1.5"),
    ("smollm2-360m-instruct-q8_0.gguf", "48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201", "smollm2-360m-instruct"),
]
for filename, digest, profile in pins:
    for mode in ("cpu", "required-gpu"):
        run(f"relocated-{profile}-{mode}", [relocated / executable.name, mode, models / filename, digest, profile],
            "PASS: rxllama lifecycle", cwd=relocated, environment=clean)

# With a packaged GPU module absent, an ambient path must not rescue it.
runtime_manifest = relocated / "rxllama.runtime.json"
backends = json.loads(runtime_manifest.read_text())["backends"]
removed = []
try:
    for backend in backends:
        if not backend["backend"].startswith("cpu"):
            path = relocated / backend["path"]
            removed.append((path, path.read_bytes()))
            path.unlink()
    poisoned = dict(clean, GGML_BACKEND_PATH=str(prefix / "bin/providers"))
    filename, digest, profile = pins[0]
    base = [relocated / executable.name]
    tail = [models / filename, digest, profile]
    run("missing-gpu-auto-fallback", base + ["auto"] + tail, "PASS: rxllama lifecycle", cwd=relocated, environment=poisoned)
    run("missing-required-gpu-rejected", base + ["required-gpu"] + tail, success=False, cwd=relocated, environment=poisoned)
finally:
    for path, data in removed:
        path.write_bytes(data)
saved_manifest = runtime_manifest.read_bytes()
try:
    runtime_manifest.unlink()
    run("missing-runtime-manifest-rejected", base + ["cpu"] + tail, success=False, cwd=relocated, environment=clean)
finally:
    runtime_manifest.write_bytes(saved_manifest)

# Exercise the same workers through both installed dynamic and native routes.
# This matrix stays an explicit target until its expanded sanitizer timing is
# measured; a previous dynamic-only measurement is not its scheduling evidence.
for test in ("worker_transport_positive", "provider_worker_acceptance", "shared_worker_lifecycle"):
    target = work / test
    run(f"{test}-compile", [prefix / "bin/rxc", "--no-exe-import", "-i", prefix / "bin", "-o", target, source / "tests/native-inference" / (test + ".crexx")])
    run(f"{test}-assemble", [prefix / "bin/rxas", "-o", target, target])
    linked = str(target) + "-linked"
    run(f"{test}-link", [prefix / "bin/rxlink", "-o", linked, target, prefix / "bin/library"])
    cases = [("positive", [], "PASS:")]
    if test == "provider_worker_acceptance":
        cases = [("foreign", [models / pins[0][0], pins[0][1]], "PASS: rxllama foreign")]
    elif test == "shared_worker_lifecycle":
        cases = [(f"{profile}-{mode}", [mode, models / filename, digest, profile], "PASS: four cREXX workers")
                 for filename, digest, profile in pins for mode in ("cpu", "required-gpu")]
    for label, args, marker in cases:
        run(f"{test}-{label}", [prefix / "bin/rxbvm", linked, "-a"] + args,
            marker, cwd=prefix / "bin")
    native = work / "native workers" / test
    native.parent.mkdir(exist_ok=True)
    run(f"{test}-native-build", [driver, "--program", native,
        source / "tests/native-inference" / (test + ".crexx"), "--jobs", "1", "--native"],
        "PUBLISHED: native program")
    if os.name == "nt":
        native = native.with_suffix(".exe")
    for label, args, marker in cases:
        run(f"{test}-native-{label}", [native] + args, marker,
            cwd=native.parent, environment=clean)
