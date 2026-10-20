# Level C instruction review

The [requirements ledger](levelc-requirements.md#whole-instruction-work-units)
owns all 25 instruction statuses, detailed native-criterion links, dependencies
and closure checks. The [plan](../PLAN.md#remaining-actions) owns the approved
sequence; [architecture](architecture.md) owns the design. This document retains
review reasoning and findings, not a duplicate progress table.

Review each instruction in LC-I order with independent values/errors, source
observations and full caller/helper RXAS. Record shared host/condition/TRACE
dependencies while advancing to the next review; a whole row stays open until
those dependencies pass. INTERPRET is designed in the complete architecture and
implemented last after static acceptance.

## SAY: contract and paths

[LC-AC-57 / LC-I-01](../../../../docs/planning/release-1/levelc-compatibility-worklist.md)
and LC-REF-068 require an optional expression evaluated once, followed by one
line through configured output, including an empty line for bare SAY. Shared
expression rules apply without an extra SAY-specific limit. LC-REF-028 and the
accepted LC-UNICODE model govern radix literals and fixed U+0000–U+00FF ordinals.

| Obligation | New path and evidence | Remaining boundary |
|---|---|---|
| Bare and expression forms | `levelc.rxl` creates `Say(value?)`; `llcfrontend.statement` lowers the expression once, then one `crexx.c.say`. Existing frontend grammar/syntax tests plus `tests/say/forms-c.crexx`. | Full expression/BIF coverage belongs to shared semantics and CFC-AC02. |
| Evaluation/output order | Ordinary numeric/string expressions, IF/group and private-procedure call with inner output before outer output. `tests/say/forms-c.crexx` has independent expected bytes. | General call forms and conditions/TRACE retain CALL/shared owners; static shared entries and EXPOSE are implemented. |
| Text and ordinals | `llcparse` joins only an adjacent single X/B symbol to a C String token; `llcfrontend.ordinalliteral` validates and uses B `b2x`/`x2c` at compilation. Existing `core.constant` preserves exact UTF-8 text. | No G radix-literal expansion. No general BIF admission is implied by compiler-side decoding. |
| Output route | `llcrxas.classic` calls checked `llctarget.sayvalue`; its B implementation calls `RexxValue.asString(1)` and the existing VM SAY instruction. | Modern SAY checks the current route, bare line, once-only expression, visible Unicode/numeric text, order and ordinary errors. Further historical callback/byte-exact host edge parity is deferred and unclaimed; retain existing native controls without adding a peripheral matrix. |
| Syntax and expression failure | Focused malformed literal/expression tests preserve existing output files. Runtime division aborts before the failing SAY or any later output, with authored location. | Classic numeric diagnostic identities for all source errors, caught conditions and full TRACE/clause observations remain open. |
| Source and RXAS | Complete literal/error RXAS and shape assertions in the maintained SAY fixtures and tests; stable logical basename, full literal span, one output call and no runtime radix conversion. | Full source clause IDs and TRACE remain CFC-AC03 / LC-I-24; generated clause ID zero does not claim full trace support. |

### CFC-SAY-DEF01: radix suffix treated as a variable

The pre-repair command `say '410042'x` succeeded but printed `410042X`, in
both compiler IR modes. The token-layout adapter inserted abutment before X;
its value came from the variable pool. The expected result is A, NUL, B, LF.
This is a new compiler defect, not a missing optimisation.

The repair combines adjacent string-plus-single-symbol X/B tokens while keeping
the authored bytes. A space, comment or longer identifier prevents that merge.
Semantic lowering validates the digits and grouping and constructs the existing
Classic constant. No new generic Level L API/schema, AST kind, runtime helper
or emitter operation is needed. In particular, the emitted program does not
call X2C/B2X or read a variable named X/B to execute the literal.

The grouping expectation follows IBM's [Classic token reference](https://www.ibm.com/docs/en/zos/2.5.0?topic=syntax-tokens):
hex groups have byte boundaries, binary groups have nibble boundaries, the
leading group can be short, and boundary blanks are invalid. cREXX's separately
approved ordinal model maps decoded byte values to Unicode scalars rather than
the reference host's EBCDIC page. Invalid grouping is diagnosed before emission,
even inside an unreachable arm. A fresh native `rxc-c -n` control rejects `'4 1'x` with RXC-LC-15.1,
agreeing with the independent expectation. Inspecting the common literal decoder
alone would miss that earlier native validation; no native defect is claimed.

### RXAS review and outstanding cost

The full literal-only baseline still constructs root activation/pool/configuration
owners and a borrowed bundle, a return packet, same-frame numeric setup and a
fatal-error handler. Constant SAY itself does not use the pool or argument
owners; an authored `options levelc` now retains its separate Name read. These costs belong to the
existing context/pool specialisation and slot/register cleanup owners in
[poc-coverage](poc-coverage.md#code-quality-and-remaining-work), not to the SAY
output contract. Record their removal criteria: prove no pool, argument or
numeric-context observer, preserve call/return/condition semantics, and assert
absence of the corresponding constructions in constant-only programs. This
review does not accept the baseline as optimal or close CFC-AC06.

The output helper itself performs one Classic string conversion followed by
one VM SAY. Its checked call has a typed argument window and cleanup. Inspect
that body as well as the caller: one helper call is not one unit of total work.
The radix repair removes the erroneous variable lookup/concatenation and their
runtime helper work; it does not hide decoding in a target helper. Source
records for literal evaluation and output describe different authored ranges;
they must not be deleted just to reduce metadata counts.

The bounded SAY route is implemented. Retain independent literal/order/error
expectations, both compiler IR modes and VMs, separate assembler optimization,
all 256 ordinals and the absence of runtime radix decoding. Shared-expression
and TRACE obligations remain open; the plan identifies the current instruction.

## DROP: ordered names and borrowed pool effects

[LC-I-02](levelc-requirements.md#whole-instruction-work-units) reuses the existing
shared pool's direct and indirect operations. The approved C v3 contract adds
`CDrop` with ordered direct/indirect items and one `crexx.c.drop` operation per
item. Each operation consumes the borrowed context and carries raw name text
plus its indirect flag. It has no result and retains context/condition effects.
The compiler neither splits subsidiary words nor implements compound tails.

| Obligation | Implementation and independent expectation | Remaining boundary |
|---|---|---|
| Source names and item spans | The generated UTF-8 lexer supplies the same Unicode-letter/punctuation/dot rule to parsing and selector name admission. `tests/drop/contracts.crexx` checks micro-sign/Greek identity, parentheses-inclusive byte spans, ordered raw attributes and item origins. | Identifier-token routine/call names use shared Unicode whole-name casing; numeric/quoted labels, expanded CALL forms and host name access retain STEP04/06/08 owners. |
| Contextual list boundaries | Source fixtures distinguish ordinary THEN/ELSE targets, a completed prior IF, a completed ELSE arm, a real nested outer ELSE and DO-contained lists. Following expressions resume ordinary concatenation. | Full IF/DO instruction review remains separate. |
| Ordered state | One direct item calls `dropSymbol` once. Each indirect item reads its carrier at that point, captures `symbolValue(...).asString(0)` once and calls `dropIndirectList` once. Independent output covers substitution after an earlier drop, self-carrier capture, arbitrary tails, defaults/tombstones and dropped versus empty. | Configured NOVALUE, complete diagnostics and condition/TRACE observations remain open. |
| Shared aliases and configuration | A test-only exported C body is invoked by a B root with stable borrowed owners, exact lowercase compound-tail aliases and configured extra blanks. The parent uppercase tail survives whole-stem drop; the alias can subsequently restore the lowercase tail. Captured self-carrier lists continue after invalid words. | Prebound alias integration composes with source EXPOSE; the shared pool retains its independent tests. |
| Refusal and atomicity | Malformed source, including unreachable source, preserves a previous output. Generic valid graphs with invalid reachable/dead load/store/drop names publish no RXAS and remain byte-identical. Existing schema/context tests reject previous/unknown C versions. | Full Classic diagnostic identity, resumable conditions and TRACE remain shared later obligations. |
| Caller and helper work | Structural checks require ordered existing drop/read/conversion calls, one capture and borrowed pool/configuration getters; inspect their receiver cleanup and existing pool bodies. A synthetic execution marker detects an accidentally entered C root wrapper in the B-root fixture. | Root owners, entry numeric setup, fatal dispatch and conservative temporaries retain C-OPT owners; no optimal-frame or speed claim follows. |

`tests/check-drop.py` owns the focused source, admission and execution checks.
Resumable conditions, complete diagnostic identities and TRACE
remain open, so whole LC-I-02 remains partial.
Existing native C DROP fixtures and shared B pool/Unicode proofs
remain read-only semantic controls, not inherited passes for the new compiler.

## Assignment: one write after RHS evaluation

[LC-AC-54/63](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria)
require scalar/stem/arbitrary compound writes, an omitted RHS as assigned empty
text, stem reset of explicit/dropped tails and substitution after RHS evaluation.
The existing frontend already evaluates the RHS before one raw-name
`crexx.c.store`; the selector/target then call the current pool's
`setSymbolValue`. The pool owns classification, case-preserved substituted tails,
aliases and value/cache storage. No product defect or new API/schema is assumed.

Assignment requires independent source,
AST/IR, prebound-context and caller/helper RXAS proof. Source marker calls and
old-value reads establish once-only RHS-before-write order. An exported test IR
body mutates the supplied live context before its store; a B owner observes the
selected tail, aliases and cache flags before formatting. These compose
with shared-entry calls and ordered EXPOSE through the same live pool binding.
Native fixtures remain read-only controls; unsupported BIF/source forms do not
become admitted merely to obtain a test.

The [plan](../PLAN.md#current-action-assignment) tracks assignment status and
remaining work. Whole LC-I-03 remains partial
for exact Classic diagnostic, resumed
condition/NOVALUE and full clause/TRACE dependencies under STEP04/06/07.
Existing native and shared B passes are controls, not inherited assignment
acceptance.

## NOP: childless statement without runtime work

[LC-AC-05/64](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria)
require childless NOP to preserve adjacent execution order and reject every
same-clause operand. The generated grammar already projects `Nop()` and the
frontend emits no operation for it. Structural traversal and NOP keep an
already-terminated block closed; only later emitting statements prepare a dead
block after their normal visit/budget check, preserving dead-source validation.
NOP checks cover authored AST spans, supported main/IF/group/while/private placements,
contextual NOP variable names and first-extra-token refusals, including dead
source. Reached and post-terminator NOP should add no IR value/effect, helper,
call, default, unreachable return path or frame demand against a valid control;
use empty DO-group wrappers for IF arms rather than an invalid missing statement.
Preserve truthful source records.

Reuse existing narrow frontend/backend tests and qualified tools/grammar;
no new runtime/schema/API or orchestration is needed. Native NOP fixtures are
read-only semantic controls; their historical ignored-tail bug is not an
acceptance target. Fresh-entry NOP consumes eligibility; exact Classic 21.1,
wider dynamic placements and clause/condition/TRACE retain STEP04/07/11 owners. Whole LC-I-04
stays partial until those dependencies close; the
[plan](../PLAN.md#current-action-nop) tracks that work. Dead-NOP construction adds no
unreachable return helper or frame slot, while following dead semantic errors
still refuse. Existing frontend/backend defaults retain the regression; no new
orchestrator, grammar, schema or runtime change was needed.

## OPTIONS: retained expressions and current empty processor

The [approved OPTIONS contract](architecture.md#approved-options-contract)
defines the source and execution checks. Each clause is
`COptions(value:expression?)`, including the first static header. Only a complete
first nonempty bare-word clause selects approved C/Classic/comment settings;
quoted/operator/call/later forms retain ordinary runtime expression semantics.
Request-local L comment preparation must recognize that header without requiring
successful default-policy lexing of later text that enabled line comments hide.
Raw operands and original Unicode/CRLF byte spans remain observable in the AST.

The existing B `RexxClassicConfig.applyOptions` accepts zero runtime words and
returns without scanning or conversion. The specialization introduced in C v3
omits that call and validates bare/simple String/Number operands without unused
value construction. Name reads, unary/binary operations, calls, required results,
errors, normal visits and budgets remain; dead malformed literals still refuse.
Reached/post-terminator blank controls must show no extra value, helper, frame
or dead return while retaining the same header and truthful source metadata.

The [plan](../PLAN.md#current-action-options) tracks OPTIONS status and
remaining work. Existing B no-word and generic L comment/budget contracts
remain shared dependencies. No runtime word, API,
family revision or native implementation is added. A future processor word or
effect requires C review, regression qualification and consumer recompilation.
Whole LC-I-05 retains full header controls/source maps, exact diagnostic
and conditions/TRACE dependencies; this increment does not
close those outcomes.

## IF: one condition and only the selected arm

IF checks cover the admitted IF/THEN/ELSE contract. Source parsing keeps original spans and binds ELSE to
the nearest open IF. An admitted instruction can occupy either arm; optional
operands must not consume their owner's contextual ELSE, while ordinary `else`
variable operands remain legal outside that boundary. Missing condition, THEN
or arm and malformed dead source still refuse before publishing output.

Evaluate the condition once, then perform one effectful Classic truth check.
Only exact text `0` and `1` is logical; other text raises 34.1 before either arm
or later output. An unchosen arm is not evaluated. A false IF without ELSE
continues normally. Exact literal branch facts may remove an unreachable arm
in IR-on, but must retain contextual truth and its effects/errors.

Source IR creates a join only when at least one arm endpoint remains open.
Both RETURN/EXIT arms and nested closed arms need no predecessorless join or
default absent-result epilogue; WHILE LEAVE/ITERATE retain their real targets.
Both source arms and later dead statements remain statically checked. Existing
per-EXIT fallback packets, owner/numeric setup and fatal handlers are separate
costs, not removable IF work. Internal calls share the caller pool until
PROCEDURE executes and binds a private one.

The [plan](../PLAN.md#current-action-if) tracks IF status and remaining work.
The optional-arm parser and lazy join construction retain source
validation and contextual effects. Whole LC-I-06 remains partial
for complete expressions, dynamic entries and exact static
diagnostics, resumed conditions/NOVALUE and clause/TRACE observations under
STEP04/05/06/07. Shared B truth and generic L algorithms retain their owners.
The approved SELECT increment is reviewed separately below.

## SELECT: ordered WHEN checks and source-backed no-match delivery

The [SELECT contract](architecture.md#approved-select-contract) owns the bounded
source and execution checks. CSelect retains ordered CWhen/COtherwise and original spans;
OTHERWISE is one optional final instruction list and may be empty. Each reached
WHEN evaluates once and performs contextual truth before its selected body.
Later tests/arms remain dormant after the first exact text `1`; exact text `0`
continues. Invalid values use the existing distinct 34.2 helper behavior.

The SELECT increment used C-owned family v4/43 operations. Its two additions are
`when.truth(value,context)` and attribute-free `select.missing`; the target adds
`whentruth` and `selectmissing` to its existing 23 exports. The no-match bridge
reuses shared 7.3 message/source and forwards its captured ERROR event with
signalorigin. Separate inline calls need not have equal physical PCs. Generated
C separately indexes the exact SELECT origin for the visible source line and
retains its nonresumable fatal policy. Fresh/loaded/dead missing operations
must have an owned exact nonempty valid source span before publication.

SELECT creates no loop frame. Only structurally open arm endpoints join;
LEAVE/ITERATE target the surrounding loop. Cold absent-result/core-return
closure after the divergent missing operation is required by the current IR,
not an unused join. Both source arms and later dead source remain validated.
The [plan](../PLAN.md#current-action-select) tracks status and remaining work. Whole
LC-I-07 retains complete expressions, dynamic transfers, exact diagnostic
recovery and condition/SIGL/clause/TRACE owners.

<a id="do-proposed-full-headers-and-explicit-loop-state-ownership"></a>
<a id="do-contract"></a>

## DO: approved full headers and explicit loop-state ownership

The [exact DO contract](architecture.md#approved-do-contract) covers
normalized headers/END names, counted/controlled/FOREVER and WHILE/UNTIL forms,
shared-runtime order/errors and unnamed transfers. Its state uses static
function-local text keys and hidden owned storage, not first-class borrowed
IR state values. All-record declaration checks and executable-CFG phase/LIFO
proof are distinct; checked dead source receives no invented live-owner facts.
Named END checks are included; the qualified v6 LEAVE increment below integrates
shared invalid-target delivery. The separately qualified ITERATE increment below uses that foundation. Complete expressions/exact recovery and
resumable conditions/SIGL/clause/TRACE remain open. Retain independent
source/CFG, metadata/protocol, runtime/error and caller/helper RXAS checks;
the admitted increment does not close whole-instruction or complete-C delivery.

<a id="leave-static-targets-and-request-policy"></a>

## LEAVE: static raw targets and request-local errors

The [approved transfer contract](architecture.md#approved-leave-iterate-contract)
owns the LEAVE-AC01–04 checks; the [plan](../PLAN.md#current-action-leave)
tracks status and remaining work. Optional names compare authored symbols
using shared Classic casing without tail substitution or runtime pool lookup;
nearest ELSE remains the existing IF authority even when a loop is named ELSE.
Valid edges close crossed states inner-to-outer including the selected loop,
then jump past its UNTIL/advance work. Ordinary C refuses provably invalid
transfers everywhere; static first-clause STRICTC defers 28.1/28.3 to reached
instructions and preserves the shared helper's raw event through the bridge.
C v6/52 operations and 33 exports were this LEAVE slice's exact surface; the
[C family contract](architecture.md#current-c-family-contract) retains PARSE and the
removal of the scalar ARG shortcut and PROCEDURE's shared entries. The
[current CALL contract](architecture.md#call-contract) advances it to C v10/61/36.
This LEAVE
checkpoint initially retained named ITERATE's profile refusal, now removed by
the separately qualified increment below. Dynamic loop invalidation,
resumable conditions, exact recovery
and clause/TRACE remain open; bounded local qualification does not close them.


<a id="iterate-selected-loop-end-work"></a>

## ITERATE: preserve the selected loop and enter its end work

The [approved transfer contract](architecture.md#approved-leave-iterate-contract)
owns the ITERATE-AC01–04 checks for the static nearest/named profile; the
[plan](../PLAN.md#current-action-iterate) tracks status and remaining work. Resolve raw names within this routine using shared
Classic casing; close only crossed inner states, retain the selected owner and
jump to its existing UNTIL/advance/count-entry path. True UNTIL skips advance;
false UNTIL and count exhaustion retain the final step. Groups/SELECT and
state-free wrappers add no target or owner. Calls enter with an empty loop
stack and cannot target caller loops, whether their pool is shared or private.

Default C rejects provable invalid targets even in dead source (28.2 without a
repetitive loop, 28.4 for a missing name); STRICTC signals only when reached.
Malformed syntax always refuses. The v6/52/33 foundation, header/admission and
shared transfer bridge were unchanged in this slice; current PARSE additions
use the current C contract. Retain source, CFG, paired execution, prior-output
and actual RXAS checks, including the shared LEAVE regression. Wider source/dynamic entries, SIGNAL/INTERPRET invalidation,
resumable conditions, exact recovery and clause/TRACE remain open.

## ARG: authored templates through the shared executor

The [approved ARG contract](architecture.md#proposed-arg-contract) owns the
requirements for this bounded LC-I-11 increment.
`CArg(templates:ParseTemplates)` retains the authored ARG instruction and exact
template/item origins, reusing every PARSE item shape. Private lowering supplies
source ARG and UPPER=1; no source token or second splitter is manufactured.
The obsolete ArgTarget/items path, scalar `arguments.read` operation and
`llctarget.readarg` body/export are removed in C v8/53 operations/33 exports.
Generic L `block.readarg`, LCP1 and shared B executor/source signatures are
unchanged; earlier research-C records are refused rather than upgraded.

Nonempty templates each acquire one raw activation ordinal and call the shared
executor once. Empty comma templates advance ordinal without acquisition or
execution; bare/comma-only ARG preserves terminated blocks without default
return/frame work. Full malformed/dead syntax remains checked. UPPER transforms
only the captured source once; raw patterns/names, originals/presence and later
PARSE ARG remain unchanged. Shared live-pool ordering governs closing versus
later operands, target/compound writes, exact remainders and codepoint positions.

The [plan](../PLAN.md#current-action-arg) tracks ARG status and remaining work;
native C remains a comparison control. Complete
configured NOVALUE/errors, dynamic activation, conditions/resumption,
clause/TRACE and declared delivery/platform obligations remain ARG-AC05 work;
bounded qualification does not close the whole instruction or complete C.
