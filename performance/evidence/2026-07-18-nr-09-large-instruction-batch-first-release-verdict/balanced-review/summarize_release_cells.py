#!/usr/bin/env python3
"""Summarize alignment-balanced NR-09 ordinary Release isolation cells."""

from collections import defaultdict
from pathlib import Path
import csv
import math
import statistics


HERE = Path(__file__).resolve().parent
SAMPLES = HERE / "release-cell-samples.tsv"
OCCURRENCES = HERE / "portfolio-occurrences.tsv"
SETTPCALL = HERE.parent / "settpcall-review" / "alignment-sweep-summary.csv"
OUT = HERE / "release-cell-summary.tsv"


def mean_ci95(values):
    mean = statistics.mean(values)
    if len(values) < 2:
        return mean, mean, mean
    # Sixteen independently positioned code layouts; t(0.975, 15)=2.131.
    half = 2.131 * statistics.stdev(values) / math.sqrt(len(values))
    return mean, mean - half, mean + half


def classification(low, high):
    if high < 0:
        return "measurable-saving"
    if low > 0:
        return "measurable-slowdown"
    return "noise-or-layout-sensitive"


with OCCURRENCES.open(newline="") as handle:
    occurrence = {row["opcode"]: row for row in csv.DictReader(handle, delimiter="\t")}

raw = defaultdict(list)
with SAMPLES.open(newline="") as handle:
    for row in csv.DictReader(handle, delimiter="\t"):
        raw[(row["vm"], row["opcode"])].append(row)

summary = []
for (vm, opcode), rows in sorted(raw.items()):
    by_padding = defaultdict(list)
    for row in rows:
        by_padding[int(row["padding_cells"])].append(float(row["delta_per_call_ns"]))
    if sorted(by_padding) != list(range(16)) or any(len(v) != 12 for v in by_padding.values()):
        raise SystemExit(f"incomplete alignment evidence for {vm} {opcode}")
    # Median within an offset rejects isolated scheduler spikes while retaining
    # repeatable code-position effects. The cross-offset mean weights all
    # sixteen positions equally.
    padding_medians = [statistics.median(by_padding[pad]) for pad in range(16)]
    delta_mean, delta_low, delta_high = mean_ci95(padding_medians)
    expanded_by_padding = defaultdict(list)
    fused_by_padding = defaultdict(list)
    for row in rows:
        pad = int(row["padding_cells"])
        expanded_by_padding[pad].append(float(row["expanded_us"]))
        fused_by_padding[pad].append(float(row["fused_us"]))
    expanded_mean = statistics.mean(statistics.median(expanded_by_padding[pad]) for pad in range(16))
    fused_mean = statistics.mean(statistics.median(fused_by_padding[pad]) for pad in range(16))
    speedup = (expanded_mean / fused_mean - 1.0) * 100.0
    executions = int(occurrence[opcode]["executions"])
    portfolio_saved_ms = -delta_mean * executions / 1_000_000.0
    summary.append({
        "vm": vm,
        "opcode": opcode,
        "n": len(rows),
        "loop_count": int(rows[0]["loop_count"]),
        "expanded_mean_us": expanded_mean,
        "fused_mean_us": fused_mean,
        "speedup_percent": speedup,
        "delta_per_call_mean_ns": delta_mean,
        "delta_per_call_ci95_low_ns": delta_low,
        "delta_per_call_ci95_high_ns": delta_high,
        "faster_fraction": sum(float(row["delta_per_call_ns"]) < 0 for row in rows) / len(rows),
        "classification": classification(delta_low, delta_high),
        "portfolio_executions": executions,
        "estimated_portfolio_saved_ms": portfolio_saved_ms,
        "evidence": "new-16-offset-cell",
    })

# Reuse the already accepted SETTPCALL alignment cell. Its report
# records the exact speedup percentages; the CSV provides alignment-averaged
# per-call deltas. Confidence bounds were not calculated in that earlier cell.
settpcall_speedup = {"rxvm": 0.458, "rxbvm": 1.507}
with SETTPCALL.open(newline="") as handle:
    for row in csv.DictReader(handle):
        if row["padding_cells"] != "all":
            continue
        vm = row["vm"]
        delta = float(row["mean_delta_per_call_ns"])
        executions = int(occurrence["SETTPCALL_REG_FUNC_REG_REG_INT"]["executions"])
        summary.append({
            "vm": vm,
            "opcode": "SETTPCALL_REG_FUNC_REG_REG_INT",
            "n": int(row["n"]),
            "loop_count": 5_000_000,
            "expanded_mean_us": "",
            "fused_mean_us": "",
            "speedup_percent": settpcall_speedup[vm],
            "delta_per_call_mean_ns": delta,
            "delta_per_call_ci95_low_ns": "",
            "delta_per_call_ci95_high_ns": "",
            "faster_fraction": "",
            "classification": "measurable-saving",
            "portfolio_executions": executions,
            "estimated_portfolio_saved_ms": -delta * executions / 1_000_000.0,
            "evidence": "reused-settpcall-5m-16-offset-cell",
        })

fields = [
    "vm", "opcode", "n", "loop_count", "expanded_mean_us", "fused_mean_us",
    "speedup_percent", "delta_per_call_mean_ns", "delta_per_call_ci95_low_ns",
    "delta_per_call_ci95_high_ns", "faster_fraction", "classification",
    "portfolio_executions", "estimated_portfolio_saved_ms", "evidence",
]
with OUT.open("w", newline="") as handle:
    writer = csv.DictWriter(handle, fieldnames=fields, delimiter="\t", lineterminator="\n")
    writer.writeheader()
    for row in sorted(summary, key=lambda item: (item["opcode"], item["vm"])):
        formatted = row.copy()
        for key in ("expanded_mean_us", "fused_mean_us"):
            if isinstance(formatted[key], float):
                formatted[key] = f"{formatted[key]:.3f}"
        for key in ("speedup_percent", "delta_per_call_mean_ns",
                    "delta_per_call_ci95_low_ns", "delta_per_call_ci95_high_ns",
                    "faster_fraction", "estimated_portfolio_saved_ms"):
            if isinstance(formatted[key], float):
                formatted[key] = f"{formatted[key]:+.6f}"
        writer.writerow(formatted)

print(OUT)
