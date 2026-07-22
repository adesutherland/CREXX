# NR-12 / bounded NR-21 call-convention worklist

Status: NR-21 accepted and complete; NR-12 deferred pending flow analysis

This is the resumable control plane for the ordered call-convention batch:

1. audit the existing convention and retained NR-05/NR-06 evidence;
2. compare and guard-prototype direct fixed-arity calls that name explicit
   argument registers for arities zero through four;
3. inspect the NR-12 protected-input opportunity before implementation;
4. audit inline output separately against equivalent manually written logic.

The first production Release verdict belongs only to the primary
explicit-register call design. Protected-input and inlining production changes
must not be bundled into it.

## Repository and scope record

- Branch: `develop`
- Starting HEAD: `5626d6b871d740387765de40bfbebd246471102f`
- Starting `origin/develop`: `5626d6b871d740387765de40bfbebd246471102f`
- Starting dirty-tree scope: clean
- Historical charter: read-only
  `docs/planning/release-1/performance-programme-report-2026-07-15.md`
- Live status: this worklist and `performance/ROADMAP.md`
- Publish boundary: do not commit or push without Adrian's explicit request.
- Architecture boundary: a public opcode, serialized RXBIN, ABI, or
  architectural change requires Adrian's explicit selection before production
  implementation. Adrian approved the permanent fixed-call forms and explicit
  RXBIN feature policy on 2026-07-20.

## Gates and stop points

- [x] Present a numbered plan before compiler, assembler, ISA, or VM edits.
- [x] Verify the actual branch, HEAD, upstream SHA, and dirty-tree scope.
- [x] Reproduce the retained NR-05 arity totals from the raw summary CSV.
- [x] Complete the current call/register/flag contract audit.
- [x] Compare status quo, fixed-arity explicit-register, and descriptor forms.
- [x] Define the smallest hot-path machine-level ceiling.
- [x] Keep the PoC default-off and visibly non-production.
- [x] Inspect protected-input RXAS before any NR-12 implementation.
- [x] Compare non-inline, inline, and manually written call-site logic.
- [x] Record the recommended first production design and required approval.
- [x] Obtain Adrian's explicit architecture/RXBIN selection.
- [x] After the approved production edit, run only minimum focused correctness,
      freeze implementation, perform the mandatory ordinary-Release verdict,
      record ACCEPT, and stop for Adrian's direction.
- [x] Obtain Adrian's acceptance of the Release verdict and approval for QA,
      documentation audit, closeout and a local commit.
- [x] Rebuild the complete Debug product, refresh only intentional compiler
      goldens, and pass the focused 17/17 plus final broad 1,871/1,871 CTests.
- [x] Audit compiler, RXAS, RXBIN and VM documentation while retaining the
      language-visible argument semantics and NR-12 deferral.

## Reproduced NR-05 arity evidence

Source:
`performance/evidence/2026-07-16-nr-05-call-census/summary/call-census.csv`.
These are dynamic counts for the retained bounded 22-image census, not a timing
baseline.

| Arity | Calls |
| ---: | ---: |
| 0 | 56,829 |
| 1 | 8,767,984 |
| 2 | 2,222,712 |
| 3 | 2,477,091 |
| 4 | 1,434,707 |
| 5 | 10,088 |
| 7 | 9 |
| 13 | 1,310,000 |
| 14 | 160,000 |
| **Total** | **16,439,420** |

Arity zero through four totals 14,959,323 calls: 90.996659% overall,
87.792028% in optimized images, and 92.868653% in unoptimized images. JSON is
the deliberate fallback/non-regression workload: only 10,002 of its 1,490,002
observed calls are arity zero through four, or 0.671%.

Keep the following populations separate throughout the investigation:

| Mechanic | Dynamic count |
| --- | ---: |
| setup swaps | 41,981,144 |
| normal restoration swaps | 41,981,144 |
| attributable defensive argument copies | 675,554 |
| unclassified copies | 13,603,218 |
| local `RET_REG` moves | 4,517,546 |
| non-local `RET_REG` copies | 8,591,365 |

