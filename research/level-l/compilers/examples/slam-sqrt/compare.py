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
import shutil
import subprocess
import tempfile


HERE = Path(__file__).resolve().parent
ROUTINES = (
    ("old", "fdsqrt-old.rexx", "FDSqrt"),
    ("new", "fdsqrt-new.rexx", "FDSqrt"),
    ("vision", "vision-sqrt.rexx", "rx_vision_sqrt"),
)
VARIATIONS = (
    ("renamed", "fdsqrt-renamed.rexx", "FDSqrt"),
    ("one-step", "fdsqrt-one-step.rexx", "FDSqrt"),
    ("no-exit", "fdsqrt-no-exit.rexx", "FDSqrt"),
    ("zero-divisor", "fdsqrt-zero-divisor.rexx", "FDSqrt"),
    ("subtract", "fdsqrt-subtract.rexx", "FDSqrt"),
)
ANALYSIS_ONLY = {"no-exit", "zero-divisor", "subtract"}
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
    if not lines or not lines[0].startswith("ROUTINE|"):
        raise RuntimeError(f"missing checked IR graph report: {lines!r}")
    allowed = ("ROUTINE|", "NODE|", "SHAPE|", "FLOW|", "IR|", "CFG|", "LIVENESS|",
               "PROOF|", "OBLIGATION|", "LIMIT|")
    if any(not line.startswith(allowed) for line in lines):
        raise RuntimeError(f"unrecognised source analysis: {lines!r}")
    if not any(line.startswith("SHAPE|") for line in lines) or not any(
            line.startswith("PROOF|") for line in lines):
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
             "llcfrontend", "llcgraph", "llceffects")
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


def check_analysis(structures: dict[str, list[str]], controls: list[str]) -> None:
    """Independent expectations from the authored control cases, not IR IDs."""
    def require(condition: bool, message: str) -> None:
        if not condition:
            raise AssertionError(message)

    require(any("direct exit when next>=guess" in row for row in structures["new"]),
            "revised source's exit condition was lost")
    require(any("this exit bypasses: guess=next" in row for row in structures["new"]),
            "LEAVE path must bypass the update")
    require(any("no direct conditional LEAVE" in row for row in structures["no-exit"]),
            "missing-exit control was not identified")
    require(any("nonzero divisor 2|proved|" in row for row in structures["new"]),
            "literal two must satisfy the bounded tree rule")
    require(any("nonzero divisor guess|unknown|" in row for row in structures["new"]),
            "tree rule must not invent a variable invariant")
    require(any("nonzero divisor 0|disproved|" in row for row in structures["zero-divisor"]),
            "literal zero must fail the nonzero obligation")
    for label, rows in structures.items():
        require(any(row == "PROOF|Classic division result available|unknown|completion.edge.unknown"
                    for row in rows), f"unexpected full Classic proof disposition: {label}")
    require(controls == [
        "CONTROL|unrelated-write|proved|state.preserved.in.acyclic.interval",
        "CONTROL|estimate-write|unknown|state.or.observation.changed",
        "CONTROL|missing-premise|unknown|effect.unknown",
    ], "proof controls lost their independent proved/unknown distinctions")


