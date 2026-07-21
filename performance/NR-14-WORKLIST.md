# NR-14 static/frozen PARSE lowering worklist

Status: complete; hybrid and artifact trade-off accepted 2026-07-21

This is the resumable control plane for NR-14. The objective is to replace
generic runtime interpretation of mechanically eligible frozen `PARSE` plans
with the fastest semantically complete execution model supported by comparative
evidence. Generic `parseExec` remains the complete fallback for dynamic,
unsupported and uncertain cases.

## Repository and scope record

- Branch: `develop`
- Starting HEAD: `8424587f258ac37f133adab4194a3e80a5ee0875`
- Starting `origin/develop`: `5626d6b871d740387765de40bfbebd246471102f`
- Starting dirty-tree scope: clean; local `develop` is intentionally two commits
  ahead of `origin/develop` and must not be reset to the remote.
- Historical charter: read-only
  `docs/planning/release-1/performance-programme-report-2026-07-15.md`
- Normative measurement policy: `performance/PERFORMANCE-GOVERNANCE.md`
- Live status: this worklist and `performance/ROADMAP.md`
- Publish boundary: Adrian authorized the local closeout commit; do not push.
- Approved design boundary: Adrian has authorized guarded opcode/RXBIN/linker/
  disassembler/VM PoCs and productionization of the fastest validated NR-14
  design after the comparison records its exact contract. Pause only for a
  source-language syntax change, public host ABI change, weakened semantics or
  architectural scope beyond the NR-14 PARSE implementation.

## Exit criterion

- [x] Specialize only mechanically recognized template/source/modifier classes.
- [x] Keep generic `parseExec` as the complete fallback.
- [x] Preserve result, error ordering, TRACE and source-coordinate behavior.
- [x] Demonstrate material gains on RexxCPS and at least one other applicable
      PARSE workload without a material unrelated-control regression.
- [x] Report `rxvm` and `rxbvm` separately and keep steady-state, startup/load,
      allocation/RSS and artifact-size costs separate.
- [x] Select the most efficient end-to-end algorithm from evidence; source-code,
      subsystem, ISA and file-format size are decision costs, not overriding
      reasons to reject a materially faster design. The selected hybrid retains
      D for exact and chainable common shapes and uses the compact prepared
      vector for remaining mechanically frozen plans. Adrian accepted the
      verdict and the compiler-exit/RXAS artifact trade-off.

## Gates and stop points

- [x] Present a numbered implementation plan before compiler/ISA/VM edits.
- [x] Verify branch, HEAD, upstream and dirty-tree scope.
- [x] Create this worklist before implementation.
- [x] Audit the authoritative PARSE pipeline and supported semantic surface.
- [x] Validate retained static/dynamic evidence revisions before reuse.
- [x] Define and measure machine-level ceilings for representative hot classes.
- [x] Compare design families A-E and both a general compiled-plan operation and
      exact hot primitives in guarded/disposable PoCs.
- [x] Run only target builds and focused tests during design iteration.
- [x] Record selected and rejected designs, including lifecycle, memory,
      artifact, compatibility and maintenance costs.
- [x] Record the selected production instruction/plan/RXBIN contract exactly.
- [x] Implement only the selected production slice.
- [x] Run minimum focused correctness, freeze implementation and build the
      ordinary profiling-off Release product.
- [x] Run the smallest decisive paired end-to-end Release verdict and stop for
      Adrian with ACCEPT, REVISE or REVERT.
- [x] Do not run broad CTest, sanitizer, packaging, cross-platform closeout,
      documentation polish or follow-on tuning before verdict acceptance.
- [x] Compare generic prepared output strategies on longer frozen templates,
      retain the fastest safe representation and record its exact contract.
- [x] Add general functional and performance coverage while preserving exact-D
      selection for the already-proven common shapes.
- [x] Freeze the hybrid after focused correctness and run its new smallest
      decisive profiling-off Release verdict before broad closeout.