## Stage A - current convention audit

### Call selection and caller marshalling

- A compiler-emitted direct call currently uses
  `CALL_REG_FUNC_REG(result, procedure, count_register)`. Even arity zero is
  normally lowered through a loaded count register, although RXAS/VM already
  have the two-operand zero-argument `CALL_REG_FUNC` form.
- Concrete local procedures use their direct symbol. Concrete methods,
  factories and matches use their mangled direct symbol; the receiver is the
  first actual (`a1` in the callee).
- Interface member/factory calls first emit `SRCMETHODSEL` or `SRCFPROCSEL` and
  then `DCALL_REG_REG_REG(result, selected_procedure, count_register)`.
- Direct native calls share the counted direct-call opcode. The VM recognizes
  a native target at runtime, calls it without a child bytecode frame and passes
  a pointer vector rooted at the contiguous caller window.
- Argument expressions are evaluated before marshalling. The compiler loads
  arity into `node->additional_registers`; the argument window starts at the
  following caller local and has one contiguous slot per actual.
- A primary actual is pointer-swapped into its final window slot. A repeated
  actual is not a permutation, so every non-primary occurrence is copied into
  its own final window slot before status setup.
- The last direct-call preparation may be fused as `SWAPCALL`, `SETTPCALL` or
  `SETTPSWAPCALL`. Other required swaps/status operations remain standalone.
  Normal return emits the reverse swaps. The compiler maps the call result
  through this permutation so restoration leaves it in the requested result
  register.
- Accepted NR-06 affinity tries to place eligible optimized actuals directly in
  the exact call window. It falls back to the existing free-window choice;
  no-opt retains the ordinary path. The retained NR-06 verdict also showed that
  affinity can alter fixed-register selection, so that collateral must remain
  visible in any follow-on allocator change.

### Callee and frame contract

- `frame_f()` allocates or recycles the ordinary frame and owns `a0`, whose
  integer value is the arity. Bytecode argument registers do not own storage:
  `a1...aN` are pointer-bound to the caller window values.
- A never-written by-value formal is marked by `mark_const_args()` and remains
  bound directly to its incoming `aN` register in optimized and no-opt output.
- A writable scalar by-value formal receives an eager typed copy such as
  `ICOPY local,aN`; subsequent writes target the private local.
- A writable string, binary, array, object or explicit-reference value branches
  on `REGTP_NOTSYM`: symbol-backed inputs are copied, while non-symbol
  temporaries can be swapped/reused. This is distinct from `.ref` aliasing.
- `.ref` / `ARG expose` intentionally retains the incoming `aN` binding and
  therefore allows writes to reach caller-visible state.
- Optional formals test `REGTP_VAL`. Omitted formals build the default and clear
  status; supplied writable formals are isolated. `REGTP_VAL` presence and
  `REGTP_NOTSYM` temporary reuse are separate meanings and remain required.
- Recursion and nested calls use independent allocated/recycled frames. Nested
  argument expression output is complete before the enclosing window is
  marshalled.
- A return writes through the frame's saved result pointer. `RET_REG` moves a
  true local value but copies a non-local argument/global/linked value. The
  retained 4,517,546 local moves and 8,591,365 non-local copies are return
  populations, not argument-copy opportunities.
- A bytecode child records `caller_arg_base`. Signal unwind uses it to restore
  the caller's pointer permutation. An interrupted native call has no child
  frame, so the cold handler decodes the canonical counted/fused call operand
  to recover the window.
- Inlined calls have no runtime frame, call opcode or call-window restoration,
  but current inline binding can still materialize formal/result scaffolding;
  that is assessed separately in Stage E.

## Stage B - design comparison

### A. Status quo plus accepted NR-06 affinity

- Operands: result, procedure/selected procedure, count register; arity is the
  runtime integer in that register and arguments follow it contiguously.
