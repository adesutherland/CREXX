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
After the four-item PARSE probe, Adrian rejected target-count-specific
lowering as the lasting design. `LC-AC-50` removes that arbitrary boundary for
direct word/dot templates before work resumes on positions and patterns.
The next variable-list increments put symbol resolution and indirect-list
execution in the shared Classic pool. Level C will preserve source order and
capture the value of a parenthesized reference at its position; the runtime
will interpret that subsidiary list. This keeps variable semantics available
to RexxScript without sharing the two products' statement parsers.

On 2026-10-03 Adrian changed the delivery unit: future implementation is
planned by **whole instruction at minimum**, with cross-cutting value,
variable, expression, BIF and lifecycle foundations planned together where
they serve several instructions. Fine-grained cases remain regression tests;
they are not independent feature-completion claims. An instruction stays open
until its valid forms, errors, nested contexts, source/evaluation order,
configuration and relevant host lifecycle are proved or a specific exception
is approved. A reviewable instruction may require several commits, but its
acceptance contract is set before those commits. Historical bounded criteria
below retain their evidence and IDs; their checkmarks do not close a whole
instruction. The [implementation review](../../../compiler/docs/levelc_working_architecture.md#2026-10-03-implementation-review-simplify-before-expansion)
records the simplification candidates and approval gates.
The instruction queue below is strict: one active instruction at a time.
Inventory its complete grammar/reference contract, review and simplify its
implementation, finish the missing forms, qualify it, close it, then start
the next row. Significant work, missing infrastructure or an awkward
implementation is not infeasibility and stays in scope. Reserve an
"infeasible" finding for a fundamental language feature or capability that
cannot be delivered without a serious compromise elsewhere. Record the
conflict, alternatives, affected behavior and evidence; the instruction
remains open until Adrian decides and approves the specific disposition.
Cross-cutting repairs are made when required by the active instruction and
are verified against all affected consumers.

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
- [x] **LC-AC-23 — bounded controlled TO loop:** parsed
  `DO scalar = non-negative integer TO non-negative integer` initializes the
  visible Classic variable pool, checks the bound before each body, advances
  the current pool value by one after each ordinary or ITERATE path, and
  leaves the correct post-loop or LEAVE value. A body assignment to the
  control variable affects the next step; a nonnumeric control value reports
  contextual `41.6`. Preserve nested source LEAVE/ITERATE ownership, local
  procedure scope, and opt/no-opt behavior. Verify bounded Regina output,
  linked execution, canonical WHILE plus end-check BLOCK_EXPR tree shape,
  focused runtime/Debug tests and the relevant correctness suite. BY, FOR,
  dynamic endpoints, named transfer, and combined conditions remain open.
- [x] **LC-AC-24 — bounded controlled BY step:** parsed scalar
  `DO name = non-negative integer TO non-negative integer BY signed integer`
  accepts TO/BY in either parsed order. A negative step checks the lower
  bound and advances downward; a zero or positive step checks the upper
  bound. The visible pool variable advances after ordinary and ITERATE paths,
  preserves body mutation, and retains the correct value after normal exit
  or LEAVE. Prove guarded zero-step behavior, opposite-direction zero-entry,
  nested transfers, and opt/no-opt parity. Verify against Regina, linked
  execution, source-anchored canonical WHILE/UNTIL tree shape, focused
  normal/Debug tests and the relevant correctness suite. Dynamic BY, FOR,
  dynamic endpoints, named transfer and combined conditions remain open.
- [x] **LC-AC-25 — bounded controlled TO with FOR:** parsed scalar
  `DO name = non-negative integer TO non-negative integer [BY signed integer]
  FOR non-negative integer` applies a signed 32-bit bounded FOR limit alongside the TO
  direction check under one source loop, regardless of parsed modifier order.
  A zero FOR count initializes the visible control variable and skips the
  body; a positive limit advances the pool value after each body or ITERATE,
  including the final count-limited body. Body mutation and LEAVE retain
  Classic final values. Verify Regina output, opt/no-opt and linked execution,
  canonical FOR plus WHILE/UNTIL tree shape, focused normal/Debug checks and
  the relevant correctness suite. Dynamic FOR, FOR without TO, dynamic
  controlled endpoints, named transfer and combined conditions remain open.
- [x] **LC-AC-26 — bounded controlled FOR without TO:** parsed scalar
  `DO name = non-negative integer [BY signed integer] FOR non-negative integer`
  applies a signed 32-bit bounded repetition limit without an entry TO check.
  The visible pool variable initializes even for zero count, advances after
  each ordinary or ITERATE path, and retains the Classic final or LEAVE
  value; body mutation and both BY clause orders behave as in Regina.
  Verify bounded Regina output, opt/no-opt and linked execution, canonical
  FOR plus UNTIL/BLOCK_EXPR tree shape with no synthetic WHILE, focused
  normal/Debug tests and the relevant correctness suite. Unbounded controlled
  DO, dynamic FOR/BY/start, named transfer and combined conditions remain open.
- [x] **LC-AC-27 — named controlled-loop transfer:** named `LEAVE name` and
  `ITERATE name` resolve a case-insensitive name to the matching active,
  supported controlled DO, including an outer loop across nested controlled
  loops, simple DO groups, IF and generated branch blocks. ITERATE reaches
  that loop's end step and next entry check; LEAVE skips its end step. The
  visible pool values, skipped statements and matched outer target agree
  with Regina, while invalid names retain `28.3`/`28.4` diagnostics. Verify
  bounded Regina output, opt/no-opt and linked execution, source-anchored
  lowered target associations, focused normal/Debug tests and the relevant
  correctness suite. Named transfer to wider unsupported controlled shapes
  remains fail-closed.
- [x] **LC-AC-28 — bounded controlled WHILE entry:** parsed scalar controlled
  `DO name = literal [TO literal] [BY signed literal] [FOR literal]
  WHILE expression` with at least one TO or FOR applies a supported WHILE
  condition at each permitted entry after the FOR and TO guards. A zero FOR
  or failed TO guard does not
  evaluate WHILE or its setup; ITERATE advances the visible control variable
  before the next entry check, while LEAVE skips that advance. Preserve named
  and childless source-loop ownership, contextual `34.3`, local procedure
  scope, and opt/no-opt behavior. Verify bounded Regina output and errors,
  linked execution, a source-anchored canonical entry BLOCK_EXPR tree,
  focused Release/Debug checks and the relevant correctness suite. Controlled
  UNTIL and dynamic controlled clauses remain open.
- [x] **LC-AC-29 — bounded controlled UNTIL end check:** parsed scalar
  controlled `DO name = literal [TO literal] [BY signed literal]
  [FOR literal] UNTIL expression` with at least one TO or FOR evaluates a
  supported UNTIL condition after each executed body and on ITERATE, before
  advancing the visible control variable. A true UNTIL leaves the current
  value unadvanced; a false UNTIL advances it, including after the final
  FOR-limited body. Zero FOR and failed TO entry guards skip the condition.
  LEAVE skips both condition and step. Preserve setup timing, named and
  childless loop ownership, contextual `34.4`, local scope, and opt/no-opt
  behavior. Verify bounded Regina output and errors, linked execution,
  source-anchored canonical end BLOCK_EXPR tree, focused Release/Debug checks
  and the relevant correctness suite. Dynamic controlled clauses remain open.
- [x] **LC-AC-30 — guarded unbounded controlled DO:** parsed scalar
  `DO name = non-negative integer [BY signed integer]` with optional supported
  WHILE or UNTIL condition runs without a synthetic TO or FOR limit. The
  visible pool variable advances after ordinary and ITERATE paths, unless a
  true UNTIL or LEAVE exits before the step; WHILE checks at entry. Guarded
  fixtures prove termination, body mutation, named and childless transfers,
  local scope, final control values and opt/no-opt parity against Regina.
  Verify canonical REPEAT with no FOR and no synthetic WHILE for bare/BY
  forms, source anchors, linked execution, focused Release/Debug checks and
  the relevant correctness suite. Dynamic controlled clauses remain open.
- [x] **LC-AC-31 — captured dynamic TO endpoint:** parsed scalar controlled
  `DO name = non-negative integer TO expression` with optional signed literal
  BY, bounded literal FOR, and a supported WHILE or UNTIL condition evaluates
  a supported TO expression exactly once before assigning the new visible
  control variable, so it observes any previous value. Later mutation of
  its source variables does not change
  the endpoint; a nonnumeric endpoint reports contextual `41.4` at setup,
  including when FOR is zero. FOR-zero still evaluates the TO expression at
  loop setup as Regina does. Preserve
  nested/named transfer ownership, condition timing, source anchors and
  opt/no-opt behavior. Verify bounded Regina output and errors, a captured
  canonical value in the lowered tree, linked execution, focused
  Release/Debug checks and the relevant correctness suite. Dynamic start,
  BY and FOR remain open.
- [x] **LC-AC-32 — captured dynamic BY step:** parsed scalar controlled
  `DO name = non-negative integer [TO literal-or-expression]
  BY expression [FOR literal]` accepts a supported BY expression evaluated
  and validated exactly once before the new visible control assignment.
  Dynamic TO and BY clauses evaluate in their written order, both seeing
  any prior control value; later source mutation changes neither capture.
  A nonnumeric BY reports contextual `41.5` at setup, including FOR-zero.
  Preserve positive, negative and guarded zero steps, TO direction,
  ITERATE/LEAVE timing, optional WHILE/UNTIL, local scope and opt/no-opt
  behavior. Verify bounded Regina output and errors, hidden canonical
  captures with source order, linked execution, focused Release/Debug checks
  and the relevant correctness suite. Dynamic start and FOR remain open.
- [x] **LC-AC-33 — captured dynamic controlled FOR count:** parsed scalar
  controlled `DO name = non-negative integer [TO literal-or-expression]
  [BY signed-literal-or-expression] FOR expression` evaluates a supported
  FOR expression once in written clause order with TO/BY, before the new
  visible control assignment. The resulting non-negative whole count is
  bounded to signed 32-bit and reports contextual `26.3` for invalid or
  out-of-range values. Zero still evaluates and validates every header
  clause but skips the body and WHILE/UNTIL checks; positive counts preserve
  the final pool value and ITERATE/LEAVE timing even if source variables
  change. Verify bounded Regina output/errors, opt/no-opt, captured canonical
  FOR count and source anchors, linked execution, focused Release/Debug
  checks and the relevant correctness suite. Dynamic start and wider counts
  remain open.
- [x] **LC-AC-34 — loop-end call argument preservation:** optimized
  canonical controlled loops preserve captured by-value arguments across a
  method call and every later iteration, including when a zero-count loop
  precedes them. Calls within canonical UNTIL end checks use a disjoint
  argument window so their receiver and by-value argument slots cannot
  overwrite a value needed at the next entry. Other call affinity shapes
  retain their existing behavior. Verify the isolated
  literal-FOR/dynamic-BY counterexample in opt and no-opt, inspect emitted
  call marshalling, run the relevant normal compiler checks, and execute
  through the full toolchain. This is a general compiler correctness repair
  exposed by Level C, independent of dynamic FOR support.
- [x] **LC-AC-35 — captured dynamic controlled start:** parsed scalar
  `DO name = expression` with the already supported TO/BY/FOR and optional
  WHILE/UNTIL clauses evaluates a supported start expression exactly once
  before the other header expressions, while they still see the previous
  visible control value. It captures that result before later side effects,
  validates TO/BY/FOR in written order, then validates the captured start as
  numeric with contextual `41.6` before assigning the visible pool. Invalid
  later clauses retain their own prior error and evaluation timing. Zero FOR,
  entry/end checks, named transfer, local scope, final control value, source
  anchors and opt/no-opt behavior remain correct. Verify Regina output and
  error-side-effect order, a canonical lowered-tree probe, focused
  Release/Debug regressions, the relevant correctness suite and linked
  execution. Wider expression shapes and complete compatibility remain open.
- [x] **LC-AC-36 — empty quoted string lowering:** parsed Classic `''` and
  `""` literals lower to canonical empty string values with their source
  anchors, in SAY, assignment, concatenation and supported call contexts.
  They compile, assemble, link and execute with Regina-equivalent output in
  optimized and no-opt modes. Numeric use reports its contextual runtime
  error rather than a compile-time unsupported-shape error. Verify a minimal
  counterexample, focused positive and negative regressions, lowered-tree
  inspection, the relevant normal compiler suite and linked execution.
- [x] **LC-AC-37 — function call after a concatenated operand:** a Classic
  function call immediately followed by `(` remains a `FUNCTION` term when
  it follows another expression operand under blank concatenation, including
  a SAY literal and a variable. Its arguments, effects and result agree with
  Regina in opt/no-opt and linked execution. A blank between the function
  name and `(` continues to mean a symbol followed by a parenthesized term,
  as Regina does. Verify minimal reference counterexamples, raw and lowered
  AST shape, supported nested arguments, focused parser/runtime checks and
  the relevant normal compiler suite. Broader call syntax remains open.
- [x] **LC-AC-38 — direct ARG uppercase binding:** a supported direct
  local-procedure `ARG` binding applies Classic `PARSE UPPER ARG` conversion
  to each supplied argument before storing it in the callee's visible pool.
  The caller's value remains unchanged, scalar target order and exposure
  still work, and the shared Classic TRANSLATE implementation supplies the
  current Level C BYTE default mapping. End-to-end delivery of an opt-in
  UTF8 call configuration remains open under LC-AC-04/06. Verify lowercase
  and mixed-case Regina
  examples, opt/no-opt and linked execution, canonical argument-frame and
  pool-set tree shape, focused shared-runtime checks and the relevant normal
  compiler suite. Wider parse templates and external-call lifecycle remain
  open.
- [x] **LC-AC-39 — single-target PARSE source and template lowering:**
  `PARSE VAR scalar target` and `PARSE VALUE expression WITH target`, each
  with one direct scalar target and optional `UPPER`, evaluate their source
  once and write the target through the Classic visible pool. The source
  variable and caller state retain their values unless the target aliases
  them. The optional uppercase path uses shared Classic TRANSLATE with the
  current BYTE default. Other parse sources, multiple targets, patterns,
  positions and comma templates remain fail-closed. Verify Regina output,
  main/procedure and self-target behavior, opt/no-opt, raw and canonical tree
  shape, focused negatives, normal compiler checks and linked execution.
- [x] **LC-AC-40 — three-target implicit-word PARSE:** `PARSE VAR scalar
  first second tail` and `PARSE VALUE expression WITH first second tail`,
  with direct scalar targets and optional UPPER, use the existing `parsewords3`
  VM primitive through a shared `RexxValue` method. The source is evaluated
  and captured once before ordered pool writes; the first two targets receive
  words and the third receives the unparsed remaining tail, preserving blanks,
  source aliases and repeated target order. Other template counts and
  patterns remain guarded. Verify Regina across leading/repeated/trailing
  blanks, short input, aliases, opt/no-opt, canonical tree, shared-method
  behavior and linked execution, then commit separately.
- [x] **LC-AC-41 — abutted expression concatenation:** when adjacent Classic
  expression terms touch in the source, the parser emits abuttal `OP_CONCAT`
  rather than blank `OP_SCONCAT`, including literal/symbol chains and
  parenthesized or call terms whose punctuation is outside their operand AST.
  Comment-only gaps act as abuttal, while spaced terms remain `OP_SCONCAT`;
  explicit `||` retains its existing path.
  Verify minimal Regina counterexamples, raw and lowered AST, opt/no-opt,
  parser/highlighter checks, the normal Level C suite and linked execution.
  This closes `LC-FIND-04`; broader expression conformance remains open.
- [x] **LC-AC-42 — decimal fraction source tokens:** Classic constants with
  a decimal point and digits on at least one side (`1.5`, `1.`, `.5`) form one
  source numeric AST operand and retain their exact spelling as a `RexxValue`.
  They work in ordinary expressions, PARSE VALUE expressions and arithmetic;
  fractional DO/FOR counts still raise their existing contextual errors.
  Verify Regina output, raw/canonical tree, opt/no-opt, focused invalid-count
  tests, the normal Level C suite and linked execution. Exponents are covered
  by `LC-AC-43`; digit-starting constant symbols are covered by `LC-AC-44`.
- [x] **LC-AC-43 — exponent numeric source tokens:** valid Classic numeric
  exponent forms with integer, trailing-dot or fractional mantissas and an
  optional exponent sign form one numeric source operand. Their displayed
  constant spelling uses uppercase `E`; arithmetic and PARSE VALUE consume
  the same value, and whole-number DO/FOR checks retain contextual behavior.
  Verify Regina and a failing baseline, raw/canonical tree, opt/no-opt,
  focused count errors, the normal Level C suite and linked execution.
  Digit-starting nonnumeric constants are covered by `LC-AC-44`;
  leading-period symbols remain open under `LC-AC-04/08`.
- [x] **LC-AC-44 — digit-starting constant symbols:** nonnumeric Classic
  constants beginning with a digit, including letter and repeated-period
  forms, remain one source `CONST_SYMBOL` operand. Their value is the uppercase
  source spelling, independent of the variable pool; expressions, PARSE VALUE
  and adjacent concatenation preserve it. Numeric constants retain the
  `INTEGER`/`DECIMAL` path, and leading-period symbols remain separate work.
  Verify Regina, raw/canonical tree, opt/no-opt, parser/highlighter checks,
  the normal Level C suite and linked execution.
- [x] **LC-AC-45 — two-target implicit-word PARSE:** `PARSE VAR scalar
  first rest` and `PARSE VALUE expression WITH first rest`, with optional
  UPPER and direct scalar targets, capture the source once before any pool
  writes. The first target receives the first blank-delimited word; the
  second receives the remaining source after exactly one separator, retaining
  further blanks. Source aliases and repeated targets follow authored write
  order. Verify Regina across leading/repeated/trailing blanks, short input,
  aliasing, local scope, shared value method, opt/no-opt, raw/canonical tree,
  normal Level C suite and linked execution. Other template shapes remain
  guarded; broader binary/UTF8 proof remains under `LC-AC-04/06`.
- [x] **LC-AC-46 — three words with final PARSE drop:** `PARSE VAR scalar
  first second third .` and the corresponding `PARSE VALUE expression WITH`
  form, with optional UPPER, use the existing `parsewords3d` primitive through
  shared `RexxValue`. The three scalar targets receive the first three words
  while the final dot discards the remaining template value; no dot binding
  reaches the pool. Source capture precedes ordered writes. Verify Regina
  across whitespace, short input, source aliases, repeated targets, local
  scope, shared method opt/no-opt, raw/canonical tree, normal Level C suite
  and linked execution. Other dot placements were carried to `LC-AC-47`;
  wider templates remain guarded.
- [x] **LC-AC-47 — bounded PARSE dot placeholders:** two- and three-item
  `PARSE VAR`/`PARSE VALUE` templates containing one or more `.` placeholders
  and at least one direct scalar target use the already shared word/tail split
  methods. Each dot consumes its corresponding word or tail without a pool
  write; scalar targets retain authored write order. The source is captured
  once before any writes, with optional UPPER through shared TRANSLATE.
  Verify Regina for leading, internal, and final dots, whitespace, short
  input, aliases/repeated targets, one-time VALUE evaluation, local scope,
  opt/no-opt, raw/canonical AST, normal Level C and linked execution. Four-item
  templates with internal dots were carried to `LC-AC-49`; other template
  shapes remain guarded.
- [x] **LC-AC-48 — all-dot direct PARSE templates:** one-, two- and
  three-item `PARSE VAR`/`PARSE VALUE` templates consisting only of `.`
  placeholders consume the source without any visible pool assignment.
  A VALUE source expression executes exactly once, including when its result
  is otherwise unused; optional UPPER and local procedure scope preserve the
  same source evaluation boundary. Verify Regina side effects and unchanged
  pool values, opt/no-opt, raw/canonical AST with no dot pool binding, focused
  negatives, normal Level C and linked execution. Four-item all-dot forms
  were carried to `LC-AC-49`; wider templates remain guarded.
- [x] **LC-AC-49 — four-item direct PARSE templates:** `PARSE VAR` and
  `PARSE VALUE` with four direct scalar/dot items consume the first three
  words and assign the remaining tail to a fourth scalar, or discard it when
  the fourth item is a dot. Dot placeholders at earlier positions suppress
  only their corresponding pool writes. A shared `RexxValue` method composes
  existing `parsewords3` splitting for a fourth scalar; the established
  `parsewords3d` method handles a final dot. Preserve source capture before
  ordered writes and optional UPPER. Verify Regina whitespace, short input,
  aliases/repeated targets, once-evaluated VALUE, local scope, shared method
  opt/no-opt, raw/canonical tree, normal Level C, RexxScript and linked
  execution. Five-item and pattern templates remain guarded.
- [x] **LC-AC-50 — generic direct word/dot PARSE templates:** `PARSE VAR`
  and `PARSE VALUE` accept a nonempty template of any representable number of
  direct scalar or `.` items, with optional UPPER. A shared `RexxValue`
  operation returns one result per item: the first `n-1` results are words
  and the final result is the unparsed tail; dots suppress only their pool
  writes. The source and result vector are captured before scalar writes,
  preserving aliases, repeated targets, local scope and once-only VALUE
  effects. The compiler has no target-count dispatch or arbitrary four-item
  guard. Verify Regina at counts 1-4 and beyond four, whitespace, short
  input, dot positions, opt/no-opt, shared runtime and RexxScript behavior,
  raw/canonical AST, the normal Level C suite and linked execution. Patterns,
  positions, comma templates and other source forms remain open under their
  own criteria.
- [x] **LC-AC-51 — static mixed PARSE template plan:** `PARSE VAR` and
  `PARSE VALUE` accept a nonempty single template with any representable
  sequence of direct scalar/dot targets, literal patterns, and literal
  absolute/relative positions. The lowerer validates the parsed AST and
  serializes its items to the VM's frozen `parseplan` descriptor; no template
  length or item-order dispatch governs acceptance. One source snapshot and
  one plan execution precede ordered visible pool writes. Dot targets
  consume fields without writes; aliases, repeated targets, UPPER, local
  scope and once-only VALUE effects retain their established behavior.
  Direct word-only templates may keep their shared generic fast path.
  Verify literal/position interaction, leading/trailing/repeated delimiters,
  absent patterns, backward/zero/out-of-range positions, ASCII/UTF8 boundaries,
  opt/no-opt, raw/canonical AST, Regina equivalence, the normal Level C suite
  and linked execution. Dynamic pattern/position operands, comma templates
  and other source forms remain open for subsequent steps.
- [x] **LC-AC-52 — backward PARSE plan cursor repair:** a frozen `parseplan`
  relative negative control leaves the next target at the moved cursor after
  storing the preceding field, matching Regina and preserving the existing
  Level B exit and Level C AST paths. Verify the minimal `3 a +2 b -1 c`
  counterexample in both products, focused opt/no-opt VM and exit tests,
  unchanged forward/absolute controls, the relevant normal correctness
  suite, and linked execution. This repairs `LC-FIND-06`.
- [x] **LC-AC-53 — direct stem and compound DROP:** a direct Classic `DROP`
  list accepts scalar, stem and supported compound symbols in authored order.
  Compound tail components are evaluated once against the visible pool,
  preserving substituted text case, before the selected tail is dropped; a
  stem drop removes its default and tails, while an individual compound drop
  reads as its expanded symbol even when the stem has a default. Exposed stems
  affect their parent binding and local unexposed stems remain local. The
  shared stem and pool classes own dropped-tail and default lifecycle, while
  the compiler emits canonical pool calls in source order. Verify Regina for
  tail substitution, dropped-tail reads, stem reset, reassignments, list order
  and scope; focused pool tests, opt/no-opt, raw/canonical AST, normal Level C
  and RexxScript regressions, and linked execution.
  Parenthesized indirect `DROP` was subsequently completed under `LC-AC-56`.
- [x] **LC-AC-54 — shared Classic stem lifecycle:** assigning a stem default
  replaces prior explicit and dropped tails; dropping one tail records a
  tombstone that reads as the expanded symbol until reassigned or reset by a
  stem assignment. Tail substitution preserves the substituted value's case
  for lookup and unresolved display. The shared `RexxStem` and
  `RexxVariablePool` implement these rules for both Level C and RexxScript.
  Verify minimal Regina counterexamples, focused runtime tests, optimized and
  no-opt execution, relevant cross-consumer regressions and linked delivery.
- [x] **LC-AC-55 — shared one-symbol DROP operation:** for a validated Classic
  variable symbol, the shared pool resolves a compound tail once against the
  current visible bindings, then drops the resulting scalar, entire stem, or
  one case-preserved tail. It preserves exposure and dropped-tail semantics.
  Verify Regina examples, focused optimized/no-opt pool regressions, linked
  execution and unchanged Level C/RexxScript consumers.
- [x] **LC-AC-56 — indirect DROP subsidiary lists:** a parenthesized `DROP`
  reference reads its variable at that point in the authored list, splits its
  value into subsidiary variable names and drops those names from left to
  right through the shared pool, including scalar, stem and compound items.
  Invalid words are ignored while later valid words still execute, following
  Regina as Adrian selected on 2026-10-03. Direct items before and after the
  reference retain their order. Verify source/canonical AST shape, Regina for
  valid and invalid lists and scope, focused opt/no-opt, normal correctness
  and linked execution.
- [ ] **LC-AC-57 — complete SAY instruction:** `SAY [expression]` evaluates an
  expression once when supplied and writes its string value, or an empty line
  when omitted, through the configured default output. Main, nested and local
  procedure contexts preserve output order, source anchors and relevant
  output/error lifecycle. Verify the grammar's expression and childless forms,
  Regina output, side-effect order, opt/no-opt, AST, normal correctness,
  linked delivery and configured host output. The already tested childless
  fixture is one case in this instruction contract, not a separate slice.
- [ ] **LC-AC-58 — simpler Level C implementation:** the active lowering has
  one context-aware instruction dispatch, one reviewed variable-operation
  ownership boundary, and one general PARSE execution path. DO header
  validation and lowering share a checked representation; a dedicated AST
  node is used only if its total validation/emitter cost is lower than the
  canonical rewrite. Preserve evaluation order, source anchors, scopes,
  loop associations, errors, opt/no-opt behavior and RexxScript isolation.
  Verify a before/after code-path inventory, focused structural invariants,
  retained functional corpus, linked execution and cross-consumer tests.
  Specific architecture changes require Adrian's approval before code edits.
- [ ] **LC-AC-59 — instruction-level delivery:** each `LC-I-*` row in the
  instruction programme below is individually closed only after its complete
  reference and parser-form matrix, diagnostics, interaction/lifecycle cases,
  relevant BYTE/UTF8 and host evidence, optimized/no-opt parity, and full
  toolchain proof are recorded, or a specific exception is approved. Preserve
  each existing bounded test as regression evidence. Verify against the
  compliance reference, parser grammar, reference-obligation appendix,
  tests and exact candidate revision.
- [x] **LC-AC-07 — scalar pool read and DROP slice:** an uninitialized or
  dropped scalar reads as its uppercase Classic symbol, direct scalar `DROP`
  affects the current visible pool (including a procedure's exposed alias),
  and reassignment restores its value. This initial scalar slice preceded
  direct stem/compound support under `LC-AC-53`; indirect `DROP` remains open.
  Verify against the Classic
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
26. **LC-STEP-26 — complete (LC-AC-08/23; depends on STEP-25):** recognize only the
    source REPEAT assignment plus literal TO shape for a scalar pool variable.
    Lower the initial pool assignment before one canonical REPEAT; use its
    WHILE entry check for the current pool value against TO and an UNTIL
    end-check BLOCK_EXPR to advance the pool value and return false. Extend
    the neutral loop builder only as needed for both checks. Use contextual
    RexxValue numeric validation and the existing hidden source-loop binding.
    Prove body mutation, zero-entry, final values, ITERATE/LEAVE and nested
    ownership against Regina. Retain fail-closed guards on other controlled
    shapes, qualify and commit separately.
27. **LC-STEP-27 — complete (LC-AC-08/24; depends on STEP-26):** extend the controlled
    clause crosswalk to one literal signed BY and one literal TO in either
    parser order. Reuse the pool-backed canonical WHILE/UNTIL loop and add
    contextual RexxValue step comparison/advance methods while preserving
    the default BY-one behavior. Prove negative, positive and guarded zero
    steps, mutation, final values and ITERATE/LEAVE against Regina; keep
    dynamic BY/FOR and wider headers fail-closed, qualify and commit.
28. **LC-STEP-28 — complete (LC-AC-08/25; depends on STEP-27):** recognize one bounded
    literal FOR alongside the existing literal TO and optional literal BY,
    in any parsed modifier order. Feed its count to the canonical REPEAT FOR
    child while retaining the pool-backed WHILE entry and UNTIL end step.
    Prove zero and positive counts, direction, final-value and transfer
    timing against Regina. Keep dynamic FOR and FOR-without-TO fail-closed,
    qualify and commit separately.
29. **LC-STEP-29 — complete (LC-AC-08/26; depends on STEP-28):** admit the existing
    bounded controlled FOR header without TO, optionally with signed literal
    BY. Generalize the controlled clause crosswalk and omit the canonical
    WHILE child when TO is absent, retaining FOR and the pool-advancing UNTIL
    end block. Prove zero count, final values, mutation, ITERATE/LEAVE and
    nested scope against Regina, then qualify and commit separately.
30. **LC-STEP-30 — complete (LC-AC-08/27; depends on STEP-29):** map a named transfer's
    source variable to a supported active controlled DO during validation and
    to that DO's hidden canonical binding during lowering. Reuse existing
    LEAVE/ITERATE nodes through the emitter, preserving source anchors and
    transfer timing across nested/generated blocks. Keep the front-end
    `28.3`/`28.4` invalid-name checks; prove outer and inner targets against
    Regina, qualify and commit separately.
31. **LC-STEP-31 — complete (LC-AC-08/28; depends on STEP-30):** admit a supported
    controlled WHILE header and combine its entry condition with the existing
    TO guard lazily under one canonical WHILE, preserving FOR-before-TO-before-
    WHILE timing. Reuse the canonical end-step block and source binding,
    including setup-bearing conditions and exact `34.3`. Keep UNTIL and
    dynamic controlled clauses fail-closed, prove Regina/tree/toolchain and
    normal/Debug checks, then commit separately.
32. **LC-STEP-32 — complete (LC-AC-08/29; depends on STEP-31):** admit a supported
    controlled UNTIL header by combining its condition and the existing pool
    advance under the canonical end-check BLOCK_EXPR. Check the condition
    first, return true without stepping, or step and return false; retain
    FOR/TO entry guards and hidden source-loop bindings. Prove ITERATE,
    LEAVE, final count timing, setup and `34.4` with Regina and normal/Debug
    checks, then commit separately.
33. **LC-STEP-33 — complete (LC-AC-08/30; depends on STEP-32):** allow the parser's
    controlled assignment header without TO/FOR, with optional literal BY
    and supported WHILE/UNTIL. Reuse the pool-backed canonical REPEAT and
    end step, omitting FOR and any synthetic entry guard. Keep guarded
    reference fixtures and bounded command execution; prove transfer,
    condition, scope, tree, toolchain and normal/Debug behavior, then commit.
34. **LC-STEP-34 — complete (LC-AC-08/31; depends on STEP-33):** accept a supported
    dynamic TO expression, lower its setup once before the initial pool set,
    validate its numeric value there, copy the resulting RexxValue into a
    hidden canonical local, and use that
    local in the existing WHILE TO check. Keep the literal TO fast path and
    dynamic start/BY/FOR guards. Prove Regina timing, mutation, `41.4`,
    source-anchored ownership and normal/Debug/toolchain behavior, then
    commit separately.
35. **LC-STEP-35 — complete (LC-AC-08/32; depends on STEP-34):** admit supported
    dynamic BY expressions, lower dynamic TO/BY clause setup in parsed
    source order before the initial pool set, validate and copy each value
    into a hidden canonical local, and reuse the existing TO comparison and
    end-step methods. Prove signed/zero steps, source mutation, FOR-zero
    `41.5`, nesting and condition timing against Regina, then qualify and
    commit separately.
36. **LC-STEP-36 — complete (LC-AC-08/33; depends on STEP-35):** admit a supported
    dynamic controlled FOR expression, evaluate and validate it once in the
    parsed TO/BY/FOR order before the initial pool set, and feed its hidden
    canonical int to the existing REPEAT FOR child. Add shared RexxValue
    contextual `26.3` conversion; prove zero/positive count, mutation,
    transfers, conditions, source anchors, linked execution and normal/Debug
    checks, then commit separately.
37. **LC-STEP-37 — complete (LC-AC-34; depends on STEP-35, blocks STEP-36):** retain a
    minimal existing-syntax reproducer for the call-window affinity alias
    exposed by captured BY across a loop back edge. Use a disjoint call window
    for canonical UNTIL end-check calls while retaining normal affinity for
    other call sites; prove the counterexample and the existing WHILE path in
    opt/no-opt runtime and emitted marshalling, then run the
    relevant normal/Debug checks and commit the independent repair.
38. **LC-STEP-38 — complete (LC-AC-08/35; depends on STEP-36):** capture a supported
    dynamic start into a hidden canonical `RexxValue` before walking the
    remaining header clauses. Keep their existing written-order validation,
    then validate start with shared contextual `41.6` immediately before the
    pool assignment. Feed that checked value into the existing canonical
    pool-set and controlled-loop builders; add no AST node unless those
    builders cannot express the required ownership or timing. Prove reference
    parity, error precedence, tree shape, opt/no-opt and linked behavior,
    qualify the relevant normal suites, and commit this increment separately.
39. **LC-STEP-39 — complete (LC-AC-08/36; depends on STEP-38):** preserve the parser's
    zero-length `STRING` payload when creating the shared `RexxValue` literal,
    without relaxing missing-payload checks on other node types. Retain the
    existing canonical string factory and emitter path; inspect the source
    anchor and opt/no-opt value flow, prove Regina and contextual numeric
    errors, run the relevant normal checks, and commit separately.
40. **LC-STEP-40 — complete (LC-AC-08/37; depends on STEP-39):** admit an adjacent
    function-call term in the Level C continuation expression grammar, using
    source token positions to preserve the spaced symbol-plus-group meaning.
    Build the existing `FUNCTION` AST node and canonical BIF/local-call path;
    add no new emitter node. Reproduce both spellings against Regina, inspect
    raw and lowered trees, qualify focused and normal checks, and commit this
    parser/AST increment separately.
41. **LC-STEP-41 — complete (LC-AC-06/08/38; depends on STEP-40):** route each supported
    ARG binding's captured `RexxValue` through the existing shared Classic
    TRANSLATE BIF context before the canonical pool set. Keep caller argument
    copies and target order intact; reuse the existing argument-frame builder
    and source anchors. Prove Regina and shared BYTE-default behavior,
    opt/no-opt and linked execution, qualify normal checks, and commit this
    binding increment separately.
42. **LC-STEP-42 — complete (LC-AC-08/39; depends on STEP-41):** validate the parser's
    `PARSE` option, source and nested template-list ownership for direct
    single-target `VAR` and `VALUE` forms. Lower source evaluation and optional
    shared TRANSLATE into canonical pool assignment with source anchors;
    reject all other template shapes. Prove Regina, opt/no-opt, nested
    main/procedure, target-tree and linked behavior, then commit separately.
43. **LC-STEP-43 — complete (LC-AC-06/08/40; depends on STEP-42):** wrap the existing
    `parsewords3` opcode in a shared `RexxValue` result method, retaining its
    exact two-word-plus-tail contract. Extend the Level C PARSE tree guard to
    only three direct scalar targets, capture the source once, call the shared
    method and write each result through the visible pool in authored order.
    Reuse shared TRANSLATE for optional UPPER. Prove reference, shared runtime,
    canonical AST, opt/no-opt and linked behavior before the separate commit.
44. **LC-STEP-44 — complete (LC-AC-08/41; depends on STEP-43):** infer the missing
    concatenation operator from the physical token span between the last
    represented token of the left AST and the first represented token of the
    right AST. Include intervening parentheses and call punctuation in the
    touch check, and apply it to the three Level C implicit-concatenation
    grammar paths. Preserve the explicit operator and spaced cases, prove
    Regina and canonical runtime equivalence, then commit this parser/AST
    correction separately.
45. **LC-STEP-45 — complete (LC-AC-08/42; depends on STEP-44):** scan bounded decimal
    fraction forms as one existing numeric token and represent them with the
    existing source `DECIMAL` AST node. Extend the Level C parser's operand
    contexts and lower that node through the same shared `RexxValue` string
    path as other Classic constants. Retain integer-only fast paths for DO/FOR
    counts and prove contextual errors, tree shape and linked equivalence
    before a separate commit.
46. **LC-STEP-46 — complete (LC-AC-08/43; depends on STEP-45):** extend the numeric
    scanner pattern to exponent forms with required exponent digits, emit the
    existing `DECIMAL` source AST node, and normalize only the exponent letter
    to Classic uppercase when constructing the shared `RexxValue` string.
    Check direct expressions, arithmetic, PARSE VALUE, count validation,
    canonical lowering and linked output before a separate commit.
47. **LC-STEP-47 — complete (LC-AC-08/44; depends on STEP-46):** scan digit-starting
    constant symbols after complete numeric forms, map them to the existing
    `CONST_SYMBOL` AST node and lowercase-to-uppercase Classic literal path,
    and retain numeric and invalid-assignment classification. Prove the raw
    symbol and canonical shared-value tree, Regina parity, parser/highlighter
    behavior and linked output before a separate commit.
48. **LC-STEP-48 — complete (LC-AC-06/08/45; depends on STEP-47):** derive the first
    word using the existing shared `parsewords3` VM wrapper, then retain the
    exact two-target remainder from the captured source in a shared
    `RexxValue.parseWordAndRest()` method. Extend the validated nested PARSE AST
    slice to two targets, capture the returned pair before ordered pool
    writes, and keep optional shared TRANSLATE before splitting. Prove
    reference behavior, shared method tests, canonical lowering and linked
    execution before the separate commit.
49. **LC-STEP-49 — complete (LC-AC-06/08/46; depends on STEP-48):** wrap the existing
    `parsewords3d` VM instruction in a shared `RexxValue` method, validate
    exactly three direct scalar targets followed by a final dot in the
    nested Level C template AST, and capture all three outputs before pool
    writes. Keep optional shared TRANSLATE and fail closed for other dot
    placements. Prove Regina, shared method, canonical AST, opt/no-opt and
    linked behavior before a separate commit.
50. **LC-STEP-50 — complete (LC-AC-08/47; depends on STEP-49):** validate `.` as a
    placeholder in two- and three-item nested PARSE templates while retaining
    the existing final-dot four-item slice. Reuse the shared two-/three-result
    split methods, suppress pool writes for dots, and preserve capture before
    scalar writes. Compare reference behavior and source-anchored raw/canonical
    trees, run opt/no-opt, normal Level C, and linked checks, then commit
    separately.
51. **LC-STEP-51 — complete (LC-AC-08/48; depends on STEP-50):** allow all-dot direct
    templates of one to three items, with a hidden canonical capture for the
    single-dot VALUE source and the existing shared split captures for two or
    three dots. Emit no pool writes, retain source anchors, and prove
    side-effect, scope, optimizer and linked parity before a separate commit.
52. **LC-STEP-52 — complete (LC-AC-06/08/49; depends on STEP-51):** compose the shared
    `parsewords3`/`parseWordAndRest()` operations into a four-result
    `RexxValue` method, admit four direct scalar/dot items in the nested PARSE
    AST, select the existing final-dot method when appropriate, and skip only
    dot pool writes. Replace newly obsolete four-item negative fixtures with
    a five-item guard, then prove reference, shared-consumer, structural,
    opt/no-opt, normal Level C and linked behavior before a separate commit.
53. **LC-STEP-53 — complete (LC-AC-06/08/50; depends on STEP-52):** replace the
    target-count dispatch with one shared word/dot result-vector operation
    that chains the certified `parsewords3` primitive through
    `parseWordAndRest()`. Validate an arbitrary nonempty direct target list,
    pass its count once, capture all results, and write non-dot targets in
    order. Replace obsolete five-item negatives with a distinct unsupported
    template family. Prove reference, shared-consumer, structural, opt/no-opt,
    normal Level C and linked behavior before a separate commit.
54. **LC-STEP-54 — complete (LC-AC-08/51; depends on STEP-53):** map the parser's mixed
    template AST to a validated, count-independent item sequence and compile
    static items into the frozen `parseplan` descriptor. Reuse the existing
    canonical assembler node and string-array result shape where possible;
    add a dedicated node only if the existing emitter boundary cannot
    represent the descriptor safely. Capture source and results before pool
    writes, compare with Regina for pattern/position interaction and Unicode,
    inspect raw/canonical ownership, run focused Debug/Release and the normal
    Level C suite, and retain linked-image evidence before committing.
55. **LC-STEP-55 — complete (LC-AC-52; discovered during STEP-54):** retain the
    Level B exit and Regina counterexample, repair the frozen VM plan's
    backward cursor without changing the descriptor format, prove focused
    opt/no-opt and normal correctness, then commit the shared runtime cause
    separately from Level C mixed-template lowering.
56. **LC-STEP-56 — complete (LC-AC-06/53/54; depends on STEP-09):** repair shared stem
    assignment/reset and dropped-tail state, preserve substituted tail case,
    and add pool operations for non-mutating compound reads and one-tail drops.
    Prove the discovered reference counterexamples and focused cross-consumer
    behavior, then commit this distinct runtime cause.
57. **LC-STEP-57 — complete (LC-AC-08/53; depends on STEP-56):** broaden the Level C
    direct-list guard and lowerer to call shared scalar/stem/compound pool
    operations in source order, and replace obsolete negative fixtures with
    Regina-based focused and linked proof. Retain parenthesized indirect lists
    as a separately guarded shape; run the normal Level C and RexxScript
    checks before committing.
58. **LC-STEP-58 — complete (LC-AC-06/55; depends on STEP-56):** implement one validated
    Classic symbol drop in `RexxVariablePool` using its existing name resolver
    and scalar/stem/tail operations. Prove substitution and exposure with
    focused pool tests, optimized/no-opt, linked execution and cross-consumer
    checks, then commit the runtime primitive separately.
59. **LC-STEP-59 — complete (LC-AC-08/56; depends on STEP-57/58):** add shared subsidiary-list interpretation and lower the
    parser's `VAR_REFERENCE` node to a canonical helper call in source order.
    Ignore invalid subsidiary-list words as approved; verify that behavior,
    reference behavior, raw/canonical trees,
    opt/no-opt, normal Level C and linked execution before committing.
60. **LC-STEP-60 — checkpoint complete (LC-AC-08/57; folded into STEP-63):**
    incorporate childless `SAY` into an instruction-level fixture covering
    expression side effects, output order, nested execution and local routines.
    Regina, opt/no-opt, tree, normal Level C and linked evidence are recorded
    under the active review below. The full `SAY` contract remains open under
    STEP-63; this checkpoint is not a feature-completion claim.

61. **LC-STEP-61 — review complete (LC-AC-58/59):** inspect the active parser,
    lowerer, remap builders, shared pool/value/BIFs, VM PARSE path, tests and
    coverage records. Record specific duplication and safe foundations in the
    architecture review; retain all prior criteria and evidence.
62. **LC-STEP-62 (LC-AC-58/59; depends on STEP-61):** apply the same strict
    review gate to each instruction before implementation: map all parsed
    forms and reference obligations, identify duplicate paths and missing
    semantics, select the simplest implementation, and identify decisive
    existing or missing tests. Obtain approval before a particular language
    or architecture decision. Refactor only the active instruction and any
    required shared foundation in reviewable commits. Measure any new
    aggregate CTest in isolated Debug and sanitizer builds before registering
    it. Close the instruction with an evidence receipt. A fundamental conflict
    requiring a serious compromise remains open while Adrian decides its
    individually proposed disposition; ordinary implementation effort never
    qualifies for that path.

### Whole-instruction programme

The following rows are the future semantic delivery units and implementation
steps for `LC-AC-59`. All are **open** until their full instruction contract
is evidenced; current bounded behavior is retained in the coverage matrix.
Work through the rows in order, with only one active row; changing the order
requires a recorded reason and must not turn a partial row into a closure.
Each
step includes parser-form inventory, reference cases, invalid forms,
main/procedure/nested execution where legal, opt/no-opt, source/AST checks,
relevant runtime and host/profile checks, and linked delivery. The relevant
normal correctness suite runs once per coherent instruction checkpoint, with
focused checks during its development. A structural change affecting all
instructions calls for the full Level C suite at that checkpoint. Reuse
unchanged evidence and leave overnight assurance to its scheduled lanes.

| Unit | Step | Complete instruction obligation and principal dependency |
| --- | --- | --- |
| LC-I-01 SAY | LC-STEP-63 | `LC-AC-57`: expression and childless forms, output and lifecycle; include pending STEP-60 case. This is the first active review. |
| LC-I-02 DROP | LC-STEP-64 | Direct and parenthesized lists, substitution, exposure, order and errors; shared pool boundary. Preserve Adrian's approved Regina invalid-word behavior. |
| LC-I-03 assignment | LC-STEP-65 | All valid scalar, stem and compound targets and expression/evaluation order; shared pool boundary. |
| LC-I-04 NOP | LC-STEP-66 | Childless instruction in every legal context and clause/trace lifecycle; existing bounded proof is the baseline. |
| LC-I-05 OPTIONS | LC-STEP-67 | All processor option words, unknown-option policy and source/configuration behavior; programme initialization. |
| LC-I-06 IF | LC-STEP-68 | Every legal instruction arm, nearest ELSE, condition/error and nesting behavior; shared statement dispatch. |
| LC-I-07 SELECT | LC-STEP-69 | WHEN/OTHERWISE forms, arm instructions, evaluation and no-match/error lifecycle; IF and statement dispatch. |
| LC-I-08 DO | LC-STEP-70 | Simple, counted, controlled, FOREVER, WHILE/UNTIL and legal combinations without arbitrary count limit; reviewed loop representation. |
| LC-I-09 LEAVE | LC-STEP-71 | Unnamed/named targets, nesting, value/state and errors across all legal loops; DO. |
| LC-I-10 ITERATE | LC-STEP-72 | Unnamed/named targets, end-step timing, nesting and errors across all legal loops; DO. |
| LC-I-11 ARG | LC-STEP-73 | Direct argument acquisition and Classic upper/template behavior, omitted positions and invocation modes; routine context and shared parse semantics. |
| LC-I-12 PROCEDURE | LC-STEP-74 | Pool creation and all direct/indirect EXPOSE forms with alias lifecycle; shared variable pool and routines. |
| LC-I-13 CALL | LC-STEP-75 | Internal/BIF/external resolution, arguments, results and ON/OFF traps; BIF registry, routines and conditions. |
| LC-I-14 RETURN | LC-STEP-76 | Value/void, function/subroutine/outermost rules and pool/result lifecycle; CALL and PROCEDURE. |
| LC-I-15 EXIT | LC-STEP-77 | Optional value, fallthrough equivalence, completion and finalization; programme lifecycle. |
| LC-I-16 PULL | LC-STEP-78 | Queue/default input, optional template and empty/error behavior; host queue and PARSE source service. |
| LC-I-17 PUSH | LC-STEP-79 | Front-of-queue ordering, optional expression and null value; configured queue. |
| LC-I-18 QUEUE | LC-STEP-80 | Back-of-queue ordering, optional expression and null value; configured queue. |
| LC-I-19 PARSE | LC-STEP-81 | Every source type, UPPER, arbitrary target/template sequence, static/dynamic patterns and positions, comma templates and errors; one reviewed parse engine plus argument/input/source services. |
| LC-I-20 ADDRESS | LC-STEP-82 | Select/swap/transient command and WITH redirection, RC and conditions; configured environment protocol. |
| LC-I-21 implicit command | LC-STEP-83 | Command clause evaluation, host execution, results and conditions; ADDRESS. |
| LC-I-22 NUMERIC | LC-STEP-84 | DIGITS/FORM/FUZZ defaults, validation, context lifetime and arithmetic effects; shared value semantics. |
| LC-I-23 SIGNAL | LC-STEP-85 | Direct/VALUE branch and ON/OFF conditions, labels, loop-state clearing and delivery; condition lifecycle. |
| LC-I-24 TRACE | LC-STEP-86 | Options, skip/inhibit, interactive and source/result/command tracing; clause hooks and host output. |
| LC-I-25 INTERPRET | LC-STEP-87 | Dynamic source parsing, current context, HALT/SYNTAX/label rules and condition state; parser and lifecycle foundation. |

Expression grammar, BIFs, variable semantics, source/character profiles,
conditions and host adapters are cross-cutting foundations under
`LC-AC-01/04/06/08`; they are not silently completed by an instruction row.
Before closing any `LC-I-*`, reconcile its reference-obligation rows and the
parser's accepted forms. An unresolved dependency keeps the row open.

### Active review: LC-I-01 SAY

`SAY` is the only active instruction. The parser has an expression child or
no child (`compiler/rxcpcgmr.y`), plus recovery for an invalid close bracket.
The lowerer already uses one canonical `SAY` builder; the pending no-child
change supplies an empty string to that builder. The emitter uses the normal
`SAY` opcode, and the VM routes output through its SAY exit callback. No new
AST node or instruction-specific runtime helper is indicated by this review.

| SAY contract area | Current evidence | Closure state |
| --- | --- | --- |
| Childless and ordinary expressions in main, IF/DO and local procedures | `levelc_say_instruction.rexx`; Regina, opt/no-opt, linked output and source-anchored tree checks | Proved for these forms; complete expression domain remains open |
| Expression evaluation once and output order | Exposed counter called inside SAY expression; its inner SAY precedes the outer line | Proved for this fixture; external/BIF effects remain open |
| Invalid source forms | Grammar has explicit close-bracket recovery and general expression diagnostics | Open: reference/error matrix and source anchors |
| Complete BIF, local and external function terms | Current guard admits only LENGTH, SUBSTR and bounded local calls; direct BIF inventory and host lookup are broader | Open: shared invocation, context, error and resolution foundation |
| General variable value terms | Compiler read guard rejects bare stems and multi-component compound tails; shared pool `symbolValue` already resolves both | Open: pool-read consolidation and reference proof |
| BYTE/UTF8 output, configured host route and failure | Embedded-NUL and BIF-error probes below; legacy SAY callback is terminated text | Open: output API decision, implementation and host/profile proof |
| Trace/condition lifecycle | Existing canonical SAY opcode and source anchors | Open: clause hooks, trapped errors and finalization |

Open before closure:

- Reconcile every expression form admitted by the Classic grammar with the
  shared expression/BIF path. The current Level C guard admits only bounded
  variable, literal, operator, local-call, LENGTH and SUBSTR forms; a valid
  expression must not fail solely because it appears in `SAY`.
- **Confirmed variable-read limit, pending ownership approval:** Regina writes
  `Q.x.y` and `Q.` for `a='x'; b='y'; say q.a.b; say q.`, while the compiler
  rejects both SAY operand shapes as unsupported
  (`/tmp/crexx-levelc-say-variable.0er3lh/` and isolated probes under
  `/tmp/crexx-levelc-say-variable-isolate.16UXVW/`). The existing shared
  `RexxVariablePool.symbolValue` resolves multi-component compound names and
  bare stems without creating bindings. Proposed consolidation routes all
  Level C variable *reads* through this method and removes the one-component
  guard. Assignment keeps its separate pre-RHS tail capture until its own
  instruction review. This is implementable; approval for the ownership
  change is pending under `AGENTS.md`.
- **Confirmed call-error defect:** `options levelc; say substr('abc', 0);
  say 'after'` stops in Regina with `40.14` and no output, but the current
  Release compiler/VM prints an empty line and `after` with a success exit
  (`/tmp/crexx-levelc-say-bif-error.AUp7Pt/`). The shared BIF context records
  an error, while the current compiler call path consumes only its blank
  return value. Complete one shared call-result/condition bridge before
  widening BIF reachability; preserve source and argument positions, current
  pool/configuration and RexxScript isolation. This is required work, not an
  infeasible exception. A direct-entry compiler table plus one shared
  call-result check that delivers Classic `SYNTAX` separately from Classic
  command `ERROR` is proposed. Approval is pending under `AGENTS.md`; no
  architecture edit has begun.
- Prove configured default-output selection, errors, BYTE/UTF8 behavior and
  source/trace lifecycle through the host interface. Inspect the existing VM
  SAY callback boundary for byte-exact output before claiming this proof.
- **Confirmed output defect, open architecture decision:** a BYTE profile
  probe `options levelc; say '410042'x` produced bytes `41 00 42 0a` with
  Regina but `41 0a` with the current Release `rxc`/`rxas`/`rxvm` path
  (`/tmp/crexx-levelc-say-nul.sdYMGN/`). `SAY_REG` and `SAY_STRING` have
  explicit byte lengths, but `rxvm_mprintf` formats to a C string and the
  default/custom `say_exit_func(char *)` path uses a terminator. This is
  implementable, not an infeasible feature. Proposed repair: keep the old
  callback ABI for existing hosts, add a length-aware per-context SAY callback
  and an internal byte-span output path, and require an explicit error rather
  than silent truncation when a legacy custom callback encounters embedded
  NUL. The default console path can use its existing length-taking writer.
  Approval for this host API change is pending under `AGENTS.md`; no VM code
  change has begun. Prove both callback routes, BYTE exactness, UTF8/error
  behavior, main/local output ordering and linked execution after approval.
- Keep the childless Regina, opt/no-opt, raw/canonical tree, normal Level C
  and linked results already obtained for unchanged code/test inputs. Add
  only missing decisive cases for expression side effects, output routing and
  diagnostics. Record exact evidence in the instruction receipt.

These are open implementation/proof items, not infeasible exceptions. Do not
start `LC-I-02 DROP` until `LC-I-01` closes or Adrian explicitly changes the
queue after reviewing a specific blocker.

2026-10-03 STEP-63 checkpoint (SAY remains open): the single
`levelc_say_instruction.rexx` fixture covers expression and childless forms
in main, IF/DO and local procedures, plus an exposed counter mutated by a
function called in a SAY expression. Regina output matches Release optimized,
no-opt and linked RXBIN byte for byte. The focused CTests passed 3/3, and the
normal Release Level C suite passed 290/290 in 27 seconds
(`/tmp/crexx-levelc-say-suite.Wt01LA`). The linked proof retained compile,
assembly, link and VM outputs under
`/tmp/crexx-levelc-say-instruction.vkHmqr`. The tree CTest also checks
source anchors for the main childless, nested, expression and local procedure
SAY forms; the production boundary verifier checks parent/sibling ownership
and residual Level C nodes. It does not prove every association or trace
lifecycle. The embedded-NUL and BIF-error defects, complete function/BIF
reachability, configured host output and failure lifecycle remain open.

## Findings

- **LC-FIND-08 — resolved indirect DROP invalid words:** the
  [IBM DROP reference](https://www.ibm.com/docs/SSGMCP_5.5.0/reference/rexx/drop.html)
  requires valid variable names in a parenthesized subsidiary list. Local
  Regina probes ignored invalid words such as `1bad`, `/` and `(b)` while
  continuing to drop later valid names. Adrian selected the Regina behavior
  on 2026-10-03 for Level C; `LC-AC-56` has a permanent pool and compiled
  regression. Configured UTF8 symbol-classification proof remains open under
  `LC-AC-04`.
- **LC-FIND-07 — resolved shared stem lifecycle:** Regina showed that a new
  stem assignment replaces prior tails, a dropped tail hides even a stem
  default until reassigned, and substituted tail text keeps its case. The
  shared `RexxStem`/`RexxVariablePool` model now records these states and
  writes back a mutated value-class stem through local or exposed bindings.
  Compiled direct stem/compound `DROP` was completed under `LC-AC-53` and
  `LC-STEP-57`.
- **LC-FIND-06 — resolved, frozen PARSE backward cursor:** Regina returns
  `cd|ef|def` for `s='abcdef'; parse var s 3 a +2 b -1 c`, while the
  existing certified Level B exit and VM `parseplan` return `cd|ef|ef`.
  The pending field was stored correctly, but the VM clamped the cursor to
  its capture start after the negative movement. The repair retains the moved
  cursor without a literal anchor while preserving the separate literal-anchor
  rule. Focused Level B regression, the existing literal-anchor reference
  checks, Release and Debug PARSE suites (11/11 each), and the opt/no-opt,
  linked, round-trip and concrete-VM frozen-plan contract pass. Evidence:
  `/tmp/crexx-levelc-mixed-cases-UcSYCM/`,
  `/tmp/crexx-parse-backward-exit-tSscyN/`,
  `/tmp/crexx-parse-backward-normal-release2.ykorOK`,
  `/tmp/crexx-parse-backward-normal-debug.XNPDtE`,
  `/tmp/crexx-parse-backward-contract-release.HYVgQ4`, and
  `/tmp/crexx-parse-backward-contract-debug.kEM5Bb`.
- **LC-FIND-01 — resolved, empty source string literal:** the independent minimal
  `options levelc; say ''` source previously failed during lowering, although the parser
  accepts its STRING node. The parser uses a zero-length payload with an
  absent text pointer; `LC-STEP-39` now lowers that valid shape through the
  canonical empty string and shared `RexxValue`. The initial reproducer is
  `/tmp/crexx-levelc-empty-literal-log.LX2Ksv`; permanent checks and closure
  evidence are recorded under `LC-AC-36` below.
- **LC-FIND-02 — resolved, SAY-adjacent function call parsed as symbols:** the
  existing-syntax `options levelc; say 'x' length('a')` previously printed
  `x LENGTH a` through Level C instead of Regina's `x 1`. `LC-STEP-40`
  now builds the `FUNCTION` node for an adjacent opening parenthesis while
  keeping the blank-separated symbol-plus-group AST. Reproducer output:
  `/tmp/crexx-levelc-adjacent-call-reference.fh07ZP` and
  `/tmp/crexx-levelc-adjacent-call-output.07V6xB`.
- **LC-FIND-03 — resolved, ARG case normalization:** a local procedure with
  `ARG value` called as `echo('z')` returns `ZZ` in Regina when it appends
  `'Z'`, while the former Level C path returned `zZ`. `LC-STEP-41` now uses
  shared TRANSLATE before binding the callee pool target. Reproducer output:
  `/tmp/crexx-levelc-adjacent-call-regina-output.qX60V2` and
  `/tmp/crexx-levelc-adjacent-call-toolchain.17jBQE`.
- **LC-FIND-04 — resolved, abutted expression terms gained spaces:** the independent
  minimal `options levelc; item='a'; say '['item']'` prints `[ a ]`, while
  Regina prints `[a]`. This is the expression/parser adjacency path, separate
  from the three-target PARSE split. Its reproducer is
  `/tmp/crexx-levelc-parse-three-probe.OKSy14/concat.rexx` with paired
  `concat-regina.out` and `concat-crexx.out`. `LC-STEP-44` now selects
  `OP_CONCAT` for physically adjacent terms and `OP_SCONCAT` for spaced terms.
  The new fixture covers groups, calls, comment-only gaps and PARSE VALUE.
- **LC-FIND-05 — digit-start repair complete, leading-period symbols open:** the former
  Level C scanner emitted integer, dot and integer tokens for `1.5`; the
  expression parser could lose the dot. Before `LC-STEP-44`, blank concatenation
  made the existing `DO ... FOR 1.5` invalid-value test pass; unrestricted
  abuttal briefly turned it into `15`. The parser now refuses to infer
  abuttal across an omitted dot, preserving the invalid-value result.
  `LC-STEP-45/46` now scan decimal fraction and valid exponent forms into a
  single numeric source token and lower them through shared `RexxValue`, so
  `1.5` retains its value and still fails the contextual FOR count check.
  `LC-STEP-47` now scans digit-starting nonnumeric constant symbols as one
  source operand. Leading-period symbols and processor-state meanings remain
  open under `LC-AC-04/08`. Reproducer:
  `compiler/tests/rexx_src/levelc_slice35_dynamic_for_fraction.rexx` and
  `/tmp/crexx-levelc-abuttal-fraction-tree.pGPSc7`.

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
| `ASSIGN`, `SAY`, `NOP`, `EXIT`, `RETURN`, `LEVELC_DROP` | Slice: guarded scalar/compound assignment, SAY, childless NOP, bare main EXIT, procedure RETURN, and ordered direct/indirect DROP lists through the Classic pool | Wider statement operands, indirect `PROCEDURE EXPOSE`, exit/return lifecycle and configuration proof remain open |
| `VAR_SYMBOL`/`VAR_TARGET`, strings, integers, expression operators, function calls | Slice: proven scalar/compound pool reads, including empty quoted strings, literal and operator methods, eager Classic `&`/`|`, bounded BIF/local calls including adjacent calls under blank concatenation | More expression shapes, remaining operator order, numeric context and missing-argument behavior remain open |
| `IF` with condition/THEN/ELSE; simple `DO` with `INSTRUCTIONS` | Slice: recursive guards and canonical branch/group builders, including nested forms | More accepted arm statements and source/scope proof as forms expand |
| `SELECT` with `INSTRUCTIONS` of `WHEN` and optional `OTHERWISE` | Slice: guarded list lowers to nested canonical `IF`/one-shot `DO`, including nested arms, local procedures, `34.2` and `7.3` | Broader statement arms, condition lifecycle and profile proof remain open |
| Header-bearing `DO`, `REPEAT`, `FOR`, `WHILE`, `UNTIL`, `BY`, `TO`, `LEAVE`, `ITERATE` | Slice: literal and bounded dynamic direct/combined counts, FOREVER, WHILE/UNTIL including setup-bearing conditions, scalar controlled starts with TO, FOR, both or neither, captured dynamic start/TO/BY/FOR, and controlled WHILE/UNTIL, plus childless and bounded named controlled-loop LEAVE/ITERATE through hidden canonical targets | Wider count values, named transfer to wider loops, other controlled endpoints, numeric errors and wider scope remain open |
| `LABEL`, `LEVELC_PROCEDURE`, `LEVELC_ARG`, `CALL`, `RETURN` | Slice: bounded direct local routines, fixed ARG, scalar/stem EXPOSE and value returns | Wider routine and argument shapes, external resolution, exposure and condition lifecycle |
| `PARSE`, `PULL`, template/pattern/position nodes | Slice: one nonempty static template in `PARSE VAR` or `PARSE VALUE`, with direct scalar/dot targets, literal patterns and positions, and optional UPPER; direct word templates use `RexxValue.parseWordTemplate(count)`, mixed templates use the VM `parseplan` helper, and both paths preserve ordered pool writes | Dynamic pattern/position operands, comma templates, other sources and errors remain open |
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
| Expressions | precedence, arithmetic, comparisons, concatenation, prefix, eager logical `&`/`|` | Slice: `levelc_slice6_expressions`, `levelc_slice19_logical_eager`, empty quoted strings in `levelc_slice37_empty_string`, and adjacent function calls in `levelc_slice38_adjacent_call` | Full numeric context, remaining operator order, boundary/error and platform equivalence |
| Variables | scalar read/write, drop, compound names, bare stems, exposure, API pool | Slice: scalar/compound read/write, scalar/stem EXPOSE, unset scalar read and direct scalar/stem/supported-compound plus parenthesized indirect DROP | Full stem assignment, indirect EXPOSE, configured profile, external/API operations and aliasing |
| Control | IF/THEN/ELSE | Slice: `levelc_slice7_if_else`, nested and procedure fixtures, opt/no-opt, invalid logical and unsupported-arm tests | Other instructions in arms and broader condition/message lifecycle remain open |
| Control | simple DO/END | Slice: `levelc_slice8_do_block` and nested/empty/procedure fixtures, opt/no-opt, tree-shape and linked execution | Broader clause lifecycle and conditions remain open |
| Control | counted/controlled/repetitive DO, WHILE/UNTIL, LEAVE/ITERATE | Slice: literal and bounded dynamic direct/combined counts, FOREVER and WHILE/UNTIL including setup-bearing conditions, scalar controlled DO with optional TO/FOR/BY, captured dynamic start/TO/BY/FOR and WHILE entry or UNTIL end checks, and childless plus bounded named LEAVE/ITERATE across generated IF/SELECT/simple-DO wrappers | Wider count values, named transfer to wider loops, remaining numeric contexts and errors |
| Control | SELECT/WHEN/OTHERWISE | Slice: `levelc_slice11_select`, opt/no-opt and linked execution, exact `34.2`/`7.3` negatives | Wider arms, lifecycle and configuration proof |
| Control | NOP | Slice: `levelc_slice9_nop` in main, local procedure, and IF/DO bodies | Full source/TRACE lifecycle and configuration proof open |
| Routines | labels, local/external CALL and functions, ARG, PROCEDURE EXPOSE, RETURN, EXIT | Slice: bounded local calls, fixed direct ARG with Classic uppercase binding, scalar/stem EXPOSE, RETURN and empty EXIT | Omitted arguments, wider PARSE templates, dynamic/external calls, full scope and return/exit lifecycle |
| PARSE | ARG, PULL, SOURCE, LINEIN, VERSION, VALUE, VAR; templates and UPPER | Slice: `VAR`/`VALUE` with one nonempty static template of direct scalar/dot targets, literal patterns and positions, and optional UPPER in `levelc_slice40_parse_single` through `levelc_slice52_parse_static`; other forms remain front end only | Other source acquisition, dynamic patterns/positions, commas, configuration and errors |
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
| `SYN-CLASSIC-DO` | Simple, counted, conditional, and forever DO | Bounded slices: simple DO, literal and bounded dynamic direct/combined counts, FOREVER and conditional headers, scalar literal start with optional captured TO/BY/FOR | Dynamic start, wider count/numeric errors and configuration proof open |
| `SYN-CLASSIC-DROP` | DROP instruction | Ordered direct scalar/stem/supported-compound and parenthesized indirect lists, including Regina-style invalid-word skip | Wider direct compound forms, full condition/profile proof open |
| `SYN-CLASSIC-EXIT` | EXIT instruction | Bounded slice: empty EXIT | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-IF` | Classic IF/THEN/ELSE | Bounded slice: bounded IF/THEN/ELSE | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-INTERPRET` | INTERPRET instruction | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-ITERATE` | ITERATE instruction | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-LEAVE` | LEAVE instruction | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-NOP` | NOP instruction | Bounded slice: standalone and nested NOP | Full source/TRACE and configuration proof open |
| `SYN-CLASSIC-NUMERIC` | NUMERIC DIGITS/FORM/FUZZ | Front end only | Execution and reference proof open |
| `SYN-CLASSIC-PARSE` | PARSE variants and templates | Nonempty direct scalar/dot `VAR`/`VALUE` templates with optional UPPER execute through one generic result-vector path; parser covers wider templates | Other source/template execution and reference proof open |
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
- A scalar controlled `DO name = integer TO integer` now initializes and
  updates the visible pool variable. The canonical REPEAT contains a WHILE
  entry check over the current pool value and an UNTIL end-check BLOCK_EXPR
  that advances the pool value, including after ITERATE. A LEAVE skips the
  advance. Source control-variable mutation, zero-entry, nested loops,
  local procedure scope and final values matched Regina in optimized/no-opt
  and linked execution. Nonnumeric control values report `41.6`; BY, dynamic
  TO and controlled-plus-condition headers remain fail-closed. The redirected
  tree probe showed source-anchored WHILE > controlToContinue and UNTIL >
  BLOCK_EXPR > controlStepByOne/setValue > LEAVE_WITH, with no structural
  validation error. The selected Release Level C/source-provenance/shared
  runtime suite passed 195/195, and Debug structural/loop/shared-value tests
  passed 70/70. An initial focused test used stale generated CTest expected
  output after adding nested fixture lines; regenerating CMake restored a
  5/5 focused pass before the larger suites. Evidence:
  `/tmp/crexx-levelc-controlled-reference-output.hCs4cu`,
  `/tmp/crexx-levelc-controlled-release-build-final.50sCac`,
  `/tmp/crexx-levelc-controlled-focused-final.0OGSCw`,
  `/tmp/crexx-levelc-controlled-release-suite.AeafLD`,
  `/tmp/crexx-levelc-controlled-debug-build.4IIBOZ`,
  `/tmp/crexx-levelc-controlled-debug-focused.9vFoUy`,
  `/tmp/crexx-levelc-controlled-tree.Pr1gSI`, and
  `/tmp/crexx-levelc-controlled-link-log.grYt3z`.
- Signed literal BY now works in either TO/BY parser order. A negative step
  checks the lower bound, while zero and positive steps check the upper bound;
  the UNTIL end block advances the current pool value after ordinary and
  ITERATE paths. Bounded Regina output matched optimized/no-opt and linked
  execution for descending, ascending, guarded zero, opposite-direction
  zero-entry, body mutation, nested transfer and final values. The dynamic
  BY shape remains fail-closed. The redirected tree probe showed source
  anchors through WHILE > controlToContinueBy and UNTIL > BLOCK_EXPR >
  controlStepBy/setValue > LEAVE_WITH, with no structural validation error.
  Release selected correctness passed 198/198 and Debug structural/loop and
  shared-value tests passed 74/74. An initial shared-value build warning
  about an implicit branch-local binding was corrected before final build
  and suites. Evidence:
  `/tmp/crexx-levelc-by-reference-output.ECkvKQ`,
  `/tmp/crexx-levelc-by-release-build.WYm1fx`,
  `/tmp/crexx-levelc-by-release-rebuild.gGQzDh`,
  `/tmp/crexx-levelc-by-release-suite.yOq0q2`,
  `/tmp/crexx-levelc-by-debug-build.GCddqg`,
  `/tmp/crexx-levelc-by-debug-focused.2pMz1t`,
  `/tmp/crexx-levelc-by-tree.hgdhBa`, and
  `/tmp/crexx-levelc-by-link-log.HBlxiF`.
- A bounded literal FOR now joins a controlled TO header, with or without
  literal BY, in any parsed modifier order. The pool assignment precedes
  canonical REPEAT; FOR checks before WHILE, and the UNTIL end block advances
  the visible control variable after every executed body, including the final
  count-limited body and ITERATE. Regina output matched optimized/no-opt and
  linked execution for zero count, BY direction, mutation, LEAVE/ITERATE and
  local scope. Dynamic FOR and FOR without TO remain fail-closed. The
  redirected tree probe showed source-anchored FOR > WHILE > UNTIL with the
  BLOCK_EXPR pool step and no structural validation error. Release controlled
  focused checks passed 15/15 and the selected correctness suite 203/203;
  Debug structural/loop/shared-value checks passed 79/79. Evidence:
  `/tmp/crexx-levelc-for-reference-output.SfXAWx`,
  `/tmp/crexx-levelc-for-release-build.XvkBMA`,
  `/tmp/crexx-levelc-for-focused.X9e0I0`,
  `/tmp/crexx-levelc-for-release-suite.oK2LjO`,
  `/tmp/crexx-levelc-for-debug-build.Fd8bOH`,
  `/tmp/crexx-levelc-for-debug-focused.VbyWh3`,
  `/tmp/crexx-levelc-for-tree.5Kfxxl`, and
  `/tmp/crexx-levelc-for-link-log.GnUCkq`.
- A bounded literal controlled FOR now works without TO, with default or
  signed literal BY in either clause order. The canonical REPEAT has FOR and
  a source-anchored UNTIL/BLOCK_EXPR pool step, and the redirected tree probe
  found zero WHILE nodes and no structural validation error. Regina output
  matched optimized/no-opt and linked execution for zero count, ascending
  and descending steps, body mutation, ITERATE, LEAVE, nested grouping, local
  scope and final values. Unbounded controlled DO remains fail-closed. Release
  controlled focused checks passed 18/18 and the selected correctness suite
  206/206; Debug structural/loop/shared-value checks passed 82/82. Evidence:
  `/tmp/crexx-levelc-foronly-reference-output.TYlJ13`,
  `/tmp/crexx-levelc-foronly-release-build.Ngk8sl`,
  `/tmp/crexx-levelc-foronly-focused.bYKbZo`,
  `/tmp/crexx-levelc-foronly-tree.UrI1Uh`,
  `/tmp/crexx-levelc-foronly-link-log.xLD8XV`,
  `/tmp/crexx-levelc-foronly-release-suite.btNax4`,
  `/tmp/crexx-levelc-foronly-debug-build.JtE4St`, and
  `/tmp/crexx-levelc-foronly-debug-focused.XyAKqI`.
- Named `LEAVE` and `ITERATE` now bind case-insensitively to active supported
  source controlled loops, then use those loops' hidden canonical symbols
  through nested controlled loops, simple DO, IF and generated SELECT arms.
  Regina output matched opt/no-opt and linked execution, including outer
  targets, local procedure scope, visible final control values and a SELECT
  branch that first iterates and then leaves. Invalid names retained `28.3`
  and `28.4`. The redirected tree probe showed both SELECT-arm transfers
  resolving to the outer `__rxcp_levelc_loop_183` symbol with source anchors,
  and no structural validation error. Focused Release and Debug checks each
  passed 87/87; the selected Release correctness suite passed 211/211.
  Evidence: `/tmp/crexx-levelc-named-final-output.ry3ND0`,
  `/tmp/crexx-levelc-named-final-release-build.BGTyHT`,
  `/tmp/crexx-levelc-named-final-focused.mSJH86`,
  `/tmp/crexx-levelc-named-final-release-suite.GpPLgy`,
  `/tmp/crexx-levelc-named-final-debug-build.IAFCQu`,
  `/tmp/crexx-levelc-named-final-debug-focused.asSU2I`,
  `/tmp/crexx-levelc-named-final-tree-log.ciEinH`,
  `/tmp/crexx-levelc-named-final-rxlink.c9e0QC`, and
  `/tmp/crexx-levelc-named-final-rxvm.HZjxKG`.
- Controlled literal TO/FOR/BY loops now accept a supported WHILE condition.
  A canonical entry BLOCK_EXPR branches on the TO guard before evaluating
  WHILE and its setup; FOR remains the outer count guard. The source-anchored
  tree probe showed the two LEAVE_WITH paths and existing UNTIL pool step.
  Regina output matched opt/no-opt and linked execution for zero FOR, failed
  TO, TO plus FOR, signed BY, named ITERATE/LEAVE, local scope and final
  control values; invalid WHILE retained `34.3`. Controlled UNTIL remains
  fail-closed. Focused Release and Debug checks each passed 30/30; the
  selected Release suite passed 215/215. Evidence:
  `/tmp/crexx-levelc-controlled-while-final-output.ZFMbkl`,
  `/tmp/crexx-levelc-controlled-while-final-release-build.blK8E2`,
  `/tmp/crexx-levelc-controlled-while-final-debug-build.fhsCvL`,
  `/tmp/crexx-levelc-controlled-while-final-focused-release.P8Rr9m`,
  `/tmp/crexx-levelc-controlled-while-final-focused-debug.ezz8o0`,
  `/tmp/crexx-levelc-controlled-while-final-tree-log.RnyIDf`,
  `/tmp/crexx-levelc-controlled-while-final-rxlink.vXcWAn`,
  `/tmp/crexx-levelc-controlled-while-final-rxvm.hWDN67`, and
  `/tmp/crexx-levelc-controlled-while-final-suite.3s3YaD`.
- Controlled literal TO/FOR/BY loops now also accept a supported UNTIL
  condition. The canonical UNTIL end BLOCK_EXPR evaluates the condition and
  its setup first, returns true without advancing the pool variable, or
  advances the pool variable and returns false. The redirected tree probe
  showed both source-anchored LEAVE_WITH paths and no structural error.
  Regina output matched opt/no-opt and linked execution for failed TO and
  zero FOR guards, TO plus FOR, signed BY, named ITERATE and LEAVE, local
  scope, true UNTIL final values and false UNTIL after the final FOR body.
  Invalid UNTIL retained `34.4`. Focused Release and Debug checks each
  passed 33/33; the selected Release suite passed 218/218. Evidence:
  `/tmp/crexx-levelc-controlled-until-final-output.sl0khV`,
  `/tmp/crexx-levelc-controlled-until-final-release-build.6GfDgD`,
  `/tmp/crexx-levelc-controlled-until-final-debug-build.eZi4Ou`,
  `/tmp/crexx-levelc-controlled-until-final-focused-release.xvEh8a`,
  `/tmp/crexx-levelc-controlled-until-final-focused-debug.kqQ3Fc`,
  `/tmp/crexx-levelc-controlled-until-final-tree-log.xlrpul`,
  `/tmp/crexx-levelc-controlled-until-final-rxlink.8NqDE8`,
  `/tmp/crexx-levelc-controlled-until-final-rxvm.o7OKTS`, and
  `/tmp/crexx-levelc-controlled-until-final-suite.4HfEBG`.
- Scalar controlled DO now runs without TO or FOR, using its literal start,
  optional signed literal BY and the existing pool-backed end step. Guarded
  Regina output matched opt/no-opt and linked execution for default and
  descending steps, body mutation, named ITERATE/LEAVE, WHILE/UNTIL and
  local scope. The redirected tree probe showed the bare source DO lowered
  to one REPEAT with its hidden assignment and UNTIL pool step, with no FOR
  or synthetic WHILE on that loop. Release slice checks passed 3/3, Debug
  structural/loop/shared-value checks passed 35/35, and the selected Release
  correctness suite passed 220/220. Evidence:
  `/tmp/crexx-levelc-unbounded-control-reference-output.HRixQf`,
  `/tmp/crexx-levelc-unbounded-control-build.a4MQjg`,
  `/tmp/crexx-levelc-unbounded-control-focused.9MyCQ7`,
  `/tmp/crexx-levelc-unbounded-control-final-debug-build.NA5bWO`,
  `/tmp/crexx-levelc-unbounded-control-final-debug-focused.wHupzq`,
  `/tmp/crexx-levelc-unbounded-control-final-tree-log.N2zw6I`,
  `/tmp/crexx-levelc-unbounded-control-final-rxlink.Uz6AhR`,
  `/tmp/crexx-levelc-unbounded-control-final-rxvm.RtgQkL`, and
  `/tmp/crexx-levelc-unbounded-control-final-suite.BJQr1F`.
- Supported dynamic TO expressions are now evaluated and validated before
  assigning the new visible control value, then captured in a hidden
  canonical RexxValue local. The redirected tree probe showed the
  `controlToValue` capture before the pool assignment and a single hidden
  reference in each TO check. Regina output matched opt/no-opt and linked
  execution for endpoint mutation, a pre-existing control value observed by
  a TO function under FOR-zero, signed BY, WHILE/UNTIL and named outer
  ITERATE. A nonnumeric TO under FOR-zero reports `41.4` at setup. The shared
  `rxfnsc` image built in Release and Debug; focused Release checks passed
  4/4, the selected Release Level C/RexxValue/RexxScript suite 223/223, and
  Debug structural/loop/shared-value/RexxScript checks 44/44. Evidence:
  `/tmp/crexx-levelc-dynamic-to-reference-output.Oxyw65`,
  `/tmp/crexx-levelc-dynamic-to-build.VacXdq`,
  `/tmp/crexx-levelc-dynamic-to-focused.7wMczl`,
  `/tmp/crexx-levelc-dynamic-to-final-suite.j7fm7V`,
  `/tmp/crexx-levelc-dynamic-to-final-debug-build.brP7hY`,
  `/tmp/crexx-levelc-dynamic-to-final-debug-focused.zlljwp`,
  `/tmp/crexx-levelc-dynamic-to-final-tree-log.3nGjLn`,
  `/tmp/crexx-levelc-dynamic-to-final-rxlink.U0YRNN`, and
  `/tmp/crexx-levelc-dynamic-to-final-rxvm.aOFKn4`.
- Supported dynamic BY expressions now capture once before the new visible
  control assignment. Dynamic TO and BY follow the parser's written order;
  the redirected tree probe showed both orders, with the captured BY value
  used for TO direction and the UNTIL pool step. Regina output matched
  opt/no-opt and linked execution for source mutation, positive/negative
  and guarded zero steps, FOR-zero, no-TO, WHILE/UNTIL, named ITERATE,
  and local scope. Nonnumeric BY under FOR-zero reports `41.5` at setup.
  The shared `rxfnsc` image built in Release and Debug; focused Release
  checks passed 4/4, the selected Release Level C/RexxValue/RexxScript
  suite 226/226, and Debug structural/loop/shared-value/RexxScript checks
  47/47. Evidence: `/tmp/crexx-levelc-dynamic-by-final-output.OPzBAF`,
  `/tmp/crexx-levelc-dynamic-by-build.WijpWN`,
  `/tmp/crexx-levelc-dynamic-by-focused.GJluYO`,
  `/tmp/crexx-levelc-dynamic-by-final-suite.wA6Kiu`,
  `/tmp/crexx-levelc-dynamic-by-final-debug-build.7Dxae4`,
  `/tmp/crexx-levelc-dynamic-by-final-debug-focused.7dUaJ5`,
  `/tmp/crexx-levelc-dynamic-by-final-tree-log.NJpdo3`,
  `/tmp/crexx-levelc-dynamic-by-final-rxlink.TkVth6`, and
  `/tmp/crexx-levelc-dynamic-by-final-rxvm.IpplQZ`.
- The independent optimized call-window defect exposed by controlled FOR/BY
  now has an existing-syntax fixture. With the original affinity, its
  optimized path reported `41.6` after one body; no-opt and Regina produced
  both bodies and final control value 3. Disabling affinity for every call
  passed this fixture but made the existing `levelc_slice15_while` hang; the
  bounded repair uses a disjoint window only for calls under a canonical
  UNTIL end check. The emitted step call now marshals/restores the captured
  BY register. Release selected compiler checks passed 250/250, Debug
  focused checks passed 52/52, and linked RXBIN output matched Regina.
  Evidence: `/tmp/crexx-levelc-for-min.Yf5AZm/zero_then_literal-old.output`,
  `/tmp/crexx-levelc-call-window-preassign-focused.j5D997` (rejected broad
  candidate), `/tmp/crexx-levelc-call-window-until-focused.h6kqpx`,
  `/tmp/crexx-levelc-call-window-release-suite-final2.JPPBXU`,
  `/tmp/crexx-levelc-call-window-debug-build.d8G1D4`,
  `/tmp/crexx-levelc-call-window-final-debug-rebuild.5NAeJ4`,
  `/tmp/crexx-levelc-call-window-debug-focused-final2.RSuasd`,
  `/tmp/crexx-levelc-call-window-emission.81LA5Z`,
  `/tmp/crexx-levelc-call-window-regina-output.sLJ3gC`,
  `/tmp/crexx-levelc-call-window-final-rxlink.fh7JWi`, and
  `/tmp/crexx-levelc-call-window-final-rxvm.wmAT3T`.
- Controlled FOR now accepts a supported dynamic expression under the same
  written-order header capture used by TO and BY, before setting the visible
  control variable. The hidden signed-32-bit canonical count is consumed by
  REPEAT FOR and validated with contextual `26.3`. The source-anchored tree
  showed FOR/TO/BY captures in written order before pool assignment and one
  hidden FOR reference, with no structural validation error. Regina output
  matched optimized/no-opt and linked execution for zero and positive counts,
  mutation, whole decimal spelling, WHILE/UNTIL timing, named ITERATE, local
  scope and final pool values. Text, fractional, negative and over-range
  counts report `26.3`. Release focused checks passed 7/7 and the selected
  Level C/source-provenance/RexxValue/RexxScript suite 234/234; Debug
  structural/loop/shared-value/RexxScript checks passed 59/59. Evidence:
  `/tmp/crexx-levelc-dynamic-for-final-output.Hx3vbP`,
  `/tmp/crexx-levelc-dynamic-for-final-build.3XoHet`,
  `/tmp/crexx-levelc-dynamic-for-final-focused.XGC1lm`,
  `/tmp/crexx-levelc-dynamic-for-final-release-suite.OU1FgC`,
  `/tmp/crexx-levelc-dynamic-for-final-debug-build.qQtBk9`,
  `/tmp/crexx-levelc-dynamic-for-final-debug-focused.AzfcWY`,
  `/tmp/crexx-levelc-dynamic-for-tree-log.imSL3L`,
  `/tmp/crexx-levelc-dynamic-for-final-rxlink.5slfNf`, and
  `/tmp/crexx-levelc-dynamic-for-final-rxvm.YpKaJt`.
- Captured dynamic start now uses a hidden canonical `RexxValue` before the
  written-order TO/BY/FOR captures. The `STAGE_LEVELC_LOWERED` tree shows
  start, TO, BY and FOR in source order, then `controlStartValue()` at the
  visible pool set, with the start source anchor intact. Regina and linked
  RXBIN output match byte for byte for zero FOR, source mutation, signed
  decimal spelling, WHILE/UNTIL, named ITERATE, local scope and final pool
  values. Invalid start reports `41.6` after valid later clauses; invalid TO
  reports `41.4` first, and an invalid FOR count reports `26.3` first.
  Final Release Level C checks passed 229/229, focused Release 6/6, Debug
  controlled/structural/shared checks 57/57, and shared Release
  RexxValue/RexxScript checks 4/4. Evidence:
  `/tmp/crexx-levelc-dynamic-start-regina-output.nv9R5P`,
  `/tmp/crexx-levelc-dynamic-start-final-release-build.DpHht2`,
  `/tmp/crexx-levelc-dynamic-start-final-release-focused.VrFMdb`,
  `/tmp/crexx-levelc-dynamic-start-final-release-suite.cqh4Re`,
  `/tmp/crexx-levelc-dynamic-start-shared-release.c1vnYz`,
  `/tmp/crexx-levelc-dynamic-start-final-debug-build.cyqgNB`,
  `/tmp/crexx-levelc-dynamic-start-debug-focused.raexX4`,
  `/tmp/crexx-levelc-dynamic-start-tree.ODAevD`,
  `/tmp/crexx-levelc-dynamic-start-final-toolchain.hHePVE`,
  `/tmp/crexx-levelc-dynamic-start-final-linked-output.lEaINL`,
  `/tmp/crexx-levelc-dynamic-start-clause-order.UametO`, and
  `/tmp/crexx-levelc-dynamic-start-for-compile.z01VEf`.
- Empty single- and double-quoted source strings now lower to a canonical
  source-anchored empty `STRING` under the shared `RexxValue` factory. Regina
  and linked RXBIN output match byte for byte for SAY, assignment,
  concatenation, supported BIF/local calls and strict comparison. Empty TO
  reports contextual `41.4` instead of failing compilation. Final focused
  Release checks passed 4/4, the Release Level C suite 233/233 and Debug
  expression/AST/shared-value checks 20/20. The first tree test assertion
  named the parser comparison node rather than the canonical method; it was
  corrected, and no runtime regression was found. Evidence:
  `/tmp/crexx-levelc-empty-literal-baseline.AfmyLS`,
  `/tmp/crexx-levelc-empty-string-regina-output.rsxwSI`,
  `/tmp/crexx-levelc-empty-string-release-build.54d6FN`,
  `/tmp/crexx-levelc-empty-string-focused-release-final.d5hGwu`,
  `/tmp/crexx-levelc-empty-string-release-suite.1ZU6a4`,
  `/tmp/crexx-levelc-empty-string-debug-build.4gThvF`,
  `/tmp/crexx-levelc-empty-string-debug-focused.OVm4j4`,
  `/tmp/crexx-levelc-empty-string-final-tree.4BIMZA`,
  `/tmp/crexx-levelc-empty-string-link-log.djwxRM`, and
  `/tmp/crexx-levelc-empty-string-linked-output.3VVRE6`.
- A continuation-expression function name now becomes a source `FUNCTION`
  when its opening parenthesis touches the name; a blank-separated name and
  group remain `VAR_SYMBOL` plus `OP_SCONCAT`. The raw and lowered tree
  probes show the intended distinction and existing canonical BIF/local-call
  paths. Regina output matches opt/no-opt and linked RXBIN for literal and
  variable left operands, nested BIF arguments, empty string arguments, a local
  function and the spaced form. Final focused Release checks passed 3/3,
  Release Level C 236/236, and Debug parser/expression/shared checks 74/74.
  The distinct `ARG` case-normalization finding remains `LC-FIND-03`.
  Evidence: `/tmp/crexx-levelc-adjacent-call-regina-output.rmEEm9`,
  `/tmp/crexx-levelc-adjacent-call-release-build.ksfzMl`,
  `/tmp/crexx-levelc-adjacent-call-focused-release-final.yn84NQ`,
  `/tmp/crexx-levelc-adjacent-call-release-suite.cfqQkL`,
  `/tmp/crexx-levelc-adjacent-call-debug-build.6GCUaK`,
  `/tmp/crexx-levelc-adjacent-call-debug-focused.7jrIHu`,
  `/tmp/crexx-levelc-adjacent-call-tree.M4wN3G`,
  `/tmp/crexx-levelc-adjacent-call-link-log.OkFc9t`, and
  `/tmp/crexx-levelc-adjacent-call-linked-output.irO9L1`.
- Direct local `ARG` now applies shared Classic TRANSLATE to each captured
  argument before its source-anchored canonical pool set. The tree shows one
  BIF context/frame and call per target, in target order, with the import
  generated only when required. Regina and linked RXBIN output match for
  lowercase/mixed-case and spaced values, multiple arguments, exposure,
  unchanged caller variables and an empty argument. Final focused Release
  checks passed 8/8, Release Level C 239/239, shared TRANSLATE/RexxValue
  4/4, and Debug procedure/structural/shared checks 31/31. The current Level C
  call configuration defaults to BYTE; UTF8 delivery remains open under
  `LC-AC-04/06`. Evidence:
  `/tmp/crexx-levelc-arg-uppercase-reference.2nn27c`,
  `/tmp/crexx-levelc-arg-uppercase-baseline.qpCi0e`,
  `/tmp/crexx-levelc-arg-uppercase-regina-output.yaoWNR`,
  `/tmp/crexx-levelc-arg-uppercase-release-build.XJaXBG`,
  `/tmp/crexx-levelc-arg-uppercase-focused-release.GhBR4m`,
  `/tmp/crexx-levelc-arg-uppercase-release-suite.YLlJyL`,
  `/tmp/crexx-levelc-arg-uppercase-shared-release.OePRCZ`,
  `/tmp/crexx-levelc-arg-uppercase-debug-build.nR7gEZ`,
  `/tmp/crexx-levelc-arg-uppercase-debug-focused.DWm9zr`,
  `/tmp/crexx-levelc-arg-uppercase-tree.3Hzw3j`,
  `/tmp/crexx-levelc-arg-uppercase-link-log.pQWMl4`, and
  `/tmp/crexx-levelc-arg-uppercase-linked-output.c6E1qt`.
- The first `PARSE` AST slice validates the parser's outer and inner
  `TEMPLATES` nodes, then lowers one direct scalar target for `VAR` or `VALUE`
  sources, with optional UPPER through the shared Classic TRANSLATE frame.
  The raw tree has distinct source/template nodes; the canonical tree has
  source-anchored `symbolValue`/`setValue` and BIF calls with no `PARSE` node.
  Regina and linked RXBIN output match byte for byte for source preservation,
  a once-evaluated source expression, nested IF/DO, procedure scope, self
  assignment and uppercase. Multiple targets and literal patterns still fail
  closed. The boundary verifier now also rejects a residual PARSE or template
  node. Final Release Level C passed 245/245, eight focused Release tests and
  13 Debug structural/procedure tests passed. Evidence:
  `/tmp/crexx-levelc-parse-probe.kjXGc9/tree.log`,
  `/tmp/crexx-levelc-parse-fixture.wWlWfo/regina.out`,
  `/tmp/crexx-levelc-parse-fixture.wWlWfo/linked.out`,
  `/tmp/crexx-levelc-parse-final-build.a4BefS`,
  `/tmp/crexx-levelc-parse-release-suite.xumsvR`,
  `/tmp/crexx-levelc-parse-final-release-focused.0ti3pv`,
  `/tmp/crexx-levelc-parse-debug-build.bALj8Y`,
  `/tmp/crexx-levelc-parse-debug-focused.CyCSsi`,
  `/tmp/crexx-levelc-parse-tree.15G3Tc`, and
  `/tmp/crexx-levelc-parse-link.FDBVgU`, plus final verifier/build and linked
  proof in `/tmp/crexx-levelc-parse-verifier-build.z0XG9n`,
  `/tmp/crexx-levelc-parse-verifier-test-build.qZLfQH`,
  `/tmp/crexx-levelc-parse-final-suite.NMhbqO`,
  `/tmp/crexx-levelc-parse-final-debug-build.5Atjbl`,
  `/tmp/crexx-levelc-parse-final-debug-tests.GYe1lb`, and
  `/tmp/crexx-levelc-parse-final-toolchain.a8dmN3`.
- Three adjacent direct scalar `PARSE` targets now use
  `RexxValue.parseThreeWords()`, which wraps the existing `parsewords3` VM
  opcode and returns two words plus the original remaining tail. The Level C
  lowerer captures the source and result array before ordered pool writes;
  shared TRANSLATE handles optional UPPER. The optimized and no-opt fixtures
  match Regina byte for byte for leading/repeated/trailing blanks, short
  input, repeated targets, a source alias, one-time VALUE evaluation and local
  scope. The raw tree's nested templates disappear from the canonical tree;
  generated array/result nodes retain source anchors. Four-target templates
  and patterns still fail closed. Release Level C passed 249/249, focused
  Release 14/14, Debug 15/15, shared RexxValue opt/no-opt 2/2 and RexxScript
  runtime/compatibility 4/4. The linked RXBIN output matches Regina.
  Evidence: `/tmp/crexx-levelc-parse-three-reference.VE49lG`,
  `/tmp/crexx-levelc-parse-three-probe.OKSy14/regina-explicit.out`,
  `/tmp/crexx-levelc-parse-three-probe.OKSy14/crexx3.out`,
  `/tmp/crexx-levelc-parse-three-rxfnsc-build.oQsQ7O`,
  `/tmp/crexx-levelc-parse-three-compiler-build.JLasPA`,
  `/tmp/crexx-levelc-parse-three-shared-build.HJlCxG`,
  `/tmp/crexx-levelc-parse-three-shared-test.VDvktb`,
  `/tmp/crexx-levelc-parse-three-focused-release.VsyHp5`,
  `/tmp/crexx-levelc-parse-three-release-suite.s0dk44`,
  `/tmp/crexx-levelc-parse-three-debug-build.LCVPRF`,
  `/tmp/crexx-levelc-parse-three-debug-tests.KBNkZ2`,
  `/tmp/crexx-levelc-parse-three-rexxscript-build.FR90F5`,
  `/tmp/crexx-levelc-parse-three-rexxscript-tests.suoapn`,
  `/tmp/crexx-levelc-parse-three-tree.WZfAtg`,
  `/tmp/crexx-levelc-parse-three-fixture.46Rrly/regina.out`,
  `/tmp/crexx-levelc-parse-three-fixture.46Rrly/linked.out`, and
  `/tmp/crexx-levelc-parse-three-linked.xPkP8j`.
- Adjacent Classic terms now select the existing `OP_CONCAT` AST node; spaced
  terms retain `OP_SCONCAT`. The parser checks token gaps across group and
  call punctuation and comment-only gaps, while refusing to infer abuttal
  across an omitted dot. The raw tree shows both implicit nodes and the
  explicit `||` node; the lowered tree uses the canonical expression path.
  Regina and linked RXBIN output match byte for byte, including PARSE VALUE
  expressions. Release Level C passed 252/252, Debug focused 8/8, and the
  decimal FOR rejection remained green. Evidence:
  `/tmp/crexx-levelc-abuttal-release-suite.ClLyo4`,
  `/tmp/crexx-levelc-abuttal-debug-build.hrdpMk`,
  `/tmp/crexx-levelc-abuttal-debug-tests.AN4Zv6`,
  `/tmp/crexx-levelc-abuttal-tree.oD0XbH`, and
  `/tmp/crexx-levelc-abuttal-linked.gCu88S`.
- Decimal fraction constants now reach the raw tree as one source `DECIMAL`
  node for `1.5`, `1.`, and `.5`; the canonical tree constructs a shared
  `RexxValue` from the exact spelling. Regina output matches optimized,
  no-opt and linked execution for literals, arithmetic, PARSE VALUE and
  abutted expressions. Existing invalid fractional DO/FOR count diagnostics
  remain green; `1.0` and `1.` whole-number FOR probes also match Regina.
  Release Level C passed 255/255 and focused Debug 8/8. Evidence:
  `/tmp/crexx-levelc-decimal-tree.yu3Qoo`,
  `/tmp/crexx-levelc-decimal-final-suite.H6aH56`,
  `/tmp/crexx-levelc-decimal-final-debug-build.8hQLB7`,
  `/tmp/crexx-levelc-decimal-final-debug-tests.Sf2900`,
  `/tmp/crexx-levelc-decimal-final-linked.wudGXo`, and
  `/tmp/crexx-levelc-decimal-count.wVmu1e`.
- Exponent numeric constants with integer, trailing-dot and fractional
  mantissas now form one source `DECIMAL` node, with uppercase `E` in the
  lowered Classic value. The failing baseline raised a decimal conversion
  panic; the new opt/no-opt fixture matches Regina for source spelling,
  arithmetic, PARSE VALUE, abuttal and a whole-number FOR. A fractional
  exponent FOR still raises contextual `26.3`. Release Level C passed
  259/259, focused Debug 8/8, and linked output matches byte for byte.
  Evidence: `/tmp/crexx-levelc-exponent-baseline.UweCIf`,
  `/tmp/crexx-levelc-exponent-count-reference.Xcpebt`,
  `/tmp/crexx-levelc-exponent-tree.pBzsPS`,
  `/tmp/crexx-levelc-exponent-release-suite.2ogRVg`,
  `/tmp/crexx-levelc-exponent-debug-build.3r0pwZ`,
  `/tmp/crexx-levelc-exponent-debug-tests.nEbinY`, and
  `/tmp/crexx-levelc-exponent-linked.M8newV`.
- Digit-starting nonnumeric constant symbols now scan as one Level C-only
  raw token and one source `CONST_SYMBOL` node. The canonical tree creates
  uppercase shared `RexxValue` strings, independent of similarly named pool
  variables. Regina, optimized/no-opt and linked output match for letters,
  repeated periods, exponent-looking nonnumbers, PARSE VALUE and abuttal.
  The nonnumeric FOR count still reports `26.3`. The dedicated highlighter
  check classifies these as constants; Release Level C passed 264/264 and
  focused Debug passed 9/9. Evidence:
  `/tmp/crexx-levelc-constant-reference.FiKTwC`,
  `/tmp/crexx-levelc-constsym-tree.xkWjAO`,
  `/tmp/crexx-levelc-constsym-release-suite.P87UU4`,
  `/tmp/crexx-levelc-constsym-debug-build.MOE5TW`,
  `/tmp/crexx-levelc-constsym-debug-tests.6wtAsS`, and
  `/tmp/crexx-levelc-constsym-linked.jx6fO9`.
- Two direct scalar `PARSE` targets now use shared
  `RexxValue.parseWordAndRest()`: the first receives the first word and the
  second receives the source after exactly one separator. The method calls
  the existing `parsewords3` VM primitive for the first word, then retains
  the unparsed remainder from the captured source. The Level C lowerer
  captures both results before ordered pool writes and reuses shared
  TRANSLATE for optional UPPER. Regina, optimized/no-opt and linked output
  match for leading/repeated/trailing blanks, short input, repeated targets,
  source alias, one-time VALUE evaluation and local scope. Raw nested
  templates become canonical array/result and pool-write nodes. Existing
  IF/DO negative fixtures were advanced to four-target shapes at that step;
  `LC-AC-49` later advanced them to five. Release Level C passed 267/267, focused Debug 7/7, shared
  RexxValue opt/no-opt 2/2 and RexxScript integration 4/4. Evidence:
  `/tmp/crexx-levelc-parse-two-reference.65kbCR`,
  `/tmp/crexx-levelc-parse-two-reconfigure.N7F6ew`,
  `/tmp/crexx-levelc-parse-two-final-suite.iG3FvQ`,
  `/tmp/crexx-levelc-parse-two-debug-build.AM9MDF`,
  `/tmp/crexx-levelc-parse-two-debug-tests.hufXxc`,
  `/tmp/crexx-levelc-parse-two-rexxscript-tests.3Xjp1D`,
  `/tmp/crexx-levelc-parse-two-tree.lFVAkL`, and
  `/tmp/crexx-levelc-parse-two-linked.JAcmAY`.
- Three direct scalar `PARSE` targets followed by a final drop dot now use
  shared `RexxValue.parseThreeWordsDrop()` and the existing `parsewords3d`
  VM primitive. The lowerer validates the nested template, captures all
  results before ordered pool writes, and emits no dot target. Regina,
  optimized/no-opt, and linked output match for whitespace, short input,
  aliases, repeated targets, one-time VALUE evaluation, UPPER, and local
  scope. Middle dots were handled in `LC-AC-47`. The raw tree contains the final
  target dot; the canonical tree contains only the three result writes.
  Release Level C passed 271/271, focused Release 9/9, Debug 6/6, shared
  RexxValue opt/no-opt 2/2, and RexxScript integration 4/4. Evidence:
  `/tmp/crexx-levelc-parse-dot-release-suite.RYgUiH`,
  `/tmp/crexx-levelc-parse-dot-focused.edkWI2`,
  `/tmp/crexx-levelc-parse-dot-debug-build.n1RpSW`,
  `/tmp/crexx-levelc-parse-dot-debug-tests.TBCnql`,
  `/tmp/crexx-levelc-parse-dot-shared-tests.ilOeIV`,
  `/tmp/crexx-levelc-parse-dot-rexxscript-tests.SlcJ1n`,
  `/tmp/crexx-levelc-parse-dot-tree.HINvyR`, and
  `/tmp/crexx-levelc-parse-dot-linked.lXTUHm`.
- Two- and three-item PARSE templates now accept dot placeholders in any
  position when at least one scalar target remains. The lowerer uses the
  existing shared split methods, captures source/results once, skips dot
  pool writes, and preserves scalar write order. Regina, opt/no-opt and
  linked output match for leading/internal/final and repeated dots, source
  aliases, repeated targets, short input, UPPER, local scope and one-time
  VALUE evaluation. The raw tree retains target dots; the canonical tree
  uses only split calls and scalar pool writes. The previous middle-dot
  negative fixture was removed after its now-supported shape passed in the
  positive fixture. Four-item internal-dot and all-dot forms were carried to
  `LC-AC-49`.
  Release Level C passed 275/275, focused Release 14/14, and focused Debug
  14/14. Evidence: `/tmp/crexx-levelc-parse-placeholder-focused.UAmhb4`,
  `/tmp/crexx-levelc-parse-placeholder-final-release-suite.uHdOV3`,
  `/tmp/crexx-levelc-parse-placeholder-debug-tests.PNHONc`,
  `/tmp/crexx-levelc-parse-placeholder-tree.lWOHrY`, and
  `/tmp/crexx-levelc-parse-placeholder-linked.VmRJjp`.
- One-, two- and three-item all-dot direct PARSE templates now consume their
  source without writing the visible pool. A hidden canonical capture for
  the single-dot form preserves one-time VALUE expression effects in both
  optimizer modes; longer forms use the existing shared split-result capture.
  Regina, optimized/no-opt and linked output agree for unchanged VAR sources,
  repeated VALUE side effects, UPPER and local procedure scope. The raw tree
  retains each dot target; the canonical tree has hidden captures but no dot
  pool binding. The obsolete two-dot negative was replaced with a four-dot
  negative at that step; `LC-AC-49` later covered four-dot templates.
  Release Level C passed 278/278, focused Release 6/6,
  and focused Debug 7/7. Evidence:
  `/tmp/crexx-levelc-parse-all-dots-reference.rXJeiX`,
  `/tmp/crexx-levelc-parse-all-dots-focused.yvfJDh`,
  `/tmp/crexx-levelc-parse-all-dots-release-suite.CaCKT2`,
  `/tmp/crexx-levelc-parse-all-dots-debug-tests.yHx2pQ`,
  `/tmp/crexx-levelc-parse-all-dots-tree.Q551Qa`, and
  `/tmp/crexx-levelc-parse-all-dots-linked.OfbdAz`.
- Four-item direct scalar/dot PARSE templates now reuse one validated nested
  AST path. When the final item is a scalar, shared
  `RexxValue.parseFourWords()` composes `parsewords3` and
  `parseWordAndRest()` to retain the tail after the third word. With a final
  dot, `parseThreeWordsDrop()` supplies three words and the lowerer skips
  all dot positions. Source and results are captured before ordered scalar
  writes. Regina, opt/no-opt and linked output match for whitespace, short
  input, internal/final/all dots, aliases, repeated targets, one-time VALUE,
  UPPER and local scope. The raw tree retains four target items; the
  canonical tree uses existing split and pool-call nodes only. Three obsolete
  four-item negatives were removed; IF/DO negatives now use five targets.
  Release Level C passed 279/279 after that fixture update, focused Release
  12/12, Debug 9/9 plus two updated negatives, shared RexxValue opt/no-opt
  2/2, and RexxScript integration 4/4. Evidence:
  `/tmp/crexx-levelc-parse-four-reference.TUuzbM`,
  `/tmp/crexx-levelc-parse-four-focused.8Wk6eF`,
  `/tmp/crexx-levelc-parse-four-final-release-suite.gFH8A3`,
  `/tmp/crexx-levelc-parse-four-debug-tests.CnB0ru`,
  `/tmp/crexx-levelc-parse-four-updated-negative-release.eyBDrr`,
  `/tmp/crexx-levelc-parse-four-updated-negative-debug.Ehqwax`,
  `/tmp/crexx-levelc-parse-four-rexxscript-tests.QMwz3F`,
  `/tmp/crexx-levelc-parse-four-tree.hlZBCl`, and
  `/tmp/crexx-levelc-parse-four-linked.LtDJF8`.
- Direct scalar/dot `PARSE VAR` and `PARSE VALUE` templates now use one
  validated AST lowering path for any positive representable item count.
  `RexxValue.parseWordTemplate(count)` returns the first `count-1` words and
  the original remaining tail; the compiler captures that result vector
  before scalar pool writes and skips only dot writes. The previous
  count-specific dispatch and four-item guard are gone. Regina and optimized,
  no-opt and linked RXBIN output match for five and eight items, whitespace,
  short input, dots including all-dot, aliases, repeated targets, one-time
  VALUE evaluation, UPPER and local scope. Prior one- through four-item
  fixtures remain green, and the IF/DO unsupported checks now use pattern
  templates. Release Level C passed 281/281; focused Release and Debug each
  passed 31/31; RexxScript integration passed 4/4. Raw/canonical tree
  inspection and the tree-shape tests show one `parseWordTemplate` lowering
  call per source PARSE and no surviving Level C template node. Evidence:
  `/tmp/crexx-levelc-parse-generic-release-build.YRynPa`,
  `/tmp/crexx-levelc-parse-generic-focused-release.m3fwAi`,
  `/tmp/crexx-levelc-parse-generic-release-suite.gPK7ld`,
  `/tmp/crexx-levelc-parse-generic-debug-build.YlI2Sa`,
  `/tmp/crexx-levelc-parse-generic-focused-debug.xN3a72`,
  `/tmp/crexx-levelc-parse-generic-rexxscript-build.dfVPXV`,
  `/tmp/crexx-levelc-parse-generic-rexxscript-tests.ERFAIo`,
  `/tmp/crexx-levelc-parse-generic-tree.zHv2HE`, and
  `/tmp/crexx-levelc-parse-generic-link.TfKI0n/`.
- `LC-AC-08/04` remain open for other controlled
  endpoints, wider count values, named
  transfers to wider unsupported loop shapes, and other structural families.

### LC-STEP-54 — static mixed PARSE templates, 2026-10-03

- `PARSE VAR` and `PARSE VALUE` now accept one validated static template with
  any representable ordered mix of scalar/dot targets, literal delimiters and
  absolute/relative positions. The compiler serializes the parsed AST to the
  existing frozen version-1 `parseplan` descriptor and emits one canonical
  `ASSEMBLER` call. The VM helper owns delimiter search, cursor movement and
  field capture; the lowerer snapshots the source and writes non-dot results
  through the Classic variable pool in authored order. Direct word-only
  templates retain the generic shared `RexxValue` path. No target-count or
  item-order dispatch and no new AST/emitter node were needed.
- Regina and optimized/no-opt output agree for leading, trailing, adjacent,
  repeated and missing patterns; absolute, relative, backward, zero and
  out-of-range positions; numeric spelling overlays; hex, binary, quoted and
  UTF-8 delimiters; all-dot templates; aliases, six targets, one-time VALUE
  effects, UPPER and local scope. Raw tree inspection retains ordered source
  `PARSE` children; the canonical tree has `parseplan` and pool writes with
  no surviving Level C template nodes. The static fixture is a permanent
  opt/no-opt and tree-shape regression. Dynamic pattern/position operands,
  comma templates and other source forms remain open.
- Release core build passed, focused Release 6/6 and Debug 6/6 passed, and
  the normal Release Level C suite passed 284/284. The final fixture compiled,
  assembled, linked and executed with byte-identical Regina output. The
  opt-in UTF8 configuration remains open under `LC-AC-04/06`; this increment
  proves UTF-8 delimiter bytes in the current BYTE configuration.
  Evidence: `/tmp/crexx-levelc-static-close-release-build.ptGl28`,
  `/tmp/crexx-levelc-static-close-debug-build.UiSZmq`,
  `/tmp/crexx-levelc-static-close-release-focused.Tj5rLe`,
  `/tmp/crexx-levelc-static-close-debug-focused.osxsnh`,
  `/tmp/crexx-levelc-static-close-suite.b1oLs5`,
  `/tmp/crexx-levelc-static.4nBnDm/close-tree.log`, and
  `/tmp/crexx-levelc-static.4nBnDm/close-linked.out`.

### LC-STEP-56 — shared Classic stem lifecycle, 2026-10-03

- Regina reference probes showed that a stem default assignment replaces old
  explicit and dropped tails; `DROP stem.tail` hides the default for that tail
  until reassignment; and a variable-derived tail retains its substituted
  case. The prior shared model discarded dropped-tail state and uppercased
  derived text. This is the distinct shared-runtime cause behind the direct
  compound `DROP` work; `LC-AC-53` remains open for compiler lowering.
- `RexxStem` now tracks dropped-tail tombstones, clears old tail state on
  default assignment and clears a tombstone on tail assignment.
  `RexxVariablePool` preserves derived-tail case and offers non-mutating
  `stemSymbolValue` plus `dropStemTail`. A focused failure found that mutated
  stems retrieved from pool slots need copyback; the helper now writes them
  back to local or exposed bindings. The permanent pool test covers default
  reset, case-distinct tails, dropped reads, reassignment, stem drop and
  exposed mutation.
- Release `rxfnsc` and the standalone RexxScript runner built. Focused pool
  opt/no-opt and linked tests passed 2/2; RexxScript integration passed 4/4;
  the normal Release Level C suite passed 284/284. Evidence:
  `/tmp/crexx-levelc-drop.qf1gA9/semantics.out`,
  `/tmp/crexx-levelc-stem-runtime-focused.PEEXmi` (initial failure),
  `/tmp/crexx-levelc-stem-runtime-rebuild.oq0Frs` (scope diagnostic),
  `/tmp/crexx-levelc-stem-runtime-rebuild.EpZx8L`,
  `/tmp/crexx-levelc-stem-runtime-final-focused.mrPABk`,
  `/tmp/crexx-levelc-stem-rexxscript-build.uJaVLM`,
  `/tmp/crexx-levelc-stem-rexxscript-tests.DT87if`, and
  `/tmp/crexx-levelc-stem-runtime-levelc-suite.1Kg8ty`.

### LC-STEP-57 — direct stem and compound DROP, 2026-10-03

- The direct `DROP` guard now accepts scalar, stem and supported single-tail
  compound names in one ordered list. Each compound tail is materialized once
  immediately before its corresponding pool call. Lowering emits canonical
  `drop`, `dropStem` and `dropStemTail` calls; `RexxVariablePool` and
  `RexxStem` retain the default, dropped-tail and exposed-alias rules. Direct
  compound reads use the non-mutating `stemSymbolValue` method. Parenthesized
  indirect references remain guarded.
- The Regina fixture covers case-distinct derived tails, dropped reads,
  reassignment, list order with an earlier scalar drop affecting a later tail,
  full stem drop, numeric tails, exposed procedure scope and nested `IF`/`DO`.
  The linked image produced byte-identical output. Raw/canonical tree probes
  show source `LEVELC_DROP` nodes replaced by ordered helper calls with hidden
  tail captures. Obsolete direct stem/compound negative fixtures were replaced
  by optimized, no-opt and tree-shape regressions. An older tree-shape test was
  updated from `stemValue` to the new non-mutating read method.
- Release Level C passed 285/285 and focused Debug passed 5/5 after rebuilding
  the Debug `rxfnsc` metadata. RexxScript 4/4 and focused pool opt/no-opt 2/2
  evidence from `LC-STEP-56` remains valid because its runtime inputs did not
  change in this compiler increment. The initial Release sweep found only the
  outdated tree-shape expectation; the initial Debug run found an outdated
  local `rxfnsc` build. Evidence: `/tmp/crexx-levelc-drop-final-build.log`,
  `/tmp/crexx-levelc-drop-final-suite.log`,
  `/tmp/crexx-levelc-drop-debug-build.log`,
  `/tmp/crexx-levelc-drop-debug-runtime-build.log`,
  `/tmp/crexx-levelc-drop-debug-final-focused.log`,
  `/tmp/crexx-levelc-drop.qf1gA9/tree.log`,
  `/tmp/crexx-levelc-drop.qf1gA9/final-reference.out`, and
  `/tmp/crexx-levelc-drop.qf1gA9/linked-output.out`.

### LC-STEP-58 — shared one-symbol DROP, 2026-10-03

- `RexxVariablePool.dropSymbol` resolves one validated Classic symbol against
  the visible pool and dispatches to the existing scalar, stem or single-tail
  drop operation. The pool regression proves case-preserved substituted tails,
  unaffected case-distinct tails, full stem/default removal and exposed scalar
  and stem aliases. The helper accepts one symbol, leaving the separate
  subsidiary-list parser and invalid-word policy to `LC-STEP-59`.
- Release and Debug pool opt/no-opt tests passed 2/2 in each build; the Release
  RexxScript runtime/compatibility tests passed 4/4, and the normal Release
  Level C suite passed 285/285. The optimized pool test uses the linked image,
  covering `rxc`, `rxas`, `rxlink` and `rxvm`. Evidence:
  `/tmp/crexx-levelc-dropsymbol-release-build.log`,
  `/tmp/crexx-levelc-dropsymbol-debug-build.log`,
  `/tmp/crexx-levelc-dropsymbol-debug-focused.log`,
  `/tmp/crexx-levelc-dropsymbol-release-focused.log`, and
  `/tmp/crexx-levelc-dropsymbol-levelc-suite.log`.

### LC-STEP-59 — indirect DROP subsidiary lists, 2026-10-03

- `RexxVariablePool.dropIndirectList` consumes one captured list value and
  iterates every word in source order. The shared Classic symbol classifier
  skips invalid words per Adrian's Regina decision; each valid word goes
  through `dropSymbol`, so earlier drops affect later compound substitutions.
  The pool module's build dependencies now stage `RexxClassicConfig` and
  `RexxClassicDatatype`. The initial focused build exposed the missing module
  staging and was repaired before qualification.
- Level C's guarded `VAR_REFERENCE` shape now lowers to a single canonical
  `dropIndirectList` call with the referenced variable evaluated at its place
  in the authored direct/indirect list. The Regina fixture proves scalar,
  invalid-word, compound, list order, whole-stem, dropped list-variable,
  compound list-reference, exposed procedure and nested `IF`/`DO` cases.
  Raw tree inspection shows `LEVELC_DROP` and `VAR_REFERENCE`; neither remains
  in the lowered tree. The obsolete indirect negative fixture was replaced
  with optimized, no-opt and tree-shape regressions. There is no fixed list
  length in the helper.
- Focused Release and Debug compiler tests passed 6/6 each; pool opt/no-opt
  passed 2/2 in each build; RexxScript passed 4/4; and the normal Release
  Level C suite passed 287/287. The fixture compiled, assembled, linked and
  executed with byte-identical Regina output. Indirect-list classification
  currently uses the default BYTE profile; configured UTF8 propagation and
  platform proof remain open under `LC-AC-04/06`. Evidence:
  `/tmp/crexx-levelc-indirect-regina.out`,
  `/tmp/crexx-levelc-indirect-release-pool-rebuild.log`,
  `/tmp/crexx-levelc-indirect-debug-pool-rebuild.log`,
  `/tmp/crexx-levelc-indirect-release-pool-focused.log`,
  `/tmp/crexx-levelc-indirect-debug-pool-focused.log`,
  `/tmp/crexx-levelc-indirect-release-core-build.log`,
  `/tmp/crexx-levelc-indirect-debug-core-build.log`,
  `/tmp/crexx-levelc-indirect-release-focused.log`,
  `/tmp/crexx-levelc-indirect-debug-focused.log`,
  `/tmp/crexx-levelc-indirect-suite.log`,
  `/tmp/crexx-levelc-indirect-rexxscript.log`, and
  `/tmp/crexx-levelc-indirect-linked.m1vxRl/`.
