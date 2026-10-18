# Build grammar reader plan

## Vision and intended outcomes

Provide a new packaged `rxbuild` command. It will load the generated grammar
module, read a `.build` file supplied on its command line, parse it, report
useful diagnostics for malformed input, and display the recognized top-level
operations. It then performs the declared grammar, reader, ordinary program,
source-language application compilation, native executable packaging, and
program execution phases directly. `.build` is the user-facing build
definition; it is not compiled into or wrapped by `.rxpp`.

## Acceptance criteria

- AC-01: **Met.** A packaged `rxbuild` command accepts one `.build` path and
  parses it through `lllanguage`. Evidence: CMake target build and `rxbuild`
  run.
- AC-02: **Met.** The command reports an `RXBUILD_PARSE` diagnostic and returns
  non-zero for malformed build input. Evidence: running the reader against
  `grammar/build.levell` returned `RC=1` with a line/column diagnostic.
- AC-03: **Met.** The sample contains valid `COMPILE GRAMMAR`, `COMPILE
  READER`, and `RUN` statements using supported attributes. Evidence: the
  reader printed all three `RXBUILD` records.
- AC-04: **Met.** Build instructions keep generated grammar, reader, and
  linker artifacts under the CMake build tree. Evidence:
  `tools/rxbuild/README.md` and the focused build output.
- AC-05: **Met.** A valid `.build` plan drives grammar compilation, reader
  compilation, and program execution directly from the new `build_reader`
  process and returns each phase status. Evidence: successful end-to-end
  `rxbuild` run with `RC=0`, generated grammar/reader/demo `.rxbin` artifacts,
  and direct `rxtvm` output from `build_demo`.
- AC-06: **Met.** A `.build` plan may assign string variables before build
  statements and use `${NAME}` substitution in quoted attribute values.
  Existing literal-only plans remain valid. Evidence: variable-based
  `example.build` completed all phases with `RC=0`.
- AC-07: **Met.** Undefined or malformed substitutions produce a clear
  `RXBUILD` diagnostic and a non-zero result before invoking a build tool.
  Evidence: `variables-invalid.build` reports an undefined-variable error.
- AC-08: **Met.** A `.build` plan can select regular files from a directory by
  extension and retain deterministic extensionless names. Missing directories,
  unreadable directories, and empty selections produce clear diagnostics.
  Evidence: `selection.build` selected the sorted grammar files and
  `selection-invalid.build` rejected a missing directory with `RC=4`.
- AC-09: **Met.** `DO OVER <selection> AS <name> ... END;` evaluates its body
  once per selected stem, making the loop variable available to `${NAME}`
  substitution while preserving existing single-file plans. Evidence:
  `selection.build` compiled both `build.levell` and `tiny.levell` with `RC=0`.
- AC-10: **Met.** Expanded build actions report their resolved paths and
  artifact names in an additional `RXBUILD RESOLVED` line while retaining the
  original source statement for traceability. Evidence: the selection sample
  must show resolved grammar, reader, and run paths.
- AC-11: **Met.** A selection may be marked `RECURSIVE`; it returns sorted
  extensionless paths relative to the selection root, and generated outputs
  mirror those relative directories.
- AC-12: **Met.** The selection result keyword is `BASENAMES`, replacing the
  REXX-conflicting `STEMS` spelling while preserving flat-selection behavior.
- AC-13: **Met.** `COMPILE PROGRAM` compiles a Level B `.crexx` source into
  the requested standalone `.rxbin` artifact, so a build plan can produce
  its own executable program instead of relying on a separately generated
  demo artifact. Evidence: the simple build plan completes and the generated
  `cmake-build-release/build/simple/build_demo.rxbin` runs successfully.
- AC-14: **Met.** `COMPILE APPLICATION` accepts a source-language input,
  grammar dependency, compiler dependency, and output name, invokes the
  declared language compiler, and assembles the resulting generated program
  into one standalone `.rxbin`. Evidence: the MiniBASIC application plan
  produces `cmake-build-release/minibasic/application/basic1.rxbin`; direct
  runs return 0 for both positive and negative input branches.
- AC-15: **Met.** Existing grammar, reader, run, variable, selection, and
  iteration plans retain their current behavior and diagnostics after the new
  phases are added. Evidence: the existing focused rxbuild samples continue
  to pass (`example.build`, `selection.build`, and the MiniBASIC interpreter
  plan).
