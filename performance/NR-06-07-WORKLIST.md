# NR-06/NR-07 compiler fast-path batch worklist

Status: complete

Starting point: clean `develop` at `6e52ef872` on 2026-07-16. Revalidated
after the unrelated datatype-source hot-fix advanced `develop` to `5e5e3b397`
on 2026-07-17; the batch files and design assumptions were unaffected.

This is the resumable ledger for the approved first queued P0 batch. It keeps
the two compiler activities in one target-build loop while retaining separate
design selection, semantic proof and Release verdicts. A prototype result does
not make either activity complete.

## Batch rules

- Preserve the contiguous call-window ABI, signal unwind, references, TRACE
  source locations, public ABI and serialized RXBIN.
- Iterate with the existing Ninja trees and the `rxc` target only (`-j 32`).
- Use minimal compiler fixtures and generated-RXAS counts before any portfolio
  measurement.
- Reuse the NR-05 call census and RXSEQ evidence. Do not regenerate a full
  profiling bundle during PoC iteration.
- Once a production candidate has the minimum focused correctness proof,
  freeze it and run its mandatory profiling-off Release verdict. Report and
  stop before broad CTest or the next production candidate.

## NR-06 design selection

### Status quo

The register pass already assigns non-symbol expression results directly to
their final contiguous call-window slots. Named arguments and other preserved
sources are pointer-swapped into the window before a call and swapped back on
normal return. Repeated sources are snapshotted first. This preserves `.ref`,
alias and cold signal-unwind behavior but costs two `SWAP` instructions for
each ordinary displaced argument.

NR-05 retained 41,981,144 setup and 41,981,144 normal-restoration swaps over
16,439,398 instruction calls (5.107382 combined swaps per instruction call).

### Approach A — typed scalar copy into the existing window (selected PoC)

For a provided, non-optional, non-reference scalar argument whose effective
type is Boolean, integer or float, copy the typed payload into its already
reserved call-window slot. The caller's source remains unchanged, the call
still consumes the same contiguous window, and signal unwind sees the identity
mapping and therefore has nothing to restore. All other arguments retain the
existing snapshot/swap/restore path.

The machine-level ceiling is one `ICOPY`/`FCOPY` dispatch instead of one setup
plus one restoration `SWAP`. The PoC must measure instruction reduction and
unprofiled Release wall-clock behavior; dispatch count alone is not a win.

Risks and gates:

- `.ref`, optional, omitted, array, string, binary, decimal, object and
  reference values are excluded.
- Member receivers and dynamic/native calls remain safe through the same
  argument annotations and fallback path.
- Call result placement must remain outside the argument window.
- Both normal return and branch-style signal unwind fixtures must pass.
- Unrelated non-call-heavy cells must remain neutral within measurement noise.

### Approach B — affinity-guided register numbering and window placement (accepted 2026-07-17)

A uniform post-allocation permutation cannot help: applying the same bijection
to source registers and call-window registers preserves every equality and
therefore preserves every swap. The useful static form acts earlier. It orders
distinct named-symbol registers from call-site affinities, leaves/reserves an
argument-count slot, and selects a contiguous window whose occupied positions
are already the exact argument symbols expected at those positions.

The current allocator already contains part of this intended path: it tries to
assign an as-yet-unassigned reference/const symbol to its call slot, and its
bottom-up release logic preserves an exact symbol/window match. It currently
misses because procedure symbols are assigned first in symbol-table order and
`get_regs()` only reserves wholly free contiguous ranges.

This is analogous to calling-convention-aware copy coalescing with precoloured
argument positions or affinity-guided register allocation. cREXX's variable
window base and consecutive-register constraint make the layout a weighted
ordered-placement problem rather than a plain final renumbering. A bounded
form can keep every named storage location distinct and reserve only exact
symbol matches, so it does not require NR-08 liveness or merge live ranges.

The design still needs focused proof for argument-count/result disjointness,
repeated sources, references/aliases, nested calls, normal and signal unwind,
and any `.locals` growth caused by layout holes. It preserves the existing
call ABI and serialized RXBIN, but it is low-risk compiler allocation work, not
a risk-free textual rewrite.

Adrian approved this as the next NR-06 PoC after rejecting the specialist-loop
extension. The first bounded implementation will:

- preassign distinct procedure-local symbols seen at call sites into one or
  more `[free argument-count slot, ordered argument slots...]` groups;
- leave expression and duplicate-source positions free rather than merging any
  storage locations;
- let call-window allocation reuse a group only when every occupied slot is the
  exact argument symbol expected there and every other required slot is
  currently free; and
