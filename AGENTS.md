# Repository Instructions

`AGENTS.md` is the canonical repository instruction file. Keep repository-specific agent guidance here. `GEMINI.md` should only refer back to this file so instructions do not drift.

## Project Context

`crexx` is a custom REXX-to-bytecode toolchain with four main CMake-built binaries:

1. `rxc`: compiler from REXX source to `rxas` assembly
2. `rxas`: assembler from `rxas` assembly to `rxbin` bytecode
3. `rxlink`: linker for combining one or more `rxbin` modules into a shared-pool linked image
4. `rxvm`: interpreter for register-based `rxbin` bytecode

`rxc-c` is the dedicated Classic Level C compiler over the shared backend.
`rxc`/`rxclib` retain typed-language and desktop editor compatibility. Common
and C code use `compiler/rxcp_tokens.h`, checked against the typed grammar's
generated token table; do not reintroduce generated B parser headers into their
closure. Metadata fields belong to the independent binutils `rxcontract` reader;
`rxcp_contract` adapts records to the common AST. Follow
`compiler/docs/metadata_conformance.md`, preserving original inline enum/payload
identities and supported optimization shapes. C-only typed source imports and editor
mode require the installed owning `rxc` tool. `rxcc_parse` is an owned frontend
API; success precedes backend validation/emission and does not implement INTERPRET.

## Checkout And Branch Discipline

Use the existing `develop` checkout for normal development, documentation,
release preparation and QA. Use the existing `hotfix` checkout only for an
urgent fix that is then promoted to `develop`. Keep the selected checkout
synchronized with its remote branch before editing, and reconcile local changes
against the current branch rather than leaving an older dirty checkout behind.

Do not create temporary topic, release-candidate or QA branches or additional
worktrees for ordinary repository work. Run pre-release qualification on the
exact `develop` commit that will be promoted. `master` is the release line:
update it from the qualified `develop` commit only when cutting a release, then
tag the resulting `master` commit. Preserve unrelated local work when syncing;
ask before discarding work unless the maintainer has explicitly directed its
disposition.

## Release Branch Status

Do not infer released status from `develop` alone. During the Release 1 beta
line, `develop` may be staged for the next beta release before the tag exists;
after a beta tag is cut, `develop` should be described as work in progress for
the following beta, for example beta 3 WIP after `v1.0.0-beta.2`.

When making release-status claims, check release tags, `docs/releases/`, and
the current README/release notes together. If they disagree, say so explicitly
and avoid calling staged `develop` content "released" until the tag/release
exists.

## Core Knowledge Sources

Before changing compiler logic or making claims about syntax, AST shape, validation, or runtime behaviour, consult the relevant project docs instead of guessing:

- `docs/ai-context/CREXX_ARCHITECTURE.md`
- `docs/ai-context/CREXX_DEBUGGING.md`
- `docs/ai-context/CREXX_LEVELB_AUTHORING.md`
- `docs/ai-context/CREXX_UNICODE.md`
- `docs/books/crexx_language_reference/classes_and_interfaces.md`
- `docs/books/crexx_language_reference/data_types.md`
- `docs/books/crexx_language_reference/statements.md`
- `docs/ai-context/RXAS_ASSEMBLER.md`
- `docs/ai-context/RXLINK_LINKER.md`
- `docs/ai-context/RXVM_INTERPRETER.md`
- `docs/ai-context/RXPP_PREPROCESSOR.md`
- `docs/ai-context/CREXX_LIBS.md`
- `docs/ai-context/CREXX_ASAN_TESTING.md`

Read only what is needed for the task, but do not rely on memory for cREXX syntax or compiler internals when the docs cover it.

For tasks that write or edit Level B `.rexx`, start with `docs/ai-context/CREXX_LEVELB_AUTHORING.md` and copy patterns from the referenced repo examples instead of inventing syntax from generic training data.

For ADDRESS environment work, `docs/ai-context/RXVM_INTERPRETER.md` is the current protocol reference. The pre-release command-only native callback registration form has been retired; use the current environment object/function protocol and `rxvml_address_register_callback_environment(ctx, name, id, command_cb, function_cb, userdata)`. ADDRESS host-variable anchors (`:name` and `${name}`) are compiler auto-expose syntax only; their command meaning belongs to the selected environment handler.

## Development publication and overnight assurance

