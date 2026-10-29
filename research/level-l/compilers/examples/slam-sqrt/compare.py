#!/usr/bin/env python3
"""Run three Classic Rexx sqrt bodies and check each result with exact rationals.

The numerical certificates are about stated inputs and output strings. They
are independent of the Rexx routines, but say nothing about all inputs or the
correctness of the compiler/VM that produced the strings.
"""

import argparse
from decimal import Decimal, InvalidOperation
from fractions import Fraction
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


HERE = Path(__file__).resolve().parent
ROUTINES = (
    ("old", "fdsqrt-old.rexx", "FDSqrt"),
    ("new", "fdsqrt-new.rexx", "FDSqrt"),
    ("vision", "vision-sqrt.rexx", "rx_vision_sqrt"),
)
TOLERANCE = Fraction(1, 100_000_000)


def rational(text: str) -> Fraction:
    try:
        decimal = Decimal(text)
    except InvalidOperation as error:
        raise ValueError(f"invalid decimal: {text!r}") from error
    if not decimal.is_finite():
        raise ValueError(f"nonfinite decimal: {text!r}")
    return Fraction(decimal)


def read_values(path: Path) -> list[str]:
    values = [line.strip() for line in path.read_text().splitlines()
              if line.strip() and not line.lstrip().startswith("#")]
    if not values:
        raise ValueError("empty value set")
    for value in values:
        if rational(value) < 0:
            raise ValueError(f"negative input is outside the comparison domain: {value}")
    return values


def within(x: Fraction, r: Fraction) -> bool:
    if x < 0 or r < 0:
        return False
    margin = r * TOLERANCE
    low = max(Fraction(0), r - margin)
    return low * low <= x <= (r + margin) * (r + margin)


def nearest_nine(x: Fraction, output: str) -> bool:
    r = rational(output)
    if x < 0 or r < 0:
        return False
    if r == 0:
        return x == 0
    exponent = Decimal(output).adjusted()
    unit = Fraction(10) ** (exponent - 8)
    digit = r / unit
    if digit.denominator != 1 or not 100_000_000 <= digit <= 999_999_999:
        return False
    # The predecessor is closer at a power-of-ten boundary: 9.99999999 -> 10.
    previous_unit = unit / 10 if digit == 100_000_000 else unit
    lower = r - previous_unit / 2
    upper = r + unit / 2
    return lower * lower <= x <= upper * upper


def certificate_controls() -> None:
    controls = (
        (within(Fraction(2), rational("1.41421356")), "true tolerance"),
        (not within(Fraction(2), rational("1.4142136")), "tampered tolerance"),
        (nearest_nine(Fraction(10), "3.16227766"), "true rounding"),
        (not nearest_nine(Fraction(10), "3.16227767"), "tampered rounding"),
        (not nearest_nine(rational("9.99999997") ** 2, "10"),
         "lower decade boundary"),
        (nearest_nine(rational("9.999999997") ** 2, "10"),
         "upper decade boundary"),
        (nearest_nine(Fraction(0), "0"), "zero"),
        (not nearest_nine(Fraction(2), "0"), "false zero"),
    )
    for passed, label in controls:
        if not passed:
            raise AssertionError(f"certificate control failed: {label}")
    for invalid in ("NaN", "Infinity", "not-a-number"):
        try:
            rational(invalid)
        except ValueError:
            pass
        else:
            raise AssertionError(f"invalid certificate operand accepted: {invalid}")


def run(command: list[str | Path]) -> str:
    completed = subprocess.run([str(part) for part in command],
                               capture_output=True, text=True, timeout=120)
    if completed.returncode != 0:
        raise RuntimeError(f"{Path(command[0]).name} exited {completed.returncode}:\n"
                           f"{completed.stdout}{completed.stderr}")
    if completed.stderr:
        raise RuntimeError(f"{Path(command[0]).name} wrote diagnostics:\n{completed.stderr}")
    return completed.stdout


def program(body: str, routine: str) -> str:
    return ("numeric digits 9\n"
            "parse arg values\n"
            "do i=1 to words(values)\n"
            "  x=word(values,i)\n"
            f"  say x || '|' || {routine}(x)\n"
            "end\n"
            "exit 0\n\n" + body)


