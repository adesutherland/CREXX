# Raw Language And Preprocessor Capability Catalogue

Stage 2 discovery data. “Observed state” records repository evidence and is not a proposed Release 1 classification.

## Typed compiler-front-end capabilities

| ID | Capability | Primary evidence | Observed state |
|---|---|---|---|
| `SYN-TYPED-OPTIONS` | Leading `OPTIONS` instruction | `compiler/rxcposcn.re`; `docs/books/crexx_language_reference/options.md` | Parsed by the options pre-scan. |
| `SYN-TYPED-LEVELB-OPTION` | `options levelb` selection | `compiler/rxcposcn.re`; `compiler/rxcpbgmr.y` | Selects the typed front end. |
| `SYN-TYPED-LEVELG-OPTION` | `options levelg` selection | `compiler/rxcposcn.re`; `lib/rxfnsg/rexx/llm.crexx` | Accepted for the typed derived surface. |
| `SYN-TYPED-LEVELL-OPTION` | `options levell` selection | `compiler/rxcposcn.re`; `lib/rxfnsl/rexx/tinyexpr.crexx` | Accepted for the language-engineering derived surface. |
| `SYN-TYPED-NUMERIC-COMMON` | `numeric_common` parser mode | `compiler/rxcposcn.re`; `docs/books/crexx_language_reference/options.md` | Typed default. |
| `SYN-TYPED-NUMERIC-CLASSIC` | `numeric_classic` parser mode | `compiler/rxcposcn.re`; `docs/books/crexx_language_reference/options.md` | Accepted file-level parsing mode. |
| `SYN-TYPED-COMMENT-OPTIONS` | Hash, slash, and dash single-line comment options | `compiler/rxcposcn.re`; `docs/books/crexx_language_reference/options.md` | Parsed file options. |
| `SYN-TYPED-BLOCK-COMMENTS` | `/* ... */` block comments | `compiler/rxcpbscn.re`; language reference | Scanner capability. |
| `SYN-TYPED-FLOAT-OPTIONS` | Binary/decimal float source options | `compiler/rxcposcn.re`; language reference | Parsed file options. |
| `SYN-TYPED-NAMESPACE` | Namespace declaration | `compiler/rxcpbgmr.y`; `docs/books/crexx_language_reference/namespace.md` | Parsed, validated, emitted. |
| `SYN-TYPED-IMPORT` | Namespace/module import | `compiler/rxcpbgmr.y`; `compiler/rxcpfunc.c` | Parsed and resolved through RXBIN/source imports. |
| `SYN-TYPED-EXPOSE-NAMESPACE` | Namespace-exposed symbols | `compiler/rxcpbgmr.y`; library sources | Used by library export metadata. |
| `SYN-TYPED-QUALIFIED-NAME` | `namespace..symbol` qualified reference | compiler validation; authoring guide | Implemented canonical spelling. |
| `SYN-TYPED-QUALIFIED-ALIAS` | `namespace::symbol` compatibility alias | compiler scanner/parser; authoring guide | Accepted compatibility spelling. |
| `SYN-TYPED-TYPE-INT` | `.int` signed integer type | compiler type tables; data-types reference | Compiled runtime type. |
| `SYN-TYPED-TYPE-BOOLEAN` | `.boolean` type | compiler type tables; data-types reference | Compiled runtime type. |
| `SYN-TYPED-TYPE-STRING` | `.string` UTF text type | compiler type tables; data-types reference | Compiled runtime type. |
| `SYN-TYPED-TYPE-BINARY` | `.binary` arbitrary byte type | compiler type tables; binary-memory reference | Compiled runtime type. |
| `SYN-TYPED-TYPE-FLOAT` | `.float` type | compiler type tables; data-types reference | Compiled runtime type. |
| `SYN-TYPED-TYPE-DECIMAL` | `.decimal` type | compiler type tables; numeric reference | Compiled through decimal VM plugin operations. |
| `SYN-TYPED-TYPE-OBJECT` | `.object` root object type | compiler type tables; classes reference | Compiled runtime type. |
| `SYN-TYPED-TYPE-VOID` | `.void` procedure result type | compiler type tables; procedures reference | Checked result type. |
| `SYN-TYPED-TYPE-UNKNOWN` | `.unknown` placeholder/dynamic metadata type | compiler type tables; library sources | Used in class/plugin metadata. |
| `SYN-TYPED-DYNAMIC-ARRAY` | Dynamic typed arrays such as `.string[]` | `compiler/rxcpbgmr.y`; data-structures reference | Parsed, validated, emitted. |
| `SYN-TYPED-FIXED-ARRAY` | Fixed-size typed arrays | grammar/type validation; data-structures reference | Compiler surface present. |
| `SYN-TYPED-MULTIDIM-ARRAY` | Multi-dimensional typed arrays | grammar/type validation; data-structures reference | Compiler surface present with narrower statement support. |
| `SYN-TYPED-CLASS` | Class declaration | `compiler/rxcpbgmr.y`; classes reference | Parsed, linked, runtime-dispatched. |
| `SYN-TYPED-INTERFACE` | Interface declaration | `compiler/rxcpbgmr.y`; classes reference | Parsed, linked, runtime-dispatched. |
| `SYN-TYPED-IMPLEMENTS` | Class/interface implementation contract | grammar; class import metadata | Parsed and validated. |
| `SYN-TYPED-FACTORY` | Typed factory declaration/call | grammar; class library | Parsed, linked, runtime-dispatched. |
| `SYN-TYPED-METHOD` | Typed method declaration/call | grammar; class library | Parsed, linked, runtime-dispatched. |
| `SYN-TYPED-ATTRIBUTE` | Typed class attributes | grammar; class library | Parsed and emitted into runtime class metadata. |
| `SYN-TYPED-SELF` | Current receiver access | grammar; classes reference | Method receiver semantics implemented. |
| `SYN-TYPED-REGISTER-VIEW` | `with register.N...` system attribute views | grammar; Level B authoring guide | Runtime/VM integration syntax. |
| `SYN-TYPED-PROCEDURE` | Typed procedure declaration | `compiler/rxcpbgmr.y`; procedures reference | Parsed, validated, emitted. |
| `SYN-TYPED-MAIN` | Explicit or implicit top-level main procedure | grammar; architecture guide | Compiler entry-point surface. |
| `SYN-TYPED-ARG` | Typed `ARG` declarations | grammar; arguments reference | Checked procedure/method arguments. |
| `SYN-TYPED-ARG-OPTIONAL` | Optional `?name` arguments | grammar; arguments reference | Signature metadata and validation support. |
| `SYN-TYPED-ARG-VARIADIC` | Variadic `...` arguments | grammar; arguments reference | Signature and runtime support. |
| `SYN-TYPED-ARG-EXPOSE` | `arg expose` caller-owned storage | grammar; authoring guide | Reference-like mutation surface. |
| `SYN-TYPED-ARG-ARRAY` | `arg[]` / `arg.0` pseudo-array | grammar; arguments reference | Runtime argument access. |
| `SYN-TYPED-ARG-COMPAT` | `arg()` compatibility operator | grammar; arguments reference | Compatibility access surface. |
| `SYN-TYPED-PROCEDURE-EXPOSE` | Procedure-local `expose` binding | grammar; authoring guide | Module-state binding surface. |
| `SYN-TYPED-CONSTANT` | Named compile-time constant | grammar; literals reference | Folded and immutable within callable scope. |
| `SYN-TYPED-ASSIGNMENT` | Typed assignment and declaration-by-type expression | grammar; variables reference | Validated conversion/assignment. |
| `SYN-TYPED-CALL` | Statement-form procedure or method call | grammar; statements reference | Implemented. |
| `SYN-TYPED-RETURN` | Typed return | grammar; statements reference | Result validation and emission. |
| `SYN-TYPED-EXIT` | Program exit with optional value | grammar; statements reference | Implemented. |
| `SYN-TYPED-IF` | `IF/THEN/ELSE` | grammar; statements reference | Implemented. |
| `SYN-TYPED-DO` | `DO/END` grouping and repetition | grammar; statements reference | Implemented. |
| `SYN-TYPED-LOOP` | `LOOP/END` repetition | grammar; statements reference | Implemented. |
| `SYN-TYPED-ITERATE` | Loop iteration control | grammar; statements reference | Implemented. |
| `SYN-TYPED-LEAVE` | Loop/block leave | grammar; statements reference | Implemented. |
| `SYN-TYPED-BLOCK-EXPRESSION` | `DO ... LEAVE WITH value ... END` expression | grammar; statements reference | Implemented typed block-expression surface. |
| `SYN-TYPED-SELECT` | `SELECT/WHEN/OTHERWISE` | grammar; statements reference | Implemented; eligible static cases can be optimised. |
| `SYN-TYPED-MATCH` | Typed match instruction/surface | grammar; compiler exits keyword table | Parser surface present. |
| `SYN-TYPED-NOP` | No-operation instruction | grammar; statements reference | Implemented. |
| `SYN-TYPED-SAY` | Standard output instruction | grammar; runtime | Implemented. |
| `SYN-TYPED-PULL` | Queue/input pull instruction | grammar; runtime | Implemented. |
| `SYN-TYPED-PUSH` | Queue push instruction | grammar; runtime | Implemented. |
| `SYN-TYPED-QUEUE` | Queue append instruction | grammar; runtime | Implemented. |
| `SYN-TYPED-DROP` | Variable/array drop instruction | grammar; runtime | Implemented typed semantics. |
| `SYN-TYPED-NUMERIC` | Procedure-scoped `NUMERIC` instruction | grammar; numeric reference | Implemented. |
| `SYN-TYPED-ADDRESS` | ADDRESS command/function/environment syntax | grammar; certified Address exit; VM protocol | Compiler-exit and runtime environment surface. |
| `SYN-TYPED-PARSE` | PARSE instruction | grammar; certified Parse exit | Compiler-exit-backed surface. |
| `SYN-TYPED-SIGNAL` | SIGNAL and block-scoped handlers | grammar; certified Signal exit | Compiler-exit/runtime signal surface. |
| `SYN-TYPED-TRACE` | TRACE instruction | grammar; certified Trace exit | Compiler-exit/runtime metadata surface. |
| `SYN-TYPED-ARRAY-APPEND` | `append array with value` | grammar; statements reference | Raw dynamic array statement. |
| `SYN-TYPED-ARRAY-INSERT` | `insert array with value at index` | grammar; statements reference | Raw dynamic array statement. |
| `SYN-TYPED-ARRAY-REMOVE` | `remove array at ...` | grammar; statements reference | Raw dynamic array statement. |
| `SYN-TYPED-ARRAY-CLEAR` | `clear array` | grammar; statements reference | Raw dynamic array statement. |
| `SYN-TYPED-ARITHMETIC` | Typed arithmetic operators | grammar; operators reference | Type-directed validation/emission. |
| `SYN-TYPED-COMPARISON` | Loose and string comparison operators | grammar; operators reference | Type-directed validation/emission. |
| `SYN-TYPED-CONCAT` | Blank, abuttal, and `||` concatenation | grammar; operators reference | String/binary-aware emission. |
| `SYN-TYPED-LOGICAL` | Boolean/logical operators | grammar; operators reference | Implemented. |
| `SYN-TYPED-NAMED-INTEGER-OPS` | `<and>`, `<or>`, `<xor>`, shifts, masks, integer division/remainder | scanner/grammar; operators reference | System-programmer operator surface. |
| `SYN-TYPED-AS` | Explicit `as` conversion | grammar; type validation | Checked conversion surface. |
| `SYN-TYPED-IS-TYPEOF` | `is` and `typeof` type tests | grammar; type validation | Object/type inspection surface. |
| `SYN-TYPED-REFERENCE` | `reference target` | grammar; operators reference | Weak reference creation. |
| `SYN-TYPED-DEREFERENCE` | `dereference ref` | grammar; operators reference | Scope-bound alias surface. |
| `SYN-TYPED-SNAPSHOT` | `snapshot ref` | grammar; operators reference | Explicit deep-copy surface. |
| `SYN-TYPED-BINARY-AT` | Binary `<at..type>` read/write views | grammar; binary-memory reference | Direct packed-memory lowering. |
| `SYN-TYPED-INLINE-RXAS` | Inline assembler and imported RXAS support | grammar/emitter; assembler docs | System/tooling integration surface. |

