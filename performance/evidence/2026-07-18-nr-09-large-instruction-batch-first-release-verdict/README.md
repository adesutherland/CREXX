# NR-09 large-instruction batch first Release verdict

Status: **the original cross-session negative verdict is superseded; the
approved corrected product removes 26 forms, selects four corrected mappings
and passes broad QA plus the final no-regression Release refresh; the batch
commit remains pending**.

This bundle compares the complete 67-mapping, 12-family Class 1/Class 2 batch
with the accepted NR-09 Rule 1 product baseline. It keeps the pre-batch
compiler-generated 22-image static portfolio, candidate static and dynamic
opcode deltas, ordinary profiling-off Release samples, exact provenance and
raw correctness output.

## Corrected drift-controlled verdict (2026-07-19)

The original `-2.233%`/`-2.289%` comparison did not rerun the accepted product
in the candidate session. A 12-round, all-permutation A/B/C campaign now
compares the reconstructed accepted product, the current VM on accepted
bytecode, and the complete fused product in one session on both VMs.

The complete batch has paired median CPS deltas of **+0.586%** on `rxvm` and
**+0.671%** on `rxbvm`; paired elapsed medians agree at -0.578% and -0.704%.
The uncertainty intervals cross zero, so this is neutral-to-slightly-positive,
not a material win. The VM/infrastructure control is neutral at -0.152% and
+0.022% paired median CPS. The same-VM fused-image comparison is slightly
positive at +0.617% and +0.805%.

The old candidate medians reproduce within +0.165%/-0.120%. The reconstructed
accepted medians are 2.577%/2.364% below their retained different-day values.
That baseline-session drift explains the former negative sign; no overall VM,
decoder or fused-handler regression reproduces. The exact method, all 78 raw
runs, paired statistics, historical decomposition and retained binaries are in
[`rebaseline/`](rebaseline/README.md).

The remainder of this document retains the original first-verdict observations
and labels their superseded interpretation explicitly.

## Accepted-batch QA closeout (2026-07-19)

Broad QA is complete. Full Debug CTest passes 1,864/1,864 after an audited
110-golden refresh. The audit caught and corrected an unsafe AST-side typed
alias fusion and missing fused-call recognition in native signal restore
before accepting the generated output. The supported Apple ASan campaign
reports no sanitizer diagnostic; its single failure was a shared scratch-file
test race, corrected with a CTest resource lock and passing 2/2 ASan rerun.
Apple's ASan does not support leak detection, which remains an explicit
platform limitation.

An isolated 112-file Release install compiles and runs the shipped hello
example. Both installed VMs also pass the exact accepted pre-batch RXBIN and
library. This build has CMake install targets but no CPack/package target, so
the installed-tree proof is the applicable packaging result.

The final drift-controlled campaign passes all 78 executions. Complete-product
paired median CPS is +1.385% on `rxvm` and +2.868% on `rxbvm`; paired elapsed
direction agrees. The `rxvm` 95% interval crosses zero while the `rxbvm`
complete-product interval is wholly positive. This confirms no regression and
a positive final sample without overstating the noisy point estimates. See
[`qa-closeout/`](qa-closeout/README.md) and
[`finalrun01/`](finalrun01/README.md).

## Historical baseline used by the original verdict

The accepted timing baseline is
`../2026-07-17-nr-09-numctx-first-release-verdict/`: median 1,203,145 CPS for
`rxvm` and 1,183,390 CPS for `rxbvm`, each from three serial recorded samples
after one warmup. It is reused under the programme baseline rule; it is not
silently rerun.

The arbitrary-operand transport prerequisite is isolated in commit
`32bf7e76f`. Of the 67 selected mappings, 38 require more than three opcode
operands (11/16 Class 1 and 27/51 Class 2). Without that prerequisite, at
least all 11 wide Class 1 backstops and as many as 38 exact selected fusions
would be blocked or forced into descriptor/partial alternatives. The three
additional trace-preserving production forms in this batch also use the wide
transport.

