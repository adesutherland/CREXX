# Level C Classic REXX Working Architecture

Status: active Level C architecture design; the original parser design record
is retained below for implementation history
Last updated: 2026-10-03

This document is the working record for the Level C programme. Level C means
Classic REXX compatibility, using the current cREXX compiler front-end style:
re2c scanner, C parser glue, Lemon grammar, and validation/fixup walkers.

The first milestone is a Level C syntax highlighter through DSLSH. That
milestone parses and validates Level C source, builds the user-facing source
tree, emits diagnostics/highlighting, and stops before broad canonical lowering,
optimization, assembly, or VM execution. A deliberately narrow set of
runtime-backed lowering tracer slices is now enabled for ordinary compilation.

## Current architecture and product boundary

This is the architecture companion to the
[Release 1 Level C compatibility worklist](../../docs/planning/release-1/levelc-compatibility-worklist.md).
The [Release 1 plan](../../docs/release-1-plan.md) owns the completion target;
the worklist owns feature status, acceptance criteria, and increment evidence.
The compliance and BIF references own Classic semantics. Historical parser
exploration later in this document does not override these current contracts.

```text
Classic source -> Level C scanner/glue/grammar -> source tree + guarded AST
              -> Level C lowering -> canonical compiler AST
              -> rxc -> rxas -> rxlink -> rxvm
                                  |
                                  v
                      shared rxfnsc runtime
       RexxValue + RexxStem + RexxVariablePool + Classic BIFs
                                  ^
                                  |
RexxScript source -> separate sandbox evaluator and intrinsic allow-list
```

The Level C path uses `rxcpcscn.re`, `rxcpcpar.c`, and `rxcpcgmr.y` for Classic
syntax and `rxcp_levelc_lower.c` for guarded conversion. The neutral
`rxcp_remap_build.c` constructs canonical AST shapes; Classic policy stays in
the Level C lowerer. Accepted shapes pass through the ordinary compiler and
bytecode toolchain. Unsupported shapes fail closed. Source anchors from the
authored program must survive lowering for diagnostics and tracing.

`lib/rxfnsc` owns one shared Classic runtime foundation:

- `RexxValue` owns scalar representations, conversion, numeric and logical
  behavior. `RexxStem` owns explicit tails, dropped-tail tombstones and the
  Classic reset of all tails when a new stem default is assigned.
- `RexxVariablePool` owns Classic bindings, dropped state, case-preserving
  compound tail substitution and exposure. Its `stemSymbolValue` reads an
  unset or dropped tail without creating a binding; `dropStemTail` mutates one
  case-preserved tail and copies the updated stem back through a local or
  exposed binding. Direct Level C `DROP` lists lower to ordered `drop`,
  `dropStem`, or `dropStemTail` calls; the compiler evaluates each compound
  tail once before its corresponding call. The pool and `RexxStem` implement
  default reset, dropped-tail reads, and alias mutation. Each Level C
  activation uses its visible pool; a RexxScript evaluator creates a
  distinct sandbox pool from the same class.
- The pool's `dropSymbol` operation resolves and drops one validated Classic
  symbol, including a compound name's substituted tail. It is the shared
  primitive for runtime subsidiary lists in parenthesized `DROP` references.
  Level C lowers each `VAR_REFERENCE` to one `dropIndirectList` call with the
  referenced variable's value evaluated at that position in the direct list.
  The helper splits the captured string into words, uses the shared Classic
  symbol classifier, skips invalid words as Adrian chose for Regina parity,
  and drops valid names sequentially. The classifier currently uses the
  default BYTE profile; configured character-profile propagation is open.
- `RexxClassicBif*` modules own compatible BIF algorithms, argument validation,
  and error construction. `RexxBifCallContext` carries `RexxValue` arguments,
  argument-presence flags, caller pool, and Classic configuration. A BIF that
  needs context must receive the correct pool and configuration from its caller.

RexxScript remains a separate interpreted product. Its parser, statement
semantics, intrinsic allow-list, captured output, and host-exposure policy are
owned by `rexxscript/`. Its evaluator still keeps string-oriented expression
state and variable arrays while mirroring variables into its sandbox
`RexxVariablePool`. It materializes `RexxValue` arguments/results at the
shared BIF boundary. Sharing these classes does not make RexxScript a Level C
compiler or imply that every Classic instruction or BIF is available there.
The caller's host CREXX pool is never passed implicitly into a RexxScript BIF.

Supported direct local-procedure `ARG` bindings now use Classic
`PARSE UPPER ARG` behavior. The lowerer copies each caller argument into a
one-value `RexxBifCallContext`, supplies the callee's visible pool, calls the
shared standalone `rexxclassicbif_translate`, then sets the parsed target in
the callee pool. It adds the TRANSLATE import only for a programme with such
bindings. This uses the current Level C BYTE default; delivery of an opt-in
UTF8 call configuration remains an open compatibility obligation.

The executable `PARSE` slice accepts a `VAR` scalar source or a `VALUE`
expression with one nonempty template containing direct scalar or `.` targets,
static literal patterns, and static absolute or relative positions, with
optional `UPPER`. The parser keeps the outer comma-template list and inner
template as separate `TEMPLATES` nodes. The lowerer validates both levels,
evaluates and captures the source before any target write, and emits canonical
pool reads, result captures and ordered `setValue` calls. A dot consumes its
field without a pool write. `UPPER` passes the source through the same shared
Classic TRANSLATE frame used by `ARG`; its import is added only when a
supported uppercase parse exists. The authored `PARSE` and template nodes do
not survive the canonical boundary. No new AST or emitter node is required.

The lowerer counts the validated targets and calls
`RexxValue.parseWordTemplate(count)` once. The shared method returns one
`RexxValue` per item: the first `count-1` items receive words and the last
receives the unparsed tail. It chains the existing `parsewords3`-based
`parseWordAndRest()` primitive, with no compiler dispatch by item count.
Lowering captures the entire result vector before visible pool writes,
preserving source aliases and repeated target order. A dot skips only its
pool write, including for an all-dot template. A `PARSE VALUE` source
expression still executes once when all items are dots.

For static mixed templates, the lowerer walks the parser's existing ordered
`TARGET`, `PATTERN`, `ABS_POS` and `REL_POS` children. It inserts the
implicit word boundary between adjacent targets and the initial absolute
position when the template begins with a target, then serializes the whole
sequence to the VM's version-1 frozen `parseplan` descriptor. The descriptor
is a compiler-owned escaped-byte string constant, not source text interpreted
at runtime. The canonical `ASSEMBLER` node calls `parseplan` into a hidden
string result array; its operand validator requires a string constant, so a
binary AST constant is not accepted for this opcode. The lowerer snapshots
the source before the call, then converts and assigns captured non-dot fields
through the Classic pool in authored order. This path has no target-count or
item-order switch. Delimiter search, cursor movement, and result capture run
in the existing VM helper shared with the Level B PARSE exit. The AST rewrite
owns only validation, plan construction, source capture, and pool assignment.
Dynamic operands require version-2 references and an explicit capture plan;
comma templates require per-source segment handling. They extend the same
item representation instead of adding shape-specific lowering paths.

This path does not call the certified `compiler/exits/parse/Parse.crexx` exit.
That exit consumes tokens and generates Level B replacement code, including
direct `parsewords`/`parsepos2` operations and packed `parseplan` descriptors;
Level C already has a parsed template AST and writes through a separate
Classic variable pool. Future pattern and position work should assess reuse
of those lower-level operations or descriptor semantics while preserving
Level C AST ownership and pool writes. Other source types, dynamic patterns
and positions, and comma templates remain guarded.
The current default proof is BYTE; the broader binary/UTF8 configuration
obligation remains open.

The intended direction is one implementation of each overlapping Classic BIF
in `rxfnsc`, called by both products with product-specific dispatch and
capability policy. Changes to shared value, pool, or BIF behavior need focused
`rxfnsc` tests plus Level C and RexxScript integration checks when both paths
use the changed contract. Compatibility differences belong in the caller's
explicit adapter or configuration, not in duplicate BIF algorithms. The
[RexxScript developer guide](../../rexxscript/doc/developer-guide.md) describes
its evaluator and sandbox boundary.

The current executable Level C surface is a set of bounded lowering slices; the
complete Classic contract remains open in the worklist. No additional
language-direction decision is needed to continue an increment that follows
the references and this architecture. A new syntax rule, compatibility
exception, or change to these ownership boundaries still requires Adrian's
explicit decision under `AGENTS.md`.

The early AST closure path checks accepted lowered trees for parent/sibling
ownership and residual Classic-only nodes before canonical validation. The
first additional structural family is `SELECT`: guarded WHEN/OTHERWISE lists
become nested canonical IF blocks, with each later condition contained in the
earlier false arm. The shared `RexxValue` class supplies exact WHEN logical
validation; a small exported runtime helper reports the Classic no-match
identity. RexxScript's evaluator and sandbox contract remain separate.
The initial loop slice accepted `DO > REPEAT > FOR > INTEGER` with a
non-negative literal count representable by the canonical integer builder.
Later bounded slices extend this tree to dynamic counts, controlled loops,
setup-bearing conditions and named controlled-loop transfers.
Childless LEAVE/ITERATE now have a bounded path through those generated blocks:
each accepted source counted loop gets a hidden canonical control symbol, and
its source transfers become named canonical transfers to that symbol. This
uses the existing loop-association pass, which can search past generated
one-shot DO wrappers for a matching controlled loop. The ordinary `ast_do()`
rule remains unchanged. Named Classic transfers now resolve an active source
controlled variable to that loop's hidden canonical symbol, including an
outer target across nested generated blocks. Other loop forms remain open in
the worklist.
`DO FOREVER` is now the second accepted source repetition form. Its parser
`REPEAT` node has no count child; the neutral controlled-DO builder omits FOR
while retaining the hidden canonical control symbol used by LEAVE/ITERATE.
The conditional loop slices accept source `DO WHILE` and `DO UNTIL`. The
neutral builder places the canonical condition under REPEAT. WHILE checks at
loop entry; UNTIL checks after the body, including on ITERATE. Direct WHILE
and UNTIL accept supported expressions that lower to setup statements. The
lowerer places that setup and a final `LEAVE WITH` of the contextual logical
value in the existing canonical `BLOCK_EXPR`, so the whole condition runs at
each loop's required check point. A setup-free condition remains a direct
expression. The same hidden
control symbol carries source LEAVE/ITERATE across generated wrappers. Shared
`RexxValue.logicalWhileValue()` and `logicalUntilValue()` enforce Classic
`34.3` and `34.4` logical identities.
The combined-header slice accepts a non-negative literal count followed by a
supported WHILE or UNTIL condition. FOR and the condition share the same
canonical REPEAT. A condition with setup uses the same BLOCK_EXPR builder as
direct WHILE/UNTIL; a zero count skips both body and condition setup. WHILE
checks at entry while UNTIL checks after the body, including the final
count-limited body. Controlled variables remain fail-closed.
`DO FOREVER WHILE` and `DO FOREVER UNTIL` reuse the same condition lowering
and hidden loop target with no FOR child under their canonical REPEAT. This
preserves zero-entry WHILE, post-body UNTIL, and source LEAVE/ITERATE binding.
Direct and WHILE/UNTIL combined `DO expression` headers accept a supported
dynamic count expression. The
lowerer places its `RexxValue.repeatCountValue()` conversion under canonical
FOR and wraps any count setup in BLOCK_EXPR, so the expression runs once at
entry even if its source variables change in the body. A zero count skips
the condition and its setup as well as the body. This bounded slice
accepts non-negative mathematical whole values up to signed 32-bit maximum;
the contextual method reports `26.2` for invalid or out-of-range values.
Wider count values and controlled header forms beyond these bounded slices
remain open. Scalar controlled DO accepts a literal or supported-expression
start with optional literal or supported-expression TO, bounded literal or supported-expression
FOR, and optional signed literal or supported-expression BY. Clause order
does not change the loop guards; dynamic clause side effects retain written
order. The visible Classic pool is initialized before one
canonical REPEAT. TO adds a WHILE entry check against the current pool value;
BY selects the upper or lower bound comparison by its sign. FOR adds its
canonical count check before WHILE, if present, so zero count skips the body.
An UNTIL end-check BLOCK_EXPR advances the current pool value by BY or the
default one and returns false. It runs after ordinary bodies and ITERATE,
including the final count-limited body, while LEAVE skips the advance. The
hidden canonical loop assignment only satisfies the emitter and does not
replace the Classic control variable. Unsupported expression shapes remain
fail-closed. Without TO or FOR, the canonical
REPEAT has no synthetic count or entry guard; its pool-backed UNTIL end block
still advances the visible variable after ordinary or ITERATE paths. Source
LEAVE, WHILE and UNTIL provide the guarded exit paths. A supported dynamic TO
expression is evaluated once before the new visible control value is assigned,
so it can observe the prior pool value. Shared `RexxValue.controlToValue()`
checks `41.4` at setup and returns a copy stored in a hidden canonical local;
the WHILE TO check reads that local on every entry. This also runs when FOR
is zero. A supported dynamic BY expression follows the same capture path:
the parser's TO/BY clause list is walked in written order before assigning
the new control value, and `RexxValue.controlByValue()` validates `41.5`
and copies the step. Both the TO direction check and the UNTIL pool advance
read that hidden step, so body mutation cannot change it. A supported
dynamic FOR expression joins the same written-order header setup, with
shared `RexxValue.controlForCountValue()` enforcing bounded whole counts
and contextual `26.3` before the visible control assignment. Its hidden
canonical int feeds the REPEAT FOR child, so a zero count still validates
all header expressions while skipping body and WHILE/UNTIL checks. A dynamic
start is evaluated first and copied into a hidden canonical `RexxValue`.
The later TO/BY/FOR expressions still see the previous visible control value
and keep their written-order validation. Only after those clauses does
`RexxValue.controlStartValue()` validate contextual `41.6` and pass the
captured value to the visible pool assignment. This order follows Regina's
error and side-effect precedence without a new AST node. Wider count values
remain open.
For a supported controlled WHILE header, the same canonical REPEAT uses one
WHILE entry node. Its FOR check runs first; when TO exists, a source-anchored
BLOCK_EXPR branches on the TO guard before evaluating the Classic WHILE
expression or its setup. The selected branch returns its Boolean value with
LEAVE_WITH. With no TO guard, the source WHILE value occupies the entry node
directly or uses the ordinary setup BLOCK_EXPR. The existing UNTIL end block
still advances the visible control variable after each body and ITERATE.
For a supported controlled UNTIL header, the canonical UNTIL end-check uses
one source-anchored BLOCK_EXPR. It evaluates the Classic condition and its
setup after every body or ITERATE. A true result returns true with LEAVE_WITH
before the pool step; a false result advances the visible control variable
and returns false. This also runs after the final FOR-limited body, while a
zero FOR, a failed TO entry guard, or LEAVE skips the end check entirely.
Classic `&` and `|` now evaluate both operands in source order. The lowerer
copies the left `RexxValue` before any right-hand setup statements, then calls
shared contextual `logicalAnd()` or `logicalOr()` methods. The earlier
branch-based short-circuit tracer shape is historical and is no longer the
Classic Level C target. Exact left/right operand errors use `34.5`/`34.6`.
The parser represents an empty quoted Classic string with a zero-length
`STRING` node whose text pointer may be absent. Literal lowering now maps
that one valid shape to a source-anchored canonical empty string before
constructing the shared `RexxValue`; other nodes still require their text
payload. The normal emitter and runtime paths handle the resulting value.
The continuation-expression grammar also accepts function calls after a
blank-concatenated operand. It uses the name and opening-parenthesis token
positions to distinguish adjacent `name(`, emitted as the existing source
`FUNCTION` node, from `name (` with a blank, emitted as a symbol followed by
a parenthesized value under `OP_SCONCAT`. Both then use the existing canonical
expression and call lowering; no emitter-specific node is added.
For all three implicit-concatenation grammar paths, the parser now examines
the physical source gaps along the token chain between the AST operands.
Touching tokens, including a gap containing only nested comments, produce
the existing `OP_CONCAT`; whitespace produces `OP_SCONCAT`. The chain check
allows parentheses omitted from a grouped or call operand's AST but does not
infer abuttal across other omitted punctuation. This matters for the current
scanner's former `1.5` split into integer, dot and integer tokens: the dot
must not be silently lost while deciding abuttal. The bounded decimal-fraction
scanner slice now emits `1.5`, `1.`, and `.5` as one `TK_DECIMAL` token and
one source `DECIMAL` AST node. Level C lowering turns that node into the
original string spelling in a shared `RexxValue`, so Classic numeric methods
and contextual DO/FOR whole-number checks remain authoritative. Valid
exponent forms with integer or fractional mantissas also use that source
node; the lowerer uppercases their `E` when constructing the Classic value,
matching the reference display while retaining the numeric spelling in the
raw tree. Digit-starting nonnumeric constant symbols now take the existing
`CONST_SYMBOL` source AST path, including mixed letters and repeated periods.
Their literal value is uppercased before the shared `RexxValue` factory; they
never read the variable pool. The scanner gives complete numeric spellings
priority and emits a Level C-only raw token for the remaining digit-starting
symbols, which the parser and highlighter classify as a constant. Leading-
period symbols remain an open lexical and processor-state case.
Adrian permits new AST node types when they simplify the supported compiler
path through validation and emission. The existing WHILE, BLOCK_EXPR and
LEAVE_WITH nodes express the required timing and scope for direct WHILE/UNTIL
and count setup, so these increments add no new emitter shape.

