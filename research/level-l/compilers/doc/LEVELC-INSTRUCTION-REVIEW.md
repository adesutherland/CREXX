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
| Evaluation/output order | Ordinary numeric/string expressions, IF/group and private-procedure call with inner output before outer output. `tests/say/forms-c.crexx` has independent expected bytes. | EXPOSE, shared-frame labels and general calls remain PROCEDURE/CALL dependencies. |
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
fatal-error handler. The pool and argument owners are unused by that example. These costs belong to the
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

Retained local qualification at `06a548321662e6dc4e91899363eb12831b4d952c`
passed the 40-command SAY workload: independent literal/order/error expectations,
both compiler IR modes and VMs with assembler optimisation disabled, all 256
ordinals and no runtime radix decoding. Focused frontend/product and Apple ASan
checks passed for this bounded repair. These are reused results, not a new run;
whole SAY stays open for required shared-expression/TRACE obligations above; deferred historical host edge parity does not block it. Review and
closure are separate states. The bounded DROP and assignment reviews below
follow that SAY disposition; LC-I-04 NOP is the current individual review.

## DROP: ordered names and borrowed pool effects

[LC-I-02](levelc-requirements.md#whole-instruction-work-units) reuses the existing
shared pool's direct and indirect operations. The approved C v3 contract adds
`CDrop` with ordered direct/indirect items and one `crexx.c.drop` operation per
item. Each operation consumes the borrowed context and carries raw name text
plus its indirect flag. It has no result and retains context/condition effects.
The compiler neither splits subsidiary words nor implements compound tails.

| Obligation | Implementation and independent expectation | Remaining boundary |
|---|---|---|
| Source names and item spans | The generated UTF-8 lexer supplies the same Unicode-letter/punctuation/dot rule to parsing and selector name admission. `tests/drop/contracts.crexx` checks micro-sign/Greek identity, parentheses-inclusive byte spans, ordered raw attributes and item origins. | Routine/call names retain the ASCII-undotted profile. Full symbols, shared labels and host name access retain STEP04/06/08 owners. |
| Contextual list boundaries | Source fixtures distinguish ordinary THEN/ELSE targets, a completed prior IF, a completed ELSE arm, a real nested outer ELSE and DO-contained lists. Following expressions resume ordinary concatenation. | Full IF/DO instruction review remains separate. |
| Ordered state | One direct item calls `dropSymbol` once. Each indirect item reads its carrier at that point, captures `symbolValue(...).asString(0)` once and calls `dropIndirectList` once. Independent output covers substitution after an earlier drop, self-carrier capture, arbitrary tails, defaults/tombstones and dropped versus empty. | NOVALUE policy and complete assignment/EXPOSE reviews remain open. |
| Shared aliases and configuration | A test-only exported C body is invoked by a B root with stable borrowed owners, exact lowercase compound-tail aliases and configured extra blanks. The parent uppercase tail survives whole-stem drop; the alias can subsequently restore the lowercase tail. Captured self-carrier lists continue after invalid words. | This proves prebound alias integration; it does not implement source EXPOSE or replace the shared pool's existing tests. |
| Refusal and atomicity | Malformed source, including unreachable source, preserves a previous output. Generic valid graphs with invalid reachable/dead load/store/drop names publish no RXAS and remain byte-identical. Existing schema/context tests reject previous/unknown C versions. | Full Classic diagnostic identity, resumable conditions and TRACE remain shared later obligations. |
| Caller and helper work | Structural checks require ordered existing drop/read/conversion calls, one capture and borrowed pool/configuration getters; inspect their receiver cleanup and existing pool bodies. A synthetic execution marker detects an accidentally entered C root wrapper in the B-root fixture. | Root owners, entry numeric setup, fatal dispatch and conservative temporaries retain C-OPT owners; no optimal-frame or speed claim follows. |

The focused normal Debug workload and its isolated scheduling measurement are
owned by `tests/check-drop.py` and DROP-AC01–04 in the current plan. The 40-command
positive component and 15 source refusals pass, with separate component timing;
no single successful full-workload timing is claimed. Existing C admission and
affected frontend checks also pass. Source EXPOSE, resumable conditions, complete
diagnostic identities and TRACE remain open, so whole LC-I-02 remains partial.
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

[ASSIGN-AC01–04](../PLAN.md#current-action-assignment) require independent source,
AST/IR, prebound-context and caller/helper RXAS proof. Source marker calls and
old-value reads establish once-only RHS-before-write order. An exported test IR
body mutates the supplied live context before its store; a B owner observes the
selected tail, aliases and cache flags before formatting. These are composed
proofs: private source calls have fresh pools and cannot mutate the parent by
inheriting a prebound alias. Do not copy native fixture EXPOSE, counted DO or
C2X forms into the admitted source profile merely to obtain a test.

The bounded review is locally qualified by the [composed normal Debug
checks](../PLAN.md#current-assignment-proof) and independent actual caller/fatal
RXAS inspection. No product change was needed. Whole LC-I-03 remains partial
for source EXPOSE/shared-label, exact Classic diagnostic, resumed
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
[NOP-AC01–04](../PLAN.md#current-action-nop)
qualify authored AST spans, supported main/IF/group/while/private placements,
contextual NOP variable names and first-extra-token refusals, including dead
source. Reached and post-terminator NOP should add no IR value/effect, helper,
call, default, unreachable return path or frame demand against a valid control;
use empty DO-group wrappers for IF arms rather than an invalid missing statement.
Preserve truthful source records.

Reuse existing narrow frontend/backend tests and qualified tools/grammar;
no new runtime/schema/API or orchestration is needed. Native NOP fixtures are
read-only semantic controls; their historical ignored-tail bug is not an
acceptance target. SELECT placements, shared labels, exact Classic 21.1 and
clause/condition/TRACE observations retain STEP04/05/06/07 owners. Whole LC-I-04
stays partial until those dependencies close. The bounded review is locally
qualified by the [focused Debug proof](../PLAN.md#current-nop-proof) and
independent actual RXAS comparison. The repaired dead-NOP construction adds no
unreachable return helper or frame slot, while following dead semantic errors
still refuse. Existing frontend/backend defaults retain the regression; no new
orchestrator, grammar, schema or runtime change was needed.