The frozen minimum correctness gate passes the opcode-effects and compiler
combiner unit checks and 9/9 selected CTests. That selection includes fresh
optimized/no-opt benchmark generation and execution on both `rxvm` and
`rxbvm`, direct large-instruction execution on both VMs, every Class 1 RXAS
positive/no-opt/boundary contract, NR-06 compatibility and the accepted NR-09
numeric-context contract. The standalone documentation example prints
`answer=42`; RXAS and its reference agree on 600 forms and 388 unique
mnemonics, and the 60 new instruction source/database rows agree exactly.

## Original cross-session verdict (superseded)

The original unmatched-session comparison did not clear the first Release
gate. Static and dynamic work reduction was substantial, while its retained
figures appeared to regress consistently on both VM modes:

| VM | Accepted Rule 1 median CPS (range) | Batch median CPS (range) | CPS delta | Accepted/batch median elapsed | Elapsed delta |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 1,203,145 (1,194,385-1,213,159) | 1,176,279 (1,169,351-1,187,568) | **-2.233%** | 8.33 s / 8.52 s | **+2.281%** |
| `rxbvm` | 1,183,390 (1,176,634-1,189,443) | 1,156,303 (1,143,771-1,178,327) | **-2.289%** | 8.46 s / 8.67 s | **+2.482%** |

All eight candidate runs pass the canonical RexxCPS contract. Each VM has one
warmup followed by three serial recorded samples. Higher CPS and lower elapsed
are better. The medians and both independent elapsed measurements agree on a
roughly 2.3% regression.

## Old-RXBIN compatibility isolation

The newly built VMs were also run against the exact retained pre-batch NR-08
optimized and no-opt RXBINs and their matching old linked library. The four
smoke cells pass. A separate one-warmup/three-recorded comparison executes the
exact old timed product with the new VM binaries:

| VM | Old VM median CPS | New VM, old RXBIN median CPS | CPS delta | Old/new median elapsed | Elapsed delta |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 1,195,649 | 1,194,926 | -0.060% | 8.38 s / 8.38 s | +0.000% |
| `rxbvm` | 1,180,487 | 1,179,673 | -0.069% | 8.48 s / 8.49 s | +0.118% |

The smoke result remains valid. Its timing comparator was also cross-session,
so its performance interpretation was provisional. The rebaseline now supplies
the missing same-session control: current versus accepted VM on identical
accepted RXBIN/library is -0.152% paired median CPS on `rxvm` and +0.022% on
`rxbvm`, with uncertainty intervals crossing zero. The new VM and
arbitrary-operand infrastructure do not materially regress old bytecode.

## Exact work and size deltas

Across the retained 11-workload, optimized/no-opt portfolio (22 images), the
compiler-generated RXAS changes from 12,901 to 10,340 instructions: -2,561
(-19.851%). RXAS text changes from 1,345,576 to 1,318,913 bytes: -26,663
(-1.982%). All 22 candidate images assemble successfully. Sixteen new
mnemonics occur 1,616 times directly in generated RXAS; RXAS-only Class 1
backstops are applied during assembly and therefore are not miscounted as rxc
output.

Canonical optimized RexxCPS changes against the accepted Rule 1 product:

| Dimension | Rule 1 | Batch | Delta |
| --- | ---: | ---: | ---: |
| generated RXAS instructions | 1,837 | 1,702 | -135 |
| RXAS bytes | 226,493 | 225,512 | -981 |
| RXBIN bytes | 79,861 | 79,485 | -376 |
| linked library bytes | 880,384 | 868,912 | -11,472 |

The matched noncanonical-smoke schema-4 profiles execute 922,301 new large
instructions on each VM, using 28 of the 60 new forms. Total executed
instructions fall from 17,746,147 to 16,220,865 on `rxvm` and from 17,746,177
to 16,220,865 on `rxbvm`: -1,525,282/-1,525,312, or **-8.595%** in each case.
The largest retired old-op populations are `NULL`, attribute capacity/link
scaffolding, `UNLINK`, `SETTP`, `SWAP`, call and integer-copy operations.

Thus the result is not a failure to form the selected units: the product
retires materially fewer dispatches and is smaller. The original inference
that the fused handler mix costs more than its retired dispatches is
superseded. In the same-session control, the fused image is slightly faster
than the accepted image on the same current VM, although the uncertainty is
too large for a material-win claim. Per-form diagnostic reporting continues
below for relevance, coherence, temporary-register cost and implementation
quality, not to explain a reproduced overall slowdown.