## 2026-10-03 implementation review: simplify before expansion

This review covers the active Level C parser-to-canonical path, neutral AST
builders, `rxfnsc` value/pool boundary, VM PARSE operation, and registered
Level C tests. It is an architectural review of the active implementation,
not a claim that every Classic behavior has been qualified. The
[worklist](../../docs/planning/release-1/levelc-compatibility-worklist.md)
keeps the full compatibility obligation open.

The shared pool, BIF context, neutral remap builders, VM `parseplan`, and
post-lowering ownership check are useful foundations. The current code does
not consist solely of one-off cases. However, the programme's case-sized
increments have left duplicate paths and guards. More parser forms should
not be added until these paths are assessed against the existing tests.

| Finding | Evidence in the active path | Proposed simplification and proof |
| --- | --- | --- |
| LC-REV-01: statement dispatch is duplicated | `levelc_main_statement_supported` and `levelc_proc_statement_supported`, then `levelc_lower_main_statement` and `levelc_lower_proc_statement`, repeat most instruction cases | Use one statement validator and one lowerer with an explicit main/procedure context. Keep the genuinely different CALL, ARG, RETURN and EXIT rules visible. Prove identical nested behavior and diagnostics. |
| LC-REV-02: variable semantics cross the compiler/runtime boundary in several ways | The lowerer classifies scalar/stem/compound names and constructs tail lookups; the pool also has `resolveSymbolName`, `symbolValue`, `setSymbolValue`, and `dropSymbol`. Direct DROP still chooses `drop`/`dropStem`/`dropStemTail` in C while indirect DROP uses the pool's generic operation | First test direct DROP through `dropSymbol` against the existing order, substitution, exposure and dropped-tail cases. Audit reads and assignment separately: a compound assignment may need its tail captured before a side-effecting right-hand expression, so blindly replacing its prelude with `setSymbolValue` would change semantics. Give each required capture one owner. |
| LC-REV-03: PARSE has two result paths | Direct word templates call `RexxValue.parseWordTemplate`; mixed static templates serialize a descriptor for VM `parseplan`. Both paths capture results and emit ordered pool writes independently | Assess one template item representation and one VM/runtime execution path for all templates, including word-only cases. Retain the single source capture and ordered writes. Prove whitespace, dots, aliases, repeated targets, patterns, positions, BYTE/UTF8 and Level B exit parity before removing a path. Existing public runtime methods need a consumer/API check before removal. Dynamic operands and comma templates belong in this whole-instruction design. |
| LC-REV-04: DO lowering has accumulated shape decisions | `levelc_do_supported` validates a header, while `levelc_lower_do` repeats positional AST interpretation and constructs captured header values, synthetic conditions, `BLOCK_EXPR` and hidden loop targets | Normalize the parsed header once into a checked loop description, then lower from that description. Compare the resulting canonical AST with a dedicated node/emitter route for the hard controlled forms. Choose the route with fewer invariants and less total code while preserving optimizer and source behavior. This is an architectural decision gate. |
| LC-REV-05: BIF construction is generic but reachability is narrow | The shared argument-frame builder and `RexxBifCallContext` avoid per-BIF AST machinery; the compiler currently admits only a small BIF subset despite the larger runtime inventory. `rexxclassicbif_call` is documented as a deprecated compatibility dispatcher | Keep the common call frame and product-specific allow-lists. Use one compiler signature/context table to select direct BIF entry points after argument and error semantics are proved; do not add one compiler branch per BIF or expand the deprecated name dispatcher. |
| LC-REV-06: functional tests are stronger than structural assertions | The Level C suite contains reference-oriented, opt/no-opt, negative and tree probes. The production verifier checks parent ownership, sibling cycles and residual source-only nodes. Many tree CTests assert only that named nodes occur in debug output | Preserve all existing cases. Before aggressive AST refactoring, add a small number of decisive assertions for association targets, source anchors, generated scope/symbol ownership and evaluation order at the riskiest trees. Keep runtime equivalence and linked-image proof. Do not multiply tests merely to mirror implementation details. |
| LC-REV-07: plan/evidence drift | The worklist has separate completed criteria for PARSE target counts and DO header variants, while whole-instruction obligations remain open. Some reference-obligation rows still describe older bounded states | Treat those criteria as historical evidence. Reconcile the reference appendix and test manifest against the current code, then track future delivery by complete instruction. |
| LC-REV-08: SAY output loses embedded NUL | The VM `SAY` handlers have byte lengths, but `rxvm_mprintf` and `say_exit_func(char *)` turn the output into a terminated C string. The current Level C toolchain writes `41 0a` for `SAY '410042'x`, while Regina writes `41 00 42 0a`. | Pending Adrian's host API approval, retain the legacy callback and add a length-aware per-context callback plus an internal byte-span output route. Use the existing length-aware console writer for default output; prevent silent truncation on legacy custom callbacks. Prove BYTE/UTF8, host isolation, errors and linked delivery before closing SAY. |
| LC-REV-09: BIF validation errors are dropped | The shared `RexxBifCallContext` stores error code and message, but Level C lowering uses the returned blank `RexxValue` without a condition check. `SAY SUBSTR('abc', 0)` therefore prints a blank line and continues, while Regina stops with `40.14`. | Design one shared call-result/condition bridge for direct BIF entries, retaining the generic argument frame and current activation pool/configuration. Do not add error checks per BIF AST shape. Prove positive/error cases, source positions and RexxScript isolation before broadening compiler reachability. Architecture approval is required for the selected bridge. |
| LC-REV-10: variable reads retain a compiler-only shape limit | `levelc_variable_value_supported` rejects bare stems and multi-component compound tails; `levelc_pool_value` also splits compound reads into compiler-built stem/tail calls. `RexxVariablePool.symbolValue` already handles general validated symbols and missing bindings. | Pending Adrian's ownership approval, route all Level C value reads through the pool's generic method and remove the redundant read guard. Keep assignment's pre-RHS tail capture and evaluate direct DROP separately. Prove substitution, case, exposure, missing/stem defaults, errors and source anchors against Regina in opt/no-opt and linked execution. |

The review proposes these changes; it does not approve a new AST node, remove
an exported runtime method, or alter a language rule. Adrian's approval is
required for an architectural shift under `AGENTS.md`. The agreed delivery
unit is now a whole instruction at minimum, with one active instruction at a
time. Individual examples remain regression cases. Each instruction receives
a complete form/semantics review and path simplification before additions;
its multi-commit implementation remains open until every required form and
lifecycle obligation is proved or an exception is individually approved.
"Infeasible" is reserved for a fundamental feature or capability whose
delivery would require a serious compromise elsewhere. Significant work or
missing infrastructure is not infeasibility. A genuine conflict is recorded
with evidence, alternatives and proposed visible behavior; it is not an
automatic exclusion or permission to advance the queue.

The latest retained Level C run passed 290/290 CTests in 31.04 seconds of wall
time (738.13 CPU-seconds across the parallel tests). The test count is useful
coverage, but repeating the whole run after each case-sized edit compounds
build and QA cost. The registered runtime tests compile with `rxc`, assemble
with `rxas` and execute with `rxvm`; linked-image proof has been run separately
for individual increments. Preserve that distinction in future receipts.
For code commits, build the affected core product, run focused tests and the
relevant normal correctness suite once for the qualified input revision;
reserve full Level C sweeps for changes with broad lowering impact and
instruction integration checkpoints. Do not re-run unchanged valid evidence.

## Historical design record

The numbered sections below capture the earlier parser and lowering design
work. Prospective wording and open approval questions there describe their
time of writing; use the current architecture above and the live worklist for
present implementation state.

For the implemented highlighter contract and the forward plan into canonical
lowering, see `compiler/docs/levelc_syntax_highlighting.md`.

## 1. Source Material Reviewed

- Repository instructions: `AGENTS.md`
- Current compiler architecture:
  - `docs/ai-context/CREXX_ARCHITECTURE.md`
  - `compiler/docs/parsing_pipeline_anatomy.md`
  - `compiler/docs/dslsh_integration.md`
  - `compiler/docs/validation_logic_map.md`
- Lemon keyword fallback:
  - `docs/lemon.md`
  - `lemon/lempar.c`
  - `lemon/lemon.c`
- Current Level B front end:
  - `compiler/rxcposcn.re`
  - `compiler/rxcpopgr.y`
  - `compiler/rxcpbscn.re`
  - `compiler/rxcpbpar.c`
  - `compiler/rxcpbgmr.y`
  - `compiler/rxcp_val_check.c`
  - `compiler/rxcp_val_orch.c`
  - `compiler/rxcp_source_tree.*`
  - `compiler/rxcp_highlight_controller.c`
- Tree surgery support:
  - `compiler/rxcp_ast_rewrite.h`
  - `compiler/rxcp_ast_rewrite.c`
- Existing Level C grammar note:
  - `docs/books/crexx_vm_spec/Level-c-Grammar.tex`
- Classic language-specification source:
  - Publicly available Classic REXX language specification supplied by the
    user.
  - Syntax, evaluation, execution, configuration, and diagnostic guidance
    extracted into `compiler/docs/levelc_compliance_reference.md`.
  - Error-message catalog normalized into
    `compiler/docs/levelc_standard_error_messages.md`.