- fail closed to the existing wholly-free `get_regs()` window and swap/restore
  marshalling whenever the exact layout is unavailable.

This removes two existing `SWAP` dispatches for each exact placed argument and
adds no runtime instruction. The current contiguous-window ABI, unique symbol
storage, result remapping, repeated-source snapshot, reference flags and signal
unwind contract remain unchanged.

#### Alternative B1 — global weighted layout solver (deferred)

A whole-procedure graph/ordered-placement solver could score every overlapping
call affinity and minimize holes globally. It may ultimately place more sites,
but is unnecessary complexity until the exact-window mechanism proves that
real swaps and Release time fall. The first PoC uses deterministic lexical
groups and reports both swap reduction and `.locals` growth; it stops without a
Release build if that trade is not promising.

### Approach C — liveness-driven destructive final placement (deferred)

Move a named value into its final slot and omit restoration when it is provably
dead. This could cover more value shapes, but the current AST ordinals do not
prove loop backedges, aliases, reference targets, lexical `ENDLIFE`, or future
uses after exceptional transfer. A correct implementation requires the wider
CFG/lifetime facts queued under NR-08, so this is not a bounded NR-06 slice.

### Approach D — general parallel swap scheduler (fallback only)

Schedule arbitrary remaining permutations and repeated sources explicitly.
The current allocator already places expression temporaries in their final
slots, while preserved named sources normally sit outside the reserved window.
A scheduler remains valuable as a correctness boundary for future allocation
changes, but it does not remove the dominant setup-plus-restoration pair by
itself and is not the first performance PoC.

## NR-07 design selection

### Status quo

`rxc` materializes an integer Boolean comparison result and then emits
`BRT`/`BRF`. RXAS can rewrite a bounded subset to `BEQ`, `BNE`, `BLT`, `BLE`,
`BGT` or `BGE`, but only when its queue-local liveness proof can show the result
dead. RXAS has an explicit form of the rule that consumes the matching
operation trace event, but its 20-instruction queue cannot prove both paths for
most non-local targets.

### Approach A — compiler direct branch for direct integer conditions (selected PoC)

In optimized compilation only, lower a comparison directly when it is the
condition child of `IF`, `WHILE` or `UNTIL`, both operands are integer/Boolean,
and neither operand has deferred cleanup. Emit the existing branch opcode with
the correct true/false inversion and constant-side normalization. Keep
operand/source trace events and the authored `.srcstep`; omit only the Boolean
operation event because the optimized value no longer exists. The TRACE
contract explicitly permits events for optimized-away values to be absent.

The machine-level ceiling is one existing conditional-compare branch instead
of a comparison dispatch plus a Boolean branch dispatch. No VM, ISA or RXBIN
change is involved.

Risks and gates:

- No-opt compilation must retain the materialized Boolean and operation event.
- Float, decimal, string, binary, loose comparisons, Boolean values used
  outside a direct condition, and operands needing cleanup must fail closed to
  the existing path.
- Register-register, register-immediate and immediate-register relations must
  preserve direction and true/false inversion.
- IF/ELSE, WHILE and UNTIL behavior, source stepping, both VMs, and the RXAS
  fallback rules require focused coverage.

### Approach B — widen RXAS into general CFG/liveness lowering (deferred)

This retains a single assembler transform for compiler and handwritten RXAS,
but requires broader control-flow, metadata-observation and cleanup reasoning.
That belongs with NR-08/NR-18 rather than this bounded compiler fast path.

### Approach C — add new fused typed opcodes (rejected)

New float/string/decimal compare-branch forms would change the serialized ISA
and RXBIN surface. Existing integer branch opcodes cover the first decisive
case, so an ISA change is neither required nor approved.

## NR-07 specialist loop-instruction extension — rejected 2026-07-17

### Status quo

The loop emitter already selects `BCF` for a general `FOR` count. RXAS folds an
adjacent local-register `INC; BR loop` pair into `BCTP`, but rxc still emits
materialized `IGT`/`ILT`, `FGT`/`FLT` or `DGT`/`DLT` Boolean results for `TO`
termination checks. The VM also has `IGTBR`/`ILTBR`, `FGTBR`/`FLTBR`,
`DGTBR`/`DLTBR`, and two- and three-register `BCT`, `BCTNM` and `BCF` forms.

### Approach A — semantic loop selection in rxc (selected PoC, then rejected)

When optimized rxc has the required source facts, emit the existing specialist
instruction directly:

- known positive/negative `BY` direction plus local numeric control and bound
  registers selects the corresponding typed `GTBR`/`LTBR` `TO`-exit check;
