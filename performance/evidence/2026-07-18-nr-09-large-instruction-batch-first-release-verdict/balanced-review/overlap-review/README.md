# NR-09 overlap and mapping-order correction

Status: **complete diagnostic correction; no production edit made**.

## Question resolved

Both inputs are post-assembler facts, but they answer different questions:

- the original 76-pattern selection came from runtime RXSEQ windows over
  assembled RXBINs;
- the all-enabled occurrence census disassembles the final candidate RXBINs
  and reads VM opcode execution counts.

RXSEQ records consecutive executed instructions. A runtime window is not by
itself proof that the instructions are spatially adjacent, have the operand
relationships required by a mapping, or survive another mapping selected
earlier. Conversely, a zero in the all-enabled candidate can mean that a
different mapping consumed part of the sequence first; it does not prove that
the alternative has no current opportunity.

## Controlled replay

`audit_prebatch_replay.zsh` applies the current final rxc Class 2 combiner to
the retained accepted pre-batch compiler RXAS for all 22 program images, then
assembles with the current optimizing RXAS and executes the bounded portfolio.
The accepted pre-batch linked library is used unchanged and is excluded from
the replay static-site count. Results and exact product boundaries are in
`prebatch-replay.tsv` and `prebatch-replay-provenance.txt`.

Five first-pass removal candidates reappear in the controlled post-RXAS replay:

| Form | All-enabled executions | Replay sites | Replay executions | Interpretation |
| --- | ---: | ---: | ---: | --- |
| `FMULTICOPY_REG_FLOAT_REG_REG` | 0 | 4 | 501,000 | masked by direct two-register `ITOF` scheduling |
| `LINKSETATTRS_REG_REG_INT_INT` | 0 | 26 | 1,218 | overlaps the direct `SETLINKATTR1` choice; negligible |
| `LINKSETATTRSLINKADD_REG_REG_INT_INT_REG_REG_INT` | 0 | 168 | 5,569,610 | a suffix `SETLINKATTR1` is selected before the longer four-operation form |
| `SETLINKILOAD_REG_REG_INT_REG_REG_INT` | 0 | 4 | 1,606,900 | direct `SETLINKATTR1` consumes the prefix before the load can be fused |
| `UNLINKLINKATTR1_REG_REG_REG_INT` | 30 | 53 | 1,652,680 | largely subsumed by retained direct attribute/cleanup forms |

The two low-value overlaps remain valid removals: `LINKSETATTRS` competes with
another one-instruction mapping for only 1,218 bounded calls, and
`UNLINKLINKATTR1` is masked by the stronger retained direct attribute forms.
The other three require head-to-head timing against the path actually chosen
by the all-enabled candidate.

## Ordinary-Release overlap cells

`run_overlap_cells.zsh` supplies four rerunnable, alignment-balanced,
profiling-off Release comparisons. Each VM/comparison has 192 recorded pairs:
16 eight-byte alternative-procedure positions, three processes per position,
and four recorded alternating pairs after one warm-up pair.

| Comparison | `rxvm` | `rxbvm` | Decision consequence |
| --- | ---: | ---: | --- |
| Use `FMULTICOPY` in the real conversion/multiply chain | +12.356% | +11.381% | retain and select the chain form |
| Remove the remaining two-register `ITOF` from that selected chain | -3.628% | -5.519% | retain `ITOF_REG_REG`, but narrow it to the chain that benefits |
| Promote `LINK + SETLINKATTR1` to `LINKSETATTRSLINKADD` | +4.656% | +2.368% | retain and add the promotion path |
| Promote `SETLINKATTR1 + LOAD` to `SETLINKILOAD` | +6.649% | +8.015% | retain and add the promotion path |

Positive percentages mean the alternative is faster than the current chosen
path. Every interval is wholly on the reported side of zero; exact per-call
means, intervals, faster fractions, bounded execution counts and projected
scale are in `summary.tsv`.

## Corrected disposition

The first-pass 30-form removal list is reduced to 26. These four forms move to
retention:

- `ITOF_REG_REG`, with generic emission narrowed rather than used everywhere;
- `FMULTICOPY_REG_FLOAT_REG_REG`, selected for the measured arithmetic chain;
- `LINKSETATTRSLINKADD_REG_REG_INT_INT_REG_REG_INT`, promoted over the shorter
  overlapping suffix mapping;
- `SETLINKILOAD_REG_REG_INT_REG_REG_INT`, promoted over the shorter overlapping
  prefix mapping.

This correction does not authorize the production changes. It corrects the
evidence and requires Adrian's approval of the revised 26-remove, 32-retain,
two-redesign disposition before production editing resumes.

## Rerun

From the repository root with the current ordinary and profiling Release
products already built:

```sh
performance/evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/balanced-review/overlap-review/audit_prebatch_replay.zsh
performance/evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/balanced-review/overlap-review/run_overlap_cells.zsh
```
