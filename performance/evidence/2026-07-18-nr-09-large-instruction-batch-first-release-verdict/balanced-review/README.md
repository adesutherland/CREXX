# NR-09 balanced per-form review

Status: **overlap-corrected evidence complete; revised proposal awaiting
Adrian's approval; no production instruction, assembler, compiler, database or
documentation removal has been made**.

## Recommendation

Reduce the provisional 60-form batch to 32 retained forms:

- remove 26 forms outright;
- remove and replace two current side-effect-preserving forms with clean
  designs;
- retain 32 forms, with four mapping-order corrections and swap/call forms
  explicitly subject to reassessment
  after the wider procedure calling convention reduces swap demand.

This is a simplification decision, not a claim that every retained cell adds
independently to product time. The corrected whole-batch verdict remains
neutral-to-slightly-positive at +0.586%/+0.671% paired median CPS with
intervals crossing zero.

## Post-RXAS overlap correction

The first scorecard correctly counted the all-enabled final RXBIN, but it
incorrectly treated a zero there as independent evidence that a form had no
current opportunity. The original selection was also post-RXAS, from runtime
RXSEQ windows, but runtime adjacency is not necessarily static adjacency and
mapping order can cause one form to mask another.

A controlled replay over the accepted pre-batch RXAS recovers five removal
candidates. Two remain removable overlaps. Three are live alternatives with
material counts and measurable dual-VM advantages over the path currently
selected. A fourth form, `ITOF_REG_REG`, is required by the winning arithmetic
selection. The corrected changes are:

- retain and select `FMULTICOPY_REG_FLOAT_REG_REG`: 501,000 replay executions;
  the real chain is +12.356%/+11.381% faster than the current path;
- retain `ITOF_REG_REG` but narrow generic emission: removing its remaining use
  from that chain is -3.628%/-5.519%;
- retain and promote
  `LINKSETATTRSLINKADD_REG_REG_INT_INT_REG_REG_INT`: 5,569,610 replay
  executions and +4.656%/+2.368%;
- retain and promote `SETLINKILOAD_REG_REG_INT_REG_REG_INT`: 1,606,900 replay
  executions and +6.649%/+8.015%.

Percentages are `rxvm`/`rxbvm`. Full method, replay counts, 16-offset ordinary
Release samples and rerun commands are in `overlap-review/README.md`. This
section supersedes the affected zero-use rationales in the original joined
scorecard; the all-enabled counts themselves remain valid facts for that exact
mapping order.

## Evidence boundary

The occurrence census executes the retained 11-workload portfolio in both
optimized and no-opt form: 22 program images, the current linked library and
bounded, correctness-checked arguments. It finds:

- 76,122,675 combined-instruction executions;
- 33/60 forms executed and 27/60 not executed;
- exact program/library static sites and workload/image breadth for all forms.

`rxvm` collected schema-4 counts; timing fields from that instrumented build
are ignored. Prior matched profiles established equal semantic instruction
counts in `rxvm` and `rxbvm`, while the ordinary-Release timing below covers
both VMs.

Every one of the 33 exercised forms has profiling-off Release evidence. Thirty
two new cells compare the exact legacy component sequence with the fused form;
the stronger existing `SETTPCALL` cell is reused. Each new form/VM result has
192 recorded paired observations: 16 eight-byte fused-procedure positions,
three fresh processes per position, five alternating pairs per process with
the first pair excluded as warm-up. Per-position medians reject scheduler
spikes; the reported interval is across all 16 runtime-image positions.

The full joined evidence is in `balanced-scorecard.tsv`. `program_sites`,
`library_sites`, workload breadth and executions are facts for this portfolio.
The projected portfolio milliseconds multiply an isolated per-call delta by
the observed executions only to show scale; projections overlap and must not
be summed into a product verdict.

## Caller-temporary rule

Six forms expose a register only to preserve an intermediate effect. Applying
the agreed rule gives a clean result:

