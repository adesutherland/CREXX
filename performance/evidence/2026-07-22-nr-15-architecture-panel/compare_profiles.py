#!/usr/bin/env python3
"""Pivot hot-handler profile averages for direct candidate comparison."""

import csv
import collections
import sys


groups = collections.defaultdict(dict)
with open(sys.argv[1], newline="") as source:
    for row in csv.DictReader(source):
        key = (row["vm"], row["mode"], row["family"], row["size"])
        groups[key][row["candidate"]] = float(row["hot_average_ns"])

fields = [
    "vm", "mode", "family", "size", "d1_hot_average_ns",
    "d2_hot_average_ns", "d2h_hot_average_ns", "fastest",
    "d2_vs_d1_percent", "d2h_vs_d1_percent",
]
writer = csv.DictWriter(sys.stdout, fieldnames=fields)
writer.writeheader()
for (vm, mode, family, size), values in sorted(groups.items()):
    if set(values) != {"d1", "d2", "d2h"}:
        continue
    writer.writerow({
        "vm": vm,
        "mode": mode,
        "family": family,
        "size": size,
        "d1_hot_average_ns": values["d1"],
        "d2_hot_average_ns": values["d2"],
        "d2h_hot_average_ns": values["d2h"],
        "fastest": min(values, key=values.get),
        "d2_vs_d1_percent": round((values["d2"] / values["d1"] - 1) * 100, 3),
        "d2h_vs_d1_percent": round((values["d2h"] / values["d1"] - 1) * 100, 3),
    })