For ordinary develop publication, require a successful core product build,
appropriate focused functional regressions and the relevant normal correctness
suite. Report the exact qualified inputs, outcomes and limitations in conversation.
Normal automatic workflows provide Release product/smoke, optimizer-parity and
CodeQL checks; local results do not establish hosted success.

Full hosted deep/comprehensive matrices, build-graph comparisons, stress and
full Linux ASan/LSan/macOS ASan assurance normally run overnight. Do not dispatch
or wait for them for routine development, compiler/runtime changes or hotfixes.
Use failures to drive prompt repairs and, where justified, a targeted repeat.
Additional pre-publication gates require an explicit scope or concrete risk,
such as an agreed native-inference change, release candidate or actual sanitizer
finding. Preserve user-required gates and the SAN closure rules below.

Avoidable testing delay is an engineering cost. Before a long or repeated run,
name the unresolved risk, why existing results or focused checks are insufficient,
and why the benefit warrants its time. Reuse valid results when code/test/build
inputs are unchanged. History-only merges and documentation changes do not
invalidate them; do not accumulate extra checks merely for more green results.

Prepare current documentation and report a concise validation summary and the
expected publication status in conversation before the first substantive push.
Once required local gates pass, assume normal workflows will succeed for that
preparation, while reporting only observed passes as verified. Do not hold the
task open for CI or leave a planned post-CI documentation update.

Never commit or push solely to mark CI passed, replace pending status or archive
workflow metadata. Hosted run links provide the result; report observed status
in conversation. If a workflow fails, inspect its logs, diagnose and repair the
cause, and include necessary documentation corrections with the substantive fix.
Do not download and retain a second copy of the logs. Required release and
sanitizer proof still applies. Keep this publication policy here, not in
component worklists.

## Emitted-code quality

Compiler acceptance includes the quality of emitted code as well as correct
execution. For changes to lowering, optimisation, register assignment or
emission, inspect a small representative set of readable RXAS and retain
checkable code-shape expectations alongside functional regressions. Include
simple cases where unnecessary machinery is conspicuous. Reuse existing
evidence where the qualified inputs are unchanged; this is not a requirement
for a new broad benchmark or release-qualification run.

- Compare equivalent inputs with the existing compiler's appropriate B, G and
  C modes, reporting compiler and assembler optimisation separately in
  conversation. An old compiler's excess code or defect is a finding, not an
  acceptance target.
  State expected behaviour from the agreed language contract and independent
  test expectations before using native output as a control. When results
  disagree, retain a minimal case and classify the cause in the new compiler,
  native compiler, shared runtime or test oracle; do not reproduce a native
  bug to obtain parity or weaken an expectation merely to match it.
- Distinguish source/debug records from executable instructions, and caller
  code from work hidden in helpers or linked libraries. Inspect calls, copies,
  frame/register requirements, branches, allocations and numeric/context
  setup. Raw RXAS line counts alone establish neither speed nor total cost.
- Preserve truthful source spans, clause identities, generated provenance and
  defined TRACE/error observations. Do not reduce counts by stripping useful
  metadata. Avoid mechanically repeated unchanged source records and diagnostic
  state updates, while preserving mappings at control-flow entries. Emitted
  source names must use stable logical identities rather than accidentally
  embedding the compiler host's absolute checkout or temporary paths; retain a
  deliberate source-resolution/mapping mechanism where needed.
- Identify which excess comes from a current construction/emission defect,
  necessary language or ownership semantics, an admitted conservative baseline,
  or a named later optimisation. Do not defer direct emitter/IR-construction
  defects merely because an optimiser could hide them. Do not remove call ABI,
  numeric-context or lifetime operations without proving their obligations.
- Record unresolved quality findings, owners, closure tests and intended steps
  in the authoritative plan. Correct output alone does not close them. Use
  structural assertions rather than brittle register-number snapshots where
  possible; agree representative resource/performance budgets before claiming
  replacement readiness, under the existing performance governance.

## Level L compiler delivery priority

Adrian narrowed this delivery to Level C only on 19 October 2026.
`research/level-l/compilers/PLAN.md` owns its coverage and acceptance. New B/G
features, proper B self-hosting and combined replacement are outside this
increment; retain their unverified criteria and existing consumer regressions.
The native B seed builds the B implementation and Level L libraries. New
compilation requests use Level L parsing, IR, transformation and emission and
must not delegate to native source compiler services.