This draft deliberately excludes built-in function definitions. The extracted
implementation-facing BIF reference is now
`compiler/docs/levelc_classic_bifs.md`.

### 1.1 External compatibility probe: Regina REXX

Regina REXX is installed locally as `/opt/homebrew/bin/rexx` and is useful as a
quick external Classic REXX syntax reference while building the Level C parser.
It is not a substitute for the public language-specification source, but it
gives a living interpreter baseline for representative syntax questions.

Trial run on 2026-05-10:

```rexx
if = 'keyword-var'
say if
```

Command:

```sh
/opt/homebrew/bin/rexx keyword_variable.rexx
```

Result:

```text
keyword-var
```

Interpretation: Regina accepts `IF` as a variable name when assignment context
makes it a variable. This confirms the working assumption that Level C must not
reserve instruction words globally.

Negative `THEN` trial:

```rexx
if then then say 'bad'
```

Result:

```text
Error 64 running "then_bad.rexx": [Syntax error while parsing]
Error 64.1: [Syntax error at line 1]
```

Interpretation: Regina aligns with the Classic `THEN` concern in this document:
after `IF`, `THEN` is a condition terminator, not a condition variable recovered
through broad fallback.

## 2. Current Front-End Facts

The current compiler front end is already close to the intended Level C shape:

1. Options pre-scan detects language level and source options.
2. re2c scanner tokenizes source text.
3. Parser glue reshapes the token stream before Lemon sees it.
4. Lemon grammar builds a raw AST.
5. Early walkers reshape the AST, compute source spans, validate syntax-level
   constraints, and build the immutable `SourceNode` tree.
6. Parser mode serializes DSLSH from `context->source_tree`, not from the later
   mutable work tree.

Important current limitations for Level C:

- `rxcpopgr.y` already knows `LEVELC`, and default headerless source currently
  defaults to Level C in the option pre-scan when no CLI default is supplied.
- `rxc_highlight_controller_parse()` currently forces `context->level = LEVELB`.
  A Level C highlighter will need parser-mode level selection rather than this
  hard-coded value.
- `rxcpbscn.re` tokenizes many words as fixed Level B keyword tokens. That is
  incompatible with Classic REXX, where words are contextual keywords rather
  than general reserved variable names.
- `rxcpbgmr.y` contains Level B-only structures and error recovery around typed
  definitions, classes, interfaces, arrays, object calls, and reserved keyword
  assignment. Level C should not reuse that grammar directly.

## 3. Programme Shape

### 3.1 Milestone 1: Level C DSLSH syntax highlighter

Goal:

- Parse Classic REXX syntax.
- Validate syntax and syntax-adjacent structural rules.
- Build a source tree and project DSLSH.
- Stop before semantic lowering to Level B/canonical form.

Out of scope for milestone 1:

- BIF semantics.
- Runtime execution.
- Full tree surgery into canonical Level B shape.
- Code emission.
- Native compatibility APIs beyond the syntax needed to expose host-variable
  anchors in editor diagnostics.

### 3.2 Milestone 2: Canonical tree lowering

After a Level C program is valid, lower it by tree surgery into the compiler's
canonical tree shape. This should look like normal validated work-tree input to
later compiler stages.

Candidate lowering rule:

- Keep the Level C source tree as authored.
- Convert the Level C work tree into canonical calls, control-flow nodes, and
  variable-pool operations.
- Preserve source provenance so diagnostics and source-step metadata still point
  at authored Classic REXX clauses.

The first executable lowering slice is documented in
`compiler/docs/levelc_remapping_target.md` under "First Level C Lowering
Slice". The tracer now opens pool setup, scalar and simple compound
assignment/read, `RexxValue` literals, BIF frames, direct local call/procedure
argument/return shapes, stem exposure, and the parsed Classic expression
operator family. Short-circuit `&` and `|` lower through generated one-shot
branch blocks rather than eager method calls. Unsupported Level C inputs still
fail closed on the existing unsupported diagnostic.

### 3.3 Milestone 3: Runtime semantics

Once canonical lowering exists, implement execution support through Level B
runtime helpers and/or targeted VM support. This includes variable pools,
compound variable derivation, condition state, `INTERPRET`, and full procedure
pool/expose behaviour.

## 4. Proposed File-Level Architecture

Names are provisional:

| Concern | Current Level B | Proposed Level C |
| --- | --- | --- |
| Options pre-scan | `rxcposcn.re`, `rxcpopgr.y`, `rxcpopar.c` | reuse, with Level C defaults reviewed |
| Scanner | `rxcpbscn.re` | `rxcpcscn.re` |
| Parser glue | `rxcpbpar.c` | `rxcpcpar.c` |
| Grammar | `rxcpbgmr.y` | `rxcpcgmr.y` |
| Source shaping | `rxcp_val_check.c` shared walkers | shared plus Level C-specific walkers |
| Validation orchestration | `rxcp_val_orch.c` | split or dispatch by language level |
| DSLSH projection | `rxcp_highlight_controller.c` | shared, level-aware parse entry |
| Tree surgery | `rxcp_ast_rewrite.*` | harden and extend before canonical lowering |

The exact names should be decided when implementation begins. The important
architecture point is that Level C should have its own scanner/glue/grammar so
Classic REXX contextual keyword rules do not destabilize the Level B grammar.

## 5. Four Stage Parsing Contract

### Stage 0: options pre-scan

Existing behaviour:

- `opt_pars()` scans leading `OPTIONS`.
- It sets `context->level`.
- It can set comment and numeric standard flags.
- If no level is supplied, current logic defaults to Level C unless a CLI
  default is present.

Level C needs:

- Confirm whether Level C default numeric mode should be Classic. The public
  language-specification source gives `NUMERIC DIGITS 9` as the arithmetic
  default; current cREXX context defaults to common numeric mode and 18 digits
  for Level B behaviour.
- Decide whether Level C accepts only Classic block comments by default or also
  preserves cREXX optional line comment modes for compatibility.
- Parser mode must keep the level selected by options/CLI instead of forcing
  Level B.

### Stage 1: scanner

The Level C scanner should be closer to the Classic lexical level than the
Level B scanner.

Requirements from the public language-specification source:

- Source characters are categorized as syntactic characters, extra letters,
  other blank characters, other negators, and other characters.
- Required syntactic characters include letters, digits, blank, `_`, `!`, `?`,
  quote characters, arithmetic/comparison/operator characters, parentheses,
  comma, colon, semicolon, period, and vertical bar.
- Comments are nested `/* ... */` block comments and are not tokens.
- A comma followed only by blanks/comments and then EOL is a continuation, not
  an end of clause, provided it is not immediately before end of source.
- EOL supplies a semicolon token to the top-level syntax.
- Strings can be single-quoted or double-quoted; doubled delimiters inside a
  string represent one delimiter character.
- Hex and binary strings are quoted strings followed by `X`/`x` or `B`/`b`
  where the suffix is not followed by a letter, digit, or period.
- General symbols start with a general letter and continue with general
  letters, digits, or periods.
- Constant symbols start with a digit or period and continue with symbol
  characters, including exponent sign handling for numbers.
- Symbol text is uppercased by the configuration before syntax use.
- Some compound operators can contain blanks between their characters, such as
  `| |`, `/ /`, `* *`, `\ =`, comparison pairs, and strict comparisons.

Scanner design rule:

- Do not make Classic REXX instruction words globally reserved at scanner time.
  Prefer generic symbol tokens plus contextual promotion in glue/parser logic.

### Stage 2: parser glue

Level C glue is where the Classic interaction between lexical and top syntax
belongs. This is more than newline-to-semicolon conversion.

Required glue behaviours:

- Convert EOL to semicolon/end-of-clause.
- Remove continuation comma plus following EOL from the token stream.
- Collapse redundant end-of-clause tokens where safe.
- Insert an end-of-clause token before recognized `THEN`, after recognized
  `THEN`, `ELSE`, and `OTHERWISE`, and after labels as required by the
  specification's top-level interaction rules.
- Classify contextual keywords based on parser context, not spelling alone.
- Pass a preceding symbol as `VAR_SYMBOL` when an `=` can be assignment in the
  current top-level context.