- AC-16: **Met.** `CREATE EXE` packages a preceding `COMPILE APPLICATION`
  into a native executable through the existing cREXX native driver, while
  preserving the application `.rxbin` as an independently runnable artifact.
  Evidence: the prime application plan produced `prime.exe`, and both
  `prime.exe` and `prime.rxbin` ran successfully with the prime output.
- AC-17: **Met.** `CREATE EXE` reports resolved output paths, rejects an
  unknown application reference or missing generated application source with
  a clear `RXBUILD` diagnostic, and does not alter existing plans that omit
  the new phase. Evidence: focused positive and negative rxbuild plans.
- AC-18: **Met.** `CREATE EXE` derives the native output filename from the
  host platform, producing `<OUTPUT>.exe` on Windows and an extensionless
  `<OUTPUT>` on Linux/macOS, matching the existing cREXX native driver.
  Evidence: Windows `prime.exe` verification plus inspection of the
  platform-specific path branch in the cREXX driver.
- AC-19: **Met locally.** Building `rxbuild_example` alone in a clean parallel build
  produces its required `library.rxbin` and certified `rxcexits.rxbin` before
  compiling the demo. Missing or changed import inputs rebuild the demo;
  unchanged inputs do no work. Verified
  the actual component declarations with Ninja and Unix Makefiles, real tools,
  and execution of the resulting demo. Preserve the normal product build and
  existing rxbuild error-path tests.

## Implementation steps

- STEP-01: **Complete.** Add the Level B reader/command source and sample plan. Serves AC-01,
  AC-02, AC-03.
- STEP-02: **Complete.** Add CMake packaging and README build/run instructions.
  Serves AC-04.
- STEP-03: **Complete.** Generate, assemble, package, and run `rxbuild` against
  valid and malformed inputs. Serves AC-01 through AC-04.
- STEP-04: **Complete.** Implement the three build phases directly in
  `build_reader`, remove the obsolete RXPP runner integration, package the
  non-recursive demo target, and qualify the end-to-end result. Serves AC-05.
- STEP-05: **Complete.** Add top-level string assignments and `${NAME}`
  substitution while preserving literal plans. Serves AC-06 and AC-07;
  file selection and iteration remain out of scope for this phase.
- STEP-06: **Complete.** Add `FILES IN ... EXTENSION ... BASENAMES` selection
  and `DO OVER ... AS ... END;` expansion over existing build actions. Serves
  AC-08 and AC-09; recursive selection and automatic reader/program pairing
  remain out of scope for this phase.
- STEP-07: **Complete.** Report resolved paths and artifact names for each expanded
  action without changing execution semantics. Serves AC-10.
- STEP-08: **Complete.** Add `RECURSIVE` selection, preserve relative
  subdirectories in the selected names, and create mirrored output directories.
  Serves AC-11.
- STEP-09: **Complete.** Replace the public `STEMS` selection keyword with
  `BASENAMES`, update the examples and documentation, and verify flat
  selection compatibility. Serves AC-12.
- STEP-10: **Complete.** Extend the build grammar and reader with
  `COMPILE PROGRAM`, including resolved diagnostics and the normal compiler /
  assembler pipeline. Serves AC-13 and preserves the existing phase model.
- STEP-11: **Complete.** Add `COMPILE APPLICATION` as a language-compiler
  phase with explicit grammar/compiler dependencies and a generated-source
  assembly boundary. Serves AC-14.
- STEP-12: **Complete.** Update the simple and MiniBASIC plans and user
  documentation, then rerun focused legacy and new end-to-end samples.
  Serves AC-15.
- STEP-13: **Complete.** Add `CREATE EXE` to the build grammar and reader,
  associating its `APPLICATION` attribute with a prior application action and
  invoking the existing cREXX native packaging path. Serves AC-16 and AC-17;
  preserve the `.rxbin` application output.
- STEP-14: **Complete.** Extend the prime sample and rxbuild documentation with
  native packaging instructions, then verify the executable and bytecode
  outputs independently. Serves AC-16 and AC-17.
- STEP-15: **Complete.** Replace the CREATE EXE reader's Windows-only suffix
  assumption with the existing cREXX platform convention and update resolved
  path diagnostics/documentation. Serves AC-18.