Complete C includes agreed instructions/BIFs, host contracts, diagnostics,
conditions/TRACE and INTERPRET. Design the complete architecture before
implementation approval; implement INTERPRET last. Review instructions one at
a time with individual semantics, errors, RXAS quality and shared dependencies.
Maintain independent Level L tests. Keep compiler policy, generic L services,
shared B runtime/Unicode libraries and RXAS/VM infrastructure separately owned.
Adrian approved the C-only plan and shared B Unicode extraction on 19 October
2026. Preserve G/C interfaces and one algorithm/data owner; exact new public
API and ISA contracts retain their decision gates.

Retain language/API/platform decision gates and explicit exclusions. Inspect
actual RXAS and helper work throughout; native C may be faulty or inefficient.
The emitted-code rules and Level L research performance exception apply;
production RXAS/VM performance edits retain their own verdict gate.

## Working Rules

- For tasks that change compiler logic, syntax, scoping, or architecture,
  present an implementation plan before editing. Include the vision and intended
  outcomes, numbered checkable acceptance criteria, and numbered implementation
  steps, following the plan and session-continuity rules below.
- Pause for user approval before making language-design decisions, syntax changes, or architectural shifts. The user is the final authority on language direction.
- For complex bugs or crashes, start with a minimal reproducer in cREXX where practical before changing core C code.
- Run focused tests frequently during compiler work. If a change causes regressions, stop, report them clearly, and distinguish expected from unintended fallout.
- Once the required full testing has passed, do not repeat it if no code or
  test/build input has changed. Verify that with the relevant diff or tree
  hashes and reuse the completed testing. A history-only merge or
  documentation-only commit does not by itself invalidate completed testing.
  Normal automatic publication workflows remain separate checks; use the
  development-publication policy above to choose gates, and do not infer a
  requirement to dispatch overnight assurance or repeat unchanged local QA.
- Before registering or materially expanding an aggregate CTest that launches
  nested builds, compilers, assemblers, linkers, VMs, or scenario matrices,
  measure it in isolation in normal Debug. For B/Level L/research source-only
  slices, use focused normal Debug qualification and reuse valid sanitizer
  evidence for unchanged native inputs. Do not run or refresh sanitizers per
  slice; any warranted new sanitizer measurement/qualification is consolidated
  at final delivery, not an automatic campaign for unchanged native code.
  Native product changes retain their applicable sanitizer requirements, and
  actual first-party sanitizer findings retain all SAN obligations below.
  Set explicit scheduling and generous timeout properties from the available
  evidence; retain serial scheduling for nested aggregates. Do not inherit an
  old timeout after materially increasing the workload or let nested aggregates
  compete in the broad pool when load makes their deadlines nondeterministic.
- In functional QA, use generous wall-clock limits as hang-protection backstops,
  not incidental performance requirements. Avoid short nested or whole-workload
  deadlines that make slower or busy GitHub runners fail correct work. Serialize
  deadline-sensitive tests where practical, while retaining deliberate
  concurrency inside ownership and worker tests. Keep explicit timeout or
  cancellation-contract tests and isolated Release performance criteria intact.
- When a broad run fails only through explicit CTest timeouts, inspect the log
  first for sanitizer diagnostics and non-timeout failures, then replay the
  unchanged timed-out tests once under their declared isolated scheduling. If
  they pass, classify and repair the QA scheduling or timeout defect before
  repeating a broad run. Do not create a new product defect or spend another
  full-suite cycle on a timeout-only result unless the isolated test reproduces
  a product failure or establishes a distinct mechanism.
- Keep a discovered finding on the current issue unless it has a distinct
  reproduced cause, affected component, repair, approval path, or shared
  cross-cutting impact. Preserve separate reviewable commits where causes do
  differ, but publish compatible commits together and use one combined
  exact-head hosted qualification when that avoids redundant long-running
  gates. Sanitizer worklist requirements below still apply to actual
  first-party sanitizer findings.
- Keep documentation current and enduring: architecture, contracts, user guides,
  intended design, known limitations, decisions and simple todo/done plans or
  worklists. Update the owning document in place; delete obsolete notes, completed
  handoffs, duplicate reports and superseded plans after incorporating any
  lasting information.
  This applies to every project and future initiative, including research,
  performance and sanitizer work.
