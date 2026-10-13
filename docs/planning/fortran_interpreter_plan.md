# FORTRAN IV Interpreter for cREXX — Initial Plan

## Vision and intended outcome

Provide a practical FORTRAN IV/FORTRAN 66 interpreter implemented in one
Level G cREXX module. The initial host program embeds the guest FORTRAN source
in an RXPP source block, prepares it into an action table, and executes it
with an explicit guest program counter. FORTRAN labels and jumps remain guest
language data; they do not depend on cREXX source-level labels or `SIGNAL`.

The first phase targets one main program with cREXX integer/float values,
simple extendable arrays, `PULL`/`SAY` I/O, and cREXX signals for interpreter
errors. The state model must leave room for later procedures, pass-by-reference
calls, and richer bounds without changing the interpreter core.

## Acceptance criteria

- **AC-01:** An RXPP `##DATA fortran_source ... ##END` block is available to the
  interpreter as ordered source lines, with original guest line numbers.
  **Status:** satisfied for the embedded line array; runtime source-line
  records remain part of STEP-02.
- **AC-02:** Preparation recognises fixed-format comments, continuation lines,
  numeric statement labels, and the initial executable subset. **Status:**
  satisfied for comments, continuations, labels, declarations, `CONTINUE`,
  `GOTO`, assignment, `STOP`, and `END`.
- **AC-03:** Preparation builds a label table and rejects duplicate or missing
  label references before execution. **Status:** satisfied by the preparation
  pass and resolved action targets.
- **AC-04:** Execution supports scalar integer/float assignment, arithmetic,
  `CONTINUE`, unconditional `GOTO`, and `STOP` through an explicit guest `pc`.
  **Status:** satisfied for scalar literals, variables, binary arithmetic,
  arrays, labeled `DO`, arithmetic `IF`, and list-directed `WRITE` in the
  current matrix slice; richer precedence remains open.
- **AC-05:** Guest control transfers preserve the interpreter state; runtime
  failures use cREXX signals with the guest source line and label where known.
  **Status:** guest `GOTO` preserves the variable table; failure routing uses
  a cREXX `failure` signal.
- **AC-06:** The state representation has explicit extension points for arrays,
  DO frames, and a future guest call stack without requiring cREXX label jumps.
  **Status:** arrays and DO frames are now explicit; guest call frames remain
  deferred.
- **AC-07:** A focused regression demonstrates embedded FORTRAN execution and
  a numerical result; malformed input and an unresolved label are tested.
  **Status:** positive matrix execution is verified; permanent malformed-input
  and unresolved-label regressions remain open.
- **AC-08:** RXPP accepts a plain `FORTRAN RUN ... END FORTRAN` block and
  emits the ordered `fortran_source` array and invokes the imported
  interpreter.
  **Status:** implementation and regression are added; verification awaits a
  rebuilt `rxpp` binary.
- **AC-09:** Labeled `DO` accepts an optional step expression, including
  negative steps, and terminates correctly in either direction.
  **Status:** satisfied. `fortran_features.rxpp` verifies both the default
  positive step and `DO 200 I = 3,1,-1`.
- **AC-10:** The expression evaluator supports the first numeric intrinsics:
  `ABS`, `MIN`, `MAX`, `SQRT`, `INT`, and `MOD`.
  **Status:** satisfied. `fortran_features.rxpp` executes all six intrinsics
  and formats their numeric results.
- **AC-11:** One-line logical `IF` supports the standard relational operators
  and executes a single `STOP` or `GOTO` statement when true.
  **Status:** satisfied. Relational operators plus `.AND.`, `.OR.`, and
  `.NOT.` are evaluated, and one-line `STOP`/`GOTO` actions are supported.
- **AC-12:** List-directed `WRITE` accepts quoted character literals and keeps
  commas inside those literals as text. **Status:** satisfied by the
  `fortran_features.rxpp` regression.
- **AC-13:** Block `IF (...) THEN`, optional `ELSE`, and `END IF` select and
  skip action ranges while preserving nested block structure. **Status:**
  satisfied by explicit action targets and the feature sample's true branch,
  false branch, and `END IF` execution.
