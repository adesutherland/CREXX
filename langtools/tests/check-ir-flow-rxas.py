#!/usr/bin/env python3
"""Read built, optimised SDK RXAS and check analyse's lattice storage shape.

Metadata resolves local registers; source/debug records and unrelated object
copies are deliberately outside the instruction checks. This is a structural
regression, not an RXAS interpreter or a proof of the analysis algorithm.
"""
from pathlib import Path
import re
import sys


class ShapeError(ValueError):
    pass


def require(condition, message):
    if not condition:
        raise ShapeError(message)


def check(text):
    lines = text.splitlines()
    starts = [i for i, line in enumerate(lines)
              if re.search(r"\s\.expose=llirservices\.analyse\s*$", line)]
    require(len(starts) == 1, "expected one exposed llirservices.analyse routine")
    start = starts[0]
    end = next((i for i in range(start + 1, len(lines))
                if re.match(r"^\S+\(.*\s\.expose=", lines[i])), len(lines))
    routine = lines[start + 1:end]
    registers = {}
    for line in routine:
        match = re.match(
            r'\s*\.meta "llirservices\.analyse\.(\w+)"="b" '
            r'"(\.[^"]+)" ([ra]\d+)\s*$', line)
        if match:
            name, kind, register = match.groups()
            registers[name] = (kind, register)
    expected = {name: ".binary" for name in ("before", "after", "temp")}
    expected.update({name: ".int" for name in ("cells", "slots", "blocks")})
    for name, kind in expected.items():
        require(name in registers and registers[name][0] == kind,
                f"missing typed local metadata for {name}")
    reg = {name: registers[name][1] for name in expected}
    require(len(set(reg.values())) == len(reg), "storage/size locals share registers")
    instructions = []
    for line in routine:
        match = re.match(r"\s+([a-z][a-z0-9]*)\s+(.+)$", line)
        if match:
            instructions.append((match[1], tuple(x.strip() for x in match[2].split(","))))

    product = next((i for i, (op, args) in enumerate(instructions)
                    if op == "imult" and len(args) == 3 and args[0] == reg["cells"]
                    and set(args[1:]) == {reg["blocks"], reg["slots"]}), None)
    require(product is not None, "cells must be formed from blocks * slots")
    require(any(op == "idiv" and len(args) == 3 and args[2] == reg["slots"]
                for op, args in instructions[:product]),
            "missing division check before lattice-size multiplication")
    for name in ("before", "after"):
        sizes = [args[1:] for op, args in instructions
                 if op == "bresize" and args[0] == reg[name]]
        require(sizes == [(reg["cells"],)],
                f"{name} needs one cells-sized direct resize")
    scratch = [args[1:] for op, args in instructions
               if op == "bresize" and args[0] == reg["temp"]]
    require(scratch and all(size == (reg["slots"],) for size in scratch),
            "scratch resizes must remain one slots-sized row")
    for name in ("after", "temp"):
        require(any(op == "bgetu8" and len(args) == 3 and args[1] == reg[name]
                    for op, args in instructions), f"missing direct byte read of {name}")
    for name in ("before", "after", "temp"):
        require(any(op == "bsetu8" and len(args) == 3 and args[0] == reg[name]
                    for op, args in instructions), f"missing direct byte write of {name}")

    lattices = {reg["before"], reg["after"]}
    rows = []
    for op, args in instructions:
        # bincopy's ordinary source argument clones the full source binary
        # before bmove copies its requested span. A small length is insufficient.
        require(not any("rxfnsb.bincopy()" == arg for arg in args),
                "bincopy hides a whole-lattice source-argument copy")
        if op in {"copy", "bcopy"} and len(args) == 2:
            require(not lattices.intersection(args),
                    "whole-value copy involving an analysis lattice")
        if op == "bcopy" and len(args) == 3 and lattices.intersection(args[:2]):
            require(args[:2] == (reg["temp"], reg["before"]),
                    "lattice span copies must target the slots-sized scratch row")
            rows.append(args)
    require(1 <= len(rows) <= 2, "expected only transfer/publication row-copy sites")


def main():
    if len(sys.argv) != 2:
        raise ShapeError("usage: check-ir-flow-rxas.py BUILT_LLIRSERVICES_RXAS")
    check(Path(sys.argv[1]).read_text(encoding="utf-8"))
    print("IR_FLOW_RXAS|PASS")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ShapeError) as error:
        print(f"IR_FLOW_RXAS|FAIL: {error}", file=sys.stderr)
        sys.exit(1)
