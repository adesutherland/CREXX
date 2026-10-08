# Level C reference obligations

This is the detailed reference appendix to the
[Level C compatibility worklist](levelc-compatibility-worklist.md), under the
[Release 1 plan](../../release-1-plan.md). It enumerates the non-BIF contracts
in the [Classic compliance reference](../../../compiler/docs/levelc_compliance_reference.md).
The worklist separately tracks the 70 recognized BIF names and 36 existing
syntax catalogue entries. A bounded slice is not a conformance verdict; every
row below needs direct evidence or an individually approved exception before
`LC-AC-01/04` can close.

**Scope notation:** `text` means the approved Unicode scalar Level C route,
including its Latin-1 ordinal bridge where byte-valued conversion is required;
`host` requires a named host adapter and platform evidence; `source` requires
source-encoding and source-location evidence. The former BYTE-default/opt-in
UTF8 profile obligations were replaced by Adrian's 2026-10-04 decision; direct
binary consumers remain separate from compiled Level C. These rows track
reference obligations and shared work, so a closed whole instruction can still
depend on open host or condition rows. The
[worklist](levelc-compatibility-worklist.md#whole-instruction-programme) owns
instruction closure and detailed receipts. The
[detailed compatibility-layer review](../../../compiler/docs/levelc_compatibility_layer.md)
maps the current source/runtime/test paths. Adrian's later clarification keeps
changes to Unicode/I/O infrastructure pending assessment and leaves
Unicode-caused signals and logic errors undefined in B/C/G; existing explicit typed/Unicode mechanisms
and RexxScript's separate binary-capable values remain distinct.

**Status reconciliation, 2026-10-08.** `LC-I-01–24` have closed whole-
instruction reviews; `LC-I-25` INTERPRET is parked and not implemented.
The [remaining-gap decision register](levelc-compatibility-worklist.md#remaining-gap-decision-register-2026-10-07)
owns current cross-cutting status. A row's older bounded proof below is not a
claim that its instruction is still open, and an instruction closure does not
close an unfinished host/source/BIF obligation.

| ID | Reference obligation | Scope | Current state / remaining proof |
| --- | --- | --- | --- |
| LC-REF-001 | Invocation modes COMMAND, FUNCTION, SUBROUTINE | host | Open: `API_Start`-equivalent entry and lifecycle |
| LC-REF-002 | Initial source identity and line inventory | source | Ordinary SOURCELINE retains original physical lines, shared locally and isolated per separately compiled unit; full initial identity/PARSE SOURCE, mapped-source count-zero behavior and physical source NUL truncation remain LC-GAP-06 |
| LC-REF-003 | Initial environment and stream selection | host | Admitted ADDRESS/default input/selected queue state exists; full configuration/stream initialization remains LC-GAP-03; new stream infrastructure is deferred by Adrian |
| LC-REF-004 | Invocation arguments and omitted positions | host | ARG, CALL, PROCEDURE and RETURN instruction reviews closed on the shared frame; wider host invocation modes and the C-string `rxvml_run()` NUL limit remain under LC-GAP-04 |
| LC-REF-005 | Caller-provided trap overrides | host | Open: trap/configuration lifecycle |
| LC-REF-006 | Completion classes: no value, result, condition, resource failure, unable to continue | host | Open: observable result/error contract |
| LC-REF-007 | Source characters, EOL/EOS and invalid-encoding `22.1` | source | Front end source/literal and selected error fixtures exist; physical embedded NUL silently compiles only the prefix; configured source/encoding and full location proof remain LC-GAP-06 |
| LC-REF-008 | Configured extra blank characters and uppercase mapping | text | Runtime foundations and selected instruction proof; scanner-to-runtime parity open |
| LC-REF-009 | Configured compare, substring, length and range ordering | text | Unicode character BIF sweep and implicit TRANSLATE ordinals pass focused checks; complete reference/context integration open |
| LC-REF-010 | Binary-digit/character conversion hooks | text | Fixed Latin-1 ordinal bridge for Level C byte-valued BIFs; complete reference/context proof open |
| LC-REF-011 | Fixed `00`–`FF` ↔ `U+0000`–`U+00FF` bridge, PAD/XRANGE and out-of-range signal | text | All 256 round trips and selected conversion/BIF cases pass; complete error and host proof open |
| LC-REF-012 | Unicode scalar text and codepoint character positions | text | Literal, SAY, PARSE/ARG and character-BIF slices pass; complete instruction/source/host proof open |
| LC-REF-013 | No implicit BYTE/UTF8 switch, normalization or grapheme behavior | text | Approved language boundary; positive/negative whole-program proof open |
| LC-REF-014 | DATATYPE extra letter/digit families, blanks and exponent limit | text | Shared classifier and focused extra letter/digit/blank/exponent tests exist; downstream BIN/HEX converter parity is an inspection-only custom-context concern, with complete caller/scanner/context proof open under LC-GAP-02/08 |
| LC-REF-015 | Configured command completion, RC/.RC/.RS and ERROR/FAILURE | host | ADDRESS and implicit-command instruction reviews closed on the admitted adapter; complete host configuration and transport remain under LC-GAP-03/05 |
| LC-REF-016 | External routine lookup with arguments, environment, streams and pool access | host | CALL closed at Adrian's approved static signed boundary; broader host routine/pool services remain under LC-GAP-03/04 |
| LC-REF-017 | Queue push, queue, pull and count services | host | PULL/PUSH/QUEUE and direct QUEUED share the execution-local selected repository; QUEUED direct/linked opt/no-opt proof checks nonconsumption, NUL and isolation. Existing RXQUEUE selects named queues for hosts; wider C/Level C selector/configuration remains LC-GAP-03 |
| LC-REF-018 | Character and line stream input/output, positioning, state and close | host | Open and deferred by Adrian: eight Classic stream BIFs have no direct implementations; current B fileio/PARSE LINEIN/ADDRESS services do not close positioning, count, state and lifecycle obligations |
| LC-REF-019 | Stream qualification, temporary names and availability queries | host | Open and deferred pending compatibility/architectural assessment: stream qualification/temporary names/availability adapter; RXPA stream proposal is unapproved |
| LC-REF-020 | Trap override/fallthrough for configuration hooks | host | Open: null-result and replacement behavior |
| LC-REF-021 | API_Set, API_Value, API_Drop with compound substitution | host | Open: externally visible pool API |
| LC-REF-022 | Direct API set/value/drop without substitution | host | Open: externally visible pool API |
| LC-REF-023 | API_ValueOther and visible-name iteration/reset | host | Open: source/argument state and iterator proof |
| LC-REF-024 | API access only during enabled external/host callbacks | host | Open: access-window and lifecycle negatives |
| LC-REF-025 | Minimum numeric, literal, symbol and string limits | text | Approved BIF positional/count WHOLE uses inclusive signed64 and 40.12 outside it; radix WHOLENUM retains arbitrary precision. Full numeric/source/string limits, intermediate-width/resource failures and matching messages remain LC-GAP-02/06 |
| LC-REF-026 | Contextual instruction words, no general reserved words | source | Front end fixtures; full reference edge cases open |
| LC-REF-027 | Quoted/doubled strings and radix suffix boundaries | source | Front end fixtures and bounded empty quoted-string runtime proof; configured conversion/limit proof open |
| LC-REF-028 | Hex/binary literal grouping and left padding | source | Fixed Latin-1 ordinal source-literal route has opt/no-opt, tree and linked proof; remaining lexical/reference cases open |
| LC-REF-029 | Numeric constants, exponent signs and period-start tokens | source | Front end partial; full Classic lexer pass open |
| LC-REF-030 | Comma continuation and inferred clause endings | source | Front end partial; whitespace edge cases open |
| LC-REF-031 | Nested comments and unterminated `6.*` messages | source | Front end diagnostics; reference edge cases open |
| LC-REF-032 | Alternative negators and inferred blank/nonblank concatenation | source | Front end plus expression slice, including adjacent function calls after blank concatenation; full lexical equivalence open |
| LC-REF-033 | Top-syntax VALUE insertion, assignment, label and keyword promotion | source | Front end fixtures; complete context matrix open |
| LC-REF-034 | Reserved .MN/.RESULT/.RC/.RS/.SIGL and `50.1` | text | Front end partial; runtime state and diagnostic proof open |
| LC-REF-035 | Function-name ending-period `51.1` | source | Front end diagnostic proof open |
| LC-REF-036 | Clause/label/null-clause program structure | source | Front end fixtures; runtime lifecycle open |
| LC-REF-037 | IF/THEN/ELSE branch selection and nearest ELSE | text | Whole IF instruction closed under LC-AC-67; shared condition/TRACE lifecycle open elsewhere |
| LC-REF-038 | SELECT/WHEN/OTHERWISE order and absent-match `7.3` | text | Whole SELECT instruction closed under LC-AC-68; shared condition/TRACE lifecycle open elsewhere |
| LC-REF-039 | Simple DO grouping and empty body | text | Whole DO instruction closed under LC-AC-65; shared clause/TRACE lifecycle open elsewhere |
| LC-REF-040 | Counted, controlled, FOREVER, WHILE and UNTIL DO | text | Whole DO instruction closed under LC-AC-65, including compound controls and arbitrary numeric counts; NUMERIC and TRACE instructions also closed, while cross-expression/condition work remains under LC-GAP-07 |
| LC-REF-041 | LEAVE/ITERATE nesting and named loop targets | text | Both whole instructions closed under LC-AC-69/70, with default/STRICTC timing; shared lifecycle work open elsewhere |
| LC-REF-042 | END-name matching and group-label branch restrictions | source | Front end diagnostics; complete reference matrix open |
| LC-REF-043 | Expression terms, prefix, power and arithmetic precedence | text | Bounded expression slice; full numeric/error equivalence open |
| LC-REF-044 | Left-associative Classic power | text | Open: reference and optimization parity proof |
| LC-REF-045 | Normal/strict comparisons and configured character ordering | text | Bounded expression slice; numeric/configuration edges open |
| LC-REF-046 | Logical AND/OR/XOR and prefix negation with exact values | text | Bounded expression slice; contextual `34.5/34.6` open |
| LC-REF-047 | PARSE ARG/PULL/SOURCE/LINEIN/VERSION/VALUE/VAR and UPPER | host | Whole PARSE instruction closed on seven agreed sources through shared `parseplan`; EXTERNAL/NUMERIC sources excluded from initial Level C by Adrian; configured host input remains under LC-GAP-03 |
| LC-REF-048 | PARSE targets, placeholders, literal/variable patterns and positions | text | Whole PARSE instruction closed on the shared template executor, including errors, source positions, opt/no-opt and linked execution; cross-source/host lifecycle remains in its owning rows |
| LC-REF-049 | Pool dropped, implicit, exposed and scalar/stem binding states | text | DROP/assignment and whole PROCEDURE reviews close admitted private/shared pool, direct/indirect scalar/stem/exact compound exposure and nested alias lifetime; externally visible/API lifecycle remains LC-GAP-03/04 |
| LC-REF-050 | Set/value/drop, stem-tail propagation and recursive exposure | text | Shared pool owns set/drop/stem propagation and source-ordered direct/indirect EXPOSE with fixed resolved compound aliases; external/API alias/access proof remains LC-GAP-03/04 |
| LC-REF-051 | Compound-tail evaluation without NOVALUE, final lookup with NOVALUE | text | Compound assignment/DROP paths closed; remaining read/condition/alias proof open |
| LC-REF-052 | Constant symbols, reserved pool 0 and SIGL/.SIGL updates | text | SIGNAL and local-label instruction review closed; full reserved-state and source/host proof remain under LC-GAP-06/09 |
| LC-REF-053 | Numeric DIGITS/FORM/FUZZ and arithmetic error/condition model | text | NUMERIC instruction closes activation-local settings and LOSTDIGITS. ABS/MAX/MIN/SIGN/TRUNC/FORMAT use approved ANSI caller-DIGITS/FORM rounding in C and existing B/G typed decimal APIs, with four-mode context tests; full arithmetic/BIF equivalence remains LC-GAP-02/07 |
| LC-REF-054 | Exact logical values and contextual `34.*` identities | text | IF/SELECT/DO checks pass; other contexts open |
| LC-REF-055 | Function/CALL omitted arguments and resolution order | host | Whole CALL review closed for local/BIF and approved static signed external providers, omitted values, RESULT and delayed handlers; broader host modes remain under LC-GAP-04 |
| LC-REF-056 | Function RETURN value, `45.1` and RESULT/.RESULT lifecycle | text | Whole RETURN review closed across main/local/function paths; broader host completion classes remain under LC-GAP-04 |
| LC-REF-057 | Program initialization and clause-boundary HALT/trap/TRACE work | host | Open: real host HALT and broader processor lifecycle. Known DATE/TIME integration defect: no production clause-sample refresh. Linked opt/no-opt TIME retains its first timestamp and elapsed zero across a one-second sleep, while Regina advances. DATE shares the inspected sample; calendar crossing was not probed. Owner LC-GAP-02/04 |
| LC-REF-058 | ADDRESS selection/swap, transient command and WITH redirection | host | Whole ADDRESS review closed, including configured callback path; `LC-HOST-ADDRESS-NUL` and full host adapter remain under LC-GAP-03/05 |
| LC-REF-059 | CALL ON/OFF delayed condition handlers | host | Whole CALL review closed for policy replacement, four delayed conditions, nested isolation, buffered HALT and static signed targets; real host HALT and cross-service producers remain under LC-GAP-04 |
| LC-REF-060 | DROP direct and parenthesized variable lists | text | Whole DROP instruction closed under LC-AC-62, including arbitrary compounds, indirect lists and Regina invalid-word skip; external pool API open elsewhere |
| LC-REF-061 | EXIT value, fallthrough and finalization | host | Whole EXIT review closed with Regina caller-return fallthrough; broader host completion remains under LC-GAP-04 |
| LC-REF-062 | INTERPRET source, HALT, syntax and label restrictions | text | Parked 2026-10-07, not implemented; parser recognizes form but lowerer rejects it; Release 1 disposition open under LC-GAP-01 |
| LC-REF-063 | NUMERIC DIGITS/FORM/FUZZ validation and defaults | text | Whole NUMERIC instruction review closed under LC-I-22; cross-expression numeric equivalence remains under LC-GAP-07 |
| LC-REF-064 | OPTIONS word handling and unknown-option policy | source | Whole OPTIONS instruction closed under LC-AC-66; shared condition/host work remains elsewhere |
| LC-REF-065 | PROCEDURE pool creation and EXPOSE aliases | text | Whole PROCEDURE review closes private pool, source-ordered direct/indirect scalar/stem/exact compound aliases, nested lifetime and first-instruction 17.1. The admitted CALL/RETURN/EXIT instruction reviews are also closed; wider host/API lifecycle remains LC-GAP-03/04 |
| LC-REF-066 | PUSH/QUEUE ordering and null-expression value | host | Whole PUSH and QUEUE reviews closed with selected-queue ordering; external host selection remains under LC-GAP-03 |
| LC-REF-067 | RETURN function/subroutine/outermost lifecycle | text | Whole RETURN review closed; broader host invocation modes remain under LC-GAP-04 |
| LC-REF-068 | SAY default output and optional empty expression | host | Whole SAY review closes expression/childless ordering, length-aware default/configured callback, NUL/text and opt/no-opt/linked proof. BIF, TRACE/SIGNAL and wider host/source obligations retain their own owners; no full host closure follows |
| LC-REF-069 | SIGNAL branch/trap modes and loop-state clearing | host | Whole SIGNAL review closed on the shared frame, including direct/VALUE branches and seven handler identities; real host HALT production remains under LC-GAP-04 |
| LC-REF-070 | TRACE options, interactive mode, skip/inhibit and source/result tracing | host | Whole TRACE review closed under approved practical divergences; correct displayed scalar values remain required; shared source/host proof remains under LC-GAP-05/06 |
| LC-REF-071 | SYNTAX, HALT, ERROR, FAILURE, NOTREADY, NOVALUE, LOSTDIGITS | host | SIGNAL/CALL delivery and live SYNTAX, NOVALUE, ADDRESS ERROR/FAILURE/NOTREADY and NUMERIC LOSTDIGITS producers have instruction evidence; real host HALT and complete cross-service producer/state proof remain under LC-GAP-02/04 |
| LC-REF-072 | Message catalog, .MN, source/line traceback and trap handling | host | Shared generated English catalog and ERRORTEXT direct/linked opt/no-opt proof exist; admitted CONDITION descriptions and authored BIF/source anchors have focused proof. Full .MN, all message producers, encoded source/traceback and trapped-event locale selection remain LC-GAP-02/06 |
| LC-REF-073 | Immediate SIGNAL ON, delayed CALL ON and HALT buffering | host | Whole SIGNAL/CALL reviews closed on frame-local policy and delivery; real host HALT production and broader host lifecycle remain under LC-GAP-04 |
| LC-REF-074 | CONDITION BIF state fields | host | Admitted C/D/E/I/S fields, seven record IDs, ON/OFF/DELAY, CALL extra clearing and child isolation have unit/compiler proof; live SYNTAX/NOVALUE/ADDRESS ERROR/FAILURE/NOTREADY/LOSTDIGITS producers have evidence. Real host HALT and complete cross-service lifecycle remain LC-GAP-02/04 |
| LC-REF-075 | NOP as a statement with no visible effect | text | Whole NOP instruction closed under LC-AC-64; shared label/TRACE lifecycle remains elsewhere |
