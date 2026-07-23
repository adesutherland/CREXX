# NR-18 post-NR-27 safe RXAS flow-harvest worklist

Status: complete
Started: 2026-07-22
Completed: 2026-07-22
Branch: `develop`
Starting HEAD: `65ea6b9e292b5335565ec63d86c6a40e5f35853b`
Starting upstream: `origin/develop` at
`25cbca679bcee9dcc8413def4341a49044e1ff66`; local `develop` is one accepted
NR-27 commit ahead
Starting worktree: clean

## Objective and boundaries

Process the separate post-NR-27 opportunity review as one bounded panel before
formal timing:

1. backward producer-to-destination forwarding for proved typed copies;
2. exact procedure-local jump-table successors so existing whole-procedure
   facts need not reject an otherwise known CFG; and
3. an exit-criterion audit of the narrow NR-12 fresh-local by-value case.

The correctness gate is mathematical semantic equivalence plus a strict
executable-instruction reduction for every accepted rewrite. Static opportunity
counts are ceilings until the exact predicate accepts a site. Retained dynamic
evidence may rank candidates, but the consolidated ordinary profiling-off
Release verdict starts only after the full panel is frozen.

No language, RXAS syntax, serialized RXBIN, opcode, public ABI or VM architecture
change is authorized. `ENDLIFE`, source/TRACE observations, metadata, signal
behavior, typed views, reference/alias lifetime and ownership remain unchanged.
Unknown facts reject the affected candidate. `-n` continues to bypass all RXAS
optimization.

## Baseline and prior-result audit

- [x] Re-read root and performance instructions, live roadmap, historical
  Strand 4, RXAS architecture and compiler argument-copy architecture.
- [x] Verify branch, exact HEAD/upstream relationship and clean worktree.
- [x] Reconcile NR-12 with accepted NR-26 rather than duplicating it. NR-26 F2
  already suppresses a private scalar by-value entry copy when every path to
  first read has a safe write; its focused fixture removes three copies. The
  retained 19-image portfolio found no F2 footprint, so no new compiler edit is
  selected here. Conditional/zero-trip/read-before-write cases correctly retain
  the copy.
- [x] Reproduce the post-NR-27 19-image static opportunity census from the
  ordinary Release product.
- [x] Retain exact focused before-images assembled by the accepted NR-27
  assembler for instruction and runtime comparisons.

## Design selection

### Destination forwarding

#### A. Keep forward copy propagation only

The accepted NR-27 pass redirects uses of a copied destination to the source
when one must-available equality reaches every use. It cannot remove a result
copy at a join when the producer temporary is the disposable value.

Disposition: retained as the existing first direction, but insufficient for the
reviewed opportunity.

#### B. Retarget one proved producer destination, then remove its typed copy

For `producer temp,...; typed-copy final,temp`, replace only the producer's
destination operand with `final` and remove the copy when typed-view liveness,
effects and observations prove that the two machine states are equivalent. The
rule owns no new instruction and every success reduces the executable count by
one. Rebuild the flow graph and all facts after each rewrite.

Disposition: selected for the bounded first implementation. Begin with
classified, non-throwing, non-barrier, single-destination integer/float producers
whose written view exactly matches the following copy. Reject metadata/TRACE
between the producer and copy, a live temporary view after the copy, an observed
or live final view before the copy, implicit effects, alias/reference/lifetime
semantics, physical-register overlap and any ownership/view mismatch.

#### C. General predecessor/SSA-style destination coalescing

Retarget multiple definitions across joins or introduce value versions and phi
lowering. This can cover more branch-result scaffolding but adds simultaneous
rename, exceptional-edge and metadata obligations before the one-producer rule
is measured.

Disposition: deferred. Reconsider only if B proves a material retained ceiling
that is blocked specifically by multiple equivalent producers.

### Jump-table control flow

#### A. Continue procedure-wide fail-close

Safe but excludes every instruction in a procedure containing a packed indirect
jump, even when its procedure-local `.jtable` and `.jcase` targets are complete.

Disposition: retained for unresolved, external, malformed or unsupported
tables, but too conservative for known local tables.

#### B. Resolve successors from existing `.jtable`/`.jcase` records

Associate each packed jump with the declared procedure-local table and add every
case target plus the architecturally required miss/fallthrough edge. The table
declarations already exist in the RXAS queue; no syntax or RXBIN change is
needed. A missing, duplicate, cross-procedure or inconsistent record keeps the
whole procedure fail-closed.

Disposition: selected, subject to a focused source/assembler audit that proves
the exact miss behavior and queue ownership before editing.

#### C. Treat every instruction as a possible indirect target

This is conservative for liveness but destroys meaningful reachability and must
facts, so it cannot admit the reviewed instruction-removing transforms.

Disposition: rejected.

## Candidate panel

| ID | Candidate | Gate | Status |
| --- | --- | --- | --- |
| H1 | Non-throwing typed producer destination forwarding | exact written view; temporary dead after copy; final view dead/unobserved before copy; no intervening executable or metadata/TRACE observation; no implicit/alias/reference/lifetime/opaque effect | accepted by focused proof; no retained 19-image footprint |
| H2 | Throwing, allocating, string/decimal/binary, call or alias-producing destination forwarding | H1 plus identical exceptional partial state, allocation/ownership/cursor/cleanup and observation behavior | deferred pending a concrete closed proof |
| J1 | Complete local `.jtable`/`.jcase` CFG successors | exact cases and miss edge; all records owned by one procedure; malformed/unknown tables fail closed | accepted; exact 19-image delta -298 |
| C1 | NR-12 fresh-local scalar entry-copy removal | existing NR-26 F2 predicates and an actual current portfolio reduction | implemented already; current portfolio footprint absent, audit only |