def analyse(vm: Path, image: Path, source: Path, routine: str,
            artifacts: Path) -> list[str]:
    lines = run([str(vm), str(image), "-a", str(source), routine,
                 str(artifacts)]).splitlines()
    if not lines or not lines[0].startswith("IR|"):
        raise RuntimeError(f"missing checked IR graph report: {lines!r}")
    if any(not line.startswith("AST|") for line in lines[1:]):
        raise RuntimeError(f"unrecognised source analysis: {lines!r}")
    if not lines[1:]:
        raise RuntimeError("no source-linked routine loop")
    return lines


def prepare_analyser(build: Path, work: Path) -> Path:
    base = work / "analyse"
    imports = build / "research/level-l/compilers/levelc/opt/imports-llccompiler"
    run([build / "bin/rxc", "-x", "--no-exe-import", "-i", imports,
         "-o", base, HERE / "analyse.crexx"])
    run([build / "bin/rxas", base])
    linked = work / "analyse-linked"
    common = build / "research/level-l/compilers/common/opt"
    classic = build / "research/level-l/compilers/levelc/opt"
    modules = [common / "llscalar.rxbin", common / "llcompilercontracts.rxbin",
               build / "langtools/common/llrxastext.rxbin",
               build / "langtools/common/llbinmeta.rxbin",
               build / "research/level-l/compilers/frontends/llcgrammar.rxbin",
               build / "lib/rxfnsc/members/RexxClassicUnicodeProperties/RexxClassicUnicodeProperties.rxbin"]
    names = ("llcir", "llclanguage", "llcsymbols", "llcbifs", "llcbindings",
             "llcsrcmap", "llcdiagnostics", "llcparseprogram",
             "llcprocedureprotocol", "llcsignalprotocol", "llcdoprotocol",
             "llcclauseprotocol", "llccommandprotocol", "llcparse",
             "llcfrontend", "llcgraph")
    modules.extend(classic / f"{name}.rxbin" for name in names)
    modules.extend(build / "bin" / f"{name}.rxbin" for name in
                   ("rxfnsl-analysis", "rxfnsl", "library"))
    run([build / "bin/rxlink", "-s", "-o", linked,
         base.with_suffix(".rxbin"), *modules])
    return linked.with_suffix(".rxbin")


def prepare_runtime(build: Path, work: Path) -> Path:
    imports = work / "imports"
    imports.mkdir()
    for name in ("library", "rxfnsc", "classlib", "rxunicode-core"):
        shutil.copy2(build / "bin" / f"{name}.rxbin", imports / f"{name}.rxbin")
    shutil.copy2(build / "research/level-l/compilers/backend/opt/llctarget.rxbin",
                 imports / "llctarget.rxbin")
    return imports


