# Structured loops, exits and short-circuit flow

Local qualification of AST6-AC01–04 in `research/level-l/PLAN.md`, on develop
base `78761cc5affe41c389e4969f18500c01f1e023cd`. The approved increment is
implemented and locally verified. Adrian authorized direct commit/push to
`develop` on 15 October 2026; the enclosing commit is the publication payload.

## Delivered contract

- Three loop mappings: pre-test WHILE, post-test UNTIL and unconditional repeat.
  BREAK leaves the nearest loop; CONTINUE reaches its test or next body.
  RETURN evaluates its optional child and leaves the analysed root.
- AND/OR short-circuit mappings preserve distinct outcomes through nested
  conditions. Ordinary value use joins those outcomes. Abrupt exits have no
  reachable normal completion into the following statement.
- `result.before(node).reachable()` is a checked borrowed query. Unreachable
  nodes return false for both reachability and definite assignment. Invalid
  jump placement reports kind, line and byte even in unreachable source.
- Graph construction uses two iterative walks and bounded events/edges.
  Definite assignment retains its finite monotone domain and converges on
  cycles. Witnesses follow actual graph paths without crossing the relevant
  assignment. The proof and resource bounds are in `AST-CONTRACT.md`.
- The worked `examples/flow/loops.rxl` / `loops.crexx` example and manual teach
  grammar captures, named AST fields, flow paths, loops, exits and skipped
  operands. Constant/proof-aware branch pruning remains outside this increment.

## Bootstrap engineering

The expanded self-description initially exceeded the default clause allowance;
its common keyword alternatives are now factored without changing the terminal
language. The resulting self grammar is admitted by the existing LL(1) backend,
which is selected explicitly. No user resource defaults were increased.
L0 remains the minimal reader; L1/L2 use the generated reader without fallback.
The four L1/L2 syntax outputs from optimized and unoptimized Level B builds
are byte-identical. Existing specification recovery remains qualified.

## Evidence

- `ctest.log`: all 64 selected runtime tests passed, including 16 new structured
  flow cases (four backends × two consumer modes × two VMs), 16 previous sparse
  property/flow cases, AST/analysis/backend controls and six RexxScript tests.
  Runtime tests launch prebuilt linked images, not nested compiler matrices.
- `surface-completed.json`, `surface-original.py`, `surface-observations.log`:
  112 sequential surface commands completed all exit-status/output assertions:
  source providers across four backends and both modes/VMs; a short-circuit
  expression as root and every insufficient graph allowance; exact source/RXAS
  output parity across L0/L1/L2; 13 semantic refusals with exact locations at all
  three stages. The harness was stopped later during an oversized malformed
  fixture, after this completed prefix. These results are retained, not rerun.
- `malformed-final.json` / `malformed-final.log`: the four syntax-refusal fixtures
  were reduced to small complete grammars and passed all 12 stage checks.
  The permanent harness supports `--malformed-only` for this focused repeat.
  General recovery is independently covered by the following panel.
- `specification-recovery.json` / `.log`: 24 candidate-recovery and 12 preservation
  cases pass on both VMs at L1/L2.
- `noopt-delivery.json` / `.log`: 34 checks pass with fully unoptimized Level B
  services, including both provider formats, both consumer modes and both VMs.
- `installed-final.json` / `.log`: 12 fresh installed checks pass after the final
  reader repair: `.rxl`, `.crexx` with `options levell`, combined program builds,
  unchanged-build reuse, separate grammar library/program and both VMs.
- `reader-repair-noopt.json` / `.log`: nine focused checks verify the repaired EOF
  location at all unoptimized stages and unchanged source/RXAS output.
- `reader-ctest.log`: both existing generated-frontend tests pass after repair.
- `guide-links.json`: 53 local links and 41 internal anchors resolve.
  `git diff --check` and Python harness syntax checks pass.

The tests independently enumerate 16 loop-arm write/break/continue combinations,
including cycles with no exit and first-iteration unsafe exits. Other cases
cover zero iterations, post-test CONTINUE, nested targets, returning branches,
short-circuit truth correlations, stale/foreign/expired views, CRLF locations,
exact graph/cell/work limits, every smaller graph allowance, and branch/jump
witness text. Deep nesting uses 30 loops on deterministic backends and five on
Earley within its existing chart budget; the ordinary path controls are shared.

## Repairs during qualification and evidence reuse

`initial-tests.log` records two placement-message failures caused by an erroneous
comparison in the new message expression. The corrected message passes exact
line/byte/text checks on every backend/mode/VM.

The first malformed EOF probe exposed a seed-reader line-0 diagnostic.
`missing-field-before.log` retains it. A checked field reader now validates
presence before consuming a required field. All stages report line 7 / byte 149
for the missing UNTIL condition. The final seed operator diagnostic also says
`expected AND or OR between shortcircuit fields`.

The earlier product build included a scope warning in the new test's loop
variable declaration. That declaration was made explicit; `test-build.log` and
the final `reader-repair-build.log` / `reader-repair-noopt.log` are warning-free.
`current-build.log` confirms that the earlier product/test inputs were current
before the final narrow reader repair.

The final repair changes field validation and seed diagnostics only. The graph
implementation bytecode (optimized and unoptimized) and four packed grammar
providers remain byte-identical to their 64-test inputs, as recorded by
`reader-repair-control.json`. All nine valid noopt output comparisons still
match the retained outputs. The two frontend CTests, malformed probes and fresh
installed checks cover the changed reader path; unchanged runtime tests are
reused. `inputs-before-reader-repair.json` and `inputs.json` identify both states.

## Qualification boundary

The host is macOS ARM64 with the normal Debug native toolchain. Product build,
focused functional correctness, Level B bootstrap and isolated installation
are verified. This record makes no hosted CI, cross-platform or sanitizer claim.
The separate mainframe/platform work was committed in `9c790a5cb` before this
publication and is not part of the Level L diff. Unrelated untracked build/lock
files remain untouched. Native tool image identities are retained so the local
results refer to their actual inputs.

The wider AST programme still includes constant-aware flow contracts,
grouping/lowering, origins and production compiler migration. This increment
closes only AST6-AC01–04. Publication does not close the wider programme.

## Develop publication reconciliation — 15 October 2026

Local and remote develop were verified equal at
`9c790a5cb4aa25b04166d30d1c9479d24a9722b0` before publication. All nine
qualified Level L source/test/example hashes still match `inputs.json`. The
intervening native VM change returns RC 2 for unreadable input modules; the
mainframe platform changes do not affect the ordinary macOS code path.

- `publication-build.log`: current `stage-product` and `testrxfnsl` build passes
  without warnings. Its retained log removes trailing whitespace only; raw and
  stored digests are in the publication receipt. The generated L1/L2 syntax in both Level B modes still
  matches the retained bootstrap hash.
- `publication-ctest.log`: all 18 focused tests pass on the current VMs: the
  16 structured-flow backend/mode/VM cases and two generated-reader controls.
- `publication-install.log`, `publication-installed.log` and
  `publication-installed.json`: refreshed isolated installation and all 12
  combined/separate-library wrapper checks pass.
- `publication-20261015.json`: exact source/native/runtime image identities and
  the retained versus refreshed evidence boundary.

Only Level L source, tests, examples, documentation and evidence are included
in this publication. Normal automatic Build CREXX and CodeQL runs are expected
to pass; their observed results belong to the conversation and GitHub run
records, not a follow-up status-only repository commit.
