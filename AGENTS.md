# Repository Instructions

`AGENTS.md` is the canonical repository instruction file. `GEMINI.md` refers
here; do not duplicate repository policy elsewhere.

## Project context

The CMake toolchain is `rxc` (REXX to RXAS), `rxas` (assembly to bytecode),
`rxlink` (shared-pool linking) and `rxvm` (register-based execution). `rxc-c`
owns Classic C; `rxc`/`rxclib` retain typed-language/editor compatibility.
Common/C code uses `compiler/rxcp_tokens.h`, checked against the typed grammar;
do not restore generated B parser headers to that closure. Binutils `rxcontract`
owns metadata reading; `rxcp_contract` adapts it to the common AST. Preserve
inline enum/payload identities and supported optimisation shapes under
`compiler/docs/metadata_conformance.md`. C-only typed imports/editor mode need
the installed owning `rxc`. `rxcc_parse` success precedes backend validation;
it does not implement INTERPRET.

## Checkout, branches and release status

Use the existing synchronized `develop` checkout for ordinary development,
documentation and QA. Reserve existing `hotfix` for urgent repairs promoted to
`develop`. Reconcile local changes with the current remote branch; preserve
unrelated work and ask before discarding it. Do not create ordinary topic/QA
branches or extra worktrees. Qualify the exact `develop` commit to be promoted.
`master` is the release line: update it from qualified `develop`, then tag it.

Do not infer release status from `develop`. Check tags, `docs/releases/` and
README/release notes together. Report disagreement; staged beta content is WIP
until its tag/release exists, then `develop` is WIP for the following beta.

## Core knowledge sources

Consult relevant owning docs before changing or describing syntax, AST,
validation or runtime behavior. Read only what the task needs:

- `docs/ai-context/CREXX_ARCHITECTURE.md`, `CREXX_DEBUGGING.md`,
  `CREXX_LEVELB_AUTHORING.md` and `CREXX_UNICODE.md`.
- `docs/books/crexx_language_reference/{classes_and_interfaces,data_types,statements}.md`.
- `docs/ai-context/{RXAS_ASSEMBLER,RXLINK_LINKER,RXVM_INTERPRETER,RXPP_PREPROCESSOR,CREXX_LIBS,CREXX_ASAN_TESTING}.md`.

For Level B `.rexx`/`.crexx` edits, start with the B authoring guide and copy
referenced repository patterns. For ADDRESS, use the current environment
object/function protocol and `rxvml_address_register_callback_environment` in
the VM guide; the old command-only callback is retired. `:name` and `${name}`
are compiler auto-expose anchors; the environment owns their command meaning.

## Development, testing and delivery lifecycle

1. **Identify the stage first.** Is this a development increment, an agreed
   integration milestone or final acceptance? State whether broader QA is
   planned later. A feature or local commit is not full-solution QA. Keep full
   initiative acceptance and milestone QA open in the existing plan; do not
   create another plan or per-step run report. Explicit user-required gates
   remain binding unless the user revises them.
2. **Select the smallest meaningful development checks.** Build the changed
   component and necessary prerequisites. Test changed behavior and directly
   affected contracts, including meaningful negative, state and cleanup cases;
   use a small integration check when a boundary needs it. Default to one fast
   normal configuration and one VM. Add modes, native B/G/C comparisons, VMs,
   platforms, sanitizers, installation or compiler-wide suites only for a
   specific changed mechanism or unresolved risk. Broader registered regressions
   may remain for milestone QA/normal CI without execution at every step.
3. **Budget the whole cost.** In commentary, name the selected scope, expected
   preparation/check cost and reason; this does not require fresh approval for
   each feature. Include rebuilds, fixture compilation, review and retesting,
   considering increment size, complexity, risk and later formal QA. If QA
   dominates a small change or expands unexpectedly, stop expanding and reduce,
   batch, reuse or restructure it autonomously; explain why. Seek user scope/
   budget direction only when substantial expansion remains necessary beyond
   the agreed scope, with its concrete unresolved risk. Use no arbitrary ratio,
   weakened oracle or claim of unrun proof.