- **AC-14:** `PARAMETER` defines named numeric constants usable in expressions
  and array bounds without becoming mutable variables. **Status:** satisfied by
  `NMAX` and `SCALE` in the feature sample.
- **AC-15:** Fixed-width `CHARACTER*n` variables support literal assignment,
  list-directed `WRITE`, and labeled `A` format descriptors. **Status:**
  satisfied by `TITLE` and `NAME` in the feature sample.
- **AC-16:** Character expressions support `LEN`, `INDEX`, substring reads and
  writes, and `//` concatenation. **Status:** satisfied by the enhanced
  `fortran_features.rxpp` regression.
- **AC-17:** RXPP accepts `FORTRAN PROCEDURES ... END FORTRAN` blocks and
  combines them with `FORTRAN RUN` into one deferred runtime source call.
  **Status:** satisfied. The focused `rxpp_fortran_block` regression verifies
  procedure text is appended and the single runtime call follows it.
- **AC-18:** The interpreter indexes multiple `SUBROUTINE` units and supports
  scalar `CALL`/`RETURN` with caller resumption and copy-in/copy-out arguments.
  **Status:** satisfied for the initial scalar slice. `fortran_call.rxpp`
  verifies `CALL DOUBLE_VALUE(I)` returns and updates `I` from 4 to 8.
- **AC-19:** Scalar calls may pass an array element actual argument, and
  formatted output supports `X` spacing and quoted literal text descriptors.
  **Status:** satisfied. `fortran_call.rxpp` verifies `VALUES(I)` copy-in/
  copy-out and `1X` plus quoted labels in the FORMAT statements.
- **AC-20:** An RXPP FORTRAN source may bind a host typed cREXX array with
  `C$RX BIND target = host`; the host cardinality is read from `host[0]` and
  values are imported from `host[1]` through `host[host[0]]`. The binding
  rejects missing, empty, oversized, or malformed host data with a FORTRAN
  diagnostic. A focused sample verifies a `.float[]` host array.
- **AC-21:** An RXPP FORTRAN source may export one or more declared FORTRAN
  arrays with `C$RX EXPORT target = host`; after execution, the host typed
  cREXX arrays contain the exported values in `[1..count]`. A focused sample
  verifies two output arrays and preserves the input-binding behavior.

## Implementation steps

- **STEP-01:** Add the initial Level G host module and embedded RXPP source sample;
  serves AC-01 and AC-07. **Complete:**
  `examples/fortran_interpreter.rxpp` expands and compiles, and the assembled
  smoke run prints `FORTRAN_SOURCE_LINES 9` and the first guest source line.
- **STEP-02:** Define the guest state, source-line records, action records, and
  label table; serves AC-03 and AC-06. **Complete:** parallel action/label/
  variable arrays and an explicit guest `pc` are present.
- **STEP-03:** Implement fixed-format preparation and the initial expression/
  assignment parser; serves AC-02 and AC-04. **Complete:** comments,
  continuations, declarations, assignment, `GOTO`, `CONTINUE`, `STOP`, and a
  first scalar expression evaluator are implemented.
- **STEP-04:** Implement the execution loop and explicit control results
  (`NEXT`, `JUMP`, `STOP`, and error); serves AC-04 and AC-05. **Complete:**
  the guest-PC loop executes the embedded sample and produces `I = 3`.
- **STEP-05:** Add focused positive and negative regressions and run the
  smallest relevant compiler/runtime checks; serves AC-02 through AC-07.
  **In progress:** the matrix program now passes the manual RXPP → `rxc` →
  `rxas` → `rxvm` smoke; permanent malformed-input and unresolved-label
  regressions remain.
- **STEP-06:** Add the plain `FORTRAN RUN ... END FORTRAN` RXPP collector,
  documentation, and a preprocessor regression; serves AC-08. **Implemented:**
  source and test are present; verification is pending the user's rebuilt
  `rxpp` binary.
- **STEP-07:** Introduce the runtime module boundary while keeping the
  implementation procedures in the example source; serves AC-04 through
  AC-06. **Implemented:** `fortran_run(source)` is the sole exported entry
  point and owns the per-run interpreter state; the `fort_*` helper procedures
  expose that caller state.
