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
  broader controlled/repetitive forms and named transfers remain open until
  their semantics are separately proved. Verify with Classic reference comparisons,
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
- [x] **LC-AC-10 — literal counted DO slice:** a parsed `DO` with one
  non-negative integer literal repetition count executes its supported body
  exactly that many times, including zero and nested groups, with opt/no-opt
  parity and valid lowered ownership/source anchors. Dynamic counts, controlled
  variables, conditions, `LEAVE` and `ITERATE` remain open. Verify against
  Regina, a targeted AST probe, focused execution and the normal Level C
  suite, then commit this increment separately.
- [x] **LC-AC-11 — loop transfer through generated AST blocks:** childless
  `LEAVE` and `ITERATE` in a supported literal counted `DO` target the nearest
  source repetitive loop even when nested in Classic simple DO, IF or SELECT
  structures. Generated branch/group wrappers must not capture those
  transfers; named transfer forms remain open. Verify a Regina-versus-canonical
  counterexample, nested opt/no-opt behavior, target-tree association, normal
  compiler regressions and linked execution.
- [x] **LC-AC-12 — bounded DO FOREVER:** a parsed childless `REPEAT` marked
  FOREVER lowers to a canonical indefinite loop, and childless LEAVE/ITERATE
  still target that source loop across generated blocks. Verify Regina output
  for guarded nested and procedure forms, opt/no-opt parity, source anchors,
  normal correctness and linked execution. Dynamic counts, controlled
  variables, combined conditions and named transfers remain open.
- [x] **LC-AC-13 — bounded DO WHILE:** a parsed `DO WHILE expression`
  reevaluates a supported, setup-free condition before each iteration,
  executes zero times for an initially false condition, and preserves
  childless LEAVE/ITERATE binding through generated blocks. Nonlogical
  conditions report `34.3`; expressions requiring per-iteration setup have
  separate criteria. Verify Regina output, opt/no-opt and linked execution, a
  lowered-tree probe, focused normal/Debug regressions and the relevant
  correctness suite.
- [x] **LC-AC-14 — bounded DO UNTIL:** a parsed `DO UNTIL expression`
  executes its supported body before the first condition check, then checks
  after each iteration, including an ITERATE. Childless LEAVE/ITERATE keep
  their nearest source loop through generated blocks; a nonlogical condition
  reports `34.4`. Only setup-free expressions are admitted in this slice.
  Verify Regina behavior, opt/no-opt and linked execution, a lowered-tree
  probe, focused normal/Debug regressions and the relevant correctness suite.
- [x] **LC-AC-15 — literal count with condition:** parsed `DO integer
  WHILE expression` and `DO integer UNTIL expression` combine the bounded
  count with the correct entry or end check. Zero count executes no body;
  LEAVE/ITERATE retain the nearest source loop through generated blocks;
  `34.3`/`34.4` remain contextual. Accept only non-negative literal counts
  and setup-free conditions. Verify Regina, optimized/no-opt and linked
  execution, source-anchored tree shape, focused normal/Debug regressions and
  the relevant correctness suite.
- [x] **LC-AC-16 — eager Classic logical operands:** Level C `&` and `|`
  evaluate their left and right operands once each, in source order, even
  when the left result determines the Boolean answer. Side effects and
  contextual invalid-value identities `34.5`/`34.6` are preserved; existing
  Boolean results and opt/no-opt output remain stable. Verify a Regina
  side-effect reproducer, left/right operand errors, nested expressions,
  shared RexxValue behavior, linked execution and the relevant normal/Debug
  suites. Keep unrelated operator and condition families open.
- [x] **LC-AC-17 — direct WHILE with setup expressions:** a parsed direct
  `DO WHILE expression` reevaluates any supported condition whose lowering
  creates setup statements on every loop entry, preserving Classic eager operand
  order, BIF argument frames, compound-tail lookup, and exact logical errors.
  Existing childless LEAVE/ITERATE retain their source loop. Verify against
  Regina with bounded fixtures, opt/no-opt and linked execution, a canonical
  block-expression tree and source anchors, focused normal/Debug regressions
  and the relevant correctness suite. UNTIL and combined setup-bearing
  headers have separate criteria for their end-check and limit proof.
- [x] **LC-AC-18 — direct UNTIL with setup expressions:** a parsed direct
  `DO UNTIL expression` executes its body before the first condition, then
  reevaluates any supported setup-bearing condition after each ordinary or
  ITERATE path. Preserve eager operand order, BIF argument frames, source
  LEAVE/ITERATE ownership and exact `34.4` errors. Verify against Regina with
  bounded fixtures, opt/no-opt and linked execution, a source-anchored
  canonical block-expression tree, focused normal/Debug regressions and the
  relevant correctness suite. Count-plus-condition setup has a separate
  criterion for its limit proof.
- [x] **LC-AC-19 — literal count with setup-bearing condition:** parsed
  `DO integer WHILE expression` and `DO integer UNTIL expression` execute
  supported setup-bearing conditions at the respective entry/end checks,
  subject to the literal count limit. A zero count evaluates neither body nor
  condition setup; eager operand effects, BIF argument frames, source
  LEAVE/ITERATE ownership and exact `34.3`/`34.4` errors hold. Verify Regina
  timing, opt/no-opt and linked execution, source-anchored FOR plus
  BLOCK_EXPR tree shape, focused normal/Debug regressions and the relevant
  correctness suite. Dynamic counts and controlled headers remain open.
