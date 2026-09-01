#!/usr/bin/env python3

import csv
import math
import statistics
import sys
from collections import defaultdict
from pathlib import Path


def describe(values):
    return {
        "n": len(values),
        "median": statistics.median(values),
        "mean": statistics.mean(values),
        "min": min(values),
        "max": max(values),
    }


def paired_description(values):
    result = describe(values)
    quartiles = statistics.quantiles(values, n=4, method="inclusive")
    stdev = statistics.stdev(values)
    standard_error = stdev / math.sqrt(len(values))
    # Two-sided Student t, 95%, df=11. This campaign is fixed at n=12.
    half_width = 2.201 * standard_error
    result.update(
        stdev=stdev,
        standard_error=standard_error,
        ci95_low=result["mean"] - half_width,
        ci95_high=result["mean"] + half_width,
        q1=quartiles[0],
        q3=quartiles[2],
    )
    return result


def write_csv(path, fieldnames, rows):
    with path.open("w", newline="") as output:
        writer = csv.DictWriter(output, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


samples_path = Path(sys.argv[1])
output_dir = Path(sys.argv[2])
with samples_path.open(newline="") as source:
    samples = list(csv.DictReader(source))

recorded = [row for row in samples if row["phase"] == "recorded"]
if len(recorded) != 72:
    raise SystemExit(f"expected 72 recorded samples, found {len(recorded)}")

by_cell = defaultdict(list)
by_round = defaultdict(dict)
by_position = defaultdict(list)
for row in recorded:
    row["native_cps"] = float(row["native_cps"])
    row["process_real_s"] = float(row["process_real_s"])
    by_cell[(row["vm_mode"], row["cell"])].append(row)
    by_round[(row["vm_mode"], int(row["round"]))][row["cell"]] = row
    by_position[(row["vm_mode"], row["cell"], int(row["position"]))].append(row)

cell_rows = []
for vm_mode in ("rxvm", "rxbvm"):
    for cell in ("A", "B", "C"):
        rows = by_cell[(vm_mode, cell)]
        cps = describe([row["native_cps"] for row in rows])
        real = describe([row["process_real_s"] for row in rows])
        cell_rows.append(
            {
                "vm_mode": vm_mode,
                "cell": cell,
                "n": cps["n"],
                "native_cps_median": f'{cps["median"]:.3f}',
                "native_cps_mean": f'{cps["mean"]:.3f}',
                "native_cps_min": f'{cps["min"]:.0f}',
                "native_cps_max": f'{cps["max"]:.0f}',
                "process_real_median_s": f'{real["median"]:.3f}',
                "process_real_mean_s": f'{real["mean"]:.3f}',
                "process_real_min_s": f'{real["min"]:.2f}',
                "process_real_max_s": f'{real["max"]:.2f}',
            }
        )

write_csv(output_dir / "cell-summary.csv", list(cell_rows[0]), cell_rows)

comparisons = (
    ("infrastructure_B_vs_A", "A", "B"),
    ("corrected_product_C_vs_B", "B", "C"),
    ("overall_C_vs_A", "A", "C"),
)
delta_rows = []
for vm_mode in ("rxvm", "rxbvm"):
    for round_number in range(1, 13):
        cells = by_round[(vm_mode, round_number)]
        if set(cells) != {"A", "B", "C"}:
            raise SystemExit(f"incomplete round: {vm_mode} {round_number}")
        for comparison, left, right in comparisons:
            left_row = cells[left]
            right_row = cells[right]
            delta_rows.append(
                {
                    "vm_mode": vm_mode,
                    "round": round_number,
                    "comparison": comparison,
                    "left_cell": left,
                    "right_cell": right,
                    "left_position": left_row["position"],
                    "right_position": right_row["position"],
                    "left_native_cps": f'{left_row["native_cps"]:.0f}',
                    "right_native_cps": f'{right_row["native_cps"]:.0f}',
                    "native_cps_delta_percent": f'{((right_row["native_cps"] / left_row["native_cps"]) - 1.0) * 100.0:+.6f}',
                    "left_process_real_s": f'{left_row["process_real_s"]:.2f}',
                    "right_process_real_s": f'{right_row["process_real_s"]:.2f}',
                    "process_real_delta_percent": f'{((right_row["process_real_s"] / left_row["process_real_s"]) - 1.0) * 100.0:+.6f}',
                }
            )

write_csv(output_dir / "paired-deltas.csv", list(delta_rows[0]), delta_rows)

summary_rows = []
for vm_mode in ("rxvm", "rxbvm"):
    for comparison, _, _ in comparisons:
        rows = [
            row
            for row in delta_rows
            if row["vm_mode"] == vm_mode and row["comparison"] == comparison
        ]
        cps = paired_description([float(row["native_cps_delta_percent"]) for row in rows])
        real = paired_description([float(row["process_real_delta_percent"]) for row in rows])
        summary_rows.append(
            {
                "vm_mode": vm_mode,
                "comparison": comparison,
                "n": cps["n"],
                "native_cps_delta_median_percent": f'{cps["median"]:+.6f}',
                "native_cps_delta_mean_percent": f'{cps["mean"]:+.6f}',
                "native_cps_delta_ci95_low_percent": f'{cps["ci95_low"]:+.6f}',
                "native_cps_delta_ci95_high_percent": f'{cps["ci95_high"]:+.6f}',
                "native_cps_delta_q1_percent": f'{cps["q1"]:+.6f}',
                "native_cps_delta_q3_percent": f'{cps["q3"]:+.6f}',
                "native_cps_favorable_count": sum(float(row["native_cps_delta_percent"]) > 0 for row in rows),
                "native_cps_delta_min_percent": f'{cps["min"]:+.6f}',
                "native_cps_delta_max_percent": f'{cps["max"]:+.6f}',
                "process_real_delta_median_percent": f'{real["median"]:+.6f}',
                "process_real_delta_mean_percent": f'{real["mean"]:+.6f}',
                "process_real_delta_ci95_low_percent": f'{real["ci95_low"]:+.6f}',
                "process_real_delta_ci95_high_percent": f'{real["ci95_high"]:+.6f}',
                "process_real_delta_q1_percent": f'{real["q1"]:+.6f}',
                "process_real_delta_q3_percent": f'{real["q3"]:+.6f}',
                "process_real_favorable_count": sum(float(row["process_real_delta_percent"]) < 0 for row in rows),
                "process_real_delta_min_percent": f'{real["min"]:+.6f}',
                "process_real_delta_max_percent": f'{real["max"]:+.6f}',
            }
        )

write_csv(output_dir / "paired-summary.csv", list(summary_rows[0]), summary_rows)

position_rows = []
for vm_mode in ("rxvm", "rxbvm"):
    for cell in ("A", "B", "C"):
        for position in (1, 2, 3):
            rows = by_position[(vm_mode, cell, position)]
            cps = describe([row["native_cps"] for row in rows])
            real = describe([row["process_real_s"] for row in rows])
            position_rows.append(
                {
                    "vm_mode": vm_mode,
                    "cell": cell,
                    "position": position,
                    "n": cps["n"],
                    "native_cps_mean": f'{cps["mean"]:.3f}',
                    "process_real_mean_s": f'{real["mean"]:.3f}',
                }
            )

write_csv(output_dir / "position-summary.csv", list(position_rows[0]), position_rows)
