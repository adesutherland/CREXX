# PERF3-12A cursorless RXAS and copied-XTOY placement

Status: in progress — architecture approved; product baseline pending commit

Started: 2026-08-04

Purpose: remove string and binary cursors as logical register state, migrate
RXAS and its producers to explicit-position operations, and then use the
simplified component model to eliminate copied-XTOY temporaries. The retained
RexxCPS target is the exact `DCOPY temporary, source` plus `DTOS temporary`
shape identified by PERF3-12.

## Authority and stop boundaries

- Branch: `codex/perf3-rxas-flow-infrastructure`.
- Product baseline commit: `0db998bea` (`perf: refresh K04e Mac and RexxCPS
  evidence`). This worklist/roadmap commit is product-neutral and becomes the
  named implementation starting point.
- Adrian approved a deliberate breaking RXAS/RXBIN change on 2026-08-04.
  Compatibility with bytecode or assembler source using the removed cursor
  forms is not required.
- Retained count baseline: optimized RexxCPS at
  53,660,581/53,660,552 instructions under `rxvm`/`rxbvm`, including exactly
  2,220,000 `DCOPY` and `DTOS` executions and 97,680,000 decimal-copy bytes.
- Retained profiling-off Mac runtime authority: PERF3-06/K04e scorecard;
  accepted baseline evidence is not replaced merely because a new candidate
  is tested.
- Scope includes RXAS/RXBIN opcode contracts, VM value/runtime helpers, opcode
  metadata, RXAS flow analysis, RXC assembler validation/partial evaluation/
  inlining, bundled libraries, tests and current documentation. Language-level
  string and binary results remain unchanged.
- The dated programme charter and old evidence are historical. Update live
  roadmap/worklists and current reference/architecture documentation only.
- The cursorless product receives its own mandatory ordinary profiling-off
  Release verdict after minimum focused correctness. Stop there before broad
  closeout or copied-XTOY production work. Copied-XTOY then receives a separate
  first verdict before final combined closeout, commit and publication.
- Do not push until Adrian requests the final combined publish.

## Status quo

String values currently expose `string_pos` and `string_char_pos`; binary
values expose `binary_pos`. Public `set/get*pos` instructions and cursor-based
slice forms make that state observable. The same string fields are also used
as an implementation cache while converting a character index to a UTF byte
offset. This conflates program state with a private acceleration detail.

The cursor surface originated when RXAS instructions were effectively limited
to three operands. RXAS/RXBIN now support arbitrary signature lengths and the
VM already executes four- and wider-operand instructions. Current bundled
libraries nevertheless contain 105 `setstrpos` and 131 `substring`
occurrences; binary library code has two `setbinpos`/`bslice` pairs.

Consequences include extra dispatches, caller-visible mutation of otherwise
read-only sources, special copy/call/inlining rules, cursor read/write effects
through SSA/use analysis, and false barriers for component placement,
conversion elimination, hoisting and register reuse.

## Cursorless design selection

### Approach C1 — explicit-position operations and private cache (selected)

- Replace cursor-based slicing with explicit four-register forms equivalent to
  `substring destination,source,start,length` and
  `bslice destination,source,start,length`.
- Remove `setstrpos`, `getstrpos`, `setbinpos`, `getbinpos`, cursor-based
  `substr`/`substring`/`bslice`, and the two-register cursor-based `strchar`
  form from the supported instruction database. Reuse or reserve their fixed
  numeric opcode slots explicitly; never renumber unrelated instructions.
- Keep existing explicit indexed character, scan and binary-memory operations.
  Their internal UTF position work is not an observable register effect.
- Remove `binary_pos` completely. Replace `string_pos`/`string_char_pos` with
  clearly named VM-private UTF cache fields, or a narrower helper-owned cache
  representation selected during implementation. The cache may affect cost,
  never program results, copies, signals, TRACE, aliases or flow proofs.
- Remove cursor read/write fields and use kinds from canonical RXAS flow
  metadata once no opcode has a logical cursor contract.
- Migrate RXC, bundled libraries, fixtures and documentation to explicit
  positions.

This is the selected long-term design: ordinary values contain value
components, not hidden iteration state.

### Approach C2 — retain cursors but track them in SSA (rejected)

Correcting every cursor reset/read/write in metadata could make current
semantics provable, but it permanently adds a hidden state dimension to copies,
calls, aliases, conversions and every future optimization. It also preserves
the extra set-position dispatch. The earlier focused metadata PoC demonstrated
the cost of this direction and is deliberately excluded from the baseline.

### Approach C3 — compatibility translation or side table (rejected)

A loader/assembler could translate simple adjacent setter/slice pairs, while a
VM side table emulates arbitrary old cursor state. Arbitrary branches, calls,
copies, swaps and `get*pos` observations prevent complete local translation,
and a side table retains the same semantic burden with worse locality. Adrian
has approved a breaking change, so neither compatibility mechanism is needed.

## Cursorless implementation plan

### C0 — product-neutral baseline

- [x] Record the architecture decision, alternatives, retained evidence and
  stop boundaries.
- [x] Remove the exploratory conversion-cursor metadata/test PoC.
- [ ] Validate the product-neutral diff and commit it locally without pushing.

### C1 — opcode and value contract

- [ ] Inventory every cursor opcode, metadata record, VM field/helper,
  compiler/library emitter and documented/tested behavior.