- Advantages: existing direct/dynamic/native behavior, RXBIN compatibility,
  compact call opcode, mature signal restoration and zero new public surface.
- Costs: count load, setup/restoration pointer swaps or allocator pressure,
  repeated-actual snapshots, and a call-window constraint at every call. The
  frame still performs one `aN` pointer binding per actual.

### B. Fixed-arity explicit-register direct calls

- Proposed production shape: direct bytecode forms for arities zero through
  four. Operands are `result, procedure, arg1...argN`; arity is encoded by the
  opcode/form. Arity greater than four and unsupported call kinds keep A.
- The callee contract does not change. The VM captures the named caller value
  pointers before frame activation and binds those pointers as ordinary
  `a1...aN`; the procedure cannot tell which call form was used.
- Required status flags are expected. The PoC preserves current `SETTP`
  behavior as standalone instructions. It falls back when a repeated physical
  register would need independent per-formal status states.
- No hot-path allocation, search, name lookup or descriptor traversal is added.
  `frame_f()` and its allocation/reuse behavior remain unchanged. The PoC adds
  at most four caller-pointer captures and retains the same one-pointer-store
  per callee argument.
- Expanded execution-image ceiling, including the opcode cell:

  | Arity | Current clean counted call | Explicit form | Cell saving before swaps/status |
  | ---: | ---: | ---: | ---: |
  | 0 | 7 | 3 | 4 |
  | 1 | 7 | 4 | 3 |
  | 2 | 7 | 5 | 2 |
  | 3 | 7 | 6 | 1 |
  | 4 | 7 | 7 | 0 |

  Each removed standalone setup or restoration `SWAP` saves one dispatch and
  three execution-image cells. A repeated-actual snapshot similarly costs one
  dispatch and three cells. Fused current forms need their whole encoding
  compared rather than being counted as separate dispatches.
- Assembler, linker and disassembler formats, effects metadata, profiler call
  census and RXSEQ opcode handling all need the new forms. The guarded PoC
  proves those generated tables can represent them.
- `rxvm` and `rxbvm` use the same C handler source; the latter's non-threaded
  dispatcher sees the same semantic work. Private expanded image size grows
  with operand count in both.
- A production serialized opcode is not backward-neutral. A guard-enabled VM
  reads current images; the current default VM rejects a guarded image as an
  RXBIN 007 container-validation failure. Production therefore needs an
  explicit opcode/RXBIN feature or version policy rather than silently emitting
  a new meaning under the same compatibility claim.

### C. Descriptor/vector mapping

- Candidate shape: result, procedure and a descriptor/index whose immutable
  data contains arity plus caller-register indices. The frame can still bind
  `aN` directly without value copies.
- It may reduce repeated call-site operands for arity three/four or permit one
  general mapped form, but it introduces a descriptor lookup and traversal on
  every hot call unless the loader rewrites the instruction to a private direct
  pointer. It also adds descriptor ownership, validation, relocation, code/data
  locality, late-load invalidation and native/dynamic mapping questions.
- A canonical descriptor needs new RXBIN data and old-image rules; a private
  loader-built descriptor still needs canonical operands from which to build it
  and does not remove per-argument pointer binding.
- Signal unwind must retain descriptor lifetime or materialize a restorable map.
  Profiler/RXSEQ tooling must dereference the descriptor to recover arity and
  operands.
- Disposition: not preferred for the first direct-bytecode boundary. The fixed
  forms cover 90.997% of observed calls with no hot indirection and materially
  reduce dispatch in the guarded PoC. Revisit descriptors for dynamic/native or
  higher-arity mapped calls only after the primary result is accepted.

## Stage C - guarded PoC result

### Guard and supported surface

- `CREXX_NR21_EXPLICIT_CALL_POC` defaults `OFF` and emits a configure warning
  when enabled. With the guard off, opcode IDs 401-404 remain reserved and the
  normal product is unchanged.