- [x] **LC-AC-20 — FOREVER with condition:** parsed `DO FOREVER WHILE` and
  `DO FOREVER UNTIL` execute with Classic entry/end check timing, including
  setup-bearing expressions. WHILE can skip its body; UNTIL executes once
  before checking. Preserve eager side effects, BIF frames, nearest-loop
  LEAVE/ITERATE ownership, local procedure scope and `34.3`/`34.4` errors.
  Verify bounded Regina output, opt/no-opt and linked execution, canonical
  REPEAT without FOR plus source-anchored condition tree, focused normal/Debug
  regressions and the relevant correctness suite.
- [x] **LC-AC-21 — bounded dynamic repetition count:** parsed direct
  `DO expression` evaluates a supported count expression once at loop entry,
  then executes its body the resulting number of times, up to the currently
  supported signed 32-bit whole-count limit. Zero skips the body, mutation of
  source variables does not change the count, BIF/local-call side effects run
  once, and childless LEAVE/ITERATE target the source loop. Invalid runtime
  counts use contextual `26.2` without silent truncation. Verify Regina for
  valid counts, opt/no-opt and linked execution, canonical FOR with a
  source-anchored setup block when needed, focused RexxValue/normal/Debug
  regressions and the relevant correctness suite. Dynamic count plus a
  condition has a separate criterion; controlled headers and wider numeric
  bounds remain open.
- [x] **LC-AC-22 — bounded dynamic count with condition:** parsed
  `DO expression WHILE expression` and `DO expression UNTIL expression`
  evaluate a supported count once, then check a supported condition at its
  Classic entry/end point within the count limit. Zero count skips condition
  setup and body; count-side effects run once, condition-side effects run at
  each required check; childless LEAVE/ITERATE keep source ownership and
  contextual `26.2`/`34.3`/`34.4` errors. Verify bounded Regina output,
  opt/no-opt and linked execution, canonical FOR plus WHILE/UNTIL with
  independent setup blocks and source anchors, focused normal/Debug tests and
  the relevant correctness suite. Controlled headers and wider count values
  remain open.
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
13. **LC-STEP-13 — complete (LC-AC-08/10; depends on STEP-11):** admit the parser's
    `DO > REPEAT > FOR > INTEGER` form only for a proven non-negative literal
    count representable in the canonical integer builder. Reuse a neutral
    counted-DO builder, validate body statements recursively, keep other
    repetition headers fail-closed, compare zero/nested counts with Regina,
    and qualify opt/no-opt, tree shape, linked execution and the normal suite.
    This is one bounded part of STEP-12, not closure of all loop forms.
14. **LC-STEP-14 — complete (LC-AC-08/11; depends on STEP-13):** reproduce the source-loop
    versus generated-wrapper association hazard. Map each accepted source
    counted loop to a hidden canonical control symbol, and lower childless
    LEAVE/ITERATE to named canonical transfers targeting that symbol through
    the existing association machinery. Validate source nesting before
    lowering, preserve source anchors and loop state, reject named/unsupported
    transfer shapes, and qualify focused/normal/linked execution. Keep global
    `ast_do()` semantics unchanged.
15. **LC-STEP-15 — complete (LC-AC-08/12; depends on STEP-14):** recognize the parser's
    `DO > REPEAT("forever") > INSTRUCTIONS` form. Reuse the hidden-control
    mapping and canonical association path from STEP-14, omitting the FOR
    count in the neutral builder. Keep every other new repetition/condition
    shape fail-closed. Qualify with bounded-termination fixtures and commit
    separately.
16. **LC-STEP-16 — complete (LC-AC-08/13; depends on STEP-15):** admit the parser's
    `DO > WHILE > INSTRUCTIONS` shape only when its condition lowers without
    setup statements. Add the WHILE condition to the neutral controlled-loop
    AST, reuse hidden source-loop transfer binding, validate exact logical
    `34.3` behavior in the shared value class, and keep UNTIL, combined headers
    and setup-requiring expressions fail-closed. Qualify condition timing,
    zero-entry, transfer targets, optimized/no-opt, source anchors, linked
    execution, and normal correctness before committing the increment.
17. **LC-STEP-17 — complete (LC-AC-08/14; depends on STEP-16):** use the existing canonical
    UNTIL node in the neutral controlled-loop builder only after confirming
    its post-body check and ITERATE path against the Classic reference. Reuse
    the setup-free condition guard and hidden source-loop binding, add exact
    `34.4` validation in shared RexxValue, and keep combined headers and
    setup-bearing expressions fail-closed. Qualify and commit separately.
18. **LC-STEP-18 — complete (LC-AC-08/15; depends on STEP-17):** admit the parser's
    `DO > REPEAT > WHILE/UNTIL > INSTRUCTIONS` shape only for a proven literal
    count and setup-free condition. Place FOR and the condition under one
    controlled canonical REPEAT; verify count/condition ordering and both
    transfer paths with bounded Classic reference fixtures. Keep dynamic
    counts, controlled variables and setup-bearing expressions fail-closed.
    Qualify and commit this combined-header increment separately.
19. **LC-STEP-19 — complete (LC-AC-08/16; depends on STEP-18):** replace the Level C
    branch materialisation of `&`/`|` with left-to-right eager lowering.
    Capture the left `RexxValue` before any right setup statements, then
    evaluate the right once and call shared contextual logical methods.
    Prove side effects and `34.5`/`34.6` against Classic references, preserve
    source anchors and scope, run focused/normal/linked/Debug evidence, then
    commit this semantic repair separately from the condition-block work.
