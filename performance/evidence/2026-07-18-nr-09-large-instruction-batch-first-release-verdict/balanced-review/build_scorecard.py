#!/usr/bin/env python3
"""Join NR-09 design, occurrence and exact Release evidence into one ledger."""

from pathlib import Path
import csv


HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
MANIFEST = ROOT / "performance/manifests/nr09-macro-review-v1.tsv"
TIMING_LEDGER = HERE.parent / "macro-timing-report/review-ledger.tsv"
OCCURRENCES = HERE / "portfolio-occurrences.tsv"
RELEASE = HERE / "release-cell-summary.tsv"
OUT = HERE / "balanced-scorecard.tsv"


REMOVE = {
    "NUMCTX_INT_INT_INT_INT_INT",
    "SETTPSWAPSETTP_REG_INT_REG_REG",
    "ILOADCOPY_REG_REG_INT",
    "ILOADN_REG_INT_REG_INT",
    "ILOADN_REG_REG_INT",
    "ITOF_REG_REG",
    "FMULTICOPY_REG_FLOAT_REG_REG",
    "FSUBILOAD_REG_REG_FLOAT_REG_INT",
    "ITOSCONCAT_REG_STRING_REG",
    "SCONCATITOS_REG_STRING_REG",
    "ILOADGETATTRS_REG_INT_REG_REG_INT",
    "IGETATTR1_REG_REG_INT",
    "MINIGETATTR1_REG_REG_INT",
    "ISETATTR1_REG_INT_INT",
    "RELINKATTR1_REG_REG_INT",
    "UNLINKRELINKATTR1_REG_REG_REG_INT",
    "LINKSETATTRS_REG_REG_INT_INT",
    "LINKSETATTRSADD_REG_REG_INT_INT_REG_REG_INT",
    "LINKSETATTRSLINKADD_REG_REG_INT_INT_REG_REG_INT",
    "SETATTRSADD_REG_INT_REG_REG_INT",
    "LINKILOAD_REG_REG_REG_REG_INT",
    "SETLINKILOAD_REG_REG_INT_REG_REG_INT",
    "UNLINKLINKATTR1_REG_REG_REG_INT",
    "LINKATTR1ADD_REG_REG_REG_INT",
    "ISETATTR1_REG_REG_INT",
    "STOIATTR1_REG_REG_INT",
    "MINSTOIATTR1_REG_REG_INT",
    "ILOADSETUNLINK_REG_REG_INT",
    "LINKILOADSETUNLINK_REG_REG_REG_REG_INT",
    "SETLINKATTR1_REG_REG_INT_INT",
}

REPLACE = {
    "FDIVSUB_REG_REG_REG_FLOAT",
    "ILOADSETUNLINKN_REG_REG_INT_REG",
}


def read_tsv(path):
    with path.open(newline="") as handle:
        return list(csv.DictReader(handle, delimiter="\t"))


manifest = read_tsv(MANIFEST)
occurrence = {row["opcode"]: row for row in read_tsv(OCCURRENCES)}
temporary = {row["opcode"]: row["temporary_register_policy"] for row in read_tsv(TIMING_LEDGER)}
release = {(row["opcode"], row["vm"]): row for row in read_tsv(RELEASE)}


def proposal(opcode):
    if opcode in REPLACE:
        return "replace-current-form"
    if opcode in REMOVE:
        return "remove"
    return "retain"


def coherence(row):
    shape = row["coherence_shape"]
    temp = temporary[row["opcode"]]
    if shape == "independent-bundle":
        return "weak-independent"
    if "temporary-register-passed" in temp:
        return "weak-caller-temporary"
    if row["review_bar"] == "coherence-review":
        return "mixed-retained-intermediate"
    if shape in {"homogeneous-bulk", "homogeneous-cleanup", "context-unit",
                 "call-setup", "call-setup-and-call", "direct-attribute-read",
                 "direct-attribute-write", "direct-attribute-conversion"}:
        return "strong"
    return "acceptable"


def use_band(occ):
    executions = int(occ["executions"])
    workloads = int(occ["dynamic_workload_breadth"])
    if executions >= 1_000_000 and workloads >= 2:
        return "high"
    if executions >= 100_000:
        return "medium"
    if executions > 0:
        return "low"
    if int(occ["program_sites"]) + int(occ["library_sites"]) > 0:
        return "static-only"
    return "none"


def performance(opcode):
    rows = [release.get((opcode, vm)) for vm in ("rxvm", "rxbvm")]
    if not any(rows):
        return "unmeasured-no-execution"
    classes = {row["classification"] for row in rows if row}
    if "measurable-slowdown" in classes:
        return "cross-vm-regression"
    if classes == {"measurable-saving"}:
        return "both-vms-saving"
    if "measurable-saving" in classes:
        return "positive-but-layout-sensitive"
    return "noise-or-layout-sensitive"


def future_calling(row):
    if row["family"] == "call-window-through-call":
        return "current-value-but-swap-demand-expected-to-fall"
    if row["family"] == "call-window-preparation" or row["family"] == "multi-swap":
        return "swap-demand-expected-to-fall"
    return "not-applicable"