- Under the guard, IDs 401-404 temporarily define `CALLX1...CALLX4` with
  `RPR...` formats and exact read/write/call/may-throw effects. Arity zero uses
  the already-existing two-operand direct call.
- The compiler selects only locally defined direct bytecode procedures,
  concrete methods/factories/matches and arity zero through four. Interface,
  dynamic, imported/native, higher-arity and ambiguous repeated-status sites
  retain the current path. A hand-authored guarded call to a native target
  fails closed as not implemented; the guarded compiler cannot emit it.
- Existing status values are written before the explicit call. `REGTP_VAL`,
  `REGTP_NOTSYM`, optional/default and `.ref` semantics are unchanged.

### Static 11-source portfolio

The portfolio comprises the retained benchmark sources and was compiled and
assembled in optimized and no-opt modes. Disposable images remain outside the
repository.

| Metric | Baseline opt | Candidate opt | Baseline no-opt | Candidate no-opt |
| --- | ---: | ---: | ---: | ---: |
| explicit fixed call sites | 0 | 74 | 0 | 132 |
| static executable instructions | 6,530 | 6,268 | 3,803 | 3,470 |
| RXAS bytes | 861,512 | 857,337 | 457,478 | 451,709 |
| RXBIN bytes | 304,937 | 304,305 | 192,080 | 191,488 |
| standalone `swap` sites | 308 | 91 | 446 | 174 |
| `swapcall` sites | 27 | 4 | 41 | 8 |
| `settpswapcall` sites | 30 | 10 | 104 | 51 |
| procedures / sum of `.locals` | 81 / 1,347 | 81 / 1,347 | 97 / 971 | 97 / 971 |
| maximum `.locals` / highest `rN` | 108 / 107 | 108 / 107 | 107 / 106 | 107 / 106 |

Static executable instructions fall 4.01% opt and 8.76% no-opt. Aggregate
RXBIN falls 632 bytes (0.207%) opt and 592 bytes (0.308%) no-opt despite the
temporary high opcode IDs requiring larger varints. Outside call/count/status/
swap lowering, opcode counts are identical. NR-06 affinity was deliberately
left unchanged, so `.locals`, maximum register and fixed-register instruction
selection are identical. This is evidence for retaining affinity unchanged in
the first production comparison; narrowing it is a later isolated question.

### Executed dispatch evidence

One correctness/profile run used the same guarded profiling VM for baseline and
candidate images. Counts are identical in `rxvm` and `rxbvm`; the table shows
one copy. Existing status setup was preserved, including extra standalone
`SETTP` dispatch where a baseline fused it into a call. Total executed copy
counts remained identical in every row.

| Mode | Workload | Executed explicit calls | Dispatch delta | Relative delta |
| --- | --- | ---: | ---: | ---: |
| opt | List | 43,959 | -34,790 | -6.415% |
| opt | Permute | 8,659 | -8,659 | -3.017% |
| opt | Towers | 16,395 | -24,583 | -2.995% |
| opt | Richards | 119,154 | -139,270 | -1.491% |
| opt | Storage | 9,557 | -4,097 | -0.642% |
| opt | JSON | 0 | 0 | 0.000% |
| no-opt | List | 43,959 | -35,503 | -6.504% |
| no-opt | Permute | 18,739 | -28,818 | -8.747% |
| no-opt | Towers | 73,760 | -81,976 | -8.256% |
| no-opt | Richards | 469,631 | -564,133 | -5.754% |
| no-opt | Storage | 19,115 | -13,657 | -2.006% |
| no-opt | JSON | 0 | 0 | 0.000% |

The candidate eliminates the call-window swap path at eligible sites. Residual
swaps are non-call work or fallback calls. The profile's call-window attribution
does not model the internal pointer exchange of fused call opcodes as a
standalone setup/restoration instruction, so the robust comparison is the
executed opcode/dispatch delta above plus the retained NR-05 exact semantic
swap census; those populations must not be conflated.

### Bounded profiling-off Release timing

