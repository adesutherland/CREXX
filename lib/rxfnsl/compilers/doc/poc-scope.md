# C/G compiling PoC scope

**Approved profile; both source-to-IR frontends implemented in STEP03.**
This defines the first slice of both compilers. STEP04 supplies executable RXAS and its [backend qualification](../evidence/step04-20261015/README.md);
STEP05 supplies shared optimisation; STEP06 consolidates the
[coverage and evidence](poc-coverage.md) for the complete bounded pipeline.
The full replacement obligations remain in [PLAN.md](../PLAN.md). Unsupported
source receives a located profile diagnostic; neither compiler falls back to
the old frontend or rewrites its input as B source.

## Shared first-language profile

The profile uses existing source syntax. It introduces no new C or G syntax.
G files start with `options levelg`; C files use `options levelc`. The STEP04
commands are `llrxc-g` and `llrxc-c`, with distinct libraries and build targets.

| ID | Included in Phase A | Boundary and required examples |
|---|---|---|
| POC-01 | UTF-8 source; LF/CRLF; semicolon and line clauses; nested `/* ... */` comments; quoted strings with doubled quotes | ASCII simple names, digits and underscore initially, case-insensitive lookup with spelling retained. Unicode string content and diagnostic spans are supported. Full language-specific identifier, continuation, other comment-option and literal repertoires follow in Phase B. |
| POC-02 | Decimal integer and ordinary decimal-point numeric literals; G explicit decimal suffix; parentheses; scalar reads and assignments | Preserve original spelling. G admits `.int`, `.boolean`, `.string`, `.float`, `.decimal`; typed declarations initialise their defined defaults. C uses RexxValue and dynamic pool reads/writes, including empty RHS and unset names. No stems/arrays/reference source forms yet. |
| POC-03 | Unary `+`, `-`, logical `\`; binary `+ - * /`, `= <> < <= > >= ==`, explicit `||`, `& | &&` | G uses its default common arithmetic profile and explicit scalar `as` casts. C uses Classic precedence, numeric conversion and comparison. Blank/abuttal concatenation is included for C; its whitespace must survive lexing. Power, remainder and extra operator spellings follow in Phase B. |
| POC-04 | Assignment, SAY, NOP, simple DO/END groups, nested IF/THEN/ELSE | Empty optional ELSE and nearest-IF association; Classic keyword-as-name assignment. SAY operands retain each language's conversion and spacing rules. |
| POC-05 | Pre-test `DO WHILE`; unnamed LEAVE and ITERATE | Zero, one and several iterations; nested loops; early return from a loop. Post-test, counted, named and combined loop forms follow in Phase B. |
| POC-06 | Named local routines, CALL, expression calls, RETURN and top-level EXIT | Recursive calls and argument evaluation order are included. G has explicit scalar parameter/result types and `.void`. C routine entries are label followed immediately by PROCEDURE; shared-pool label-only routines/fallthrough are diagnosed as outside this profile. Main must terminate before such routine entries. |
| POC-07 | C ARG with one simple target per comma-separated argument position, omitted actual arguments, `ARG()`/`ARG(n,'E')`, CALL RESULT handling | Use the existing activation and Classic ARG uppercase/parse semantics. RETURN without a value clears the CALL result according to the current contract. G CALL discards a returned value. STEP07A now adds scalar G optional defaults and presence; varargs and procedure EXPOSE remain later. |
| POC-08 | C NUMERIC DIGITS/FUZZ and FORM SCIENTIFIC/ENGINEERING, including reset and expression-valued DIGITS/FUZZ | Evaluate operands once; preserve inherited/restored call context and runtime errors. G uses existing default context plus contextual decimal literals; broader G NUMERIC/options profiles follow in Phase B. |
| POC-09 | Library call/import path | G `import rxfnsb` and LENGTH; C LENGTH, ARG and DIGITS direct BIF routes through existing runtime contracts. Read the required callable/type/member/argument/autoload metadata in cREXX. C's runtime may need object/member contracts internally even though authored classes are outside the slice. |
| POC-10 | Clean-input gate, syntax/type/profile diagnostics, source origins, runtime error locations | Compilation reports multiple syntax errors where existing recovery supports it; repaired input never produces a successful executable. Plain syntax failure and valid-but-unsupported source have distinct codes. |

Both profiles retain their normal default trace/condition policy. Full authored
TRACE settings and condition-handler installation are Phase B work. The IR must
already model observable failures, source events and calls conservatively, so
that adding those constructs does not require changing the meaning of an
existing operation. No debugger/TRACE parity claim is made from the first slice.

Recognised unadmitted instructions and semantic forms report
`LL_COMPILER_PROFILE` with their source location. Syntax errors use
`LL_COMPILER_SYNTAX`; source preparation retains `LL_UTF8`/`LL_COMMENT`.
See the [implemented service and diagnostics](frontends.md). Arbitrary source
outside the grammar can still receive a syntax error. These grammars declare no
recovery policy and stop at the first syntax error; the service also rejects
recovered trees. Full-language diagnostic/recovery parity remains a later gate.

## Runnable source examples

The examples run with the existing tools and now also produce source trees and
verified IR through the new frontends. The [check receipt](../evidence/step01-20261015/README.md)
records exact binary/source identities and the limited verification boundary.

| Pair | What it establishes | Expected stdout |
|---|---|---|
| [pipeline C](../examples/step01/pipeline-c.crexx), [pipeline G](../examples/step01/pipeline-g.crexx) | Arithmetic, IF, local function/ARG/RETURN, loop, output and an actual imported LENGTH call | Both: `10`, `0`, `3`, one per line. |
| [evaluation C](../examples/step01/evaluation-c.crexx), [evaluation G](../examples/step01/evaluation-g.crexx) | Side-effecting right operand of AND/OR | C: `evaluated`, `0`, `evaluated`, `1`. G: `0`, `1`. |
| [numbers C](../examples/step01/numbers-c.crexx), [numbers G](../examples/step01/numbers-g.crexx) | Literal identity, decimal arithmetic, numeric context, unset/default variables | C: `0`, `0.3`, `0.333`, `0.333333333`, `UNASSIGNED`. G: `1`, `0.3`, `0`. |

These six controls passed with compiler/assembler optimisation disabled on
both VMs. The [generated STEP03 dumps](../examples/step03/README.md) establish source
admission and mappings, not emitted execution. Compile-time constant
folding in the new pipeline must preserve these outcomes and the timing of
observable effects/errors.

## Acceptance corpus and retained evidence

The [coverage matrix](poc-coverage.md) connects the six controls, existing
regressions and STEP06 additions to each POC row, including:

1. Exact numeric literal/cast round trips, int64 overflow, float signed zero,
   decimal precision and changes of numeric context around a call.
2. Calls with missing/empty arguments, ARG presence, recursion, no-value RETURN,
   unchanged caller variables after private PROCEDURE, and G CALL discard.
3. Every IF arm, zero-iteration loops, LEAVE/ITERATE, early return and nested
   short-circuiting. G default declarations must not gain spurious
   definite-assignment errors; C unset reads must not become typed-local errors.
4. Runtime errors in taken and untaken paths. An optimiser must not diagnose an
   untaken `1 / 0` merely because its operands are constants.
5. Invalid/missing THEN, END, operand or argument; non-Boolean Classic condition;
   unsupported constructs; malformed UTF-8; multibyte/CRLF origin preservation.
6. Missing/wrong imports, forged signatures, stale handles, invalid graph edges,
   unknown operation contracts and budget exhaustion, with no partial output.

Existing source controls to reuse include the `levelc_arg_frame_lifecycle`,
`levelc_call_resolution_result`, `levelc_expression_comparisons`,
`levelc_numeric_validation`, `levelc_if_nested_missing_then` and
`inline_test_short_circuit_expr` fixtures under `compiler/tests/rexx_src/`.
Some exceed Phase A; classify their individual cases rather than claiming the
whole fixture passes through the PoC.

## Migration inventory beyond the PoC

| Area | Existing contract owner | Later obligation |
|---|---|---|
| Proper B and self-hosting | [B implementation contract](../../../../docs/ai-context/CREXX_LEVELB_AUTHORING.md), [bootstrap architecture](architecture.md#bootstrap-native-seed-to-self-hosted-b-and-l) | STEP07B is deferred until after the C-first milestone, retaining full B language/ABI and compiler/library closure criteria. Native B seeds proper B through L; proper B rebuilds B libraries, L and itself with stage comparison and independent regressions. |
| Complete Classic instructions/BIFs | [Classic worklist](../../../../docs/planning/release-1/levelc-compatibility-worklist.md), [working architecture](../../../../compiler/docs/levelc_working_architecture.md) | Active C-first priority: all agreed instructions and catalogued BIFs, stems/exposure, full ARG/PARSE, label dispatch, SIGNAL, conditions, TRACE, ADDRESS and streams. Review proceeds instruction by instruction. STEP08 INTERPRET and its mechanism review are deferred until later, retaining existing LC acceptance and eventual C-completion scope. |
| G language | [Language reference](../../../../docs/books/crexx_language_reference/statements.md), [types](../../../../docs/books/crexx_language_reference/data_types.md), [classes](../../../../docs/books/crexx_language_reference/classes_and_interfaces.md) | Classes/interfaces, references, arrays, complete numerics/options, initialisers, namespace exposure, all routine forms and extensions/tasks. |
| Module ABI | [Metadata conformance](../../../../compiler/docs/metadata_conformance.md) | Full imports, exports, providers/autoload, layouts, optional defaults, source and supported inline templates. Phase A emits no optional inline body and refuses unsupported required contracts explicitly. |
| Tools | [Compiler architecture](../../../../docs/ai-context/CREXX_ARCHITECTURE.md) | Wrappers, project builds, diagnostics/localisation, editors, source imports and host services. Existing commands keep their routing until replacement. |
| Delivery | [Repository instructions](../../../../AGENTS.md), [single-threaded port](../../../../ports/single-threaded/README.md) | Maintained OS/compiler/VM matrix, packaging and constrained-host applicability, bootstrap and migration rollback. |

STEP07 turns this category inventory into a feature-by-feature disposition
before claiming replacement coverage. Newly added upstream features must enter
that inventory; the PoC scope is not a frozen definition of the full language.

## STEP07A admission beyond the original PoC

The [approved call contract](STEP07A-CONTRACTS.md) adds scalar literal/primitive
optional formals, original-presence queries, positional holes and declaration-only
G namespace providers with checked qualified imports. C keeps its existing ARG
semantics and runtime-call allowlist with stronger signature checks. Arbitrary
default expressions, classes/references/arrays, provider initialisers, source
imports, general C external loading and INTERPRET remain open. This extension
does not close the full migration categories above.
