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

The [Level C architecture design](../../../compiler/docs/levelc_working_architecture.md)
records the common `rxfnsc` value, variable-pool and BIF foundation with
RexxScript. Adrian confirmed this shared foundation on 2026-10-03 while
retaining the two products' distinct language and sandbox contracts.

The first delivery increments are a coverage inventory, then executable
`IF/THEN/ELSE`, then simple `DO ... END`. They do not complete the Beta 4
contract. Later increments are selected from the open coverage rows rather
than redefining compatibility around the first slices.
Adrian prioritized closing the high-risk AST tree-manipulation path early on
2026-10-03. Structural lowering and its invariant evidence now precede broad
BIF and host-service expansion; the full compatibility contract is unchanged.

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
- [ ] **LC-AC-06 — shared runtime contract:** compiled Level C and RexxScript
  use the same `rxfnsc` `RexxValue`, `RexxVariablePool`, and overlapping Classic
  BIF implementations with their own caller contexts. Verify shared BIF
  argument/error behavior from both products, RexxScript sandbox isolation,
  Level C visible-pool behavior, and configuration/profile boundaries with
  focused cross-consumer integration evidence. RexxScript's string-oriented
  evaluator and allow-list remain product-specific.
- [ ] **LC-AC-08 — AST lowering closure:** every parsed Level C instruction,
  expression and program-structure shape is explicitly mapped to canonical
  lowering, a source-level diagnostic, or a visibly open feature row. Accepted
  rewrites preserve parent/sibling ownership, generated scope and symbol
  validity, evaluation order, source anchors, and opt/no-opt behavior, with no
  Level C-only nodes surviving the lowering boundary. Verify with a parser-node
  crosswalk, structural validation, nested target-tree probes, focused runtime
  equivalence and the normal compiler suite. Keep this criterion open until
  the structural shape families are closed; a first verifier is only a gate.
- [x] **LC-AC-09 — bounded SELECT execution:** parsed `SELECT`/ordered `WHEN`
  clauses and an optional `OTHERWISE` lower to a lazy canonical branch chain.
  Only the selected arm executes; later conditions are skipped, nesting and
  local procedures retain source order, a nonlogical `WHEN` reports `34.2`,
  and an unmatched SELECT without OTHERWISE reports `7.3` with its source
  line. Verify against the Classic reference for successful paths, optimized
  and no-opt execution, negative runtime checks, lowered-tree inspection,
  the normal correctness suite, and linked-image execution.