20. **LC-STEP-20 — complete (LC-AC-08/17; depends on STEP-19):** for direct `DO WHILE`,
    lower a supported setup-bearing condition into a canonical `BLOCK_EXPR`
    whose instructions are the condition prelude followed by `LEAVE WITH`
    the contextual logical value. Preserve the existing direct expression
    path when no setup is generated. Prove the block expression executes on
    every entry, its generated names remain in scope, and inner LEAVE WITH
    does not disturb source LEAVE/ITERATE. Keep UNTIL/combined setup-bearing
    forms fail-closed, qualify and commit this structural increment.
21. **LC-STEP-21 — complete (LC-AC-08/18; depends on STEP-20):** admit supported
    setup-bearing conditions for direct `DO UNTIL` and reuse the canonical
    `BLOCK_EXPR` condition builder from STEP-20. Prove the generated prelude
    runs only after the body and on ITERATE, with the correct source loop
    association and `34.4` identity. Remove the obsolete unsupported fixture,
    keep combined headers fail-closed, qualify and commit this increment.
22. **LC-STEP-22 — complete (LC-AC-08/19; depends on STEP-21):** allow supported
    setup-bearing WHILE/UNTIL conditions under the existing literal-count
    canonical REPEAT. Reuse the condition `BLOCK_EXPR` builder while retaining
    the no-setup direct path. Confirm zero-count suppression, limit-boundary
    check timing, ITERATE and LEAVE behavior against Regina. Remove the
    obsolete combined-setup negative fixture, retain dynamic/controlled
    guards, qualify and commit this increment.
23. **LC-STEP-23 — complete (LC-AC-08/20; depends on STEP-22):** recognize the parser's
    childless `REPEAT("forever")` followed by WHILE/UNTIL, and omit FOR in the
    existing canonical controlled-loop builder. Reuse condition block
    expressions and source-loop transfer binding. Prove zero-entry versus
    post-body semantics, ITERATE and LEAVE, remove the obsolete negative
    fixture, qualify and commit this structural increment.
24. **LC-STEP-24 — complete (LC-AC-08/21; depends on STEP-23):** add a contextual
    `RexxValue` whole-count conversion and permit supported direct repetition
    expressions. Lower count setup under the canonical FOR as a BLOCK_EXPR so
    it runs once, reuse the neutral builder and hidden source-loop binding,
    and retain fail-closed guards on combined dynamic and controlled forms.
    Prove valid whole spellings, zero, mutation, BIF side effects and `26.2`
    boundaries against the references; qualify and commit separately.
25. **LC-STEP-25 — complete (LC-AC-08/22; depends on STEP-24):** permit the already
    bounded dynamic FOR conversion and setup block alongside a supported
    WHILE/UNTIL condition under one canonical REPEAT. Prove runtime setup
    order and zero-count suppression against Regina, including BIF/local
    calls and transfer paths. Remove the obsolete dynamic-combined negative
    fixture, keep controlled headers fail-closed, qualify and commit.

## Open findings

- **LC-FIND-01 — empty source string literal:** the independent minimal
  `options levelc; say ''` source fails during lowering, although the parser
  accepts its STRING node. It was exposed by a first combined-loop fixture;
  that fixture now uses nonempty comparison operands to keep the AST loop
  increment scoped. Investigate and repair as a separate expression increment
  with a permanent regression. Reproducer log:
  `/tmp/crexx-levelc-empty-literal-log.LX2Ksv`.

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
| `VAR_SYMBOL`/`VAR_TARGET`, strings, integers, expression operators, function calls | Slice: proven scalar/compound pool reads, literal and operator methods, eager Classic `&`/`|`, bounded BIF/local calls | More expression shapes, remaining operator order, numeric context and missing-argument behavior remain open |
| `IF` with condition/THEN/ELSE; simple `DO` with `INSTRUCTIONS` | Slice: recursive guards and canonical branch/group builders, including nested forms | More accepted arm statements and source/scope proof as forms expand |
| `SELECT` with `INSTRUCTIONS` of `WHEN` and optional `OTHERWISE` | Slice: guarded list lowers to nested canonical `IF`/one-shot `DO`, including nested arms, local procedures, `34.2` and `7.3` | Broader statement arms, condition lifecycle and profile proof remain open |
| Header-bearing `DO`, `REPEAT`, `FOR`, `WHILE`, `UNTIL`, `BY`, `TO`, `LEAVE`, `ITERATE` | Slice: literal and bounded dynamic direct/combined counts, FOREVER, WHILE/UNTIL including setup-bearing conditions, plus childless LEAVE/ITERATE bound to the nearest source repetitive DO through a hidden canonical target | Wider count values, named transfer, controlled headers, numeric errors and wider scope remain open |
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
| Expressions | precedence, arithmetic, comparisons, concatenation, prefix, eager logical `&`/`|` | Slice: `levelc_slice6_expressions` and `levelc_slice19_logical_eager` | Full numeric context, remaining operator order, boundary/error and platform equivalence |
| Variables | scalar read/write, drop, compound names, bare stems, exposure, API pool | Slice: scalar/compound read/write, scalar/stem EXPOSE, unset scalar read and direct scalar DROP | Remaining stem/compound/indirect DROP, external/API operations and aliasing |
| Control | IF/THEN/ELSE | Slice: `levelc_slice7_if_else`, nested and procedure fixtures, opt/no-opt, invalid logical and unsupported-arm tests | Other instructions in arms and broader condition/message lifecycle remain open |
| Control | simple DO/END | Slice: `levelc_slice8_do_block` and nested/empty/procedure fixtures, opt/no-opt, tree-shape and linked execution | Broader clause lifecycle and conditions remain open |
| Control | counted/controlled/repetitive DO, WHILE/UNTIL, LEAVE/ITERATE | Slice: literal and bounded dynamic direct/combined counts, FOREVER and WHILE/UNTIL including setup-bearing conditions, and childless LEAVE/ITERATE across generated IF/SELECT/simple-DO wrappers | Wider count values, controlled variables, named transfer, exact numeric errors |
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
| `SYN-CLASSIC-DO` | Simple, counted, conditional, and forever DO | Bounded slices: simple DO, literal and bounded dynamic direct/combined counts, FOREVER and conditional headers | Controlled forms, wider count/numeric errors and configuration proof open |
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

