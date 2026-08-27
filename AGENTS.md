# Repository Instructions

`AGENTS.md` is the canonical repository instruction file. Keep repository-specific agent guidance here. `GEMINI.md` should only refer back to this file so instructions do not drift.

## Project Context

`crexx` is a custom REXX-to-bytecode toolchain with four main CMake-built binaries:

1. `rxc`: compiler from REXX source to `rxas` assembly
2. `rxas`: assembler from `rxas` assembly to `rxbin` bytecode
3. `rxlink`: linker for combining one or more `rxbin` modules into a shared-pool linked image
4. `rxvm`: interpreter for register-based `rxbin` bytecode

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

## Working Rules

- For tasks that change compiler logic, syntax, scoping, or architecture, present a numbered implementation plan before editing.
- Pause for user approval before making language-design decisions, syntax changes, or architectural shifts. The user is the final authority on language direction.
- For complex bugs or crashes, start with a minimal reproducer in cREXX where practical before changing core C code.
- Run focused tests frequently during compiler work. If a change causes regressions, stop, report them clearly, and distinguish expected from unintended fallout.
- Once the required full testing has passed, do not repeat it if no code or
  test/build input has changed. Verify that with the relevant diff or tree
  hashes and reuse the retained evidence. A history-only merge or
  documentation-only commit does not by itself invalidate completed testing.
  Exact-SHA hosted workflows required for publication remain separate gates;
  they do not justify repeating unchanged local QA.
- Keep documentation in sync with code. If you uncover important undocumented behaviour or architecture, update the relevant docs as part of the change.
- Treat library development as an opportunity to validate the complete product
  toolchain. Library changes should exercise `rxc`, `rxas`, `rxlink`, and
  `rxvm`, and a library-discovered defect in any of those layers should gain a
  focused regression test when it is fixed.
- When correcting an inlining defect, preserve the supported optimized shape
  by fixing its rewrite, ownership, or copyback mechanics. Retain an ordinary
  call only when the required equivalence cannot be proved; do not use a broad
  fail-closed guard as the final fix for a representable inline shape.
- Treat source documentation tags as first-class code-adjacent assets during refactors. When rewriting or replacing `.crexx` classes, methods, plugins, or library surfaces, preserve existing `/** ... */` RexxDoc blocks and tags such as `@param`, `@parm`, `@return`, `@author`, examples, and notes wherever the documented API still exists. If behaviour, signatures, backing implementation, or return contracts change, update the tags in the same change instead of dropping them. Before large classlib or library refactors, compare relevant doc-tag coverage before and after; if tags are intentionally removed because an API is removed, call that out explicitly in the change summary.

## Performance Programme

For performance-programme work, read `performance/AGENTS.md` and
`performance/ROADMAP.md` before changing benchmarks, profiling automation,
compiler/assembler optimisations, VM execution paths, or recorded performance
evidence. The dated programme charter remains
`docs/planning/release-1/performance-programme-report-2026-07-15.md`; use the
roadmap for live status and idea capture rather than editing historical findings
into the charter.

After an approved production performance edit, follow the mandatory first
Release verdict in `performance/AGENTS.md` before doing broad closeout work.
Once the minimum focused correctness checks needed for safe measurement pass,
freeze implementation, build the ordinary profiling-off Release product, run
the smallest decisive end-to-end comparison against retained valid baseline
evidence, report it to Adrian, and stop for direction. The implementation
remains provisional and revertable until that verdict is accepted. Full Debug
CTest, sanitizer, install/package proof, harness refinement, follow-on PoCs,
cleanup, and documentation polish come after this decision gate, not before it.

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
  retained log, affected revision, owner/next action, and closure test.  A
  dated evidence note alone is not a valid disposition.
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
- When debugging anything that can emit large or unbounded output, redirect both stdout and stderr to a temp log file created with `mktemp`, then inspect that file with focused reads such as `tail`, `sed`, or `grep`.
- Apply the temp-log workflow to verbose builds, parser-mode sessions, tracing, fixed-point loops, and any command that might otherwise flood the terminal or crash the agent session.
- Treat `rxc -d*`, parser traces, fixed-point validation traces, and syntax-highlighting debug runs as mandatory temp-log cases: do not stream their raw output directly to the terminal.
- Keep the terminal-facing output short: print the temp log path, then read back only the relevant slices needed for diagnosis.