## Rerunnable per-form timing and review report

`macro-timing-report/report.md` applies one recorded semantic expansion to
each of all 60 NR-09 forms and compares the candidate macro handler with the
global average/totals for its legacy component opcodes. It reports both raw
handler-only and transition-aware estimates for `rxvm` and `rxbvm`, preserves
the exact component rows in `component-timings.csv`, and emits the 60-form
decision surface in `review-ledger.tsv`. The review ledger also records
coherence shape, Class 1/Class 2 ownership, whether a compiler temporary is
elided or passed to the opcode, a raised review bar for incoherent or
side-effect-preserving forms, and pending coherence/implementation/decision
fields.

The canonical profile observes 28/60 forms. The expansion model estimates
1,525,298 retired dispatches in each VM, within +16/-14 of the exact observed
1,525,282/1,525,312 reductions. Twenty-one observed forms show an instrumented
saving signal on `rxvm` and 23 on `rxbvm`; seven/five show a possible slowdown,
but six observed forms have fewer than 100 calls. The first nontrivial timing
review candidates are:

- `SETTPCALL_REG_FUNC_REG_REG_INT`: 56,968 calls, estimated transition-aware
  delta +15.026% on `rxvm` and +6.676% on `rxbvm`;
- `SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG`: 10,100 calls, mixed +0.694% and
  -2.625%, a much smaller and VM-dependent signal.

Those percentages are profiler diagnostics, not product verdicts. Each
instruction timing includes per-instruction profiler bookkeeping, the timer
minimum is 41 ns while most short handlers average below it, and component and
transition averages are global rather than exact replaced call sites. Rows
must not be summed because legacy opcode populations overlap. Ordinary
profiling-off Release isolation remains decisive.

Rerun from the repository root with:

```sh
performance/tools/report_nr09_macro_timings.zsh \
  performance/evidence/2026-07-17-nr-09-numctx-first-release-verdict/profiles \
  performance/evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/profiles \
  performance/evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/macro-timing-report
```

The versioned form inventory and review fields are in
`performance/manifests/nr09-macro-review-v1.tsv`.

## Balanced per-form Release review and simplification proposal

The complete follow-on review is retained in [`balanced-review/`](balanced-review/README.md).
It expands the occurrence boundary from one canonical profile to the full
22-image optimized/no-opt portfolio and records 76,122,675 executions across
33/60 forms. Every exercised form has matched profiling-off Release evidence
in both VMs: 32 new 16-position cells with 192 recorded comparisons per
form/VM, plus the stronger existing `SETTPCALL` cell.

The resulting proposal is **30 outright removals, two clean replacements and
28 retained forms**. All six caller/trace-temporary forms are removed or
replaced: four have no sufficient benefit, while `FDIVSUB` and the wide
`ILOADSETUNLINKN` show double-digit isolated savings and therefore qualify for
redesign without the intermediate side effect. No production removal or
redesign has been made; the disposition is stopped for Adrian's approval.

## Profile-product and build notes

The ordinary and profiling builds produce byte-identical generated RXAS. Their
RXBIN hashes differ only because the assembler records the input RXAS path as
module/description metadata; normalized disassemblies are otherwise identical
with SHA-256
`946b87cddcf10fcfa3e31a3a685fdf1aeb69076e7334d56b138d0d179c977def`.
The profile counts therefore describe the timed instruction product.

The default broad profile-tree build encountered an existing
`test_rxvmprofile` harness link failure for missing module/graph/signature
symbols after the profiling product binaries had linked. The required
`rxc`/`rxas`/`rxvm`/`rxbvm` plus `bin/library.rxbin` target build completed,
and the exact profile contracts passed. The harness failure is retained as a
separate build-system issue; it is not counted as a batch correctness result.

Evidence is retained in `comparison.csv`, `candidate-summary.csv`,
`candidate-samples.csv`, `static-image-deltas.csv`,
`static-mnemonic-deltas.csv`, `dynamic-opcode-deltas.csv`,
`dynamic-new-instructions.csv`, `compatibility/`, `profiles/`, `raw/`,
`profile-raw/` and `rebaseline/`. The implementation remains provisional and
revertable. No
broad Debug CTest, sanitizer, package/install proof, golden refresh or
production-batch commit has run.