- implicit integer `BY 1` plus a local control register selects `BCTP` for the
  back edge when no stronger counted-loop instruction applies; and
- a compile-time positive `FOR` count selects two-register `BCT`, or
  three-register `BCT` when it can also perform an implicit local integer
  control-variable increment.

The positive `FOR` count is a hidden retained temporary, so rxc can omit the
initial general `BCF` check and use the count directly at the back edge.
No-opt output and all uncertain cases retain the current sequence. This uses
only existing opcodes and does not change RXBIN, the public ABI or language
semantics.

The machine-level ceilings are one typed compare-and-branch dispatch instead
of compare materialization plus Boolean branch, one `BCTP` dispatch instead of
`INC` plus `BR`, and one three-register `BCT` dispatch instead of `BCF` plus
`BCTP` for a proved-positive counted loop.

### Approach B — reconstruct complete loops in RXAS (rejected for this batch)

RXAS should retain exact local peepholes and accept explicit specialist
instructions, but it does not know whether a loop variable is user-visible,
may be modified in the body, aliases another value, or has a statically
positive hidden count. Reconstructing that semantic state from the bounded
optimizer queue would require wider CFG/lifetime work and would still duplicate
facts already available in rxc.

### Approach C — precompute trip counts for general `TO` loops (deferred)

A derived trip count could use `BCT` more widely, but is not safe merely because
bounds and step are constant: cREXX permits the visible control variable to be
modified in the body, and float/decimal iteration depends on the normal numeric
operations. This requires stronger non-modification/alias facts and is outside
the bounded batch.

### Fail-closed boundary

- Runtime or non-positive `FOR` counts keep `BCF`; `BCTNM` is not substituted
  because its negative-count behaviour differs from the current sequence.
- Dynamic `BY`, non-local operands, unsupported value types, explicit
  increments, and any uncertain register shape keep the existing lowering.
- Three-register forms require distinct local integer count and control
  registers.
- `WHILE`, `UNTIL`, `LEAVE`, `ITERATE`, overflow signalling, final visible
  control values and source/TRACE metadata require focused proof.

### Release verdict and disposition

The implementation passed 34/34 focused Debug tests, the ordinary
profiling-off Release build, and 35/35 focused Release tests. A three-way
round-interleaved canonical RexxCPS comparison then ran the retained original,
direct-condition-only and full specialist-loop images with one warmup and five
recorded samples per image and VM. All 36 executions passed.

Full specialist loops versus direct-condition-only changed median CPS by
-0.099% in `rxvm` and -0.293% in `rxbvm`; elapsed time changed +0.072% and
+0.270%. Round-paired medians were -0.025% and -0.294% CPS. This is no
practical performance improvement.

Disassembly also showed that normal RXAS already folds all 12 RexxCPS
`INC; BR` pairs to `BCTP`. Direct rxc `BCTP` emission therefore changed source
RXAS but not the assembled product. Only 13 typed `TO` compare/branch pairs
and one positive-count back edge survived as product differences.

Adrian rejected the extension and directed that it be removed. The isolated
opcode ceilings remain retained evidence, but the ordinary product showed no
practical gain and does not justify the additional compiler complexity.
Evidence: `evidence/2026-07-17-nr-07-loop-release-verdict/`.

## Fast PoC and focused proof ledger

- [x] Retain baseline generated RXAS and instruction counts for minimal call
      and direct-condition fixtures.
- [x] Add structural compiler fixtures for NR-06 eligibility/fallback and
      NR-07 direct/fallback lowering.
- [x] Build only Debug `rxc` with Ninja `-j 32` after compiler edits.
- [x] Prove NR-06 normal calls, repeated arguments, `.ref`, optional arguments,
      member/dynamic calls and signal unwind with focused tests.
- [x] Prove NR-07 IF/ELSE, WHILE, UNTIL, constants, strict direction, fallback,
      generated source metadata and both-VM output.
- [x] Compare static instruction counts on retained high-signal portfolio
      images without running the full bundle.
- [x] Time-box ordinary profiling-off Release PoCs on the smallest decisive
      call-heavy and compare-heavy cells, serially in both VM modes.
- [x] Record chosen/rejected result and clean the first production candidate.
- [x] Run the first candidate's mandatory focused Release verdict and stop for
      Adrian.
- [x] On Adrian's direction, remove NR-06 scalar copying, restore the validated
      NR-07 lowering, run its focused Release verdict, and stop again.
- [x] Implement and causally audit the bounded NR-06 affinity-guided register
      placement, then retain it on Adrian's direction because it removes real
      static and executed swaps without adding runtime instructions.