def results(build: Path, work: Path, imports: Path, label: str,
            source: Path, values: list[str]) -> list[str]:
    base = work / label
    run([build / "research/level-l/compilers/products/llrxc-c",
         "-i", imports, "-o", base.with_suffix(".rxas"), source])
    run([build / "bin/rxas", base.with_suffix(".rxas")])
    linked = work / f"{label}-linked"
    run([build / "bin/rxlink", "-o", linked, base.with_suffix(".rxbin"),
         *[imports / f"{name}.rxbin" for name in
           ("library", "llctarget", "rxfnsc", "classlib", "rxunicode-core")]])
    lines = run([build / "bin/rxbvm", "--no-autoload",
                 linked.with_suffix(".rxbin"), "-a", " ".join(values)]).splitlines()
    if len(lines) != len(values):
        raise RuntimeError(f"expected {len(values)} rows, got {len(lines)}: {lines!r}")
    outputs = []
    for expected, line in zip(values, lines):
        if "|" not in line:
            raise RuntimeError(f"invalid result record: {line!r}")
        actual, output = line.split("|", 1)
        if actual != expected or output == "":
            raise RuntimeError(f"result input mismatch: {line!r} for {expected!r}")
        rational(output)
        outputs.append(output)
    return outputs


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", type=Path, default=Path("cmake-build-debug"))
    parser.add_argument("--values", type=Path, default=HERE / "values.txt")
    parser.add_argument("--check", action="store_true",
                        help="check the published 29-value contrast")
    args = parser.parse_args()
    build = args.build.resolve()
    values = read_values(args.values)
    if args.check and args.values.resolve() != (HERE / "values.txt").resolve():
        parser.error("--check requires the maintained 29-value set")
    certificate_controls()
    vm = build / "bin/rxbvm"
    required = (vm, build / "bin/rxc", build / "bin/rxas", build / "bin/rxlink",
                build / "research/level-l/compilers/products/llrxc-c",
                build / "research/level-l/compilers/backend/opt/llctarget.rxbin",
                build / "research/level-l/compilers/levelc/opt/imports-llccompiler")
    for executable in required:
        if not executable.exists():
            parser.error(f"missing prepared product: {executable}")

    output: dict[str, list[str]] = {}
    structures: dict[str, list[str]] = {}
    with tempfile.TemporaryDirectory(prefix="slam-sqrt-") as temporary:
        work = Path(temporary)
        image = prepare_analyser(build, work)
        imports = prepare_runtime(build, work)
        for label, file, routine in ROUTINES:
            source = work / f"{label}.rexx"
            source.write_text(program((HERE / file).read_text(), routine))
            structures[label] = analyse(vm, image, source, routine, build / "bin")
            output[label] = results(build, work, imports, label, source, values)

    print("# SLAM square-root comparison (Classic Rexx / Level L)")
    print(f"Inputs: {len(values)}; NUMERIC DIGITS 9; VM: {vm.name}")
    print("Domain: nonnegative finite decimal input; negative/nonnumeric input is rejected before execution.")
    print("\n## Source and graph facts")
    for label, _, _ in ROUTINES:
        print(f"{label}: {structures[label][0]}")
        for row in structures[label][1:]:
            match = re.fullmatch(r"AST\|form=([^|]+)\|first=(\d+)\|last=(\d+)", row)
            if not match:
                raise RuntimeError(f"malformed AST row: {row!r}")
            print(f"  {match[1]} at source bytes {match[2]}..{match[3]}")
    print("Graph cycles are CFG facts; they do not prove termination.")

    print("\n## Per-input evidence")
    print("| Input | Old | Revised | Vision | Revised=Vision | Old within 1e-8 | Revised within 1e-8 | Revised nearest 9 |")
    print("|---:|---:|---:|---:|:---:|:---:|:---:|:---:|")
    old_bad = 0; new_bad = 0; not_nearest = []; agreement_off = []
    for i, value in enumerate(values):
        x = rational(value)
        old = output["old"][i]; new = output["new"][i]; vision = output["vision"][i]
        old_ok = within(x, rational(old))
        new_ok = within(x, rational(new))
        near = nearest_nine(x, new)
        agree = rational(new) == rational(vision)
        old_bad += not old_ok; new_bad += not new_ok
        if not near:
            not_nearest.append(value)
            if agree:
                agreement_off.append(value)
        mark = lambda condition: "yes" if condition else "no"
        print(f"| {value} | {old} | {new} | {vision} | {mark(agree)} | "
              f"{mark(old_ok)} | {mark(new_ok)} | {mark(near)} |")
    print(f"\nOld disproved within tolerance: {old_bad}/{len(values)}; "
          f"revised disproved: {new_bad}/{len(values)}.")
    print("Revised results outside nearest-nine certificate: " +
          (", ".join(not_nearest) if not_nearest else "none"))
    print("Agreed with Vision but outside nearest-nine certificate: " +
          (", ".join(agreement_off) if agreement_off else "none"))
    print("All-input correctness and termination: UNKNOWN (no general proof attempted).")
    if args.check:
        if len(values) != 29 or old_bad != 12 or new_bad != 0 or \
           set(not_nearest) != {"0.5", "99.5", "2000000000"} or \
           set(agreement_off) != set(not_nearest):
            raise AssertionError("Classic results differ from the published SLAM contrast")
        print("Published 29-value contrast: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
