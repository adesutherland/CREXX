# Level C reference obligations

This is the detailed reference appendix to the
[Level C compatibility worklist](levelc-compatibility-worklist.md), under the
[Release 1 plan](../../release-1-plan.md). It enumerates the non-BIF contracts
in the [Classic compliance reference](../../../compiler/docs/levelc_compliance_reference.md).
The worklist separately tracks the 70 recognized BIF names and 36 existing
syntax catalogue entries. A bounded slice is not a conformance verdict; every
row below needs direct evidence or an individually approved exception before
`LC-AC-01/04` can close.

**Scope notation:** `both` means the configured BYTE and opt-in UTF8 profiles
on supported hosts; `host` requires a named host adapter and platform evidence;
`source` requires source-encoding and source-location evidence. An open row has
no proved end-to-end Level C behavior yet, even if a parser or runtime helper
exists.

| ID | Reference obligation | Scope | Current state / remaining proof |
| --- | --- | --- | --- |
| LC-REF-001 | Invocation modes COMMAND, FUNCTION, SUBROUTINE | host | Open: `API_Start`-equivalent entry and lifecycle |
| LC-REF-002 | Initial source identity and line inventory | source | Open: diagnostics, PARSE SOURCE, SOURCELINE |
| LC-REF-003 | Initial environment and stream selection | host | Open: configuration adapter |
| LC-REF-004 | Invocation arguments and omitted positions | host | Bounded direct local ARG with Classic uppercase binding; omitted positions and API invocation proof open |
| LC-REF-005 | Caller-provided trap overrides | host | Open: trap/configuration lifecycle |
| LC-REF-006 | Completion classes: no value, result, condition, resource failure, unable to continue | host | Open: observable result/error contract |
| LC-REF-007 | Source characters, EOL/EOS and invalid-encoding `22.1` | source | Front end; configured source service and error proof open |
| LC-REF-008 | Configured extra blank characters and uppercase mapping | both | Runtime foundations; scanner-to-runtime parity open |
| LC-REF-009 | Configured compare, substring, length and range ordering | both | Runtime BIF foundations; complete integration open |
| LC-REF-010 | Binary-digit/character conversion hooks | both | Runtime foundations; exact configured conversion proof open |
| LC-REF-011 | Default BYTE profile, exact byte units and PAD/XRANGE | BYTE | Runtime foundations; whole-program integration open |
| LC-REF-012 | Opt-in UTF8 profile, valid text and codepoint units | UTF8 | Runtime foundations; whole-program integration open |
| LC-REF-013 | No implicit profile fallback, normalization or grapheme behavior | both | Open: positive/negative configuration proof |
| LC-REF-014 | DATATYPE extra letter/digit families, blanks and exponent limit | both | Runtime helper exists; complete caller/context proof open |
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
| LC-REF-025 | Minimum numeric, literal, symbol and string limits | both | Open: documented limits and over-limit messages |
| LC-REF-026 | Contextual instruction words, no general reserved words | source | Front end fixtures; full reference edge cases open |
| LC-REF-027 | Quoted/doubled strings and radix suffix boundaries | source | Front end fixtures and bounded empty quoted-string runtime proof; configured conversion/limit proof open |
| LC-REF-028 | Hex/binary literal grouping and left padding | source | Front end diagnostics; byte/profile equivalence open |
| LC-REF-029 | Numeric constants, exponent signs and period-start tokens | source | Front end partial; full Classic lexer pass open |
| LC-REF-030 | Comma continuation and inferred clause endings | source | Front end partial; whitespace edge cases open |
| LC-REF-031 | Nested comments and unterminated `6.*` messages | source | Front end diagnostics; reference edge cases open |
| LC-REF-032 | Alternative negators and inferred blank/nonblank concatenation | source | Front end plus expression slice, including adjacent function calls after blank concatenation; full lexical equivalence open |
| LC-REF-033 | Top-syntax VALUE insertion, assignment, label and keyword promotion | source | Front end fixtures; complete context matrix open |
| LC-REF-034 | Reserved .MN/.RESULT/.RC/.RS/.SIGL and `50.1` | both | Front end partial; runtime state and diagnostic proof open |
| LC-REF-035 | Function-name ending-period `51.1` | source | Front end diagnostic proof open |
| LC-REF-036 | Clause/label/null-clause program structure | source | Front end fixtures; runtime lifecycle open |
| LC-REF-037 | IF/THEN/ELSE branch selection and nearest ELSE | both | Bounded slice 7; wider instruction arms open |
| LC-REF-038 | SELECT/WHEN/OTHERWISE order and absent-match `7.3` | both | Bounded slice 11: ordered lazy branches, `34.2`, `7.3`; wider lifecycle proof open |
| LC-REF-039 | Simple DO grouping and empty body | both | Bounded slice 8; broader clause lifecycle open |
| LC-REF-040 | Counted, controlled, FOREVER, WHILE and UNTIL DO | both | Bounded literal/dynamic direct and combined counts, FOREVER conditions and scalar control with optional TO/FOR, captured dynamic start/TO/BY/FOR and controlled WHILE/UNTIL; wider count and controlled forms, state and errors open |
| LC-REF-041 | LEAVE/ITERATE nesting and named loop targets | both | Bounded childless transfer through generated blocks and named targets for supported controlled loops; wider loop lifecycle open |
| LC-REF-042 | END-name matching and group-label branch restrictions | source | Front end diagnostics; complete reference matrix open |
| LC-REF-043 | Expression terms, prefix, power and arithmetic precedence | both | Bounded expression slice; full numeric/error equivalence open |
| LC-REF-044 | Left-associative Classic power | both | Open: reference and optimization parity proof |
| LC-REF-045 | Normal/strict comparisons and configured character ordering | both | Bounded expression slice; numeric/configuration edges open |
| LC-REF-046 | Logical AND/OR/XOR and prefix negation with exact values | both | Bounded expression slice; contextual `34.5/34.6` open |
| LC-REF-047 | PARSE ARG/PULL/SOURCE/LINEIN/VERSION/VALUE/VAR and UPPER | host | Front end; source acquisition and execution open |
| LC-REF-048 | PARSE targets, placeholders, literal/variable patterns and positions | both | Front end; template execution and errors open |
| LC-REF-049 | Pool dropped, implicit, exposed and scalar/stem binding states | both | Bounded unset scalar read and direct scalar DROP; full lifecycle proof open |
| LC-REF-050 | Set/value/drop, stem-tail propagation and recursive exposure | both | Bounded direct scalar DROP/access/exposure slice; stem/compound and full alias proof open |
| LC-REF-051 | Compound-tail evaluation without NOVALUE, final lookup with NOVALUE | both | Bounded compound slice; error/alias proof open |
| LC-REF-052 | Constant symbols, reserved pool 0 and SIGL/.SIGL updates | both | Front end/runtime pieces; state and label-search proof open |
| LC-REF-053 | Numeric DIGITS/FORM/FUZZ and arithmetic error/condition model | both | RexxValue foundation; full context and condition proof open |
| LC-REF-054 | Exact logical values and contextual `34.*` identities | both | `IF` reports `34.1`; other contexts open |
| LC-REF-055 | Function/CALL omitted arguments and resolution order | host | Bounded local-call slice including adjacent calls after another operand; BIF/external order open |
| LC-REF-056 | Function RETURN value, `45.1` and RESULT/.RESULT lifecycle | both | Bounded local RETURN; complete call state open |
| LC-REF-057 | Program initialization and clause-boundary HALT/trap/TRACE work | host | Open: processor lifecycle |
| LC-REF-058 | ADDRESS selection/swap, transient command and WITH redirection | host | Front end; configured execution open |
| LC-REF-059 | CALL ON/OFF delayed condition handlers | host | Front end; condition delivery open |
| LC-REF-060 | DROP direct and parenthesized variable lists | both | Bounded direct scalar list; stem/compound/parenthesized and error proof open |
| LC-REF-061 | EXIT value, fallthrough and finalization | host | Empty EXIT slice; value/finalization open |
| LC-REF-062 | INTERPRET source, HALT, syntax and label restrictions | both | Front end; execution and `47.1` open |
| LC-REF-063 | NUMERIC DIGITS/FORM/FUZZ validation and defaults | both | Front end/runtime pieces; instruction integration open |
| LC-REF-064 | OPTIONS word handling and unknown-option policy | source | Level selection works; full processor option contract open |
| LC-REF-065 | PROCEDURE pool creation and EXPOSE aliases | both | Bounded scalar/stem EXPOSE; dynamic list and lifecycle open |
| LC-REF-066 | PUSH/QUEUE ordering and null-expression value | host | Front end; queue service open |
| LC-REF-067 | RETURN function/subroutine/outermost lifecycle | both | Bounded local RETURN; invocation modes open |
| LC-REF-068 | SAY default output and optional empty expression | host | Active whole-instruction review: expression output is bounded; a childless compiler change and Regina/opt/no-opt/linked case are tested but uncommitted. Embedded NUL is confirmed lost at the VM's NUL-terminated SAY callback boundary (`41 00 42 0a` Regina versus `41 0a` current VM); an additive length-aware host output API is proposed and awaits the required architecture approval. Configured output, error/lifecycle and complete expression integration remain open. |
| LC-REF-069 | SIGNAL branch/trap modes and loop-state clearing | host | Front end; runtime state and conditions open |
| LC-REF-070 | TRACE options, interactive mode, skip/inhibit and source/result tracing | host | Front end/runtime pieces; complete trace contract open |
| LC-REF-071 | SYNTAX, HALT, ERROR, FAILURE, NOTREADY, NOVALUE, LOSTDIGITS | host | Open: Classic condition state and delivery |
| LC-REF-072 | Message catalog, .MN, source/line traceback and trap handling | host | Open: Classic diagnostic/condition bridge |
| LC-REF-073 | Immediate SIGNAL ON, delayed CALL ON and HALT buffering | host | Open: clause/lifecycle behavior |
| LC-REF-074 | CONDITION BIF state fields | host | BIF name recognized; condition state integration open |
| LC-REF-075 | NOP as a statement with no visible effect | both | Bounded slice 9 in main/procedure and IF/DO bodies; source/TRACE and profile proof open |