def negative_analysis_checks(vm: Path, image: Path, work: Path, artifacts: Path) -> None:
    """Grammar failure and unsupported semantics must not yield a shape."""
    for label, text, diagnostic in (
        ("malformed", "FDSqrt: procedure\ndo forever\n", "ERROR|frontend|"),
        ("unsupported", "FDSqrt: procedure expose x\nreturn x\n", "EXPOSE is outside"),
        ("missing", "say 'FDSqrt: is only text'\n", "missing top-level routine label"),
    ):
        source = work / f"negative-{label}.rexx"
        source.write_text(text)
        completed = subprocess.run([str(vm), str(image), "-a", str(source),
                                    "FDSqrt", str(artifacts)],
                                   capture_output=True, text=True, timeout=30)
        evidence = completed.stdout + completed.stderr
        if completed.returncode == 0 or diagnostic not in evidence or "SHAPE|" in evidence:
            raise AssertionError(f"failed negative analysis control {label}: {evidence}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", type=Path, default=Path("cmake-build-debug"))
    parser.add_argument("--values", type=Path, default=HERE / "values.txt")
    parser.add_argument("--check", action="store_true",
                        help="check the published 29-value contrast")
    parser.add_argument("--analysis-only", action="store_true",
                        help="show cREXX tree/graph/proof findings without numerical execution")
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
        controls = run([vm, image, "-a", "--proof-controls"]).splitlines()
        if args.check:
            negative_analysis_checks(vm, image, work, build / "bin")
        imports = prepare_runtime(build, work) if not args.analysis_only else None
        for label, file, routine in (*ROUTINES, *VARIATIONS):
            source = work / f"{label}.rexx"
            source.write_text(program((HERE / file).read_text(), routine))
            # Analyse the authored fixture directly: its reported lines are
            # useful to readers and do not include the execution wrapper.
            structures[label] = analyse(vm, image, HERE / file, routine, build / "bin")
            if not args.analysis_only and label not in ANALYSIS_ONLY:
                output[label] = results(build, work, imports, label, source, values)
        domain = {}
        if not args.analysis_only:
            for label, file, routine in ROUTINES[:2]:
                # Reuse each compiled program; these concrete domain probes
                # are observations, outside the sqrt certificate's domain.
                domain[label] = run([vm, "--no-autoload", work / f"{label}-linked.rxbin",
                                     "-a", "-1 0"]).splitlines()

    print("# SLAM square-root comparison (Classic Rexx / Level L)")
    print(f"Inputs: {len(values)}; NUMERIC DIGITS 9; VM: {vm.name}")
    print("Numerical certificates use nonnegative inputs. Separate domain probes retain routine behaviour.")
    print("\n## Source and graph facts")
    originals = {item[0] for item in ROUTINES}
    for label, _, _ in (*ROUTINES, *VARIATIONS):
        print(f"\n### {label}")
        for row in structures[label]:
            if row.startswith("SHAPE|"):
                continue
            if row.startswith("NODE|"):
                _, depth, line, kind, text = row.split("|", 4)
                if label not in originals and kind != "CDo":
                    continue
                print(f"{'  ' * int(depth)}- line {line}: {kind}: {text}")
            elif row.startswith("FLOW|"):
                _, line, text = row.split("|", 2)
                print(f"- Source flow interpretation, line {line}: {text}")
            elif row.startswith("OBLIGATION|"):
                _, line, claim, status, basis = row.split("|", 4)
                print(f"- Tree rule, line {line}: {claim}: **{status}** ({basis}).")
            elif row.startswith("PROOF|"):
                _, claim, status, reason = row.split("|", 3)
                print(f"- Checked effect query: {claim}: **{status}** (`{reason}`).")
            elif row.startswith("LIMIT|"):
                continue
            else:
                name, text = row.split("|", 1)
                print(f"- {name}: {text}")
    shape = lambda label: next(row for row in structures[label] if row.startswith("SHAPE|"))
    print("\n## Controlled AST comparisons (computed by cREXX)")
    comparisons = (("new", "renamed", True), ("old", "one-step", False),
                   ("new", "no-exit", False), ("old", "zero-divisor", False),
                   ("new", "subtract", False))
    for left, right, expected in comparisons:
        same = shape(left) == shape(right)
        print(f"{left} / {right}: normalized shape {'same' if same else 'different'}")
        if args.check and same != expected:
            raise AssertionError(f"unexpected shape comparison: {left}/{right}")
    print("no-exit, zero-divisor and subtract: ANALYSIS ONLY; never executed. Shape equality is not semantic equivalence.")
    print("\n## Bounded proof API controls (separate illustrative IR)")
    print("| Step premise | Checked outcome | Reason |")
    print("|---|---|---|")
    for row in controls:
        _, premise, status, reason = row.split("|", 3)
        print(f"| {premise} | {status} | `{reason}` |")
    print("These proofs check explicit effect premises in a tiny acyclic IR, not Newton arithmetic.")
    print("Numerical variable invariants, convergence and all-input termination remain unknown.")
    if args.check:
        check_analysis(structures, controls)
        print("Tree/graph/proof controls: PASS")
    if args.analysis_only:
        return 0

    print("\n## Domain observations (not sqrt certificates)")
    for label, rows in domain.items():
        print(f"{label}: " + ", ".join(rows))
        if args.check and rows != ["-1|0", "0|0"]:
            raise AssertionError(f"unexpected domain behaviour: {label}")
    print("Vision negative-input exception was not ported: no negative Vision execution or equivalence claim.")

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
    renamed_same = output["renamed"] == output["new"]
    one_step_bad = sum(not within(rational(x), rational(r))
                       for x, r in zip(values, output["one-step"]))
    print(f"Renamed outputs equal revised on this set: {renamed_same}; one-step failures: {one_step_bad}/{len(values)}.")
    if args.check:
        if len(values) != 29 or old_bad != 12 or new_bad != 0 or \
           set(not_nearest) != {"0.5", "99.5", "2000000000"} or \
           set(agreement_off) != set(not_nearest):
            raise AssertionError("Classic results differ from the published SLAM contrast")
        if not renamed_same or one_step_bad != 28:
            raise AssertionError("controlled numerical variation differed from its independent expectation")
        print("Published 29-value contrast: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