This five-recorded-run rotating PoC capture is architecture-selection evidence,
not the mandatory production verdict. Baseline and candidate used the same
guarded, profiling-off Release VM binary; only the linked image differed.

| Workload | `rxvm` median delta | `rxbvm` median delta | Reading |
| --- | ---: | ---: | --- |
| List | -6.331% | -5.119% | clear positive; one noisy `rxbvm` sample |
| Permute | -3.255% | -2.433% | clear positive |
| Towers | -0.015% | +0.062% | neutral |
| Richards | +0.125% | +0.168% | neutral / tiny negative |
| Storage | -0.161% | -0.488% | neutral / tiny positive |
| JSON | -0.932% | +1.796% | unchanged bytecode; drift/noise control |

A separate 20-run very-short startup/load probe was noisy and requested a
rerun: guarded candidate versus baseline medians were +4.04% for `rxvm` and
-0.56% for `rxbvm`. No startup conclusion is justified. The aggregate portfolio
RXBIN size decreased; a tiny NR-06 fixture changed from 6,931 to 6,939 bytes opt
and 6,955 to 6,947 bytes no-opt, demonstrating that opcode-varint/compression
effects can dominate very small images.

### Correctness and tooling checks

- Default-off focused build passed for `rxc`, `rxas`, `rxlink`, `rxvm`, `rxbvm`
  and opcode-effects metadata; guarded equivalents also passed.
- A guarded 11-source portfolio passed 44/44 linked smoke cells: opt/no-opt,
  both VMs. A baseline/candidate profiling matrix passed 48/48 cells.
- A focused arity fixture emitted the existing zero-argument direct form,
  `CALLX1...CALLX4`, then retained counted `CALL` for arities 5, 13 and 14.
  Baseline/candidate output matched in both VMs.
- NR-06 coverage matched in both VMs and modes, including repeated scalar
  actuals, `.ref`, optional supplied/omitted and writable string status.
- Richards supplied real result/argument overlap sites in both modes; linked
  output passed in both VMs.
- A protected-input fixture covering read-only, unconditional/conditional/loop
  writes, onward value/reference, optional and large values matched baseline
  output in both VMs.
- A non-inlined explicit one-argument call that raises a signal unwound to its
  caller correctly in both VMs without a call-window restoration.
- Assemble, link, disassemble, reassemble and execute round-trip passed for all
  fixed arities. Guarded opcode effects are fully classified.
- A guarded VM reads current images. A current default VM/disassembler rejects
  guarded serialized images with a generic RXBIN 007 validation failure, which
  confirms the need for an explicit production compatibility policy.

## Approved production boundary

Adrian approved the recommended fixed-call design and explicit RXBIN
compatibility policy on 2026-07-20. The provisional production implementation
is deliberately limited to:

- permanent `CALL1...CALL4` direct-bytecode forms at opcode IDs 401-404;
- the existing two-operand `CALL` for arity zero and counted `CALL` fallback for
  arity greater than four, imported/native, dynamic and unsupported sites;
- unchanged callee-visible `a1...aN`, frame allocation/recycling and argument
  status semantics;
- RXBIN 007 feature bit 0, `RXBIN007_FEATURE_FIXED_CALLS`, inferred by the
  writer and enforced by the reader. Zero-feature 007 images remain readable;
  fixed-call opcodes without the bit and unknown bits are rejected precisely;
  and older zero-only readers reject a call-bearing image;
- unchanged NR-06 affinity, NR-12 protected-input behavior and inline lowering.

The PoC-only build guard and `CALLX` names are removed. Focused contracts cover
all fixed arities, the zero/five boundary, both VMs, linker propagation,
assembler/disassembler round-trip and positive/negative feature handling.
The implementation remained provisional and revertable until the mandatory
ordinary profiling-off Release verdict was reported and accepted.

## First production Release verdict - ACCEPT

