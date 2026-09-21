#!/usr/bin/env python3
"""Host interface proof; TEST-XOR is a mock, not IBM1047/CMS qualification."""
import argparse
import os
from pathlib import Path
import subprocess

p = argparse.ArgumentParser()
for name in ("rxc", "rxas", "mock-rxc", "mock-rxas", "link", "vm", "work"):
    p.add_argument("--" + name, required=True)
a = p.parse_args()
work = Path(a.work).resolve()
work.mkdir(parents=True, exist_ok=True)
src = work / "source"
src.mkdir(exist_ok=True)
log = work / "commands.log"
log.write_text("")
opens = work / "opens.log"
opens.write_text("")
count = 0

def run(args, *, failure=None, match=None, skip=0, expected=0, cwd=src):
    global count
    env = dict(os.environ, CREXX_TEST_TEXT_LOG=str(opens))
    env.pop("CREXX_TEST_TEXT_FAILURE", None)
    env.pop("CREXX_TEST_TEXT_MATCH", None)
    env["CREXX_TEST_TEXT_SKIP"] = str(skip)
    if failure:
        env["CREXX_TEST_TEXT_FAILURE"] = failure
    if match:
        env["CREXX_TEST_TEXT_MATCH"] = match
    result = subprocess.run(list(map(str, args)), cwd=cwd, env=env,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            timeout=120)
    with log.open("ab") as f:
        f.write((repr(args) + f" failure={failure} match={match} skip={skip} rc={result.returncode}\n").encode())
        f.write(result.stdout + result.stderr)
    if expected == "error":
        assert result.returncode > 0, (args, result.returncode, result.stderr)
    else:
        assert result.returncode == expected, (args, result.returncode, result.stderr)
    count += 1
    return result.stdout + result.stderr

def transform(data):
    return bytes(b ^ 0x80 for b in data)

sources = {
    "main.crexx": "options levelb\nimport cms_text_provider\nsay greeting()\nreturn 0\n",
    "provider.crexx": ("options levelb\nnamespace cms_text_provider expose greeting\n"
                       "/* " + "a long comment; [] ^ " * 200 + " */\n"
                       "greeting: procedure = .string\nreturn 'café ^ []'\n"),
}

def write_sources(encoded=False):
    for name, body in sources.items():
        data = body.encode("utf-8")
        (src / name).write_bytes(transform(data) if encoded else data)

for mode in ([], ["-n"]):
    products = []
    for label, compiler, assembler, encoding in (
        ("default", a.rxc, a.rxas, []),
        ("utf8", a.rxc, a.rxas, ["-E", "UTF8"]),
        ("adapter-utf8", a.mock_rxc, a.mock_rxas, ["-E", "UTF8"]),
        ("converted", a.mock_rxc, a.mock_rxas, ["-E", "TEST-XOR"]),
    ):
        out = work / (("noopt-" if mode else "opt-") + label)
        out.mkdir(exist_ok=True)
        for old in out.glob("*.rxbin"):
            old.unlink()
        stem = out / "program"
        write_sources(label == "converted")
        run([compiler, "--no-exe-import", "-x", *mode, *encoding, "-i", src,
             "-o", "program", src / "main.crexx"], cwd=out)
        assembly = stem.with_suffix(".rxas").read_bytes()
        run([assembler, *mode, *encoding, "-o", "program", "program.rxas"], cwd=out)
        binary = stem.with_suffix(".rxbin").read_bytes()
        run([compiler, "--no-exe-import", "-x", *mode, *encoding,
             "-o", "provider", src / "provider.crexx"], cwd=out)
        run([assembler, *mode, *encoding, "-o", "provider", "provider.rxas"], cwd=out)
        run([a.link, "-o", "linked", "program.rxbin", "provider.rxbin"], cwd=out)
        output = run([a.vm, "linked.rxbin"], cwd=out)
        assert "café ^ []".encode() in output
        products.append((transform(assembly) if label == "converted" else assembly,
                         binary, (out / "provider.rxbin").read_bytes(),
                         (out / "linked.rxbin").read_bytes(), output))
    assert all(product == products[0] for product in products), "encoding changed compilation products"

write_sources()
stem = work / "failure"
for compiler in (a.rxc, a.mock_rxc):
    for option in ([], ["unknown"]):
        run([compiler, "-E", *option], expected=2)
for assembler in (a.rxas, a.mock_rxas):
    for option in ([], ["unknown"]):
        run([assembler, "-E", *option], expected=2)
for tool in (a.rxc, a.rxas):
    run([tool, "-E", "IBM1047"], expected=2)

for failure in ("read", "close-read"):
    for name in ("main.crexx", "provider.crexx"):
        diagnostic = run([a.mock_rxc, "--no-exe-import", "-x", "-o", stem, src / "main.crexx"],
                         failure=failure, match=name, expected="error")
        assert b"read input" in diagnostic, diagnostic
    # The first provider open scans its header; the next parses its procedures.
    diagnostic = run([a.mock_rxc, "--no-exe-import", "-x", "-o", stem, src / "main.crexx"],
                     failure=failure, match="provider.crexx", skip=1, expected="error")
    assert b"Importing Procedures - Can't read input" in diagnostic, diagnostic
    diagnostic = run([a.mock_rxas, "-o", stem, work / "opt-default/program.rxas"],
                     failure=failure, match="program.rxas", expected="error")
    assert b"read input" in diagnostic, diagnostic
for failure in ("write", "close-write", "convert-write"):
    diagnostic = run([a.mock_rxc, "--no-exe-import", "-x", "-d2", "-o", stem, src / "main.crexx"],
                     failure=failure, match="failure.rxas", expected=2)
    assert b"complete assembly output" in diagnostic, diagnostic
    assert b"Compiler Exiting - Success" not in diagnostic, diagnostic

# Binary RXBIN output must bypass the text hook entirely, even for encoded input.
assert ".rxbin" not in opens.read_text(), "binary file reached text adapter"
print(f"PASS: {count} text/import/assembly/execution and failure checks; opt/noopt products identical")
