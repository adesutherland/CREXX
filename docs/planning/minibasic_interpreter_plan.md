# MiniBASIC Reader and Interpreter Plan

## Vision and intended outcome

Turn the MiniBASIC Level B reader into a small executable interpreter for the
language slice already described by `examples/minibasic/grammar/minibasic.levell`.
Running the linked reader with a `.bas` file should execute numbered statements
and produce the program's `PRINT` output, while retaining the current parser
diagnostics and parse-tree behavior for invalid input.

The first phase deliberately covers only the constructs already used by
`basic1.bas`: variables, integer and string values, arithmetic and comparison
expressions, `LET`, optional `LET`, `PRINT`, `INPUT`, `IF ... THEN`, `GOTO`, and
`END`. Arrays, `FOR/NEXT`, subroutines, real values, and multiple statements
per line remain future extensions.

## Acceptance criteria

- **AC-01 (verified)** — A valid MiniBASIC source file is parsed using the generated Level L
  grammar before execution. Evidence: the linked reader accepts `basic1.bas`.
- **AC-02 (verified)** — Numeric assignments and arithmetic expressions update scalar
  variables. Evidence: `basic1.bas` calculates and prints `55`.
- **AC-03 (verified)** — `IF ... THEN` and `GOTO` transfer control using source line
  numbers. Evidence: the sample loop reaches line 100.
- **AC-04 (verified)** — `PRINT` emits strings and evaluated numeric expressions in list
  order. Evidence: the sample prints the sum and prompt text.
- **AC-05 (verified)** — `INPUT` reads a value from standard input and stores it in
  the named variable. Evidence: runs with `5` and `-5` exercise both branches.
- **AC-06 (open)** — Unsupported or malformed runtime constructs produce a clear
  `MINIBASIC_EXEC` diagnostic and non-zero return status.
- **AC-07 (verified)** — Existing parse-error reporting remains intact from the
  parser-only reader validation.

## Implementation steps

- **STEP-01 (complete)** — Add interpreter state for scalar string/numeric variables and a
  source-line-number table. Serves AC-02 and AC-03.
- **STEP-02 (complete)** — Add expression evaluation for literals, variables, grouping,
  arithmetic, and comparisons. Serves AC-02 and AC-03.
- **STEP-03 (complete)** — Add statement execution for assignment, output, input, branch,
  and termination. Serves AC-02 through AC-05.
- **STEP-04 (partial)** — Run the sample plus both input branches. AC-06 remains open;
  parser diagnostics and the successful execution evidence are retained.

## Scope gate

This is the intentionally small first interpreter slice. Additional language
features require a separate scope decision after the sample executes correctly.

## Application compiler extension

### Vision and intended outcome

Add the first direct MiniBASIC application path without changing the existing
reader/interpreter contract. A `.bas` source selected by an rxbuild plan is
parsed with the generated MiniBASIC grammar, translated into a generated cREXX
program containing the supported MiniBASIC control flow and expressions, and
then compiled and assembled into one deliverable `.rxbin`. A separate
`CREATE EXE` phase may package that application through the existing cREXX
native driver. The resulting application must run without the reader or
grammar modules being linked into the final image; the reader remains
available for interactive/debugging use.

### Acceptance criteria

- **AC-C01 (verified)** — The MiniBASIC compiler accepts the current grammar slice
  and reports `MINIBASIC_COMPILE` diagnostics for invalid input.
- **AC-C02 (verified)** — `basic1.bas` is translated and assembled to a standalone
  `basic1.rxbin` through `COMPILE APPLICATION`.
- **AC-C03 (verified)** — The generated application prints `55`, accepts standard
  input, and selects the same branch as the interpreter for both positive and
  negative input.
- **AC-C04 (verified)** — The generated application does not require
  `minibasic_reader.rxbin` or `minibasicgrammar.rxbin` at runtime.
- **AC-C05 (verified)** — The prime application can be packaged as a native
  `prime.exe` while its `prime.rxbin` remains independently runnable. Evidence:
  the prime rxbuild plan produced and ran both artifacts successfully.

### Implementation steps

- **STEP-C01 (complete)** — Factor the parse-tree expression/statement model
  into a compiler that emits a temporary cREXX application source.
- **STEP-C02 (complete)** — Add the rxbuild application phase and invoke the
  compiler, cREXX compiler, and assembler in sequence.
- **STEP-C03 (complete)** — Add the application build sample and verify output,
  runtime behavior, and missing/invalid-source diagnostics.
- **STEP-C04 (complete)** — Add the rxbuild `CREATE EXE` phase to the prime
  application sample and verify native and bytecode execution independently.