- [x] Obtain Adrian's explicit acceptance of the performance verdict and
      +249,179 B/+20.579% compiler-exit artifact trade-off.
- [x] Remove disposable PoCs, document the new RXAS/RXBIN/VM surface, pass the
      broad Debug and supported sanitizer gates, and prove the isolated
      installed/native-package path before the approved local commit.

## Stage A - current pipeline and semantic inventory

### Compiler exit and frozen-plan path

- [x] Record certified-exit parsing and the exact frozen-plan encoding.
- [x] Record source selection for VALUE, VAR, ARG, PULL, SOURCE, LINEIN and
      VERSION, including evaluation order and failure behavior.
- [x] Record modifier lowering for UPPER, LOWER, CASELESS, TRIM and INTO.
- [x] Record target/result mapping, dropped fields/placeholders, repeated targets
      and source/target aliasing.
- [x] Compare optimized and no-opt emitted RXAS, RXBIN and linked images.
- [x] Record compiler-exit protocol, diagnostics and source metadata propagation.

Audit record:

- The certified exit compiles every accepted template item to the frozen
  length-prefixed stream `kind,length:text;`. Kinds are target, literal,
  absolute, relative-forward, relative-backward and implicit-word boundary.
- Plain expression, `VALUE`, `VAR` and `ARG` source construction are implemented.
  `ARG` concatenates the compatibility argument view once. `PULL`, `SOURCE`,
  `LINEIN` and `VERSION` are not source selectors in the authoritative exit and
  are therefore outside NR-14 eligibility.
- `UPPER`, `LOWER`, `LOG`, `TRACE`, `TRIM` and `INTO` are implemented;
  `CASELESS` is not. Exact three-site classification remains deliberately
  narrow. The hybrid can prepare or chain safe `UPPER`, `LOWER`, `TRIM` and
  `ARG` forms after a typed source copy. `LOG`, `TRACE` and explicit `INTO`
  retain `parseExec` because they expose contracts beyond compact target values.
- The exit copies/evaluates the source, then performs ordered target assignments.
  The prepared descriptor compacts dropped result slots, while explicit `INTO`
  keeps the original complete `parseExec` array. Exact/chained operations write
  internal temporaries; repeated and compound targets remain ordered caller-side
  assignments. This preserves source aliases and visible write order.
- Exit replacement fragments carry source steps and variable/assignment trace
  metadata. Direct opcode results are written to internal scalar temporaries;
  normal assignments remain the visible target writes.

### Generic runtime path

- [x] Map `parseExec(source, plan, template, debug)` control flow and allocations.
- [x] Separate runtime plan-text interpretation, descriptor traversal, string
      scans, name lookup, intermediate result-array construction and final
      assignment costs.
- [x] Record error ordering, TRACE/debug behavior and source coordinates.

`parseExec` decodes the complete `kind,length:text;` stream on every call with
two-item lookahead, allocates the result array and control arrays, executes the
generic state machine, returns the array and leaves the caller to link/copy/
unlink every selected result. There is no runtime variable-name lookup, but
target names still occupy the stream and are decoded even though execution only
needs positional result slots. Malformed-plan and debug diagnostics belong only
to this generic entry; compile-time-selected exact opcodes cannot encounter a
runtime plan error.

### Syntax and fallback matrix

- [x] Literal delimiters, including empty and unmatched delimiters.
- [x] Absolute, relative and variable positions.
- [x] Empty fields, placeholders and dropped fields.
- [x] Repeated targets and source/target aliasing.
- [x] Dynamic patterns and variable delimiters/positions.
- [x] Unicode/string behavior.
- [x] Unsupported or uncertain shapes mechanically retain `parseExec`.

The existing 140-case runtime suite covers literals, empty/unmatched fields,
positions and drops. Variable delimiter/position syntax is explicitly disabled
in the authoritative test source. The focused `parse_frozen` workload adds
repeated-target, source/target-alias, `TRIM`, `INTO` and Unicode boundaries. The
direct handler splits only on ASCII blank bytes; that is equivalent to the
current codepoint scan because UTF-8 continuation bytes cannot equal ASCII 32.

