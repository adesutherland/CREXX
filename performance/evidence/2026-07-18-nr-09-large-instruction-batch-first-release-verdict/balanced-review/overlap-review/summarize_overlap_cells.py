#!/usr/bin/env python3
"""Summarize alignment-balanced NR-09 overlap-selection cells."""

from collections import defaultdict
from pathlib import Path
import csv
import math
import statistics


HERE = Path(__file__).resolve().parent


def mean_ci95(values):
    mean = statistics.mean(values)
    half = 2.131 * statistics.stdev(values) / math.sqrt(len(values))
    return mean, mean - half, mean + half


with (HERE / "manifest.tsv").open(newline="") as handle:
    executions = {
        row["comparison"]: int(row["portfolio_executions"])
        for row in csv.DictReader(handle, delimiter="\t")
    }

raw = defaultdict(list)
with (HERE / "samples.tsv").open(newline="") as handle:
    for row in csv.DictReader(handle, delimiter="\t"):
        raw[(row["vm"], row["comparison"])].append(row)

fields = [
    "vm", "comparison", "n", "loop_count", "current_mean_us",
    "alternative_mean_us", "alternative_speedup_percent",
    "delta_per_call_mean_ns", "delta_per_call_ci95_low_ns",
    "delta_per_call_ci95_high_ns", "faster_fraction",
    "classification", "portfolio_executions",
    "estimated_portfolio_saved_ms",
]
rows_out = []
for (vm, comparison), rows in sorted(raw.items()):
    by_padding = defaultdict(list)
    current = defaultdict(list)
    alternative = defaultdict(list)
    for row in rows:
        pad = int(row["padding_cells"])
        by_padding[pad].append(float(row["delta_per_call_ns"]))
        current[pad].append(float(row["current_us"]))
        alternative[pad].append(float(row["alternative_us"]))
    if sorted(by_padding) != list(range(16)) or any(len(v) != 12 for v in by_padding.values()):
        raise SystemExit(f"incomplete alignment evidence for {vm} {comparison}")
    padding_medians = [statistics.median(by_padding[pad]) for pad in range(16)]
    delta, low, high = mean_ci95(padding_medians)
    current_mean = statistics.mean(statistics.median(current[pad]) for pad in range(16))
    alternative_mean = statistics.mean(statistics.median(alternative[pad]) for pad in range(16))
    if high < 0:
        classification = "alternative-saving"
    elif low > 0:
        classification = "alternative-slowdown"
    else:
        classification = "noise-or-layout-sensitive"
    count = executions[comparison]
    rows_out.append({
        "vm": vm,
        "comparison": comparison,
        "n": len(rows),
        "loop_count": int(rows[0]["loop_count"]),
        "current_mean_us": f"{current_mean:.3f}",
        "alternative_mean_us": f"{alternative_mean:.3f}",
        "alternative_speedup_percent": f"{(current_mean / alternative_mean - 1.0) * 100.0:+.6f}",
        "delta_per_call_mean_ns": f"{delta:+.6f}",
        "delta_per_call_ci95_low_ns": f"{low:+.6f}",
        "delta_per_call_ci95_high_ns": f"{high:+.6f}",
        "faster_fraction": f"{sum(float(row['delta_per_call_ns']) < 0 for row in rows) / len(rows):+.6f}",
        "classification": classification,
        "portfolio_executions": count,
        "estimated_portfolio_saved_ms": f"{-delta * count / 1_000_000.0:+.6f}",
    })

with (HERE / "summary.tsv").open("w", newline="") as handle:
    writer = csv.DictWriter(handle, fieldnames=fields, delimiter="\t", lineterminator="\n")
    writer.writeheader()
    writer.writerows(rows_out)

print(HERE / "summary.tsv")
