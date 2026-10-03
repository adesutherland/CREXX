# Level C compatibility worklist

Status: active component worklist under [the Release 1 plan](../../release-1-plan.md).
Started 2026-10-03 on `develop`. The Release 1 plan owns the Beta 4 completion
contract (`R1-AC-01/02`) and the roadmap owns portfolio order. This worklist
records the coverage and incremental implementation evidence; it does not
change either scope or the 2026-11-30 target.

## Vision and intended outcome

Compile and execute Classic REXX through `rxc`, `rxas`, `rxlink`, and `rxvm`
with the syntax, scalar and numeric rules, variable-pool behavior, control
flow, routines, built-in functions, conditions, diagnostics, source services,
and host interfaces described in the [compliance reference](../../../compiler/docs/levelc_compliance_reference.md)
and [BIF reference](../../../compiler/docs/levelc_classic_bifs.md).
Keep the BYTE and opt-in UTF8 configuration boundary, source provenance,
Classic error identities, and fail-closed handling for shapes not yet proved.
Preserve Level B behavior and the separate RexxScript sandbox. An exclusion
counts only after Adrian individually approves its reason, user-visible
behavior, and documentation; unfinished work remains open.

The first delivery increments are a coverage inventory, then executable
`IF/THEN/ELSE`, then simple `DO ... END`. They do not complete the Beta 4
contract. Later increments are selected from the open coverage rows rather
than redefining compatibility around the first slices.

## Acceptance criteria

- [ ] **LC-AC-01 — complete inventory (R1-AC-01):** every statement,
  expression, BIF, numeric context, variable-pool/stem, procedure/call,
  PARSE, condition/signal, source/TRACE, and host-interface feature in the
  references has a row with implementation state, evidence or open work, and
  a platform/configuration boundary. Proposed exceptions remain open until
  individually approved. Verify by reference-to-matrix reconciliation.
- [x] **LC-AC-02 — IF execution:** supported `IF expression THEN instruction
  [ELSE instruction]` evaluates the condition once, selects the correct arm,
  binds `ELSE` correctly in nested forms, and enforces Classic exact logical
  values. Unsupported branch statements still fail closed. Verify with
  Classic reference output/error comparisons, positive and negative CTests,
  opt/no-opt runs, lowered-tree inspection, and `rxc`/`rxas`/`rxlink`/`rxvm`
  execution.
- [x] **LC-AC-03 — simple DO execution:** supported `DO ... END` executes its
  body once with correct statement order and nested `IF`/`DO` behavior;
  controlled/repetitive forms, `LEAVE`, and `ITERATE` remain open until their
  semantics are separately proved. Verify with Classic reference comparisons,
  positive and negative CTests, opt/no-opt, lowered-tree inspection, and the
  full toolchain.
- [x] **LC-AC-05 — NOP execution:** a childless Classic `NOP` has no visible
  effect in main, local procedures, and supported `IF`/`DO` bodies; adjacent
  statements retain their order and unsupported statement shapes still fail
  closed. Verify against the Classic interpreter, focused optimized/no-opt
  runs, the relevant normal Level C suite, and linked-image execution.
- [ ] **LC-AC-04 — full compatibility (R1-AC-02):** every required matrix row
  has executable behavior and documentation, or an individually approved
  exception with its diagnostic and user-visible limit. Verify reference
  equivalence, supported-platform and BYTE/UTF8 configuration behavior,
  optimized/no-opt parity, errors/conditions, lifecycle, toolchain, and
  packaging evidence on the exact Beta 4 candidate. This remains open after
  the first control-flow increments.

## Implementation steps

1. **LC-STEP-01 (LC-AC-01):** reconcile the two references, the compiler's
   Classic BIF recognition inventory, runtime modules, compiler lowering,
   and registered tests into the matrix below. Link this worklist from the
   Release 1 plan. Commit this inventory increment. Detailed compatibility
   choices and exceptions require Adrian's decision.