## Classic front-end capabilities

| ID | Capability | Primary evidence | Observed state |
|---|---|---|---|
| `SYN-CLASSIC-OPTIONS` | Classic `OPTIONS` clauses and Level C selection | `compiler/rxcposcn.re`; `compiler/rxcpcgmr.y` | Parsed by the dedicated front end. |
| `SYN-CLASSIC-CLAUSES` | Semicolon/EOL clause model | `compiler/rxcpcscn.re`; `compiler/rxcpcgmr.y` | Parser/highlighter support. |
| `SYN-CLASSIC-CONTEXTUAL-KEYWORDS` | Instruction words usable as symbols outside instruction context | Level C scanner/glue; working architecture | Dedicated token-adapter/parser behaviour. |
| `SYN-CLASSIC-LABELS` | Labels and local routine names | Level C grammar | Parsed and source-mapped. |
| `SYN-CLASSIC-SYMBOLS` | Simple, compound, and constant symbols | Level C scanner/grammar | Parsed and validated. |
| `SYN-CLASSIC-STEMS` | Classic stems and compound-variable tails | Level C grammar/lowering | Parser support; partial runtime lowering through the variable pool. |
| `SYN-CLASSIC-STRINGS` | Quoted, doubled-quote, hex, and binary strings | Level C scanner | Parsed/highlighted. |
| `SYN-CLASSIC-ASSIGNMENT` | Simple and compound assignment | Level C grammar/lowering | Parser support; scalar and simple compound lowering slices exist. |
| `SYN-CLASSIC-COMMAND` | Implicit command clause | Level C grammar | Parsed; execution is outside the broad supported subset. |
| `SYN-CLASSIC-ADDRESS` | Classic ADDRESS forms | Level C grammar | Parsed/highlighted; broad runtime lowering incomplete. |
| `SYN-CLASSIC-ARG` | Classic ARG instruction | Level C grammar | Parsed/highlighted. |
| `SYN-CLASSIC-CALL` | CALL routine and CALL ON/OFF forms | Level C grammar | Parsed; narrow direct local-call lowering exists. |
| `SYN-CLASSIC-DO` | Simple, counted, conditional, and forever DO | Level C grammar | Parsed/highlighted; only proven lowering shapes execute. |
| `SYN-CLASSIC-DROP` | DROP instruction | Level C grammar | Parsed/highlighted. |
| `SYN-CLASSIC-EXIT` | EXIT instruction | Level C grammar | Parsed; narrow lowering support. |
| `SYN-CLASSIC-IF` | Classic IF/THEN/ELSE | Level C grammar | Parsed with standard diagnostics; selected lowering support. |
| `SYN-CLASSIC-INTERPRET` | INTERPRET instruction | Level C grammar/architecture | Parser surface; runtime semantics not broadly implemented. |
| `SYN-CLASSIC-ITERATE` | ITERATE instruction | Level C grammar | Parsed/highlighted. |
| `SYN-CLASSIC-LEAVE` | LEAVE instruction | Level C grammar | Parsed/highlighted. |
| `SYN-CLASSIC-NOP` | NOP instruction | Level C grammar | Parsed/highlighted. |
| `SYN-CLASSIC-NUMERIC` | NUMERIC DIGITS/FORM/FUZZ | Level C grammar | Parsed with Classic-shaped diagnostics. |
| `SYN-CLASSIC-PARSE` | PARSE variants and templates | Level C grammar | Parsed/highlighted; broad execution incomplete. |
| `SYN-CLASSIC-PROCEDURE` | PROCEDURE and EXPOSE | Level C grammar | Parsed; pool/exposure runtime incomplete. |
| `SYN-CLASSIC-PULL` | PULL instruction/templates | Level C grammar | Parsed/highlighted. |
| `SYN-CLASSIC-PUSH` | PUSH instruction | Level C grammar | Parsed/highlighted. |
| `SYN-CLASSIC-QUEUE` | QUEUE instruction | Level C grammar | Parsed/highlighted. |
| `SYN-CLASSIC-RETURN` | RETURN instruction | Level C grammar | Parsed; narrow direct-call lowering exists. |
| `SYN-CLASSIC-SAY` | SAY instruction | Level C grammar/lowering | Parsed and supported in proven execution slices. |
| `SYN-CLASSIC-SELECT` | SELECT/WHEN/OTHERWISE | Level C grammar | Parsed with standard diagnostics; broad lowering incomplete. |
| `SYN-CLASSIC-SIGNAL` | SIGNAL target and ON/OFF conditions | Level C grammar | Parsed/highlighted; runtime incomplete. |
| `SYN-CLASSIC-TRACE` | TRACE options/value | Level C grammar | Parsed/highlighted; runtime incomplete. |
| `SYN-CLASSIC-EXPRESSIONS` | Classic arithmetic, comparison, Boolean, and concatenation expressions | Level C scanner/grammar/lowering | Parsed; a documented operator family has canonical lowering. |
| `SYN-CLASSIC-BIF-CALL` | Recognised ANSI BIF calls | `compiler/rxcpcsym.c`; Level C lowering | Names recognised; only a narrow runtime slice is implemented. |
| `SYN-CLASSIC-LOCAL-CALL` | Direct local function/procedure calls | Level C lowering | Narrow canonical lowering slice. |
| `SYN-CLASSIC-DSLSH` | Source tree, diagnostics, and syntax-highlighting projection | Level C parser/highlighter docs and tests | Implemented milestone. |
| `SYN-CLASSIC-CANONICAL-LOWERING` | Transformation to canonical compiler AST | `compiler/rxcp_levelc_lower.c` | Proven subset; unsupported shapes fail closed. |