4. **Review early, qualify the feature once.** Establish independent expected
   behavior, review coherent source and first representative RXAS early, batch
   material fixes, then freeze. Later fixes receive only affected review/checks;
   formatting and documentation do not trigger code reruns. Reuse unchanged
   prepared compilers/images and valid results, checking relevant inputs rather
   than restarting a review/test/review loop. History-only merges do not
   invalidate results. Document accepted minor TRACE/presentation deviations;
   fix material wrong values, control, state, ownership or required errors.
5. **Keep native and source-only risks distinct.** cREXX/B/L edits do not require
   per-step sanitizer rebuilds/runs merely because native tools execute them.
   Native memory/ownership changes may justify focused native checks, not an
   automatic full campaign. Actual first-party ASan/LSan findings retain every
   worklist, closure and no-suppression rule below.
6. **Schedule slow tests proportionately.** For new/expanded nested tests, use
   the smallest representative normal Debug timing needed for safe scheduling,
   reusing valid timings. Registration does not require running the full
   aggregate, mode matrix or sanitizer suite. Use generous hang-protection
   timeouts and serialize costly nested groups; preserve intentional concurrency,
   explicit timeout/cancellation tests and governed Release criteria. For a
   timeout-only failure, first check logs for sanitizer/other defects, then
   replay the unchanged timed-out check once in isolation. If it passes, repair
   scheduling/timeouts before broader repetition; do not invent a product bug.
7. **Reserve broad QA for named agreed integration milestones or initiative
   completion only.** Finishing an instruction, feature or local commit does
   not create a milestone. At the agreed milestone, deliberately cover the whole
   compiler, optimisation modes, concrete VMs, platforms, delivery and sanitizers
   as agreed. Publication needs
   a current core build (reuse unchanged valid inputs), affected correctness
   checks and normal automatic CI; a local commit does not invoke publication
   gates. Release smoke, optimizer parity and CodeQL are normal workflows;
   local success does not establish hosted success. Do not dispatch or wait for
   overnight deep/stress/build-graph/full-sanitizer assurance unless expressly
   required. Production performance verdicts and actual SAN closure keep their
   separate explicit gates.
   For an agreed full functional baseline, repair and replay failing cells,
   freeze inputs, then run the complete ordinary regression suite once. Only
   after that suite is green, run the bounded two-platform component-memory
   sanitizer panel and any agreed small performance/resource screen. An actual
   sanitizer finding may require earlier focused Debug and maintained-sanitizer
   diagnosis, but does not commission an instrumented functional matrix.
8. **Finish the increment when selected checks pass.** Report input identity,
   scope, outcomes and limits in conversation, commit authorized paths and move
   on. Do not add reassurance tests or close unverified initiative criteria.
   Prepare enduring docs and publication status before pushing; do not wait for
   normal CI to make a status-only follow-up commit. Repair actual CI failures
   and link hosted results; never commit merely to mark CI green or archive
   workflow metadata. Keep plans to contracts, known limits and simple todo/done.

The sanitizer profile/preparation strategy belongs to `docs/SANITIZER-WORKLIST.md`
and `docs/ai-context/CREXX_ASAN_TESTING.md`. Daily component memory assurance
runs every day, including Sunday; compiler/assembler/linker memory producers
stop at their own output, and VM panels use ordinary producers. Daily assurance
cannot certify full release/SAN-closure scope or replace original instrumented
producers. Those guides own schedules, conservative fallback and runner details.
After ordinary functional QA, do not rerun the whole functional matrix under
ASan merely to repeat correctness proof. Select the owned native-memory panel
and focused regressions; reserve full original-producer instrumentation for an
explicit broad qualification or the existing actual-SAN closure gate.

## Emitted-code quality

Correct execution and emitted-code quality both matter. Use the lifecycle above
to select a small representative RXAS inspection, including a simple case where
unnecessary machinery is conspicuous; retain useful structural assertions.