- [x] **LC-AC-07 — scalar pool read and DROP slice:** an uninitialized or
  dropped scalar reads as its uppercase Classic symbol, direct scalar `DROP`
  affects the current visible pool (including a procedure's exposed alias),
  and reassignment restores its value. Compound, stem and indirect `DROP`
  forms remain fail-closed pending separate proof. Verify against the Classic
  interpreter, optimized/no-opt and linked execution, runtime pool tests,
  and the relevant Level C and RexxScript correctness suites.
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
7. **LC-STEP-07 — complete (LC-AC-06):** reconcile the existing Level C and
   RexxScript implementation with one current architecture design. Record the
   shared value/pool/BIF ownership, both call paths, sandbox boundary, and
   acceptance evidence in this worklist and the architecture document.
8. **LC-STEP-08 (LC-AC-06; depends on STEP-07):** qualify overlapping BIFs and
   pool operations in reviewable cross-consumer increments. Use the shared
   `rxfnsc` implementation, test both adapters and isolation, and commit each
   qualified increment. Keep any observed behavior gap visibly open. Prioritize
   AST closure steps before broad BIF or host expansion.
9. **LC-STEP-09 — complete (LC-AC-07; serves LC-AC-06; depends on STEP-07):** retain the
   Regina and compiled reproducer for an unset scalar read; use the shared
   pool's `symbolValue()` for scalar reads and lower guarded direct scalar
   `DROP` through that pool. Compare main/procedure/IF/DO cases and unsupported
   forms with the Classic reference, then run focused and normal regressions
   before committing this separate increment.
10. **LC-STEP-10 — complete (LC-AC-08; depends on STEP-09):** inventory parser-emitted
    structural node families and their lowering disposition. Add a production
    boundary check that rejects invalid parent/sibling ownership or surviving
    Level C-only nodes before canonical validation; exercise it on nested
    accepted shapes and retain debug tree evidence. Commit the verifier and
    crosswalk as one reviewable increment.
11. **LC-STEP-11 — complete (LC-AC-08/09/04; depends on STEP-10):** open the next Classic
    control-flow tree shape, starting with `SELECT/WHEN/OTHERWISE`, only after
    confirming its parsed AST and reference condition behavior. Prove branch
    order, nested arms, source anchors, opt/no-opt and missing-match behavior;
    keep unproved forms fail-closed. Commit separately.
12. **LC-STEP-12 (LC-AC-08/04; depends on STEP-11):** continue structural
    families such as repetitive/controlled `DO`, `LEAVE`/`ITERATE`, routine
    boundaries, PARSE templates and condition branches in risk-sized commits.
    Close `LC-AC-08` only after the full parser-node crosswalk and structural
    invariants are evidenced; a runtime or host dependency remains an open
    criterion rather than an implicit exclusion.

## AST structural crosswalk and closure order

This crosswalk is grounded in `compiler/rxcpcgmr.y`'s `program`,
`instruction`, `simple_instruction`, control-flow, and expression productions,
the `NodeType` catalogue, and the guards in `rxcp_levelc_lower.c`. **Slice**
means an accepted subset with executable evidence; **open** means the parser
may emit the shape but the lowerer rejects it, or runtime proof is incomplete.
Parser recovery `ERROR`/`WARNING` nodes remain source diagnostics and never
enter accepted lowering. A generic node name shared with Level B does not by
itself make its Classic shape executable.

| Parser-emitted family | Current AST/lowering disposition | Structural risk and next proof |
| --- | --- | --- |
| Program shell, `REXX_OPTIONS`, top-level `INSTRUCTIONS`, `LABEL` | Slice: plan partitions main and bounded local procedures; generated `REXX_OPTIONS` imports and canonical siblings replace the Classic instruction wrapper | Multiple file/label layouts, option placement, source anchors and generated symbol/scope ownership |
| `ASSIGN`, `SAY`, `NOP`, `EXIT`, `RETURN`, `LEVELC_DROP` | Slice: guarded scalar/compound assignment, SAY, childless NOP, bare main EXIT, procedure RETURN, and direct scalar DROP | Wider statement operands, stem/compound/indirect DROP and exit/return lifecycle remain open |
| `VAR_SYMBOL`/`VAR_TARGET`, strings, integers, expression operators, function calls | Slice: proven scalar/compound pool reads, literal and operator methods, lazy logical branches, bounded BIF/local calls | More expression shapes, exact evaluation order, numeric context and missing-argument behavior remain open |
| `IF` with condition/THEN/ELSE; simple `DO` with `INSTRUCTIONS` | Slice: recursive guards and canonical branch/group builders, including nested forms | More accepted arm statements and source/scope proof as forms expand |
| `SELECT` with `INSTRUCTIONS` of `WHEN` and optional `OTHERWISE` | Slice: guarded list lowers to nested canonical `IF`/one-shot `DO`, including nested arms, local procedures, `34.2` and `7.3` | Broader statement arms, condition lifecycle and profile proof remain open |
| Header-bearing `DO`, `REPEAT`, `FOR`, `WHILE`, `UNTIL`, `BY`, `TO`, `LEAVE`, `ITERATE` | Open: only child-list-only `DO` is accepted | Loop header ownership, mutation/evaluation order, control transfer and scope |
| `LABEL`, `LEVELC_PROCEDURE`, `LEVELC_ARG`, `CALL`, `RETURN` | Slice: bounded direct local routines, fixed ARG, scalar/stem EXPOSE and value returns | Wider routine and argument shapes, external resolution, exposure and condition lifecycle |
| `PARSE`, `PULL`, template/pattern/position nodes | Open: parser and diagnostic coverage only | Template target ownership, source acquisition, ordered assignment and source anchors |
| `LEVELC_ADDRESS`, command expression, `LEVELC_PUSH`, `LEVELC_QUEUE` | Open: parser/front end only | Host/queue protocol and side-effect ordering |
| `LEVELC_NUMERIC`, `LEVELC_SIGNAL`, `LEVELC_TRACE`, `LEVELC_INTERPRET`, condition CALL forms | Open: parser/front end only | Context changes, dynamic code, signal transfer, trace and error identity |

The production post-lowering boundary verifier walks every accepted node,
rejects a missing root, sibling cycle, mismatched parent pointer, or surviving
`LEVELC_*` instruction node before normal canonical validation. Its direct
negative unit test proves each rejection path; accepted nested fixtures and
the `STAGE_LEVELC_LOWERED` debug probe cover generated output. This gate does
not yet prove generated scope/symbol correctness, source anchors, evaluation
order, or completeness of accepted structural families. Those remain open in
`LC-AC-08` and are tackled through `LC-STEP-12` and later structural increments.

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
| Variables | scalar read/write, drop, compound names, bare stems, exposure, API pool | Slice: scalar/compound read/write, scalar/stem EXPOSE, unset scalar read and direct scalar DROP | Remaining stem/compound/indirect DROP, external/API operations and aliasing |
| Control | IF/THEN/ELSE | Slice: `levelc_slice7_if_else`, nested and procedure fixtures, opt/no-opt, invalid logical and unsupported-arm tests | Other instructions in arms and broader condition/message lifecycle remain open |
| Control | simple DO/END | Slice: `levelc_slice8_do_block` and nested/empty/procedure fixtures, opt/no-opt, tree-shape and linked execution | Broader clause lifecycle and conditions remain open |
| Control | controlled/repetitive DO, WHILE/UNTIL, LEAVE/ITERATE | Front end: parser/validation fixtures | Execution, exact loop semantics and errors |
| Control | SELECT/WHEN/OTHERWISE | Slice: `levelc_slice11_select`, opt/no-opt and linked execution, exact `34.2`/`7.3` negatives | Wider arms, lifecycle and configuration proof |
| Control | NOP | Slice: `levelc_slice9_nop` in main, local procedure, and IF/DO bodies | Full source/TRACE lifecycle and configuration proof open |
| Routines | labels, local/external CALL and functions, ARG, PROCEDURE EXPOSE, RETURN, EXIT | Slice: bounded local calls, fixed ARG, scalar/stem EXPOSE, RETURN and empty EXIT | Omitted arguments, dynamic/external calls, full scope and return/exit lifecycle |
| PARSE | ARG, PULL, SOURCE, LINEIN, VERSION, VALUE, VAR; templates and UPPER | Front end: parser fixtures | Runtime source acquisition, template assignment, errors |
| Environment | ADDRESS, command clauses, WITH redirection | Front end: parser/validation | Configured command/stream service and RC/condition behavior |
| Conditions | CALL ON/OFF, SIGNAL, HALT, ERROR, FAILURE, NOTREADY, NOVALUE, LOSTDIGITS, SYNTAX | Front end: selected parser forms | Trap lifecycle, delivery, messages and error identity |
| Numeric | DIGITS, FORM, FUZZ, decimal arithmetic, rounding, logical conversion | Runtime: `RexxValue` foundation | Full context, limits, signal and optimized parity |
| Source/trace | TRACE, SOURCELINE, clause hooks, source preservation | Front end/runtime pieces | Visible source, tracing and clause lifecycle |
| Host | commands, external routines, queues, streams, time/random, traps, API variable pools, initialization/termination | Runtime foundation only | Configuration adapters and supported-platform contract |
| BIFs | each recognized Classic BIF | See individual rows below | Direct compiler calls, Classic argument/error/context equivalence |
| Shared runtime | Level C and RexxScript value, pool and overlapping BIF contracts | Both import `rxfnsc`; RexxScript uses a sandbox pool and `RexxValue` BIF frames | Cross-consumer behavior, errors, isolation and profile evidence under `LC-AC-06` |

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
| `SYN-CLASSIC-DROP` | DROP instruction | Bounded slice: direct scalar list | Stem, compound and indirect forms, full condition/profile proof open |
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
| `SYN-CLASSIC-CANONICAL-LOWERING` | Transformation to canonical compiler AST | Bounded slice: ten proven slices | Remaining Classic forms, errors and configuration proof open |

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

### LC-STEP-07 — shared architecture reconciliation, 2026-10-03

- The current Level C architecture design now identifies the guarded compiler
  path, common `rxfnsc` value/pool/BIF modules, and separate RexxScript
  evaluator and sandbox pool. This records Adrian's shared-foundation
  direction without treating the two languages as identical.
- The existing `RexxScriptEvaluator` uses `RexxVariablePool.setString()` for
  sandbox variables and `RexxValue` frames for shared BIF calls; Level C
  lowering uses the same library from compiled code. The required behavioral
  and isolation proof remains open under `LC-AC-06` and `LC-STEP-08`.

### LC-STEP-09 — scalar pool read and direct DROP, 2026-10-03

- The retained before-change reproducer showed Regina output `MISSING`,
  while compiled Level C printed a blank line for an unset scalar. A separate
  `DROP a` fixture printed `A` under Regina but was rejected by the compiler.
  Before-change log: `/tmp/crexx-levelc-pool-before.zIeQRx`.
- Scalar reads now use `RexxVariablePool.symbolValue()` rather than `value()`;
  guarded direct scalar lists lower to the pool's `drop()` method. The
  nine-line fixture matches Regina in main, nested `IF`/`DO`, and an exposed
  procedure pool, with optimized/no-opt parity. Indirect, stem and compound
  `DROP` remain fail-closed.
- Release core/runtime build passed, focused tests 6/6, and the selected Level
  C, pool and RexxScript correctness suite 115/115. The linked fixture matched
  Regina through `rxvm`. Retained logs:
  `/tmp/crexx-levelc-drop-build.2mXMyN`,
  `/tmp/crexx-levelc-drop-consumers-build.AVcSFk`,
  `/tmp/crexx-levelc-drop-focused.vvOvKu`,
  `/tmp/crexx-levelc-drop-suite.EB7iFh`,
  `/tmp/crexx-levelc-drop-linked-run.YcStKq`.
- Compound/stem reads and `DROP` variants, NOVALUE trap delivery, BYTE/UTF8
  profiles and complete cross-consumer semantics remain open under
  `LC-AC-01/04/06`.

### LC-STEP-10 — AST boundary and structural crosswalk, 2026-10-03

- The crosswalk above maps the parser's structural families to accepted slices
  and open lowering obligations. It keeps `SELECT`, controlled DO, PARSE,
  condition branches, and host/queue shapes visible for early AST closure.
- Accepted lowering now checks the active tree for missing root, wrong parent,
  sibling cycles and residual `LEVELC_*` nodes before canonical passes. The
  direct unit test rejects each malformed case and accepts the restored tree.
- Release `rxc` and unit-test build passed. The earlier nested/tree-shape
  focused set passed 8/8, the selected Level C suite passed 111/111, and a
  redirected `rxc -d2` probe found `STAGE_LEVELC_LOWERED` without AST errors.
  After the final unit-test wiring, `levelc_*` tests passed 61/61, including
  the direct negative unit. Retained logs:
  `/tmp/crexx-levelc-tree-build.cVVuMD`,
  `/tmp/crexx-levelc-tree-focused.ywIWWn`,
  `/tmp/crexx-levelc-tree-debug.8M7ww7`,
  `/tmp/crexx-levelc-tree-suite.awW5rx`,
  `/tmp/crexx-levelc-tree-build-confirm.q84koC`,
  `/tmp/crexx-levelc-tree-final-suite.aHpaIM`.
- `LC-AC-08` remains open: the verifier is a boundary invariant, while
  generated scope/symbols, evaluation order, source anchors and the remaining
  parser families still need implementation and evidence.

### LC-STEP-11 — bounded SELECT/WHEN/OTHERWISE, 2026-10-03

- Before-change, a simple SELECT printed `zero` in Regina and failed compiled
  Level C as an unsupported main statement. The raw parsed tree is `SELECT >
  INSTRUCTIONS > WHEN... [OTHERWISE]`; each WHEN has condition and body, while
  OTHERWISE has an optional inline body plus an `INSTRUCTIONS` list. Retained
  reproducer and AST logs:
  `/tmp/crexx-levelc-select-before.npooyz`,
  `/tmp/crexx-levelc-select-ast.HWxpA1`.
- The lowerer validates that shape, builds nested canonical `IF` branches in
  reverse order, and places each later condition inside the preceding ELSE
  block. `RexxValue.logicalWhenValue()` reports `RXC-LC-34.2`; the unmatched
  path reports `RXC-LC-7.3` with the SELECT source line. Regina reports base
  error `7` for this unmatched case; the compliance reference specifies `7.3`
  and is the current implementation target.
- The seven-line nested fixture matched Regina with optimized/no-opt parity.
  Release core and `rxfnsc` builds passed, five focused SELECT tests passed,
  and the selected Level C, `RexxValue` and RexxScript correctness suite passed
  127/127. A redirected debug probe showed source-anchored generated WHEN
  conditions and no AST validation error. `rxlink` plus `rxvm` reproduced the
  same seven lines. Retained logs:
  `/tmp/crexx-levelc-select-final-build.7BDc9R`,
  `/tmp/crexx-levelc-select-focused.X6HxHI`,
  `/tmp/crexx-levelc-select-suite.xQepez`,
  `/tmp/crexx-levelc-select-debug.jZHZB4`,
  `/tmp/crexx-levelc-select-link.PrvySs`.
- `LC-AC-08` and `LC-AC-04` remain open for the other structural and complete
  compatibility obligations.
