# Level C requirements and implementation crosswalk

**Completion ledger for the new B/L compiler.** The [plan](../PLAN.md#c-first-delivery-milestone)
owns CFC acceptance and CLC sequencing; [architecture](architecture.md) owns design.
This ledger carries the native contract inventories forward, with separate new-C
status and closure checks. **No whole instruction, BIF or reference capability is
closed yet.** “Partial” means an admitted bounded route, “Not implemented” means
the new compiler lacks the route, and “Runtime only” means an existing B asset
still needs new-C integration. Source inspection is not execution qualification.

Current design baseline is `e0b11db9e7293bc1f45e86574cb1f857edb1c038`, including
the committed shared PARSE cursor. The plan records bounded unchanged-input
local qualification and remaining real platform proof. Existing native,
direct B and G fixtures are reusable controls; their passes never become new-C
passes without exercising the new compiler.

## Authorities and accepted boundaries

- [Classic compliance reference](../../../../compiler/docs/levelc_compliance_reference.md),
  [native working architecture](../../../../compiler/docs/levelc_working_architecture.md),
  [compatibility-layer review](../../../../compiler/docs/levelc_compatibility_layer.md),
  [BIF catalogue](../../../../compiler/docs/levelc_classic_bifs.md), and the
  [current compatibility worklist](../../../../docs/planning/release-1/levelc-compatibility-worklist.md).
  The worklist's current decisions supersede older partial-status paragraphs.
- [Unicode/stream contract](../../../../compiler/docs/levelc_unicode_and_streams.md):
  exact Unicode scalar text/codepoint positions, UTF-8 source, fixed Latin-1
  ordinal bridge, simple symbol casing, no implicit normalization/graphemes;
  explicit Unicode algorithms and raw/encoded streams remain admitted. Ordinary
  C text uses Unicode codepoints. Codes 0–255 preserve the fixed Latin-1 bridge;
  agreed Unicode behavior above U+00FF is intentional, not a historical byte
  compatibility defect.
- External C calls retain the static signed B/G provider contract, including
  documented missing-provider timing/error differences. No general late dynamic
  provider search or new foreign-language interface is inferred.
- PARSE EXTERNAL/NUMERIC remain excluded from Adrian's current C scope;
  their final Release 1 disposition is still an explicit decision (LC-GAP-05). TRACE retains Adrian's practical scope and documented interactive,
  skip/suppress, SCAN and event-coverage divergences; verify their exact current
  behavior rather than claiming complete reference TRACE.
- Host ADDRESS NUL transport, full host/source/configuration APIs and real HALT
  are separate tracked obligations. Their prior deferral is not full-C closure.
  The selected NUL rejection/transport policy must be explicit at the adapter.
- INTERPRET is required, last. The source-group, activation, transfer/resumption
  and resident-code lifetime contract is LC-87-01–05. No label clauses inside
  generated text; caller labels/routines remain reachable under that contract.

No new syntax, normalization default, arithmetic representation, reference
exception or platform exclusion is approved by writing this design.

## Current new-compiler baseline

The independent `levelc/levelc.rxl` / `llcparse` / `llcfrontend` pipeline admits
bounded scalar expressions/control, private PROCEDURE-labelled routines,
complete-template ARG and LENGTH/ARG/DIGITS. The approved DROP increment adds ordered
direct/indirect lists and shared Unicode/raw variable names, including ARG
destinations; routine/call names retain the ASCII-undotted profile. Full Classic
source, operators, shared labels, pools, conditions and host contracts are open.
SAY's adjacent radix-literal repair has retained proof, not whole-SAY closure.

The current implementation selects one `llctarget` borrowed-context surface and
C-family v8/53 operations and 33 target exports with one required context-reference procedure argument,
contextual DROP/WHEN truth, source-indexed SELECT no-match handling and keyed
frame-owned DO state. The approved PARSE implementation adds all seven source
forms and immutable templates through the shared cursor/B executor; its focused
C profile is locally qualified. Bounded static DO is locally qualified; whole-C/shared
dependencies remain open. Root/private owners
and numeric entry setup remain conservative; argument/result transport remains
by value. The shared B `RexxClassicContext` is independently qualified. STEP03d
owns the qualified v2 baseline; DROP-AC01–04 own the v3 increment and
SELECT-AC01–04 own the v4 increment; [DO-AC01–04](../PLAN.md#current-action-do)
own the qualified v5 increment. [LEAVE-AC01–04](../PLAN.md#current-action-leave)
own locally qualified v6/52-op/33-export integration. Complete shared-label activation
remains open. Earlier research C images/IR must be rebuilt, with no compatibility
bridge. Neutral scalar ABI records still use `crexx.g` names.

Shared B Unicode extraction/public API and the existing eight stream/23 Unicode
BIF bodies are completed reusable assets. Remaining work is C recognition,
binding/context/host integration and missing named contracts, not a second
Unicode extraction or BIF algorithm implementation.

## Shared requirements before instruction closure

| ID / stage | Design and required proof |
|---|---|
| C-SRC / STEP04 | Owned exact source bytes and logical mapping; contextual instruction words, quoted names, nested comments, continuations, radix boundaries, all whitespace/Unicode symbol rules and configured additions. Preserve byte spans and scalar columns separately; NUL is data, not EOF. Compare located static errors including unreachable text, 6.*, 22.1, 50.1/51.1 and complete clause inventories. |
| C-EXPR / STEP04 | Named AST nodes and C IR for constants, reads, compound names, calls and every prefix/power/arithmetic/concatenation/comparison/logical operator. Left-associative Classic power; correct prefix precedence; eager logical operands; exact logical values with context-specific 34.*. RHS and compound substitution order must be explicit. Use side effects and independently calculated values, not only native equality. |
| C-NUM / STEP03/04/07 | Shared B values/numeric context retain arbitrary precision semantics, rounding, DIGITS/FUZZ/FORM, spelling and LOSTDIGITS. Direct operation entries carry borrowed context. Cover overflow/invalid whole arguments, guard digits, negative quotient/remainder, exponent and conversion boundaries; signed64 BIF WHOLE positional/count-argument limit and arbitrary-precision radix rules remain distinct. |
| C-POOL / STEP03/05/06 | Runtime owns dropped/implicit/scalar/stem/compound states, recursive aliases and source-ordered exposure. Compiler emits ordered name evaluation/pool effects. Property tests include DROP versus empty, stem propagation, tail substitution without NOVALUE and final read with NOVALUE; callbacks and VALUE invalidate local facts. |
| C-ACT / STEP03/06/07 | One source-unit label/body model, live replaceable pool binding, argument presence, invocation kind, RESULT, program exit and defined inherited/restored state. Loops, labels and handlers do not become unrelated private procedures. Verify recursion, fall-through and first-label precedence. |
| C-ABI / STEP01/03/06 | B implementation reads/writes admitted metadata with exact signatures, optional presence and ownership. Constants retain original metadata identities; no native reader fallback. Independent producer/consumer tests, incompatible/malformed imports and installed absence-of-native-source-compiler controls. |
| C-OBS / STEP03/07 | Explicit clause/source/TRACE and failure effects prevent illegal movement/removal. Actual condition identity/origin belongs to runtime; optimizer must preserve observer order and state. One sample per observed DATE/TIME clause and program elapsed/reset state; no unconditional per-clause clock read when unused. |
| C-HOST / STEP08 | Execution-owned configuration and host adapter contracts: invocation/result, queues, streams, environment callbacks, pool access windows, clock/HALT, source inventory and resource errors. Concurrent contexts must not share mutable program state. APIs distinguish omitted/empty/NUL and forbid stale callback access. |
| C-UNI / STEP03/04 | Shared B Unicode core/API, one data owner, preserved G facade and acyclic bootstrap are qualified foundations. Integrate new-C source/name/simple-casing and BF04 bindings with C error/ordinal/presence rules. Introduce property/simple-map/traversal/budget APIs only for a demonstrated consumer need and exact contract approval; existing pinned data/helpers may suffice. |

## Whole-instruction work units

Each LC-I row is a separate implementation/review assignment. STEP05–07 are
sequence groups, not permission to combine all instructions into one prompt.
All whole-instruction rows remain open. Review/disposition proceeds in LC-I order;
record unfinished host/TRACE dependencies explicitly while advancing to the next
review, rather than creating a STEP05 → STEP07/08 → STEP05 cycle.
Worker output includes grammar/AST, C IR and effect/control schema, runtime ABI,
independent positive/error expectations, source/TRACE behavior and representative
RXAS/helper inspection. Required dependencies introduced early remain owned by
their full instruction review; e.g. ARG reuses PARSE infrastructure.

| Contract / stage | New C status | Next closure check (full row remains open) | Detailed native contract to reuse |
|---|---|---|---|
| LC-I-01 SAY / STEP05 | Partial: radix fix retained | `Say(value?)` evaluates its optional expression once and produces one ordered output effect: bare line, visible Unicode/numeric text, output/evaluation order and ordinary errors through the current route. Further historical processor/callback/byte-exact host edge parity is deferred and unclaimed; preserve existing useful controls. Literal-only RXAS has no runtime radix decode; unused context/pool setup needs C-OPT-01 proof. | [LC-AC-57/60](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) |
| LC-I-02 DROP / STEP05 | Partial: ordered v3 forms locally qualified | Direct/indirect lists, arbitrary compounds, dropped/default/empty states, Unicode/configured blanks, private pools and prebound aliases pass independent state/origin/RXAS checks. One captured indirect list uses existing invalid-word continuation. Source EXPOSE, resumable conditions, full diagnostic identities and TRACE remain STEP06/07 dependencies. | [LC-AC-07/53/55/56/62](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) |
| LC-I-03 assignment / STEP05 | Partial: existing one-write forms locally qualified | Independent source/AST/IR, prebound-context and RXAS checks cover once-only RHS before raw-name storage, omitted RHS, arbitrary tails, stem reset/tombstones, private pools, aliases and exact Unicode/NUL/cache copies. Source-call ordering and same-context mutation are composed proofs; source EXPOSE/shared labels, exact diagnostic identities, resumed conditions/NOVALUE and full TRACE remain open. | [LC-AC-54/63](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) |
| LC-I-04 NOP / STEP05 | Partial: childless forms locally qualified | Main/IF/group/while/private placements, contextual variable, first-extra-token/dead refusals and unchanged NOP/blank IR/instruction/frame shape pass. Repaired dead-NOP construction preserves later static validation. SELECT/shared labels, exact 21.1 and clause/TRACE/condition observations remain open. | [LC-AC-05/64](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) |
| LC-I-05 OPTIONS / STEP05 | Partial: bounded COptions forms locally qualified | Retain every optional operand, including header Names; select settings only from a complete first bare-word clause using request-local comments. Current zero-word B processor is specialized away; preserve operand effects/errors and validate discarded literals/dead source. Request-local static STRICTC is integrated for LEAVE/ITERATE; full header/source-map, shared-label and conditions/TRACE work remains open under [OPTIONS-AC01–04](../PLAN.md#current-action-options). | [LC-AC-66](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) |
| LC-I-06 IF / STEP05 | Partial: admitted forms locally qualified | Once-only condition/truth, lazy selected arms/no ELSE, exact text 0/1 and fatal 34.1, nearest/contextual ELSE and optional/default arm minima pass. Both closed arms add no join/default epilogue; dead validation and source remain. SELECT/full DO/shared labels/EXPOSE, full expressions/exact static diagnostics, resumed conditions/NOVALUE and clause/TRACE stay open. | [LC-AC-02/67](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) |
| LC-I-07 SELECT / STEP05 | Partial: admitted forms locally qualified | Ordered lazy WHEN edges, full/empty OTHERWISE, nested/loop/private arms and source-indexed 34.2/7.3 use shared B bodies. [SELECT-AC01–04](../PLAN.md#current-select-proof) pass current focused proof. Shared labels/fall-through/EXPOSE, complete expressions/DO/named transfers, full diagnostic recovery and condition/SIGL/clause/TRACE remain open. | [LC-AC-09/68](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) |
| LC-I-08 DO / STEP05 | Partial: admitted static forms locally qualified | [DO-AC01–04](../PLAN.md#current-do-proof) qualify counted/controlled/FOREVER/WHILE/UNTIL forms, ordered once-only headers, arbitrary counts, live compound controls, END names and shared errors. Existing shared B state owns numeric/pool algorithms; C owns finite keyed protocol admission and frame cleanup. Source EXPOSE/shared labels, complete expressions, exact recovery/resumable conditions and clause/TRACE remain open. | [LC-AC-03/10–15/17–35/65](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) |
| LC-I-09 LEAVE / STEP05 | Partial: named static forms locally qualified | [LEAVE-AC01–04](../PLAN.md#current-leave-proof) qualify raw nearest/named/shadowed targets within the current routine, crossed-state cleanup without end work, exact default/dead versus STRICTC reached-time 28.1/28.3 and no runtime target search. Shared labels/EXPOSE, SIGNAL/INTERPRET invalidation, exact recovery/resumable conditions and clause/TRACE stay open. | [LC-AC-69](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) |
| LC-I-10 ITERATE / STEP05 | Partial: named static forms locally qualified | [ITERATE-AC01–04](../PLAN.md#current-iterate-proof) qualify nearest/named raw targets, selected-owner retention and UNTIL/advance/count-entry order, including final FOR advance, live compound tails and default/STRICTC 28.2/28.4. Shared labels/EXPOSE, dynamic loop invalidation, full recovery/resumable conditions and clause/TRACE remain open. | [LC-AC-70](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) |
| LC-I-11 ARG / STEP06 | Partial: complete-template profile locally qualified | [ARG-AC01–05](../PLAN.md#current-action-arg) require `PARSE UPPER ARG` through the shared executor: arbitrary comma/templates, exact presence/raw arguments, live pool order, Unicode and authored errors/RXAS. The [approved AST/IR/export contract](architecture.md#proposed-arg-contract) uses CArg templates and C v8/53 operations/33 exports, retiring the scalar shortcut. Shared labels/EXPOSE, complete conditions/TRACE and delivery/platform closure remain open. No second splitter or research-C compatibility path. | [LC-AC-38/71](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) |
| LC-I-12 PROCEDURE / STEP06 | Partial: private routine subset | First-instruction eligibility, new pool and direct/indirect source-ordered EXPOSE for scalar/stem/resolved compound aliases. Pool binding replaces live view; recursive/callback/INTERPRET eligibility proof retained. No mandatory PROCEDURE after all labels. | [LC-AC-54](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria); [LC-74-01–05](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps) |
| LC-I-13 CALL / STEP06 | Partial: local private routine subset | Local/BIF/static signed external resolution, quoted/VALUE forms, omitted positions, shared-label code and delayed CALL ON/OFF policy. Lower to checked call/handler effects, preserve RESULT drop/value, 43.1/16.1 and provider boundary. Inspect call windows, copies and handler suspension. | [LC-75-01–06](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps) |
| LC-I-14 RETURN / STEP06 | Partial: value/no-value subset | Explicit completion with result presence, function-required 45.1, subroutine RESULT and outermost host status. One code body, recursive cleanup and condition propagation; empty string is a present result. | [LC-76-01–05](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps) |
| LC-I-15 EXIT / STEP06 | Partial: basic return-like path | Program-root termination across internal/external/handler frames, optional value, EOF/fall-through and host completion. Propagate an exit outcome to correct owner and finalise once; do not silently return to a caller. | [LC-77-01–06](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps) |
| LC-I-16 PULL / STEP06 | Not implemented | Acquire selected queue once, otherwise configured input, then shared UPPER PARSE executor. Empty/EOF/NOTREADY, NUL and queue selection, nested contexts; no duplicated transport or template engine. | [LC-78-01–05](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps) |
| LC-I-17 PUSH / STEP06 | Not implemented | Evaluate optional expression once, null for bare form, selected queue front insertion. Effects/order, Unicode/NUL and nested isolation; share queue adapter with PULL/QUEUE. | [LC-79-01–04](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps) |
| LC-I-18 QUEUE / STEP06 | Not implemented | Same input semantics as PUSH, tail insertion; FIFO ordering, count/nonconsumption and shared selection. Check linked/host lifecycle; no global queue chosen accidentally. | [LC-80-01–04](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps) |
| LC-I-19 PARSE / STEP06 | Partial: finite C integration locally qualified | ARG/PULL/SOURCE/LINEIN/VERSION/VALUE/VAR, UPPER, empty/arbitrary comma lists, targets/drops, literal/variable patterns and absolute/relative positions. [CPARSE-AC01–08](architecture.md#proposed-c-parse-integration) specify independent source/order/error/Unicode, v3 section normalization, shared B word policy, packet/ABI, cleanup, observations and delivery proof. Complete conditions/TRACE/platform obligations remain open; existing cursor/B proof alone does not close this row. | [LC-AC-39/40/45–52](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria); [LC-81-01–07](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps) |
| LC-I-20 ADDRESS / STEP07 | Not implemented | Selection/swap, transient command, WITH resources and configured environment object protocol. One command evaluation, RC/.RC/.RS, error/failure/notready and resource lifetime. Compiler owns anchors; adapter owns command meaning. Test NUL transport policy explicitly. | [LC-82-01–06](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps) |
| LC-I-21 implicit command / STEP07 | Not implemented | Expression becomes one command through active ADDRESS state and lasting connections. No duplicate host route. Preserve expression errors, warning/source observations, return status and traps. | [LC-83-01–04](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps) |
| LC-I-22 NUMERIC / STEP07 | Partial | DIGITS/FORM/FUZZ default/value forms, once-only expression validation, local inheritance/restoration and BIF/display/arithmetic effects. Invalidate numeric facts; no per-operator rebuilding of unchanged configuration. | [LC-84-01–05](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps) |
| LC-I-23 SIGNAL / STEP07 | Not implemented | Direct/quoted/VALUE targets, first-label order, ON/OFF seven conditions, immediate handler transfer, SIGL/.SIGL and loop-state clearing. Compiler owns labels/edges; runtime owns policy/delivery. Missing labels and errors use causing clause; preserve existing signalorigin/CLASSIC_CONDITION contracts. | [LC-AC-76](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria); [LC-85-01–05](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps) |
| LC-I-24 TRACE / STEP07 | Not implemented | Compile clause/result events with current VM metadata/exit protocol, share activation option/query state and command inhibition. Verify actual displayed values/origins and approved practical divergences. Disabled tracing must not require avoidable formatting/allocation. | [LC-AC-77](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria); [LC-86-01–05](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps) |
| LC-I-25 INTERPRET / STEP11 | Not implemented | Same compiler in group mode, live caller state, source-once semantics, complete/no-label groups, nested compile/execute, CALL-handler resumption versus SIGNAL discard, caller routines, RETURN/EXIT and bounded resident ownership. Include all LC-87 proof cases and installed/platform lifecycle; no second evaluator. | [LC-87-01–05](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps) |

The approved PARSE contract preserves complete LC-I-19 as the outcome. The committed
generic engine retains **PARSE-ENGINE-AC01–06** and its open platform boundary;
the v3 descriptor/C v7/B signatures received separate approval on 20 October
2026. CPARSE-STEP01 is complete; STEP02 has local normal Debug/focused Apple
ASan proof. STEP03/04 have bounded shared-B and new-C qualification.
STEP05 remains open for complete observations, delivery and real platforms.
Forward section/ASCII-word probes do not prove backward/empty-pattern or
Unicode/configured-blank semantics. Existing shared CharacterScan owns word
policy; a new compiler/VM scanner is not commissioned. Fatal text-error proof
cannot replace required NOVALUE, typed conditions, resumed handlers or actual
practical TRACE observations: **CPARSE-AC07 / C-OBS remain open** until their
STEP03/07 owners integrate them. ARG and PULL reuse the executor but retain
their individual LC-I-11/16 acceptance. All twelve CFC criteria and last-stage
INTERPRET remain open.

## SAY dependency packet — CLC-STEP01b

This is the first concrete instruction dependency packet, inspected at local
`975afbc15` on 19 October 2026. It does not close LC-I-01. The [bounded baseline qualification](../PLAN.md#completed-cabi-proof-and-limits) records unchanged-input
proof; the [SAY review](LEVELC-INSTRUCTION-REVIEW.md) retains its grammar/RXAS
findings. This packet adds no new execution verdict. Native whole-SAY acceptance remains a reusable control, not a new-C pass.

Paths under `tests/` below are relative to `research/level-l/compilers/`.

| Dependency | Current implementation / reusable independent expectations | Remaining closure obligation and owner |
|---|---|---|
| **SAY-D01 — C-SRC** | `levelc/llcparse.crexx` fuses adjacent X/B suffixes and preserves authored spelling/ranges; `llcfrontend.ordinalliteral` decodes them into ordinal constants in the existing String/Say path. `tests/say/forms-c.crexx`, `literal-c.crexx` and the invalid-literal rows in `tests/check-say.py` cover empty/partial groups, Unicode/NUL, adjacency and located refusal. The retained focused radix proof includes all 256 ordinals. | Complete C token/clause admission and static validation of all unreachable source under STEP04. A passing literal subset does not establish the whole expression/source contract. |
| **SAY-D02 — C-EXPR / BF01–05** | `llcfrontend.statement` evaluates the optional operand once, then appends one `crexx.c.say`; an omitted operand supplies the empty value. `tests/say/forms-c.crexx` expects nested `inner` output before `outer=done`; `error-c.crexx` expects `before`, then arithmetic failure with no later output. Retained new-C proof covers both IR modes/VMs with assembler optimisation disabled. Native `compiler/tests/rexx_src/levelc_say_instruction.rexx` supplies an additional counter/recursive-output oracle. | Complete operators, compound reads, BIF and call forms, missing/empty presence and errors using shared runtime bodies. Port reusable cases through the new compiler as their prerequisites become available; do not inherit native closure. |
| **SAY-D03 — C-POOL / C-ACT / C-NUM** | `llcfrontend.lowerall` threads one context reference through each routine; `levelc/runtime/llctarget.crexx` borrows activation/pool/configuration and reuses existing BIF bodies. The selector creates caller-owned private activation/pool bundles and installs validated numeric settings in the executing frame. `tests/backend/calls-c.crexx`, `arguments-c.crexx` and `numeric-c.crexx` have independent expected rows in `tests/backend/cases.json`; native `levelc_say_pool_reads.rexx` and `levelc_say_bif_family.rexx` are reusable controls. | STEP03d qualifies the borrowed selector migration separately from complete shared-label activation and configured text/numeric behavior. Current private PROCEDURE routines and copied argument/result packets do not close those wider contracts. |
| **SAY-D04 — C-HOST output** | `llcrxas` selects `llctarget.sayvalue`; that adapter performs `say value.asString(1)`. Existing native `compiler/tests/src/test_levelc_say_host_output.c` supplies exact NUL/high-scalar bytes and six callback invocations from `levelc_say_bytes.rexx`; `levelc_say_host_output` registers that control. | Modern SAY requires a bare line, once-only optional expression, visible Unicode/numeric text, order and ordinary errors through the current output route. Further historical processor/callback/byte-exact host edge parity is deferred and unclaimed, not a new matrix or completion blocker. Retain existing controls; record a specific non-conformance only when observed. Source NUL, argument lengths, streams and ADDRESS host contracts remain separately required. |
| **SAY-D05 — C-OBS / conditions / TRACE** | `levelc/llcir.crexx` declares SAY state `output`, failure `conversion.io`, ordinary continuation `returns`; generic optimisation retains it. `tests/backend-tests.crexx` and `tests/check-provenance.py` check existing logical identities/origins, and `tests/say/error-c.crexx` checks the reached fatal-error ordering. | Explicit authored clause identity, trace/result observations, actual condition origin and trapped/resumed failure must be preserved under STEP03/07. Source ranges and a conservative effect summary alone do not establish those semantics. New exceptional/resumable L edges require their own approved contract. |
| **SAY-D06 — C-OPT-01 / target quality** | `tests/check-say.py` rejects literal X/B pool reads/runtime decoding; the retained review distinguishes complete RXAS from helper work. `llcrxas` still unconditionally constructs root and private-call owners and installs the entry context; it does not specialise unused ownership setup. | C-owned requirement analysis must justify omitting unused setup for constant-only SAY, with read/write/call/RESULT/observation negatives. Preserve allocation, ABI, failure and lifetime obligations; do not make generic L discard named allocation/state effects. Register reuse remains the separate L/target contract. |
| **SAY-D07 — delivery / independent foundations** | `llcompiler_c_only_{opt,noopt}_{rxbvm,rxtvm}` reuses the SAY expectations in separate B/L compiler-host and static-runtime roots with native compilers/G absent. `tests/check-say.py` enumerates the explicit target libraries, including the B Unicode core; the compiler host does not acquire that runtime. Independent `ts_llirflow`, `ts_llirservices` and `ts_lloptimise` cover normal flow, effects, owner/revision invalidation and atomic limits. | Preserve exact producer/import/install inventories and both-mode/engine proof as this packet grows. Required host/platform and full CFC obligations remain open; current desktop fixture success establishes neither complete SAY nor complete C. INTERPRET remains STEP11. |

## BIF integration and verification packages

**Reuse the existing BIF implementation.** All 95 native table names already
have runtime entries: the 70-name catalogue, LOWER/UPPER and 23 Unicode adapters.
The current compatibility worklist records the admitted library baseline and
its existing qualification. This plan does not commission 95 new BIF bodies,
repeat their semantic design, or reopen unchanged library qualification.

The new compiler currently admits only LENGTH, ARG and DIGITS, with restricted
forms. Its work is to bind the other 92 names and finish admitted call forms;
evaluate arguments in order with omitted/empty presence; pass the existing
runtime ABI and live activation context; and preserve return/error/condition
and source behavior. Reuse one checked call mechanism and existing B entrypoints
rather than add a separate compiler implementation of each BIF.

BF01–09 below are integration/verification groups. Reuse existing independent
expected results and fixtures through the new compiler, extending tests only
where the new call path, activation handling or changed packaging needs proof.
Unchanged direct-library evidence remains valid; new-compiler execution is the
new proof obligation. Context-sensitive BIFs close alongside their owning
instruction/host service, not through duplicate library work.

Library changes require a specific reproduced defect, an explicitly identified
unfinished contract from the owning worklist, or the approved shared-Unicode
packaging/API extraction. Wider host/HALT/source/resource obligations remain
with those owners; their existence is not a blanket BIF rewrite mandate. No new
BIF-body defect was established by this architecture review. The cases below
identify observable contracts, not a direction to rebuild their algorithms.

### BF01 — STEP04

`ABBREV`, `CENTER`, `CENTRE`, `CHANGESTR`, `COMPARE`, `COPIES`, `COUNTSTR`, `DELSTR`, `INSERT`, `LASTPOS`, `LEFT`, `LENGTH`, `OVERLAY`, `POS`, `REVERSE`, `RIGHT`, `STRIP`, `SUBSTR`, `TRANSLATE`, `VERIFY`, `LOWER`, `UPPER`.

Text/ordinal defaults, pad and range rules, codepoint indexing, exact comparisons and simple versus full casing; zero/negative/large positions and high ordinals. Calls select existing B entries, no C-specific string engine.

### BF02 — STEP04

`DELWORD`, `SPACE`, `SUBWORD`, `WORD`, `WORDINDEX`, `WORDLENGTH`, `WORDPOS`, `WORDS`.

Configured blanks and codepoint word boundaries, count/index/pad behavior and empty inputs; shared scanner helpers, not repeated compiler slicing.

### BF03 — STEP04

`ABS`, `DATATYPE`, `FORMAT`, `MAX`, `MIN`, `SIGN`, `TRUNC`, `B2X`, `BITAND`, `BITOR`, `BITXOR`, `C2D`, `C2X`, `D2C`, `D2X`, `X2B`, `X2C`, `X2D`, `XRANGE`.

Exact numeric/rounding/context and byte-ordinal conversion, width/sign/grouping, all 256 ordinals, >U+00FF rejection where required, configured digits and limits. Independently computed edge results and condition identities.

### BF04 — STEP03/04

`UNICODEVERSION`, `TONFD`, `TONFC`, `TONFKD`, `TONFKC`, `ISNFD`, `ISNFC`, `ISNFKD`, `ISNFKC`, `TOUPPERCASE`, `TOLOWERCASE`, `TOCASEFOLD`, `TOSIMPLECASEFOLD`, `TOTURKICCASEFOLD`, `TOTURKICSIMPLECASEFOLD`, `GRAPHEMECOUNT`, `GRAPHEMEREVERSE`, `GRAPHEMESUBSTR`, `GRAPHEMEPOS`, `ENCODE`, `DECODE`, `ISDECODABLE`, `ISENCODINGSUPPORTED`.

Shared B Unicode core with pinned data; C presence/error/ordinal adapters. UCD conformance, combining sequences, expansions, Turkic controls, supplementary scalars, malformed codecs and explicit replacement; no implicit normalization.

### BF05 — STEP04/06/07

`ARG`, `DIGITS`, `FORM`, `FUZZ`, `VALUE`, `SYMBOL`.

One live activation/pool owner, omitted versus empty and argument count/presence; current numeric state, compound resolution, pool mutation/query and invalidation. Validate host-selected pool behavior separately in STEP08.

### BF06 — STEP06

`QUEUED`.

Selected execution queue count without consuming, mixed PUSH/QUEUE/PULL, nested and isolated host contexts; no process-global substitute.

### BF07 — STEP07

`ADDRESS`, `CONDITION`, `ERRORTEXT`, `SOURCELINE`, `TRACE`.

Same environment/trap/trace/source state as instructions; condition fields and caller isolation, physical/mapped lines including NUL, message identity/catalog and errors. No hidden source inventory or stale state cache.

### BF08 — STEP06/08

`CHARIN`, `CHAROUT`, `CHARS`, `LINEIN`, `LINEOUT`, `LINES`, `QUALIFY`, `STREAM`.

Eight stream functions use execution-owned raw/encoded stream adapters; positions/counts/EOF/close/NOTREADY, queries and ownership. Byte counts versus text positions, path failures and current host/codec contracts.

### BF09 — STEP07/08

`DATE`, `TIME`, `RANDOM`.

Clause-stable clock samples with program elapsed/reset state, calendars/conversions/timezones and injectable error clocks; RANDOM range/seed/state contracts and isolation. Deterministic tests plus named live-host checks, not timing assumptions.

### Exact runtime binding inventory — CLC-STEP01b

Reviewed against the current source inputs above, this inventory lists all
**95 names exactly once**: 70 catalogue names, LOWER/UPPER
and 23 Unicode extensions. The authority for selected entries is
[`levelc_direct_bifs`](../../../../compiler/rxcp_levelc_lower.c); implementations
are in [`lib/rxfnsc`](../../../../lib/rxfnsc/). This inventory records existing
ABI and dependencies; it does not authorise a new API or reimplement a BIF.

Every selected entry returns `.RexxValue` and takes a mandatory first
`context_ref = reference .rexxclassicbifs..RexxBifCallContext`. It carries the
BIF name, ordered `.RexxValue[]` operands, a separate presence vector and count,
current configuration and error transport. Explicit empty and omitted operands
must remain distinct. ADDRESS, ARG, CONDITION and TRACE additionally take
`activation_ref = reference .rexxactivation..RexxActivationArguments`, in that
order. The table names the actual frame entries for ADDRESS/TRACE, the CENTRE
alias in the CENTER module, and the common BIT*/LOWER/UPPER entries; selecting a
similarly named legacy helper is not sufficient.

The common context can hold borrowed pool/configuration/activation references.
The requirements below identify state that binding must preserve, not a request
to construct or copy a whole activation for every call. Numeric wrappers retain
their authored `numeric ... inherited` settings; merely matching a signature
cannot establish inherited NUMERIC, observer or host semantics.

| Context code | Required state beyond ordinary operands/presence/error transport |
|---|---|
| `T` | Current text configuration: exact codepoints, admitted character/case/comparison rules and padding/ranges used by the entry. No implicit normalization. |
| `W` | `T` plus configured word/blank scanning. |
| `O` | Fixed Latin-1 ordinal bridge and rejection above U+00FF for byte-valued operations; radix grouping/width remains entry-owned. |
| `N` | Caller NUMERIC state required by the entry and its argument checks; preserve inherited DIGITS/FORM/FUZZ, rounding and condition behavior where applicable. |
| `A` | Explicit second activation reference: argument state, current condition or frame ADDRESS/TRACE, according to the entry. |
| `P` | Live visible pool and text-name configuration; VALUE may mutate it or access a configured external pool. |
| `K` | Context-bound activation clause clock and shared elapsed/reset state; direct harnesses may use the existing injected pool clock. |
| `Q` | Execution-selected queue through current configuration; query must not consume it. |
| `R` | Execution/configuration-owned RNG state and explicit seed mutation. |
| `S` | Retained source inventory supplied through configuration, including exact line/count/error behavior. |
| `H` | Execution-owned stream/configuration/provider state; qualification/path policy, codec, lifecycle and NOTREADY/error transport. |
| `U` | Shared B Unicode provider with existing C text/ordinal, optional-presence and error adapters. |
| `—` | No additional state identified here; the common context and entry-specific checks still apply. ERRORTEXT uses the shared message catalog. |

Fixture abbreviations name maintained inputs with authored expected results:

- `B/Foo` means [`lib/rxfnsc/tests_functional/testRexxClassicBifFoo.crexx`](../../../../lib/rxfnsc/tests_functional/).
  `B/Numeric` and `B/Bitwise` are the existing shared harnesses;
  `B/Bifs` denotes `testRexxClassicBifs.crexx` in that directory.
- `CU` means [`compiler/tests/rexx_src/levelc_unicode_services.rexx`](../../../../compiler/tests/rexx_src/levelc_unicode_services.rexx)
  and its `.expected`, plus [`classic_unicode_contract.py`](../../../../compiler/tests/classic_unicode_contract.py)
  for ordinals/replacement omission and the independent public B
  [`ts_unicode_public.crexx`](../../../../lib/rxfnsb/unicode/tests/ts_unicode_public.crexx).
- `CS` means the stream cases of [`classic_unicode_contract.py`](../../../../compiler/tests/classic_unicode_contract.py)
  named in the [Unicode/stream crosswalk](../../../../compiler/docs/levelc_unicode_and_streams.md#regression-crosswalk),
  [`test_classic_stream_lifecycle.c`](../../../../compiler/tests/src/test_classic_stream_lifecycle.c)
  and [`classic_stream_package.rexx`](../../../../tests/rxpa/classic_stream_package.rexx).
  These provide reusable semantics/lifecycle/byte expectations, not coverage of every stream case.

**Partial** in the table means only an admitted route exists:
LENGTH takes one actual, DIGITS none, ARG none or two present actuals. Their
existing new-C controls are `tests/backend/arguments-c.crexx`, `argv-c.crexx`,
`numeric-c.crexx` and `tests/frontend-tests.crexx`; emission uses current
`llctarget` adapters, not a completed generic direct-BIF binding. **Unbound** means the
name is not bound by the new frontend. Local-routine shadowing remains distinct.
All 95 whole BIF integration obligations remain open. The cited B/native/G
fixtures may be reused as oracles; their existing passes are never new-C passes.

Some direct harnesses exercise a different retained entry: `B/Address` and
`B/Trace` test pool-oriented clients, while the catalogue selects activation
frame entries. Reuse native [`levelc_address_whole.rexx`](../../../../compiler/tests/rexx_src/levelc_address_whole.rexx)
with its `.expected` and [`levelc_trace_values.rexx`](../../../../compiler/tests/rexx_src/levelc_trace_values.rexx)
with its opt/no-opt goldens for that distinction. `B/Subword` calls the common
helper rather than the selected standalone entry; the compiled
`levelc_bif_reference_audit.rexx`/`.expected` is an additional control. These
differences must remain visible when attributing later new-C results.

**BIF-CHECK — closure for each named row:** route a separately compiled new-C
call to its listed existing B entry; verify source-ordered operands, omitted
versus empty, defaults/arity/ranges, declared context inheritance/restoration,
values/mutations/errors and authored source. Exercise relevant direct/linked/
service/installed paths, malformed/incompatible metadata and both compiler
modes/VMs; inspect caller and hidden helper RXAS. Family BF01–09 adds each row's
specific cases. A selected symbol or direct B pass alone does not close a row.
Shared ABI/import/lifecycle/installed/VM checks may cover multiple rows using
the same qualified call route; reuse that proof. Every name still needs its
own new-C value/error/context cases, without repeating the whole matrix 95 times.

| Name | Existing B runtime symbol | Context | Reusable fixture | New C status | Owner / next closure check |
|---|---|---|---|---|---|
| `ABBREV` | `rexxclassicbifabbrev.rexxclassicbif_abbrev` | T | `B/Abbrev` | Unbound | BF01 / STEP04; BIF-CHECK |
| `ABS` | `rexxclassicbifabs.rexxclassicbif_abs` | N | `B/Abs` | Unbound | BF03 / STEP04; BIF-CHECK |
| `ADDRESS` | `rexxclassicbifaddress.rexxclassicbif_address_frame` | A | `B/Address` | Unbound | BF07 / STEP07; BIF-CHECK |
| `ARG` | `rexxclassicbifarg.rexxclassicbif_arg` | A | `B/Arg` | Partial: borrowed-context route | BF05 / STEP04/06/07; BIF-CHECK |
| `B2X` | `rexxclassicbifb2x.rexxclassicbif_b2x` | — | `B/B2x` | Unbound | BF03 / STEP04; BIF-CHECK |
| `BITAND` | `rexxclassicbifs.rexxclassicbif_bitand` | O | `B/Bitwise` | Unbound | BF03 / STEP04; BIF-CHECK |
| `BITOR` | `rexxclassicbifs.rexxclassicbif_bitor` | O | `B/Bitwise` | Unbound | BF03 / STEP04; BIF-CHECK |
| `BITXOR` | `rexxclassicbifs.rexxclassicbif_bitxor` | O | `B/Bitwise` | Unbound | BF03 / STEP04; BIF-CHECK |
| `C2D` | `rexxclassicbifc2d.rexxclassicbif_c2d` | O, N | `B/C2d` | Unbound | BF03 / STEP04; BIF-CHECK |
| `C2X` | `rexxclassicbifc2x.rexxclassicbif_c2x` | O | `B/C2x` | Unbound | BF03 / STEP04; BIF-CHECK |
| `CENTER` | `rexxclassicbifcenter.rexxclassicbif_center` | T | `B/Center` | Unbound | BF01 / STEP04; BIF-CHECK |
| `CENTRE` | `rexxclassicbifcenter.rexxclassicbif_centre` | T | `B/Centre` | Unbound | BF01 / STEP04; BIF-CHECK |
| `CHANGESTR` | `rexxclassicbifchangestr.rexxclassicbif_changestr` | T | `B/Changestr` | Unbound | BF01 / STEP04; BIF-CHECK |
| `COMPARE` | `rexxclassicbifcompare.rexxclassicbif_compare` | T | `B/Compare` | Unbound | BF01 / STEP04; BIF-CHECK |
| `CONDITION` | `rexxclassicbifcondition.rexxclassicbif_condition` | A | `B/Condition` | Unbound | BF07 / STEP07; BIF-CHECK |
| `COPIES` | `rexxclassicbifcopies.rexxclassicbif_copies` | T | `B/Copies` | Unbound | BF01 / STEP04; BIF-CHECK |
| `COUNTSTR` | `rexxclassicbifcountstr.rexxclassicbif_countstr` | T | `B/Countstr` | Unbound | BF01 / STEP04; BIF-CHECK |
| `D2C` | `rexxclassicbifd2c.rexxclassicbif_d2c` | O, N | `B/D2c` | Unbound | BF03 / STEP04; BIF-CHECK |
| `D2X` | `rexxclassicbifd2x.rexxclassicbif_d2x` | O, N | `B/D2x` | Unbound | BF03 / STEP04; BIF-CHECK |
| `DATATYPE` | `rexxclassicbifdatatype.rexxclassicbif_datatype` | T | `B/Datatype` | Unbound | BF03 / STEP04; BIF-CHECK |
| `DATE` | `rexxclassicbifdate.rexxclassicbif_date` | K | `B/Date` | Unbound | BF09 / STEP07/08; BIF-CHECK |
| `DELSTR` | `rexxclassicbifdelstr.rexxclassicbif_delstr` | T | `B/Delstr` | Unbound | BF01 / STEP04; BIF-CHECK |
| `DELWORD` | `rexxclassicbifdelword.rexxclassicbif_delword` | W | `B/Delword` | Unbound | BF02 / STEP04; BIF-CHECK |
| `DIGITS` | `rexxclassicbifnumeric.rexxclassicbif_digits` | N | `B/Numeric` | Partial: borrowed-context route | BF05 / STEP04/06/07; BIF-CHECK |
| `ERRORTEXT` | `rexxclassicbiferrortext.rexxclassicbif_errortext` | — | `B/Errortext` | Unbound | BF07 / STEP07; BIF-CHECK |
| `FORM` | `rexxclassicbifnumeric.rexxclassicbif_form` | N | `B/Numeric` | Unbound | BF05 / STEP04/06/07; BIF-CHECK |
| `FORMAT` | `rexxclassicbifformat.rexxclassicbif_format` | N | `B/Format` | Unbound | BF03 / STEP04; BIF-CHECK |
| `FUZZ` | `rexxclassicbifnumeric.rexxclassicbif_fuzz` | N | `B/Numeric` | Unbound | BF05 / STEP04/06/07; BIF-CHECK |
| `INSERT` | `rexxclassicbifinsert.rexxclassicbif_insert` | T | `B/Insert` | Unbound | BF01 / STEP04; BIF-CHECK |
| `LASTPOS` | `rexxclassicbiflastpos.rexxclassicbif_lastpos` | T | `B/Lastpos` | Unbound | BF01 / STEP04; BIF-CHECK |
| `LEFT` | `rexxclassicbifleft.rexxclassicbif_left` | T | `B/Left` | Unbound | BF01 / STEP04; BIF-CHECK |
| `LENGTH` | `rexxclassicbiflength.rexxclassicbif_length` | T | `B/Length` | Partial: borrowed-context route | BF01 / STEP04; BIF-CHECK |
| `LOWER` | `rexxclassicbifs.rexxclassicbif_lower` | T | `B/Bifs` | Unbound | BF01 / STEP04; BIF-CHECK |
| `MAX` | `rexxclassicbifmax.rexxclassicbif_max` | N | `B/Max` | Unbound | BF03 / STEP04; BIF-CHECK |
| `MIN` | `rexxclassicbifmin.rexxclassicbif_min` | N | `B/Min` | Unbound | BF03 / STEP04; BIF-CHECK |
| `OVERLAY` | `rexxclassicbifoverlay.rexxclassicbif_overlay` | T | `B/Overlay` | Unbound | BF01 / STEP04; BIF-CHECK |
| `POS` | `rexxclassicbifpos.rexxclassicbif_pos` | T | `B/Pos` | Unbound | BF01 / STEP04; BIF-CHECK |
| `QUEUED` | `rexxclassicbifqueued.rexxclassicbif_queued` | Q | `B/Queued` | Unbound | BF06 / STEP06; BIF-CHECK |
| `RANDOM` | `rexxclassicbifrandom.rexxclassicbif_random` | R | `B/Random` | Unbound | BF09 / STEP07/08; BIF-CHECK |
| `REVERSE` | `rexxclassicbifreverse.rexxclassicbif_reverse` | T | `B/Reverse` | Unbound | BF01 / STEP04; BIF-CHECK |
| `RIGHT` | `rexxclassicbifright.rexxclassicbif_right` | T | `B/Right` | Unbound | BF01 / STEP04; BIF-CHECK |
| `SIGN` | `rexxclassicbifsign.rexxclassicbif_sign` | N | `B/Sign` | Unbound | BF03 / STEP04; BIF-CHECK |
| `SOURCELINE` | `rexxclassicbifsourceline.rexxclassicbif_sourceline` | S | `B/Sourceline` | Unbound | BF07 / STEP07; BIF-CHECK |
| `SPACE` | `rexxclassicbifspace.rexxclassicbif_space` | W | `B/Space` | Unbound | BF02 / STEP04; BIF-CHECK |
| `STRIP` | `rexxclassicbifstrip.rexxclassicbif_strip` | T | `B/Strip` | Unbound | BF01 / STEP04; BIF-CHECK |
| `SUBSTR` | `rexxclassicbifsubstr.rexxclassicbif_substr` | T | `B/Substr` | Unbound | BF01 / STEP04; BIF-CHECK |
| `SUBWORD` | `rexxclassicbifsubword.rexxclassicbif_subword` | W | `B/Subword` | Unbound | BF02 / STEP04; BIF-CHECK |
| `SYMBOL` | `rexxclassicbifsymbol.rexxclassicbif_symbol` | P | `B/Symbol` | Unbound | BF05 / STEP04/06/07; BIF-CHECK |
| `TIME` | `rexxclassicbiftime.rexxclassicbif_time` | K | `B/Time` | Unbound | BF09 / STEP07/08; BIF-CHECK |
| `TRACE` | `rexxclassicbiftrace.rexxclassicbif_trace_frame` | A | `B/Trace` | Unbound | BF07 / STEP07; BIF-CHECK |
| `TRANSLATE` | `rexxclassicbiftranslate.rexxclassicbif_translate` | T | `B/Translate` | Unbound | BF01 / STEP04; BIF-CHECK |
| `TRUNC` | `rexxclassicbiftrunc.rexxclassicbif_trunc` | N | `B/Trunc` | Unbound | BF03 / STEP04; BIF-CHECK |
| `UPPER` | `rexxclassicbifs.rexxclassicbif_upper` | T | `B/Bifs` | Unbound | BF01 / STEP04; BIF-CHECK |
| `VALUE` | `rexxclassicbifvalue.rexxclassicbif_value` | P | `B/Value` | Unbound | BF05 / STEP04/06/07; BIF-CHECK |
| `VERIFY` | `rexxclassicbifverify.rexxclassicbif_verify` | T | `B/Verify` | Unbound | BF01 / STEP04; BIF-CHECK |
| `WORD` | `rexxclassicbifword.rexxclassicbif_word` | W | `B/Word` | Unbound | BF02 / STEP04; BIF-CHECK |
| `WORDINDEX` | `rexxclassicbifwordindex.rexxclassicbif_wordindex` | W | `B/Wordindex` | Unbound | BF02 / STEP04; BIF-CHECK |
| `WORDLENGTH` | `rexxclassicbifwordlength.rexxclassicbif_wordlength` | W | `B/Wordlength` | Unbound | BF02 / STEP04; BIF-CHECK |
| `WORDPOS` | `rexxclassicbifwordpos.rexxclassicbif_wordpos` | W | `B/Wordpos` | Unbound | BF02 / STEP04; BIF-CHECK |
| `WORDS` | `rexxclassicbifwords.rexxclassicbif_words` | W | `B/Words` | Unbound | BF02 / STEP04; BIF-CHECK |
| `X2B` | `rexxclassicbifx2b.rexxclassicbif_x2b` | — | `B/X2b` | Unbound | BF03 / STEP04; BIF-CHECK |
| `X2C` | `rexxclassicbifx2c.rexxclassicbif_x2c` | O | `B/X2c` | Unbound | BF03 / STEP04; BIF-CHECK |
| `X2D` | `rexxclassicbifx2d.rexxclassicbif_x2d` | O, N | `B/X2d` | Unbound | BF03 / STEP04; BIF-CHECK |
| `XRANGE` | `rexxclassicbifxrange.rexxclassicbif_xrange` | O | `B/Xrange` | Unbound | BF03 / STEP04; BIF-CHECK |
| `CHARIN` | `rexxclassicbifstream.rexxclassicbif_charin` | H | `CS` | Unbound | BF08 / STEP06/08; BIF-CHECK |
| `CHAROUT` | `rexxclassicbifstream.rexxclassicbif_charout` | H | `CS` | Unbound | BF08 / STEP06/08; BIF-CHECK |
| `CHARS` | `rexxclassicbifstream.rexxclassicbif_chars` | H | `CS` | Unbound | BF08 / STEP06/08; BIF-CHECK |
| `LINEIN` | `rexxclassicbifstream.rexxclassicbif_linein` | H | `CS` | Unbound | BF08 / STEP06/08; BIF-CHECK |
| `LINEOUT` | `rexxclassicbifstream.rexxclassicbif_lineout` | H | `CS` | Unbound | BF08 / STEP06/08; BIF-CHECK |
| `LINES` | `rexxclassicbifstream.rexxclassicbif_lines` | H | `CS` | Unbound | BF08 / STEP06/08; BIF-CHECK |
| `QUALIFY` | `rexxclassicbifstream.rexxclassicbif_qualify` | H | `CS` | Unbound | BF08 / STEP06/08; BIF-CHECK |
| `STREAM` | `rexxclassicbifstream.rexxclassicbif_stream` | H | `CS` | Unbound | BF08 / STEP06/08; BIF-CHECK |
| `UNICODEVERSION` | `rexxclassicbifunicode.rexxclassicbif_unicodeversion` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `TONFD` | `rexxclassicbifunicode.rexxclassicbif_tonfd` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `TONFC` | `rexxclassicbifunicode.rexxclassicbif_tonfc` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `TONFKD` | `rexxclassicbifunicode.rexxclassicbif_tonfkd` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `TONFKC` | `rexxclassicbifunicode.rexxclassicbif_tonfkc` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `ISNFD` | `rexxclassicbifunicode.rexxclassicbif_isnfd` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `ISNFC` | `rexxclassicbifunicode.rexxclassicbif_isnfc` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `ISNFKD` | `rexxclassicbifunicode.rexxclassicbif_isnfkd` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `ISNFKC` | `rexxclassicbifunicode.rexxclassicbif_isnfkc` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `TOUPPERCASE` | `rexxclassicbifunicode.rexxclassicbif_touppercase` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `TOLOWERCASE` | `rexxclassicbifunicode.rexxclassicbif_tolowercase` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `TOCASEFOLD` | `rexxclassicbifunicode.rexxclassicbif_tocasefold` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `TOSIMPLECASEFOLD` | `rexxclassicbifunicode.rexxclassicbif_tosimplecasefold` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `TOTURKICCASEFOLD` | `rexxclassicbifunicode.rexxclassicbif_toturkiccasefold` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `TOTURKICSIMPLECASEFOLD` | `rexxclassicbifunicode.rexxclassicbif_toturkicsimplecasefold` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `GRAPHEMECOUNT` | `rexxclassicbifunicode.rexxclassicbif_graphemecount` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `GRAPHEMEREVERSE` | `rexxclassicbifunicode.rexxclassicbif_graphemereverse` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `GRAPHEMESUBSTR` | `rexxclassicbifunicode.rexxclassicbif_graphemesubstr` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `GRAPHEMEPOS` | `rexxclassicbifunicode.rexxclassicbif_graphemepos` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `ENCODE` | `rexxclassicbifunicode.rexxclassicbif_encode` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `DECODE` | `rexxclassicbifunicode.rexxclassicbif_decode` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `ISDECODABLE` | `rexxclassicbifunicode.rexxclassicbif_isdecodable` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |
| `ISENCODINGSUPPORTED` | `rexxclassicbifunicode.rexxclassicbif_isencodingsupported` | U | `CU` | Unbound | BF04 / STEP03/04; BIF-CHECK |

The native `levelc_bif_inventory.rexx` contains unreachable calls and therefore
checks admission only; it is not a semantic oracle. The broader
`levelc_bif_reference_audit.rexx` and `.expected` provide additional fixed
results for many ordinary entries. Stateful entries retain their direct B
harnesses and instruction/host proof; a pure-text classification here does not
permit generic L to infer purity from the entry name.

This slice checks source identities/signatures and available fixture paths,
not executable metadata or runtime behavior. C-ABI still requires separately
compiled producer/consumer, incompatible/malformed metadata, optional-presence,
ownership and installed controls through the new compiler. The existing 25
instruction and 75 reference routes below remain open under their own criteria.

## Complete reference-obligation routing

All 75 source obligation IDs are retained below. Every row is **open for the
new compiler** until its own implementation/verification record exists. The
[source appendix](../../../../docs/planning/release-1/levelc-reference-obligations.md)
carries the detailed contract and reusable native controls. Each row below
records the new implementation status, owner and next closure check; none copies
a native verdict. In every row, the check covers the complete named obligation,
including relevant error, source, state and lifecycle negatives. The CLC-STEP10 static
checkpoint excludes only dynamic LC-REF-062/LC-87 proof, closed in STEP11.

| ID | Reference obligation | New C status | Owner / step | Next closure check |
|---|---|---|---|---|
| LC-REF-001 | Invocation modes COMMAND, FUNCTION, SUBROUTINE | Not implemented | C-HOST / STEP08 | Qualify COMMAND/FUNCTION/SUBROUTINE entry and teardown. |
| LC-REF-002 | Initial source identity and line inventory | Partial: logical origins, no inventory | C-SRC / STEP04; source/host completion STEP07/08 | Provide retained physical/mapped source and initial identity; test no runtime reread. |
| LC-REF-003 | Initial environment and stream selection | Not implemented | C-HOST / STEP08 | Bind configured initial environment/streams; test defaults and context isolation. |
| LC-REF-004 | Invocation arguments and omitted positions | Partial: private-call presence | C-HOST / STEP08 | Carry exact-length/omitted/empty/NUL arguments through each invocation mode. |
| LC-REF-005 | Caller-provided trap overrides | Not implemented | C-HOST / STEP08 | Install/inherit/restore caller overrides; test forbidden/stale use. |
| LC-REF-006 | Completion classes: no value, result, condition, resource failure, unable to continue | Partial: private result packets | C-HOST / STEP08 | Expose distinct result/no-result/condition/resource/unable-to-continue outcomes. |
| LC-REF-007 | Source characters, EOL/EOS and invalid-encoding `22.1` | Partial: bounded source reader | C-SRC / STEP04; source/host completion STEP07/08 | Test UTF8, NUL, every newline/EOF form and authored 22.1 refusal. |
| LC-REF-008 | Configured extra blank characters and uppercase mapping | Runtime only | C-UNI / STEP03/04; BF01–04 and STEP08 limits | Integrate configured blanks/simple uppercase in scanner, pools and calls. |
| LC-REF-009 | Configured compare, substring, length and range ordering | Partial: admitted comparisons | C-UNI / STEP03/04; BF01–04 and STEP08 limits | Run comparison/substring/length/range cases through configured new-C entries. |
| LC-REF-010 | Binary-digit/character conversion hooks | Partial: radix literals | C-UNI / STEP03/04; BF01–04 and STEP08 limits | Bind conversion hooks and test configured digits, errors and isolation. |
| LC-REF-011 | Fixed `00`–`FF` ↔ `U+0000`–`U+00FF` bridge, PAD/XRANGE and out-of-range signal | Partial: radix literals | C-UNI / STEP03/04; BF01–04 and STEP08 limits | Run all 256 ordinals, pad/range and >U+00FF refusal through new C. |
| LC-REF-012 | Unicode scalar text and codepoint character positions | Partial: scalar runtime/literals | C-UNI / STEP03/04; BF01–04 and STEP08 limits | Qualify codepoint positions across expressions, names, BIFs, PARSE and host text. |
| LC-REF-013 | No implicit BYTE/UTF8 switch, normalization or grapheme behavior | Partial: exact-text subset | C-UNI / STEP03/04; BF01–04 and STEP08 limits | Prove ordinary text has no implicit encoding/normalization/grapheme switch. |
| LC-REF-014 | DATATYPE extra letter/digit families, blanks and exponent limit | Runtime only | C-UNI / STEP03/04; BF01–04 and STEP08 limits | Bind DATATYPE; test configured classes/blanks and exponent/resource limits. |
| LC-REF-015 | Configured command completion, RC/.RC/.RS and ERROR/FAILURE | Not implemented | C-HOST / STEP08 | Bind command completion and RC/.RC/.RS plus ERROR/FAILURE behavior. |
| LC-REF-016 | External routine lookup with arguments, environment, streams and pool access | Partial: private calls only | C-HOST / STEP08 | Implement approved static signed providers; test presence, environments and pool windows. |
| LC-REF-017 | Queue push, queue, pull and count services | Runtime only | C-HOST / STEP08 | Bind one selected queue across all producers/counts; test FIFO/LIFO/NUL/isolation. |
| LC-REF-018 | Character and line stream input/output, positioning, state and close | Runtime only | C-HOST / STEP08 | Bind eight existing stream entries; test raw/encoded position, EOF/error and close. |
| LC-REF-019 | Stream qualification, temporary names and availability queries | Runtime only | C-HOST / STEP08 | Bind QUALIFY/STREAM queries; disposition temporary-name/host capabilities. |
| LC-REF-020 | Trap override/fallthrough for configuration hooks | Not implemented | C-HOST / STEP08 | Test hook null-result, replacement, trap override and fallthrough. |
| LC-REF-021 | API_Set, API_Value, API_Drop with compound substitution | Not implemented | C-HOST / STEP08 | Expose checked callback-window APIs with ordered compound substitution. |
| LC-REF-022 | Direct API set/value/drop without substitution | Not implemented | C-HOST / STEP08 | Expose direct APIs and prove no substitution or alias confusion. |
| LC-REF-023 | API_ValueOther and visible-name iteration/reset | Not implemented | C-HOST / STEP08 | Test other-pool access and iteration/reset under mutation and teardown. |
| LC-REF-024 | API access only during enabled external/host callbacks | Not implemented | C-HOST / STEP08 | Refuse pool access outside enabled callbacks and after owner expiry. |
| LC-REF-025 | Minimum numeric, literal, symbol and string limits | Partial: request limits | C-UNI / STEP03/04; BF01–04 and STEP08 limits | Pin numeric/source/string budgets; test boundaries and recoverable resource failures. |
| LC-REF-026 | Contextual instruction words, no general reserved words | Partial: bounded grammar | C-SRC / STEP04; source/host completion STEP07/08 | Test every instruction word as a noninstruction symbol where legal. |
| LC-REF-027 | Quoted/doubled strings and radix suffix boundaries | Partial: quoted/radix subset | C-SRC / STEP04; source/host completion STEP07/08 | Complete quoted/doubled/radix lexical forms and located invalid cases. |
| LC-REF-028 | Hex/binary literal grouping and left padding | Partial: radix repair | C-SRC / STEP04; source/host completion STEP07/08 | Extend retained radix proof across contexts, grouping, padding and failures. |
| LC-REF-029 | Numeric constants, exponent signs and period-start tokens | Partial: numeric tokens | C-SRC / STEP04; source/host completion STEP07/08 | Test numeric/exponent/period-start cases and configured limits. |
| LC-REF-030 | Comma continuation and inferred clause endings | Partial: bounded grammar | C-SRC / STEP04; source/host completion STEP07/08 | Test continuation and clause inference across comments/newlines/Unicode blanks. |
| LC-REF-031 | Nested comments and unterminated `6.*` messages | Partial: bounded tokenizer | C-SRC / STEP04; source/host completion STEP07/08 | Test arbitrary nested comments and exact unterminated-comment identities. |
| LC-REF-032 | Alternative negators and inferred blank/nonblank concatenation | Partial: concatenation subset | C-SRC / STEP04; source/host completion STEP07/08 | Complete alternative negators and blank/abutment including adjacent calls. |
| LC-REF-033 | Top-syntax VALUE insertion, assignment, label and keyword promotion | Partial: bounded grammar | C-SRC / STEP04; source/host completion STEP07/08 | Test every VALUE/assignment/label/keyword promotion ambiguity. |
| LC-REF-034 | Reserved .MN/.RESULT/.RC/.RS/.SIGL and `50.1` | Not implemented | C-POOL / STEP03/05/06; STEP08 external API | Implement reserved processor values and precise 50.1 with lifecycle tests. |
| LC-REF-035 | Function-name ending-period `51.1` | Not implemented | C-SRC / STEP04; source/host completion STEP07/08 | Emit authored 51.1 for illegal ending-period function names. |
| LC-REF-036 | Clause/label/null-clause program structure | Partial: private labels | C-SRC / STEP04; source/host completion STEP07/08 | Implement one-body labels/null clauses/fall-through and source/TRACE anchors. |
| LC-REF-037 | IF/THEN/ELSE branch selection and nearest ELSE | Partial: admitted IF locally qualified | LC-I-06–10 / STEP05 | Complete remaining SELECT/full-DO/shared-label placements, expression/condition/diagnostic and TRACE dependencies in LC-I-06. |
| LC-REF-038 | SELECT/WHEN/OTHERWISE order and absent-match `7.3` | Partial: admitted forms locally qualified | LC-I-06–10 / STEP05 | Retain qualified SELECT-AC01–04 and LC-I-07 shared-label, expression/control, diagnostic/recovery and observation dependencies. |
| LC-REF-039 | Simple DO grouping and empty body | Partial: simple DO | LC-I-06–10 / STEP05 | Complete simple-group placements, empty bodies, labels and observations. |
| LC-REF-040 | Counted, controlled, FOREVER, WHILE and UNTIL DO | Partial: static forms locally qualified | LC-I-06–10 / STEP05 | [DO-AC01–04](../PLAN.md#current-do-proof) qualify header/entry/step timing, arbitrary counts/errors and keyed frame lifetime; whole-instruction dependencies stay open. |
| LC-REF-041 | LEAVE/ITERATE nesting and named loop targets | Partial: named static transfers locally qualified | LC-I-06–10 / STEP05 | LEAVE/ITERATE-AC01–04 qualify static targets, distinct crossed-state cleanup/end work and default/STRICTC timing; whole-instruction dependencies stay open. |
| LC-REF-042 | END-name matching and group-label branch restrictions | Partial: bounded groups | C-SRC / STEP04; source/host completion STEP07/08 | Validate END names and forbidden group-label transfers with correct source. |
| LC-REF-043 | Expression terms, prefix, power and arithmetic precedence | Partial: operator subset | C-EXPR / STEP04; STEP05/07 contexts | Complete all terms/operators/order with independent values and side effects. |
| LC-REF-044 | Left-associative Classic power | Not implemented | C-EXPR / STEP04; STEP05/07 contexts | Implement and independently test left-associative power and prefix interaction. |
| LC-REF-045 | Normal/strict comparisons and configured character ordering | Partial: comparison subset | C-EXPR / STEP04; STEP05/07 contexts | Complete normal/strict families and configured character-order tests. |
| LC-REF-046 | Logical AND/OR/XOR and prefix negation with exact values | Partial: logical subset | C-EXPR / STEP04; STEP05/07 contexts | Complete eager AND/OR/XOR/NOT and context-specific exact logical failures. |
| LC-REF-047 | PARSE ARG/PULL/SOURCE/LINEIN/VERSION/VALUE/VAR and UPPER | Partial: seven-source C profile locally qualified | LC-I-11/16/19 / STEP06 | CPARSE-AC01/04/07/08: all seven once-only sources, empty/comma behavior, raw argument preservation, simple UPPER and source/condition/platform proof. |
| LC-REF-048 | PARSE targets, placeholders, literal/variable patterns and positions | Partial: v3/shared B/C profile locally qualified | LC-I-11/16/19 / STEP06 | CPARSE-AC01–03/05–08: v3 backward/anchor/empty-pattern normalization, live ordered fields, shared configured word policy, errors, ownership and observations. |
| LC-REF-049 | Pool dropped, implicit, exposed and scalar/stem binding states | Partial: DROP states/private pools/prebound aliases | C-POOL / STEP03/05/06; STEP08 external API | Complete source EXPOSE, recursive alias lifetime and host access. |
| LC-REF-050 | Set/value/drop, stem-tail propagation and recursive exposure | Partial: assignment/DROP with prebound aliases | C-POOL / STEP03/05/06; STEP08 external API | Complete source-ordered recursive EXPOSE, host access and condition observations. |
| LC-REF-051 | Compound-tail evaluation without NOVALUE, final lookup with NOVALUE | Not implemented | C-POOL / STEP03/05/06; STEP08 external API | Test tail substitution suppresses NOVALUE while final lookup may raise it. |
| LC-REF-052 | Constant symbols, reserved pool 0 and SIGL/.SIGL updates | Not implemented | C-POOL / STEP03/05/06; STEP08 external API | Implement constant/reserved symbols and causing-clause SIGL/.SIGL updates. |
| LC-REF-053 | Numeric DIGITS/FORM/FUZZ and arithmetic error/condition model | Partial: NUMERIC/arithmetic subset | C-EXPR / STEP04; STEP05/07 contexts | Qualify DIGITS/FORM/FUZZ, rounding, error/LOSTDIGITS and restoration end to end. |
| LC-REF-054 | Exact logical values and contextual `34.*` identities | Partial: IF 34.1 locally qualified | C-EXPR / STEP04; STEP05/07 contexts | Complete remaining WHEN/loop/logical identities and resumed condition/origin/TRACE contracts; bounded IF proof is not all contexts. |
| LC-REF-055 | Function/CALL omitted arguments and resolution order | Partial: private calls/3 BIFs | C-ACT and LC-I-13–15 / STEP06; STEP08 host completion | Complete local/BIF/static-external resolution, source order and omissions. |
| LC-REF-056 | Function RETURN value, `45.1` and RESULT/.RESULT lifecycle | Partial: private result presence | C-ACT and LC-I-13–15 / STEP06; STEP08 host completion | Prove required function result 45.1 and RESULT/.RESULT drop/presence lifecycle. |
| LC-REF-057 | Program initialization and clause-boundary HALT/trap/TRACE work | Not implemented | C-HOST / STEP08 | Provide real HALT and clause-boundary trap/TRACE initialization without unused clocks. |
| LC-REF-058 | ADDRESS selection/swap, transient command and WITH redirection | Not implemented | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 | Complete ADDRESS selection/swap/transient/WITH resources and NUL disposition. |
| LC-REF-059 | CALL ON/OFF delayed condition handlers | Not implemented | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 | Implement delayed CALL handlers, policy replacement, suspension and resumption. |
| LC-REF-060 | DROP direct and parenthesized variable lists | Partial: ordered v3 forms locally qualified | C-POOL / STEP03/05/06; STEP08 external API | Retain ordered/captured-list/Unicode/prebound-alias proof; close source EXPOSE, conditions, full diagnostics and TRACE dependencies. |
| LC-REF-061 | EXIT value, fallthrough and finalization | Partial: return-like EXIT | C-ACT and LC-I-13–15 / STEP06; STEP08 host completion | Propagate EXIT to program root; finalize once and expose host completion. |
| LC-REF-062 | INTERPRET source, HALT, syntax and label restrictions | Not implemented | LC-I-25 / STEP11; LC-87-01–05 | Close LC-87-01–05 through same compiler with live caller and bounded code. |
| LC-REF-063 | NUMERIC DIGITS/FORM/FUZZ validation and defaults | Partial: NUMERIC subset | C-EXPR / STEP04; STEP05/07 contexts | Validate all NUMERIC forms/defaults atomically and install in owning C frame. |
| LC-REF-064 | OPTIONS word handling and unknown-option policy | Partial: current zero-word forms locally qualified | LC-I-05 / STEP05 | Current zero runtime words have no processor effect; all nontrivial operands retain once-only effects/errors. Complete header controls and shared dependencies remain open. |
| LC-REF-065 | PROCEDURE pool creation and EXPOSE aliases | Partial: private pool only | C-POOL / STEP03/05/06; STEP08 external API | Complete first-instruction eligibility and ordered direct/indirect EXPOSE. |
| LC-REF-066 | PUSH/QUEUE ordering and null-expression value | Not implemented | LC-I-17/18 / STEP06 | Bind one queue owner; test null expressions, once-only evaluation and ordering. |
| LC-REF-067 | RETURN function/subroutine/outermost lifecycle | Partial: private RETURN | C-ACT and LC-I-13–15 / STEP06; STEP08 host completion | Complete main/subroutine/function RETURN, result presence and recursive cleanup. |
| LC-REF-068 | SAY default output and optional empty expression | Partial: SAY/radix subset | LC-I-01 / STEP05; STEP07/08 observations/host | Complete modern expression/output order and ordinary errors; further historical host edge parity is deferred and unclaimed. |
| LC-REF-069 | SIGNAL branch/trap modes and loop-state clearing | Not implemented | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 | Implement every SIGNAL target/trap, causing clause and crossed-state cleanup. |
| LC-REF-070 | TRACE options, interactive mode, skip/inhibit and source/result tracing | Not implemented | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 | Implement approved practical TRACE state/display/inhibition and divergences. |
| LC-REF-071 | SYNTAX, HALT, ERROR, FAILURE, NOTREADY, NOVALUE, LOSTDIGITS | Partial: fatal error path | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 | Exercise all seven condition producers/traps and nested ownership; real HALT separate. |
| LC-REF-072 | Message catalog, .MN, source/line traceback and trap handling | Partial: logical fatal origins | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 | Bind messages/.MN/source traceback; preserve original condition and mapped origin. |
| LC-REF-073 | Immediate SIGNAL ON, delayed CALL ON and HALT buffering | Not implemented | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 | Prove immediate versus delayed delivery, buffered HALT and nested restoration. |
| LC-REF-074 | CONDITION BIF state fields | Not implemented | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 | Bind CONDITION fields to authoritative current event; test all options/isolation. |
| LC-REF-075 | NOP as a statement with no visible effect | Partial: bounded childless forms locally qualified | LC-I-04 / STEP05; STEP07 observations | Complete SELECT/shared-label placements, exact diagnostics and clause/condition/TRACE observations. |

## Syntax and detailed native contract crosswalk

The [native syntax catalogue](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#syntax-and-instruction-inventory)
contains **36 contracts**, including capabilities beyond the 25 instruction
names. Each row below inherits the full current owner/closure check named here.
All are open for the new compiler; native “closed” or “parked” statuses do not
transfer. Existing native editor behavior is a retained consumer contract, not
an instruction or an automatic requirement for a new editor protocol.

| Native syntax contract | New C status | Owner / step | Next closure check |
|---|---|---|---|
| `SYN-CLASSIC-OPTIONS` | Partial: bounded COptions/header forms locally qualified | LC-I-05 / STEP05 | Complete header controls, exact diagnostics, shared labels and conditions/TRACE remain open under LC-I-05. |
| `SYN-CLASSIC-CLAUSES` | Partial: bounded grammar/source | C-SRC/C-OBS; STEP04/07/08 | Close LC-REF-007/030/036/057/070/072: clause endings/identity, observations and initialization. |
| `SYN-CLASSIC-CONTEXTUAL-KEYWORDS` | Partial: bounded grammar | C-SRC; STEP04 | Close LC-REF-026/033 for every contextual symbol/promotion case. |
| `SYN-CLASSIC-LABELS` | Partial: private PROCEDURE labels | C-ACT/C-SRC/C-OBS; STEP03/06/07/11 | Close LC-REF-036/042/055/056/069 and dynamic caller-label reachability. |
| `SYN-CLASSIC-SYMBOLS` | Partial: shared Unicode/raw variable names | C-SRC/C-POOL; STEP03/04/05/06/08 | Retain distinct micro-sign/Greek and raw ARG/name admission proof; close remaining constant symbols, routine/call names, configuration and host access. |
| `SYN-CLASSIC-STEMS` | Partial: shared reads/writes/DROP | C-POOL; STEP03/05/06/08 | Complete LC-I-03/12 and LC-REF-049–051 with source-ordered recursive aliases and conditions. |
| `SYN-CLASSIC-STRINGS` | Partial: quoted/radix subset | C-SRC/C-UNI; STEP04 | Close LC-REF-011–013/025/027/028, quoted doubling/grouping, limits and exact text. |
| `SYN-CLASSIC-ASSIGNMENT` | Partial: one-write forms locally qualified | LC-I-03 / STEP05 | Close the remaining source EXPOSE/shared-label, diagnostic, condition/NOVALUE and TRACE dependencies in LC-I-03. |
| `SYN-CLASSIC-COMMAND` | Not implemented | LC-I-21 / STEP07 | Full LC-I-21 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-ADDRESS` | Not implemented | LC-I-20 / STEP07 | Full LC-I-20 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-ARG` | Partial | LC-I-11 / STEP06 | Full LC-I-11 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-CALL` | Partial | LC-I-13 / STEP06 | Full LC-I-13 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-DO` | Partial | LC-I-08 / STEP05 | Full LC-I-08 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-DROP` | Partial: direct/indirect forms locally qualified | LC-I-02 / STEP05 | Full LC-I-02 closure waits the named EXPOSE/condition/diagnostic/TRACE dependencies. |
| `SYN-CLASSIC-EXIT` | Partial | LC-I-15 / STEP06 | Full LC-I-15 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-IF` | Partial: admitted forms locally qualified | LC-I-06 / STEP05 | Close the remaining placements, expressions, exact diagnostics, condition/NOVALUE and TRACE dependencies in LC-I-06. |
| `SYN-CLASSIC-INTERPRET` | Not implemented | LC-I-25 / STEP11 | Full LC-I-25 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-ITERATE` | Partial | LC-I-10 / STEP05 | Full LC-I-10 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-LEAVE` | Partial | LC-I-09 / STEP05 | Full LC-I-09 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-NOP` | Partial: childless forms locally qualified | LC-I-04 / STEP05 | Close SELECT/shared labels, exact 21.1 and clause/condition/TRACE dependencies in LC-I-04. |
| `SYN-CLASSIC-NUMERIC` | Partial | LC-I-22 / STEP07 | Full LC-I-22 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-PARSE` | Partial: admitted forms locally qualified | LC-I-19 / STEP06 | Full LC-I-19 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-PROCEDURE` | Partial | LC-I-12 / STEP06 | Full LC-I-12 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-PULL` | Not implemented | LC-I-16 / STEP06 | Full LC-I-16 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-PUSH` | Not implemented | LC-I-17 / STEP06 | Full LC-I-17 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-QUEUE` | Not implemented | LC-I-18 / STEP06 | Full LC-I-18 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-RETURN` | Partial | LC-I-14 / STEP06 | Full LC-I-14 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-SAY` | Partial | LC-I-01 / STEP05 | Full LC-I-01 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-SELECT` | Partial: admitted forms locally qualified | LC-I-07 / STEP05 | SELECT-AC01–04 pass; full LC-I-07 and shared dependencies remain open. |
| `SYN-CLASSIC-SIGNAL` | Not implemented | LC-I-23 / STEP07 | Full LC-I-23 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-TRACE` | Not implemented | LC-I-24 / STEP07 | Full LC-I-24 check in the instruction ledger; shared dependencies remain open. |
| `SYN-CLASSIC-EXPRESSIONS` | Partial: operator subset | C-EXPR/C-NUM; STEP04/07 | Close LC-REF-043–046/053/054, including power, integer division/remainder and full comparisons. |
| `SYN-CLASSIC-BIF-CALL` | Partial: 3 names; 92 unbound | BF01–09; STEP04/06/07/08 | Close every one of the 95 binding rows with BIF-CHECK, including LC-REF-055. |
| `SYN-CLASSIC-LOCAL-CALL` | Partial: private procedures | C-ACT/C-ABI; STEP03/06/07 | Close LC-I-11–15 and LC-REF-055/056/061/067; shared-label recursion and outcomes. |
| `SYN-CLASSIC-DSLSH` | Partial: new source/diagnostics; native editor retained | C-SRC; STEP04; existing editor owner | Qualify new-C trees/located diagnostics. Preserve existing native rxc/editor projection; no new B/L editor protocol or editor replacement is implied. |
| `SYN-CLASSIC-CANONICAL-LOWERING` | Partial: C AST/IR pipeline | CFC-AC07/10/11; STEP01/02/04 | Verify ownership, structure, types/effects, provenance and refusal at C AST→L IR→RXAS boundaries; native canonical AST/lowerer reuse is not required or allowed as fallback. |

The instruction ledger links all **72 detailed LC-74–87 criteria** and its
instruction-owned **LC-AC** criteria to their exact native sections. The remaining
cross-cutting criteria below complete the **LC-AC-01–77** reconciliation. Each
inherits the new owner's open status and evidence requirements; there is no
second completion list. Preserve the semantic edge cases and independent
oracles. Earlier bounded slice limits, 32-bit count caps and native canonical-AST
construction are not new-C limits or mandated implementation: current whole-
instruction, Unicode and host contracts supersede those snapshots.

| Detailed native criteria | New owner / next closure check |
|---|---|
| [LC-AC-01/04/59](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) | CFC-AC01/08; STEP01/10/11: reconcile every required row and exception, then qualify full static/dynamic coverage. |
| [LC-AC-06](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) | CFC-AC04/09/11; C-HOST and STEP03/08: shared B values/pools/BIFs, separate caller contexts, RexxScript isolation and host contracts. |
| [LC-AC-08/58/61](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) | CFC-AC01/06/07/09; STEP01/04/09/10: exhaustive C AST→IR disposition, structure/ownership/symbol/effect/provenance validation, duplication review and independent consistency proof. Preserve the semantic checks in L; do not import the native AST/backend. |
| [LC-AC-16/37/41](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) | C-EXPR; STEP04/05/07: eager logical/operator order, exact values/errors and setup-bearing expressions under all control contexts. |
| [LC-AC-36/42–44](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) | C-SRC/C-EXPR; STEP04: spelling/token/context/concatenation and source-preserving expression cases, including rejected forms. |
| [LC-AC-72–75](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#acceptance-criteria) | C-UNI, BF04 and all affected instruction owners; STEP03/04/08: UTF8/codepoints,256 ordinals/>U+00FF refusal, names/simple casing, explicit Unicode and raw/encoded streams. Existing B core proof is reused; new-C bindings/host/limits still need proof. No implicit binary text profile. |

## Remaining shared-contract gaps

The [current native gap register](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#remaining-gap-decision-register-2026-10-07)
supersedes older native inventory prose. These are explicit new-C obligations,
not instructions to redo unchanged B library algorithms or copy native status.

| Source gap | New owner / remaining action and closure check |
|---|---|
| LC-GAP-01 | STEP11 / LC-I-25 / CFC-AC05: implement required INTERPRET last and prove LC-87-01–05; native parked status is not a new-C exclusion. |
| LC-GAP-02 | BF01–09 / STEP04/06–08/10: reuse all implemented B entries, integrate and independently qualify every new-C name, including resource/host/error cases. |
| LC-GAP-03 | C-HOST / STEP08 and LC-I-13/16–21: complete configured command, queue, input, stream and static signed-provider adapters with resource/condition/result proof. |
| LC-GAP-04 | C-ACT / STEP03 and C-HOST / STEP08: qualify invocation modes, exact-length NUL arguments, caller traps/completions, enabled pool-access windows and real host HALT. |
| LC-GAP-05 | STEP01/06/07/08: preserve accepted command-NUL FAILURE when transport cannot represent it, while length-aware host delivery remains open; retain practical TRACE divergences; current PARSE EXTERNAL/NUMERIC exclusion is not its final Release1 disposition. |
| LC-GAP-06 | C-SRC/C-OBS/C-HOST / STEP04/07/08: full mapped identity/source inventory, configured scanner/conversion, exact lengths and declared source/resource limits. |
| LC-GAP-07 | C-NUM/C-EXPR / STEP03/04/07/08: independent arithmetic/order/error/rounding/configured-comparison proof and limits; native parity is only a control. |
| LC-GAP-08 | C-UNI/BF04 / STEP03/04/08: reuse completed Unicode/stream foundation, then close new recognition/bindings and wider host/configuration/resource proof. Old blanket Unicode/stream deferral is superseded. |
| LC-GAP-09 | CFC-AC01/06/07/09 / STEP01/04/09/10: complete AST/IR coverage, ownership, provenance and real duplication review in B/L owners. |
| LC-GAP-10 | CFC-AC08/12 / STEP10/11: final-input static/dynamic, installed and declared-platform proof. Hosted, supported sanitizers and Release1 approval remain their own gates; local slices do not close them. |

## Wider programme outside current C delivery

Unverified wider **LLCOMP-AC01–16** outcomes remain: **LLCOMP-STEP07B** proper B and B1/B2/B3
self-hosting from the retained seed, with deterministic closure/mode/semantic/dependency
comparisons; complete typed B/G frontends, metadata
conformance and replacement coverage; common graph/codec/transform/emission
quality and bounded reusable services; full C/G/platform/installed/constrained
qualification; representative production replacement latency/storage/code-size
budgets; current schemas/algorithms/walkthroughs; and an explicitly approved
default switch with seed-retention/retirement and rollback proof. C portions
are covered by the current CFC criteria. New B/G/self-hosting/combined replacement
is not scheduled by this C plan and has not been completed or cancelled.

## Test and evidence design

For each work unit, name fixtures before implementation and state expected
values, side effects, conditions, source positions and resource behavior. Use
small native/Regina/reference controls only within matching dialects; record
Unicode, provider and TRACE exceptions explicitly. A discrepancy gets a minimal
case classified as new compiler, native compiler, shared runtime or bad oracle.
Never weaken an independent expectation merely to match native output.

Exercise compiler optimisation on/off with RXAS optimisation independently on/
off for the changed semantics; affected direct/linked/service/installed routes
and both concrete VMs are additional dimensions, not interchangeable labels.
Use representative RXAS structural assertions rather than register snapshots:
allocations/config construction, live registers, calls/windows, copies, branch
structure, cleanup and observation anchors. Include constant-only SAY, numeric
loop with context mutation, exposed compound names, omitted-argument calls,
condition resumption and eventually nested INTERPRET. Count hidden helper work.

Generic L tests use a non-C language/IR family, adversarial stale facts and
incomplete effect/edge contracts. Runtime tests use direct B callers; Unicode
also has unchanged G users. VM/RXAS tests use direct assembly independently of
the new compiler. This triangulation avoids a shared compiler/oracle defect.

Every completion record in the current plan names input revision, decisive
command/result, observed platform/mode, code-shape outcome and remaining gaps.
Retain source fixtures and intended documentation outputs, not logs/dumps/run
receipts. Follow [AGENTS.md](../../../../AGENTS.md) for isolated aggregate
measurement, scheduling and sanitizer scope.
Reuse unchanged qualified evidence; do not rerun broad QA per instruction.

## Platform and capability gates

| Surface | Required disposition before complete C |
|---|---|
| Desktop macOS ARM64/Intel, Linux and Windows MSVC/MinGW | Product, selected C compiler/service, runtime libraries, install/relocation, errors and normal hosted suites; both VMs where built. Source inspection is not cross-platform proof. |
| Maintained CMS/TSO profiles | Preserve native seed/standalone stages, text/encoding and named host contracts; record actual guest/SDK/resource/dynamic-loader capability. An unavailable required feature stays open for Adrian, not silently excluded. |
| Embedded consumers | Exact-length arguments/source/result, callback windows, host resources, source/clock/HALT producers, reuse/concurrency and teardown. Legacy C-string APIs do not prove NUL support. |
| Dynamic execution | Same compiler pipeline, assembler/load integration, continuation ABI and bounded code retention. Installed capability must agree with documentation for each required platform. |
| Sanitizers and release | Follow AGENTS and SAN worklist for actual findings; unsupported facilities have supported-platform coverage. Local correctness, full sanitizer closure, hosted results, signing and release remain distinct. |

The plan does not promise support for an unspecified mainframe transport or
new host language API. CLC-STEP01 pins exact maintained profiles and STEP08
resolves missing contracts before completion is claimed.