- Set independent expectations before native comparisons. Use appropriate
  dialects/modes only where the changed mechanism needs a control; distinguish
  compiler and assembler optimisation. Classify disagreement in the new compiler,
  native compiler, runtime or oracle; never copy a native defect for parity.
- Separate metadata from instructions and caller work from helpers/libraries.
  Inspect calls, copies, frames/registers, branches, allocations and context
  setup. Line counts alone establish neither speed nor total cost.
- Preserve truthful spans, clause/provenance and TRACE/error observations,
  including control-entry mappings. Avoid repeated unchanged records/diagnostic
  updates. Use stable logical source names, with deliberate source resolution,
  rather than leaking checkout or temporary paths.
- Fix direct construction/emission defects. Classify necessary ownership/ABI
  work, admitted conservative costs and named later optimisation. Do not remove
  numeric, call or lifetime operations without proving their obligations.
- Give unresolved quality findings owners, closure checks and steps in the
  authoritative plan. Prefer structural assertions to register snapshots.
  Replacement readiness needs agreed resource/performance budgets under the
  applicable governance; ordinary research correctness is not a speed claim.

## Level L compiler delivery authority

Adrian narrowed this initiative to C on 19 October 2026. Its authoritative plan
is `research/level-l/compilers/PLAN.md`. New B/G features, proper B self-hosting
and combined replacement are outside this increment; retain their unverified
criteria and existing consumer regressions. Native B builds the implementation
and L libraries; new C requests use L parsing, IR, transformation and emission,
never native source-compiler services.

Research C closure uses the existing Classic instruction/BIF panel and a compact
interaction panel, extending maintained Classic fixtures for INTERPRET and source
calls. Known defects may remain explicitly recorded in the conformance and known
limitations document. The user guide, architecture and conformance statement own
the enduring C account; remove superseded C planning and review material after
preserving its useful contracts. Broader delivery/platform qualification,
optimisation, default replacement and release are future work, not prerequisites
for this research closure. Actual SAN findings retain their worklist obligations.
Native C's implemented coverage and approved practical TRACE scope are comparison
inputs, not correctness oracles. Keep independent L tests and native defects
explicit. Compiler policy,
generic L, shared B runtime/Unicode and RXAS/VM infrastructure have separate
owners, preserved G/C interfaces and one algorithm/data owner.

Standing authority covers reviewed local commits for the agreed C research
closure. INTERPRET and Rexx source calls are implemented under their approved
same-frontend IR contract.
Record exact contracts and obtain internal coordinator review before product
edits; select reviews/checks through the lifecycle above. Necessary C/L/shared
runtime and RXAS/VM interfaces are covered within that architecture. It does not
authorize native C frontend edits, publication, new platform/host scope, a second
runtime algorithm or unrelated native changes. Preserve explicit API/ISA,
language and platform decision gates outside this authority; the user remains
the final authority. Production performance edits retain their verdict gate.

## Working and documentation rules

- Before compiler logic, syntax, scoping or architectural edits, record the
  implementation plan described below. Ask about unapproved design changes;
  do not repeat approval requests within existing authority. For complex
  crashes/bugs, start with a minimal cREXX reproducer where practical.
- Keep findings together unless a distinct reproduced cause, component, repair,
  approval path or cross-cutting impact warrants separation. Keep distinct
  repairs reviewable; compatible publication may share exact-head QA.
- Keep owning architecture, contracts, guides, known limits and todo/done plans
  current. Fold lasting conclusions into them; remove obsolete notes, duplicate
  plans and completed handoffs. Do not retain result/timing tables, run reports,
  logs, traces, dumps, samples, receipts or hash bundles in Git or another archive.
  Ignored directories are not retention policies. Report verification in
  conversation, with hosted links when useful; delete scratch after diagnosis.
  Preserve maintained fixtures/oracles, referenced examples, licensed data and
  build inputs with named purposes; check references before deleting assets.