The frozen production implementation passed the minimum focused Debug CTest
set 17/17 and the ordinary Release pre-timing smoke matrix 16/16. A fresh clean
baseline at starting commit `5626d6b87` and the provisional candidate used
equivalent CMake 4.3.2 / Ninja / AppleClang 21 Release configurations with
profiling disabled. The checked-in Level B runner performed one warmup plus 12
recorded serial, workload-rotated rounds per cell.

Paired candidate-versus-baseline median deltas, where negative is faster:

| Workload | `rxvm` | `rxbvm` | Reading |
| --- | ---: | ---: | --- |
| List | -6.015255% | -5.784389% | clear positive |
| Permute | -3.848622% | -3.223565% | clear positive |
| Richards | +0.024715% | -0.332876% | neutral |
| JSON | -0.904035% | -1.983448% | high-arity control positive/no regression |

Every timing cell reported `rerun_recommended=no`. Candidate linked images
were smaller for all four workloads: List -144 bytes, Permute -40, Richards
-232 and JSON -88. RXBIN feature compatibility also passed the production
boundary: the new product reads old zero-feature images, old zero-only readers
reject call-bearing images, and missing or unknown feature bits fail precisely.

Verdict: **ACCEPT**. The call-heavy target workloads improve in both VMs,
Richards is neutral, the unrelated high-arity control does not regress, and
artifact size falls. Exact evidence is retained at
`performance/evidence/2026-07-20-nr-21-first-release-verdict/`.

This was the mandatory first-Release stop. Adrian accepted the verdict and
approved the shortest QA/documentation closeout on 2026-07-20.

## Accepted QA closeout

- The complete Debug product rebuilt successfully, followed by the focused
  NR-21/NR-06/argument/signal/RXBIN acceptance set at 17/17.
- The first broad CTest run passed 1,775 tests and exposed 96 compiler golden
  mismatches. Every failure was generated-RXAS drift from the accepted call
  lowering; runtime counterparts remained green.
- The 96 goldens were refreshed through the documented
  `crexx_test_driver --update-gold` path. The mechanical audit found 500 added
  and 933 removed RXAS lines: additions were fixed/zero-argument calls,
  preserved standalone `SETTP`, and four expected unfused `SETLINKATTR1`
  forms; removals were counted-call loads/calls, call-window swaps/fusions,
  their four `SETLINKILOAD` combinations, and one redundant repeated-scalar
  snapshot. There was no `.locals`, metadata, source/TRACE, or unrelated
  opcode drift.
- Final full Debug CTest passed 1,871/1,871 with
  `ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure`.
- Documentation now covers compiler selection/fallback and callee-visible
  invariants in addition to the RXAS, RXBIN and VM contracts. The language
  reference remains unchanged because pass-by-value, `.ref`, optional and
  argument-register semantics did not change.
- The decisive Release evidence remains the accepted 12-round paired capture;
  no valid baseline or timing campaign was repeated.
- Per the approved shortest closeout path, no sanitizer, cross-platform,
  install/package, alternate-layout or follow-on PoC work was added.

Closeout evidence is retained in
`performance/evidence/2026-07-20-nr-21-first-release-verdict/qa-closeout/`.
NR-21 is complete. NR-12 remains deferred for the forthcoming flow analysis;
inline formal/result simplification remains a separate future activity.

## Stage D - protected-input inspection

Disposition: inspection complete; implementation deferred to the forthcoming
flow analysis. No NR-12 compiler change belongs in the primary batch.

Representative generated RXAS established:

- read-only scalar by-value: metadata binds the formal directly to `a1`; no
  defensive copy in opt or no-opt;
- assigned before read: `ICOPY r0,a1` is immediately overwritten by `LOAD r0,7`;
  this one copy is genuinely redundant and is a future DSE opportunity;
- read then assign: the eager `ICOPY` is required; moving it to first write
  saves no executed work;
- conditional write and join: current output eagerly copies once. Copy-on-write
  can avoid it on the false path but needs path-correct binding/merge state;
- loop write: avoiding a copy for a zero-iteration loop requires a preheader or
  first-iteration state and can add comparable control flow;