| Current form | Executions | Release result | Proposal |
| --- | ---: | --- | --- |
| `ILOADCOPY_REG_REG_INT` | 0 | unmeasured/no use | remove |
| `FDIVSUB_REG_REG_REG_FLOAT` | 501,000 | +34.38%/+23.87% cell speedup (`rxvm`/`rxbvm`) | replace with clean result-only semantics; do not retain the quotient side effect |
| `ITOSCONCAT_REG_STRING_REG` | 15,420 | +0.82%/+1.24%; about 0.004/0.008 ms projected | remove; benefit is far too small for the exposed conversion temporary |
| `ILOADSETUNLINK_REG_REG_INT` | 0 | unmeasured/no use | remove |
| `ILOADSETUNLINKN_REG_REG_INT_REG` | 364,203 | +11.79%/+18.18% mean cell speedup; `rxvm` remains layout-sensitive | replace with a TRACE-correct compact form that does not accept the loaded temporary |
| `LINKILOADSETUNLINK_REG_REG_REG_REG_INT` | 0 | unmeasured/no use | remove |

The two positive cases justify retaining the operation concept, not the
current caller-visible side effect. `FDIVSUB` should calculate the quotient in
the handler and leave its divisor input unchanged. The wide unlink form should
route to the already-defined compact `ILOADSETUNLINKN_REG_INT_REG` shape after
TRACE retargeting is proved; if that proof fails, it should remain expanded
rather than retain the ugly operand.

## Proposed outright removals (26)

### Weak coherence or pointless side effects (10)

- `ILOADCOPY_REG_REG_INT`
- `FSUBILOAD_REG_REG_FLOAT_REG_INT`
- `ITOSCONCAT_REG_STRING_REG`
- `SCONCATITOS_REG_STRING_REG`
- `ILOADGETATTRS_REG_INT_REG_REG_INT`
- `LINKSETATTRSADD_REG_REG_INT_INT_REG_REG_INT`
- `SETATTRSADD_REG_INT_REG_REG_INT`
- `LINKILOAD_REG_REG_REG_REG_INT`
- `ILOADSETUNLINK_REG_REG_INT`
- `LINKILOADSETUNLINK_REG_REG_REG_REG_INT`

The remaining independent bundles have no material use. `SCONCATITOS` executes
only 36 times and has no consistent dual-VM saving. The retained-intermediate
and trace-temp forms do not meet the raised review bar.

### Speculative or subsumed forms (13)

- `NUMCTX_INT_INT_INT_INT_INT`
- `SETTPSWAPSETTP_REG_INT_REG_REG`
- `ILOADN_REG_INT_REG_INT`
- `ILOADN_REG_REG_INT`
- `IGETATTR1_REG_REG_INT`
- `MINIGETATTR1_REG_REG_INT`
- `ISETATTR1_REG_INT_INT`
- `RELINKATTR1_REG_REG_INT`
- `UNLINKRELINKATTR1_REG_REG_REG_INT`
- `LINKSETATTRS_REG_REG_INT_INT`
- `LINKATTR1ADD_REG_REG_REG_INT`
- `ISETATTR1_REG_REG_INT`
- `STOIATTR1_REG_REG_INT`

`NUMCTX` here is the new five-field form, not the separately accepted compiler
use of existing `NUMSCI`/`NUMENG`. Removing it does not undo the accepted Rule
1 improvement. The compact unlink forms are not in this list because they are
the clean target for eliminating the wide trace-temp forms.

`LINKSETATTRS` does reappear in the controlled replay, but only as a 1,218-call
one-instruction alternative to direct `SETLINKATTR1`; overlap is valid and it
does not justify retaining both forms.

### Exercised but not worth a dedicated form (3)

- `SETLINKATTR1_REG_REG_INT_INT`: 1,218 calls; projected saving below 0.001 ms.
- `UNLINKLINKATTR1_REG_REG_REG_INT`: largely subsumed by retained direct
  attribute/cleanup forms; its 1,652,680 replay calls do not survive that
  stronger mapping choice.
