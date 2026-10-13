# Practical consumers and parser requirements

Reviewed 10 October 2026. This is source inspection, not qualification of these
language implementations. The complete pinned revisions and inspected file
inventory are in [mike-sources.json](evidence/mike-sources.json).

Mike's 10 October email, “add ons - based on CREXX”,
links Smalltalk, xBase, Pascal, PL/M, Lisp, BASIC, LOGO and SNOBOL. It also refers
to the previously supplied Prolog and separately links MQ, followed by his
testing caveat. The email is a useful discovery list; it does not establish
language completeness or release qualification. MQ is outside the parser PoCs.

## Observed front ends

| Consumer and pinned source | Observed parsing approach | Diagnostics observed | Level L opportunity and constraint |
|---|---|---|---|
| [Smalltalk `3a1f2f064902`](https://github.com/mikebeer/smalltalk/tree/3a1f2f064902069b3f910e2a45ec8a356c5148fd/src) | `b_lex.crexx` scans tokens and also owns parsing helpers; `lex_mark`, `lex_reset`, `lex_drop` save/restore lexer state. `b2_top.crexx` uses these during declaration parsing. Resolver/evaluator are separate source files. | `perr` reports source name and `line_of(tp)`, preserving the first error. | Shared positioned tokens could simplify source diagnostics. Smalltalk unary/binary/keyword messages and selector arity need their own grammar/semantic adapter; a universal expression grammar is insufficient. |
| [xBase `b4fe9f185078`](https://github.com/mikebeer/xbase/tree/b4fe9f185078acf6ae842864e45365987f6960b1/src) | `20_lex.crexx` handles chunks, logical lines, directives/includes and block state. `30_pexpr`, `31_pstmt`, `32_pcmd` separate expression/statement/command parsing from evaluation. `isword` checks spelling on identifier tokens. | `perr` records a message and `tl[tp]`, then moves to the token-stream end. | A lexer can preserve existing soft-keyword decisions. Include/macro origins, logical-line boundaries and case policy must survive generation. |
| [Pascal `134b1bf7e9c3`](https://github.com/mikebeer/pascal/blob/134b1bf7e9c35bbbeaa4dad8f2a88d6ed0e23a2a/pascal.crexx) | `tokenize` fills kind/text/line arrays; `isid`/`issym` and recursive compile routines consume them. Identifiers and keyword spellings use the same `I` token category. Compiler emits RXAS and has a separate runtime. | `error`/`errorat` report file and line and exit; expectation helpers include expected/found spelling. | A good first generated lexer consumer: conventional regular tokens, existing token-first compiler. Keep symbol/type checking and RXAS emission; add columns/ranges before replacing parsing. |
| [PL/M `17edaec2b83f`](https://github.com/mikebeer/PLM/blob/17edaec2b83f2882b2ceb60105a7cc9ff43a1e85/plm.crexx) | `lexinto` creates positioned tokens; `compilemodule` and expression tiers consume them and emit RXAS. Source includes textual macro facilities. | `error` uses current token line; `errorat` accepts an explicit source line; both exit. | Similar lexer reuse opportunity to Pascal, with a mandatory source-origin adapter for macro expansion. PL/M byte/address machine semantics belong downstream. |
| [Lisp `477c3d9dadde`](https://github.com/mikebeer/lisp/blob/477c3d9daddeec5a1b829b21f3068112bf38d2a3/lisp.crexx) | `readobj` dispatches directly from source; `readlist`, `parseatom`, radix/quote reader paths build Lisp objects. It has an evaluator and condition machinery. | Reader errors include unmatched `)` and end of input after quote; `lerror` retains the first error kind/message. | Reuse lexical atoms and source positions. A list reader is already simple; a Pika port is not automatically a better reader. Reader macros and evaluator semantics remain separate. |
| [BASIC `e6f75ec1418c`](https://github.com/mikebeer/basic/blob/e6f75ec1418c8056525b03bc45dc29ad529aeafc/basic.crexx) | `tokenize_line` has NUM/STR/ID/KW/OP/EOL/END/TYP categories, line starts, statement/header state and suffix/typing handling. A linking/preparation phase resolves control structures before token-array execution. | `link_error`, `rt_error`, `report_error` and `handle_error` distinguish preparation/runtime errors. | This already demonstrates the useful lex-first strategy. Preserve EOL, numeric dialect rules and contextual header decisions; an unconditional keyword lexer would lose behavior. |
| [LOGO `43dfe8dd055d`](https://github.com/mikebeer/LOGO/blob/43dfe8dd055d9378539796275bcd3ed513173e8d/logo.crexx) | `tokenize` plus `expr`, `addexpr`, `mulexpr`; list/word/variable notation and procedure execution share the interpreter. | `lerror` emits a message, writes the SVG state and exits. | Good compact lexer/demo grammar. Prefix procedures and their argument consumption require semantic context; lists and quoted words must retain spelling and boundaries. |
| [SNOBOL `0fb6482f2cf2`](https://github.com/mikebeer/snobol/blob/0fb6482f2cf2b7d76b71c13d7b969878ff25a7f3/src/b_parse.crexx) | `parse_statement`/`parse_body` organize statement fields. `parse_expr(minp)` uses precedence and the result of `skipb()` to recognize blank concatenation. Runtime pattern backtracking is a different subsystem. | Parser records `perr_msg`; syntax errors include line number. | Preserve whitespace and statement/continuation fields. Deleting trivia changes this language. A DFA lexer does not replace SNOBOL's runtime pattern engine. |
| [Prolog in this checkout](../../examples/prolog-in-crexx/prolog.crexx) at source base `91313bcc5` | `PTokenize` prepares a clause/query token array, with tags for variables, quoted atoms, strings and terminators; `ParseClauseTerm` consumes it using the engine's term/operator rules. | Current manual and engine distinguish query/consult failures; APIs reuse parser and resolution state. | Preserve quoted versus unquoted atoms, variable identity, symbolic-atom runs and clause/query termination. Unification, trailing and resolution do not belong in a generated lexer. |

The public `mikebeer/prolog` repository was empty when checked. The actual
owning source is the checked-in example above. Historical notes in
`PORT_NOTES.md` discuss older toolchain limitations; this audit does not turn
those historical claims into current compiler facts.

## RexxScript as the first practical grammar consumer

**Later maintainer direction, 10 October:** RexxScript is the first product
target, modernized as a reduced Level G rules/configuration language, initially
without classes. The source observations below remain valid; the earlier
compatibility-preserving experiment is not the successor's language contract.
See PLAN.md RS-D01–12 for the proposed minimum backends and remaining decisions.

The current owning implementation is
[RexxScriptEvaluator.crexx](../../rexxscript/RexxScriptEvaluator.crexx), with its
[developer guide](../../rexxscript/doc/developer-guide.md). The older concept
document is explicitly historical and is not the product contract.

The evaluator owns per-instance statement, block, expression and sandbox state.
It splits statements while respecting strings, scans labels and executes with
a program counter. Its expression routines operate on text and keep `expr_pos`.
`_set_error` records a code, message, statement number and column. Its intrinsic
dispatcher uses the shared Classic BIF context and allow-list; the public facade
wraps that evaluator. Parsing is only one part of its runtime behavior.

A useful first integration experiment is to replace repeated expression text
scanning with positioned tokens while keeping all evaluator operations, sandbox
policy, variable-pool behavior and host adapters. Retain clause/line maps,
omitted arguments, string boundaries and concatenation evidence. Compare actual
RexxScript outputs, error codes/locations, host write-back and resource limits;
the arithmetic/assignment PoCs here do not establish RexxScript compatibility.
The procedural last-result facade is transitional; a production parser context
should follow RexxScript's instance ownership instead of the PoCs' module state.

## Shared toolkit shape supported by the audit

The common abstraction is a source buffer plus token kinds, raw spans, source
origins and optional trivia/mode/soft-keyword metadata. It can serve both a
handwritten Pratt/recursive-descent parser and a generated recognizer. Token
generation should not force all nine languages into one grammar or one keyword
policy.

Start with Pascal/PL/M lexical adapters and RexxScript expression diagnostics.
Keep an existing handwritten parser as a correctness control. Lisp and LOGO
provide smaller demonstration languages; SNOBOL and contextual BASIC/xBase
stress trivia and lexical modes. These are prioritized experiments, not approved
refactors of Mike's repositories.

## Frontend-stage and migration inventory (research branch)

Owning-source inspection uses the selected branch's `compiler/docs/parsing_pipeline_anatomy.md`,
`validation_logic_map.md`, `rxcp_ast.h`, `rxcp_source_tree.*`, `rxcpmain.c`,
`rxcpbpar.c`, `rxcpcpar.c` and the corresponding scanners/Lemon grammars.
The frozen executable is identified by hash separately; its prior dirty build
cannot be claimed as an exact build of this branch. Concurrent develop extraction
is not modified or treated as a stable public service ABI.

| Responsibility | Owning source / evidence | Appropriate general mechanism |
|---|---|---|
| Static level/options preparse and CLI defaults | `rxcpopar.c`, `rxcposcn.re`, `rxcpopgr.y`; dispatch in `rxcpmain.c` | Separate header grammar/policy, then fresh language ingress. Dynamic OPTIONS is not a partial static declaration. |
| B/G/L lexical classes, source spelling, nested comments | `rxcpbscn.re` | Regular scanner plus declared counter/stack stages; retain original trivia, do not claim nested comments are regular. |
| B/G comma/EOL normalization, repeated EOC and final shutdown | `rxcpbpar.c` | Finite-state transducer for the suitable subset; the current native stage omits final-EOC synthesis and full grouping context. |
| Classic neutral words, assignment/label precedence, IF/WHEN/THEN and PARSE modes | `rxcpcpar.c` and `rxcpcscn.re`; `LEVELC_TAIL_*` and `LEVELC_PARSE_*` state transitions | Explicit contextual classifier with mode identity and nesting obligations; a global hard-keyword lexer is not equivalent. |
| Raw syntax and reduction-built nodes | `rxcpbgmr.y`, `rxcpcgmr.y` | Declared PEG/CFG profile plus pure selected-tree construction. Current grammar action order is observable where it mutates state. |
| Procedure/body grouping, implicit main, declarations, loop defaults | `rxcp_val_check.c`, `rxcp_val_orch.c` | Declared grouping/rewrite for structural subsets; type-dependent return/default decisions stay separate. |
| Source ranges, abuttal refinement and authored snapshot | `source_location_walker`, syntax validation, `rxcp_source_tree.c` | Pure source propagation, explicit trivia tests and owned immutable source tree. |
| Imports, exit fragments, symbols and types | `rxcp_val_orch.c`, `rxcp_val_sym.c`, `rxcp_val_plugin.c`, exit bridge | Attribute/dependency scheduling where proved; host services with explicit invalidation/effect contracts elsewhere. A loop cap is not a convergence proof. |
| Later optimization, register/storage and output fragments | `rxcp_ast.h` and emitter/flow passes | Consumer semantic IR/facts; not fields every general parser tree must carry. |

The existing mutable `ASTNode` combines child/sibling/parent structure, source
ownership/provenance, type and target-type arrays, symbols/scopes, control
associations, register facts, diagnostics and emission fragments. A shared source
tree cannot honestly replace all of this merely by having children and spans.
The current compiler already distinguishes immutable `SourceNode` ownership from
its mutable work AST, which is a useful migration boundary.

Migration sequence to test/approve after this research:

1. Shared immutable source/token/tree and structured diagnostic ownership, with
   adapters to current consumers. Qualify byte/scalar coordinates, comments,
   recovered/synthetic nodes and independent context lifetime.
2. Shared builder/visitor/grouping contracts; compare selected structural passes
   against current source and work trees. Preserve mutation invalidation and
   exact/inherited/synthetic/composite provenance explicitly.
3. Consumer sidecars for scope, symbol, type, control and ownership facts. Move
   proved attribute dependencies and monotone finite-lattice analyses first;
   keep imports and arbitrary compiler exits as versioned/effectful services.
4. Move mutable semantic rewrites and proof consumers after their preconditions,
   postconditions and invalidation rules have executable correspondence.
   Register allocation and emitter fragment state may belong to a later IR.
5. Retire legacy adapters only after source diagnostics, lifecycle, mutation,
   optimization and all B/G/C runtime consumers have their own migration gates.

The native grouping experiment is the beginning of step 2, not completion of
this route. Its compiler control uses `-x -d1 -dp -n` so certified exits are
explicitly disabled; the sample does not require them. The three frozen B/G raw
and fixup snapshots cover two explicit procedures and implicit main. The adapter
compares selected node/value/order observations, omitting OPTIONS, synthetic empty
ARGS, synthetic CLASS and later type facts. Source anchors are independently
recovered within this closed corpus because the raw debug snapshot reports
`(-1,-1)`; it is not a general serialized AST or a full span-equivalence proof.
