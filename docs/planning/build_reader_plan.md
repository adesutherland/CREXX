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
  `tools/build/README.md` and the focused build output.
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