- Pass an operand followed by colon as `LABEL` when labels are permitted.
- Insert a synthetic `VALUE` token before `+`, `-`, `\`, or `(` in contexts
  where `VALUE` may appear, except after `PARSE`.
- Infer concatenation:
  - blank concatenation when there was blank before the next operand;
  - `||` concatenation when no blank exists and the grammar permits it;
  - no inferred operator for a left parenthesis immediately following an
    operand when that parenthesis forms a function call.

This suggests a Level C token adapter rather than a pure scanner. The adapter
can keep clause context, parser-context hints, lookahead, and blank-presence
metadata without forcing re2c to become a parser.

### Stage 3: Lemon grammar

The Level C Lemon grammar should mirror the Classic top syntax, but be shaped for
LALR parsing and recovery. It should own:

- Program, clause, label, and null-clause structure.
- Instruction grouping:
  - `DO`
  - `IF`
  - `SELECT`
- Single instructions:
  - `ADDRESS`
  - `ARG`
  - assignment
  - `CALL`
  - command expression
  - `DROP`
  - `EXIT`
  - `INTERPRET`
  - `ITERATE`
  - `LEAVE`
  - `NOP`
  - `NUMERIC`
  - `OPTIONS`
  - `PARSE`
  - `PROCEDURE`
  - `PULL`
  - `PUSH`
  - `QUEUE`
  - `RETURN`
  - `SAY`
  - `SIGNAL`
  - `TRACE`
- Parse templates.
- Expression precedence and operators.
- Error productions that keep a usable partial AST for editor feedback.

Important grammar notes:

- Classic labels are clauses made from a symbol or literal followed by a colon.
- A command is just an expression clause whose evaluated string is sent to the
  current environment.
- `IF`/`ELSE` association follows nearest valid prior `IF`.
- `SELECT` requires at least one `WHEN` and may include `OTHERWISE`.
- `END` may include an optional variable symbol for `DO`/`SELECT` validation.
- `PROCEDURE` is an instruction at the start of an internal routine, not a
  Level B procedure definition header.

#### Lemon `%fallback` for contextual keywords

The local Lemon documentation and template confirm that `%fallback` supports
exactly the SQL-style use case where a keyword token can be retried as an
identifier if the keyword form would otherwise be a syntax error.

Syntax shape:

```lemon
%fallback TK_VAR_SYMBOL TK_IF TK_THEN TK_SAY TK_DO TK_END ...
```

Generated-parser behaviour:

- Lemon emits a `yyFallback[]` table when the grammar contains `%fallback`.
- On a lookahead miss in the current state, the parser changes the lookahead
  token type to the fallback token and retries the parse action before raising
  a syntax error.
- The original token payload is preserved; only the token code changes.
- Fallback is one step only; Lemon asserts that fallback chains terminate.
- `ParseFallback(token)` is available in generated parsers.

Throwaway PoC result against the bundled Lemon binary:

- With no `IF` statement production, `IF = THEN ;` can parse as
  `ID = ID ;` through fallback.
- `SAY IF ;` can parse as a `SAY` instruction whose operand falls back to `ID`.
- When an actual `IF` statement production exists, `IF = THEN ;` fails:
  the parser shifts the first `IF` as a valid keyword at clause start, and
  Lemon does not backtrack that already-shifted token to `ID` when the later
  `=` makes the `IF` production invalid.

Implementation conclusion:

- `%fallback` is worth a Level C PoC and may remove a large number of
  keyword-as-identifier recovery productions.
- `%fallback` is not enough by itself for full Classic REXX contextual keyword
  behaviour. The Level C glue still needs assignment/context recognition before
  keyword promotion for cases such as a keyword-spelled variable at clause
  start.
- A practical design is likely hybrid:
  - scanner emits symbol tokens with spelling and spacing metadata;
  - glue promotes symbols to hard contextual keyword tokens where the Classic
    interaction rules require a keyword;
  - glue forces `TK_VAR_SYMBOL` where assignment, label, expression, template,
    or command context requires a variable symbol;
  - Lemon `%fallback`, if retained, covers only the remaining candidate keyword
    cases that are proven safe by a small grammar PoC.

Special caution for `THEN`:

- The public language-specification source gives `THEN` a stronger context rule
  than ordinary keywords:
  after `IF` or `WHEN`, a symbol spelled `THEN` is a keyword wherever a
  `VAR_SYMBOL` would be part of the immediately following expression.
- That rule lets `THEN` terminate the condition expression and also makes cases
  such as `if then then ...` a syntax error rather than a condition variable
  named `THEN`.
- Lemon `%fallback` is global and state-based. If `TK_THEN` globally falls back
  to `TK_VAR_SYMBOL`, then in some expression states after `IF`/`WHEN` Lemon can
  incorrectly retry `THEN` as an identifier because the expression parser is
  looking for a term.
- Therefore `THEN` should either be excluded from the global fallback set, or
  classified by the glue layer so it is never fallback-eligible while parsing
  the condition after `IF`/`WHEN`.
- The current Level B grammar's `if ::= TK_IF expression ncl0 then` and
  `then ::= TK_THEN ncl0 instruction` is structurally close to the specification's
  `IF expression [ncl] THEN ncl instruction`, but Level C still needs explicit
  handling for the specification's inserted semicolon before and after recognized
  `THEN` so clause spans/tracing/highlighting remain faithful.

### Stage 4: validation and fixup walkers

Milestone 1 walkers should validate structure and annotate source nodes without
lowering to executable canonical form.

Required syntax-adjacent validation:

- `END` with a control variable must match the control assignment in the
  corresponding repetitive `DO`.
- `LEAVE` and `ITERATE` must target an enclosing repetitive `DO`; optional
  labels must match a suitable control variable.
- `PROCEDURE` must occur only where Classic REXX permits it, effectively first
  in an internal routine after routine initialization.
- `DROP` and `PROCEDURE EXPOSE` variable lists must validate parenthesized
  indirect names as variable-list syntax.
- `PARSE` templates must validate targets, variable references in patterns, and
  positional forms.
- `INTERPRET` syntax can be parsed, but milestone 1 should report only static
  syntax for the containing source, not execute interpreted text.
- Dangling `ELSE` should associate with the nearest valid `IF`.
- Clause line numbers and source spans must remain anchored to authored
  clauses, including inferred semicolons.

## 6. Classic REXX Syntax Extraction

This section summarizes syntax/parsing requirements from the public
language-specification source. It is an implementation extraction, not a
verbatim copy of the source.

### 6.1 No general reserved words

Classic REXX has contextual keywords, not a global reserved-word set.

The language specification defines a keyword as a token with special meaning only when its
spelling is recognized in a particular context. Otherwise a symbol with that
spelling remains a variable symbol. The only "reserved symbols" described in
the syntax chapter are constant symbols starting with period:

- `.MN`
- `.RESULT`
- `.RC`
- `.RS`
- `.SIGL`

Any other non-number constant symbol starting with period is a syntax error in
the standard grammar.

Implementation consequence:

- Level C must allow variables such as `say`, `if`, `then`, `do`, or `parse`
  when context makes them variables, especially assignment left-hand sides.
- Highlighting should color such words by actual syntactic role, not spelling.
- Scanner-level keyword tokens are unsafe unless glue can demote them reliably.

### 6.2 Clauses, labels, and source positions

- A program is a sequence of clauses.
- A semicolon may be explicit or inferred from EOL or from standard token
  insertion around control keywords.
- A null clause has no tokens.
- A label clause is a symbol or literal followed by `:`.
- Labels are followed by an implicit semicolon in the syntax stream.
- The line number of a clause is based on the number of EOL events before the
  first token of the clause.
- Trace-only labels inside grouping instructions exist in the syntax model even
  if they are not normal transfer targets.

Implementation consequence:

- The source tree needs clause containers or stable statement source spans so
  DSLSH can fold/report by clause even where semicolons are inferred.
- Label handling should be part of Level C glue/parser interaction, not a
  fixed scanner classification based only on `symbol:`.

### 6.3 Lexical tokens

Token categories:

- operands: string literals, variable symbols, constant symbols/numbers
- operators
- special characters: comma, colon, semicolon, parentheses

String forms:

- single-quoted and double-quoted strings
- doubled delimiters inside strings
- binary and hexadecimal quoted strings with suffixes
- unterminated comments and strings are syntax errors

Symbol forms:

- `VAR_SYMBOL`: starts with a general letter and may contain periods.
- `CONST_SYMBOL`: starts with a digit or period.
- numbers are a subset of constant symbols.
- non-number constant symbols beginning with a period are restricted to the
  reserved-symbol list.

Operator forms:

- arithmetic: `+`, `-`, `*`, `/`, `%`, `**`
- integer/remainder form: `//`
- concatenation: blank and `||`
- logical: `&`, `|`, `&&`, prefix `\`
- normal comparisons: `=`, `\=`, `<>`, `><`, `>`, `<`, `>=`, `<=`, `\>`, `\<`
- strict comparisons: `==`, `\==`, `>>`, `<<`, `>>=`, `<<=`, `\>>`, `\<<`
- some multi-character operators permit blanks between their characters.

### 6.4 Expressions

Expression grammar, highest to lowest:

1. terms: symbol, string, function call, parenthesized expression
2. prefix operators: `+`, `-`, `\`
3. power: `**`
4. multiplication family: `*`, `/`, `//`, `%`
5. addition family: `+`, `-`
6. concatenation: blank and `||`
7. comparisons
8. logical AND: `&`
9. logical OR: `|` and `&&`

Extraction notes:

- Function calls use a taken constant followed by `(` argument list `)`.
- The leftmost function-name component must not end with a period.
- Level C uses Classic numeric syntax by default in the parser path:
  `context->numeric_standard = 1`, Classic `%` integer divide, Classic `//`
  remainder, and Classic `**` left associativity.
- Classic prefix `-` is higher priority than `**`. This matches the existing
  Level B `NUMERIC_CLASSIC` scanner/parser shape: the scanner returns the
  high-priority minus token and the grammar places it in the prefix level above
  power.
- The first expression-core implementation covers symbols, integer and string
  terms, parenthesized expressions, prefix `+`/`-`/`\`, power, multiplication,
  integer division, remainder, addition/subtraction, explicit and blank
  concatenation, all normal comparisons, all Classic strict comparisons, `&`, `|`,
  and `&&`.
- `&&` is accepted at the Classic logical OR/exclusive-OR precedence level and
  maps to the shared `OP_XOR` logical-expression node. The node is distinct from
  `OP_COMPARE_NEQ` so boolean/logical targeting can happen before code emission
  or future Level C tree surgery.
- Function calls, omitted argument lists, compound/stem references, number
  forms beyond integers, and expression-level runtime validation remain later
  slices.
- Expression lists support omitted expressions around commas.
- A comma or unmatched right parenthesis at expression level is a syntax error.
- The Classic grammar's power expression is left-recursive. Current Level B
  already has separate left/right power handling based on numeric standard, so
  Level C must choose the Classic rule explicitly during implementation.

### 6.5 Program and instruction structure

Single instruction families:

| Instruction | Syntax shape to parse |
| --- | --- |
| assignment | variable symbol, `=`, expression |
| command | expression as a clause |
| `ADDRESS` | optional environment/command/value expression plus optional `WITH` connection |
| `ARG` | optional parse template list |
| `CALL` | taken constant plus optional expression list, or `ON`/`OFF` condition form |
| `DROP` | variable list |
| `EXIT` | optional expression |
| `INTERPRET` | expression |
| `ITERATE` | optional control symbol |
| `LEAVE` | optional control symbol |
| `NOP` | no operands |
| `NUMERIC` | `DIGITS`, `FORM`, or `FUZZ` forms |
| `OPTIONS` | expression |
| `PARSE` | optional `UPPER`, parse type, optional template list |
| `PROCEDURE` | optional `EXPOSE` variable list |
| `PULL` | optional parse template list |
| `PUSH` | optional expression |
| `QUEUE` | optional expression |
| `RETURN` | optional expression |
| `SAY` | optional expression |
| `SIGNAL` | condition control, `VALUE` expression, or target constant |
| `TRACE` | optional constant or `VALUE` expression |

Grouping instruction families:

- `DO`
- `IF`
- `SELECT`

### 6.6 DO

`DO` supports:

- simple grouping
- counted expression repetition
- control assignment repetition
- `TO`, `BY`, and `FOR` count modifiers
- `WHILE` and `UNTIL` conditions
- `FOREVER`, optionally with a condition
- optional variable after `END`

Implementation consequences:

- The parser should retain a distinct raw `DO` shape for simple grouping versus
  repetitive loops.
- Validation must associate `LEAVE` and `ITERATE` with repetitive `DO`, not
  just any `DO` group.
- The optional `END name` validation is semantic/structural and belongs in a
  walker.
- Canonical lowering must not optimize the loop control variable into a typed
  Level B local if doing so bypasses Classic variable-pool semantics.

### 6.7 IF

`IF` syntax:

- `IF` expression, optional end-of-clause, `THEN`, instruction
- optional `ELSE`, instruction

Implementation consequences:

- `THEN` and `ELSE` participate in token insertion around semicolons.
- `ELSE` binds to the nearest valid previous `IF`.
- The THEN/ELSE instruction may be a group or a single instruction.

### 6.8 SELECT

`SELECT` syntax:

- `SELECT`, end-of-clause, select body, `END`
- select body requires `WHEN` clauses and may include `OTHERWISE`
- each `WHEN` has an expression and `THEN` instruction
- `OTHERWISE` has an optional instruction list
- `END` may carry an optional variable symbol that is invalid for `SELECT`
  according to the Classic diagnostic rules

Implementation consequence:

- Current Level B `SELECT expression` switch shape is not Classic REXX syntax.
  Level C should parse Classic `SELECT` separately and lower later if needed.

### 6.9 ADDRESS

Syntax extraction only:

- `ADDRESS` can be bare, can name an environment, can include a command
  expression, or can use `VALUE expression`.
- `WITH` introduces connection redirection syntax.
- Connections include `INPUT`, `OUTPUT`, and `ERROR`.
- Resources include `STREAM`, `STEM`, and `NORMAL`; output/error can use
  `APPEND` or `REPLACE`.

Implementation consequences:

- The current certified exit path is the likely owner for execution lowering.
- For parsing/highlighting, Level C needs explicit syntax recognition for the
  Classic redirection shape, but milestone 1 does not need to implement command
  execution.
- Host-variable anchors such as `:name` and `${name}` are current compiler
  auto-expose syntax for ADDRESS handlers, not Classic command syntax. Keep that
  distinction visible in future docs.

### 6.10 PARSE and templates

Parse forms:

- `PARSE ARG`
- `PARSE PULL`
- `PARSE SOURCE`
- `PARSE LINEIN`
- `PARSE VERSION`
- `PARSE VALUE [expression] WITH`
- `PARSE VAR variable`
- optional `UPPER`
- optional template list

Template components:

- targets: variable symbol or `.`
- literal patterns: strings
- variable patterns: parenthesized variable symbol
- absolute positionals: numbers or `= position`
- relative positionals: `+ position` or `- position`
- templates are comma-separated and can contain omitted templates

Implementation consequences:

- Parse templates are a syntax sublanguage. They should not be parsed as normal
  expressions after the parse type.
- Highlighting should distinguish template targets from expression variables.
- Lowering must write through the Classic variable pool map.

### 6.11 Variable lists

Variable-list syntax is used by `DROP` and `PROCEDURE EXPOSE`.

Forms:

- direct variable symbols
- parenthesized variable references for indirect lists

Validation/lowering consequences:

- Direct variable names can be stem names or compound names.
- Parenthesized references evaluate one variable at their place in the source
  list; the shared pool splits its captured value into words. Each valid word
  is resolved against the then-visible pool, uppercasing source spelling while
  preserving substituted tail text case, and dropped in order. Invalid words
  are ignored for Regina parity. Other variable-list consumers, including
  `PROCEDURE EXPOSE`, retain their own open lowering work.

## 7. Canonical Lowering Requirements

The central rule for Level C lowering:

Classic REXX variables are string values in a variable-pool model, not Level B
typed locals.

Canonical lowering uses the shared `RexxVariablePool` runtime surface through
hidden pool references and method calls. The Level C
compiler must not silently map ordinary Classic variables to Level B locals
unless the transformation preserves:

- case normalization by `Config_Upper`
- dropped/not-dropped state
- implicit and explicit stem behaviour
- derived-name evaluation for compound variables
- reserved-symbol pool-zero access
- `PROCEDURE` pool creation
- `PROCEDURE EXPOSE` and stem exposure
- `DROP` semantics
- ADDRESS host-variable binding and write-back behaviour
- `PARSE` assignment semantics
- `INTERPRET` access to the active variable pool

### 7.1 Variable pool map

Each Level C routine needs an active variable pool. Suggested compiler model:

- Attach a `LEVELC_POOL` or equivalent sidecar to routine/source scopes.
- Represent every variable reference as a pool access in the work tree.
- Track whether the authored syntax is direct, stem, compound, indirect, or
  reserved-symbol access.
- Keep source-tree semantics simple for the highlighter: identifier ownership
  and identifier ids should refer to authored names even when the work tree
  later becomes helper calls.

### 7.2 Loop control map

The Classic execution model uses loop state for repetitive `DO`, including the
control variable identity, iterate target, once target, leave target, repeat
count, by/to/for values, and nesting correction for labelled `LEAVE`/`ITERATE`.

Compiler rule:

- Maintain explicit loop associations in the AST, similar to current `DO`
  association handling.
- Keep a Level C loop-control sidecar for validation and future lowering.
- Loop control variable updates must go through the variable-pool map.

### 7.3 Tree surgery hardening

Before executable lowering, `rxcp_ast_rewrite.*` probably needs hardening:

- replacement of one node with zero, one, or many sibling nodes
- batch replacement inside instruction lists
- source provenance propagation for synthetic helper calls
- safe movement of child lists while preserving source-owned diagnostics
- symbol decoupling for reused nodes, including future Level C pool symbols
- validation helpers for parent/child/sibling consistency after rewrite
- debug stress checks similar to existing fixed-point idempotency checks

## 8. DSLSH Requirements

Milestone 1 should reuse the current DSLSH architecture:

- Parse into AST.
- Run Level C source-shaping and syntax-validation walkers.
- Build `context->source_tree`.
- Sync diagnostics onto source-owned state.
- Project source tree to DSLSH.

Additional Level C highlighting needs:

- Contextual keyword coloring.
- Label coloring.
- Clause-level source spans.
- Parse-template target coloring.
- Compound variable/stem segment projection.
- Distinguish reserved symbols from ordinary variables.
- Correct comment recovery for nested block comments.
- Correct inferred semicolon ownership for diagnostics.
- Standard diagnostic text and error numbers from
  `levelc_standard_error_messages.md` through the shared message catalogs in
  `messages/`.

Level C diagnostics should be emitted in two layers:

1. The parser and validators emit a stable diagnostic identity plus any
   structured inserts through the common `RxcpDiagnostic` payload. The legacy
   `node_string`/source diagnostic message is only the final fallback.
2. The common renderer maps the identity and inserts to localized message text.
   Normal CLI/DSLSH output defaults to localized rendering; golden tests force
   `CREXX_DIAGNOSTICS=raw`.

The raw rendered form is:

```text
RXC-LC-<standard-code> [insert-name="escaped value" ...]
```

Level C warning identities use the same prefix and are carried on WARNING
nodes rather than ERROR nodes. The first non-standard warning identity is
`RXC-LC-IMPLICIT_ADDRESS`, emitted when a clause is parsed as an implicit
ADDRESS command and its first command token is not a string literal. This
matches the Level B migration warning while keeping Classic REXX string-literal
commands such as `"listfiles"` or `'listfiles' template` warning-free.

Examples:

```text
RXC-LC-18.1 linenumber="12"
RXC-LC-35.1 token="then"
RXC-LC-40.4 name="FOO"
```

Insert names should follow the standard placeholder where there is an obvious
one, such as `token`, `linenumber`, `keywords`, `value`, `name`, `operator`,
`char`, `argnumber`, `bif`, `description`, and `position`. Tests that need
stable diagnostic text should run in raw mode and primarily assert the
`RXC-LC-...` identity, only asserting insert payloads where the insert is part
of the parser contract under test.

Recovery should be attached to the offending source token where possible. For
statement-level syntax errors, the first recovery boundary is the end of the
current clause. IF/THEN/ELSE recovery may also use `THEN`, `ELSE`, and `END`
as synchronization points where doing so preserves useful highlighting for the
following clause.

The Level C highlighter should not use a separate highlighting grammar. It
should use the Level C parser and validation path, as Level B does today.

## 9. Open Approval Points

These are language/architecture decisions and should be confirmed before code
implementation:

1. Level C default options:
   - Should `options levelc` imply Classic numeric precedence/power mode?
   - Should Level C default `NUMERIC DIGITS` be 9 for execution lowering?
   - Should Level C allow cREXX line-comment options, or start Classic-only with
     nested block comments?
2. Parser ownership:
   - Create separate Level C scanner/glue/grammar, or attempt a shared scanner
     with level-specific keyword classification?
3. Contextual keyword implementation:
   - Implement the likely hybrid: neutral scanner symbols, context-aware
     promotion/demotion in glue, and Lemon `%fallback` only for candidate
     keyword tokens that are proven safe.
   - Confirm whether fallback is worth keeping, and the exact candidate-token
     set if it is, with a small Level C grammar PoC before committing to the
     full grammar.
4. Runtime variable model:
   - Lower variables to helper calls around a runtime pool object first, or add
     VM-native variable-pool support earlier?
5. ADDRESS:
   - Treat Classic `ADDRESS WITH` syntax as Level C syntax from milestone 1, with
     execution deferred, or gate it behind a later milestone?
6. Existing `Level-c-Grammar.tex`:
   - Retire, replace, or keep as historical implementation notes after this
     working document matures.

## 10. Detailed Parsing Strategy Review

This section is a working recommendation, not an approved implementation
contract. It deepens the parser design around Classic REXX contextual keywords,
especially the "no reserved words" rule and Lemon's `%fallback` capability.

### 10.1 Problem boundary

Classic REXX should not be treated as a language where the scanner emits fixed
reserved-word tokens and the grammar consumes them. The public
language-specification source describes a
coordinated relationship between the lexical and top syntax levels:

- assignment and label recognition can override keyword recognition;
- some words become keywords only inside specific syntax subcontexts;
- `THEN` has a special rule after `IF` and `WHEN`;
- `THEN`, `ELSE`, and `OTHERWISE` can cause semicolon insertion;
- inferred concatenation is allowed only when the next operand is not a
  recognized keyword.

Therefore the Level C front end needs a real token adapter between the re2c
scanner and Lemon. Lemon can remain the structural parser, but it should not be
the only component deciding whether a word is a keyword.

### 10.2 Options considered

| Option | Shape | Strengths | Main failures or risks | Working verdict |
| --- | --- | --- | --- | --- |
| A. Scanner keywords plus broad Lemon `%fallback` | Scanner emits `TK_IF`, `TK_THEN`, `TK_SAY`, etc.; Lemon retries failed keyword tokens as `TK_VAR_SYMBOL`. | Smallest change from Level B. Uses a feature Lemon explicitly supports. | Cannot backtrack an already shifted keyword, so cases like `IF = THEN` fail once `IF` was shifted as an `IF` instruction. `%fallback` is global and can incorrectly turn `THEN` into a condition variable after `IF`/`WHEN`. | Reject as the main design. Keep only as a limited helper after PoC. |
| B. Generic-symbol scanner and all keyword checks in grammar actions | Scanner emits every word as `TK_VAR_SYMBOL`; grammar rules compare spellings in actions. | Maximally preserves "no reserved words" at scanner level. | Lemon states become less discriminating, grammar actions become procedural, error recovery gets weaker, and DSLSH cannot color roles until late reductions. | Possible but unattractive for maintainability. |
| C. Context-aware token adapter plus narrow `%fallback` | Scanner emits neutral symbol tokens with spelling/spacing metadata; glue promotes or demotes according to Classic context; Lemon parses hard tokens and optionally uses narrow fallback for proven-safe candidate tokens. | Matches Classic's split between lexical and top syntax. Keeps Lemon grammar readable. Gives DSLSH a precise token role. Allows focused fallback where it helps. | Requires a stateful adapter and a representative test matrix. | Preferred direction. |
| D. Stateful scanner lexical modes | re2c switches modes for `PARSE`, `ADDRESS`, `DO`, `IF`, etc. | Useful for true lexical sublanguages such as strings/comments. | The scanner does not naturally know Lemon's top syntax state; this couples parsing decisions into lexical rules and makes recovery brittle. | Use scanner modes only for lexical facts, not keyword authority. |
| E. PEG/PIKA-style parser for Level C now | Use a parser with richer lookahead/backtracking for Classic REXX. | Closer to the logical grammar direction in `Logical-Grammar-Specification.tex`. Could express some contextual rules more directly. | Diverges from the first-milestone architecture and DSLSH integration. Much higher integration cost before highlighter value appears. | Do not use for milestone 1. Keep as a future logical grammar track. |

Conclusion: the preferred direction is clear enough that we should not build
multiple production approaches. We should run small throwaway PoCs only where
they answer a narrow risk question: exact Lemon fallback behaviour and the
minimum adapter state needed for representative Classic REXX clauses.

### 10.3 Recommended architecture

Use three token identities for Level C words:

1. Raw scanner symbol:
   - The re2c scanner recognizes `VAR_SYMBOL` spelling and preserves original
     spelling, uppercased spelling, source span, preceding-blank information,
     and whether the spelling is in the keyword table.
   - It should not decide that `IF`, `THEN`, or `SAY` is a reserved token by
     itself.
2. Adapter-promoted hard keyword:
   - The glue layer emits hard tokens such as `TK_IF`, `TK_THEN`, `TK_DO`, or
     `TK_WITH` only when the current Classic top-syntax context requires keyword
     treatment.
   - Hard structural tokens should not depend on Lemon fallback to become
     variables later.
3. Optional candidate keyword:
   - If the PoC shows real value, the adapter may emit candidate tokens for
     non-structural words in uncertain contexts.
   - Candidate tokens may be listed in `%fallback TK_VAR_SYMBOL ...`.
   - Candidate fallback must never be the only mechanism that makes assignment,
     labels, `THEN`, `ELSE`, `OTHERWISE`, or `END` correct.

The adapter should be implemented in the Level C parser glue, not in grammar
actions, because it must also insert or suppress semicolon/concatenation tokens
before Lemon can parse the stream.

### 10.4 Adapter state model

The adapter needs enough shallow state to model Classic lexical/top-syntax
interaction without becoming a second full parser.

Required state:

- `clause_start`: true after an explicit or inferred end-of-clause, and after
  the implicit semicolon following a label.
- `label_permitted`: true where an operand followed by `:` may become a label.
- `assignment_permitted`: true where `VAR_SYMBOL = expression` can be a single
  instruction.
- `if_condition`: true after a hard `IF` until a hard `THEN` is recognized.
- `when_condition`: true after a hard `WHEN` until a hard `THEN` is recognized.
- `do_specification`: true after a hard `DO` until the clause terminator that
  starts the body.
- `do_rep`: true inside the repeat portion of a `DO` specification where
  `TO`, `BY`, and `FOR` are special.
- `parse_value`: true while recognizing `PARSE VALUE ... WITH`.
- `parse_template`: true while scanning parse-template syntax rather than
  expression syntax.
- `address_tail`: true after `ADDRESS` where `VALUE`, `WITH`, connection
  words, and environment/command expression boundaries are special.
- `group_stack`: enough shallow nesting state to recognize `END` for
  `DO`/`SELECT`, and `WHEN`/`OTHERWISE` only in a `SELECT` body.
- `operand_left`: whether the last significant token can be the left operand
  of inferred concatenation.
- `blank_before_next`: preserved from the scanner so blank concatenation can be
  distinguished from abuttal concatenation.

The adapter state should be derived from the token stream it emits. Lemon and
the walkers remain responsible for full nesting validation, dangling `ELSE`
binding, and diagnostics when the shallow state guesses are contradicted by the
actual grammar.

The adapter should also carry a token-role annotation for DSLSH, for example
`role=keyword`, `role=identifier`, `role=label`, `role=parse_target`,
`role=reserved_symbol`, `role=synthetic_eoc`, or `role=implicit_concat`.

### 10.5 Keyword categories and fallback policy

The keyword table should be split by grammar role, not just by spelling.

| Category | Words | Promotion rule | `%fallback` policy |
| --- | --- | --- | --- |
| Clause-leading instructions | `ADDRESS`, `ARG`, `CALL`, `DO`, `DROP`, `EXIT`, `IF`, `INTERPRET`, `ITERATE`, `LEAVE`, `NOP`, `NUMERIC`, `OPTIONS`, `PARSE`, `PROCEDURE`, `PULL`, `PUSH`, `QUEUE`, `RETURN`, `SAY`, `SELECT`, `SIGNAL`, `TRACE` | At clause start, after label and assignment lookahead have been handled, promote when the spelling starts a recognized instruction in the current context. | Usually no fallback needed at clause start. If a candidate-token class is introduced, prove it does not turn invalid keyword instructions into commands accidentally. |
| Structural delimiters | `THEN`, `ELSE`, `WHEN`, `OTHERWISE`, `END` | Promote only when a control-group context recognizes the delimiter. Insert synthetic semicolons before and after `THEN`, after `ELSE`/`OTHERWISE`, and after labels according to Classic rules. | No broad fallback. Demote to `TK_VAR_SYMBOL` before Lemon when they are variables; emit hard tokens only when structural. |
| `IF`/`WHEN` condition terminator | `THEN` | While `if_condition` or `when_condition` is active, a symbol spelled `THEN` must be a hard `TK_THEN` wherever a variable symbol could be part of the expression. | Exclude from fallback in these contexts. The safest implementation is no global fallback for `THEN`; the adapter demotes non-keyword `THEN` before Lemon. |
| `DO` repeat and condition words | `TO`, `BY`, `FOR`, `WHILE`, `UNTIL`, `FOREVER` | Promote only inside the correct part of `DO` specification. `WHILE`/`UNTIL` are special wherever a variable symbol would be part of an expression within `do_specification`; `TO`/`BY`/`FOR` are special inside `do_rep`. | Prefer adapter promotion/demotion. Fallback is risky because these words can be normal variables outside `DO`. |
| `ADDRESS` tail words | `VALUE`, `WITH`, `INPUT`, `OUTPUT`, `ERROR`, `STREAM`, `STEM`, `NORMAL`, `APPEND`, `REPLACE` | Promote only inside `ADDRESS` syntax where the grammar recognizes environment, command, value expression, or connection redirection forms. | Candidate fallback may be safe for connection words outside `ADDRESS`, but only after tests. |
| `PARSE` words | `UPPER`, `ARG`, `PULL`, `SOURCE`, `LINEIN`, `VERSION`, `VALUE`, `VAR`, `WITH` | Promote in the `PARSE` instruction and template sublanguage. `WITH` is hard inside `PARSE VALUE`. | Do not rely on fallback for `WITH`; it determines the expression/template boundary. |
| Condition-control words | `ON`, `OFF`, condition names, `NAME` | Promote only in `CALL`/`SIGNAL` condition-control forms. | Candidate fallback is plausible, but should wait until the core grammar is stable. |
| Numeric subkeywords | `DIGITS`, `FORM`, `FUZZ`, `ENGINEERING`, `SCIENTIFIC`, `VALUE` | Promote only after `NUMERIC`. | Candidate fallback likely safe outside `NUMERIC`, but not necessary for milestone 1. |

Fallback should be opt-in, not a blanket keyword policy. The initial grammar PoC
should start with no fallback for structural words and add fallback only for
candidate tokens whose negative tests stay correct.

### 10.6 Required lookahead rules

The adapter needs a small token buffer. It should not stream every scanner token
directly to Lemon.

Lookahead rules:

- If a symbol or literal is followed by `:` and labels are permitted, emit
  `TK_LABEL` and then a synthetic end-of-clause.
- If a symbol is followed by `=` in an assignment-permitted clause context, emit
  `TK_VAR_SYMBOL`, even if the symbol spelling is a keyword.
- If a digit-starting constant or disallowed period constant is followed by
  `=`, emit the standard assignment-left-side error shape rather than
  promoting any keyword.
- If `+`, `-`, `\`, or `(` appears where Classic syntax allows omitted `VALUE`, and the
  context is not after `PARSE`, inject a synthetic `TK_VALUE`.
- If a left operand has been seen and the next token is an operand or left
  parenthesis that is not a keyword, infer blank or abuttal concatenation.
- Do not infer concatenation before a recognized keyword. This is another
  reason keyword classification must happen before expression token insertion.

### 10.7 `THEN` and Lemon alignment

The Classic `THEN` rule is not aligned with broad Lemon `%fallback`.

Lemon fallback is tried only when the current lookahead token has no parse
action in the current state. It does not backtrack previously shifted tokens,
and it does not know that `THEN` has special meaning only in the expression
immediately following `IF` or `WHEN`.

Therefore:

- `THEN` should be a hard token while `if_condition` or `when_condition` is
  active.
- `THEN` should not be in a global fallback list unless it is split into a
  separate candidate token that is never emitted in `IF`/`WHEN` condition
  context.
- The simpler first implementation is to have no `THEN` fallback at all:
  adapter emits hard `TK_THEN` when recognized and `TK_VAR_SYMBOL` otherwise.
- This is compatible with the current Level B grammar shape
  `IF expression THEN instruction`, but Level C must add the Classic synthetic
  semicolon before and after recognized `THEN` so source spans and DSLSH
  structure stay faithful.

### 10.8 PoC plan

Do not build competing production parsers. Run two focused PoCs before the full
Level C grammar:

1. Lemon fallback microgrammar:
   - prove that fallback cannot recover after a keyword has already shifted;
   - prove that excluding `THEN` avoids condition-expression misclassification;
   - decide whether candidate tokens are worth the complexity.
2. Level C token-adapter trace harness:
   - input source text and dump scanner tokens, adapter tokens, synthetic
     tokens, and roles before Lemon;
   - cover keyword variables, assignment, labels, `IF`/`THEN`/`ELSE`,
     `SELECT`/`WHEN`/`OTHERWISE`, `DO` modifiers, `PARSE VALUE ... WITH`,
     `ADDRESS ... WITH`, and inferred concatenation.

Representative fixtures:

- `if = then`
- `say = if`
- `say then`
- `if a then say b`
- `if then then say b`
- `if a then if b then say c else say d`
- `select; when a then say b; otherwise say c; end`
- `do i = 1 to 10 by 2 while ready; say i; end i`
- `parse value a b with x . y`
- `address value env with output stem out.`
- `label: say label`

Success criteria:

- token roles match Classic REXX context, not spelling;
- hard structural delimiters are never recovered as variables by Lemon fallback;
- keyword-spelled assignment left-hand sides parse as variables;
- invalid `IF`/`WHEN` condition forms involving `THEN` remain invalid;
- synthetic semicolons and concatenation tokens have source provenance suitable
  for DSLSH and diagnostics.

PoC 1 execution result:

- Implemented at `compiler/pocs/levelc_lemon_fallback/`.
- Built and run against `cmake-build-debug/lemon/lemon`.
- Confirmed that `%fallback` handles simple expression-context
  keyword-as-identifier cases such as `SAY IF ;`.
- Confirmed that `%fallback` cannot backtrack a clause-leading keyword already
  shifted as a statement keyword, as shown by `IF = A ;` and `SAY = A ;`.
- Confirmed that adding `THEN` to broad fallback cleanly accepts
  `IF THEN THEN SAY A ;`, which is not aligned with the Classic `THEN` rule.

PoC 2 execution result:

- Implemented at `compiler/pocs/levelc_token_adapter_trace/`.
- Built and run with `cc -std=c11 -Wall -Wextra -Werror`.
- Confirmed that a shallow adapter can demote keyword-spelled assignment
  left-hand sides, promote contextual control keywords, preserve `THEN` after
  `IF`/`WHEN`, emit synthetic end-of-clause tokens, mark parse-template roles,
  and suppress inferred concatenation before promoted keywords.
- Confirmed that the adapter needs at least a small pending-`ELSE` hint for
  useful tracing of nested `IF` fixtures; Lemon and the walkers should still
  own full dangling-`ELSE` validation.

## 11. First Implementation Sketch (Historical)

The first production tracer bullet was added on 2026-05-10. It keeps Level C
inside new Level C components and parser-mode routing:

- `compiler/rxcpcscn.re`: neutral Level C scanner. Words scan as variable
  symbols; the scanner does not reserve Classic instruction words.
- `compiler/rxcpcpar.c`: Level C parser glue and contextual adapter. The
  current tracer promotes the implemented Level C statement/control keywords,
  preserves keyword-spelled assignment left-hand sides, and treats a same-line
  label as a clause boundary for the next instruction.
- `compiler/rxcpcgmr.y`: small Level C Lemon grammar for the tracer subset.
- `compiler/rxcpcval.c`: minimal Level C source-tree preparation for DSLSH.
- `compiler/rxcp_highlight_controller.c`: parser mode now dispatches to the
  Level C parser only when the options pre-scan selected Level C.

The tracer subset currently supports enough syntax to exercise the no-reserved
word rule and DSLSH source tree:

```rexx
if = 'keyword-var'
say if
if ready then say 'ready'
label: say label
```

Confirmed DSLSH behaviour:

- `if` at assignment left hand side highlights as `LEXER_IDENTIFIER`.
- clause-leading `say`, control `if`, and condition terminator `then`
  highlight as `LEXER_KEYWORD`.
- `label:` highlights as `LEXER_FUNCTION_IDENTIFIER`.
- the parsed output contains `PARSE_TREE_STATEMENT` nodes and no
  `SYNTAX_ERROR` for the tracer fixture.

Safety boundary:

- Normal `rxc` compilation of `options levelc` still fails with
  `REXX Level A/C/D (cREXX Classic) - Not supported yet`.
- The tracer does not lower to canonical Level B, validate Classic semantics,
  emit assembly, or execute.
- At this historical point, the Level C highlighter grammar had not yet parsed
  `OPTIONS` as a normal instruction after the pre-scan. That gap is now closed;
  current manual tests should usually include `OPTIONS LEVELC`.

Regression tests added:

- `syntaxhighlight_levelc_tracer`
- `syntaxhighlight_levelc_keyword_variables`
- `syntaxhighlight_levelc_if_else`
- `syntaxhighlight_levelc_labels_literals`
- `syntaxhighlight_levelc_then_boundary`
- `syntaxhighlight_levelc_if_missing_then`
- `syntaxhighlight_levelc_loose_then`
- `syntaxhighlight_levelc_loose_else`
- `syntaxhighlight_levelc_multiline_if_else`
- `levelc_compile_unsupported`

Diagnostic slice 1 was added on 2026-05-10:

- `compiler/rxcpcdiag.c` provides Level C-only diagnostic helpers for the
  `RXC-LC-<standard-code>` identity and named inserts.
- The Level C grammar now emits standard identities for the first
  IF/THEN/ELSE recovery cases:
  - `18.1`: IF expression reaches end-of-clause without a matching THEN.
  - `35.1`: IF condition cannot start because `THEN` was found immediately
    after IF.
  - `8.1`: THEN has no corresponding IF or WHEN clause.
  - `8.2`: ELSE has no corresponding THEN clause.
  - `6.1`, `6.2`, `6.3`, and `13.1`: initial mappings for unmatched comments,
    unmatched quotes, and invalid source characters.
- The Level C adapter now promotes loose clause-leading `THEN` and `ELSE`
  only when they are not assignment left-hand sides, and suppresses physical
  EOL tokens after `THEN`, after `ELSE`, and immediately before a pending
  `ELSE` so multiline IF/THEN/ELSE source parses as one instruction.

Focused verification for the diagnostic slice:

```sh
cmake -S /Users/adrian/CLionProjects/CREXX -B /Users/adrian/CLionProjects/CREXX/cmake-build-release
cmake --build /Users/adrian/CLionProjects/CREXX/cmake-build-release --target rxc -j 32
ctest --test-dir /Users/adrian/CLionProjects/CREXX/cmake-build-release -R 'syntaxhighlight_levelc|levelc_compile_unsupported' --output-on-failure
```

Result: all 10 Level C syntax-highlighting and unsupported-compile tests pass.

Full release verification after the slice:

```sh
cmake --build /Users/adrian/CLionProjects/CREXX/cmake-build-release --target all -j 32
ctest --test-dir /Users/adrian/CLionProjects/CREXX/cmake-build-release --output-on-failure
```

Result: full build passes; all 1039 CTest tests pass.

Diagnostic slice 2 was added on 2026-05-11:

- The Level C adapter promotes `SELECT`, `WHEN`, `OTHERWISE`, and `END` as
  contextual keywords when they are not assignment left-hand sides.
- Parser-mode editor configuration now includes `select`, `when`, and
  `otherwise` in its keyword list so THE/editor lexical coloring agrees with
  the parser projection for the new SELECT-family syntax.
- The Level C grammar now parses the first Classic `SELECT` subset:
  `SELECT`, one or more `WHEN expression THEN instruction` clauses, optional
  `OTHERWISE`, and matching `END`.
- The grammar and Level C fallback now emit standard identities for the first
  SELECT-family recovery cases:
  - `7.1`: SELECT reaches END without any WHEN clause.
  - `9.1`: WHEN has no corresponding SELECT.
  - `9.2`: OTHERWISE has no corresponding SELECT.
  - `10.1`: END has no corresponding DO or SELECT.
  - `14.2`: SELECT requires a matching END.
  - `18.2`: WHEN expression reaches end-of-clause without a matching THEN.
- Level C fallback diagnostics are owned by the Level C front end, not by the
  shared fallback module. `rexcpars()` invokes the Level C fallback only when
  Lemon does not produce an AST. The shared fallback remains the Level B-style
  last-resort path, and DSLSH only calls it when the parser has not already
  supplied detached diagnostics.

Regression tests added:

- `syntaxhighlight_levelc_select_when_otherwise`
- `syntaxhighlight_levelc_select_do_otherwise`
- `syntaxhighlight_levelc_select_do_otherwise_upper`
- `syntaxhighlight_levelc_do_keyword_variables`
- `syntaxhighlight_levelc_select_keyword_variables`
- `syntaxhighlight_levelc_select_missing_when`
- `syntaxhighlight_levelc_select_missing_end`
- `syntaxhighlight_levelc_loose_when`
- `syntaxhighlight_levelc_loose_otherwise`
- `syntaxhighlight_levelc_loose_end`
- `syntaxhighlight_levelc_when_missing_then`

Focused verification for the SELECT slice:

```sh
cmake -S /Users/adrian/CLionProjects/CREXX -B /Users/adrian/CLionProjects/CREXX/cmake-build-release
cmake --build /Users/adrian/CLionProjects/CREXX/cmake-build-release --target rxc -j 32
ctest --test-dir /Users/adrian/CLionProjects/CREXX/cmake-build-release -R 'syntaxhighlight_levelc|levelc_compile_unsupported' --output-on-failure
```

Result: all 18 Level C syntax-highlighting and unsupported-compile tests pass.

Full release verification after the SELECT slice:

```sh
cmake --build /Users/adrian/CLionProjects/CREXX/cmake-build-release --target all -j 32
ctest --test-dir /Users/adrian/CLionProjects/CREXX/cmake-build-release --output-on-failure
```

Result: full build passes; all 1047 CTest tests pass.

SELECT/DO highlighter correction on 2026-05-11:

- DSLSH `parser_tester` debug was used against the classic shape:
  `SELECT`, `WHEN condition THEN DO`, body, `END`, `OTHERWISE`, `END`.
- Before the correction, the parser projected `OTHERWISE` as a keyword but with
  error severity because the inner simple `DO ... END` was not yet accepted by
  the Level C tracer grammar. In THE this can render as not-blue/diagnostic
  coloring even though a keyword leaf exists.
- The Level C adapter now promotes clause-leading `DO` as a keyword only when
  it is not an assignment left-hand side.
- The Level C grammar now accepts a simple `DO`, end-of-clause, nested
  instruction list, and matching `END` as a grouping instruction. This is still
  syntax-highlighter support only; repetitive `DO` options and DO validation
  remain later slices.
- New fixtures keep the no-reserved-word rule in view: `do = 1` still
  highlights `do` as an identifier, while `THEN DO` highlights `DO` as a
  keyword.

Regina compatibility probe for the corrected fixtures:

```sh
rexx compiler/tests/rexx_src/levelc_select_do_otherwise.rexx
rexx compiler/tests/rexx_src/levelc_select_do_otherwise_upper.rexx
```

Result: both run successfully and print `fallback`.

Focused verification for the SELECT/DO correction:

```sh
cmake --build /Users/adrian/CLionProjects/CREXX/cmake-build-release --target rxc -j 32
ctest --test-dir /Users/adrian/CLionProjects/CREXX/cmake-build-release -R 'syntaxhighlight_levelc|levelc_compile_unsupported|highlight_editor_diagnostics|highlight_cache' --output-on-failure
```

Result: all 23 focused Level C/highlighting tests pass.

Full release verification after the SELECT/DO correction:

```sh
cmake --build /Users/adrian/CLionProjects/CREXX/cmake-build-release --target all -j 32
ctest --test-dir /Users/adrian/CLionProjects/CREXX/cmake-build-release --output-on-failure
```

Result: full build passes; all 1050 CTest tests pass.

Manual DSLSH/THE testing:

- Automated fixture tests use `parser_tester -p cmake-build-debug/bin/rxc`.
  `parser_tester` appends `-d --syntaxhighlight` itself.
- For THE or another real DSLSH editor integration, use the parser command
  `/Users/adrian/CLionProjects/CREXX/cmake-build-debug/bin/rxc --syntaxhighlight`.
- Use `OPTIONS LEVELC` for Level C manual tests unless intentionally checking
  headerless parser-mode defaulting.

Level C DO/END control slice on 2026-05-11:

- The Level C token adapter now recognizes the Classic DO-specification words
  `TO`, `BY`, `FOR`, `WHILE`, `UNTIL`, and `FOREVER` only while scanning a
  `DO` header. Clause-leading `LEAVE` and `ITERATE` are recognized as
  instructions. Assignment lookahead still wins, so forms such as `to = 1`,
  `forever = 7`, `leave = 8`, and `do to = 1 to 2` remain variables/control
  variables rather than reserved words.
- Regina confirms that `DO TO` is not a counted-loop expression using variable
  `TO`; it is standard error `27.1`. The adapter therefore promotes
  `TO`/`BY`/`FOR` in the DO header unless assignment lookahead identifies the
  word as the control variable. Once a `WHILE` or `UNTIL` condition starts,
  subsequent words are left as expression variables for the no-reserved-words
  rule.
- The Level C grammar now accepts these tracer/highlighter forms:
  - `DO` simple grouping.
  - `DO expression`.
  - `DO symbol = expression [TO expression] [BY expression] [FOR expression]`.
  - `DO WHILE expression` and `DO UNTIL expression`.
  - `DO repetition WHILE/UNTIL expression`.
  - `DO FOREVER`, optionally followed by `WHILE` or `UNTIL`.
  - `END [symbol]`.
  - `LEAVE [symbol]` and `ITERATE [symbol]`.
- Validation remains Level C-only and currently runs as a structural token scan
  after the Level C AST is projected. It emits standard identities for:
  - `10.2`: controlled `DO` ended with the wrong symbol.
  - `10.3`: un-controlled `DO` ended with a symbol.
  - `10.4`: `SELECT` ended with a symbol.
  - `25.16`: `FOREVER` followed by something other than `WHILE`, `UNTIL`, or
    end-of-clause.
  - `28.1`/`28.2`: `LEAVE`/`ITERATE` outside a repetitive `DO`.
  - `28.3`/`28.4`: named `LEAVE`/`ITERATE` not matching a current repetitive
    DO control variable.
- The first source-symbol validation slice adds a small Level C-only symbol
  helper (`rxcpcsym.c`) and keeps the checks deliberately before tree surgery:
  - labels are canonicalized case-insensitively and recorded with their current
    `DO`/`SELECT` group depth;
  - direct `SIGNAL label` emits `16.1` when the label is not present in the
    current source, and `16.2` when the source proves the target label is inside
    a `DO`/`SELECT` group;
  - direct `CALL label` emits `16.3` only when the target is a present local
    label inside a `DO`/`SELECT` group;
  - unknown `CALL` targets are not errors, because Classic REXX procedures and
    functions can be external and resolved at runtime;
  - Classic BIF names are preloaded in the Level C helper so later BIF-aware
    validation has one source of truth, but this slice does not make unknown
    external calls invalid;
  - `PROCEDURE` emits `17.1` unless it is the first instruction following a
    local label;
  - simple constant `NUMERIC DIGITS`, `NUMERIC FUZZ`, and `NUMERIC FORM VALUE`
    clauses emit `26.5`, `26.6`, and `33.6` when the invalid value is known at
    parse/highlight time, including direct quoted-string constants for
    `DIGITS`/`FUZZ`.
- The deep validation pass adds conservative source-proven checks that are still
  Level C-only:
  - binary and hexadecimal string literals emit Level C identities `15.1` through
    `15.4` for invalid blank placement and invalid characters. The shared
    string decoder still owns the raw decoding, but Level C source-tree
    preparation rewrites its legacy `INVALID_HEX`/`INVALID_BIN` AST diagnostics
    into the standard Level C codes before DSLSH sees them;
  - static `TRACE` request constants emit `24.1` when the first effective
    request letter is not one of `ACEFILNOR`; signed whole-number trace settings
    are accepted, and a bare sign after `TRACE` emits `19.6`;
  - malformed `LEAVE` and `ITERATE` targets emit `20.2` and suppress the
    misleading loop-context `28.*` diagnostics for the same clause;
  - number-leading assignment such as `10 = value` emits `31.1` rather than
    being treated as an implicit ADDRESS command;
  - `ADDRESS WITH` resources now validate the reachable static resource shape:
    missing `INPUT`/`OUTPUT`/`ERROR` resources (`25.6`, `25.7`, `25.14`),
    invalid `APPEND`/`REPLACE` resources (`25.8`, `25.9`), missing
    `STREAM`/`STEM` variables (`53.1`, `53.2`), and invalid `STEM` names
    (`53.3`).
- The `ADDRESS WITH` validation deliberately checks resource syntax, not every
  possible connection-order semantic. Duplicate or oddly ordered connection
  clauses can be tightened in a later AST validation pass if the standard text
  gives us a source-proven compile-time error.
- The validator intentionally checks repetitive loops rather than every
  grouping `DO`, so `LEAVE` inside plain `DO ... END` is rejected until an
  enclosing repetitive loop exists.
- Runtime checks such as whole-number repetition counts, logical values for
  `WHILE`/`UNTIL`, and numeric `TO`/`BY`/`FOR` values are recorded for later
  validation/lowering slices. The first milestone still stops at
  syntax-highlighting and structural diagnostics.
- Compile-time validation intentionally remains conservative. We should only
  emit standard errors when the source text proves the error before execution;
  dynamic procedure/function lookup, `SIGNAL VALUE`, variable-driven `NUMERIC`
  settings, and BIF argument semantics stay for later runtime or post-lowering
  validation.

Regina compatibility probes for the DO slice:

```sh
rexx compiler/tests/rexx_src/levelc_do_control.rexx
rexx compiler/tests/rexx_src/levelc_do_conditions_leave.rexx
```

These fixtures are intended to run under Regina and provide an external sanity
check for the accepted Classic REXX syntax while the cREXX Level C path still
stops at parsing/highlighting.

Result: `levelc_do_control.rexx` prints `1`, `2`, `3`;
`levelc_do_conditions_leave.rexx` prints `1`. Regina also rejects `DO TO`
with standard error `27.1`, matching
`syntaxhighlight_levelc_do_invalid_initial_to`.

Additional Regina probes for the source-symbol slice:

```sh
rexx /tmp/levelc-procedure-main.rexx
rexx /tmp/levelc-procedure-caller/caller.rexx
```

where the first file starts with `PROCEDURE`, and the second calls an external
file whose first instruction is `PROCEDURE`. Regina reports standard error
`17.1` for the `PROCEDURE` instruction in both cases. That supports checking
the placement of the `PROCEDURE` instruction itself, but it does not make an
unknown `CALL` target invalid: external procedure/function resolution remains a
runtime concern and the Level C parser/highlighter should not emit
label-not-found diagnostics for unknown `CALL` or future function-call targets.

Additional Regina probes for the deep-validation slice:

```sh
probe=$(mktemp /tmp/levelc-regina-positive.XXXXXX)
printf '%s\n' \
  'options levelc' \
  'trace normal' \
  'trace "??"' \
  'trace 3' \
  'trace -3' \
  "say 'F'X" \
  "say '0F 0A'X" \
  "say '1'B" \
  "say '0000 1111'B" > "$probe"
rexx "$probe"
```

The probe returned zero under Regina, confirming the accepted `TRACE` constants,
signed numeric `TRACE`, and valid binary/hexadecimal string forms. Regina rejects
the new `ADDRESS WITH INPUT ...` positive fixture with `25.5`; for this slice
the cREXX parser follows the public language-specification grammar text for
`ADDRESS WITH` rather than
using Regina as the deciding oracle for that subgrammar.

Regression tests added for the DO slice:

- `syntaxhighlight_levelc_do_control`
- `syntaxhighlight_levelc_do_conditions_leave`
- `syntaxhighlight_levelc_do_end_bad_symbol`
- `syntaxhighlight_levelc_do_end_uncontrolled_symbol`
- `syntaxhighlight_levelc_select_end_symbol`
- `syntaxhighlight_levelc_leave_outside_loop`
- `syntaxhighlight_levelc_iterate_bad_target`
- `syntaxhighlight_levelc_do_forever_bad_tail`
- `syntaxhighlight_levelc_do_invalid_initial_to`
- expanded `syntaxhighlight_levelc_do_keyword_variables`

Level C simple-instruction header slice on 2026-05-12:

- The Level C token adapter now recognizes these remaining Classic single
  instruction headers at clause start only: `ADDRESS`, `ARG`, `CALL`, `DROP`,
  `EXIT`, `INTERPRET`, `NOP`, `NUMERIC`, `PROCEDURE`, `PULL`, `PUSH`,
  `QUEUE`, `RETURN`, `SIGNAL`, and `TRACE`.
- Assignment lookahead still wins. Forms such as `address = 1`, `arg = 2`,
  `call = 3`, and `trace = 15` remain ordinary assignment clauses, preserving
  the Classic REXX no-reserved-words rule.
- For milestone 1 highlighting, this slice gives each instruction a distinct
  statement-shaped AST node and source-tree statement projection. Full operand
  validation is deliberately narrower than the standard grammar for now:
  `EXIT`, `INTERPRET`, `PUSH`, `QUEUE`, and `RETURN` use the current expression
  parser; `ARG`, `CALL`, `DROP`, `NUMERIC`, `PROCEDURE`, `PULL`, `SIGNAL`,
  `TRACE`, and `ADDRESS` retain a flat token tail until their subgrammars are
  widened.
- `PARSE` and full parse-template recognition remain deferred. `ARG` and
  `PULL` are accepted as instruction headers in this slice, but complete
  template validation, dot placeholders, positional patterns, and
  parenthesized pattern variables belong with the `PARSE` slice.
- Contextual tail keywords such as `WITH`, `VALUE`, `ON`, `OFF`, `NAME`,
  `DIGITS`, `FORM`, `FUZZ`, and `EXPOSE` are not yet promoted separately in
  Level C tails. They remain identifier-colored inside the flat tail until the
  related subgrammar owns them.

Regina compatibility probe for the simple-instruction header slice:

A temporary smoke script was generated and run with Regina using `rexx "$probe"`.
Result: exits with status `0` for a smoke script covering bare
`ADDRESS`, `ARG`, `DROP`, `INTERPRET`, `NOP`, `NUMERIC DIGITS`, `NUMERIC FORM`,
`NUMERIC FUZZ`, `PUSH`, `QUEUE`, `CALL`, `SIGNAL`, `TRACE`, `PROCEDURE EXPOSE`,
and `RETURN`. The probe is execution-safe and keeps `PULL` in a non-executed
branch because running `PULL` would wait for input.

Focused verification for the DO slice:

```sh
cmake -S /Users/adrian/CLionProjects/CREXX -B /Users/adrian/CLionProjects/CREXX/cmake-build-release
cmake --build /Users/adrian/CLionProjects/CREXX/cmake-build-release --target rxc parser_tester -j 32
ctest --test-dir /Users/adrian/CLionProjects/CREXX/cmake-build-release -R 'syntaxhighlight_levelc|levelc_compile_unsupported|highlight_editor_diagnostics|highlight_cache' --output-on-failure
```

Result: all 32 focused Level C/highlighting tests pass.

Full release verification after the DO slice:

```sh
cmake --build /Users/adrian/CLionProjects/CREXX/cmake-build-release --target all -j 32
ctest --test-dir /Users/adrian/CLionProjects/CREXX/cmake-build-release --output-on-failure
```

Result: full build passes; all 1059 CTest tests pass.

Level C expression-core slice on 2026-05-11:

- The Level C scanner now recognizes the Classic expression operator set needed
  for the highlighter milestone:
  - arithmetic: `+`, `-`, `*`, `/`, `%`, `//`, `**`
  - concatenation: `||` and inferred blank concatenation in the grammar
  - logical: prefix `\`, `&`, `|`, `&&`
  - normal comparisons: `=`, `\=`, `<>`, `><`, `>`, `<`, `>=`, `<=`, `\>`,
    `\<`
  - strict comparisons: `==`, `\==`, `>>`, `<<`, `>>=`, `<<=`, `\>>`, `\<<`
- Compound operators continue to accept optional blanks between their character
  components, matching the public extraction notes.
- The grammar implements the Classic precedence ladder documented in section
  6.4 and mirrors the Level B `NUMERIC_CLASSIC` expression structure where that
  is already proven: high-priority prefix minus above left-associative power,
  then multiplication, addition, concatenation, comparison, logical AND, and
  logical OR/exclusive-OR.
- `&&` now maps to shared `OP_XOR`, not to ordinary not-equals. This keeps the
  syntax tree aligned with Classic REXX logical semantics and leaves the Level C
  validator room to enforce Classic logical-value diagnostics before lowering.
- `DO symbol = expression` now uses a Level C-only contextual
  `CTK_DO_CONTROL_SYMBOL` parser token when the first DO-header symbol is
  followed by `=`, avoiding the Lemon ambiguity between a counted control
  assignment and a comparison expression.
- Bad comma and stray right-parenthesis recovery now emit standard error
  identities `37.1` and `37.2` and preserve the following clause for
  highlighting. Formatting of the final user-facing text is still deferred to
  the common error-message stage.
- A trailing comparison operator at end-of-clause, for example `say 1 =`,
  is invalid Classic REXX. The Level C tracer now anchors standard error
  `35.1` on the visible operator with `token="end-of-clause"` and keeps the
  next clause as a separate source-tree statement.
- The bad-expression sign-off slice extends that same recovery shape to:
  trailing arithmetic/concatenation/logical operators, dangling prefix
  operators, leading binary operators, unmatched left parentheses, and missing
  operands before `)`. The grammar uses Level C-only synthetic
  `CTK_MISSING_EXPR` and `CTK_MISSING_RPAREN` tokens from the glue layer rather
  than broad empty productions; this keeps Lemon conflicts out of the normal
  expression grammar while still letting the editor resynchronize at the clause
  boundary. Standard identities are `35.1` for invalid/missing expression
  operands and `36` for unmatched `(`.
- `say = 1` is deliberately not treated as a bad expression in Classic REXX:
  because there are no reserved words, it is an assignment to a variable named
  `say`.
- DSLSH currently classifies the single `=` token as
  `LEXER_OPERATOR_ASSIGN` even when it is a comparison operator. That is a
  shared highlighting classification issue, not a Level C parse failure.

Regina compatibility probes for the expression slice:

```sh
rexx compiler/tests/rexx_src/levelc_expression_precedence.rexx
rexx compiler/tests/rexx_src/levelc_expression_comparisons.rexx
rexx compiler/tests/rexx_src/levelc_expression_bad_comma.rexx
rexx compiler/tests/rexx_src/levelc_expression_bad_rparen.rexx
rexx compiler/tests/rexx_src/levelc_expression_bad_trailing_operator.rexx
rexx compiler/tests/rexx_src/levelc_expression_bad_prefix_operator.rexx
rexx compiler/tests/rexx_src/levelc_expression_bad_leading_operator.rexx
rexx compiler/tests/rexx_src/levelc_expression_bad_paren.rexx
printf "say 1 =\n" >/tmp/levelc_bad_trailing_compare.rexx && rexx /tmp/levelc_bad_trailing_compare.rexx
```

Result: the positive precedence fixture prints `69 0 ab c 0`, confirming the
Classic prefix-minus/power, remainder, blank-concatenation, and logical
precedence shape. The comparison fixture runs and prints only the expected true
branches under Regina's uninitialized-variable behaviour. The bad-comma fixture
is rejected by Regina with syntax error `64.1`; the Level C highlighter reports
the language-specification-derived identity `RXC-LC-37.1` and resynchronizes at the following
`SAY`. The bad-right-parenthesis fixture is rejected by Regina and the Level C
highlighter reports `RXC-LC-37.2`. Regina also rejects `say 1 =` with syntax
error `64.1`; the Level C highlighter reports `RXC-LC-35.1
token="end-of-clause"` and resynchronizes at the next `SAY`.

Regina behaviour for the wider bad-expression fixtures is consistent with the
standard-message mapping: trailing operators produce syntax error `64.1`,
dangling prefixes and leading binary operators produce `35.1`, and unmatched
left parentheses produce `36`. The Level C highlighter reports the same
standard identities where the standard provides them and keeps the following
`SAY` clause as a separate source-tree statement.

Regina also confirms the Classic XOR truth table: `0 && 0`, `0 && 1`, `1 && 0`,
`1 && 1` prints `0`, `1`, `1`, `0`.

Regression tests added for the expression slice:

- `syntaxhighlight_levelc_expression_precedence`
- `syntaxhighlight_levelc_expression_comparisons`
- `syntaxhighlight_levelc_expression_bad_comma`
- `syntaxhighlight_levelc_expression_bad_rparen`
- `syntaxhighlight_levelc_expression_bad_trailing_compare`
- `syntaxhighlight_levelc_expression_bad_trailing_operator`
- `syntaxhighlight_levelc_expression_bad_prefix_operator`
- `syntaxhighlight_levelc_expression_bad_leading_operator`
- `syntaxhighlight_levelc_expression_bad_paren`
- `err_xor_common_fail`
- `classic_xor_run_noopt`
- `classic_xor_run_opt`

The XOR tests deliberately include `2 && 3`, which must evaluate as false after
logical targeting. This distinguishes Classic logical exclusive-or from normal
not-equals code generation.

Focused verification for the expression slice:

```sh
cmake -S /Users/adrian/CLionProjects/CREXX -B /Users/adrian/CLionProjects/CREXX/cmake-build-release
cmake --build /Users/adrian/CLionProjects/CREXX/cmake-build-release --target rxc parser_tester -j 32
ctest --test-dir /Users/adrian/CLionProjects/CREXX/cmake-build-release -R 'syntaxhighlight_levelc_expression|syntaxhighlight_levelc_do_control|syntaxhighlight_levelc_if_else' --output-on-failure
```

Result: all 6 focused tests pass.

XOR follow-up verification:

```sh
cmake --build /Users/adrian/CLionProjects/CREXX/cmake-build-release --target compiler_exit_bin -j 32
ctest --test-dir /Users/adrian/CLionProjects/CREXX/cmake-build-release -R 'classic_xor|err_xor_common|syntaxhighlight_levelc_expression|levelc_compile_unsupported|highlight_editor_diagnostics|highlight_cache' --output-on-failure
```

Result: the compiler exit bundle rebuilds cleanly after regenerating the bundled
exit artifacts, and all 10 focused XOR/Level C/highlighting tests pass.

Full release verification after the expression slice:

```sh
cmake --build /Users/adrian/CLionProjects/CREXX/cmake-build-release --target all -j 32
ctest --test-dir /Users/adrian/CLionProjects/CREXX/cmake-build-release --output-on-failure
```

Result: full build passes; all 1071 CTest tests pass after the bad-expression
coverage sign-off.

Level C parse/instruction completion slice on 2026-05-12:

- The Level C adapter now promotes `PARSE` and its contextual words:
  `UPPER`, `ARG`, `PULL`, `SOURCE`, `LINEIN`, `VERSION`, `VALUE`, `VAR`, and
  `WITH`. `WITH` is treated as the `PARSE VALUE` expression/template boundary,
  and parse-template syntax after that point is not parsed as an ordinary
  expression.
- Parse templates now cover the milestone-1 syntax sublanguage:
  direct targets, dot placeholders, string patterns, parenthesized variable
  patterns, absolute positions, relative positions, and comma-separated template
  lists with omitted templates.
- `ARG` and `PULL` now share the parse-template list grammar, so their tails are
  no longer flat token lists.
- Structured instruction tails were added for the remaining simple instruction
  families needed before the validation/symbols slice:
  - `ADDRESS [VALUE expression] [WITH ...]`, including `INPUT`, `OUTPUT`,
    `ERROR`, `STREAM`, `STEM`, `NORMAL`, `APPEND`, and `REPLACE` keyword roles.
  - `CALL` target form plus `CALL ON/OFF condition [NAME target]`.
  - `DROP` variable lists and parenthesized indirect references.
  - `NUMERIC DIGITS`, `NUMERIC FORM ENGINEERING/SCIENTIFIC/VALUE`, and
    `NUMERIC FUZZ`.
  - `PROCEDURE [EXPOSE variable-list]`.
  - `SIGNAL` target, `SIGNAL VALUE expression`, and condition-control forms.
  - `TRACE` target and `TRACE VALUE expression`.
- Command clauses are now parsed as Level C-only `IMPLICIT_CMD` statements. The
  adapter emits a special command-start parser token only for a clause-leading
  symbol that is not assignment lookahead and not a promoted instruction keyword;
  this keeps `name = value` as assignment while accepting `name value` as a
  command expression.
- The grammar now emits additional standard diagnostic identities for malformed
  simple-instruction forms:
  - `6.2`/`6.3`: unmatched single or double quotes, including malformed string
    operands in instruction tails.
  - `19.2`: bare `CALL`.
  - `19.4`: bare `SIGNAL`.
  - `20.1`: bare `DROP` or empty indirect variable reference.
  - `25.5`: `ADDRESS WITH` without a connection keyword.
  - `25.11`: `NUMERIC FORM` without `ENGINEERING`, `SCIENTIFIC`, or `VALUE`.
  - `25.12`/`25.13`: invalid or missing `PARSE` type, including after `UPPER`.
  - `25.15`: invalid `NUMERIC` subkeyword.
  - `25.17`: invalid `PROCEDURE` tail keyword.
  - `38.2`/`38.3`: invalid parse positional form and missing `WITH` for
    `PARSE VALUE`.
  - `19.7`/`46.1`: invalid or unterminated parenthesized parse pattern variable.
- The AST source-projection cleanup in this slice makes structured child keyword
  nodes refer to the actual contextual token (`ON`, `OFF`, `DIGITS`, `FUZZ`)
  instead of reusing the parent instruction token.
- Follow-up recovery tightening anchors invalid immediate tail tokens on the
  offending token rather than allowing the next clause to be blamed. The smoke
  case `NUMERIC 10` followed by `ARG a b` now reports `25.15` on `10`, while
  `ARG` and its template targets remain clean. The same boundary fixture covers
  `DROP 10`, `CALL 10`, `SIGNAL 10`, `PROCEDURE 10`,
  `PROCEDURE EXPOSE 10`, `PARSE 10`, and `PARSE UPPER 10`.
- Silent accepts are a first-class highlighter failure. The recovery/warning
  tightening pass adds explicit recovery for malformed instruction tails that
  previously flattened into ordinary tokens: `CALL ON/OFF` condition tails,
  `CALL/SIGNAL ... NAME` missing and bad targets, `SIGNAL ON/OFF` condition
  tails, `ADDRESS VALUE`, `ADDRESS WITH`, `NUMERIC FORM`, `PROCEDURE EXPOSE`,
  `PARSE VAR`, bad `ARG`/`PULL` templates, and expression-required tails such
  as `PUSH +`, `RETURN +`, `SIGNAL VALUE`, and `ADDRESS VALUE +`.
- `ADDRESS WITH` now requires the first connection word after `WITH` to be
  `INPUT`, `OUTPUT`, or `ERROR`; a bare word such as `ADDRESS WITH banana`
  reports `25.5` at `banana` rather than being accepted as a connection atom.
- Non-string implicit commands now emit the Level C warning
  `RXC-LC-IMPLICIT_ADDRESS`. String-literal command clauses remain clean, which
  preserves common Classic REXX command-program style while making likely typos
  such as `SEY value` visible in DSLSH.
- At this historical point this remained parser/highlighter work only. The
  later first execution-lowering tracer deliberately opens only the narrow
  slice documented in `levelc_remapping_target.md`; all other Level C compile
  inputs still stop with the unsupported diagnostic.

Regression tests added or widened for this slice:

- `syntaxhighlight_levelc_simple_instructions`
- `syntaxhighlight_levelc_command_instructions`
- `syntaxhighlight_levelc_implicit_command_warning`
- `syntaxhighlight_levelc_instruction_bad_forms`
- `syntaxhighlight_levelc_instruction_recovery_boundaries`
- `syntaxhighlight_levelc_instruction_tail_bad_forms`
- `syntaxhighlight_levelc_parse_templates`
- `syntaxhighlight_levelc_parse_bad_forms`
- `syntaxhighlight_levelc_deep_validation`
- `syntaxhighlight_levelc_deep_validation_ok`

Focused verification for the parse/instruction completion slice:

```sh
cmake -S /Users/adrian/CLionProjects/CREXX -B /Users/adrian/CLionProjects/CREXX/cmake-build-release
cmake --build /Users/adrian/CLionProjects/CREXX/cmake-build-release --target rxc parser_tester -j 32
ctest --test-dir /Users/adrian/CLionProjects/CREXX/cmake-build-release -R 'syntaxhighlight_levelc|levelc_compile_unsupported|highlight_editor_diagnostics|highlight_cache' --output-on-failure
```

Result after the deep-validation slice: all 52 Level C tests pass.

Full release verification after the parse/instruction completion slice:

```sh
cmake --build /Users/adrian/CLionProjects/CREXX/cmake-build-release --target all -j 32
ctest --test-dir /Users/adrian/CLionProjects/CREXX/cmake-build-release --output-on-failure
```

Result after the deep-validation slice: full build passes; all
1084 CTest tests pass.

The remaining first implementation sequence is:

1. Review the remaining lexer/parser gaps against the public syntax text:
   non-integer number forms, period-start constants and reserved symbols,
   function-call syntax, nested comments, continuation edge cases, and any
   keyword fallback cases that still depend on adapter state.
2. Add the next conservative AST validation pass before tree surgery:
   parse-template target restrictions, `CALL`/`SIGNAL` condition and `NAME`
   target checks, compound/stem symbol-table rules, reserved-symbol rules, and
   any source-proven `ADDRESS WITH` connection-order diagnostics.
3. Keep adding parser-mode fixtures for every accepted/rejected instruction form,
   with positive coverage and negative `RXC-LC-...` identities.
4. Only after highlighter validation is stable, start canonical lowering and tree
   surgery hardening.