Every row will finish as accepted, rejected or deferred with exact static and,
where executed, bounded dynamic instruction evidence. J1 is an analysis enabler;
it receives no performance credit unless an admitted rewrite removes an
instruction in a newly analyzable procedure.

## Focused proof matrix

- [x] H1 integer/float positive cases, including a branch-result consumer.
- [x] H1 temporary-live, final-live, same-register and typed-view negatives.
- [x] H1 source/TRACE/register-metadata and asynchronous handler negatives.
- [x] H1 may-throw, call, implicit, reference, alias and lifetime barriers.
- [x] J1 complete one-case/multi-case tables and miss/fallthrough behavior.
- [x] J1 missing/duplicate/cross-procedure/unsupported-table fail-close.
- [x] Ordinary hand-written RXAS and compiler-generated RXAS coverage.
- [x] Optimized versus `-n` structural comparison.
- [x] Runnable focused fixtures produce identical output and status in `rxvm`
  and `rxbvm`.

## Frozen panel evidence

- The exact accepted-NR-27 versus NR-18 19-image census is **45,476 ->
  45,178 (-298)** with no image growth. Accounting changes from 979 to 1,273
  unreachable removals and from 14 to 18 typed-copy removals; H1 has no current
  retained portfolio footprint, so J1 supplies the production reduction.
- The focused hand-RXAS image removes eight instructions: integer and float
  result copies, one comparison-result copy, the jump-table miss copy and four
  case copies. Every throwing, ownership, typed-view, live-temporary,
  destination-read, TRACE/source-step, metadata and asynchronous-observation
  negative remains.
- Optimized and `-n` runtime images pass with identical output/status on both
  `rxvm` and `rxbvm`. Six structural tests and 42 existing jump-table roundtrip,
  algorithm, boundary and malformed-input tests pass.
- A 95,445-line generated evaluator is the scalability control. Accepted
  NR-27 assembles it in 1.36 seconds and emits 27,375 instructions. An unbatched
  J1 implementation took 91.56 seconds and emitted 25,008. Pairwise-disjoint
  copy batching reduced that to 35.24 seconds with byte-identical output.
  The final one-million-cell value-analysis bound emits 25,030 instructions in
  3.65 seconds with debug enabled: it retains 2,345 of 2,367 new removals while
  avoiding the pathological global-liveness cost. The largest procedure keeps
  2,073 unreachable removals and defers only 22 value-copy rewrites.
- The five formal wall-clock workload modules have no NR-18 source-image delta.
  The ordinary Release verdict must therefore include the matching exact
  library and use changed product artifacts rather than claim timing credit
  from byte-identical workload modules.

## Ordered execution and stop point

1. [x] Establish the exact starting state, prior-result audit, design comparison
   and candidate ledger.
2. [x] Reproduce focused and panel baselines before production code edits.
3. [x] Implement J1 and H1 incrementally with target-only builds and focused
   tests; retain negative dispositions.
4. [x] Re-run the exact static panel and smallest bounded dynamic comparisons.
5. [x] Freeze the complete panel after mathematical correctness and instruction
   gates pass.
6. [x] Build the ordinary profiling-off Release product and run one consolidated
   governed verdict against valid retained/current evidence.
7. [x] Report and stop for Adrian before broad CTest, sanitizer, install/package,
   documentation polish, commit or push.
8. [x] After Adrian accepted the verdict, rebuild the complete Debug product,
   rerun the affected optimizer/runtime/jump-table checks, pass the required
   broad Debug CTest gate, record the final evidence and prepare the requested
   local commit.

## First ordinary Release verdict

The frozen profiling-off Release product builds successfully. The exact linked
library is **55,664 -> 54,829 instructions (-835, -1.500%)** and **860,816 ->
858,896 bytes (-1,920)** against the retained accepted-NR-27 library.

The five common timing-workload modules are unchanged, and linking each with
its matching library produces byte-identical baseline/candidate products for
Sieve, Permute, Bounce, Richards and Base64. This is an exact no-regression and
no-exposure result: a wall-clock campaign over identical bytes cannot measure
NR-18 and was not manufactured.

The smallest changed end-to-end product checks pass in both ordinary Release
VMs and both profiling VMs. The cross-runtime-matrix self-test executes
**2,208 -> 2,189 instructions (-19, -0.861%)** in both VM modes. The evidence
tool (2,410), lifecycle tool (388), artifact inventory (354), generic frozen
PARSE (522), and real benchmark-runner Sieve dispatch (11,601) are dynamically
unchanged; their static reductions are unreachable preparation/code-size work.
All 24 ordinary Release runs and the matching 24 profile runs return zero and
pass their observable-result checks.

This is a favorable code-size/preparation verdict with one exact bounded
dynamic reduction, not a portfolio wall-clock speedup claim. No regression is
observable in the common product because those linked images are identical.
The implementation is provisional and uncommitted at the mandatory decision
stop. Evidence is retained under
`evidence/2026-07-22-nr-18-first-release-verdict/`.

## Accepted closeout

Adrian accepted the first Release verdict on 2026-07-22 and requested the
quality closeout plus a local commit. The complete Debug build passed all 1,129
steps. The consolidated affected-surface run passed 63/63, covering the six
whole-procedure/NR-18 structural checks, four optimized/unoptimized two-VM
runtime checks, complete and malformed jump tables, corruption handling and
their linked-artifact fixture. The required broad command
`ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure` passed
1,891/1,891 in 169.38 seconds.

The accepted shortest closeout adds no sanitizer, install/package,
cross-platform or repeated timing campaign. The change is confined to RXAS
analysis plus its tests and documentation; the focused semantic surface and
complete Debug suite are green, the ordinary Release evidence remains valid,
and the five common linked products remain byte-identical. `git diff --check`
passes. NR-18 is complete with no push requested.
