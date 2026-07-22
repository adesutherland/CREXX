#!/usr/bin/env python3
"""Generate matched profiling-off Release cells for exercised NR-09 forms.

Each cell holds setup and loop control constant and differs only at the
expanded legacy sequence versus fused instruction.  The runner sweeps the
fused procedure across all sixteen eight-byte positions in a 128-byte cache
line and alternates measurement order.  RXAS is invoked with -n so that the
expanded half cannot be re-fused by the Class 1 keyhole optimiser.
"""

from pathlib import Path
import csv
import re


ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
OCCURRENCES = HERE / "portfolio-occurrences.tsv"
OUT = HERE / "release-cells" / "templates"


def cell(opcode, loops, init, each, expanded, fused):
    return {
        "opcode": opcode,
        "loops": loops,
        "init": init,
        "each": each,
        "expanded": expanded,
        "fused": fused,
    }


CELLS = [
    cell("SWAPN_REG_REG_REG_REG", 2_000_000,
         ["load r1,1", "load r2,2", "load r3,3", "load r4,4"], [],
         ["swap r1,r2", "swap r3,r4"], ["swapn r1,r2,r3,r4"]),
    cell("SWAPN_REG_REG_REG_REG_REG_REG", 2_000_000,
         ["load r1,1", "load r2,2", "load r3,3", "load r4,4", "load r5,5", "load r6,6"], [],
         ["swap r1,r2", "swap r3,r4", "swap r5,r6"], ["swapn r1,r2,r3,r4,r5,r6"]),
    cell("SWAPN_REG_REG_REG_REG_REG_REG_REG_REG", 2_000_000,
         ["load r1,1", "load r2,2", "load r3,3", "load r4,4", "load r5,5", "load r6,6", "load r7,7", "load r8,8"], [],
         ["swap r1,r2", "swap r3,r4", "swap r5,r6", "swap r7,r8"],
         ["swapn r1,r2,r3,r4,r5,r6,r7,r8"]),
    cell("SETTPSWAP_REG_INT_REG", 2_000_000,
         ["load r1,1", "load r2,2"], [],
         ["settp r1,256", "swap r1,r2"], ["settpswap r1,256,r2"]),
    cell("LOADSETTP2_REG_INT_REG_INT", 2_000_000,
         [], [], ["load r1,11", "settp r2,512"], ["loadsettp2 r1,11,r2,512"]),
    cell("LOADSETTPSWAP_REG_INT_REG_INT_REG", 2_000_000,
         ["load r2,2", "load r3,3"], [],
         ["load r1,12", "settp r2,256", "swap r2,r3"],
         ["loadsettpswap r1,12,r2,256,r3"]),
    cell("SWAPSETTP_REG_REG_REG_INT", 2_000_000,
         ["load r1,1", "load r2,2"], [],
         ["swap r1,r2", "settp r3,512"], ["swapsettp r1,r2,r3,512"]),
    cell("SWAPSETTPSWAP_REG_REG_REG_INT_REG", 2_000_000,
         ["load r1,1", "load r2,2", "load r3,3", "load r4,4"], [],
         ["swap r1,r2", "settp r3,256", "swap r3,r4"],
         ["swapsettpswap r1,r2,r3,256,r4"]),
    cell("SETTPSWAPSETTPSWAP_REG_INT_REG_REG_REG", 2_000_000,
         ["load r1,1", "load r2,2", "load r3,3", "load r4,4"], [],
         ["settp r1,256", "swap r1,r2", "settp r3,256", "swap r3,r4"],
         ["settpswapsettpswap r1,256,r2,r3,r4"]),
    cell("NULLN_REG_REG", 2_000_000,
         ["load r1,1", "load r2,2"], [],
         ["null r1", "null r2"], ["nulln r1,r2"]),
    cell("NULLN_REG_REG_REG", 2_000_000,
         ["load r1,1", "load r2,2", "load r3,3"], [],
         ["null r1", "null r2", "null r3"], ["nulln r1,r2,r3"]),
    cell("NULLN_REG_REG_REG_REG", 2_000_000,
         ["load r1,1", "load r2,2", "load r3,3", "load r4,4"], [],
         ["null r1", "null r2", "null r3", "null r4"], ["nulln r1,r2,r3,r4"]),
    cell("SWAPCALL_REG_FUNC_REG_REG_REG", 500_000,
         ["load r10,1", "load r11,41", "load r12,0"], [],
         ["swap r11,r12", "call r1,return_arg(),r10"],
         ["swapcall r1,return_arg(),r10,r11,r12"]),
    cell("SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG", 500_000,
         ["load r10,1", "load r11,0", "load r12,42"], [],
         ["settp r12,256", "swap r12,r11", "call r1,return_arg(),r10"],
         ["settpswapcall r1,return_arg(),r10,r12,256,r11"]),
    # SETTPCALL already has a focused 20M-call diagnostic and a decisive
    # 5M-call x 16-offset dual-VM cell in ../settpcall-review; reuse it.
    cell("UNLINKN_REG_REG", 500_000,
         ["setattrs r1,2"], ["linkattr1 r2,r1,1", "linkattr1 r3,r1,2"],
         ["unlink r2", "unlink r3"], ["unlinkn r2,r3"]),
    cell("ISETUNLINK_REG_REG", 500_000,
         ["setattrs r1,1", "load r4,20"], ["linkattr1 r2,r1,1"],
         ["icopy r2,r4", "unlink r2"], ["isetunlink r2,r4"]),
    cell("IGETUNLINK_REG_REG", 500_000,
         ["setattrs r1,1", "linkattr1 r2,r1,1", "load r2,20", "unlink r2"],
         ["linkattr1 r2,r1,1"], ["icopy r5,r2", "unlink r2"], ["igetunlink r5,r2"]),
    cell("ISETUNLINKN_REG_REG_REG", 500_000,
         ["setattrs r1,2", "load r4,22"], ["linkattr1 r2,r1,1", "linkattr1 r3,r1,2"],
         ["icopy r2,r4", "unlink r2", "unlink r3"], ["isetunlinkn r2,r4,r3"]),
    cell("UNLINKBR_REG_ID", 500_000,
         ["setattrs r1,1"], ["linkattr1 r2,r1,1"],
         ["unlink r2", "br {body_done}"], ["unlinkbr r2,{body_done}"]),
    cell("SETLINKATTR1_REG_REG_INT_INT", 500_000,
         [], [], ["setattrs r1,4", "linkattr1 r2,r1,2"], ["setlinkattr1 r2,r1,4,2"]),
    cell("SETLINKATTR1_REG_REG_INT_REG", 500_000,
         ["load r4,2"], [], ["setattrs r1,4", "linkattr1 r2,r1,r4"],
         ["setlinkattr1 r2,r1,4,r4"]),
    cell("SETLINKATTR1_REG_REG_INT_REG_INT", 500_000,
         ["load r4,2"], [],
         ["setattrs r1,4", "iadd r5,r4,2", "linkattr1 r2,r1,r5"],
         ["setlinkattr1 r2,r1,4,r4,2"]),
    cell("MINLINKATTR1_REG_REG_INT", 500_000,
         [], [], ["minattrs r1,5", "linkattr1 r2,r1,5"], ["minlinkattr1 r2,r1,5"]),
    cell("MINLINKATTR1_REG_REG_REG_INT", 500_000,
         ["load r4,5"], [], ["minattrs r1,r4,0", "linkattr1 r2,r1,r4"],
         ["minlinkattr1 r2,r1,r4,0"]),
    cell("ITOF_REG_REG", 1_000_000,
         ["load r1,7"], [], ["icopy r2,r1", "itof r2"], ["itof r2,r1"]),
    cell("FDIVSUB_REG_REG_REG_FLOAT", 500_000,
         ["load r1,8.0"], ["load r2,2.0"],
         ["fdiv r2,r1,r2", "fsub r3,r2,1.5"], ["fdivsub r3,r1,r2,1.5"]),
    cell("ITOSCONCAT_REG_STRING_REG", 100_000,
         ["load r2,84"], [], ["itos r2", "concat r1,\"value=\",r2"],
         ["itosconcat r1,\"value=\",r2"]),
    cell("SCONCATITOS_REG_STRING_REG", 100_000,
         ["load r2,85"], ["load r1,\"head\""],
         ["sconcat r1,r1,\"tail\"", "itos r2"], ["sconcatitos r1,\"tail\",r2"]),
    cell("ISETATTR1_REG_INT_REG", 500_000,
         ["setattrs r1,1", "load r3,35"], [],
         ["linkattr1 r4,r1,1", "icopy r4,r3", "unlink r4"], ["isetattr1 r1,1,r3"]),
    cell("UNLINKLINKATTR1_REG_REG_REG_INT", 500_000,
         ["setattrs r1,2", "setattrs r6,1"], ["linkattr1 r7,r6,1"],
         ["unlink r7", "linkattr1 r8,r1,2"], ["unlinklinkattr1 r7,r8,r1,2"]),
    cell("MINSTOIATTR1_REG_REG_INT", 100_000,
         ["setattrs r1,1", "linkattr1 r4,r1,1", "load r4,\"77\"", "unlink r4"], [],
         ["minattrs r1,1", "linkattr1 r4,r1,1", "stoi r4", "icopy r2,r4", "unlink r4"],
         ["minstoiattr1 r2,r1,1"]),
    cell("ILOADSETUNLINKN_REG_REG_INT_REG", 500_000,
         ["setattrs r1,2"], ["linkattr1 r2,r1,1", "linkattr1 r3,r1,2"],
         ["load r4,25", "icopy r2,r4", "unlink r2", "unlink r3"],
         ["iloadsetunlinkn r4,r2,25,r3"]),
]