- [ ] Define exact explicit string/binary slice contracts: zero-based position,
  length, UTF character versus byte unit, clipping, negative/range behavior,
  destination/source aliasing, allocation failure and signal phase/writes.
- [ ] Replace the cursor instruction entries without renumbering unrelated
  opcodes; update effects, components, signals and format validation.
- [ ] Delete `string_pos`, `string_char_pos`, and `binary_pos` from the value
  structure early. Treat resulting compiler errors as the mandatory inventory
  cross-check: each use must become an explicit operation, a clearly private
  UTF-cache use, or be removed.
- [ ] Add a repository fence proving the deleted logical field names and
  cursor opcodes are absent from production code and current documentation.

### C2 — VM and private UTF cache

- [ ] Implement explicit string and binary slicing in both VM dispatch modes,
  including same-storage operand cases and failure-visible-write semantics.
- [ ] Replace cursor-mutating UTF scans with a private cache/helper whose state
  is invalidated by string writes but is otherwise semantically invisible.
- [ ] Prove explicit indexed `strchar`, `concchar`, `hexchar`, `poschar`,
  `fndblnk`, `fndnblnk`, substring and related string operations no longer
  mutate observable source state.
- [ ] Remove binary cursor maintenance from allocation, resize, append, copy,
  conversion, I/O and cleanup paths.

### C3 — producers and consumers

- [ ] Migrate all bundled Level B and RXAS library sources to explicit-position
  operations, preserving RexxDoc blocks and tags.
- [ ] Update RXC assembler validation, partial-call evaluation, inlining/remap
  metadata and any direct emitter path. Remove cursor isolation/copy rules that
  no longer have a semantic purpose.
- [ ] Remove cursor capabilities, use-index entries and proof guards from RXAS
  flow analysis; retain no optimizer-visible private-cache state.
- [ ] Update focused assembler/runtime/compiler/library fixtures and regenerate
  only source-controlled derived artifacts required by normal builds.

### C4 — cursorless correctness and first Release verdict

- [ ] Focused positives: empty, ASCII, UTF-8, zero/past-end start, zero/large
  length, negative inputs, same-register aliases and allocation/signal paths
  under `rxvm` and `rxbvm`.
- [ ] Run opcode metadata, RXBIN round-trip/validation, VM string/binary,
  compiler partial-call/inlining, Level B library and RXAS optimizer floors.
- [ ] Build the ordinary profiling-off Release product immediately after the
  minimum gate.
- [ ] Measure exact static/dynamic removal of setter dispatches, paired
  RexxCPS under both VMs, Sieve and string/binary-heavy guards. Report and stop
  for Adrian before broad closeout or copied-XTOY production work.

## Copied-XTOY design selection

### Approach X1 — RXAS atomic component placement (selected)

After cursors are no longer logical state, add a sparse proof plan for an exact
typed copy followed by a one-register derived conversion. It proves the copied
input value unchanged, source result component unobserved, temporary input and
other components dead, all temporary result uses redirectable, storage local
and unaliased, and signal/context/call/reference/metadata/TRACE behavior
equivalent. The immutable transaction retargets XTOY to the source, redirects
result uses and deletes the typed copy.

This keeps reusable semantic ownership in RXAS and makes `DCOPY`/`DTOS` the
first measured case without benchmark-specific recognition.

### Approach X2 — RXC direct-source emission (control, not selected)

This is mechanically small but moves liveness, alias, component, cleanup,
signal and TRACE decisions into RXC. Retain only as a ceiling if RXAS cannot
express a demonstrated compiler-only fact.

### Approach X3 — two-register XTOY/runtime flags (rejected)

The VM register already contains independent value components. A new opcode or
runtime state adds dispatch/compatibility/steady-state cost without solving the
proof problem.

## Copied-XTOY implementation and verdict

- [ ] Add a distinct SSA/use-owned component-placement route, immutable proof
  plan, rejection reasons/metrics and atomic queue consumer.
- [ ] Prove the measured `DCOPY`/`DTOS` case plus bounded metadata-driven
  positives; add source-result, temporary-component, later-write, context,
  branch/phi, link/ref, call, signal-handler and TRACE negatives.
- [ ] Run focused metadata/signal/optimizer/runtime tests and inherited
  M01-M06/K04 floors.
- [ ] Build the ordinary Release product and compare exact static/fixed-work
  reductions, paired RexxCPS under both VMs, Sieve and a copy/string-heavy
  guard. Report and stop for Adrian before closeout.
- [ ] After both verdicts are accepted, run the proportionate broad closeout,
  update current docs/evidence/roadmap, review, commit locally, then publish
  only with Adrian's explicit direction.

## Acceptance contract

The breaking change removes obsolete RXAS/RXBIN source and bytecode behavior;
it must not change language-level string/binary results. Explicit-position
operations must be deterministic and free of source mutation. Private UTF
cache state must be unobservable and excluded from optimizer semantics.

The cursorless first verdict expects one setter-plus-slice pair to become one
slice dispatch and accepts neutral runtime only when correctness and the
architectural simplification are demonstrated without a material guard
regression. Copied-XTOY succeeds only when its exact `DCOPY` count falls by the
proved amount, `DTOS` work remains, both VMs agree, and target/guard runtime is
materially better or neutral within paired noise. Any regression, unexplained
instruction increase, incomplete field/opcode fence or semantic ambiguity
stops for analysis.
