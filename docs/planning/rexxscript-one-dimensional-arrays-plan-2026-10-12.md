# RexxScript One-Dimensional String Arrays

## Vision and intended outcome

Give RexxScript scripts a small, practical one-dimensional string-array
facility without changing the host `REXXSCRIPT` contract or introducing the
more invasive bracket-expression parser. The first surface uses Rexx-style
compound names such as `measurements.1`, with `measurements.0` conventionally
holding the current element count. Numeric suffixes are stored as independent
string values, and a variable suffix such as `measurements.i` resolves `i` to
support ordinary loops.

The host boundary uses the existing `EXPOSE` clause. A host `.string[]`
variable is flattened into `name.0`, `name.1`, ... for RexxScript and is
reconstructed after execution. No additional `[]` marker is added to the
`EXPOSE` clause.

## Acceptance criteria

- **AC-01:** RexxScript accepts assignments to `array.0` and
  `array.<numeric-index>` and reads those values in expressions.
- **AC-02:** A variable suffix such as `array.i` resolves the current integer
  value of `i`, so a `DO i = 1 TO array.0` loop can read each element.
- **AC-03:** Array elements are string values and preserve arbitrary text;
  missing elements read as the existing undefined-variable error rather than
  silently becoming a scalar.
- **AC-04:** A declared host `.string[]` exposed through `EXPOSE name` is
  imported as `name.0`, `name.1`, ... and updated from the RexxScript result.
- **AC-05:** Existing scalar identifiers, intrinsic calls, OUTPUT, RESULT,
  and COLLECT behavior remain unchanged.
- **AC-06:** Focused runtime and compiler-exit tests cover literal indexes,
  dynamic indexes, string values, malformed suffixes, and host array
  round-trip behavior.
- **AC-07:** User and developer documentation describe the notation and the
  type-driven `EXPOSE` array boundary.

## Implementation steps

- **STEP-01:** Extend the RexxScript name parser and assignment validation to
  recognize one dotted suffix and normalize dynamic suffixes. Serves AC-01,
  AC-02, and AC-03.
- **STEP-02:** Extend the RexxScript compiler-exit to inspect declared host
  types, flatten `.string[]` input into dotted names, and copy returned dotted
  values back into the host array. Serves AC-04 and AC-05.
- **STEP-03:** Add focused functional coverage and documentation. Serves
  AC-06 and AC-07.
- **STEP-04:** Rebuild the RexxScript runtime and compiler-exit targets and
  run the focused no-opt/opt regressions. Serves AC-05 and AC-06.

## CI repair — 12 October 2026

### Vision and intended outcome

Restore normal development CI after the RexxScript array sample was registered
in the runtime test matrix with compiler exits disabled. All six failed product
and parity lanes in Build CREXX run
[38231688994](https://github.com/adesutherland/CREXX/actions/runs/38231688994),
and the build in CodeQL run
[38231688600](https://github.com/adesutherland/CREXX/actions/runs/38231688600),
stop at `testRexxScriptArrayExpose.crexx:22` with the same syntax error. The
preceding RXPP/RexxScript push also failed before the mainframe integration.
Preserve the approved array and compiler-exit contracts, keep the regression
enabled, verify the applicable local publication gates, and publish the repair
from the explicitly requested `hotfix2` branch to remote `develop`. Preserve
unrelated local work; no language or architectural change is intended.

### Checkable acceptance criteria

1. **CI-AC01 — verified:** Retain the failing command and identify its owning
   configuration error from the hosted logs and a local reproduction.
   Release target reproduction failed with the same `-x` syntax error; enabling
   exits makes both array modes pass. The retained reproduction is in the
   [qualification receipt](../qa/rexxscript-ci-2026-10-12/README.md).
2. **CI-AC02 — verified:** The array sample builds and reports its full array
   round-trip success in both compiler/assembler modes. Retain focused Release
   and Debug CTest results; keep the ordinary API tests independent of exits.
   Align direct-exit assertions with the approved array-aware insertion cursor,
   use a non-keyword identifier to test unsupported array types, and make
   generated helper bindings explicit so repeated statements compile cleanly.
   Final focused checks pass 11/11 in Release and Debug with no warnings.
3. **CI-AC03 — verified:** The ordinary Release product build and Release
   `qa-smoke`, plus Debug `qa-optimizer-parity`, pass on the repaired inputs.
   Inspect complete logs for errors and warnings introduced by the repair.
   These gates reproduce the failed CI paths without dispatching overnight QA.
   Release product/optional builds pass; smoke passes 186/186 and Debug parity
   passes 802/802. Exact unchanged repair inputs and results are in the receipt.
4. **CI-AC04 — publication prepared:** Include the diagnosis, local qualification receipt and
   expected publication status in the substantive repair commit. Publish to
   remote `develop` and verify the remote ref; report hosted results only if
   observed, without a CI-status-only follow-up commit.
   Local prerequisites are verified. Successful publication and remote-ref
   verification fulfill the final criterion and are reported live in the chat.

### Implementation steps

1. **CI-STEP01 — complete (CI-AC01):** Read the failed hosted logs and
   owning build/docs; reproduce the array compile with exits disabled.
   `hotfix2` starts at synchronized `9548714f9b5c964ee5a7b2e040562cbae0f3ebcb`.
2. **CI-STEP02 — complete (CI-AC02):** Correct this fixture's exit
   configuration, repair stale direct-exit assertions and declare generated
   helper bindings explicitly. Depends on CI-STEP01. Initial focused runs pass
   9/11 tests in Release and Debug; the two direct-exit modes each reproduce
   three stale assertions. No runtime array assertion failed.
   Any newly identified language-design change requires Adrian's approval.
   Final repair preserves language contracts and passes every focused assertion.
3. **CI-STEP03 — complete (CI-AC02, CI-AC03):** Build the product, run focused
   array/exit/runtime regressions, then the normal Release smoke and Debug
   parity gates. Reuse passing evidence when code/test/build inputs are unchanged.
   All chosen local publication gates pass; no code/test/build input changed
   after qualification. Documentation/receipt edits reuse that evidence.
4. **CI-STEP04 — publication prepared (CI-AC04):** Complete the receipt and plan statuses,
   commit only this repair, reconcile remote advancement if any, and publish
   the qualified fix to remote `develop`. Depends on CI-STEP03 passing.
