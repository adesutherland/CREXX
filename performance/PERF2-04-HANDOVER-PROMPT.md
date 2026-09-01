# PERF2-04 new-session handover prompt

Recommended setting: **Ultra** for the current-profile census, algorithm/owner
comparison and bounded PoC panel. After Adrian selects a production design, an
individual proved implementation slice can normally return to **Very High**.

Copy the prompt below into a new session.

---

You are working in `/Users/adrian/CLionProjects/CREXX` on **PERF2-04 —
inlining-first core Level B BIF campaign**.

Complete the current-profile BIF census, semantic/machine-ceiling comparison
and bounded design panel. Recommend the most efficient correct owner for each
selected family, but do not install a production implementation until Adrian
has reviewed the panel and explicitly selected the next slice or ladder.

This is not a request to make every BIF native, nor to assume that the smallest
compiler change is the fastest design. Begin from the maintainable Level B
implementation, use PERF2-03's cleaned inlining ceiling, compare the best
semantically equivalent algorithm and placement, and allow compiler lowering,
RXAS/RXBIN/VM assistance or a native control when evidence shows it is needed.
The decision must optimize end-to-end machine work while preserving the Level
B source as the complete fallback and documentation of behavior.

## Accepted starting point

PERF2-03 is complete. Its five approved production slices culminate in:

`d1c5245d49c0bd9cc48a7d33ef16f2f4555cc986`

`perf: complete proved receiver inlining`

Accepted closure facts:

- Architecture H supplies versioned local/imported body summaries, a detached
  candidate transaction, bounded cleanup/profitability and a normal-call
  fallback when proof or profitability is absent.
- Basic proved scalar/object getters and setters share the receiver efficiently
  as a matter of course. Exact reference-attribute getters/setters are also
  admitted through their separately proved body family.
- Slice 5 removed five static `ListElement.next()` calls and one `setNext()`
  call; the measured 3,820,600 dynamic `next()` calls disappeared.
- The profiling-off Release List median improved by 52.818% on `rxvm` and
  53.212% on `rxbvm`; Permute, Richards, JSON, RexxCPS and the linked library
  were byte-identical to the immediate pre-slice-5 baseline.
- Final Debug QA is 1,915/1,915 and the retained PERF2-03 evidence verifies
  against its checksum manifest.

The production commit above is the exact compiler starting point. This prompt
is part of the following documentation-only PERF2-03 closeout commit, whose
hash must be verified from live repository state. Do not assume either commit
is still HEAD without checking.

The accepted PERF2-01 profile is orientation evidence, not a frozen PERF2-04
selection. It predates the completed inliner and reported:

- RexxCPS `upper` at roughly 79-84 ms, call opcode time around 44-47 ms and
  decimal/string conversions around 47-48 ms in the bounded diagnostic;
- Base64 `SETSTRPOS` around 180-207 ms, a 1.45-1.47 s decoder path and about
  3.08 million string copies; and
- zero `srcmethodsel`/`srcfprocsel` selector attempts in the accepted optimized
  portfolio.

Treat those numbers only as hypotheses to re-rank. In particular, distinguish
RexxCPS setup/reporting BIFs from its timed kernel. The roadmap identifies
`LENGTH`, `SUBSTR` and `WORD` as known timed controls; formatting outside the
kernel must not be presented as a RexxCPS cause.

## Mandatory reading and instruction order

Read before task actions:

1. repository `AGENTS.md`;
2. `performance/AGENTS.md`;
3. `performance/ROADMAP.md`, especially the live activity register,
   PERF2-03 successor proof ledger and PERF2-04/05/07 boundaries;
4. `performance/PERFORMANCE-GOVERNANCE.md`;
5. `performance/README.md`;
6. `performance/PERF2-03-WORKLIST.md`,
   `performance/PERF2-03-ARCHITECTURE.md` and retained production Slice 5
   evidence;
7. the PERF2-01 `mechanism-census.md`, `candidate-panels.md` and relevant
   workload dossiers under
   `performance/evidence/2026-07-23-perf2-01-current-baseline/10-dossiers/`;
8. `docs/ai-context/CREXX_LEVELB_AUTHORING.md` and the linked language/runtime
   references needed for the selected BIF semantics;
9. the current `lib/rxfnsb/rexx/*.crexx` implementations and their RexxDoc/API
   companions, emitted optimized/no-opt RXAS, imported library image and
   focused BIF tests; and
10. compiler inlining/lowering, RXAS effects/optimization and VM string/value
    handlers only to the extent selected by the current evidence.

