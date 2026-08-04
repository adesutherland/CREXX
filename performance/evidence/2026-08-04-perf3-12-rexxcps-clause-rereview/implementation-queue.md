# PERF3-12 implementation queue

This queue orders implementation risk and dependency, not only raw machine
ceiling.  Every item requires a new worklist/design-selection section, focused
correctness, an ordinary profiling-off Release product, the smallest decisive
paired target/guard panel, and a stop for Adrian's verdict.

## PERF3-12A — copied XTOY temporary elimination (`R12-C01`)

Recommended first.

### Status quo

RXC emits a typed copy to protect the source storage, then a one-register XTOY
conversion on the temporary.  RXAS understands the conversion's input/output
components and same-register derivation, but has no transaction that moves the
conversion to the source component and redirects the temporary's uses.

### Approaches to compare

1. Selected PoC direction: an RXAS atomic component-placement plan proves the
   source input component unchanged, the source result component/cursor
   unobserved, all temporary result uses redirectable, and temporary cleanup
   irrelevant.  It retargets XTOY to the source, redirects uses, and deletes
   the typed copy in one epoch.
2. RXC emits directly into the source.  This is mechanically small but moves
   liveness, alias, component, TRACE and cleanup decisions back into the
   compiler; retain only as a ceiling/control.
3. Add a two-register XTOY or runtime representation flag.  The current value
   already holds multiple components, so neither is needed for the measured
   shape; reject unless the atomic existing-opcode PoC fails for a demonstrated
   semantic reason.

### Minimum correctness gate

- positive `ICOPY`/`FCOPY`/`SCOPY`/`DCOPY` followed by applicable XTOY cases;
- source result component already live, cursor live, temporary other component
  live, later source write, numeric/plugin context change, link/reference,
  call window, branch/phi, signal handler and TRACE negatives;
- existing M01-M06/K04 floors and metadata tests;
- exact optimized/no-opt RexxCPS outputs under both VMs.

### First Release verdict

Confirm exact static and fixed-work dynamic `DCOPY` reduction, then run paired
ordinary Release RexxCPS under `rxvm` and `rxbvm`.  Use Sieve plus at least one
copy/string-heavy workload as no-candidate/regression guards.  Stop before
generalizing another XTOY family or polishing documentation.

## PERF3-12B — compound-tail representation panel (`R12-S01+H01`)

### Status quo

RXC constant-folds `key1` into joined-tail concatenations.  RXAS sees ordinary
CONCAT followed by opaque stem operations.  Existing segmented stem opcodes
are not selected, and no production consumer requests loop analysis.

### Approaches to compare

1. Segmented native-stem route: prove an exact `left || "." || right` key used
   only by one `STEMGET`/`STEMSET`, provide stable segment registers, and emit
   existing `STEMGET2`/`STEMSET2`.
2. Loop-scoped first-use value reuse: materialize the joined tail once for an
   operand generation and redirect later equivalent uses until either operand,
   numeric context, storage mapping or observation invalidates it.
3. Simple RXC tail hoisting is a control.  Do not select it unless compiler-only
   knowledge is necessary and can be expressed more simply than preserving
   the relevant provenance for RXAS.

### Minimum correctness gate

Cover empty/dotted/non-ASCII segments, invalid UTF-8, hit/miss/insert/update,
stem generations/defaults, aliased source/value/result, loop zero/one/many,
operand mutation, branches, calls, signal handlers and TRACE.  Measure both VM
modes and assembler memory.  Do not combine both candidate routes until each
has an independent ceiling and verdict.

### First Release verdict

Compare exact instruction, string-copy/buffer and joined-key reductions, then
paired ordinary Release RexxCPS with stem and non-stem guards.  Stop for route
selection even if both candidates are correct.

## PERF3-12C — transactional direct-destination PARSE (`R12-P01`)

Largest ceiling; starts only after a signal-contract gate.

### Required stages

1. Audit the three frozen PARSE VM handlers and replace `RXSC_UNKNOWN` with the
   exact conditional signal/failure-write contract.  Source/result aliasing and
   snapshot allocation are the critical distinction.  This metadata correction
   is not permission to change language signal semantics.
2. Generalize M06-style producer forwarding to an immutable multi-result plan.
   Retarget only destinations whose temporary values and cursors have no other
   observations; validate repeated physical registers and source aliasing.
3. Forward a copied PARSE source only when source cursor/value observations and
   failure behavior agree.
4. Delete temporary initialization only after the same transaction proves
   every temporary and metadata/TRACE observation disappears.
5. Compact register numbers after, not during, semantic rewriting.

### Alternatives

- A simple RXC emitter that parses directly to variables is a ceiling but is
  unsafe as a general replacement while PARSE can signal before assignment.
- A new runtime atomic PARSE form is a fallback only if conditional RXAS proof
  cannot cover common source/result-disjoint cases.

### First Release verdict

Require exact optimized/no-opt semantics for positional, word, dropped-tail,
empty, repeated-target, source-alias, handler and TRACE cases under both VMs.
Then confirm the expected SCOPY/NULLN and `.locals` reductions and run paired
ordinary Release RexxCPS plus general PARSE and no-candidate guards.  Stop
before inlining or further parse-plan work.

## PERF3-12D — late inlining and register finalization (`R12-I01+R01`)

After 12A-12C, refresh the actual `cps_subroutine` body, call/frame counts,
image size and assembler memory.  Apply the existing `AT04` comparison:
current early RXC inlining, late RXAS inlining, and hybrid semantic eligibility
from RXC with cost/placement in RXAS.  Decide whether analysis occurs before,
after, or in two bounded inlining epochs.

Register-number compaction is a linear final pass preserving parameters, call
windows, links, references, unwind, TRACE, metadata and `.locals`.  Register
reuse remains a separate interference/liveness consumer.  Neither activity may
recreate a dense procedure-by-register analysis or hide a scalability failure.