2. **LC-STEP-02 — complete (LC-AC-02):** create a small Classic `IF` reproducer and
   target-shape baseline; implement guarded AST lowering using the existing
   remap builder, preserving one condition evaluation and source anchors.
   Add focused positives, negatives, and reference/opt/no-opt/toolchain
   evidence. Commit only when the normal correctness gate passes.
3. **LC-STEP-03 — complete (LC-AC-03; depends on STEP-02):** add the simple `DO` shape
   and nested cases, retain fail-closed gates for loop variants, qualify as
   above, then commit separately.
4. **LC-STEP-04 (LC-AC-01/04; depends on the approved matrix):** select and
   implement the remaining feature rows in reviewable increments, committing
   each qualified increment. Get Adrian's approval before any new language
   syntax, compatibility exception, or architectural change.
5. **LC-STEP-05 (LC-AC-04):** close the exact-candidate matrix with reference,
   correctness, supported-platform, package and release evidence; update
   user and compiler documentation. This is a release gate, not a substitute
   for normal focused development checks.
6. **LC-STEP-06 — complete (LC-AC-05; depends on STEP-03):** recognize only the childless
   Classic `NOP` AST in the supported statement validator, emit a canonical
   source-anchored `NOP` in main and procedure contexts, and compare a nested
   fixture with the Classic interpreter. Run focused opt/no-opt and normal
   Level C tests, verify the linked image, then commit this bounded increment.

## Coverage matrix

Inventory pass 2: the 70 recognized BIF names and 36 existing syntax catalogue
contracts have individual rows below. The
[reference-obligation appendix](levelc-reference-obligations.md) adds 75
finer non-BIF rows, including configuration, error and host behavior. Complete
reference-to-row reconciliation and feature-level evidence remain open under
`LC-AC-01`; the rows are a coverage map, not a conformance verdict.

States are **slice** (bounded end-to-end execution), **front end** (parsed or
diagnosed, not generally executable), **runtime** (standalone Classic helper,
not general Level C compilation), and **open** (not yet evidenced). These are
implementation observations, not claims of full conformance. Every row
remains open for LC-AC-04 until qualified or explicitly excepted. No exception
has been approved in this worklist.

| Area | Feature | Current state and evidence | Remaining proof |
| --- | --- | --- | --- |
| Source | comments, clauses, literals, symbols, contextual keywords, labels, continuations, source characters | Front end: `levelc_syntax_highlighting.md` | Reference edge cases, configured character/length limits, diagnostics |
| Expressions | precedence, arithmetic, comparisons, concatenation, prefix, short-circuit logic | Slice: `levelc_slice6_expressions` | Full numeric context, boundary/error and platform equivalence |
| Variables | scalar read/write, drop, compound names, bare stems, exposure, API pool | Slice: scalar/compound read/write and scalar/stem EXPOSE | Remaining stem value, DROP, indirect/external/API operations and aliasing |
| Control | IF/THEN/ELSE | Slice: `levelc_slice7_if_else`, nested and procedure fixtures, opt/no-opt, invalid logical and unsupported-arm tests | Other instructions in arms and broader condition/message lifecycle remain open |
| Control | simple DO/END | Slice: `levelc_slice8_do_block` and nested/empty/procedure fixtures, opt/no-opt, tree-shape and linked execution | Broader clause lifecycle and conditions remain open |
| Control | controlled/repetitive DO, WHILE/UNTIL, LEAVE/ITERATE | Front end: parser/validation fixtures | Execution, exact loop semantics and errors |
| Control | SELECT/WHEN/OTHERWISE | Front end: parser fixtures | Execution and condition errors |
| Control | NOP | Slice: `levelc_slice9_nop` in main, local procedure, and IF/DO bodies | Full source/TRACE lifecycle and configuration proof open |
| Routines | labels, local/external CALL and functions, ARG, PROCEDURE EXPOSE, RETURN, EXIT | Slice: bounded local calls, fixed ARG, scalar/stem EXPOSE, RETURN and empty EXIT | Omitted arguments, dynamic/external calls, full scope and return/exit lifecycle |
| PARSE | ARG, PULL, SOURCE, LINEIN, VERSION, VALUE, VAR; templates and UPPER | Front end: parser fixtures | Runtime source acquisition, template assignment, errors |
| Environment | ADDRESS, command clauses, WITH redirection | Front end: parser/validation | Configured command/stream service and RC/condition behavior |
| Conditions | CALL ON/OFF, SIGNAL, HALT, ERROR, FAILURE, NOTREADY, NOVALUE, LOSTDIGITS, SYNTAX | Front end: selected parser forms | Trap lifecycle, delivery, messages and error identity |
| Numeric | DIGITS, FORM, FUZZ, decimal arithmetic, rounding, logical conversion | Runtime: `RexxValue` foundation | Full context, limits, signal and optimized parity |
| Source/trace | TRACE, SOURCELINE, clause hooks, source preservation | Front end/runtime pieces | Visible source, tracing and clause lifecycle |
| Host | commands, external routines, queues, streams, time/random, traps, API variable pools, initialization/termination | Runtime foundation only | Configuration adapters and supported-platform contract |
| BIFs | each recognized Classic BIF | See individual rows below | Direct compiler calls, Classic argument/error/context equivalence |