### LC-STEP-13 — literal counted DO, 2026-10-03

- Before-change, Regina executed `DO 2` twice while compiled Level C rejected
  the parser's `DO > REPEAT > FOR > INTEGER` header. The reproducer and raw
  AST are retained at `/tmp/crexx-levelc-count-before.CVPYiy`.
- A neutral counted-DO builder now receives a fresh canonical count node.
  The Level C guard accepts only a non-negative decimal integer literal
  representable by the current `.int` builder, with no extra repetition or
  condition sibling. All other loop headers remain fail-closed; the prior
  unsupported fixture now covers a control-variable loop.
- The eleven-line zero/nested/IF/procedure fixture matched Regina in optimized
  and no-opt runs. Release build passed, focused tests 5/5, normal Level C and
  source-provenance suite 124/124, and linked RXBIN execution reproduced the
  eleven lines. Debug tree output showed source-anchored generated loops and
  no AST validation failure. Retained logs:
  `/tmp/crexx-levelc-count-build.913mV7`,
  `/tmp/crexx-levelc-count-focused.OBuaIY`,
  `/tmp/crexx-levelc-count-suite.zVJrGT`,
  `/tmp/crexx-levelc-count-debug.uso2v0`,
  `/tmp/crexx-levelc-count-link.6iwRyj`.
- Dynamic repetition counts, controlled loops, WHILE/UNTIL, LEAVE/ITERATE and
  their exact runtime errors remain open under `LC-AC-08/04`.
- A focused Debug build passed. The first Debug CTest attempt had one `Not
  Run` because `test_levelc_tree_boundary` had not been built in that tree;
  after building it, the focused Debug set passed 6/6. Logs:
  `/tmp/crexx-levelc-count-debug-build.wX76xJ`,
  `/tmp/crexx-levelc-count-debug-focused.9ijG98`,
  `/tmp/crexx-levelc-count-debug-unit-build.QGU5mL`,
  `/tmp/crexx-levelc-count-debug-focused-final.VkqpEj`.

### LC-STEP-14 — loop transfer across generated blocks, 2026-10-03

- The retained counterexample shows Regina leaving the source counted loop
  once, while an equivalent canonical tree with a bare `LEAVE` inside a
  generated `DO 1` branch only leaves that inner wrapper and continues the
  outer loop. This is the AST association risk Adrian prioritized. Reproducer:
  `/tmp/crexx-levelc-loop-association.Ecw4Bg`.
- Accepted literal counted loops now use a hidden canonical control symbol.
  Source childless `LEAVE` and `ITERATE` lower to named transfers to that
  symbol; existing canonical validation resolves the correct enclosing loop
  through generated wrappers. `ast_do()` and global association semantics are
  unchanged. Named Classic transfers remain outside this slice.
- The eight-line fixture matched Regina in optimized/no-opt and linked runs,
  covering IF, SELECT, nested simple/counted DO and a local procedure. A
  redirected tree probe showed source-anchored transfers and hidden typed
  control symbols without AST validation errors. Release build passed,
  focused transfer/count tests 6/6 plus a named-transfer diagnostic test,
  the selected normal Level C/source-provenance suite 128/128, and Debug
  focused tests 8/8. Retained logs:
  `/tmp/crexx-levelc-transfer-build.M5xxeb`,
  `/tmp/crexx-levelc-transfer-focused.518e63`,
  `/tmp/crexx-levelc-transfer-suite.BI7lhP`,
  `/tmp/crexx-levelc-transfer-debug-tree.3bjoKi`,
  `/tmp/crexx-levelc-transfer-link.RQWxpa`,
  `/tmp/crexx-levelc-transfer-debug-build.xUyrVm`,
  `/tmp/crexx-levelc-transfer-debug-focused.3t20z1`.
- `LC-AC-08/04` remain open for the other structural and full compatibility
  obligations; dynamic/controlled loops and named transfers are still open.

### LC-STEP-15 — DO FOREVER, 2026-10-03

- Before-change, Regina completed a guarded `DO FOREVER` example with output
  `3`, while Level C rejected its `DO > REPEAT("forever") > INSTRUCTIONS`
  shape. The raw-tree and reference receipts are retained at
  `/tmp/crexx-levelc-forever-before.S2g0km`.
- The guarded source shape now uses the existing hidden canonical loop symbol
  and named transfer mapping, with no FOR count. Unsupported extra header
  conditions remain fail-closed. The fixture has explicit termination guards
  so a wrong LEAVE target fails quickly instead of hanging a test runner.
- Regina and optimized/no-opt compiled execution matched all eight lines,
  including nested simple/SELECT/counted groups and a local procedure. Release
  build and focused tests 7/7 passed; the normal Level C/source-provenance
  suite passed 131/131. Debug focused tests passed 8/8. Linked execution
  matched Regina, and the redirected tree probe showed source-anchored
  transfers with no AST validation errors. Retained logs:
  `/tmp/crexx-levelc-forever-build.EXyQ2y`,
  `/tmp/crexx-levelc-forever-focused.EiyMdU`,
  `/tmp/crexx-levelc-forever-suite.t6MpZR`,
  `/tmp/crexx-levelc-forever-link.K1bR9s`,
  `/tmp/crexx-levelc-forever-tree.QvkEh6`,
  `/tmp/crexx-levelc-forever-debug-build.rEPkRy`,
  `/tmp/crexx-levelc-forever-debug-focused.pwsTd1`.