- Observed run reports, result/timing tables, logs, traces, raw samples, generated
  dumps, run receipts, per-run hash bundles and scratch results are disposable.
  Use temporary or ignored build locations while actively diagnosing or measuring,
  then delete them after extracting the useful conclusions. Do not track, commit,
  archive elsewhere or retain them "for evidence" or as plan history. Active
  sanitizer findings retain the worklist requirements below. An ignored directory
  is not a retention policy. Do not create
  dated evidence/run directories or commit junk first to preserve it in Git.
  Existing tracked junk should be removed as part of the relevant cleanup.
- Report verification in conversation: revision or relevant input identity,
  command, result, scope and limitations, with hosted run links when relevant.
  Keep only lasting contracts, known limitations and current todo/done status
  in the owning document; do not store observed results or run history in plans
  or elsewhere. Reuse valid results without keeping run reports or raw output.
  Maintained source, regression fixtures, test oracles, example outputs used by
  tests/docs, licensed data and build
  inputs remain legitimate assets; give each a named purpose and owner, rather
  than relabelling an evidence dump as a fixture. Before deletion, check actual
  references and move any real test input into its owning test/fixture directory.
- Keep normal successful builds free of compiler warnings and unnecessary generated notices. Remove notices at their emitting source and fix warning/error causes. Do not hide command stdout/stderr, filter diagnostics, or use blanket warning suppression to make a build look clean. Avoid unnecessary generator work on unchanged inputs.
- For native object providers, use the C RXPA factories/method bindings and
  checked `SETOBJECTTYPE` service documented in `docs/ai-context/CREXX_LIBS.md`.
  Do not add duplicate Rexx construction or forwarding classes when those C
  bindings already implement the contract. Use `tests/rxpa/rxpa_objects.c` and
  `lib/plugins/llama/typed.h` as executable examples; `rxpa_classdecl` is only a
  metadata compilation fixture. Preserve Rexx classes that still own behavior.
  When replacing a shim, remove its obsolete source/build entries and update
  human and agent guides together; fold lasting conclusions into current docs
  and delete obsolete notes.
- Treat library development as an opportunity to validate the complete product
  toolchain. Library changes should exercise `rxc`, `rxas`, `rxlink`, and
  `rxvm`, and a library-discovered defect in any of those layers should gain a
  focused regression test when it is fixed.
- When correcting an inlining defect, preserve the supported optimized shape
  by fixing its rewrite, ownership, or copyback mechanics. Retain an ordinary
  call only when the required equivalence cannot be proved; do not use a broad
  fail-closed guard as the final fix for a representable inline shape.
- Treat source documentation tags as first-class code-adjacent assets during refactors. When rewriting or replacing `.crexx` classes, methods, plugins, or library surfaces, preserve existing `/** ... */` RexxDoc blocks and tags such as `@param`, `@parm`, `@return`, `@author`, examples, and notes wherever the documented API still exists. If behaviour, signatures, backing implementation, or return contracts change, update the tags in the same change instead of dropping them. Before large classlib or library refactors, compare relevant doc-tag coverage before and after; if tags are intentionally removed because an API is removed, call that out explicitly in the change summary.

## Plans, Acceptance and Session Continuity

Every required implementation plan, and every plan for work spanning sessions,
must contain all three of the following:

1. **Vision and intended outcomes:** explain the problem, why the work matters,
   and the complete intended end state from the user's perspective. Preserve
   the requested capabilities, platforms, integration and delivery requirements,
   constraints, and any explicitly agreed scope boundaries. A list of immediate
   implementation tasks is not a substitute for the intended outcome.
2. **Numbered checkable acceptance criteria:** give each criterion a stable ID
   such as `AC-01`, an observable pass condition, and the test or inspection
   method needed to verify it. Together the criteria must cover the
   full intended outcome, including required failure, lifecycle, packaging and
   platform behavior where applicable. Keep unverified criteria visibly open.
3. **Numbered implementation steps:** give each step a stable ID such as
   `STEP-01`, identify the acceptance criteria it serves, and record dependencies
   and applicable decision gates. Separate phased delivery from overall success.

Before implementation, record this plan in the appropriate repository planning
or worklist file so it survives conversation compaction and session takeover.
Reuse an existing authoritative plan where one exists; do not create competing
scope records. Include its path in progress handoffs, and keep criterion/step
status current as simple todo/done, without observed run results or history.
Preserve IDs when updating the plan.

After compaction or at session takeover, read the repository instructions and
the authoritative plan, including its vision and every acceptance criterion,
before continuing implementation. Reconcile completed and outstanding work with
the current checkout and conversation context. Handoffs must identify the plan,
accepted scope decisions, unmet criteria, blockers and next steps; they must not
reduce the objective to the latest task or the easiest remaining subset.