### Syntax and instruction inventory

These 36 contract names come from the existing [raw language catalogue](component-catalogue/raw-language-syntax.md). Each row is distinct from full compatibility; several raw catalogue names group multiple Classic variants and still need finer reference reconciliation under `LC-AC-01`.

| Contract | Feature | Current evidence level | Remaining proof |
| --- | --- | --- | --- |
| `SYN-CLASSIC-OPTIONS` | Classic `OPTIONS` clauses and Level C selection | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-CLAUSES` | Semicolon/EOL clause model | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-CONTEXTUAL-KEYWORDS` | Instruction words usable as symbols outside instruction context | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-LABELS` | Labels and local routine names | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-SYMBOLS` | Simple, compound, and constant symbols | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-STEMS` | Classic stems and compound-variable tails | Bounded slice: stem exposure and simple compound access | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-STRINGS` | Quoted, doubled-quote, hex, and binary strings | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-ASSIGNMENT` | Simple and compound assignment | Bounded slice: scalar and simple compound assignment | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-COMMAND` | Implicit command clause | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-ADDRESS` | Classic ADDRESS forms | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-ARG` | Classic ARG instruction | Bounded slice: fixed procedure ARG | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-CALL` | CALL routine and CALL ON/OFF forms | Bounded slice: direct local CALL | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-DO` | Simple, counted, conditional, and forever DO | Bounded slice: simple DO only | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-DROP` | DROP instruction | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-EXIT` | EXIT instruction | Bounded slice: empty EXIT | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-IF` | Classic IF/THEN/ELSE | Bounded slice: bounded IF/THEN/ELSE | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-INTERPRET` | INTERPRET instruction | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-ITERATE` | ITERATE instruction | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-LEAVE` | LEAVE instruction | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-NOP` | NOP instruction | Bounded slice: standalone and nested NOP | Full source/TRACE and configuration proof open |
| `SYN-CLASSIC-NUMERIC` | NUMERIC DIGITS/FORM/FUZZ | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-PARSE` | PARSE variants and templates | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-PROCEDURE` | PROCEDURE and EXPOSE | Bounded slice: scalar/stem PROCEDURE EXPOSE | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-PULL` | PULL instruction/templates | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-PUSH` | PUSH instruction | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-QUEUE` | QUEUE instruction | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-RETURN` | RETURN instruction | Bounded slice: value/void RETURN in local procedures | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-SAY` | SAY instruction | Bounded slice: supported SAY expressions | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-SELECT` | SELECT/WHEN/OTHERWISE | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-SIGNAL` | SIGNAL target and ON/OFF conditions | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-TRACE` | TRACE options/value | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-EXPRESSIONS` | Classic arithmetic, comparison, Boolean, and concatenation expressions | Bounded slice: documented operator family | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-BIF-CALL` | Recognised ANSI BIF calls | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-LOCAL-CALL` | Direct local function/procedure calls | Bounded slice: direct local function and procedure calls | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-DSLSH` | Source tree, diagnostics, and syntax-highlighting projection | Parser-mode milestone | Execution and full diagnostic conformance remain separate |
| `SYN-CLASSIC-CANONICAL-LOWERING` | Transformation to canonical compiler AST | Bounded slice: nine proven slices | Remaining Classic forms, errors and configuration proof open |

### Individual BIF inventory

The source list is `component-catalogue/raw-levelc-bifs.md` (recognition
only). A standalone `lib/rxfnsc/RexxClassicBif<Name>.crexx` module establishes
a runtime entry, not compiler reachability or conformance. The two bounded
compiler BIF shapes are `LENGTH` and `SUBSTR`; the legacy dispatcher is not a
general Level C BIF implementation. Every BIF row below is still open for
full compatibility proof.

| BIF | Current state | Evidence / next proof |
| --- | --- | --- |
| `ABBREV` | Runtime | [`RexxClassicBifAbbrev.crexx`](../../../lib/rxfnsc/RexxClassicBifAbbrev.crexx); compiler and reference proof open |
| `ABS` | Runtime | [`RexxClassicBifAbs.crexx`](../../../lib/rxfnsc/RexxClassicBifAbs.crexx); compiler and reference proof open |
| `ADDRESS` | Runtime | [`RexxClassicBifAddress.crexx`](../../../lib/rxfnsc/RexxClassicBifAddress.crexx); compiler and reference proof open |
| `ARG` | Open | No standalone Classic BIF module; implementation and proof open |
| `B2X` | Runtime | [`RexxClassicBifB2x.crexx`](../../../lib/rxfnsc/RexxClassicBifB2x.crexx); compiler and reference proof open |
| `BITAND` | Runtime | [`RexxClassicBifBitand.crexx`](../../../lib/rxfnsc/RexxClassicBifBitand.crexx); compiler and reference proof open |
| `BITOR` | Runtime | [`RexxClassicBifBitor.crexx`](../../../lib/rxfnsc/RexxClassicBifBitor.crexx); compiler and reference proof open |
| `BITXOR` | Runtime | [`RexxClassicBifBitxor.crexx`](../../../lib/rxfnsc/RexxClassicBifBitxor.crexx); compiler and reference proof open |
| `C2D` | Runtime | [`RexxClassicBifC2d.crexx`](../../../lib/rxfnsc/RexxClassicBifC2d.crexx); compiler and reference proof open |
| `C2X` | Runtime | [`RexxClassicBifC2x.crexx`](../../../lib/rxfnsc/RexxClassicBifC2x.crexx); compiler and reference proof open |
| `CENTER` | Runtime | [`RexxClassicBifCenter.crexx`](../../../lib/rxfnsc/RexxClassicBifCenter.crexx); compiler and reference proof open |
| `CENTRE` | Open | No standalone Classic BIF module; implementation and proof open |
| `CHANGESTR` | Runtime | [`RexxClassicBifChangestr.crexx`](../../../lib/rxfnsc/RexxClassicBifChangestr.crexx); compiler and reference proof open |
| `CHARIN` | Open | No standalone Classic BIF module; implementation and proof open |
| `CHAROUT` | Open | No standalone Classic BIF module; implementation and proof open |
| `CHARS` | Open | No standalone Classic BIF module; implementation and proof open |
| `COMPARE` | Runtime | [`RexxClassicBifCompare.crexx`](../../../lib/rxfnsc/RexxClassicBifCompare.crexx); compiler and reference proof open |
| `CONDITION` | Open | No standalone Classic BIF module; implementation and proof open |
| `COPIES` | Runtime | [`RexxClassicBifCopies.crexx`](../../../lib/rxfnsc/RexxClassicBifCopies.crexx); compiler and reference proof open |
| `COUNTSTR` | Runtime | [`RexxClassicBifCountstr.crexx`](../../../lib/rxfnsc/RexxClassicBifCountstr.crexx); compiler and reference proof open |
| `DATATYPE` | Runtime | [`RexxClassicBifDatatype.crexx`](../../../lib/rxfnsc/RexxClassicBifDatatype.crexx); compiler and reference proof open |
| `DATE` | Runtime | [`RexxClassicBifDate.crexx`](../../../lib/rxfnsc/RexxClassicBifDate.crexx); compiler and reference proof open |
| `DELSTR` | Runtime | [`RexxClassicBifDelstr.crexx`](../../../lib/rxfnsc/RexxClassicBifDelstr.crexx); compiler and reference proof open |
| `DELWORD` | Runtime | [`RexxClassicBifDelword.crexx`](../../../lib/rxfnsc/RexxClassicBifDelword.crexx); compiler and reference proof open |
| `DIGITS` | Open | No standalone Classic BIF module; implementation and proof open |
| `D2C` | Runtime | [`RexxClassicBifD2c.crexx`](../../../lib/rxfnsc/RexxClassicBifD2c.crexx); compiler and reference proof open |
| `D2X` | Runtime | [`RexxClassicBifD2x.crexx`](../../../lib/rxfnsc/RexxClassicBifD2x.crexx); compiler and reference proof open |
| `ERRORTEXT` | Open | No standalone Classic BIF module; implementation and proof open |
| `FORM` | Open | No standalone Classic BIF module; implementation and proof open |
| `FORMAT` | Runtime | [`RexxClassicBifFormat.crexx`](../../../lib/rxfnsc/RexxClassicBifFormat.crexx); compiler and reference proof open |
| `FUZZ` | Open | No standalone Classic BIF module; implementation and proof open |
| `INSERT` | Runtime | [`RexxClassicBifInsert.crexx`](../../../lib/rxfnsc/RexxClassicBifInsert.crexx); compiler and reference proof open |
| `LASTPOS` | Runtime | [`RexxClassicBifLastpos.crexx`](../../../lib/rxfnsc/RexxClassicBifLastpos.crexx); compiler and reference proof open |
| `LEFT` | Runtime | [`RexxClassicBifLeft.crexx`](../../../lib/rxfnsc/RexxClassicBifLeft.crexx); compiler and reference proof open |
| `LENGTH` | Slice + runtime | [`levelc_slice3_bif_length`](../../../compiler/tests/rexx_src/levelc_slice3_bif_length.rexx); other argument/context cases open |
| `LINEIN` | Open | No standalone Classic BIF module; implementation and proof open |
| `LINEOUT` | Open | No standalone Classic BIF module; implementation and proof open |
| `LINES` | Open | No standalone Classic BIF module; implementation and proof open |
| `MAX` | Runtime | [`RexxClassicBifMax.crexx`](../../../lib/rxfnsc/RexxClassicBifMax.crexx); compiler and reference proof open |
| `MIN` | Runtime | [`RexxClassicBifMin.crexx`](../../../lib/rxfnsc/RexxClassicBifMin.crexx); compiler and reference proof open |
| `OVERLAY` | Runtime | [`RexxClassicBifOverlay.crexx`](../../../lib/rxfnsc/RexxClassicBifOverlay.crexx); compiler and reference proof open |
| `POS` | Runtime | [`RexxClassicBifPos.crexx`](../../../lib/rxfnsc/RexxClassicBifPos.crexx); compiler and reference proof open |
| `QUALIFY` | Open | No standalone Classic BIF module; implementation and proof open |
| `QUEUED` | Open | No standalone Classic BIF module; implementation and proof open |
| `RANDOM` | Runtime | [`RexxClassicBifRandom.crexx`](../../../lib/rxfnsc/RexxClassicBifRandom.crexx); compiler and reference proof open |
| `REVERSE` | Runtime | [`RexxClassicBifReverse.crexx`](../../../lib/rxfnsc/RexxClassicBifReverse.crexx); compiler and reference proof open |
| `RIGHT` | Runtime | [`RexxClassicBifRight.crexx`](../../../lib/rxfnsc/RexxClassicBifRight.crexx); compiler and reference proof open |
| `SIGN` | Runtime | [`RexxClassicBifSign.crexx`](../../../lib/rxfnsc/RexxClassicBifSign.crexx); compiler and reference proof open |
| `SOURCELINE` | Open | No standalone Classic BIF module; implementation and proof open |
| `SPACE` | Runtime | [`RexxClassicBifSpace.crexx`](../../../lib/rxfnsc/RexxClassicBifSpace.crexx); compiler and reference proof open |
| `STREAM` | Open | No standalone Classic BIF module; implementation and proof open |
| `STRIP` | Runtime | [`RexxClassicBifStrip.crexx`](../../../lib/rxfnsc/RexxClassicBifStrip.crexx); compiler and reference proof open |
| `SUBSTR` | Slice + runtime | [`levelc_slice4_bif_substr`](../../../compiler/tests/rexx_src/levelc_slice4_bif_substr.rexx); other argument/context cases open |
| `SUBWORD` | Runtime | [`RexxClassicBifSubword.crexx`](../../../lib/rxfnsc/RexxClassicBifSubword.crexx); compiler and reference proof open |
| `SYMBOL` | Runtime | [`RexxClassicBifSymbol.crexx`](../../../lib/rxfnsc/RexxClassicBifSymbol.crexx); compiler and reference proof open |
| `TIME` | Runtime | [`RexxClassicBifTime.crexx`](../../../lib/rxfnsc/RexxClassicBifTime.crexx); compiler and reference proof open |
| `TRACE` | Runtime | [`RexxClassicBifTrace.crexx`](../../../lib/rxfnsc/RexxClassicBifTrace.crexx); compiler and reference proof open |
| `TRANSLATE` | Runtime | [`RexxClassicBifTranslate.crexx`](../../../lib/rxfnsc/RexxClassicBifTranslate.crexx); compiler and reference proof open |
| `TRUNC` | Runtime | [`RexxClassicBifTrunc.crexx`](../../../lib/rxfnsc/RexxClassicBifTrunc.crexx); compiler and reference proof open |
| `VALUE` | Runtime | [`RexxClassicBifValue.crexx`](../../../lib/rxfnsc/RexxClassicBifValue.crexx); compiler and reference proof open |
| `VERIFY` | Runtime | [`RexxClassicBifVerify.crexx`](../../../lib/rxfnsc/RexxClassicBifVerify.crexx); compiler and reference proof open |
| `WORD` | Runtime | [`RexxClassicBifWord.crexx`](../../../lib/rxfnsc/RexxClassicBifWord.crexx); compiler and reference proof open |
| `WORDINDEX` | Runtime | [`RexxClassicBifWordindex.crexx`](../../../lib/rxfnsc/RexxClassicBifWordindex.crexx); compiler and reference proof open |
| `WORDLENGTH` | Runtime | [`RexxClassicBifWordlength.crexx`](../../../lib/rxfnsc/RexxClassicBifWordlength.crexx); compiler and reference proof open |
| `WORDPOS` | Runtime | [`RexxClassicBifWordpos.crexx`](../../../lib/rxfnsc/RexxClassicBifWordpos.crexx); compiler and reference proof open |
| `WORDS` | Runtime | [`RexxClassicBifWords.crexx`](../../../lib/rxfnsc/RexxClassicBifWords.crexx); compiler and reference proof open |
| `XRANGE` | Runtime | [`RexxClassicBifXrange.crexx`](../../../lib/rxfnsc/RexxClassicBifXrange.crexx); compiler and reference proof open |
| `X2B` | Runtime | [`RexxClassicBifX2b.crexx`](../../../lib/rxfnsc/RexxClassicBifX2b.crexx); compiler and reference proof open |
| `X2C` | Runtime | [`RexxClassicBifX2c.crexx`](../../../lib/rxfnsc/RexxClassicBifX2c.crexx); compiler and reference proof open |
| `X2D` | Runtime | [`RexxClassicBifX2d.crexx`](../../../lib/rxfnsc/RexxClassicBifX2d.crexx); compiler and reference proof open |

## Increment receipts

### LC-STEP-01 — initial inventory, 2026-10-03

- First-pass category matrix and all 70 recognized BIF names recorded in
  commit `941a8469c`. Fifty-three BIF names have standalone Classic modules;
  source recognition and module presence are deliberately not conformance
  claims. `LC-AC-01` remains open pending individual non-BIF rows and complete
  reference reconciliation.
- A second inventory pass records 36 syntax contracts and 74 detailed
  non-BIF reference obligations with explicit BYTE/UTF8/host/source scope.
  Current implementation states remain conservative; the final crosswalk,
  feature-level qualification and exception decisions remain open.

### LC-STEP-02 — bounded IF, 2026-10-03

- Regina reference run of `levelc_slice7_if_nested.rexx` produced
  `true`, `false`, `nested`, `after`, `once`, `1`, `1` in order. `IF 2`
  produced `34.1`; the compiled path now rejects it with `RXC-LC-34.1`.
- Release: 8/8 initial slice-7 focused tests, 94/94 Level C tests, 2/2
  `testRexxValue` opt/no-opt; the subsequently added generic logical-value
  negative test passed 1/1 on unchanged code. Retained logs:
  `/tmp/crexx-levelc-if-ctest.JsMsZT`,
  `/tmp/crexx-levelc-if-suite.24iyqv`,
  `/tmp/crexx-levelc-if-final-tests.k7ROaa`.
- Debug: 95/95 Level C tests and 2/2 `testRexxValue` opt/no-opt on the same
  source change. Retained log: `/tmp/crexx-levelc-if-final-tests.k7ROaa`.
- Release `rxlink` of the compiled nested fixture ran through `rxvm` and
  produced the same seven output lines. Retained log:
  `/tmp/crexx-levelc-if-link.Y5Cdut`.
- `RexxValue` now rejects values other than strict `0`/`1` for the shared
  logical helper. The `IF` path reports `34.1`; contextual `34.5/34.6`
  identities for logical-operator operands remain open in the full matrix.
  Regina accepts some padded/leading-zero spellings that the extracted Classic
  reference treats as invalid; those interpreter-specific cases are not used
  as conformance evidence for this slice.

### LC-STEP-03 — simple DO, 2026-10-03

- Regina and the compiled nested fixture both produced `before`, `nested`,
  `2`, `2`, `arm`, `3` in order. The fixture covers body order, nesting,
  `DO` in an `IF` arm, `IF` in a `DO` body, an empty group, and a local
  procedure group. The canonical target is a one-shot `DO 1` block.
- Release 8/8 focused slice-8 tests and 103/103 Level C suite; Debug 103/103
  Level C suite. The focused cases include opt/no-opt, tree shape, a counted
  `DO` rejection and an unsupported-body rejection. Retained logs:
  `/tmp/crexx-levelc-do-focused.OA1FOA`,
  `/tmp/crexx-levelc-do-release-suite.XnH4dK`,
  `/tmp/crexx-levelc-do-debug-suite.Pju44v`.
- Release `rxlink` of the compiled nested fixture ran through `rxvm` with
  the same six output lines. Retained log:
  `/tmp/crexx-levelc-do-link.gYjrFs`.
- Controlled/repetitive `DO`, `WHILE`/`UNTIL`, `LEAVE`/`ITERATE` and their
  state/condition behavior remain open. `LC-AC-01/04` and `R1-AC-01/02`
  remain open; this increment does not establish full Classic compatibility.

### LC-STEP-06 — childless NOP, 2026-10-03

- Regina and the optimized/no-opt compiled fixture each produced `before`,
  `middle`, `after`. The fixture places `NOP` in main, a local procedure,
  both selected `IF` arms and nested `DO` bodies; adjacent statements retain
  their order.
- Release toolchain build passed; focused slice-9 tests passed 2/2 and the
  normal Level C suite passed 105/105. Retained logs:
  `/tmp/crexx-levelc-nop-build.bO3BoM`,
  `/tmp/crexx-levelc-nop-tests.iKKtAb`,
  `/tmp/crexx-levelc-nop-suite.HEoZ0w`.
- `rxlink` of the optimized RXBIN ran through `rxvm` with the same three
  lines. Retained log: `/tmp/crexx-levelc-nop-link.73Ab9Z`.
- `NOP` source/TRACE lifecycle and full profile/platform qualification remain
  open under `LC-AC-04`; this is a bounded execution slice.
