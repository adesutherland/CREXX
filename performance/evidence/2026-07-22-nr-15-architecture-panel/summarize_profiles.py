#!/usr/bin/env python3
"""Emit a compact, reproducible summary of the NR-15 panel profiles."""

import csv
import pathlib
import re
import sys


def as_int(value):
    return int(value or 0)


def main(directory):
    fields = [
        "profile", "vm", "candidate", "mode", "family", "size",
        "total_instructions", "native_ops", "native_total_ns",
        "native_average_ns", "hot_ops", "hot_total_ns", "hot_average_ns",
        "direct_bytecode_calls", "frame_activations", "frame_reuses",
        "binary_buffers", "binary_buffer_bytes", "binary_buffer_max_bytes",
        "string_buffers", "string_buffer_bytes", "string_buffer_max_bytes",
        "attribute_value_blocks", "attribute_value_bytes",
        "attribute_value_max_bytes", "attribute_pointer_storage",
        "attribute_pointer_bytes", "attribute_pointer_max_bytes",
        "value_slots", "value_slot_bytes", "value_slot_max_bytes",
    ]
    writer = csv.DictWriter(sys.stdout, fieldnames=fields)
    writer.writeheader()
    for path in sorted(pathlib.Path(directory).glob("*.csv")):
        match = re.fullmatch(
            r"(rxvm|rxbvm)-(d1|d2|d2h)-"
            r"(get-hit|get-miss|set-existing|set-new|reset-default|multi-tail)-"
            r"([a-z]+)-([0-9]+)\.csv",
            path.name,
        )
        if not match:
            continue
        vm, candidate, mode, family, size = match.groups()
        result = {field: 0 for field in fields}
        result.update(
            profile=path.name, vm=vm, candidate=candidate, mode=mode,
            family=family, size=size,
        )
        allocations = {}
        with path.open(newline="") as source:
            for row in csv.DictReader(source):
                if row["section"] == "instruction":
                    count = as_int(row["count"])
                    result["total_instructions"] += count
                    if row["name"].startswith("NR15D"):
                        result["native_ops"] += count
                        result["native_total_ns"] += as_int(row["total_ns"])
                        name = row["name"]
                        hot = (
                            (mode in ("get-hit", "get-miss") and "GET_REG" in name)
                            or (mode in ("set-existing", "set-new") and "SET_REG" in name)
                            or (mode == "reset-default" and
                                ("RESET_REG" in name or "GET_REG" in name))
                            or (mode == "multi-tail" and "GET2_REG" in name)
                        )
                        if hot:
                            result["hot_ops"] += count
                            result["hot_total_ns"] += as_int(row["total_ns"])
                elif row["section"] == "census" and row["name"] == "call_path":
                    if row["value"] == "direct_bytecode":
                        result["direct_bytecode_calls"] = as_int(row["count"])
                elif row["section"] == "allocation":
                    allocations[row["name"]] = row
        if result["native_ops"]:
            result["native_average_ns"] = round(
                result["native_total_ns"] / result["native_ops"], 3
            )
        if result["hot_ops"]:
            result["hot_average_ns"] = round(
                result["hot_total_ns"] / result["hot_ops"], 3
            )
        for name, prefix in [
            ("frame_activations", "frame_activations"),
            ("frame_reuses", "frame_reuses"),
            ("binary_buffers", "binary_buffer"),
            ("string_buffers", "string_buffer"),
            ("attribute_value_blocks", "attribute_value"),
            ("attribute_pointer_storage", "attribute_pointer"),
            ("value_slots", "value_slot"),
        ]:
            row = allocations.get(name, {})
            result[name] = as_int(row.get("count"))
            if prefix not in ("frame_activations", "frame_reuses"):
                result[prefix + "_bytes"] = as_int(row.get("bytes"))
                result[prefix + "_max_bytes"] = as_int(row.get("max_bytes"))
        writer.writerow(result)


if __name__ == "__main__":
    main(sys.argv[1])