- [x] Remove both rejected NR-07 compiler paths and their dedicated fixtures.
- [x] Accept the intentional optimized golden changes, rebuild the complete
      Debug product and pass the required broad Debug CTest gate.

## PoC results and first-candidate selection — 2026-07-17

The pre-change compiler is the existing profiling-off Release `rxc` at
`db94bc9ca`; `git log db94bc9ca..5e5e3b397 -- compiler tests/benchmarks`
is empty, so it is a valid compiler/codegen baseline for this batch. Raw
minimal-fixture RXAS remains in
`/tmp/crexx-nr0607-baseline2.sKwImL/`; raw candidate fixtures and dual-VM
output remain in `/tmp/crexx-nr0607-poc.UkUtAw/` and
`/tmp/crexx-nr07-relations.q73Cxz/` for the current decision cycle.

Minimal-fixture result:

- NR-06 optimized output changes 12 swaps to 6 swaps plus two `ICOPY`s and
  one `FCOPY`; no-opt remains at 12 swaps. Optional, string and `.ref` calls
  retain their setup/restoration pairs.
- NR-07 optimized output changes nine direct integer-condition compare/branch
  pairs to nine existing compare-branch opcodes. The retained Boolean and
  strict-string, float and string fallbacks remain materialized; no-opt has no
  direct compare branches.
- Both fixtures have identical opt/no-opt output in `rxvm` and `rxbvm`. The
  generated structural contract passes in 2.3 seconds.
- Twenty focused NR-06 tests pass, covering duplicate arguments, optional and
  reference fallbacks, method calls, both signal-unwind VMs, and dynamic/static
  native unwind. Their first CTest invocation also pulled the
  `linked_opt_runtime_artifacts` fixture and spent 143.76 seconds rebuilding
  it; subsequent PoC checks avoid that dependency and run the tools directly.

Static comparison of the 11 retained optimized portfolio sources, compiled
with the baseline and candidate compilers against the same Release imports:

| Workload | NR-06 swaps removed | NR-06 typed copies added | NR-07 pairs removed/direct branches added |
|---|---:|---:|---:|
| Sieve | 0 | 0 | 3 |
| Permute | 4 | 2 | 4 |
| Mandelbrot | 0 | 0 | 10 |
| Towers | 16 | 8 | 32 |
| Bounce | 0 | 0 | 3 |
| Storage | 2 | 1 | 5 |
| List | 4 | 2 | 6 |
| Richards | 34 | 17 | 43 |
| JSON | 0 | 0 | 3 |
| Base64 | 0 | 0 | 56 |
| RexxCPS | 0 | 0 | 118 |
| **Total** | **60** | **30** | **283** |

NR-06 is the first production candidate because its 30 static sites are
concentrated in the call-heavy cells for which NR-05 already retained exact
high dynamic counts. NR-07 is a valid bounded PoC with wider static coverage,
but its production lowering is removed while NR-06 receives an uncontaminated
first Release verdict. Its design, fixture and observed counts remain ready for
the next decision gate.

## NR-06 first Release verdict — 2026-07-17

The isolated NR-06 candidate passed the ordinary profiling-off full Release
build and the smallest decisive end-to-end Permute x50 comparison. Across nine
recorded serial samples per image and VM, `rxbvm` was neutral at -0.252%, while
`rxvm` was materially noisy/adverse at +3.369% median with candidate outliers
up to 126.484 ms. All samples passed correctness. Adrian rejected the scalar
copy experiment, and its production lowering was removed before the NR-07
verdict. The wider static numbering/window-placement idea remains open as the
separate Approach B above. Evidence:
`evidence/2026-07-17-nr-06-first-release-verdict/`.

## NR-06 affinity-guided register placement verdict — 2026-07-17

The bounded Approach B implementation preassigns distinct procedure-local
symbols into lexical call-affinity groups and adds exact-register reservation
for compatible contiguous call windows. It rolls the partial reservation back
and uses the existing wholly-free allocator whenever an occupied/free slot does
not match. Unique storage, repeated-source snapshots, result disjointness,
reference flags, signal unwind, the call ABI and serialized RXBIN remain
unchanged.

The strengthened minimal fixture reduces optimized `SWAP`s from 20 to two. It
also proves that a repeated-source call whose desired second slot is occupied
retains one snapshot plus the normal swap/restore fallback. No-opt remains at
20 swaps. Twenty-five focused Debug checks passed, covering duplicate
arguments, object/reference behavior, optional arguments, interface/method
calls, both signal-unwind VMs, and dynamic/static native unwind.