## RXPP source-language extensions

| ID | Capability | Primary evidence | Observed state |
|---|---|---|---|
| `SYN-RXPP-DEFINE` | `##DEFINE` macros | `preprocessor/rxpp-Users-Guide.md`; `rxpp.crexx` | Source-to-source expansion. |
| `SYN-RXPP-INCLUDE` | `##INCLUDE` positional source inclusion | RXPP guide/implementation | Source-to-source expansion. |
| `SYN-RXPP-USE` | `##USE` append-style source inclusion | RXPP guide/implementation | Source-to-source expansion. |
| `SYN-RXPP-SET` | `##SET` preprocessor variables | RXPP guide/implementation | Compile-time state. |
| `SYN-RXPP-UNSET` | `##UNSET` preprocessor variables | RXPP guide/implementation | Compile-time state. |
| `SYN-RXPP-IF` | `##IF`/`##IFN`/`##ELSE`/`##END` conditional compilation | RXPP guide/implementation | Source-to-source control. |
| `SYN-RXPP-ARRAY` | `##ARRAY` declarations | RXPP guide/implementation | Expands to typed array initialisation. |
| `SYN-RXPP-GLOBAL` | `##GLOBAL` declarations | RXPP guide/implementation | Expands namespace exposure. |
| `SYN-RXPP-STEM` | `##STEM` and stem rewrite engine | RXPP guide/implementation | Source transformation. |
| `SYN-RXPP-CFLAG` | `##CFLAG` processing controls | RXPP guide/implementation | Preprocessor configuration. |
| `SYN-RXPP-DATA` | `##DATA ... ##END` data blocks | RXPP guide/implementation | Expands data into assignments. |
| `SYN-RXPP-SYS-DATA` | `##SYSxxx` system data blocks | RXPP guide/implementation | Reserved data-block family. |
| `SYN-RXPP-OO-CALL` | OO-style call translation | `preprocessor/rxpp-module.md` | Token-oriented source rewrite. |
| `SYN-RXPP-DYNAMIC-STEM` | Dynamic/computed stem tails | RXPP guide/implementation | Token-oriented source rewrite. |
| `SYN-RXPP-FOR` | FOR convenience syntax | RXPP guide/implementation | Source rewrite. |
| `SYN-RXPP-SELECT` | SELECT/WHEN convenience syntax | RXPP guide/implementation | Source rewrite. |
| `SYN-RXPP-SWITCH` | SWITCH/CASE convenience syntax | RXPP guide/implementation | Source rewrite. |
| `SYN-RXPP-SRCMAP` | Generated-source provenance map | RXPP source and focused tests | Emits source-map information for downstream diagnostics. |
