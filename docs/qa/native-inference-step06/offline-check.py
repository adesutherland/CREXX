"""macOS STEP-06 offline proof using the unchanged STEP-07 native examples.

Arguments: PROGRAM_DIRECTORY MODEL_DIRECTORY FRESH_OUTPUT. Denial is confined
to child processes; no machine firewall or network configuration is changed.
"""
import hashlib
from http.server import BaseHTTPRequestHandler, HTTPServer
import json
import os
from pathlib import Path
import subprocess
import sys
from threading import Thread

programs, models, output = [Path(p).resolve() for p in sys.argv[1:4]]
output.mkdir(parents=True, exist_ok=False)
deny = ["/usr/bin/sandbox-exec", "-p", "(version 1)(allow default)(deny network*)"]
environment = dict(os.environ)
for key in ("CREXX_HOME", "DYLD_LIBRARY_PATH", "DYLD_FALLBACK_LIBRARY_PATH",
            "LD_LIBRARY_PATH", "GGML_BACKEND_PATH", "CREXX_PROVIDER_PATH"):
    environment.pop(key, None)
records = []


def run(name, command):
    result = subprocess.run(list(map(str, command)), env=environment, cwd=programs,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=180, text=True)
    (output / (name + ".log")).write_text(result.stdout)
    records.append({"name": name, "argv": list(map(str, command)), "returncode": result.returncode})
    (output / "results.json").write_text(json.dumps(records, indent=2) + "\n")
    return result


class Probe(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.end_headers()
        self.wfile.write(b"offline-proof-listener\n")

    def log_message(self, *_):
        pass


listener = HTTPServer(("127.0.0.1", 0), Probe)
worker = Thread(target=listener.serve_forever)
worker.start()
try:
    curl = ["/usr/bin/curl", "--noproxy", "*", "--silent", "--show-error", "--fail",
            "--max-time", "3", f"http://127.0.0.1:{listener.server_port}/"]
    for name, prefix, expected in (("network-before", [], True),
                                   ("network-denied", deny, False),
                                   ("network-after", [], True)):
        result = run(name, prefix + curl)
        if expected:
            assert result.returncode == 0 and "offline-proof-listener" in result.stdout, name
        else:
            assert result.returncode != 0 and "offline-proof-listener" not in result.stdout, name
            assert "Failed to connect" in result.stdout or "Operation not permitted" in result.stdout, result.stdout
finally:
    listener.shutdown()
    listener.server_close()
    worker.join()

identities = {}
for name, filename, digest, marker in (
    ("embeddings", "bge-small-en-v1.5-f16.gguf", "f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999", "PASS: persistent typed embedding example"),
    ("generation", "smollm2-360m-instruct-q8_0.gguf", "48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201", "PASS: persistent typed generation example"),
):
    model = models / filename
    assert hashlib.sha256(model.read_bytes()).hexdigest() == digest, filename
    executable = programs / name
    identities[name] = {"program": str(executable), "sha256": hashlib.sha256(executable.read_bytes()).hexdigest(),
                        "model": str(model), "model_sha256": digest}
    result = run(name + "-offline", deny + [executable, "auto", model, digest])
    assert result.returncode == 0 and marker in result.stdout, result.stdout[-3000:]
    assert not any(s in result.stdout for s in ("FAIL:", "ERROR:", "PANIC:")), result.stdout[-3000:]
    print("PASS:", name, "under enforced child-process network denial", flush=True)
(output / "identities.json").write_text(json.dumps(identities, indent=2) + "\n")
print("PASS: live-network positive controls, enforced denial and both offline native examples", flush=True)