The exact retained Release baseline/candidate compilers then produced the
following 11-source static result:

- 423 to 365 `SWAP`s, removing 58;
- summed `.locals` 1,317 to 1,322, an increase of five; and
- RexxCPS 121 to 77 `SWAP`s while its summed `.locals` fell from 181 to 180.

The ordinary profiling-off full Release build and the same 25 focused Release
checks passed. Canonical RexxCPS then ran one warmup plus six order-balanced,
round-interleaved recorded samples per image and VM. All 28 executions passed,
but host throughput drifted heavily and the VM results split. Same-round paired
medians were -1.928% CPS/+1.962% elapsed for `rxvm` and +1.676% CPS/-1.647%
elapsed for `rxbvm`; individual paired CPS changes ranged from -7.299% to
+7.996%.

The static placement objective was achieved, but the mandatory first Release
gate did not demonstrate a reliable practical performance improvement. A
follow-up causal audit then verified the exact images, assembler output,
dynamic counts and direct instruction cost. All 44 removed RexxCPS swaps are
in its unexecuted trace handler, so both exact images execute 484,376 swaps.
Four other workloads remove 248,362 executed swaps at their retained argv, but
a 100,000,000-iteration profiling-off Release diagnostic measures only 0.434
ns per swap in `rxvm` and 0.706 ns in `rxbvm`. Their implied end-to-end
opportunity is 0.000108%--0.040474%, far below observed process noise.

The audit also found that numbering can affect subsequent RXAS choices:
Storage moves a loop counter to `r1`, causing the fixed-register `INC1` rule to
pre-empt the more valuable `INC`+`BR` to `BCTP` fold for 40 executions. This is
correct but demonstrates that renumbering is not mechanically isolated to call
swaps.

The causal result shows that this is not a material product-speed opportunity,
but it also verifies that the implementation removes real executed work and
adds no runtime instruction. Adrian therefore selected the bounded placement
for retention: reducing swaps is sufficient justification even though the
measurable end-to-end gain is very small. The implementation and its dedicated
tests remain in production; the exact audited patch, PoC fixtures, profiles,
timing cells and direct SWAP diagnostic remain in the evidence. The
constant/by-value flag and branch backlog remains separate. Evidence:
`evidence/2026-07-17-nr-06-register-affinity-verdict/`.

## NR-07 first Release verdict — 2026-07-17

The restored NR-07 lowering reproduced the earlier validated optimized and
no-opt fixture RXAS byte-for-byte, passed the structural contract, and matched
the focused golden output in both VMs. The ordinary profiling-off Release
build completed, then canonical default RexxCPS compared the retained baseline
module with the 118-site direct-branch module using one warmup and three
recorded serial samples per image/VM.

The candidate regressed both modes despite reversed order for the `rxbvm`
control. Median benchmark-native CPS fell 11.139% in `rxvm` (1,003,222 to
891,473) and 5.573% in `rxbvm` (805,410 to 760,524); median process elapsed
rose 12.551% and 5.898% respectively. All 16 executions passed the canonical
contract and correctness marker. The later specialist-loop comparison did not
reproduce the large direct-only regression, but also found no practical gain;
RXAS already performed the useful combined-loop fold. Adrian rejected both the
direct-condition lowering and specialist extension because no measured
performance improvement justified the added compiler complexity. Their
production code and dedicated test registrations are removed, while the exact
patches and evidence are retained. Evidence:
`evidence/2026-07-17-nr-07-first-release-verdict/`.

## Formal batch closeout — 2026-07-17

The accepted production boundary is NR-06 affinity-guided register numbering
and exact compatible call-window reservation only. NR-07 direct-condition and
specialist-loop lowering remain rejected and removed.

The retained NR-06 implementation rebuilt in both Debug and ordinary
profiling-off Release. The focused selection passed 26/26 in each tree,
including the linked-runtime fixture. The first
full Debug CTest exposed seven optimized compiler goldens whose register layout
changed intentionally; their corresponding runnable fixtures all passed and
the new output removes another 86 static swaps with every other opcode count
unchanged. The documented
`--update-gold` path accepted those seven optimized outputs, after which the
targeted compiler/runtime set passed 16/16. The final full Debug build passed
and the required `ctest --test-dir cmake-build-debug --parallel 30
--output-on-failure` gate passed 1,849/1,849 in 331.21 seconds.

Per the approved shortest closeout path, no sanitizer, install/package,
cross-platform or additional benchmark campaign was added. The decisive
timings, causal profiles, direct SWAP diagnostic, initial golden-failure log,
final build/test logs and checksum ledger are retained with the evidence.