- `LC-AC-08/04` remain open for dynamic and controlled counts, WHILE/UNTIL,
  named transfers and other structural families.

### LC-STEP-16 — bounded DO WHILE, 2026-10-03

- The source `DO > WHILE > INSTRUCTIONS` shape lowers to a canonical
  controlled loop with WHILE under REPEAT. The condition stays on the loop
  check path and is evaluated before each iteration, including the first.
  The validator admits only supported expressions that emit no setup
  statements; the lowerer asserts that invariant again. Short-circuit,
  function and compound-tail condition forms remain fail-closed.
- Childless LEAVE/ITERATE reuse the source-loop hidden target from STEP-14.
  The shared `RexxValue.logicalWhileValue()` supplies exact `34.3` errors;
  no new AST node type was needed. Adrian permits a new type where the full
  validation-to-emitter path is simpler and more supportable.
- Regina and optimized/no-opt compiled output matched for zero-entry,
  condition reevaluation, nested wrappers, counted nesting and a local
  procedure. Release focused tests passed 5/5, selected normal Level C and
  source-provenance tests 136/136, shared RexxValue/RexxScript tests 8/8,
  and Debug structural/loop focused tests 13/13. Linked RXBIN output matched
  Regina. A redirected tree probe showed WHILE and logical calls anchored to
  source lines with no AST validation diagnostic. Retained logs:
  `/tmp/crexx-levelc-while-build.JsRHRZ`,
  `/tmp/crexx-levelc-while-focused.FPR0uT`,
  `/tmp/crexx-levelc-while-suite.pljBeV`,
  `/tmp/crexx-levelc-while-shared.1gClVb`,
  `/tmp/crexx-levelc-while-debug-build.NT8SKe`,
  `/tmp/crexx-levelc-while-debug-runtime.8bBCRi`,
  `/tmp/crexx-levelc-while-debug-focused.EBYGJh`,
  `/tmp/crexx-levelc-while-link-log.Yauzdi`, and
  `/tmp/crexx-levelc-while-tree.0MmH0y`.
- The final comment-only source tidy rebuilt Release and Debug `rxc`; the
  combined normal Level C/source-provenance/shared-runtime selection passed
  144/144, and Debug structural/loop focused tests passed 13/13. Final logs:
  `/tmp/crexx-levelc-while-release-final-build.JnHXOE`,
  `/tmp/crexx-levelc-while-debug-final-build.v4z9zZ`,
  `/tmp/crexx-levelc-while-release-final-tests.TYllNI`, and
  `/tmp/crexx-levelc-while-debug-final-tests.E0en89`.
- `LC-AC-08/04` remain open for UNTIL, setup-bearing conditions and the
  other structural and full compatibility families.

### LC-STEP-17 — bounded DO UNTIL, 2026-10-03

- Source `DO > UNTIL > INSTRUCTIONS` now lowers through the existing
  canonical end-check node under REPEAT. The condition executes after the
  first body and on ITERATE. The first Regina fixture mistakenly kept the
  condition false after changing its variable; it was corrected to become
  true after one iteration, and subsequent reference/VM runs used bounded
  output and time limits.
- The setup-free guard and hidden source-loop target are shared with WHILE;
  shared `RexxValue.logicalUntilValue()` reports `34.4`. Combined headers
  and setup-bearing expressions remain fail-closed. No new AST node type was
  needed for the existing end-check contract.
- Regina and optimized/no-opt compiler output matched for first-body
  execution, post-body checks, ITERATE through a generated DO, LEAVE through
  SELECT, nested counted loops and a local procedure. Release focused tests
  passed 6/6 and the selected normal Level C/source-provenance/shared-runtime
  suite passed 150/150. Debug structural/loop focused tests passed 19/19.
  Linked RXBIN output matched Regina; the redirected tree probe showed
  source-anchored UNTIL checks with no AST validation diagnostic. Logs:
  `/tmp/crexx-levelc-until-reference-output.2iuZg3`,
  `/tmp/crexx-levelc-until-release-build.ulMbQg`,
  `/tmp/crexx-levelc-until-direct-log.ySsHY7`,
  `/tmp/crexx-levelc-until-focused.F6YTOR`,
  `/tmp/crexx-levelc-until-suite.XL1FOh`,
  `/tmp/crexx-levelc-until-debug-build.vq3DBg`,
  `/tmp/crexx-levelc-until-debug-focused.NlTVt0`,
  `/tmp/crexx-levelc-until-link.3yt1BJ`, and
  `/tmp/crexx-levelc-until-tree.ZF35LG`.
- `LC-AC-08/04` remain open for setup-bearing conditions, combined and
  controlled loops, named transfers and the other structural families.

### LC-STEP-18 — literal count with WHILE/UNTIL, 2026-10-03

- The parser's `REPEAT` followed by WHILE or UNTIL now lowers when its FOR
  expression is a non-negative integer literal and its condition is setup
  free. The neutral controlled-loop builder places FOR and the condition in
  one canonical REPEAT; the source-loop hidden target continues to bind
  childless LEAVE/ITERATE across generated IF/DO/SELECT wrappers. Dynamic
  counts, FOREVER with a condition and setup-bearing conditions remain
  fail-closed. The prior negative combined-header fixture was removed when
  that exact form became executable, with new negative cases for the
  remaining unsupported headers.