def indent(lines, spaces=4):
    prefix = " " * spaces
    return "\n".join(prefix + line for line in lines)


def slug(opcode):
    return opcode.lower()


def render_measure(which, spec):
    name = slug(spec["opcode"])
    prefix = f"{name}_{which}"
    body = [line.format(body_done=f"{prefix}_body_done") for line in spec[which]]
    padding = "@PADDING@\n" if which == "fused" else ""
    return f"""measure_{which}_{name}() .locals=24
{padding}{indent(spec['init'])}
    mtime r22
    load r0,{spec['loops']}
{prefix}_loop:
{indent(spec['each'])}
{indent(body)}
{prefix}_body_done:
    dec r0
    brt {prefix}_loop,r0
    mtime r23
    isub r1,r23,r22
    itos r1
    say r1
    ret r1
"""


def render(spec):
    name = slug(spec["opcode"])
    return f"""/* Generated matched Release cell for {spec['opcode']}. */

.globals=0

main() .locals=8
    load r5,5
    load r6,0
pair:
    brt reverse_order,r6
forward_order:
    say "expanded_us"
    call r1,measure_expanded_{name}()
    say "fused_us"
    call r1,measure_fused_{name}()
    load r6,1
    br pair_done
reverse_order:
    say "fused_us"
    call r1,measure_fused_{name}()
    say "expanded_us"
    call r1,measure_expanded_{name}()
    load r6,0
pair_done:
    dec r5
    brt pair,r5
    ret 0

{render_measure('expanded', spec)}
{render_measure('fused', spec)}
return_arg() .locals=0
    ret a1
"""


