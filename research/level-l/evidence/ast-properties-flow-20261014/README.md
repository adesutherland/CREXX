# Sparse properties and simple flow analysis

Local qualification of AST5-AC01–05 from `research/level-l/PLAN.md`, based on
`91ad050457a39367d810785b74a30924a8af90c3` plus the inputs identified in
`inputs.json`. Adrian authorized publication to `develop` on 14 October 2026.
The enclosing commit is the publication payload; see the reconciliation below.

## Implemented contract

- `properties` groups declare `.string` and `.int` literal defaults. Generated
  `getmessage()`, `hasmessage()` and `setmessage()` methods use sparse explicit
  node/property pairs. A read of a default does not insert a pair. Freeze,
  type, owner/snapshot and counted-entry checks apply to both output formats.
- Complete kind/field-checked flow mappings define sequence, evaluation,
  assignment, branches, reads and values. Assignment evaluates its value first;
  an optional absent ELSE joins directly. Graphs and analyses capture their
  owners on construction and reject replacement even before their first query.
- Definite assignment uses a finite monotone may-unassigned domain, explicit
  entry names and checked state/work allowances. Each unsafe read can produce
  an entry-to-read graph path with source-linked branch explanations. This is
  relative to the declared execution graph; value-dependent path feasibility
  and new loop/call/exception mappings are outside this increment.
- The worked rules-language example stores the diagnostic in sparse properties.
  The manual explains the components and public operations. Mathematical
  obligations and cost bounds are in `AST-CONTRACT.md`.

## Regression design

`ts_llflow.crexx` runs as precompiled, linked images on LL(1), LR(1), packrat and
Earley, with optimized/unoptimized consumers and both VMs. Sixteen CTests call
only a VM; there is no nested build or scenario matrix inside a CTest.

The runtime cases cover defaults and explicit default values; replacement and
freeze; failed/expired/copied/rewritten/reassigned owners; stale views without
poisoning a replacement group; exact and exhausted graph/cell/work allowances;
all graph allowances below the required size; witness exhaustion without
invalidating a completed result; CRLF diagnostic lines; missing ELSE; reads
before writes; case-sensitive names; empty, nested and wide inputs. Sixteen
write configurations through two branches are compared with independent
four-path enumeration.

The wide/deep fixtures use 200 statements/40 nested branches on deterministic
backends and 20/8 on Earley. The original 200-statement Earley fixture correctly
refused `413 * 804^2` chart cells against its 8,000,000-cell default. This was a
test-workload correction; no parser budget or product diagnostic was weakened.
All backends retain the same branch-path controls and ordinary semantic cases.

`check-flow-surface.py` is an explicit qualification command. It checks source
and packed RXAS providers, all four general parser backends, both optimization
modes and both VMs; Pratt operator trees and evaluation order; Unicode quoted
and minimum signed-integer defaults; 16 negative grammar fixtures at L0/L1/L2
with exact line/byte/message assertions; and misspelled/type-invalid generated
application APIs.

`check-delivery.py` checks a fully unoptimized Level B runtime, identical
worked-grammar generation at all bootstrap stages, installed `.rxl` and
`.crexx` with `options levell`, combined program builds, unchanged-build reuse,
and a separate grammar library plus program. Installed cases use isolated
output directories so unrelated generated modules cannot affect import/cache
selection.

## Findings repaired during qualification

Two new runtime findings gained permanent tests:

1. A stale property view marked its replacement group as failed. The view now
   raises its ownership error without changing the replacement group's state.
2. A zero graph allowance could continue with event ID zero and raise a raw
   range signal. Every event-allocation boundary now stops graph construction
   immediately. Every smaller allowance in the representative graph is checked.

The initial broader CTest run passed the existing 34 AST, parser and RexxScript
cases. Its four Earley stress-fixture failures are retained in
`initial-regressions.log`; the final new-flow results supersede those failures.
Unchanged existing tests are reused rather than rerunning the full script suite
for corrections confined to the new annotation/flow services.

## Final results

- `product-and-flow-tests.log`: successful Debug `stage-product`, Level L and
  RexxScript test-image build; all 16 new flow CTests passed (12.04 seconds).
- `initial-regressions.log`: all 34 unchanged AST, parser and RexxScript tests
  passed. Together with the final flow run, all 50 selected cases are verified.
- `noopt-bootstrap.log` and `inputs.json`: standalone Level B build with `-n`
  passed. L1/L2 generated parser source is byte-identical in each mode and
  between the optimized and unoptimized builds.
- `surface.json` / `surface.log`: 200 commands passed their expected outcomes:
  32 general-backend/format/optimization/VM executions, four literal-default
  executions, four Pratt executions (including left-to-right evaluation), 48
  exactly located grammar refusals and two expected application compile errors.
- `delivery.json` / `delivery.log`: 46 commands passed, including eight runtime
  executions with fully unoptimized services, identical worked-grammar output
  from L0/L1/L2, six installed executions, both supported file forms, repeated
  unchanged builds, and separate library/program linkage.
- `guide-links.json`: all 51 local documentation links resolve. `git diff
  --check` passed. Final implementation/test/build hashes match the frozen
  inputs; final builds and successful application compiles emitted no warnings.

The qualification host is macOS ARM64 with the Debug native toolchain. These
are local product and focused correctness results. Normal automatic platform CI
is expected to pass after publication, with actual results reported in the
conversation. No cross-platform or sanitizer pass is claimed by this receipt.

## Publication reconciliation — 14 October 2026

Before committing, fast-forwarded the checkout from `91ad05045` to
`0dc462b6d70f2bdca69827c811dedb5e2ae18ea8`, preserving the local AST changes.
The sole incoming edit adds an existing-directory check in `bin/crexx.crexx`.
All 13 source/test/build inputs and eight tool/library images in `inputs.json`
still match their retained hashes, so the earlier analysis/parser qualification
remains applicable.

Rebuilt `stage-product`, installed into the existing isolated test prefix, and
repeated the installed section of `check-delivery.py` with fresh output under
`research/level-l/build/ast5/publication-delivery`. The only harness substitution
was `output=base/'delivery'` to `output=base/'publication-delivery'`; no expected
result or tested source changed. All 12 commands passed: both source forms,
combined and separate library/program builds, both VMs, and unchanged-build
reuse. These are macOS checks of the integrated wrapper; the automatic Windows
job provides its platform-specific validation.

`publication-build.log`, `publication-installed.log`,
`publication-delivery.json` and `publication-inputs.json` retain the results,
commands and reconciled identities. No extra hosted matrix was dispatched.

Final staged-file review removed trailing whitespace from one Python test line
and two retained log copies. The Python syntax tree is identical before and
after cleanup; original and final hashes are recorded in
`publication-inputs.json`. The original qualification identities remain in
`inputs.json`, and this formatting change does not invalidate their test results.
