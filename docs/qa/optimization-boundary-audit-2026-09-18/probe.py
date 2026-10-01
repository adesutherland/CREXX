"""Bounded architecture probes; deliberately inconsistent library is not valid output.

Run: python3 docs/qa/optimization-boundary-audit-2026-09-18/probe.py
Uses the existing Debug tools and creates an isolated temporary workspace.
No repository implementation files or installed tools are changed.
Reports the discovered entry-alias defect separately from passing controls.
"""
import json
import pathlib
import re
import subprocess
import tempfile

REPO = pathlib.Path(__file__).resolve().parents[3]
BIN = REPO / "cmake-build-debug/bin"
WORK = pathlib.Path(tempfile.mkdtemp(prefix="crexx-optimization-boundary-probe-"))
RECORDS = []


def run(tool, args, cwd):
    command = [str(BIN / tool), *map(str, args)]
    result = subprocess.run(command, cwd=cwd, capture_output=True, text=True,
                            timeout=120)
    RECORDS.append(dict(command=command, cwd=str(cwd), exit=result.returncode,
                        stdout=result.stdout, stderr=result.stderr))
    (WORK / "commands.json").write_text(json.dumps(RECORDS, indent=2) + "\n")
    if result.returncode:
        raise RuntimeError(f"{command}: {result.returncode}: {result.stderr}")
    return result.stdout


print(f"Evidence workspace: {WORK}", flush=True)
(WORK / "auditlib.crexx").write_text(
    "options levelb\nnamespace auditlib expose answer\n\n"
    "answer: procedure = .int\n  return 42\n")
run("rxc", ["-i", BIN, "-o", "auditlib", "auditlib.crexx"], WORK)
original = (WORK / "auditlib.rxas").read_text()
assert original.count("   ret 42\n") == 1
assert '.inline" "I6;' in original
stale = original.replace("   ret 42\n", "   ret 99\n")
without_inline = "\n".join(line for line in stale.splitlines()
                             if '=".inline"' not in line) + "\n"
main = ("options levelb\nimport auditlib\n\nmain: procedure = .int\n"
        "  say answer()\n  return 0\n")
summary = {}
for name, source, expected in [
        ("consistent", original, ["42", "42"]),
        ("stale_template", stale, ["42", "99"]),
        ("template_removed", without_inline, ["99", "99"])]:
    case = WORK / name
    case.mkdir()
    (case / "auditlib.rxas").write_text(source)
    run("rxas", ["auditlib"], case)
    # Keep the exact input as evidence while excluding it from import lookup.
    (case / "auditlib.rxas").rename(case / "auditlib-input.txt")
    disassembly = run("rxdas", ["auditlib.rxbin"], case)
    (case / "auditlib-disassembly.txt").write_text(disassembly)
    (case / "main.crexx").write_text(main)
    outputs = []
    for mode, flags in [("opt", []), ("noopt", ["-n"])]:
        run("rxc", [*flags, "-i", case, "-i", BIN, "-o", mode,
                    "main.crexx"], case)
        run("rxas", [mode], case)
        run("rxlink", ["-o", f"{mode}-linked", "auditlib.rxbin",
                       f"{mode}.rxbin"], case)
        output = run("rxvm", [f"{mode}-linked.rxbin"], case).strip()
        outputs.append(output)
        linked = run("rxdas", [f"{mode}-linked.rxbin"], case)
        assert '=".inline"' not in linked
        (case / f"{mode}-linked-disassembly.txt").write_text(linked)
    assert outputs == expected, (name, outputs, expected)
    run("rxlink", ["-i", "-o", "preserved", "auditlib.rxbin"], case)
    preserved = run("rxdas", ["preserved.rxbin"], case)
    assert ('=".inline"' in preserved) == (name != "template_removed")
    summary[name] = dict(optimized=outputs[0], unoptimized=outputs[1])

hand = WORK / "handwritten"
hand.mkdir()
(hand / "input.rxas").write_text(
    (REPO / "tests/rxas_optimizer/redundant_itos_runtime.rxas").read_text())
counts = {}
for mode, flags in [("opt", []), ("noopt", ["-n"])]:
    run("rxas", [*flags, "-o", f"{mode}.rxbin", "input"], hand)
    output = run("rxvm", [f"{mode}.rxbin"], hand).strip()
    assert output == "PASS: PERF3-10 redundant ITOS runtime", output
    disassembly = run("rxdas", [f"{mode}.rxbin"], hand)
    (hand / f"{mode}-disassembly.txt").write_text(disassembly)
    counts[mode] = len(re.findall(r"^\s+itos\s", disassembly, re.MULTILINE))
assert counts["opt"] < counts["noopt"], counts
summary["handwritten"] = dict(outputs="both PASS", itos_counts=counts)

# Neither globals nor two argument registers may be assumed constant/no-alias.
mutable = WORK / "mutable_boundary"
mutable.mkdir()
(mutable / "input.rxas").write_text(""".globals=1
main() .locals=5
    numsci 18,1,1
    load g0,41
    itos g0
    call mutate_global()
    itos g0
    say g0
    load r0,2
    load r1,41
    link r2,r1
    call r3,aliased_args(),r0
    unlink r2
    say r3
    ret 0
mutate_global() .locals=0
    inc g0
    ret
aliased_args() .locals=0
    itos a1
    inc a2
    itos a1
    ret a1
""")
mutable_outputs = {}
for mode, flags in [("opt", []), ("noopt", ["-n"])]:
    run("rxas", [*flags, "-o", f"{mode}.rxbin", "input"], mutable)
    output = run("rxvm", [f"{mode}.rxbin"], mutable).strip()
    mutable_outputs[mode] = output
    (mutable / f"{mode}-disassembly.txt").write_text(
        run("rxdas", [f"{mode}.rxbin"], mutable))
assert mutable_outputs["noopt"] == "42\n42", mutable_outputs
run("rxas", ["-d", "-o", "debug.rxbin", "input"], mutable)
summary["mutable_boundary"] = dict(
    expected="42\n42", outputs=mutable_outputs,
    optimization_preserves_semantics=mutable_outputs["opt"] == "42\n42")
(WORK / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
print(json.dumps(summary, indent=2))
