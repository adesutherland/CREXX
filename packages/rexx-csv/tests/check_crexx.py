#!/usr/bin/env python3
"""Run the shared oracle and cREXX ownership/limit tests with an explicit SDK.

JSON belongs to this test adapter, never the installed library. Generated Rexx,
RXAS, bytecode and logs live in a disposable directory, including a spaced path.
"""
import argparse
import json
from pathlib import Path
import re
import shutil
import subprocess
import os
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def literal(value):
    """Exact UTF-8 including NUL/newlines; no shell or source-quote escaping."""
    return f"(('{value.encode('utf-8').hex()}'x as .binary) as .string)"


def common_source(cases):
    lines = ["options levelb", "import rxfnsb", "import rexx_csv",
             "values = .string[]", "rows = .csvrow[]",
             'empty = csvparse("")', "emptyrows = empty.rows()"]

    def integer(label, actual, expected):
        lines.append(f"call check_int({literal(label)}, {actual}, {expected})")

    def string(label, actual, expected):
        lines.append(f"call check_text({literal(label)}, {actual}, {literal(expected)})")

    def table(label, obj, expected):
        integer(label + " row count", obj + ".row_count()", len(expected))
        for r, row in enumerate(expected, 1):
            integer(f"{label} row {r} width", f"{obj}.field_count({r})", len(row))
            for c, value in enumerate(row, 1):
                string(f"{label} ({r},{c})", f"{obj}.field({r},{c})", value)

    for case in cases:
        identifier = case["id"]
        expected = case["expected"]
        if case["operation"] == "parse":
            lines.append(f"parsed = csvparse({literal(case['input'])})")
            obj = "parsed"
        else:
            lines.append("rows = emptyrows")
            for r, row in enumerate(case["rows"], 1):
                lines.append("call arraydrop values")
                for c, value in enumerate(row, 1):
                    lines.append(f"values[{c}] = {literal(value)}")
                lines.append(f"rows[{r}] = .csvrow(values)")
            lines.append("written = csvwrite(rows)")
            obj = "written"
        integer(identifier + " status", obj + ".ok()", int(expected["ok"]))
        if expected["ok"]:
            for name in ("error_code", "error_limit", "error"):
                string(identifier + " " + name, obj + "." + name + "()", "")
            integer(identifier + " diagnostic record", obj + ".error_record()", 0)
            integer(identifier + " diagnostic field", obj + ".error_field()", 0)
            if case["operation"] == "parse":
                table(identifier, "parsed", expected["rows"])
                lines += ["rows = parsed.rows()", "written = .csvoutput(rows)",
                          "again = .csvdocument(written.text())"]
                integer(identifier + " writer status", "written.ok()", 1)
                integer(identifier + " round trip status", "again.ok()", 1)
                table(identifier + " round trip", "again", expected["rows"])
            else:
                string(identifier + " canonical bytes", "written.text()", expected["text"])
                lines.append("again = csvparse(written.text())")
                integer(identifier + " round trip status", "again.ok()", 1)
                table(identifier + " round trip", "again", case["rows"])
        else:
            error = expected["error"]
            string(identifier + " code", obj + ".error_code()", error["code"])
            string(identifier + " limit", obj + ".error_limit()", error.get("limit", ""))
            integer(identifier + " record", obj + ".error_record()", error["record"])
            integer(identifier + " field", obj + ".error_field()", error["field"])
            integer(identifier + " message", f"length({obj}.error()) > 0", 1)
            if case["operation"] == "parse":
                integer(identifier + " atomic rows", "parsed.row_count()", 0)
                integer(identifier + " invalid count", "parsed.field_count(1)", -1)
                lines += ["rows = parsed.rows()"]
                integer(identifier + " atomic row array", "rows[0]", 0)
            else:
                string(identifier + " atomic output", "written.text()", "")
    lines += [f'say "PASS: {len(cases)} common fixtures and valid round trips"',
              "exit 0", HELPERS]
    return "\n".join(lines) + "\n"


HELPERS = '''
check_int: procedure = .void
  arg label = .string, actual = .int, expected = .int
  if actual \\= expected then do
    say "FAIL" label "expected" expected "got" actual
    exit 1
  end
  return
check_text: procedure = .void
  arg label = .string, actual = .string, expected = .string
  if actual \\== expected then do
    say "FAIL" label "text bytes differ"
    exit 1
  end
  return
'''