Do not silently remove, weaken, defer or reinterpret outcomes or acceptance
criteria because of context loss, implementation difficulty, elapsed time or a
partial implementation. Record explicit user-approved scope changes and their
effect on acceptance. Until then, preserve the original requirement as open.
Report phase completion separately from overall completion, and claim the
overall outcome complete only when every acceptance criterion in the current
explicitly agreed scope is verified. A scope revision does not itself satisfy
the criteria that remain.

## Performance Programme

### Level L research exception

For the Level L research project, including the experimental B-only C/G
compilers under `research/level-l/compilers`, formal performance testing is not a
completion or publication requirement. Adrian set this scope on 15 October
2026. Do not gate research increments on a formal Release benchmark, prescribed
sample counts, AC power, or a separate performance-verdict approval. Focused
functional tests, explicit resource-limit checks and inspection of emitted-code
quality remain required. Lightweight timings or counts may inform development;
label their conditions and do not present them as formal performance claims.
Investigate observed pathological cost without automatically launching a
benchmark campaign. Source-only slices follow the consolidated sanitizer rule
above; unchanged native inputs do not require a new ASan/LSan campaign. This
exception does not waive actual sanitizer findings, native product requirements,
measured CTest scheduling, or the later explicitly agreed production-replacement
gates.

For performance-programme work, read `performance/AGENTS.md` and
`performance/ROADMAP.md` before changing benchmarks, profiling automation,
compiler/assembler optimisations, VM execution paths, or performance measurements.
Use `performance/PERFORMANCE-GOVERNANCE.md` for measurement rules and
`performance/DECISIONS.md` for durable accepted/rejected choices. Keep the
consolidated plans, durable decisions and current capability/limitation status
in HEAD.
Report observed performance results in conversation; do not preserve result
summaries, run history or raw run bundles.

Outside the Level L research exception above, after an approved production performance edit, follow the mandatory first
Release verdict in `performance/AGENTS.md` before doing broad closeout work.
Once the minimum focused correctness checks needed for safe measurement pass,
freeze implementation, build the ordinary profiling-off Release product, run
the smallest decisive end-to-end comparison against a valid baseline with
unchanged relevant inputs, report it to Adrian in conversation, and stop for
direction. The implementation remains provisional and revertable until that
verdict is accepted. Full Debug CTest, sanitizer, install/package proof, harness
refinement, follow-on PoCs, cleanup, and documentation polish come after this
decision gate, not before it.

## Linux Install Notes

- The default CMake install prefix is the user prefix, `$HOME/.local`, so a
  normal developer install should not require sudo:
  `cmake --install cmake-build-release`.
- The release install intentionally places runtime bytecode, plugins, helper
  tools, `libcrexxsaa.so`, and static archives together under `bin/`. Do not
  "tidy" installed `.a` files out of `bin` without retesting
  `crexx -native`; native packaging depends on the installed static libraries
  and `crexx_native_libs`.
- On ELF platforms, `crexxsaa` is installed with `$ORIGIN` runpath so it can
  resolve the adjacent `libcrexxsaa.so` without `LD_LIBRARY_PATH`.

## Build Performance Notes

- Full Debug CTest performs much better with deliberate test-level
  parallelism. On Adrian's macOS ARM64 development machine, `uname -srm`
  currently reports `Darwin arm64` and `sysctl -n hw.logicalcpu` reports 10
  CPUs, but broad validation works best with `ctest --test-dir
  cmake-build-debug --parallel 30 --output-on-failure` or
  `CTEST_PARALLEL_LEVEL=30`. The compiler syntax-highlighting tests are
  serialized with a CTest `RESOURCE_LOCK` because they drive parser-thread
  tooling; do not remove that lock without proving `ctest --parallel 30` remains
  stable. On slower or unknown Unix-like hosts, prefer at least `--parallel 10`
  for full or large CTest sweeps unless memory pressure or platform locking
  suggests less. Keep focused CTest runs narrow by `-R` as usual.
- On the 2026-06-17 Linux ARM64 VM, a clean `develop` Release build with
  `--parallel $(nproc)` used 6 jobs and hit swap while compiling multiple
  optimized `rxvmintp.c` variants. If Release builds look unexpectedly slow,
  check memory pressure and the number of concurrent `rxvmintp.c` compiles
  before assuming CTest or generated bytecode is the bottleneck.
