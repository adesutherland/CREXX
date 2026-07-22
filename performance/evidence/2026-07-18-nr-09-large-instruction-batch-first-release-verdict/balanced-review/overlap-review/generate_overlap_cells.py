#!/usr/bin/env python3
"""Generate matched Release cells for NR-09 mappings hidden by other mappings."""

from pathlib import Path
import csv


HERE = Path(__file__).resolve().parent
TEMPLATES = HERE / "templates"


CELLS = [
    {
        "name": "FMULTICOPY_CHAIN_SELECTION",
        "loops": 1_000_000,
        "executions": 501_000,
        "init": ["load r5,7", "load r6,8"],
        "current": [
            "itof r8,r5",
            "fmult r8,r8,2.0",
            "itof r9,r6",
        ],
        "alternative": [
            "itof r8,r5",
            "fmulticopy r8,2.0,r9,r6",
            "itof r9",
        ],
    },
    {
        "name": "ITOF2_CHAIN_RETENTION",
        "loops": 1_000_000,
        "executions": 501_000,
        "init": ["load r5,7", "load r6,8"],
        "current": [
            "itof r8,r5",
            "fmulticopy r8,2.0,r9,r6",
            "itof r9",
        ],
        "alternative": [
            "icopy r8,r5",
            "itof r8",
            "fmulticopy r8,2.0,r9,r6",
            "itof r9",
        ],
    },
    {
        "name": "LINKSETATTRSLINKADD_PROMOTION",
        "loops": 500_000,
        "executions": 5_569_610,
        "init": ["setattrs r1,2", "load r4,1"],
        "current": [
            "linkattr1 r2,r1,2",
            "setlinkattr1 r3,r2,6,r4,1",
            "unlink r3",
            "unlink r2",
        ],
        "alternative": [
            "linksetattrslinkadd r2,r1,2,6,r3,r4,1",
            "unlink r3",
            "unlink r2",
        ],
    },
    {
        "name": "SETLINKILOAD_PROMOTION",
        "loops": 500_000,
        "executions": 1_606_900,
        "init": ["null r1", "load r4,1"],
        "current": [
            "setlinkattr1 r2,r1,4,r4",
            "load r3,1",
            "isetunlink r2,r3",
        ],
        "alternative": [
            "setlinkiload r2,r1,4,r4,r3,1",
            "isetunlink r2,r3",
        ],
    },
]


def indent(lines):
    return "\n".join(f"    {line}" for line in lines)


def render_measure(side, spec):
    stem = spec["name"].lower()
    padding = "@PADDING@\n" if side == "alternative" else ""
    return f"""measure_{side}_{stem}() .locals=24
{padding}{indent(spec['init'])}
    mtime r22
    load r0,{spec['loops']}
{stem}_{side}_loop:
{indent(spec[side])}
    dec r0
    brt {stem}_{side}_loop,r0
    mtime r23
    isub r1,r23,r22
    itos r1
    say r1
    ret r1
"""


def render(spec):
    stem = spec["name"].lower()
    return f"""/* Generated overlap-selection Release cell for {spec['name']}. */

.globals=0

main() .locals=8
    load r5,5
    load r6,0
pair:
    brt reverse_order,r6
forward_order:
    say "current_us"
    call r1,measure_current_{stem}()
    say "alternative_us"
    call r1,measure_alternative_{stem}()
    load r6,1
    br pair_done
reverse_order:
    say "alternative_us"
    call r1,measure_alternative_{stem}()
    say "current_us"
    call r1,measure_current_{stem}()
    load r6,0
pair_done:
    dec r5
    brt pair,r5
    ret 0

{render_measure('current', spec)}
{render_measure('alternative', spec)}
"""


def main():
    TEMPLATES.mkdir(parents=True, exist_ok=True)
    for old in TEMPLATES.glob("*.rxas.in"):
        old.unlink()
    for spec in CELLS:
        (TEMPLATES / f"{spec['name'].lower()}.rxas.in").write_text(render(spec))
    with (HERE / "manifest.tsv").open("w", newline="") as handle:
        writer = csv.writer(handle, delimiter="\t", lineterminator="\n")
        writer.writerow(["comparison", "loop_count", "portfolio_executions"])
        for spec in CELLS:
            writer.writerow([spec["name"], spec["loops"], spec["executions"]])
    print(f"generated {len(CELLS)} overlap templates in {TEMPLATES}")


if __name__ == "__main__":
    main()