def run(command, directory, label, timeout=300):
    log = directory / "command.log"
    with log.open("wb") as out:
        result = subprocess.run(list(map(str, command)), cwd=directory,
                                stdout=out, stderr=subprocess.STDOUT, timeout=timeout)
    output = log.read_text(encoding="utf-8", errors="replace")
    if result.returncode or "Warning" in output or "FAIL" in output:
        raise RuntimeError(f"{label}: exit {result.returncode}\n" + "\n".join(output.splitlines()[:25]))
    return output


def inspect_shape(path):
    """Structural expectations, independent of temporary register numbers."""
    text = path.read_text(encoding="utf-8")

    def block(name):
        match = re.search(r"^" + re.escape(name) + r" \s*\.locals=.*?(?=^\S+\(\) \.locals=|\Z)",
                          text, re.MULTILINE | re.DOTALL)
        assert match, name
        return "\n".join(line for line in match.group().splitlines()[1:]
                         if not line.strip().startswith((".meta", ".src", ".trace")))

    parse = block("_csv_parse()")
    span = block("_csv_append_span()")
    field = block("§rexx_csv.csvdocument.field()")
    assert "bgetu8" in parse and "_csv_append_span()" in parse
    assert "bcopy" in span and "bappend a1," in span
    assert "linkattr1" in field and "minlinkattr1" in field
    assert not re.search(r"\bcopy\s+[^,\n]+,a1\b", field), "field access copied the table receiver"
    assert str(ROOT) not in text and str(path.parent) not in text, "host path leaked into emitted source"


def check(binary, modes, vms, work_root=None):
    binary = binary.resolve()
    def tool(name):
        return binary / (name + (".exe" if os.name == "nt" else ""))
    manifest = json.loads((ROOT / "tests/fixtures/cases.json").read_bytes().decode("utf-8"))
    assert manifest["schema_version"] == 1 and manifest["profile"] == "rexx-csv-0.1"
    cases = manifest["cases"]
    assert len(cases) == len({case["id"] for case in cases}), "duplicate fixture IDs"
    with tempfile.TemporaryDirectory(prefix="rexx-csv checks ", dir=work_root) as temp:
        print(run([tool("rxc"), "-v"], Path(temp), "version").strip(), flush=True)
        for mode in modes:
            directory = Path(temp) / mode
            directory.mkdir()
            shutil.copyfile(ROOT / "crexx/rexx_csv.crexx", directory / "rexx_csv.crexx")
            (directory / "common.crexx").write_text(common_source(cases), encoding="utf-8")
            shutil.copyfile(ROOT / "crexx/tests/contracts.crexx", directory / "contracts.crexx")
            shutil.copyfile(ROOT / "examples/crexx_demo.crexx", directory / "crexx_demo.crexx")
            flags = ["-n"] if mode == "noopt" else []
            for module in ("rexx_csv", "common", "contracts", "crexx_demo"):
                run([tool("rxc"), *flags, "-i", directory, "-o", module, module + ".crexx"],
                    directory, mode + " compile " + module)
                if module == "rexx_csv" and mode == "opt":
                    inspect_shape(directory / "rexx_csv.rxas")
                run([tool("rxas"), *flags, module], directory, mode + " assemble " + module)
                if module == "rexx_csv":
                    continue
                run([tool("rxlink"), "-o", module + "-linked", module, "rexx_csv",
                     binary / "library.rxbin"], directory, mode + " link " + module)
                for vm in vms:
                    output = run([tool(vm), "--no-autoload", module + "-linked.rxbin"],
                                 directory, f"{mode}/{vm}/{module}", timeout=600)
                    print(f"{mode}/{vm}: {output.strip()}", flush=True)
            # Keep readable output only during active inspection, on explicit request.
            if work_root:
                target = work_root / ("inspect-" + binary.parent.name + "-" + mode)
                target.mkdir(exist_ok=True)
                for module in ("rexx_csv", "crexx_demo"):
                    shutil.copyfile(directory / (module + ".rxas"), target / (module + ".rxas"))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bin", type=Path, required=True, help="complete explicit core bin directory")
    parser.add_argument("--modes", nargs="+", choices=("opt", "noopt"), default=["opt", "noopt"])
    parser.add_argument("--vms", nargs="+", default=["rxtvm", "rxbvm"])
    parser.add_argument("--inspect-dir", type=Path, help="temporary existing QA directory for RXAS inspection")
    args = parser.parse_args()
    check(args.bin, args.modes, args.vms, args.inspect_dir)