- **STEP-08:** Extract `fortran_run(source)` and its helper procedures into
  `examples/Fortran/fortran.crexx`; import that module from the RXPP sample;
  serves AC-04 through AC-06. **Implemented:** source separation is complete;
  rebuilt RXPP/compiler/runtime verification remains open.
- **STEP-09:** Make `FORTRAN RUN ... END FORTRAN` emit the runtime invocation
  automatically, removing the explicit call from samples; serves AC-08.
  **Implemented:** `END FORTRAN` now emits `call fortran_run fortran_source`.
- **STEP-10:** Report elapsed interpreter time when `fortran_run` completes;
  serves AC-04 and AC-07. **Implemented:** the module prints elapsed
  microseconds with the stable `FORTRAN_ELAPSED_US` tag.
- **STEP-11:** Add labeled `FORMAT` and formatted `WRITE` output; serves
  AC-04 and AC-07. **Implemented:** the first `Fw.d` and repeated-descriptor
  slice, including `3F10.3`, is available; list-directed output remains
  supported.
- **STEP-12:** Remove the temporary `FORTRAN_RESULT I` smoke diagnostic;
  serves AC-07. **Implemented:** runtime output now contains only guest
  `WRITE` output and the `FORTRAN_ELAPSED_US` timing line.
- **STEP-13:** Support the common FORTRAN implied-DO output item, such as
  `WRITE (*,300) (C(I,J),J = 1,3)`; serves AC-04 and AC-07. **Implemented:**
  implied-DO values are expanded and passed through the selected `FORMAT`.
- **STEP-14:** Add integer `Iw` descriptors to formatted output; serves
  AC-04 and AC-07. **Implemented:** `I10` and repeated integer descriptors are
  accepted alongside `Fw.d`.
- **STEP-15:** Optimize labeled `DO` execution by caching each loop variable's
  variable-table index; serves AC-04 and AC-07. **In progress:** implementation
  is applied and benchmarked: the 1,000,000-iteration sample improved from
  3.070417 seconds to 2.746918 seconds, approximately 10.5% faster.
- **STEP-16:** Optimize repeated scalar increment/decrement assignments such
  as `SUM = SUM + 1`; serves AC-04 and AC-07. **Complete:** the direct fast
  path reduced the amended 1,000,000-iteration benchmark from 17.667296 to
  9.388936 seconds, approximately 46.8% faster.
- **STEP-17:** Add optional and negative labeled-`DO` steps; serves AC-09.
  **Complete:** omitted steps default to `1`; explicit positive and negative
  steps select the corresponding termination direction, with zero rejected.
- **STEP-18:** Add the first numeric intrinsic functions; serves AC-10.
  **Complete:** `ABS`, `MIN`, `MAX`, `SQRT`, `INT`, and `MOD` are recognized
  as expression atoms, including nested intrinsic arguments.
- **STEP-19:** Add one-line logical `IF`; serves AC-11. **Status:** planned.
  **Complete:** relational `.EQ.`, `.NE.`, `.LE.`, `.GE.`, `.LT.`, and `.GT.`
  conditions execute one-line `STOP` or `GOTO` actions.
- **STEP-20:** Add quoted text items to list-directed `WRITE`; serves AC-12.
  **Complete:** single- and double-quoted literals are emitted by list-directed
  `WRITE`; commas inside quoted items do not split the output list.
- **STEP-21:** Complete logical conditions with `.AND.`, `.OR.`, and `.NOT.`;
  serves AC-11. **Complete:** logical operators are recursively evaluated with
  `.OR.` and `.AND.` precedence and unary `.NOT.`.
- **STEP-22:** Add block `IF`, `ELSE`, and `END IF` action targets; serves AC-13.
  **Complete:** preparation pairs nested blocks and execution selects the true
  range or skips to the matching `ELSE`/`END IF` target.
- **STEP-23:** Complete and regression-test the intrinsic-function slice; serves
  AC-10. **Complete:** all six numeric intrinsics are exercised together with
  the new control-flow sample.
- **STEP-24:** Add numeric `PARAMETER` constants and use them in expressions
  and dimensions; serves AC-14. **Complete:** parameter values are resolved in
  expressions and array dimensions and assignment to them is rejected.