def read_exercised():
    with OCCURRENCES.open(newline="") as handle:
        rows = csv.DictReader(handle, delimiter="\t")
        return {row["opcode"] for row in rows if int(row["executions"]) > 0}


def main():
    exercised = read_exercised()
    generated = {spec["opcode"] for spec in CELLS}
    reused = {"SETTPCALL_REG_FUNC_REG_REG_INT"}
    missing = exercised - generated - reused
    extra = generated - exercised
    if missing or extra:
        raise SystemExit(f"cell coverage mismatch: missing={sorted(missing)} extra={sorted(extra)}")
    OUT.mkdir(parents=True, exist_ok=True)
    for old in OUT.glob("*.rxas.in"):
        old.unlink()
    for spec in CELLS:
        (OUT / f"{slug(spec['opcode'])}.rxas.in").write_text(render(spec))
    with (HERE / "release-cell-manifest.tsv").open("w", newline="") as handle:
        writer = csv.writer(handle, delimiter="\t", lineterminator="\n")
        writer.writerow(["opcode", "loop_count", "evidence"])
        for spec in CELLS:
            writer.writerow([spec["opcode"], spec["loops"], "new-16-offset-cell"])
        writer.writerow(["SETTPCALL_REG_FUNC_REG_REG_INT", 5_000_000,
                         "reuse-settpcall-review-16-offset-cell"])
    print(f"generated {len(CELLS)} templates in {OUT}")


if __name__ == "__main__":
    main()