- `MINSTOIATTR1_REG_REG_INT`: 22 calls; projected saving below 0.001 ms.

## Forms retained after overlap correction (4)

- `ITOF_REG_REG`: retain the opcode but narrow generic emission to the measured
  chain where one use is required by the winning schedule.
- `FMULTICOPY_REG_FLOAT_REG_REG`: retain and promote in that arithmetic chain.
- `LINKSETATTRSLINKADD_REG_REG_INT_INT_REG_REG_INT`: retain and promote over
  `LINK + SETLINKATTR1`.
- `SETLINKILOAD_REG_REG_INT_REG_REG_INT`: retain and promote over
  `SETLINKATTR1 + LOAD`.

## Forms to replace rather than retain unchanged (2)

- `FDIVSUB_REG_REG_REG_FLOAT`: preserve the fused arithmetic opportunity but
  redesign it so only the requested result is written.
- `ILOADSETUNLINKN_REG_REG_INT_REG`: remove the explicit trace-only temporary
  operand and use/prove the compact form.

These redesigns are not authorized by this review. They are the two exceptions
to the default removal rule because their isolated savings are double-digit
and their occurrence counts are material enough to investigate.

## Retention and the future calling convention

The remaining 32 forms are listed by the scorecard plus the overlap correction.
The current largest observed populations and useful cells include direct
attribute write/link, multi-null, multi-swap, cleanup and call-window forms.
For example, `SETLINKATTR1_REG_REG_INT_REG_INT` and
`ISETATTR1_REG_INT_REG` each show roughly 30-41% cell speedups at more than five
million executions, while `LOADSETTPSWAP` executes 7.15 million times and
saves roughly 21% in both VMs.

The calling-convention qualification is explicit:

- `SETTPCALL` remains retained on its accepted exact 16-offset result:
  +0.458%/+1.507% at 1,784,956 portfolio calls.
- `SETTPSWAPCALL` currently executes 10,689,396 times and saves
  +1.17%/+2.52%; current evidence earns retention.
- `SWAPCALL` executes 4,013,349 times; it is a measurable `rxbvm` winner and
  layout/noise-limited but slightly positive in `rxvm`.
- swap-only preparation forms are retained for current measured value, not
  credited as permanent architecture. Reassess them after wider direct
  argument placement removes their producing swaps.

The one zero-use call-preparation form, `SETTPSWAPSETTP`, is proposed for
removal now rather than protected by speculative future value.

## Rerun

From the repository root:

```sh
performance/evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/balanced-review/collect_portfolio_occurrences.zsh
performance/evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/balanced-review/run_release_cells.zsh
performance/evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/balanced-review/build_scorecard.py
performance/evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/balanced-review/overlap-review/audit_prebatch_replay.zsh
performance/evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/balanced-review/overlap-review/run_overlap_cells.zsh
```

The first command requires the current profiling-enabled Release product in
`cmake-build-profile`; the second requires the ordinary profiling-off product
in `cmake-build-release`. All product hashes and measurement boundaries are in
the two provenance files.

## Approval gate

No removals or redesigns have been applied. Adrian's earlier approval of the
30-form removal list predates the overlap correction. Production editing is
paused for approval of the revised disposition of 26 removals, two replacements
and 32 retained forms. The next batch is then:

1. remove the 26 instructions across instruction inventory/database, VM,
   direct tests, RXAS/rxc mappings, docs and examples;
2. correct the four retained mapping selections, then record the two clean
   replacement designs without implementing them;
3. rerun focused correctness, including new-VM/accepted-old-RXBIN compatibility,
   and the mandatory first ordinary-Release verdict;
4. only after acceptance, complete broad Debug QA, sanitizer, install/package
   and final documentation/evidence closeout.