- Regina and compiled opt/no-opt output matched for count-limited WHILE,
  UNTIL after-body checks, zero counts that skip nonlogical conditions,
  ITERATE, LEAVE through SELECT and a local procedure. Release focused tests
  passed 8/8 and the selected normal Level C/source-provenance/shared-runtime
  suite passed 157/157. Debug structural/loop focused tests passed 26/26.
  Linked RXBIN output matched Regina. The redirected tree probe showed FOR
  and WHILE/UNTIL under each accepted canonical REPEAT, with source lines and
  no AST validation diagnostic. Logs:
  `/tmp/crexx-levelc-combined-reference-output.dulkdk`,
  `/tmp/crexx-levelc-combined-release-build.sxX4Up`,
  `/tmp/crexx-levelc-combined-release-final-build.B7WfWR`,
  `/tmp/crexx-levelc-combined-direct-log.5H8bnC`,
  `/tmp/crexx-levelc-combined-release-final-focused.ihtUU8`,
  `/tmp/crexx-levelc-combined-suite.CAS8Ce`,
  `/tmp/crexx-levelc-combined-debug-build.ikZRTX`,
  `/tmp/crexx-levelc-combined-debug-focused.XHy7K8`,
  `/tmp/crexx-levelc-combined-link.Nym16W`, and
  `/tmp/crexx-levelc-combined-tree.ISdWvo`.
- `LC-AC-08/04` remain open for dynamic/controlled and setup-bearing loop
  headers, named transfers and the other structural families.

### LC-STEP-19 — eager Classic logical operands, 2026-10-03