- Keep successful builds warning-free. Fix emitting sources and causes; do not
  hide/filter diagnostics or blanket-suppress warnings/notices. Avoid needless
  generator work on unchanged inputs.
- Native object providers use C RXPA factories/method bindings and checked
  `SETOBJECTTYPE` from `CREXX_LIBS.md`; examples are `tests/rxpa/rxpa_objects.c`
  and `lib/plugins/llama/typed.h`. `rxpa_classdecl` is metadata-only. Do not add
  duplicate construction/forwarding shims; preserve Rexx behavior owners. Remove
  replaced shim/build entries and update human/agent guides together.
- At library integration/delivery milestones, exercise a representative
  compile/assemble/link/run consumer through `rxc`, `rxas`, `rxlink` and `rxvm`,
  reusing unchanged prepared stages. Ordinary library edits use the development
  checks above. A repaired toolchain defect needs a focused permanent regression.
- Repair supported inlining rewrite/ownership/copyback shapes. Keep an ordinary
  call only when equivalence cannot be proved; broad fail-closed guards are not
  the final fix for a representable inline shape.
- Preserve existing RexxDoc `/** ... */` blocks, parameters, returns, authors,
  examples and notes when APIs survive. Update changed behavior/signatures in
  the same edit. Compare tag coverage for large library refactors; explain
  intentional removal when the API itself is removed.

## Plans, acceptance and continuity

Use the existing authoritative plan, written before implementation, with:

1. **Vision/outcomes:** the complete user outcome, capabilities, platforms,
   integration/delivery, constraints and approved exclusions.
2. **Stable numbered acceptance criteria:** observable pass conditions and
   verification methods, covering failure, lifecycle and packaging/platforms
   where applicable. Keep unverified criteria and later milestone QA open.
3. **Stable numbered steps:** criteria served, dependencies and decision gates;
   distinguish an increment/local commit from full initiative completion.

Keep IDs and simple current todo/done status, not run history. Name the plan in
handoffs; do not create competing scope records. After compaction/takeover, read
AGENTS and the whole plan's vision/criteria, then reconcile checkout/context.
Handoffs name accepted decisions, open criteria, blockers and next steps.
Do not weaken, defer or reinterpret outcomes because of context loss, difficulty,
time or partial delivery. Only explicit user scope changes alter acceptance;
completion requires every criterion in the currently agreed scope to be verified.

## Performance governance

Level L research, including `research/level-l/compilers`, has no formal
performance completion/publication gate (Adrian, 15 October 2026). Do not require
Release benchmarks, fixed sample counts, AC power or a performance verdict for
ordinary research increments. Select functional/resource-limit/code-quality
checks through the lifecycle. Label incidental timings; investigate pathological
cost without automatically launching a benchmark campaign. This exception does
not waive actual SAN findings or agreed production-replacement gates.

For performance work, read `performance/AGENTS.md`, `performance/ROADMAP.md`,
`performance/PERFORMANCE-GOVERNANCE.md` and `performance/DECISIONS.md`. Keep only
durable decisions/capability limits in HEAD; report measurements in conversation.
Outside the research exception, an approved production optimisation remains
provisional: after minimum focused correctness, freeze, build ordinary
profiling-off Release, make the smallest decisive end-to-end comparison against
a valid unchanged-input baseline, report it and stop for Adrian's verdict.
Broad Debug/sanitizer/install proof, harness refinement and closeout follow that
verdict, not precede it.

## Build and installation constraints

- Linux defaults to `$HOME/.local`: `cmake --install cmake-build-release` needs
  no sudo. Runtime bytecode/plugins/helpers, `libcrexxsaa.so` and static archives
  intentionally share `bin/`; do not move archives without `crexx -native`
  packaging proof. ELF `crexxsaa` uses `$ORIGIN` to find its adjacent library.
