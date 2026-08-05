# PERF3-12B B4 comparative Release panel

Date: 2026-08-04

Status: complete; route-selection stop, no production candidate installed

## Verdict

Select **H1 — lazy loop-scoped joined-key reuse** for B5 production
reimplementation.

H1 is clearly favorable under both profiling-off Release VMs after the full
governed 36-pair panel. Its paired median improvement is 3.075212% on `rxvm`
and 4.274944% on `rxbvm`; the 95% intervals for the paired means remain wholly
positive at 2.517267%..3.704893% and 3.898184%..6.267461%. It is favorable in
35/36 and 36/36 pairs respectively.

S1 is a valid but materially weaker route. It is 0.673386% favorable on `rxvm`
but remains `noisy_inconclusive` at the 36-pair cap (-0.450322%..0.942436%
mean interval). Its `rxbvm` median is 0.523554% favorable with a positive
0.336964%..2.595843% interval. S1 should remain replayable as the rejected
fallback; its stable-left LOAD consumes 280,000 of the 1,960,000 removed hot
CONCAT dispatches.

S0 remains the control. No route is installed by this evidence commit; B5 is
blocked on Adrian's explicit S0/S1/H1 selection.

## Formal timing

The host was on AC, low-power mode was off and the remote terminal was absent.
The first attempted block was interrupted when Adrian closed other applications;
it produced no sample rows and is retained under `timing/superseded-host-change/`.
After the machine settled, every warmup and recorded round restarted from the
beginning.

The clean block used one warmup per cell and 12 balanced/rotating rounds. One
low S0/rxbvm observation triggered the standing absolute-cell rule, so all 12
samples were retained and 10 serial S0/rxbvm observations were appended; that
append is stable at 45.047237M..46.016829M CPS. S1's paired interval crossed
zero, so the full six-cell matrix received two further 12-pair extensions to
the 36-pair cap. A second low S0/rxbvm observation in the last block is also
retained. It leaves the combined control span above 10%, but does not invalidate
the block: the required serial append is clean, H1 remains clear on `rxvm`, and
all 36 H1/rxbvm pairs are favorable. No sample was removed.

Final canonical-default medians from `timing/absolute-summary-36.csv`:

| Route | rxvm CPS | rxbvm CPS |
|---|---:|---:|
| S0 | 45,615,173 | 45,500,731.5 |
| S1 | 45,906,186 | 45,806,401 |
| H1 | 47,084,651 | 47,335,875 |

Every timed process passed, and every output used `effective_count=500`.
`timing/paired-summary-12.csv`, `paired-summary-24.csv` and
`paired-summary-36.csv` make the predeclared extension decisions replayable.

## Exact work and storage

Counts-only profiles use the exact three retained RXBIN images with a dedicated
diagnostic VM build and `--smoke-count 200`. They are not wall-clock evidence.

| Route | VM | Instructions | LOAD string | CONCAT | String buffers | Copy operations |
|---|---|---:|---:|---:|---:|---:|
| S0 | rxvm | 52,559,060 | 4,228,620 | 4,480,006 | 280,022 | 10,071,574 |
| S1 | rxvm | 50,879,035 | 4,508,618 | 2,520,006 | 280,022 | 10,071,571 |
| H1 | rxvm | 50,599,088 | 4,228,621 | 2,520,006 | 280,022 | 10,071,577 |
| S0 | rxbvm | 52,559,060 | 4,228,620 | 4,480,006 | 280,022 | 10,071,574 |
| S1 | rxbvm | 50,879,060 | 4,508,620 | 2,520,006 | 280,022 | 10,071,574 |
| H1 | rxbvm | 50,599,068 | 4,228,620 | 2,520,006 | 280,022 | 10,071,574 |

This directly confirms the B2/B3 mechanism: both candidates remove 1,960,000
CONCAT dispatches; S1 adds about 280,000 LOADs while H1 adds no hot setup
instruction. String-buffer allocations and bytes are exactly identical across
all six cells. The single-digit copy differences are non-material benchmark
housekeeping; copy bytes differ by at most 31 from the same-VM control.

## RSS, image and assembler scale

Three RSS observations per route/VM are stable. Relative to the 19,709,952-byte
S0 median, H1 is -16,384 bytes on `rxvm` and -32,768 bytes on `rxbvm`; S1 is
+81,920 and -81,920 bytes. No memory guard is approached.

| Route | Main static | Main locals | Code segment | RXBIN bytes | First SSA epoch | Ordinary assembly median |
|---|---:|---:|---:|---:|---:|---:|
| S0 | 369 | 103 | `0xf58` | 68,449 | 83,902,504 | 0.33 s / 132,907,008 B |
| S1 | 366 | 104 | `0xf4f` | 68,353 | 83,902,520 | 0.32 s / 132,743,168 B |
| H1 | 365 | 104 | `0xf48` | 68,377 | 83,902,504 | 0.33 s / 133,054,464 B |

H1 requests loop facts only for its candidate and increases first-epoch proof
work from 10,606 to 16,715, but retained SSA bytes remain exactly equal to S0
and the matched ordinary time/RSS series is neutral. This satisfies the B4
assembler-scale gate without adding eager whole-procedure loop analysis.

## Correctness and negatives

The final native selector suites pass 16/16 for each candidate, graph/metadata
checks pass 2/2 in each PoC tree, and all six exact route/VM images pass with
zero stderr. The accepted Sieve input is a no-candidate guard: S1 selects zero,
H1 reuses zero, both remain 73 -> 73 and their emitted images are byte-identical.
See `correctness-summary.txt` and `guards/`.

## Provenance and replay

- S0/common F1 control uses the S1 PoC RXAS tool with unmodified accepted RXAS;
  the segmented proof selects zero.
- S1 applies `artifacts/s1/s1-left-load.patch` to the generated accepted RXAS,
  then assembles with the S1 PoC tool.
- H1 applies `artifacts/h1/h1-cache-seed.patch` to the same generated base,
  then assembles with the H1 PoC tool.
- `artifacts.csv` records source commits and SHA-256 identities for every tool,
  input, patch, product and runtime. The prebuilt PoC banners contain a stale
  `dirty` suffix, so the clean worktree commits and exact binary hashes—not the
  banner alone—are the authority.
- `tools/` contains the Level B absolute, paired and counts summarizers.
- `checksums.sha256` closes the full evidence directory except itself.

The canonical benchmark source was not changed. S1 and H1 remain independent;
their overlapping ceilings are not summed. The next action, if H1 is selected,
is **B5 — clean production H1 reimplementation and mandatory first ordinary
Release verdict**. B5 must stop again after the smallest decisive Release
comparison; B6 broad closeout waits for that verdict to be accepted.