Use the live roadmap and current sources over the dated programme charter.
Preserve the charter and closed evidence as history. Do not infer cREXX Level B
syntax, BIF semantics or instruction effects from generic REXX knowledge.

## Required plan and resumable control plane

Before the first production edit:

1. give Adrian a numbered execution plan;
2. create `performance/PERF2-04-WORKLIST.md` with `apply_patch`;
3. update PERF2-04 from `queued` to `in progress` in the live roadmap when the
   activity actually starts;
4. record branch, HEAD/upstream, dirty scope, host/power state, toolchain and
   build configurations;
5. define scratch builds/worktrees for competing PoCs so the ordinary product
   baseline and each candidate remain independently identifiable;
6. write the semantic-invariant matrix, current BIF census schema and complete
   design-selection table before production coding; and
7. state the first stop: present the ranked panel and smallest recommended
   production ladder to Adrian before installing it.

Any maintained census, analysis or orchestration program must be cREXX Level B,
not Python. Reuse the existing profiling and evidence tools where they already
answer the question; do not build a second control plane merely for PERF2-04.

## Stage 0 — exact current state and BIF census

Re-rank the actual current product after PERF2-03. Inventory the current Level
B BIF/library callable surface and, for every hot or size-significant candidate,
record at least:

- exact callable and source file, direct/imported form and current inlining
  eligibility/result;
- static call sites/modules and dynamic call count in relevant current
  workloads, separating setup/reporting from timed kernels;
- procedure self/child/native time, call overhead, emitted instruction count,
  peak locals/registers and RXAS/RXBIN/linked-image bytes;
- formal/default/result initialization, general/typed copies, conversions,
  string scans/slices/searches and allocation/value-transfer bytes;
- output/validation/signal/TRACE semantics and both-VM behavior; and
- the smallest end-to-end workload cell capable of deciding whether removing
  the cost matters.

Start with the roadmap seed families—`LENGTH`; `SUBSTR`/`LEFT`/`RIGHT`;
`WORD`/`WORDS`/`WORDPOS`; `POS` and related search; and profile-selected typed
conversions—but let the current evidence reorder, combine or reject them.
Include `upper` and the Base64 string-position/copy path only if their current
timed-product attribution supports them. Do not infer priority from source size
or a historical million-call claim without confirming the current image.

Audit the generated code before assuming the BIF body or inliner is the
bottleneck. Several current Level B implementations already use direct
assembler primitives such as `strlen`, `setstrpos`, `substring`, `fndnblnk`,
`fndblnk` and `strpos`; the missing mechanism may instead be validation,
repeated scanning, result ownership, representation crossing or an algorithmic
choice.

Produce a ranked disposition for each candidate: `clean Level B already at
ceiling`, `inline/cleanup opportunity`, `algorithm opportunity`, `general
assist candidate`, `native upper bound only`, `not currently material`, or
`evidence missing`.

## Stage 1 — semantic proof and full candidate panel

For every shortlisted BIF/family, complete this panel before recommending a
production owner:

1. **Current clean source inline:** exact current Level B body compiled through
   PERF2-03 with existing primitives.
2. **Hand-equivalent machine ceiling:** the simplest mathematically correct
   instruction/register/scan/result path for the selected semantic cases.
3. **Best Level B algorithm:** restructure the maintainable source algorithm
   where repeated scans, materialization or avoidable work—not call scaffolding—
   owns the gap.
4. **Compiler-owned lowering/composition:** use existing semantic primitives
   when the compiler has the required proof and this reaches the ceiling.
5. **General RXAS/VM assist control:** prototype a narrow reusable semantic
   unit only when cleaned Level B/existing composition cannot reach the ceiling.
6. **Native/intrinsic control:** bound the remaining overhead; do not treat the
   native control as the automatic production answer.
7. **Placement decision:** compare Level B inline, compiler lowering, public
   RXAS, private runtime/quickened form and native ownership using the
   PERF2-02/05 adoption gates.

The panel is a required floor, not a ban on a stronger design. Add a stable ID
for any companion algorithm or placement that is plausibly faster. Keep each
candidate separable so its incremental instruction, image and end-to-end
effect can be measured.

Use mathematical correctness plus machine-work reduction as the pre-baseline
gate. A candidate that cannot prove the semantic equivalence of its selected
case, or does not reduce instructions/scans/copies/allocations against the best
safe current form, does not proceed to formal product timing. Conversely, do
not leave a case closed once new evidence proves it; open exactly that case and
add a distinguishing regression test.

## Semantic and correctness matrix