- onward by value: a read-only formal stays on `aN`; no defensive copy exists;
- onward by reference: the outer by-value formal is copied before its private
  local is intentionally exposed, so delaying the copy saves nothing;
- optional supplied/omitted: presence branching/default construction and
  status clearing are already required. An unconditional later write suggests
  DSE, not removal of `REGTP_VAL`;
- writable string/binary/array/object: the prologue branches on
  `REGTP_NOTSYM`, copying symbol-backed values or swapping/reusing temporaries.
  Immediate overwrite can make that prologue redundant, but general delayed
  isolation needs flow-sensitive proof;
- caller status work is often already fused as `SETTPCALL` or
  `SETTPSWAPCALL`. Removing one flag can shorten a combined instruction without
  removing a dispatch. The explicit-call PoC deliberately preserved status as
  standalone work and still reduced total dispatch.

Retained dynamic defensive-copy counts are 552,693 optimized and 122,861
no-opt. The opt/no-opt excess is 429,832 executions; generated RXAS ties the
dominant 252,000 Permute population to inline formal scaffolding, not a real
callee copy. This is a bound, not a complete provenance reclassification.
Storage contributes 95,570 opt and 95,560 no-opt, while Towers contributes
204,920 opt and 27,300 no-opt. A new protected-input implementation should wait
for flow analysis that can classify first read/write and control-flow joins,
rather than adding an isolated ad-hoc state machine now.

## Stage E - initial inlining audit

A dynamic-value fixture compared the same `value = value` helper logic:

| Form | Static executable instructions | Total `COPY` / `ICOPY` sites | Main `.locals` | RXAS bytes |
| --- | ---: | ---: | ---: | ---: |
| ordinary no-opt call | 18 | 2 | 5 | 2,360 |
| compiler-inlined opt | 16 | 4 | 5 | 2,116 |
| manually written opt | 13 | 2 | 4 | 1,704 |

The inlined form correctly removes the count load, call, setup/restoration and
runtime frame. Compared with manual logic it retains two extra `ICOPY`
instructions, one extra branch, one register and 412 RXAS bytes for formal and
result block-expression scaffolding. A literal variant amplified the same
effect: manual logic constant-folded to three instructions, while the inlined
form retained nine because the scaffolding blocked equivalent simplification.

This is a clear separate follow-on opportunity, and it explains a substantial
part of the optimized defensive-copy census. It is not bundled into the primary
call-convention change. Writable inline formals, `.ref`, optional/default,
repeated actuals and joins remain correctness gates for any later simplifier.

## Focused correctness matrix

- [x] arities zero through four
- [x] arity five and JSON-style arity 13/14 fallback
- [x] repeated actual register
- [x] argument/result overlap
- [x] read-only and writable by-value
- [x] `.ref`
- [x] optional supplied and omitted
- [x] conditional writes
- [x] recursion and nested calls
- [x] inline paths inspected separately
- [x] signal unwind
- [x] optimized and no-opt
- [x] old/current RXBIN compatibility behavior

## Decision record

Recommended first production design: fixed direct-bytecode call forms for
arities zero through four, with the existing counted contiguous-window path as
fallback. Preserve current status semantics and standalone `SETTP` in the first
production comparison; retain NR-06 affinity unchanged so allocator collateral
does not contaminate the opcode verdict. Do not add dynamic/native mapping,
descriptor traversal, protected-input changes, inline simplification or a new
allocator heuristic to that verdict.

Approved 2026-07-20: fixed direct-bytecode calls for arities one through four
at IDs 401-404, plus enforced RXBIN 007 feature bit 0. The permanent mnemonics
are `CALL1...CALL4`; the existing zero-argument and counted fallback forms
remain. The frozen production implementation completed the mandatory first
Release gate with an ACCEPT verdict against a fresh clean starting-commit
baseline. Adrian accepted that verdict on 2026-07-20; formal QA and
documentation closeout passed, completing NR-21. NR-12 remains deferred.
