# PERF3-02-C1B receiver-link ownership analysis

Status: **complete — bounded PoC candidate; no implementation or timing**

This analysis reopens only the material C1a-R1 correctness rejection from the
checksum-closed PERF3-02 panel. It does not modify that panel, production code,
benchmarks, generated products or retained lifecycle artifacts.

## Disposition

C1b is a **bounded PoC candidate**, but only as the narrow
`C1b-R1 detached receiver-guard snapshot` route described in
[`analysis.md`](analysis.md). The rejected C1a-R1 rule remains invalid: absence
of receiver writes does not prove that receiver-derived alias registers are
balanced at every rewritten return.

The two material sites are structurally simple enough for a finite compiler
proof:

- both callees are read-only Boolean methods with four explicit returns;
- they contain no calls, loops, fallthrough, reference operations, assembler
  blocks or signal blocks;
- every receiver read is one indexed scalar Boolean guard; and
- the containing procedures install no same-frame signal handler.

The proposed PoC would materialize every receiver-derived guard into a
compiler-owned scalar statement before the `IF`, then bind the receiver
directly only when that structural proof passes. Assignment lowering already
emits the right-hand-side cleanup before the following statement, so no
receiver-derived alias remains live when a rewritten `LEAVE_WITH` branches to
the common inline epilogue. All other shapes retain the current private full
receiver copy.

## Material ceiling retained

If correctness succeeds, the exact Richards ceiling removes two static and
172,394 dynamic top-level receiver copies, 25,341,738 recursive copy
operations and 201,354,752 logical bytes. This is about 5.2 times the
top-level and recursive work removed by the correct C1a-R2 slice, but it is an
operation ceiling rather than a speed forecast. C1a-R1 has no valid timing.

The ceiling permits scalar snapshot/copy and alias-cleanup instructions. It
requires zero full receiver-copy allocation, recursive traversal or ownership
search at the two sites and requires canonical Richards output
`23246/9297` in optimized and non-optimized images on both VMs.

## Evidence map

- [`analysis.md`](analysis.md): failure proof, strategy comparison, bounded
  structural contract, exceptional-edge boundary and proposed next gate;
- [`site-ledger.csv`](site-ledger.csv): exact source/RXAS identity and retained
  operation counts;
- [`exit-link-ledger.csv`](exit-link-ledger.csv): all eight explicit return
  paths and the four alias-unsafe taken edges;
- [`strategy-comparison.csv`](strategy-comparison.csv): B0-B3 comparison and
  disposition; and
- [`provenance.md`](provenance.md): immutable inputs, hashes and source-code
  anchors.

No PoC, product build, execution, timing, production edit, commit or push was
performed by this activity.