- **STEP-25:** Add fixed-width character variables and related `A` formats;
  serves AC-15. **Complete:** character declarations, padded/truncated literal
  assignment, list-directed output, and `A`/`An` descriptors are implemented.
- **STEP-26:** Add the first character expression operations; serves AC-16.
  **Complete:** `LEN`, `INDEX`, `NAME(start:end)` reads and writes, and `//`
  concatenation are implemented with fixed-width assignment semantics.
- **STEP-27:** Add the RXPP `FORTRAN PROCEDURES` free-form block and deferred
  source combination; serves AC-17. **Complete:** `FORTRAN RUN` and
  `FORTRAN PROCEDURES` append to one source array, with one deferred runtime
  call after collection.
- **STEP-28:** Add the initial subroutine table and call stack; serves AC-18.
  **Complete:** multiple `SUBROUTINE` units are indexed; `CALL`, `RETURN`,
  implicit procedure-end return, and scalar copy-in/copy-out are implemented.
  Full procedure-local scope and array aliasing remain follow-up refinements.
- **STEP-29:** Add array-element call descriptors and `FORMAT` `X`/literal
  descriptors; serves AC-19. **Complete:** scalar subroutine calls can bind an
  array element actual, and formatted output accepts spacing and quoted text.
- **STEP-30:** Add RXPP-to-interpreter host-array bindings using the
  `C$RX BIND` directive and typed-array `[0]` cardinality semantics; serves
  AC-20. **In progress:** add generated binding payloads, runtime import and
  focused regression coverage.
- **STEP-31:** Add post-run `C$RX EXPORT` array copy-out using a packed result
  payload and generated typed-array assignments; serves AC-21. **Planned.**
  **In progress:** runtime result packing and RXPP copy-out generation are
  implemented; compiler/runtime verification remains open.

## Next incremental roadmap

The next features are intentionally small FORTRAN additions that fit the
existing action-table interpreter. Each can be implemented and tested without
introducing a call stack or changing the guest-state model.

- **NEXT-01 — labeled `FORMAT` and formatted `WRITE` (implemented):** recognize
  labeled `FORMAT` statements and support `Fw.d` plus repeated descriptors
  such as `3F10.3`. Map them to the existing output path and retain
  list-directed `WRITE (*,*)`. Integer `Iw` descriptors remain open.
- **NEXT-02 — sequential text-file input:** add the small `OPEN`, list-directed
  `READ`, and `CLOSE` slice for sequential text files. The implementation may
  use cREXX `linein`/`lines` internally, but the guest interface remains
  FORTRAN unit-based and does not map `READ` to interactive `PULL`.
- **NEXT-03 — logical `IF`:** implemented, including `.EQ.`, `.NE.`, `.LT.`,
  `.LE.`, `.GT.`, `.GE.`, `.AND.`, `.OR.`, and `.NOT.`.
- **NEXT-04 — block `IF`:** implemented with explicit action targets for
  `IF (...) THEN`, `ELSE`, and `END IF`.
- **NEXT-05 — easy intrinsic functions:** `ABS`, `MAX`, `MIN`, `SQRT`, `INT`,
  and `MOD` are implemented and covered by the feature sample.
- **NEXT-06 — simple `PARAMETER`:** implemented for named numeric constants in
  expressions and array bounds. `COMMON`, `EQUIVALENCE`, and historical
  storage rules remain deferred.
- **NEXT-07 — character slice:** implemented for fixed-width character values
  and `A` output descriptors. Substrings, concatenation, and broader character
  editing remain deferred.
- **NEXT-08 — character expressions:** implemented for `LEN`, `INDEX`,
  substrings, and concatenation while retaining fixed-width assignment
  semantics.
- **NEXT-07 — regression examples:** maintain one small example per feature
  and retain the matrix program as the combined integration sample.

Deferred until these add-ons are stable: interactive console input as the
default `READ` path, `CALL`/`RETURN`, `SUBROUTINE`,
function procedures, `COMMON`, `EQUIVALENCE`, `FORMAT` character descriptors,
and full FORTRAN input/output control lists.

Deferred by decision: external procedures, `CALL`/`RETURN`, `COMMON`,
historical numeric widths, advanced array bounds, complete `FORMAT`, and the
parameterized FORTRAN block names and callback forms.