- A local Regina side-effect reproducer and the
  [official ooRexx reference](https://www.oorexx.org/docs/pdf/rexxref.pdf)
  confirm that Classic `&` evaluates both operands; `|` follows the same
  source-order binary-operator rule. The prior branch materialisation skipped
  the right operand when the left determined the result. This was a separate
  pre-existing expression defect found while probing WHILE condition setup.
- The Level C lowerer now captures a copy of the left `RexxValue` before
  lowering right setup statements, then calls shared `logicalAnd()` or
  `logicalOr()` with the right value. The methods enforce contextual
  `34.5`/`34.6`. The old expression fixture's side-effect expectations and
  manual Level B target were corrected to the Classic reference; its first
  unchanged run failed in exactly those old expectations before correction.
  The block-expression WHILE work was held in a local stash and remains a
  distinct increment.
- Regina and optimized/no-opt compiled output matched for both operators,
  nested expressions, right-side calls on determining left values and a
  right-side mutation of the left variable. Release focused tests passed 9/9
  after the expected fixture correction; the selected normal
  Level C/source-provenance/shared-runtime suite passed 163/163. Debug focused
  tests passed 16/16. Linked RXBIN output matched Regina. The redirected
  tree probe showed source-anchored left snapshots and `logicalAnd`/
  `logicalOr` calls with no AST validation diagnostic. Logs:
  `/tmp/crexx-levelc-logical-eager-reference-output.72prLS`,
  `/tmp/crexx-levelc-logical-old-reference-output.QJx8aL`,
  `/tmp/crexx-levelc-logical-eager-release-build.WrFSVV`,
  `/tmp/crexx-levelc-logical-eager-focused.MFAkwl`,
  `/tmp/crexx-levelc-logical-eager-focused-final.7LpcTC`,
  `/tmp/crexx-levelc-logical-eager-suite.sdJEh8`,
  `/tmp/crexx-levelc-logical-eager-debug-build.BVglbq`,
  `/tmp/crexx-levelc-logical-eager-debug-focused.Zshigu`,
  `/tmp/crexx-levelc-logical-eager-link-log.VXRbv9`, and
  `/tmp/crexx-levelc-logical-eager-tree.9P77iG`.
- `LC-AC-08/04` remain open for the other expression and structural families.

### LC-STEP-20 — setup-bearing direct DO WHILE, 2026-10-03

- Direct `DO WHILE` accepts supported expressions with lowering setup. The
  neutral builder wraps setup statements and `LEAVE WITH` of the contextual
  logical value in the existing canonical `BLOCK_EXPR`; no new AST node or
  emitter path is needed. Setup-free conditions keep their direct expression
  shape. Direct UNTIL and count-plus-condition headers still reject setup
  because their distinct timing/limit paths need proof.
- A bounded Regina fixture and optimized/no-opt compiled runs matched for
  eager logical side effects, a per-entry SUBSTR BIF argument frame, changing
  compound tails, childless LEAVE/ITERATE through generated wrappers and a
  local procedure. Nonlogical conditions report `34.3`. The previous
  unsupported-WHILE fixture was removed when that form became executable.
  Release focused tests passed 4/4, the selected normal Level C/source-
  provenance/shared-runtime suite passed 166/166, and Debug structural/loop
  tests passed 28/28. Linked RXBIN output matched Regina. The redirected tree
  probe showed source-anchored BLOCK_EXPR, LEAVE_WITH and BIF context nodes
  with no AST validation error. Retained evidence:
  `/tmp/crexx-levelc-while-setup-reference-output.hlkmtT`,
  `/tmp/crexx-levelc-while-block-release-build.MoY4Cx`,
  `/tmp/crexx-levelc-while-block-direct-log.QGCOrX`,
  `/tmp/crexx-levelc-while-block-focused.zMyYeC`,
  `/tmp/crexx-levelc-while-block-release-suite.ui8xzl` (obsolete negative
  fixture, then removed),
  `/tmp/crexx-levelc-while-block-release-reconfigure.WsupOj`,
  `/tmp/crexx-levelc-while-block-release-final-suite.HdrpiK`,
  `/tmp/crexx-levelc-while-block-debug-build.qeDYFh`,
  `/tmp/crexx-levelc-while-block-debug-focused.bo880Q`,
  `/tmp/crexx-levelc-while-block-link.irp1c4`, and
  `/tmp/crexx-levelc-while-block-tree.s1TS6B`.
- `LC-AC-08/04` remain open for setup-bearing UNTIL/combined headers,
  dynamic/controlled loops, named transfers and other structural families.

### LC-STEP-21 — setup-bearing direct DO UNTIL, 2026-10-03

- Direct `DO UNTIL` now admits the same supported setup-bearing condition
  expressions as direct WHILE. The existing canonical `BLOCK_EXPR` places
  condition setup at UNTIL's post-body check, including the ITERATE path;
  setup-free conditions remain direct. Count-plus-condition headers still
  reject setup. No new AST node or emitter shape was needed.
- Bounded Regina and optimized/no-opt compiled output matched for an initially
  true condition checked only after the body, eager side effects after
  ITERATE, per-check SUBSTR BIF frames, changing compound tails, childless
  LEAVE/ITERATE through generated blocks and a local procedure. A nonlogical
  setup-bearing condition reports `34.4`. The obsolete unsupported-UNTIL
  fixture was removed. Release focused tests passed 4/4; the selected normal
  Level C/source-provenance/shared-runtime suite passed 169/169. Debug
  structural/loop tests passed 31/31. Linked RXBIN output matched Regina.
  The redirected tree probe showed source-anchored UNTIL > BLOCK_EXPR >
  LEAVE_WITH and a BIF context, with no AST validation error. Evidence:
  `/tmp/crexx-levelc-until-setup-reference-output.PUMS4G`,
  `/tmp/crexx-levelc-until-block-release-build.C8LELR`,
  `/tmp/crexx-levelc-until-block-direct-log.NsPSk4`,
  `/tmp/crexx-levelc-until-block-focused.tVTwVb`,
  `/tmp/crexx-levelc-until-block-release-suite.eZu6Hq`,
  `/tmp/crexx-levelc-until-block-debug-build.GKVpZm`,
  `/tmp/crexx-levelc-until-block-debug-focused.P0RHvW`,
  `/tmp/crexx-levelc-until-block-link.LrLH8w`, and
  `/tmp/crexx-levelc-until-block-tree.VWYALR`.
- `LC-AC-08/04` remain open for setup-bearing combined headers,
  dynamic/controlled loops, named transfers and other structural families.

### LC-STEP-22 — literal count with setup-bearing WHILE/UNTIL, 2026-10-03

- The literal-count combined headers now reuse the canonical condition
  BLOCK_EXPR builder. The obsolete setup-free expression guard and its
  negative fixture were removed; the literal-count, dynamic-count and
  FOREVER-with-condition guards remain. A zero count skips body and condition
  setup. WHILE does not check after reaching its count limit; UNTIL checks
  after the final body. Both follow Regina's eager side-effect counts.
- A bounded Regina fixture matched optimized/no-opt output for zero-count
  suppression, condition and count limits, BIF argument frames, ITERATE and
  LEAVE through generated blocks, local procedure scope, and `34.3`/`34.4`
  errors. The first fixture exposed independent `LC-FIND-01`; replacing its
  empty comparison operand with supported nonempty operands kept this
  increment scoped. Release focused tests passed 5/5; the selected normal
  Level C/source-provenance/shared-runtime suite passed 173/173. Debug
  structural/loop tests passed 35/35. Linked RXBIN output matched Regina.
  The redirected tree probe showed source-anchored FOR with WHILE/UNTIL
  BLOCK_EXPR and LEAVE_WITH nodes, with no AST validation error. Evidence:
  `/tmp/crexx-levelc-combined-setup-reference-output-v2.w5TaoW`,
  `/tmp/crexx-levelc-combined-block-release-build.73av2a`,
  `/tmp/crexx-levelc-combined-block-direct-log-v2.7S7eH9`,
  `/tmp/crexx-levelc-combined-block-focused.Ni1Hdp`,
  `/tmp/crexx-levelc-combined-block-release-suite.56iMw2`,
  `/tmp/crexx-levelc-combined-block-debug-build.kDTS4J`,
  `/tmp/crexx-levelc-combined-block-debug-focused.0HdBZ9`,
  `/tmp/crexx-levelc-combined-block-link.kpLZoK`, and
  `/tmp/crexx-levelc-combined-block-tree.MzWV5G`.
- `LC-AC-08/04` remain open for dynamic/controlled loop headers, named
  transfers, numeric errors and the other structural families.

### LC-STEP-23 — DO FOREVER WHILE/UNTIL, 2026-10-03

- The parsed childless FOREVER REPEAT can now carry WHILE or UNTIL. The
  existing neutral controlled-loop builder omits FOR, reuses the condition
  BLOCK_EXPR when setup is needed, and preserves the hidden source loop
  target. The obsolete unsupported FOREVER-condition fixture was removed.
- Bounded Regina and optimized/no-opt compiled output matched for zero-entry
  WHILE, post-body UNTIL, eager side effects, per-check BIF frames,
  LEAVE/ITERATE through generated blocks, a local procedure, and contextual
  `34.3`/`34.4` errors. Release focused tests passed 5/5; the selected normal
  Level C/source-provenance/shared-runtime suite passed 177/177. Debug
  structural/loop tests passed 42/42. Linked RXBIN output matched Regina.
  The redirected tree probe showed source-anchored REPEAT with WHILE/UNTIL
  BLOCK_EXPR and LEAVE_WITH nodes, and no FOR child on the source FOREVER
  loops. Evidence:
  `/tmp/crexx-levelc-forever-condition-reference-output.rCALZY`,
  `/tmp/crexx-levelc-forever-block-release-build.OHiWpe`,
  `/tmp/crexx-levelc-forever-block-direct-log.Tfs2mU`,
  `/tmp/crexx-levelc-forever-block-focused.vU9vI7`,
  `/tmp/crexx-levelc-forever-block-release-suite.VzhBl8`,
  `/tmp/crexx-levelc-forever-block-debug-build.Desvx2`,
  `/tmp/crexx-levelc-forever-block-debug-focused.4md9Cz`,
  `/tmp/crexx-levelc-forever-block-link.Apjgj4`, and
  `/tmp/crexx-levelc-forever-block-tree.9eUean`.
- `LC-AC-08/04` remain open for dynamic/controlled headers, named
  transfers, numeric errors and the other structural families. `LC-FIND-01`
  remains open independently.

### LC-STEP-24 — bounded dynamic direct DO count, 2026-10-03

- Direct `DO expression` now lowers supported count expressions through
  `RexxValue.repeatCountValue()` into canonical FOR. Setup-bearing expressions
  use the same neutral BLOCK_EXPR and LEAVE_WITH builder as conditions so
  their setup runs once at loop entry. The bounded conversion uses whole
  number validation and decimal materialization before integer conversion;
  values outside signed 32-bit range report contextual `26.2`. Dynamic count
  plus WHILE/UNTIL and controlled DO remain fail-closed. The former negative
  literal compile fixture is now a `26.2` runtime regression.
- A bounded Regina fixture matched optimized/no-opt output for scalar counts
  held despite body mutation, zero entry, `2.0`/`2E0`, one local-call side
  effect, BIF setup under FOR, childless LEAVE/ITERATE and procedure scope.
  The first direct run exposed an unintended `CONVERSION_ERROR` for `2.0`
  from `asInt()`; the decimal conversion repair passed before wider testing.
  Invalid text, fraction, negative and over-range values report `26.2`,
  including a large positive exponent. Regina reports Error 33 for a
  negative repetition expression; this slice follows the repository's
  standard `26.2` catalog pending full diagnostic reconciliation under
  `LC-AC-04`.
- Release focused tests passed 9/9 after the tree fixture gained a SUBSTR
  count that actually emits setup. The selected normal
  Level C/source-provenance/shared-runtime suite passed 183/183; the later
  added large-exponent test passed separately 1/1 without changing product
  code. Debug structural/loop and shared-value tests passed 59/59. Linked
  RXBIN output matched Regina. The redirected tree probe showed a
  source-anchored FOR > BLOCK_EXPR > LEAVE_WITH > repeatCountValue path for
  SUBSTR, with no AST validation error. Evidence:
  `/tmp/crexx-levelc-dynamic-count-reference-output-v2.ifBks8`,
  `/tmp/crexx-levelc-dynamic-count-release-build.i3tNX6`,
  `/tmp/crexx-levelc-dynamic-count-direct-log.7ys7ae` (initial failure),
  `/tmp/crexx-levelc-dynamic-count-release-rebuild.cfDcwu`,
  `/tmp/crexx-levelc-dynamic-count-direct-log-v3.fBdb6W`,
  `/tmp/crexx-levelc-dynamic-count-focused-final.lT4NK1`,
  `/tmp/crexx-levelc-dynamic-count-release-suite.4R6vgr`,
  `/tmp/crexx-levelc-dynamic-count-exponent-focused.RinOfO`,
  `/tmp/crexx-levelc-dynamic-count-debug-build.ZVUWtV`,
  `/tmp/crexx-levelc-dynamic-count-debug-focused.ayKd6Z`,
  `/tmp/crexx-levelc-dynamic-count-link.BbnPLj`, and
  `/tmp/crexx-levelc-dynamic-count-tree.JvWOu8`.
- Dynamic repetition counts now combine with supported WHILE/UNTIL conditions
  under one canonical REPEAT. Count conversion and any BIF/local-call setup
  run once under FOR; a zero count skips the condition and its setup. WHILE
  checks at entry, UNTIL after the body, including the count-limited final
  iteration. The bounded Regina fixture matched optimized/no-opt and linked
  RXBIN output. Release focused tests passed 6/6 and the selected normal
  Level C/source-provenance/shared-runtime suite passed 189/189. Debug
  structural/loop and shared-value tests passed 64/64. The redirected tree
  probe showed source-anchored FOR > repeatCountValue and independent
  WHILE/UNTIL > BLOCK_EXPR > LEAVE_WITH paths, with no AST validation error.
  Evidence: `/tmp/crexx-levelc-dynamic-combined-reference-output.UG88oS`,
  `/tmp/crexx-levelc-dynamic-combined-release-build.v9hJq7`,
  `/tmp/crexx-levelc-dynamic-combined-direct-log.jJs2jq`,
  `/tmp/crexx-levelc-dynamic-combined-focused.CrPv4i`,
  `/tmp/crexx-levelc-dynamic-combined-release-suite.eeyvBk`,
  `/tmp/crexx-levelc-dynamic-combined-debug-build.eBz0yG`,
  `/tmp/crexx-levelc-dynamic-combined-debug-focused.R4FHmc`,
  `/tmp/crexx-levelc-dynamic-combined-link.zqKoCK`, and
  `/tmp/crexx-levelc-dynamic-combined-tree.Nbveg1`.
- `LC-AC-08/04` remain open for wider count values, controlled headers,
  named transfers and other structural families.
  `LC-FIND-01` remains open.
