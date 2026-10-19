# Level C requirements and implementation crosswalk

**Approved requirements baseline, not completion evidence.** Reviewed against develop
`016e6a65466c65a5c722b96452d95025acb463b2`, 19 October 2026; reconciled with
`e5c066b87945dc697df6c771fc5a56a20962c772`, which changes sanitizer scheduling/docs
without changing reviewed compiler/runtime implementation. The [plan](../PLAN.md#c-first-delivery-milestone) owns CFC acceptance and CLC steps;
[architecture](architecture.md) defines ownership. All rows require new-compiler
proof. Existing runtime/native tests are reusable controls where unchanged;
none establishes a new compiler pass simply by existing.

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
  explicit Unicode algorithms and raw/encoded streams remain admitted.
- External C calls retain the static signed B/G provider contract, including
  documented missing-provider timing/error differences. No general late dynamic
  provider search or new foreign-language interface is inferred.
- PARSE EXTERNAL/NUMERIC remain explicitly excluded from the current language
  contract. TRACE retains Adrian's practical scope and documented interactive,
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

`levelc/levelc.rxl` and `llcparse` implement a bounded generated grammar/token
adapter. `llcfrontend` builds C IR and supports only private PROCEDURE-labelled
routines, scalar comma ARG and three BIF names (LENGTH, ARG, DIGITS). The grammar
has a limited operator/control surface and an ASCII identifier pattern. The
C service and selector are independent from typed frontends, but neutral scalar
ABI records still use `crexx.g` names. The target adapter creates configuration
for operations and uses snapshots/packets rather than the complete live frame.

SAY's adjacent radix suffix repair has focused retained proof. Full SAY remains
open for expressions, callbacks, conditions and TRACE. Supported partial syntax
in the table below is source inspection, not new execution qualification.

## Shared requirements before instruction closure

| ID / stage | Design and required proof |
|---|---|
| C-SRC / STEP04 | Owned exact source bytes and logical mapping; contextual instruction words, quoted names, nested comments, continuations, radix boundaries, all whitespace/Unicode symbol rules and configured additions. Preserve byte spans and scalar columns separately; NUL is data, not EOF. Compare located static errors including unreachable text, 6.*, 22.1, 50.1/51.1 and complete clause inventories. |
| C-EXPR / STEP04 | Named AST nodes and C IR for constants, reads, compound names, calls and every prefix/power/arithmetic/concatenation/comparison/logical operator. Left-associative Classic power; correct prefix precedence; eager logical operands; exact logical values with context-specific 34.*. RHS and compound substitution order must be explicit. Use side effects and independently calculated values, not only native equality. |
| C-NUM / STEP03/04/07 | Shared B values/numeric context retain arbitrary precision semantics, rounding, DIGITS/FUZZ/FORM, spelling and LOSTDIGITS. Direct operation entries carry borrowed context. Cover overflow/invalid whole arguments, guard digits, negative quotient/remainder, exponent and conversion boundaries; signed64 positional/count limit and arbitrary-precision radix rules remain distinct. |
| C-POOL / STEP03/05/06 | Runtime owns dropped/implicit/scalar/stem/compound states, recursive aliases and source-ordered exposure. Compiler emits ordered name evaluation/pool effects. Property tests include DROP versus empty, stem propagation, tail substitution without NOVALUE and final read with NOVALUE; callbacks and VALUE invalidate local facts. |
| C-ACT / STEP03/06/07 | One source-unit label/body model, live replaceable pool binding, argument presence, invocation kind, RESULT, program exit and defined inherited/restored state. Loops, labels and handlers do not become unrelated private procedures. Verify recursion, fall-through and first-label precedence. |
| C-ABI / STEP01/03/06 | B implementation reads/writes admitted metadata with exact signatures, optional presence and ownership. Constants retain original metadata identities; no native reader fallback. Independent producer/consumer tests, incompatible/malformed imports and installed absence-of-native-source-compiler controls. |
| C-OBS / STEP03/07 | Explicit clause/source/TRACE and failure effects prevent illegal movement/removal. Actual condition identity/origin belongs to runtime; optimizer must preserve observer order and state. One sample per observed DATE/TIME clause and program elapsed/reset state; no unconditional per-clause clock read when unused. |
| C-HOST / STEP08 | Execution-owned configuration and host adapter contracts: invocation/result, queues, streams, environment callbacks, pool access windows, clock/HALT, source inventory and resource errors. Concurrent contexts must not share mutable program state. APIs distinguish omitted/empty/NUL and forbid stale callback access. |
| C-UNI / STEP03/04 | Extract shared B Unicode core from G packaging. One versioned data/algorithm owner, direct B facade, G API preserved, C error/ordinal adapters and L recognition independent. Cold bootstrap must be acyclic; B/G/C conformance, table ownership and freshness tests cover extraction. |

## Whole-instruction work units

Each LC-I row is a separate implementation/review assignment. STEP05–07 are
sequence groups, not permission to combine all instructions into one prompt.
“Partial” describes the baseline; all whole-instruction rows remain open.
Worker output includes grammar/AST, C IR and effect/control schema, runtime ABI,
independent positive/error expectations, source/TRACE behavior and representative
RXAS/helper inspection. Required dependencies introduced early remain owned by
their full instruction review; e.g. ARG reuses PARSE infrastructure.

| Contract / stage | Baseline | Required design and acceptance cases |
|---|---|---|
| LC-I-01 SAY / STEP05 | Partial; radix fix retained | `Say(value?)` lowers once, then one ordered output effect. Empty, NUL, high scalars, output ordering and configured callback/error behavior. Literal-only RXAS has no runtime radix decode; unused context/pool setup needs C-OPT-01 proof. |
| LC-I-02 DROP / STEP05 | Missing | Direct/indirect source-ordered variable lists become pool-drop operations; arbitrary compounds and exposed aliases use runtime pool rules. Invalid list words follow accepted policy. No argument/alias reevaluation or numeric conversion. |
| LC-I-03 assignment / STEP05 | Simple scalar only | Evaluate RHS before compound substitution; one pool write with exact scalar text/cache semantics. Stems, implicit tails, exposed aliases and NUL/Unicode names; inspect duplicate reads/copies and alias invalidation. |
| LC-I-04 NOP / STEP05 | Partial | No value/effect operation, but preserve clause/label/TRACE observations. Reject trailing operands in every legal statement position; no runtime helper for an unobserved NOP. |
| LC-I-05 OPTIONS / STEP05 | Header subset | Separate admitted static header controls from executable expression/empty form and unknown-word policy. Evaluate once at its clause; no implicit Unicode profile switch. Context mutation is an effect, not a compile-time global. |
| LC-I-06 IF / STEP05 | Partial | Condition, true/false edges and nearest ELSE with arbitrary admitted arms. Exactly one logical validation, correct 34.1 and source; branch folding must retain required diagnostic/evaluation effects. |
| LC-I-07 SELECT / STEP05 | Missing | Ordered WHEN edges, first true arm, OTHERWISE/no-match 7.3 and nesting. Each tested condition gets its own 34.* origin; no eager evaluation of later WHEN expressions. |
| LC-I-08 DO / STEP05 | Group/WHILE subset | Explicit loop-entry/test/body/step/exit blocks and runtime state only where needed. Counted/controlled/FOREVER/WHILE/UNTIL combinations, scalar/compound control, arbitrary numeric count, END-name checks and negative syntax. Header terms evaluated at specified times, not each iteration. |
| LC-I-09 LEAVE / STEP05 | Unnamed subset | Resolve named/unnamed active-loop destination, clear crossed loop state, preserve default/STRICTC reached-time error distinction. No unnecessary runtime search for statically proved target; dynamic cases stay explicit. |
| LC-I-10 ITERATE / STEP05 | Unnamed subset | Transfer to correct target's step/test point, preserving step timing, active outer loops and cleanup. Cover every DO form and named/invalid target; do not implement as LEAVE or direct body jump. |
| LC-I-11 ARG / STEP06 | Scalar comma subset | `PARSE UPPER ARG` through one runtime template executor, with frame argument presence and arbitrary comma/templates. Missing versus empty, placeholders, Unicode upper rules and authored errors. No second ARG splitter. |
| LC-I-12 PROCEDURE / STEP06 | Private routine subset | First-instruction eligibility, new pool and direct/indirect source-ordered EXPOSE for scalar/stem/resolved compound aliases. Pool binding replaces live view; recursive/callback/INTERPRET eligibility proof retained. No mandatory PROCEDURE after all labels. |
| LC-I-13 CALL / STEP06 | Local private routine subset | Local/BIF/static signed external resolution, quoted/VALUE forms, omitted positions, shared-label code and delayed CALL ON/OFF policy. Lower to checked call/handler effects, preserve RESULT drop/value, 43.1/16.1 and provider boundary. Inspect call windows, copies and handler suspension. |
| LC-I-14 RETURN / STEP06 | Value/no-value subset | Explicit completion with result presence, function-required 45.1, subroutine RESULT and outermost host status. One code body, recursive cleanup and condition propagation; empty string is a present result. |
| LC-I-15 EXIT / STEP06 | Basic return-like path | Program-root termination across internal/external/handler frames, optional value, EOF/fall-through and host completion. Propagate an exit outcome to correct owner and finalise once; do not silently return to a caller. |
| LC-I-16 PULL / STEP06 | Missing | Acquire selected queue once, otherwise configured input, then shared UPPER PARSE executor. Empty/EOF/NOTREADY, NUL and queue selection, nested contexts; no duplicated transport or template engine. |
| LC-I-17 PUSH / STEP06 | Missing | Evaluate optional expression once, null for bare form, selected queue front insertion. Effects/order, Unicode/NUL and nested isolation; share queue adapter with PULL/QUEUE. |
| LC-I-18 QUEUE / STEP06 | Missing | Same input semantics as PUSH, tail insertion; FIFO ordering, count/nonconsumption and shared selection. Check linked/host lifecycle; no global queue chosen accidentally. |
| LC-I-19 PARSE / STEP06 | Missing | ARG/PULL/SOURCE/LINEIN/VERSION/VALUE/VAR, UPPER, comma lists, placeholders, literal/variable patterns, absolute/relative positions. Compiler produces validated template records, runtime executes one parser; order-dependent variables/errors/source data and supported Unicode behavior. |
| LC-I-20 ADDRESS / STEP07 | Missing | Selection/swap, transient command, WITH resources and configured environment object protocol. One command evaluation, RC/.RC/.RS, error/failure/notready and resource lifetime. Compiler owns anchors; adapter owns command meaning. Test NUL transport policy explicitly. |
| LC-I-21 implicit command / STEP07 | Missing | Expression becomes one command through active ADDRESS state and lasting connections. No duplicate host route. Preserve expression errors, warning/source observations, return status and traps. |
| LC-I-22 NUMERIC / STEP07 | Partial | DIGITS/FORM/FUZZ default/value forms, once-only expression validation, local inheritance/restoration and BIF/display/arithmetic effects. Invalidate numeric facts; no per-operator rebuilding of unchanged configuration. |
| LC-I-23 SIGNAL / STEP07 | Missing | Direct/quoted/VALUE targets, first-label order, ON/OFF seven conditions, immediate handler transfer, SIGL/.SIGL and loop-state clearing. Compiler owns labels/edges; runtime owns policy/delivery. Missing labels and errors use causing clause; preserve existing signalorigin/CLASSIC_CONDITION contracts. |
| LC-I-24 TRACE / STEP07 | Missing full contract | Compile clause/result events with current VM metadata/exit protocol, share activation option/query state and command inhibition. Verify actual displayed values/origins and approved practical divergences. Disabled tracing must not require avoidable formatting/allocation. |
| LC-I-25 INTERPRET / STEP11 | Missing | Same compiler in group mode, live caller state, source-once semantics, complete/no-label groups, nested compile/execute, CALL-handler resumption versus SIGNAL discard, caller routines, RETURN/EXIT and bounded resident ownership. Include all LC-87 proof cases and installed/platform lifecycle; no second evaluator. |

## SAY dependency packet — CLC-STEP01b

This is the first concrete instruction dependency packet, inspected at local
`975afbc15` on 19 October 2026. It does not close LC-I-01. The retained
[CFC-SAY criteria and validation](../PLAN.md#say-validation),
[CLC-STEP01a controls](../PLAN.md#active-first-implementation-increment) and
[Unicode migration qualification](../PLAN.md#shared-b-unicode-interface-and-c-adapter-migration)
state the actual qualified inputs and modes; this ledger adds no new execution
verdict. Native whole-SAY acceptance remains a reusable control, not a new-C pass.

Paths under `tests/` below are relative to `research/level-l/compilers/`.

| Dependency | Current implementation / reusable independent expectations | Remaining closure obligation and owner |
|---|---|---|
| **SAY-D01 — C-SRC** | `levelc/llcparse.crexx` fuses adjacent X/B suffixes and preserves authored spelling/ranges; `llcfrontend.ordinalliteral` decodes them into ordinal constants in the existing String/Say path. `tests/say/forms-c.crexx`, `literal-c.crexx` and the invalid-literal rows in `tests/check-say.py` cover empty/partial groups, Unicode/NUL, adjacency and located refusal. `CFC-SAY-AC02` retains the focused radix proof, including all 256 ordinals. | Complete C token/clause admission and static validation of all unreachable source under STEP04. A passing literal subset does not establish the whole expression/source contract. |
| **SAY-D02 — C-EXPR / BF01–05** | `llcfrontend.statement` evaluates the optional operand once, then appends one `crexx.c.say`; an omitted operand supplies the empty value. `tests/say/forms-c.crexx` expects nested `inner` output before `outer=done`; `error-c.crexx` expects `before`, then arithmetic failure with no later output. `CFC-SAY-AC03` records both IR modes/VMs with assembler optimisation disabled. Native `compiler/tests/rexx_src/levelc_say_instruction.rexx` supplies an additional counter/recursive-output oracle. | Complete operators, compound reads, BIF and call forms, missing/empty presence and errors using shared runtime bodies. Port reusable cases through the new compiler as their prerequisites become available; do not inherit native closure. |
| **SAY-D03 — C-POOL / C-ACT / C-NUM** | `llcfrontend.lowerall` currently creates pool, argument and numeric slots for every routine; `levelc/runtime/llctarget.crexx` supplies pool/value/snapshot adapters. `tests/backend/calls-c.crexx`, `arguments-c.crexx` and `numeric-c.crexx` have independent expected rows in `tests/backend/cases.json`; native `levelc_say_pool_reads.rexx` and `levelc_say_bif_family.rexx` are reusable controls. | STEP03 owns live replaceable pool/context binding and shared-label activation, including recursion/restoration and configured text/numeric behavior. Current private PROCEDURE routines and copied packets do not close those contracts. |
| **SAY-D04 — C-HOST output** | `llcrxas` selects `llctarget.sayvalue`; that adapter performs `say value.asString(1)`. Existing native `compiler/tests/src/test_levelc_say_host_output.c` supplies exact NUL/high-scalar bytes and six callback invocations from `levelc_say_bytes.rexx`; `levelc_say_host_output` registers that control. | Run equivalent new-C output through the execution-owned length-aware output/configuration route, including once-only conversion, declared callback/error behavior and exact NUL/Unicode lengths. STEP03/08 own the shared adapter/host interface; a plain B SAY path is not complete configured Classic output proof. |
| **SAY-D05 — C-OBS / conditions / TRACE** | `levelc/llcir.crexx` declares SAY state `output`, failure `conversion.io`, ordinary continuation `returns`; generic optimisation retains it. `tests/backend-tests.crexx` and `tests/check-provenance.py` check existing logical identities/origins, and `tests/say/error-c.crexx` checks the reached fatal-error ordering. | Explicit authored clause identity, trace/result observations, actual condition origin and trapped/resumed failure must be preserved under STEP03/07. Source ranges and a conservative effect summary alone do not establish those semantics. New exceptional/resumable L edges require their own approved contract. |
| **SAY-D06 — C-OPT-01 / target quality** | `tests/check-say.py` rejects literal X/B pool reads/runtime decoding; retained `CFC-SAY-AC04` distinguishes complete RXAS from helper work. `llcfrontend.lowerall` still unconditionally constructs a pool; `llcrxas` supplies argument/context entry machinery. | C-owned requirement analysis must justify omitting unused setup for constant-only SAY, with read/write/call/RESULT/observation negatives. Preserve allocation, ABI, failure and lifetime obligations; do not make generic L discard named allocation/state effects. Register reuse remains the separate L/target contract. |
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

### BF05 — STEP04/06

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

This source inventory is pinned to local `975afbc15`, after the shared B Unicode
migration. It lists all **95 names exactly once**: 70 catalogue names, LOWER/UPPER
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

`P` in the **New C** column means only an admitted partial route exists:
LENGTH takes one actual, DIGITS none, ARG none or two present actuals. Their
existing new-C controls are `tests/backend/arguments-c.crexx`, `argv-c.crexx`,
`numeric-c.crexx` and `tests/frontend-tests.crexx`; emission uses current
`llctarget` adapters, not a completed generic direct-BIF binding. `O` means the
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

| Name | Fully qualified runtime symbol | Context | Reusable fixture | New C |
|---|---|---|---|---|
| `ABBREV` | `rexxclassicbifabbrev.rexxclassicbif_abbrev` | T | `B/Abbrev` | O |
| `ABS` | `rexxclassicbifabs.rexxclassicbif_abs` | N | `B/Abs` | O |
| `ADDRESS` | `rexxclassicbifaddress.rexxclassicbif_address_frame` | A | `B/Address` | O |
| `ARG` | `rexxclassicbifarg.rexxclassicbif_arg` | A | `B/Arg` | P |
| `B2X` | `rexxclassicbifb2x.rexxclassicbif_b2x` | — | `B/B2x` | O |
| `BITAND` | `rexxclassicbifs.rexxclassicbif_bitand` | O | `B/Bitwise` | O |
| `BITOR` | `rexxclassicbifs.rexxclassicbif_bitor` | O | `B/Bitwise` | O |
| `BITXOR` | `rexxclassicbifs.rexxclassicbif_bitxor` | O | `B/Bitwise` | O |
| `C2D` | `rexxclassicbifc2d.rexxclassicbif_c2d` | O, N | `B/C2d` | O |
| `C2X` | `rexxclassicbifc2x.rexxclassicbif_c2x` | O | `B/C2x` | O |
| `CENTER` | `rexxclassicbifcenter.rexxclassicbif_center` | T | `B/Center` | O |
| `CENTRE` | `rexxclassicbifcenter.rexxclassicbif_centre` | T | `B/Centre` | O |
| `CHANGESTR` | `rexxclassicbifchangestr.rexxclassicbif_changestr` | T | `B/Changestr` | O |
| `COMPARE` | `rexxclassicbifcompare.rexxclassicbif_compare` | T | `B/Compare` | O |
| `CONDITION` | `rexxclassicbifcondition.rexxclassicbif_condition` | A | `B/Condition` | O |
| `COPIES` | `rexxclassicbifcopies.rexxclassicbif_copies` | T | `B/Copies` | O |
| `COUNTSTR` | `rexxclassicbifcountstr.rexxclassicbif_countstr` | T | `B/Countstr` | O |
| `D2C` | `rexxclassicbifd2c.rexxclassicbif_d2c` | O, N | `B/D2c` | O |
| `D2X` | `rexxclassicbifd2x.rexxclassicbif_d2x` | O, N | `B/D2x` | O |
| `DATATYPE` | `rexxclassicbifdatatype.rexxclassicbif_datatype` | T | `B/Datatype` | O |
| `DATE` | `rexxclassicbifdate.rexxclassicbif_date` | K | `B/Date` | O |
| `DELSTR` | `rexxclassicbifdelstr.rexxclassicbif_delstr` | T | `B/Delstr` | O |
| `DELWORD` | `rexxclassicbifdelword.rexxclassicbif_delword` | W | `B/Delword` | O |
| `DIGITS` | `rexxclassicbifnumeric.rexxclassicbif_digits` | N | `B/Numeric` | P |
| `ERRORTEXT` | `rexxclassicbiferrortext.rexxclassicbif_errortext` | — | `B/Errortext` | O |
| `FORM` | `rexxclassicbifnumeric.rexxclassicbif_form` | N | `B/Numeric` | O |
| `FORMAT` | `rexxclassicbifformat.rexxclassicbif_format` | N | `B/Format` | O |
| `FUZZ` | `rexxclassicbifnumeric.rexxclassicbif_fuzz` | N | `B/Numeric` | O |
| `INSERT` | `rexxclassicbifinsert.rexxclassicbif_insert` | T | `B/Insert` | O |
| `LASTPOS` | `rexxclassicbiflastpos.rexxclassicbif_lastpos` | T | `B/Lastpos` | O |
| `LEFT` | `rexxclassicbifleft.rexxclassicbif_left` | T | `B/Left` | O |
| `LENGTH` | `rexxclassicbiflength.rexxclassicbif_length` | T | `B/Length` | P |
| `LOWER` | `rexxclassicbifs.rexxclassicbif_lower` | T | `B/Bifs` | O |
| `MAX` | `rexxclassicbifmax.rexxclassicbif_max` | N | `B/Max` | O |
| `MIN` | `rexxclassicbifmin.rexxclassicbif_min` | N | `B/Min` | O |
| `OVERLAY` | `rexxclassicbifoverlay.rexxclassicbif_overlay` | T | `B/Overlay` | O |
| `POS` | `rexxclassicbifpos.rexxclassicbif_pos` | T | `B/Pos` | O |
| `QUEUED` | `rexxclassicbifqueued.rexxclassicbif_queued` | Q | `B/Queued` | O |
| `RANDOM` | `rexxclassicbifrandom.rexxclassicbif_random` | R | `B/Random` | O |
| `REVERSE` | `rexxclassicbifreverse.rexxclassicbif_reverse` | T | `B/Reverse` | O |
| `RIGHT` | `rexxclassicbifright.rexxclassicbif_right` | T | `B/Right` | O |
| `SIGN` | `rexxclassicbifsign.rexxclassicbif_sign` | N | `B/Sign` | O |
| `SOURCELINE` | `rexxclassicbifsourceline.rexxclassicbif_sourceline` | S | `B/Sourceline` | O |
| `SPACE` | `rexxclassicbifspace.rexxclassicbif_space` | W | `B/Space` | O |
| `STRIP` | `rexxclassicbifstrip.rexxclassicbif_strip` | T | `B/Strip` | O |
| `SUBSTR` | `rexxclassicbifsubstr.rexxclassicbif_substr` | T | `B/Substr` | O |
| `SUBWORD` | `rexxclassicbifsubword.rexxclassicbif_subword` | W | `B/Subword` | O |
| `SYMBOL` | `rexxclassicbifsymbol.rexxclassicbif_symbol` | P | `B/Symbol` | O |
| `TIME` | `rexxclassicbiftime.rexxclassicbif_time` | K | `B/Time` | O |
| `TRACE` | `rexxclassicbiftrace.rexxclassicbif_trace_frame` | A | `B/Trace` | O |
| `TRANSLATE` | `rexxclassicbiftranslate.rexxclassicbif_translate` | T | `B/Translate` | O |
| `TRUNC` | `rexxclassicbiftrunc.rexxclassicbif_trunc` | N | `B/Trunc` | O |
| `UPPER` | `rexxclassicbifs.rexxclassicbif_upper` | T | `B/Bifs` | O |
| `VALUE` | `rexxclassicbifvalue.rexxclassicbif_value` | P | `B/Value` | O |
| `VERIFY` | `rexxclassicbifverify.rexxclassicbif_verify` | T | `B/Verify` | O |
| `WORD` | `rexxclassicbifword.rexxclassicbif_word` | W | `B/Word` | O |
| `WORDINDEX` | `rexxclassicbifwordindex.rexxclassicbif_wordindex` | W | `B/Wordindex` | O |
| `WORDLENGTH` | `rexxclassicbifwordlength.rexxclassicbif_wordlength` | W | `B/Wordlength` | O |
| `WORDPOS` | `rexxclassicbifwordpos.rexxclassicbif_wordpos` | W | `B/Wordpos` | O |
| `WORDS` | `rexxclassicbifwords.rexxclassicbif_words` | W | `B/Words` | O |
| `X2B` | `rexxclassicbifx2b.rexxclassicbif_x2b` | — | `B/X2b` | O |
| `X2C` | `rexxclassicbifx2c.rexxclassicbif_x2c` | O | `B/X2c` | O |
| `X2D` | `rexxclassicbifx2d.rexxclassicbif_x2d` | O, N | `B/X2d` | O |
| `XRANGE` | `rexxclassicbifxrange.rexxclassicbif_xrange` | O | `B/Xrange` | O |
| `CHARIN` | `rexxclassicbifstream.rexxclassicbif_charin` | H | `CS` | O |
| `CHAROUT` | `rexxclassicbifstream.rexxclassicbif_charout` | H | `CS` | O |
| `CHARS` | `rexxclassicbifstream.rexxclassicbif_chars` | H | `CS` | O |
| `LINEIN` | `rexxclassicbifstream.rexxclassicbif_linein` | H | `CS` | O |
| `LINEOUT` | `rexxclassicbifstream.rexxclassicbif_lineout` | H | `CS` | O |
| `LINES` | `rexxclassicbifstream.rexxclassicbif_lines` | H | `CS` | O |
| `QUALIFY` | `rexxclassicbifstream.rexxclassicbif_qualify` | H | `CS` | O |
| `STREAM` | `rexxclassicbifstream.rexxclassicbif_stream` | H | `CS` | O |
| `UNICODEVERSION` | `rexxclassicbifunicode.rexxclassicbif_unicodeversion` | U | `CU` | O |
| `TONFD` | `rexxclassicbifunicode.rexxclassicbif_tonfd` | U | `CU` | O |
| `TONFC` | `rexxclassicbifunicode.rexxclassicbif_tonfc` | U | `CU` | O |
| `TONFKD` | `rexxclassicbifunicode.rexxclassicbif_tonfkd` | U | `CU` | O |
| `TONFKC` | `rexxclassicbifunicode.rexxclassicbif_tonfkc` | U | `CU` | O |
| `ISNFD` | `rexxclassicbifunicode.rexxclassicbif_isnfd` | U | `CU` | O |
| `ISNFC` | `rexxclassicbifunicode.rexxclassicbif_isnfc` | U | `CU` | O |
| `ISNFKD` | `rexxclassicbifunicode.rexxclassicbif_isnfkd` | U | `CU` | O |
| `ISNFKC` | `rexxclassicbifunicode.rexxclassicbif_isnfkc` | U | `CU` | O |
| `TOUPPERCASE` | `rexxclassicbifunicode.rexxclassicbif_touppercase` | U | `CU` | O |
| `TOLOWERCASE` | `rexxclassicbifunicode.rexxclassicbif_tolowercase` | U | `CU` | O |
| `TOCASEFOLD` | `rexxclassicbifunicode.rexxclassicbif_tocasefold` | U | `CU` | O |
| `TOSIMPLECASEFOLD` | `rexxclassicbifunicode.rexxclassicbif_tosimplecasefold` | U | `CU` | O |
| `TOTURKICCASEFOLD` | `rexxclassicbifunicode.rexxclassicbif_toturkiccasefold` | U | `CU` | O |
| `TOTURKICSIMPLECASEFOLD` | `rexxclassicbifunicode.rexxclassicbif_toturkicsimplecasefold` | U | `CU` | O |
| `GRAPHEMECOUNT` | `rexxclassicbifunicode.rexxclassicbif_graphemecount` | U | `CU` | O |
| `GRAPHEMEREVERSE` | `rexxclassicbifunicode.rexxclassicbif_graphemereverse` | U | `CU` | O |
| `GRAPHEMESUBSTR` | `rexxclassicbifunicode.rexxclassicbif_graphemesubstr` | U | `CU` | O |
| `GRAPHEMEPOS` | `rexxclassicbifunicode.rexxclassicbif_graphemepos` | U | `CU` | O |
| `ENCODE` | `rexxclassicbifunicode.rexxclassicbif_encode` | U | `CU` | O |
| `DECODE` | `rexxclassicbifunicode.rexxclassicbif_decode` | U | `CU` | O |
| `ISDECODABLE` | `rexxclassicbifunicode.rexxclassicbif_isdecodable` | U | `CU` | O |
| `ISENCODINGSUPPORTED` | `rexxclassicbifunicode.rexxclassicbif_isencodingsupported` | U | `CU` | O |

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

Static validation used in-memory Python extraction of `levelc_direct_bifs`,
namespace/procedure lookup and fixture/link checks: all 95 rows match the live
source, return/context forms match, exactly four entries take the activation
reference, and every named direct fixture/local link exists. The pre-existing
25 instruction and 75 reference rows are byte-identical; `git diff --check`
passes. No product build or runtime test was run for this documentation slice.

## Complete reference-obligation routing

All 75 source obligation IDs are retained below. Every row is **open for the
new compiler** until its own implementation/verification record exists. The
source appendix carries the detailed contract and native status; this table
assigns the new owner/stage, not a copied native verdict. Tests for each row must
exercise its stated behavior and refusal/lifecycle cases. The CLC-STEP10 static
checkpoint excludes only dynamic LC-REF-062/LC-87 proof, closed in STEP11.

| ID | Reference obligation | New owning contract / delivery step |
|---|---|---|
| LC-REF-001 | Invocation modes COMMAND, FUNCTION, SUBROUTINE | C-HOST / STEP08 |
| LC-REF-002 | Initial source identity and line inventory | C-SRC / STEP04; source/host completion STEP07/08 |
| LC-REF-003 | Initial environment and stream selection | C-HOST / STEP08 |
| LC-REF-004 | Invocation arguments and omitted positions | C-HOST / STEP08 |
| LC-REF-005 | Caller-provided trap overrides | C-HOST / STEP08 |
| LC-REF-006 | Completion classes: no value, result, condition, resource failure, unable to continue | C-HOST / STEP08 |
| LC-REF-007 | Source characters, EOL/EOS and invalid-encoding `22.1` | C-SRC / STEP04; source/host completion STEP07/08 |
| LC-REF-008 | Configured extra blank characters and uppercase mapping | C-UNI / STEP03/04; BF01–04 and STEP08 limits |
| LC-REF-009 | Configured compare, substring, length and range ordering | C-UNI / STEP03/04; BF01–04 and STEP08 limits |
| LC-REF-010 | Binary-digit/character conversion hooks | C-UNI / STEP03/04; BF01–04 and STEP08 limits |
| LC-REF-011 | Fixed `00`–`FF` ↔ `U+0000`–`U+00FF` bridge, PAD/XRANGE and out-of-range signal | C-UNI / STEP03/04; BF01–04 and STEP08 limits |
| LC-REF-012 | Unicode scalar text and codepoint character positions | C-UNI / STEP03/04; BF01–04 and STEP08 limits |
| LC-REF-013 | No implicit BYTE/UTF8 switch, normalization or grapheme behavior | C-UNI / STEP03/04; BF01–04 and STEP08 limits |
| LC-REF-014 | DATATYPE extra letter/digit families, blanks and exponent limit | C-UNI / STEP03/04; BF01–04 and STEP08 limits |
| LC-REF-015 | Configured command completion, RC/.RC/.RS and ERROR/FAILURE | C-HOST / STEP08 |
| LC-REF-016 | External routine lookup with arguments, environment, streams and pool access | C-HOST / STEP08 |
| LC-REF-017 | Queue push, queue, pull and count services | C-HOST / STEP08 |
| LC-REF-018 | Character and line stream input/output, positioning, state and close | C-HOST / STEP08 |
| LC-REF-019 | Stream qualification, temporary names and availability queries | C-HOST / STEP08 |
| LC-REF-020 | Trap override/fallthrough for configuration hooks | C-HOST / STEP08 |
| LC-REF-021 | API_Set, API_Value, API_Drop with compound substitution | C-HOST / STEP08 |
| LC-REF-022 | Direct API set/value/drop without substitution | C-HOST / STEP08 |
| LC-REF-023 | API_ValueOther and visible-name iteration/reset | C-HOST / STEP08 |
| LC-REF-024 | API access only during enabled external/host callbacks | C-HOST / STEP08 |
| LC-REF-025 | Minimum numeric, literal, symbol and string limits | C-UNI / STEP03/04; BF01–04 and STEP08 limits |
| LC-REF-026 | Contextual instruction words, no general reserved words | C-SRC / STEP04; source/host completion STEP07/08 |
| LC-REF-027 | Quoted/doubled strings and radix suffix boundaries | C-SRC / STEP04; source/host completion STEP07/08 |
| LC-REF-028 | Hex/binary literal grouping and left padding | C-SRC / STEP04; source/host completion STEP07/08 |
| LC-REF-029 | Numeric constants, exponent signs and period-start tokens | C-SRC / STEP04; source/host completion STEP07/08 |
| LC-REF-030 | Comma continuation and inferred clause endings | C-SRC / STEP04; source/host completion STEP07/08 |
| LC-REF-031 | Nested comments and unterminated `6.*` messages | C-SRC / STEP04; source/host completion STEP07/08 |
| LC-REF-032 | Alternative negators and inferred blank/nonblank concatenation | C-SRC / STEP04; source/host completion STEP07/08 |
| LC-REF-033 | Top-syntax VALUE insertion, assignment, label and keyword promotion | C-SRC / STEP04; source/host completion STEP07/08 |
| LC-REF-034 | Reserved .MN/.RESULT/.RC/.RS/.SIGL and `50.1` | C-POOL / STEP03/05/06; STEP08 external API |
| LC-REF-035 | Function-name ending-period `51.1` | C-SRC / STEP04; source/host completion STEP07/08 |
| LC-REF-036 | Clause/label/null-clause program structure | C-SRC / STEP04; source/host completion STEP07/08 |
| LC-REF-037 | IF/THEN/ELSE branch selection and nearest ELSE | LC-I-06–10 / STEP05 |
| LC-REF-038 | SELECT/WHEN/OTHERWISE order and absent-match `7.3` | LC-I-06–10 / STEP05 |
| LC-REF-039 | Simple DO grouping and empty body | LC-I-06–10 / STEP05 |
| LC-REF-040 | Counted, controlled, FOREVER, WHILE and UNTIL DO | LC-I-06–10 / STEP05 |
| LC-REF-041 | LEAVE/ITERATE nesting and named loop targets | LC-I-06–10 / STEP05 |
| LC-REF-042 | END-name matching and group-label branch restrictions | C-SRC / STEP04; source/host completion STEP07/08 |
| LC-REF-043 | Expression terms, prefix, power and arithmetic precedence | C-EXPR / STEP04; STEP05/07 contexts |
| LC-REF-044 | Left-associative Classic power | C-EXPR / STEP04; STEP05/07 contexts |
| LC-REF-045 | Normal/strict comparisons and configured character ordering | C-EXPR / STEP04; STEP05/07 contexts |
| LC-REF-046 | Logical AND/OR/XOR and prefix negation with exact values | C-EXPR / STEP04; STEP05/07 contexts |
| LC-REF-047 | PARSE ARG/PULL/SOURCE/LINEIN/VERSION/VALUE/VAR and UPPER | LC-I-11/16/19 / STEP06 |
| LC-REF-048 | PARSE targets, placeholders, literal/variable patterns and positions | LC-I-11/16/19 / STEP06 |
| LC-REF-049 | Pool dropped, implicit, exposed and scalar/stem binding states | C-POOL / STEP03/05/06; STEP08 external API |
| LC-REF-050 | Set/value/drop, stem-tail propagation and recursive exposure | C-POOL / STEP03/05/06; STEP08 external API |
| LC-REF-051 | Compound-tail evaluation without NOVALUE, final lookup with NOVALUE | C-POOL / STEP03/05/06; STEP08 external API |
| LC-REF-052 | Constant symbols, reserved pool 0 and SIGL/.SIGL updates | C-POOL / STEP03/05/06; STEP08 external API |
| LC-REF-053 | Numeric DIGITS/FORM/FUZZ and arithmetic error/condition model | C-EXPR / STEP04; STEP05/07 contexts |
| LC-REF-054 | Exact logical values and contextual `34.*` identities | C-EXPR / STEP04; STEP05/07 contexts |
| LC-REF-055 | Function/CALL omitted arguments and resolution order | C-ACT and LC-I-13–15 / STEP06; STEP08 host completion |
| LC-REF-056 | Function RETURN value, `45.1` and RESULT/.RESULT lifecycle | C-ACT and LC-I-13–15 / STEP06; STEP08 host completion |
| LC-REF-057 | Program initialization and clause-boundary HALT/trap/TRACE work | C-HOST / STEP08 |
| LC-REF-058 | ADDRESS selection/swap, transient command and WITH redirection | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 |
| LC-REF-059 | CALL ON/OFF delayed condition handlers | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 |
| LC-REF-060 | DROP direct and parenthesized variable lists | C-POOL / STEP03/05/06; STEP08 external API |
| LC-REF-061 | EXIT value, fallthrough and finalization | C-ACT and LC-I-13–15 / STEP06; STEP08 host completion |
| LC-REF-062 | INTERPRET source, HALT, syntax and label restrictions | LC-I-25 / STEP11; LC-87-01–05 |
| LC-REF-063 | NUMERIC DIGITS/FORM/FUZZ validation and defaults | C-EXPR / STEP04; STEP05/07 contexts |
| LC-REF-064 | OPTIONS word handling and unknown-option policy | LC-I-05 / STEP05 |
| LC-REF-065 | PROCEDURE pool creation and EXPOSE aliases | C-POOL / STEP03/05/06; STEP08 external API |
| LC-REF-066 | PUSH/QUEUE ordering and null-expression value | LC-I-17/18 / STEP06 |
| LC-REF-067 | RETURN function/subroutine/outermost lifecycle | C-ACT and LC-I-13–15 / STEP06; STEP08 host completion |
| LC-REF-068 | SAY default output and optional empty expression | LC-I-01 / STEP05; STEP07/08 observations/host |
| LC-REF-069 | SIGNAL branch/trap modes and loop-state clearing | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 |
| LC-REF-070 | TRACE options, interactive mode, skip/inhibit and source/result tracing | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 |
| LC-REF-071 | SYNTAX, HALT, ERROR, FAILURE, NOTREADY, NOVALUE, LOSTDIGITS | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 |
| LC-REF-072 | Message catalog, .MN, source/line traceback and trap handling | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 |
| LC-REF-073 | Immediate SIGNAL ON, delayed CALL ON and HALT buffering | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 |
| LC-REF-074 | CONDITION BIF state fields | C-OBS and LC-I-13/20/21/23/24 / STEP07/08 |
| LC-REF-075 | NOP as a statement with no visible effect | LC-I-04 / STEP05; STEP07 observations |

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
receipts. Measure any new nested aggregate in normal Debug and the maintained
sanitizer tree before registration; set serial/resource/timeout properties.
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