def rationale(row, occ):
    opcode = row["opcode"]
    executions = int(occ["executions"])
    sites = int(occ["program_sites"]) + int(occ["library_sites"])
    if opcode == "FDIVSUB_REG_REG_REG_FLOAT":
        return "Double-digit dual-VM saving at 501k calls justifies the concept, but rxc should not preserve the quotient side effect; replace with clean result-only semantics."
    if opcode == "ILOADSETUNLINKN_REG_REG_INT_REG":
        return "Double-digit cell saving at 364k calls justifies redesign, but the explicit trace-only load register must disappear; route to a trace-correct compact form."
    if opcode == "ITOF_REG_REG":
        return "Coherent but measurably slower in rxvm and only oppositely positive in rxbvm; one million calls do not yield a cross-VM product win."
    if opcode == "ITOSCONCAT_REG_STRING_REG":
        return "Caller-visible conversion temporary buys only 0.8-1.2% in-cell and about 0.004-0.008 ms across the bounded portfolio."
    if opcode == "SCONCATITOS_REG_STRING_REG":
        return "Independent effects, only 36 executions, and no consistent dual-VM saving."
    if opcode in {"SETLINKATTR1_REG_REG_INT_INT", "UNLINKLINKATTR1_REG_REG_REG_INT", "MINSTOIATTR1_REG_REG_INT"}:
        return f"Coherent but only {executions} executions; projected portfolio saving is below 0.001 ms and does not justify a dedicated form."
    if opcode == "SETTPSWAPSETTP_REG_INT_REG_REG":
        return "No sites or executions, and the wider calling convention is expected to reduce the swap sequence that creates it."
    if opcode == "NUMCTX_INT_INT_INT_INT_INT":
        return "No sites or executions in program or library images; the separately accepted NUMSCI/NUMENG rule already covers the measured constant-context win."
    if opcode in REMOVE and row["coherence_shape"] == "independent-bundle":
        return "Independent effects and no material observed use; coupling them into one opcode adds complexity without portfolio benefit."
    if opcode in REMOVE and "temporary-register-passed" in temporary[opcode]:
        return "Caller/compiler temporary is exposed only to retain an intermediate side effect, with no material evidence strong enough to justify redesign."
    if opcode in REMOVE and executions == 0 and sites == 0:
        return "No program sites, library sites or executions across the 22-image portfolio; speculative form is not earning its opcode and maintenance cost."
    if opcode in REMOVE:
        return "Observed benefit is too small or inconsistent to justify the dedicated form."
    if opcode in {"ILOADSETUNLINK_REG_INT", "ILOADSETUNLINKN_REG_INT_REG"}:
        return "Retain as the clean no-temporary destination for the trace-preserving wide-form redesign."
    if executions == 0:
        return "Coherent form has static evidence or a specific replacement role; retain provisionally despite no bounded execution."
    if future_calling(row) != "not-applicable":
        return "Current measured use and saving justify retention, but reassess after the wider calling convention reduces swap demand."
    return "Coherent form with representative executions and a positive ordinary-Release signal."


fields = [
    "opcode", "class", "family", "components", "coherence", "temporary_policy",
    "future_calling_discount", "program_sites", "library_sites", "static_workload_breadth",
    "executions", "dynamic_workload_breadth", "use_band", "performance_signal",
    "rxvm_speedup_percent", "rxvm_delta_per_call_ns", "rxvm_estimated_portfolio_saved_ms",
    "rxbvm_speedup_percent", "rxbvm_delta_per_call_ns", "rxbvm_estimated_portfolio_saved_ms",
    "proposal", "rationale",
]

with OUT.open("w", newline="") as handle:
    writer = csv.DictWriter(handle, fieldnames=fields, delimiter="\t", lineterminator="\n")
    writer.writeheader()
    for row in manifest:
        opcode = row["opcode"]
        occ = occurrence[opcode]
        rxvm = release.get((opcode, "rxvm"), {})
        rxbvm = release.get((opcode, "rxbvm"), {})
        writer.writerow({
            "opcode": opcode,
            "class": row["class"],
            "family": row["family"],
            "components": row["components"],
            "coherence": coherence(row),
            "temporary_policy": temporary[opcode],
            "future_calling_discount": future_calling(row),
            "program_sites": occ["program_sites"],
            "library_sites": occ["library_sites"],
            "static_workload_breadth": occ["static_workload_breadth"],
            "executions": occ["executions"],
            "dynamic_workload_breadth": occ["dynamic_workload_breadth"],
            "use_band": use_band(occ),
            "performance_signal": performance(opcode),
            "rxvm_speedup_percent": rxvm.get("speedup_percent", ""),
            "rxvm_delta_per_call_ns": rxvm.get("delta_per_call_mean_ns", ""),
            "rxvm_estimated_portfolio_saved_ms": rxvm.get("estimated_portfolio_saved_ms", ""),
            "rxbvm_speedup_percent": rxbvm.get("speedup_percent", ""),
            "rxbvm_delta_per_call_ns": rxbvm.get("delta_per_call_mean_ns", ""),
            "rxbvm_estimated_portfolio_saved_ms": rxbvm.get("estimated_portfolio_saved_ms", ""),
            "proposal": proposal(opcode),
            "rationale": rationale(row, occ),
        })

counts = {kind: 0 for kind in ("retain", "remove", "replace-current-form")}
for row in read_tsv(OUT):
    counts[row["proposal"]] += 1
print(f"{OUT}: {counts}")