- STEP-16: **Complete.** Repair the reader/demo's missing target and file
  dependencies on the Level B library and certified exits, add a focused
  clean/incremental build regression,
  measure it in isolation in Debug and the maintained sanitizer tree before
  registering its CTest scheduling, and qualify the product/error paths.
  Serves AC-19; no compiler or language change is required.

## Current build repair

The [CodeQL build for `2796992b2`](https://github.com/adesutherland/CREXX/actions/runs/39053829195/job/117457631839)
failed before analysis when the clean Ninja build compiled `build_demo.crexx`
without `bin/library.rxbin`: `EXIT_MODULE_LOAD_ERROR`. The demo depended on
`rxc` and `rxas`, but not on the library it imports. The isolated reproduction
also exposed the next missing prerequisite, certified `rxcexits.rxbin`. Both
ordinary compiler invocations in this component need those producers and
their file freshness dependencies. The intended outcome is a
self-contained, correctly ordered demo target, including parallel builds and
incremental freshness; unrelated CMS/TSO cleanup and active work stay intact.
Local checks and normal hosted workflows remain separate qualification gates.

Validation of this repair, based on `2796992b24f2e555c21ae2c8103c19135bdba4c1`:

- Before the dependency fix, `check-demo-build.py` reproduced the hosted
  missing-library error. Adding the library alone reproduced the missing-exits
  error. With both producers declared, the unchanged demo compiled and ran.
- `cmake --build cmake-build-debug --target stage-product --parallel 8` passed
  without warnings. No compiler/runtime source or bytecode input changed in
  this repair; the subsequent CTest registration does not change product code.
- `ctest --test-dir cmake-build-debug --parallel 1 --output-on-failure -R
  '^rxbuild_'` passed all three tests: the new build-order fixture and the
  existing missing/blocked-directory regressions.
- The isolated fixture passed with Debug/Ninja tools in about 6 seconds and
  the already-built maintained Apple-ASan/Make tools in 17 seconds through
  `tools/asan-run.sh --phase build --build-target rxbuild_demo_order_measure
  --build-jobs 1 --build-leaks off`. It exercises the actual component CMake
  declarations with private import producers, both missing/changed images,
  no-op rebuilds and five demo executions. This is scheduling and functional
  evidence, not a fresh full sanitizer build or performance claim. Apple LSan
  is unavailable. CTest uses `RUN_SERIAL`, a 600-second hang limit and explicit
  tool/import preparation targets.
- Normal push-triggered Build CREXX and CodeQL are expected to verify the
  repair; local success does not establish their result. No extra hosted matrix
  or status-only follow-up commit is required. Temporary diagnostics are deleted.

## Explicit RXBUILD-only RXPP sources

### Vision and intended outcomes

Allow an RXPP source to declare that it is an application input for a
controlled `.build` pipeline rather than a directly compilable standalone
source. A `##RXBUILD REQUIRED` declaration stops direct RXPP invocation with
an actionable diagnostic, while `rxbuild` passes an explicit internal mode
option so its own RXPP phase can process the same source. The central `crexx`
driver remains unchanged.

### Acceptance criteria

- AC-38: RXPP recognizes `##RXBUILD REQUIRED` and rejects direct preprocessing
  without build mode, with a diagnostic naming the source that requires the
  associated `.build` plan.
- AC-39: RXPP accepts the declaration with the internal `--rxbuild` option,
  and `rxbuild` supports `COMPILE RXPP` for the controlled phase.
- AC-40: A following `COMPILE PROGRAM` compiles the generated source with the
  output directory on its import path, and `RUN` supplies that directory to
  the VM for external modules.
- AC-41: Direct failure, controlled report-writer success, and existing simple
  plan regression checks pass without changing the central `crexx` driver.

### Implementation steps

- STEP-34: **In progress.** Add RXPP mode parsing, the `##RXBUILD REQUIRED`
  validation, and the stable diagnostic catalog entries. Serves AC-38.
- STEP-35: **In progress.** Restore `COMPILE RXPP` in the current
  `tools/rxbuild` grammar and reader, pass `--rxbuild`, and mark the
  report-writer source. Serves AC-39 and AC-40.
- STEP-36: Update user and component documentation, rebuild the packaged tools,
  and run direct-failure, controlled-success, and regression checks. Serves
  AC-41.
