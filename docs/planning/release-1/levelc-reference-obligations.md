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
instruction closure and detailed receipts.

| ID | Reference obligation | Scope | Current state / remaining proof |
| --- | --- | --- | --- |
| LC-REF-001 | Invocation modes COMMAND, FUNCTION, SUBROUTINE | host | Open: `API_Start`-equivalent entry and lifecycle |
| LC-REF-002 | Initial source identity and line inventory | source | Open: diagnostics, PARSE SOURCE, SOURCELINE |
| LC-REF-003 | Initial environment and stream selection | host | Open: configuration adapter |
| LC-REF-004 | Invocation arguments and omitted positions | host | Activation frames, omitted/present distinction, Unicode ARG and ARG BIF, explicit-length `rxvml` and `crexxsaa` main entries, and direct CALL expression actuals have bounded proof; full invocation and label lifecycle audit open under LC-I-11/LC-STEP-63T and the broader host-service rows |
| LC-REF-005 | Caller-provided trap overrides | host | Open: trap/configuration lifecycle |
| LC-REF-006 | Completion classes: no value, result, condition, resource failure, unable to continue | host | Open: observable result/error contract |
| LC-REF-007 | Source characters, EOL/EOS and invalid-encoding `22.1` | source | Front end; configured source service and error proof open |
| LC-REF-008 | Configured extra blank characters and uppercase mapping | text | Runtime foundations and selected instruction proof; scanner-to-runtime parity open |
| LC-REF-009 | Configured compare, substring, length and range ordering | text | Unicode character BIF sweep and implicit TRANSLATE ordinals pass focused checks; complete reference/context integration open |
| LC-REF-010 | Binary-digit/character conversion hooks | text | Fixed Latin-1 ordinal bridge for Level C byte-valued BIFs; complete reference/context proof open |
| LC-REF-011 | Fixed `00`–`FF` ↔ `U+0000`–`U+00FF` bridge, PAD/XRANGE and out-of-range signal | text | All 256 round trips and selected conversion/BIF cases pass; complete error and host proof open |
| LC-REF-012 | Unicode scalar text and codepoint character positions | text | Literal, SAY, PARSE/ARG and character-BIF slices pass; complete instruction/source/host proof open |
| LC-REF-013 | No implicit BYTE/UTF8 switch, normalization or grapheme behavior | text | Approved language boundary; positive/negative whole-program proof open |
| LC-REF-014 | DATATYPE extra letter/digit families, blanks and exponent limit | text | Runtime helper exists; complete caller/context proof open |
| LC-REF-015 | Configured command completion, RC/.RC/.RS and ERROR/FAILURE | host | Open: command adapter and conditions |
| LC-REF-016 | External routine lookup with arguments, environment, streams and pool access | host | Open: invocation adapter and nested lifecycle |
| LC-REF-017 | Queue push, queue, pull and count services | host | Open: configured external data queue |
| LC-REF-018 | Character and line stream input/output, positioning, state and close | host | Open: Classic stream adapter and BIF integration |
| LC-REF-019 | Stream qualification, temporary names and availability queries | host | Open: stream adapter and BIF integration |
| LC-REF-020 | Trap override/fallthrough for configuration hooks | host | Open: null-result and replacement behavior |
| LC-REF-021 | API_Set, API_Value, API_Drop with compound substitution | host | Open: externally visible pool API |
| LC-REF-022 | Direct API set/value/drop without substitution | host | Open: externally visible pool API |
| LC-REF-023 | API_ValueOther and visible-name iteration/reset | host | Open: source/argument state and iterator proof |
| LC-REF-024 | API access only during enabled external/host callbacks | host | Open: access-window and lifecycle negatives |
| LC-REF-025 | Minimum numeric, literal, symbol and string limits | text | Open: documented limits and over-limit messages |
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
| LC-REF-040 | Counted, controlled, FOREVER, WHILE and UNTIL DO | text | Whole DO instruction closed under LC-AC-65, including compound controls and arbitrary numeric counts; shared NUMERIC/condition/TRACE work open elsewhere |
| LC-REF-041 | LEAVE/ITERATE nesting and named loop targets | text | Both whole instructions closed under LC-AC-69/70, with default/STRICTC timing; shared lifecycle work open elsewhere |
| LC-REF-042 | END-name matching and group-label branch restrictions | source | Front end diagnostics; complete reference matrix open |
| LC-REF-043 | Expression terms, prefix, power and arithmetic precedence | text | Bounded expression slice; full numeric/error equivalence open |
| LC-REF-044 | Left-associative Classic power | text | Open: reference and optimization parity proof |
| LC-REF-045 | Normal/strict comparisons and configured character ordering | text | Bounded expression slice; numeric/configuration edges open |
| LC-REF-046 | Logical AND/OR/XOR and prefix negation with exact values | text | Bounded expression slice; contextual `34.5/34.6` open |
| LC-REF-047 | PARSE ARG/PULL/SOURCE/LINEIN/VERSION/VALUE/VAR and UPPER | host | Shared `parseplan` runs ARG and VAR/VALUE templates; remaining source acquisition and whole PARSE instruction review open |
| LC-REF-048 | PARSE targets, placeholders, literal/variable patterns and positions | text | Arbitrary target lists, commas, compound targets and static/dynamic patterns/positions run through one executor; whole PARSE reference/error review open |
| LC-REF-049 | Pool dropped, implicit, exposed and scalar/stem binding states | text | DROP and assignment whole instructions closed; scalar/stem EXPOSE bounded; full cross-activation/API lifecycle open |
| LC-REF-050 | Set/value/drop, stem-tail propagation and recursive exposure | text | Shared pool handles whole assignment and DROP contracts; indirect EXPOSE and external/API alias proof open |
| LC-REF-051 | Compound-tail evaluation without NOVALUE, final lookup with NOVALUE | text | Compound assignment/DROP paths closed; remaining read/condition/alias proof open |
| LC-REF-052 | Constant symbols, reserved pool 0 and SIGL/.SIGL updates | text | Front end/runtime pieces; state and label-search proof open |
| LC-REF-053 | Numeric DIGITS/FORM/FUZZ and arithmetic error/condition model | text | RexxValue foundation; full context and condition proof open |
| LC-REF-054 | Exact logical values and contextual `34.*` identities | text | IF/SELECT/DO checks pass; other contexts open |
| LC-REF-055 | Function/CALL omitted arguments and resolution order | host | Direct local CALL expression actuals/omissions, local-before-BIF resolution, quoted BIF bypass and ordinary RESULT presence/drop pass bounded proof under LC-STEP-75C; delayed local/BIF handler frames have controlled producer-surrogate proof under LC-STEP-75D; external resolution and whole CALL lifecycle open |
| LC-REF-056 | Function RETURN value, `45.1` and RESULT/.RESULT lifecycle | text | Bounded local RETURN; complete call state open |
| LC-REF-057 | Program initialization and clause-boundary HALT/trap/TRACE work | host | Open: processor lifecycle |
| LC-REF-058 | ADDRESS selection/swap, transient command and WITH redirection | host | Front end; configured execution open |
| LC-REF-059 | CALL ON/OFF delayed condition handlers | host | Per-activation policy and one generated local/BIF dispatcher have opt/no-opt controlled event-injection proof under LC-STEP-75D; IF/WHEN/DO and transfer checkpoint cases pass in both modes. Real producers, the complete clause/lifecycle matrix, external targets, dynamic missing-handler source identity and whole CALL remain open |
| LC-REF-060 | DROP direct and parenthesized variable lists | text | Whole DROP instruction closed under LC-AC-62, including arbitrary compounds, indirect lists and Regina invalid-word skip; external pool API open elsewhere |
| LC-REF-061 | EXIT value, fallthrough and finalization | host | Empty EXIT slice; value/finalization open |
| LC-REF-062 | INTERPRET source, HALT, syntax and label restrictions | text | Front end; execution and `47.1` open |
| LC-REF-063 | NUMERIC DIGITS/FORM/FUZZ validation and defaults | text | Front end/runtime pieces; instruction integration open |
| LC-REF-064 | OPTIONS word handling and unknown-option policy | source | Whole OPTIONS instruction closed under LC-AC-66; shared condition/host work remains elsewhere |
| LC-REF-065 | PROCEDURE pool creation and EXPOSE aliases | text | Whole PROCEDURE instruction closed under LC-74-01–05: private pool, source-ordered direct/indirect scalar/stem/exact compound aliases, nested lifetime and first-instruction 17.1; adjacent CALL/RETURN/EXIT and full lifecycle remain open under their own rows |
| LC-REF-066 | PUSH/QUEUE ordering and null-expression value | host | Front end; queue service open |
| LC-REF-067 | RETURN function/subroutine/outermost lifecycle | text | Bounded local RETURN; invocation modes open |
| LC-REF-068 | SAY default output and optional empty expression | host | Whole SAY instruction closed under LC-AC-57: expression/childless ordering, one length-aware default/configured host callback, embedded NUL and Unicode text, optimized/no-opt and linked evidence pass. Missing BIF, TRACE, SIGNAL and external-host services remain in their own rows. |
| LC-REF-069 | SIGNAL branch/trap modes and loop-state clearing | host | Parser and Regina reference contract reviewed under LC-STEP-63T-1; approved frame-local AST/emitter and runtime implementation open |
| LC-REF-070 | TRACE options, interactive mode, skip/inhibit and source/result tracing | host | Front end/runtime pieces; complete trace contract open |
| LC-REF-071 | SYNTAX, HALT, ERROR, FAILURE, NOTREADY, NOVALUE, LOSTDIGITS | host | Open: Classic condition state and delivery |
| LC-REF-072 | Message catalog, .MN, source/line traceback and trap handling | host | Open: Classic diagnostic/condition bridge |
| LC-REF-073 | Immediate SIGNAL ON, delayed CALL ON and HALT buffering | host | SIGNAL and CALL policy replacement plus one buffered nested HALT pass bounded activation proof; real event delivery, clause boundaries and host HALT lifecycle open |
| LC-REF-074 | CONDITION BIF state fields | host | BIF name recognized; condition state integration open |
| LC-REF-075 | NOP as a statement with no visible effect | text | Whole NOP instruction closed under LC-AC-64; shared label/TRACE lifecycle remains elsewhere |
