"""Functional package controls: hashes, traversal, identity, collision, relocation.

No inference throughput measurements or model downloads. Takes the built
metadata helper and an existing rxllama.native.json; uses a temporary copy.
"""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

helper, source = map(lambda p: Path(p).resolve(), sys.argv[1:])
with tempfile.TemporaryDirectory(prefix="crexx llama package Å ") as work:
    root = Path(work)
    package = root / "package"
    package.mkdir()
    original = json.loads(source.read_text())
    assert not any("clang_rt" in e["path"] or e["path"].startswith(("libasan", "libubsan", "libtsan")) for e in original["runtime_files"]), "instrumentation runtime bundled twice"
    for entry in original["runtime_files"] + original["link_libraries"]:
        shutil.copyfile(source.parent / entry["path"], package / entry["path"])
    manifest = package / source.name
    manifest.write_text(json.dumps(original))
    consumer = root / "consumer space Ω" / "program"

    def fingerprint(success):
        result = subprocess.run([str(helper), "--fingerprint", str(manifest)], capture_output=True, text=True, timeout=60)
        assert (result.returncode == 0) == success, result.stdout + result.stderr
        if success:
            assert result.stdout.strip() == hashlib.sha256(manifest.read_bytes()).hexdigest()
        return result.stdout.strip()

    first = fingerprint(True)
    assert not consumer.parent.exists(), "fingerprint mutated destination"
    manifest.write_text(json.dumps(dict(original, engine=original["engine"] + "-changed")))
    assert fingerprint(True) != first, "changed manifest reused old fingerprint"
    manifest.write_text(json.dumps(original))
    dependency = package / original["runtime_files"][0]["path"]
    saved = dependency.read_bytes()
    dependency.write_bytes(b"modified dependency with stale manifest")
    fingerprint(False)
    dependency.write_bytes(saved)

    def run(data, success):
        manifest.write_text(json.dumps(data))
        result = subprocess.run([str(helper), str(manifest), str(consumer), "GNU"],
                                capture_output=True, text=True, timeout=60)
        assert (result.returncode == 0) == success, result.stdout + result.stderr
        return result

    for field, value in [("version", 999), ("platform", "OtherOS"), ("arch", "wrong")]:
        bad = dict(original, **{field: value})
        run(bad, False)
    bad = json.loads(json.dumps(original))
    bad["runtime_files"][0]["path"] = "../escape"
    run(bad, False)
    bad = json.loads(json.dumps(original))
    bad["runtime_files"][0]["sha256"] = "0" * 64
    run(bad, False)
    run(dict(original, command="must never execute"), False)
    assert not consumer.parent.exists(), "negative controls mutated destination"
    run(original, True)
    run(original, True)  # Idempotent, hashes unchanged.
    for entry in original["runtime_files"]:
        assert hashlib.sha256((consumer.parent / entry["path"]).read_bytes()).hexdigest() == entry["sha256"]
    collided = consumer.parent / original["runtime_files"][0]["path"]
    collided.write_bytes(b"application-owned change")
    run(original, False)
    assert collided.read_bytes() == b"application-owned change"
    print("PASS: native manifest identity/hash/traversal/collision/Unicode controls")