Cover the contract actually used by each selected BIF, including:

- Unicode/codepoint rather than byte behavior;
- empty strings, zero width/length, end boundaries and out-of-range positions;
- 1-based public indexing versus zero-based internal cursors;
- padding and validation of exactly one padding codepoint;
- optional/default/omitted/status arguments and evaluation order;
- numeric context and typed conversion errors;
- alias/reference lifetime, result ownership and repeated/overlapping actuals;
- signal identity/order, TRACE/source identity and optimized/no-opt behavior;
- local, source-import and binary-import forms where the optimization uses
  transported evidence; and
- both `rxvm` and `rxbvm`.

Preserve existing RexxDoc blocks, `@param`, `@return`, examples and notes. If a
selected algorithm changes behavior, signature, backing mechanism or return
contract, update the documentation in the same slice. Language-design changes
require Adrian's decision before implementation.

## PERF2-03 successor points

Do not reopen PERF2-03 as a general cleanup programme. Import one of its future
points only when a current PERF2-04 candidate supplies the reopen evidence:

- `PERF2-03-F03`: a hot BIF still materially above its hand-equivalent ceiling
  may carry a bounded formal/result/block-exit/temporary cleanup companion;
- `PERF2-03-F05`: any new I6 fact must be independently reconstructed and gain
  a contradictory-evidence CTest; and
- `PERF2-03-F01/F02/F04`: reference ownership, direct attribute lowering,
  vararg/association transport and assembler-effect work remain with their
  routed successor unless the chosen BIF directly proves and needs them.

Basic getters/setters must remain efficiently inlined. Include a focused guard
when a BIF/library refactor introduces or exposes a small accessor path, but do
not attribute already-enabled PERF2-03 behavior as a new PERF2-04 gain.

## Stage 2 — efficient PoC and measurement loop

Use small isolated PoC loops first:

- build only affected compiler/library/assembler/VM targets;
- run the narrow semantic and generated-RXAS checks after each change;
- compare static instructions, scans, copies, locals and image bytes before
  spending time on broad builds;
- use the exact hand-equivalent control to distinguish inlining blockage from
  an already-enabled optimization or an algorithmic/runtime limit;
- test the deciding workload plus one or two mechanism-specific guards; and
- retain raw, serial, correctness-qualified evidence for neutral and negative
  candidates as well as winners.

Use profiler timings only for attribution. Production claims come from ordinary
profiling-off Release wall clock. Run `rxvm` and `rxbvm` separately. Keep
steady-state, lifecycle, RSS and artifact size separate and follow
`PERFORMANCE-GOVERNANCE.md` when variants are close or a production design is
being selected.

A new assist advances only if it:

1. is general beyond one benchmark and appears at multiple real sites;
2. beats the fully cleaned inline and best Level B algorithm, not just the old
   wrapper;
3. reduces machine work and improves the smallest decisive end-to-end cell;
4. preserves all relevant semantic and dual-VM contracts;
5. has explicit startup, image/state, RSS, fallback and maintenance costs; and
6. is demonstrably better in its selected owner than compiler composition or a
   private form.

A BIF may complete with no new opcode. A public RXAS instruction, serialized
RXBIN change, ABI change or irreversible architecture selection requires
Adrian's explicit approval even when an isolated prototype wins.

## Required decision package and mandatory stop

Produce:

1. `performance/PERF2-04-WORKLIST.md` complete through the decision gate;
2. a current BIF census and ranked dynamic/static ownership panel;
3. a per-family semantic matrix and cleaned/hand/algorithm/compiler/assist/
   native comparison;
4. focused correctness evidence and exact generated-code/artifact comparisons;
5. bounded profiling-off Release target/guard evidence for the strongest PoCs;
6. a selected or rejected disposition for every measured candidate, including
   neutral results; and
7. a recommended ordered production ladder whose slices are independently
   provable, measurable and revertable. Recommend the breadth justified by the
   completed panel rather than stopping at an arbitrarily tiny first change.

Then report to Adrian and stop for selection. Do not install the production
ladder, run a full formal portfolio, or commit/push production changes without
explicit authorization.

If Adrian later approves a production slice, follow the mandatory first
ordinary profiling-off Release verdict in `performance/AGENTS.md`: minimum
focused correctness, freeze implementation, smallest decisive end-to-end
comparison against retained valid baseline evidence, report the verdict and
stop. Only after Adrian accepts that verdict should the slice receive broad
QA, review-derived regression CTests, retained closeout evidence and its own
local commit. Do not push unless Adrian separately asks.

---