- The VM core build is intentionally shared through the internal CMake object
  libraries `rxtvm_core_objects` and `rxbvm_core_objects`. Keep `NTHREADED`
  confined to the portable `rxbvm` object variant. Product `rxvm` is a
  compiler-selected symlink/copy target and must not compile a third core;
  MSVC builds only `rxbvm_core_objects`.
- VM OS/TLS support is intentionally shared through `rxvm_platform_objects`.
  That target is compiled with the configured platform TLS backend. The RXPA
  static-plugin harness VMs use `rxvm_platform_notls_objects` so their
  historical no-TLS socket behavior remains explicit and is compiled only once.
- Do not replace the object-library sharing with repeated per-target
  `rxvmintp.c` or `rxvmsock.c` source lists unless you have measured the
  Release build impact and validated Debug/Release CTest.
- On Windows, avoid overlapping build and test invocations in the same build
  tree. CTest fixtures and generated-runtime targets can rebuild and remove
  `.rxbin`/`.exe` artifacts while other `ctest`, `ninja`, `cmake`, `rxc`,
  `rxas`, `rxlink`, or VM processes still have them open, producing transient
  "file can't be removed and still exist" failures. If a build/test run is
  interrupted or times out, check for and stop leftover build/test processes
  before retrying. For generated-runtime cleanup lock failures, retry the
  relevant target after the processes are gone; use `--parallel 1` for the
  retry when the lock appears tied to parallel cleanup.

## Debugging Output Discipline

- Treat every first-party ASan, LSan, or maintained sanitizer finding as a
  repository-level blocker, even when it is independent of the change that
  exposed it.  Before continuing unrelated closeout work, give the finding a
  stable `SAN-nnn` entry in `docs/SANITIZER-WORKLIST.md` with its reproducer,
  concise diagnostic, affected revision, owner/next action, and closure test.
  Inspect temporary logs, summarize the finding in that entry, then delete the
  logs. A separate dated note is not a valid disposition.
- Do not describe a release candidate, release, or cross-platform sanitizer
  pass as complete or sanitizer-clean while a first-party `SAN-nnn` item is
  open. A bounded implementation phase may close only when Adrian explicitly
  assigns the outstanding platform proof to a named later release-QA gate. The
  phase record must name every open `SAN-nnn`, its current repair evidence and
  its release-QA owner; the handoff does not close or downgrade the item and
  does not support a sanitizer-clean or release-ready claim. "Pre-existing"
  and "independent" identify attribution; they do not lower priority or permit
  an ownerless deferral.
- Close a `SAN-nnn` item only after retaining a permanent focused regression,
  passing the same focused command in normal Debug and the maintained
  sanitizer build, and completing the broad platform sanitizer gate required
  by `docs/ai-context/CREXX_ASAN_TESTING.md`.  Unsupported platform facilities
  such as Apple LeakSanitizer must be recorded as capability limits and covered
  on a supported platform instead.
- Do not add a sanitizer suppression, test exclusion, leak-off wrapper on a
  supported platform, or time-bounded waiver for first-party code without the
  user's explicit approval.  Any approved exception remains an open,
  release-blocking `SAN-nnn` item until the underlying defect is repaired and
  requalified.
- For ASan/LSan builds or ctests, use `tools/asan-run.sh` and consult
  `docs/ai-context/CREXX_ASAN_TESTING.md`. Do not hand-run broad sanitizer
  commands unless the runner is broken or the task is specifically to debug the
  runner.
- After a sanitizer-related fix, validate the same focused build/test command
  against the normal Debug build when it is available, then rerun the same
  focused command against `cmake-build-debugasan` with the relevant sanitizer
  options. This separates ordinary command/test breakage from sanitizer-only
  findings.
- For unattended sanitizer sessions, request or reuse a saved approval for the
  `tools/asan-run.sh` command prefix, then keep build/test/status/kill actions
  inside that runner so the session does not stall on permission prompts.
- Redirect verbose or unbounded stdout/stderr to a temporary file with `mktemp`;
  inspect focused slices with `rg`, `sed` or `tail`. This is mandatory for
  verbose builds, `rxc -d*`, parser/fixed-point traces and syntax-highlighting
  debug runs. Show only the path and relevant excerpts while diagnosing.
  Delete task-created logs after reporting relevant results in conversation and
  updating any lasting contract, known limitation or active sanitizer finding
  in its owning document; never copy them into source, evidence or archive
  directories.