## Stage B - evidence reconstruction

- [x] Inventory PARSE sites and template classes in RexxCPS and the current
      portfolio, recording target counts, source sizes and repeated plan shapes.
- [x] Reproduce `parseExec` call, self/child time, instruction and allocation
      counts from retained exact-revision evidence or a new bounded diagnostic.
- [x] Record current RXAS, RXBIN, linked-image, startup/load and memory sizes.
- [x] Count eligible and fallback populations in optimized and no-opt modes.
- [x] Select at least one additional parse-heavy workload and one unrelated
      control without modifying canonical benchmark semantics.

RexxCPS has seven static PARSE sites and 98 calls per outer iteration: six
three-word shapes and one `target +5 target .` shape dynamically, with one of
the word shapes discarding a fourth/rest field. The current portfolio has no
other PARSE-bearing workload. Historical NR-05 evidence is implementation-valid
but source-revision historical: bounded 2.2c profiling recorded 294 calls,
`parseExec` optimized self time 11,612,747 ns (39,499 ns/call), and only about
0.5 us/call of call entry/exit overhead. A new `parse_frozen.crexx` focused
workload provides the second applicable lane; AWFY Richards is the unrelated
control. The completed production gate is retained at
`evidence/2026-07-20-nr-14-first-release-verdict/`. Candidate RexxCPS optimized
RXAS contains seven direct sites and no PARSE fallback sites: five
`parsewords3`, one `parsewords3d` and one `parsepos2`. Candidate focused PARSE
contains nine direct sites and five deliberate generic fallback statements in
both optimized and no-opt output. The exact artifact inventory records every
formal RXAS, RXBIN, stripped linked image and product hash/size.

## Stage C - machine-level ceilings and guarded designs

The ceiling for an eligible frozen class executes a precompiled immutable plan
without runtime plan-text parsing, name lookup, generic descriptor traversal or
avoidable allocation. Measure exact hot primitives as well as the integrated
candidate so dispatch and orchestration costs remain visible.

| ID | Guarded design | Required comparison |
| --- | --- | --- |
| A | Compiler-exit expansion into existing operations and direct assignments | generated instruction/register pressure, intermediate allocation, code/image size and both-VM execution |
| B | Faster specialized `parseExec` selected by a preclassified plan | call/frame cost, plan access, allocation, branch count and fallback boundary |
| C | One general runtime-assist instruction over a compact immutable plan/descriptor | dispatch, validation, serialization, relocation, load preparation, memory and image size |
| D | Runtime instruction/family writing directly to explicit result/target registers where alias/status semantics permit | operand width, direct-write ceiling, repeated/aliased-target fallback and register effects |
| E | Link/load-time preparation into a private execution representation versus canonical serialized RXBIN assistance | eager/lazy/narrow preparation, ownership, failure, late load, startup, retained memory and canonical compatibility |

- [x] Compare one general compiled-plan instruction with a small exact family of
      common primitives; do not assume fewer opcodes or operands is faster.
- [x] Keep every PoC default-off or isolated, disposable and clearly
      non-production until selection.

Guarded six-site comparison, 50,000 repetitions, six balanced serial rounds,
ordinary profiling-off Release build (`/tmp/nr14-poc-compare.QMQmqE`):

