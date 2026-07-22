#!/usr/bin/env python3
"""Summarise COPY and allocation evidence for the NR-15 lifecycle control."""

import csv
import pathlib
import re
import sys


def number(value):
    return int(value or 0)


fields = [
    "profile", "vm", "candidate", "size", "copy_count", "copy_total_ns",
    "copy_average_ns", "binary_buffers", "binary_buffer_bytes",
    "binary_buffer_max_bytes", "attribute_value_blocks",
    "attribute_value_bytes", "attribute_value_max_bytes",
    "attribute_pointer_storage", "attribute_pointer_bytes",
    "attribute_pointer_max_bytes", "value_slots", "value_slot_bytes",
    "value_slot_max_bytes",
]

writer = csv.DictWriter(sys.stdout, fieldnames=fields)
writer.writeheader()
for path in sorted(pathlib.Path(sys.argv[1]).glob("lifecycle-*.csv")):
    match = re.fullmatch(r"lifecycle-(rxvm|rxbvm)-(d1|d2|d2h)-([0-9]+)\.csv", path.name)
    if not match:
        continue
    vm, candidate, size = match.groups()
    result = {field: 0 for field in fields}
    result.update(profile=path.name, vm=vm, candidate=candidate, size=size)
    with path.open(newline="") as source:
        for row in csv.DictReader(source):
            if row["section"] == "instruction" and row["name"] == "COPY_REG_REG":
                result["copy_count"] = number(row["count"])
                result["copy_total_ns"] = number(row["total_ns"])
                result["copy_average_ns"] = number(row["average_ns"])
            elif row["section"] == "allocation":
                mapping = {
                    "binary_buffers": "binary_buffer",
                    "attribute_value_blocks": "attribute_value",
                    "attribute_pointer_storage": "attribute_pointer",
                    "value_slots": "value_slot",
                }
                prefix = mapping.get(row["name"])
                if prefix:
                    result[row["name"]] = number(row["count"])
                    result[prefix + "_bytes"] = number(row["bytes"])
                    result[prefix + "_max_bytes"] = number(row["max_bytes"])
    writer.writerow(result)