- When broad Debug QA is selected, macOS ARM64 uses deliberate CTest parallelism
  (typically 30); other Unix hosts normally at least 10 subject to resources.
  Preserve the syntax-highlighter RESOURCE_LOCK and prove parallel stability
  before removing it. Check memory/swap and concurrent VM-core compilation when
  Release builds slow down. These notes do not commission a broad run.
- Preserve shared `rxtvm_core_objects`/`rxbvm_core_objects`; `NTHREADED` belongs
  only to the latter. `rxvm` is a symlink/copy, not a third core; MSVC builds
  only rxbvm core. Preserve `rxvm_platform_objects` and the RXPA harness's one
  `rxvm_platform_notls_objects`. Replacing sharing requires measured Release
  build impact and appropriate Debug/Release correctness proof.
- On Windows, do not overlap builds/tests in one tree. After interruption,
  stop leftover processes before retrying generated artifacts; cleanup-lock
  failures may need a serial retry. Shared-tree preparation is centrally owned.

## Agent models and escalation

Adrian's current allocation applies to the primary coordinator and subagents:

- **GPT-6 Sol Extra High** (`gpt-6-sol`, `xhigh`) coordinates work and owns
  build/test execution, QA runs and diagnostics, including ASan/LSan, memory
  ownership, signal unwinding, loaders and low-level VM debugging.
- **GPT-6.1 Sol Extra High** (`gpt-6.1-sol`, `xhigh`) owns design, detailed
  implementation and source review. These workers hand build/test execution
  to the Sol 6 coordinator or execution worker, preserving central ownership
  of shared preparation and valid results.
- **GPT-6 Astra Extra High** (`gpt-6-astra`, `xhigh`) may be used for a
  substantial blocker or key design decision only after Adrian explicitly
  approves that escalation. Explain the question and why escalation is needed.

Use models available under the account's configured access. Confirm eligibility
before changing the execution/debugging allocation. If the client cannot change
the active coordinator model, report that limit and preserve the chat, checkout,
running processes and completed results for continuation with the requested model.

The coordinator inspects its own and subagent failures for explicit cybersecurity
rejections, distinguishing them from tool, build and test failures. Identify the
affected role and action, report the restriction, preserve completed work and
continue independent authorised work. Resolve rejected actions through supported
access or approval channels; do not change models merely to bypass a security
rejection or weaken task authorisation or safety requirements.

## Actual sanitizer findings and diagnostic output

Every first-party ASan/LSan or maintained sanitizer finding is a repository
blocker, including independent/pre-existing findings. Before unrelated closeout,
record `SAN-nnn` in `docs/SANITIZER-WORKLIST.md`: reproducer, concise diagnostic,
revision, owner/next action and closure check; inspect then delete temporary logs.
Never substitute a dated note or ownerless deferral.

No release/sanitizer-clean claim is allowed with open first-party SAN items.
Only explicit user assignment of outstanding platform proof to a named release
QA gate can close a bounded phase; list every open SAN item, repair evidence
and owner. It does not close/downgrade the item. Closure requires a permanent
regression, the same focused normal Debug and maintained-sanitizer command,
and the broad platform gate in `CREXX_ASAN_TESTING.md`. Record unsupported
capabilities such as Apple LSan and cover them on a supported platform.

No first-party suppression, exclusion, supported-platform leak-off wrapper or
time waiver without explicit user approval; approved exceptions remain open
release blockers until repaired/requalified. Use `tools/asan-run.sh` and its
owning guide for sanitizer build/test/status/kill actions; broad hand commands
are allowed only when debugging/bypassing a broken runner. After sanitizer
repairs run the focused normal command before the matching maintained command.
For unattended work, reuse/request runner-prefix approval rather than stalling.

Redirect verbose/unbounded output (builds, `rxc -d*`, parser/fixed-point and
syntax-highlighting traces) to `mktemp` files, inspect relevant excerpts and
report conclusions. Do not filter away diagnostics or copy logs into evidence
archives; delete task-created output after updating lasting contracts, limits
or active SAN findings in their owning documents.