| VM | generic | A existing ops | B specialized Level B | C array opcode | D direct opcode |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` median seconds | 0.739547491 | 0.106342435 | 0.096851230 | 0.035807014 | **0.030827641** |
| `rxbvm` median seconds | 0.757522464 | 0.108442426 | 0.098516941 | 0.036481142 | **0.029353976** |

A used existing `word`/`subword` operations and direct scalar observation. B
used one preclassified Level B scan returning an array. C used the same native
scan but materialized the intermediate result array. D wrote explicit scalar
result registers. A second six-round 100,000-repetition comparison including
ordinary result copies (the production TRACE/assignment shape) measured D-copy
at 0.050172567/0.048336029 seconds versus C-array at
0.055396438/0.054565072 (`rxvm`/`rxbvm`).

A general descriptor opcode and link/load-prepared E representation were first
considered against the exact benchmark population. Exact D remains faster for
those sites. Adrian then required general frozen coverage, so the descriptor
candidate was re-evaluated on a seven-target template. The reusable prepared
vector materially beat `parseExec`; chaining exact D was faster again where
tail semantics permit. A link/load private cache remains unnecessary because
the canonical descriptor is already directly executable.

## Runtime-assist contract checklist

Selected production hybrid contract:

- Opcode 405 `PARSEWORDS3_REG_REG_REG_REG`, RXAS `parsewords3 R,R,R,R`:
  write the first two implicit blank-delimited words and the remaining tail to
  operands 1-3 from source operand 4.
- Opcode 408 `PARSEPOS2_REG_REG_REG_INT`, RXAS `parsepos2 R,R,R,I`: write the
  fixed character prefix of length operand 4 and the next implicit word to
  operands 1-2 from source operand 3; the following drop field is implicit.
- Opcode 409 `PARSEWORDS3D_REG_REG_REG_REG`, RXAS `parsewords3d R,R,R,R`:
  write the first three implicit words and discard the remaining tail.
- Opcode 410 `PARSEPLAN_REG_REG_STRING`, RXAS `parseplan R,R,S`: populate a
  reusable compact result vector in operand 1 from copied source operand 2 and
  immutable prepared descriptor operand 3.
- All four are `FLOW_NEXT`, optimizer barriers, read only their source
  register, write/kill explicit result registers and are opaque/may-throw in
  effects metadata. Operands may be wide RXBIN 007 varints.
- RXBIN 007 feature bit 1 (`RXBIN007_FEATURE_FROZEN_PARSE`) is mandatory when
  any selected opcode is serialized. Unknown bits and an opcode missing its
  bit fail closed. No new section, relocation or byte-order-sensitive payload
  exists; the existing register/int encoding, linker propagation and generated
  disassembler apply unchanged.
- Exact opcodes carry their complete plan in the opcode identity. Opcode 410's
  version-1 descriptor records target store/drop flags, UTF-8 literal byte and
  character lengths, fixed-width numeric movements, item count and compact
  result count. It is little-endian, validated in bounds and contains no target
  names or runtime decimal plan text. There is no private load state or cache.
- Compiler eligibility is fail-closed. Exact shapes keep the direct opcodes;
  longer odd implicit templates chain opcode 405; remaining safe frozen plans
  use opcode 410. `_source` is evaluated/copied first and normal ordered
  assignments preserve repeated/compound writes, aliases, `TRIM` and metadata.
  Logging/TRACE, explicit `INTO` and unsupported forms call `parseExec`.

For each applicable C/D/E candidate record and test:

- [x] canonical opcode name, numeric identity, operands and effects metadata;
- [x] immutable plan/descriptor ownership, lifetime and validation;
- [x] portable serialization and byte-order handling: existing RXBIN 007
      register/string-constant encoding plus the versioned little-endian
      descriptor payload;
- [x] assembler/disassembler round-trip and linker relocation/propagation;
- [x] RXBIN feature/version compatibility and fail-closed behavior;
- [x] profiling and RXSEQ visibility through canonical opcode identity and the
      ordinary `START_INSTRUCTION` path;
- [x] signal, TRACE and source-coordinate behavior;
- [x] load/startup, retained memory and artifact-size cost; and
- [x] equivalent behavior in `rxvm` and `rxbvm`.

## Design-selection record

Status quo: the compiler exit freezes a plan and calls generic
`parseExec(source, plan, template, debug)`. This is the baseline, not a candidate
for retention merely because it minimizes change.

| Design | Correctness/coverage | Steady-state | Lifecycle/memory/size | Disposition |
| --- | --- | --- | --- | --- |
| A | exact three-word PoC only; broader expansion would multiply code | 6.9-7.1x faster than generic, but slower than B-D | repeated existing helper scans and larger emitted code | rejected |
| B | exact three-word specialized entry; generic fallback intact | 7.6-7.8x faster than generic | retains call, result array and caller mapping | rejected |
| C | compact prepared descriptor with reusable result vector | about 7.2-7.3x faster than `parseExec` on the seven-target PoC | vector attributes are reused; no private load cache | **selected for general frozen plans** |
| D | exact three-word, three-word-plus-drop and positional hot classes; fail-closed fallback | fastest raw and fastest with ordinary assignments | no plan state/allocation; three canonical opcodes plus RXBIN feature bit | **selected** |
| E | can cover broader static plans after link/load classification | cannot beat D body for the current fully classified population | adds load work, retained descriptor state and lookup | rejected for NR-14 |

Selected production design: hybrid D+C. D remains the exact direct-result
family and chain primitive; C supplies generic mechanically frozen coverage.

### Approved generic extension (2026-07-21)

Adrian accepted the exact primitives as the right specialization for the
observed common shapes but required NR-14 to remain generic rather than being
defined by the benchmark population. The production target is now a hybrid:

- retain D for exact three-word, three-word-plus-drop and positional shapes;
- add one compact immutable prepared-plan operation for longer and otherwise
  general mechanically frozen templates;
- compile the existing kind/text stream once into a portable descriptor, then
  execute it without runtime decimal/length parsing or target-name decoding;
- keep ordered caller-side assignments so repeated targets, compound targets,
  source/target aliasing, `TRIM` and source metadata retain their established
  boundary; and
- retain `parseExec` for logging/TRACE or any dynamic, malformed, unsupported
  or uncertain form.

The generic output representation was selected by a guarded comparison. A
reusable compiler-owned result vector populated by one descriptor-walking VM
instruction avoids a variable-arity opcode family and amortizes vector storage
across loop iterations. The exact chain remains faster where its tail semantics
fit. A private link/load cache is unnecessary because the canonical compact
descriptor is already directly executable.
Regex substitution is explicitly outside scope until a real regex requirement
exists.

### Hybrid selection and first Release verdict

The six-round 300,000-repetition guarded comparison measured medians of
1.685/1.645 s for `parseExec`, 0.230/0.230 s for the prepared vector, and
0.060/0.060 s for the exact chain (`rxvm`/`rxbvm`). This selects the prepared
vector for general templates and exact chaining for eligible odd implicit-word
templates.

The retained first Release verdict is
`evidence/2026-07-21-nr-14-hybrid-first-release-verdict/`:

- generic frozen elapsed: -90.757%/-90.744%, all 12 pairs favorable;
- exact frozen elapsed: -97.262%/-97.423%, all 12 pairs favorable;
- RexxCPS CPS: +45.249%/+45.499%, all 12 pairs favorable;
- byte-identical Richards at the 36-pair cap: -0.193%/-0.020% median with both
  mean intervals spanning zero; and
- one-repetition lifecycle, RSS and footprint favorable on both VMs.

The verdict recommends ACCEPT but requires Adrian's explicit artifact decision:
`rxcexits.rxbin` is +249,179 B (+20.579%), and both measured RXAS artifacts
cross the 5%/4 KiB guard. The shipped VMs are below +2%, while the generic and
RexxCPS stripped linked images shrink 64.681% and 6.534% respectively.

## Measurement contract

- Ordinary profiling-off Release products and identical exact inputs.
- RexxCPS, at least one additional applicable PARSE workload and one unrelated
  control; both VMs reported separately.
- Correctness before timing; optimized/no-opt and fallback boundaries covered.
- Serial, balanced/interleaved paired samples. Prototype comparisons may use a
  bounded pilot; the production decision follows the governance minimum unless
  the mandatory first-verdict rule explicitly reuses valid retained baseline
  evidence for the smallest decisive gate.
- Benchmark-native throughput remains separate from process elapsed time.
- Startup/load, steady-state, allocation/RSS and artifact size remain separate.
- Retain raw samples, hashes, exact commands, host/build state and all negative,
  noisy or neutral outcomes.

## Activity log

- 2026-07-20: Started from clean local `develop` at `8424587f2`, intentionally
  two commits ahead of `origin/develop` at `5626d6b87`. Adrian authorized the
  full A-E comparison, including guarded runtime-assist RXAS/RXBIN/linker/VM
  designs, and selection by end-to-end performance rather than smallest change.
  No production design was selected at this point.
- 2026-07-20: Completed the pipeline audit and guarded A-E comparison. Direct
  explicit scalar results beat the array opcode by 9-11% after including
  ordinary result assignments and by 14-20% at the raw handler ceiling. Selected
  D and removed the discarded array opcode from the production slice. Added the
  focused `parse_frozen` workload and began minimum focused validation; no broad
  CTest, sanitizer, packaging, commit or push has been run.
- 2026-07-20: Froze the selected D implementation after 11/11 focused Debug
  checks passed, then built the ordinary profiling-off Release candidate and
  repeated the same focused 11/11 gate. Twelve paired canonical RexxCPS rounds
  improved native CPS by a 45.965%/45.826% median (`rxvm`/`rxbvm`), favorable
  in all pairs. The focused frozen-PARSE workload reduced elapsed time by
  97.248%/97.445%, also favorable in all pairs. The byte-identical Richards
  control is neutral/noisy on `rxvm` at the 36-pair cap and slightly favorable
  on `rxbvm`; lifecycle below 5 ms is noisy/inconclusive, and RSS is slightly
  lower. `rxcexits.rxbin` grows 131,483 bytes (10.859%) and crosses the artifact
  guard, while the stripped linked RexxCPS image shrinks 13,674 bytes (6.534%).
  First verdict: **ACCEPT recommended**, with the artifact-size trade-off
  explicitly escalated to Adrian. Evidence:
  `evidence/2026-07-20-nr-14-first-release-verdict/`. Implementation remains
  provisional; no broad closeout, commit or push has run.
- 2026-07-20: Final source review found that relative `+0` has a non-advancing
  generic word-capture rule and must not enter the positive-position primitive.
  Narrowed direct eligibility to splits greater than zero and added the focused
  fallback assertion. The final Debug and Release focused gates still pass
  11/11; all three rebuilt workload RXAS/RXBIN/stripped linked candidate images
  are byte-identical to the measured versions, so the first-verdict timings and
  interpretation are unchanged.
- 2026-07-21: Adrian agreed with retaining D for the common exact shapes but
  reopened NR-14 against its generality requirement. Approved functional and
  performance tests plus implementation of a generic compact prepared-plan
  path, with exact opcode selection preserved wherever faster. Regex remains
  out of scope. The previous first-verdict bundle remains valid comparison
  evidence; the hybrid now requires its own focused correctness and ordinary
  profiling-off Release verdict before final approval.
- 2026-07-21: Implemented opcode 410 and compact descriptor execution, exact
  chaining for eligible longer odd word templates, prepared lowering for the
  remaining safe frozen surface, and explicit logging/TRACE/INTO fallback.
  Focused Debug and Release gates pass 13/13. The new first Release verdict is
  favorable on both PARSE lanes and RexxCPS and neutral at the capped Richards
  control; at that gate it awaited explicit approval of the compiler-exit/RXAS
  size guard.
- 2026-07-21: Adrian accepted the verdict and explicitly approved the artifact
  guard, full closeout and local commit without push. Removed the disposable
  PoCs and documented all four instructions. Focused Debug/ASan pass 15/15,
  full Debug passes 1,876/1,876, and the supported Apple ASan product build is
  clean; LSan is unavailable on this platform. The ordinary Release build,
  131-file isolated install, installed native `hello`, and installed dual-VM
  generic frozen-PARSE smoke all pass. Final closeout evidence is under
  `evidence/2026-07-21-nr-14-hybrid-first-release-verdict/qa-closeout/`.
