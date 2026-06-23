# Level C Syntax Highlighting And Integration Plan

Status: Milestone 1 syntax-highlighting baseline implemented; first narrow
execution-lowering tracer implemented
Last updated: 2026-06-23

This document is the durable handoff for the current Level C Classic REXX
front end. The working history and Classic extraction live in
`levelc_working_architecture.md`, with a consolidated compliance reference in
`levelc_compliance_reference.md`; this file records the implemented shape,
manual test workflow, remaining parser gaps, and the planned route from
highlighter-only parsing into canonical compiler integration.

## Scope

The main Level C path is a parser-mode and syntax-highlighting front end:

- it parses Classic REXX source selected by `OPTIONS LEVELC` or parser-mode
  defaulting;
- it builds the normal user-facing source tree used by DSLSH;
- it emits syntax and syntax-adjacent diagnostics using stable `RXC-LC-*`
  identities;
- it deliberately stops before broad canonical lowering, semantic execution
  support, assembly, or VM execution.

Normal `rxc` compilation of Level C remains unsupported except for the first
explicit tracer slice documented in `levelc_remapping_target.md`: top-level
direct scalar assignment/read, string/integer `RexxValue` literals, binary `+`,
and `SAY`. Unsupported Level C compile inputs still fail with the existing
unsupported diagnostic.

## Current Implementation Map

| File | Role |
| --- | --- |
| `compiler/rxcpcscn.re` | Level C scanner; keeps Classic words neutral until glue has context |
| `compiler/rxcpcpar.c` | Level C parser glue and contextual keyword adapter |
| `compiler/rxcpcgmr.y` | Level C Lemon grammar and grammar-level recovery |
| `compiler/rxcpcdiag.c` | Level C syntax-adjacent validation and standard diagnostic helpers |
| `compiler/rxcpcval.c` | Level C source-tree preparation for DSLSH |
| `compiler/rxcpcsym.c` / `.h` | Level C source-symbol helpers and Classic BIF seed list |
| `compiler/rxcp_highlight_controller.c` | Parser-mode dispatch and DSLSH projection |
| `compiler/tests/rexx_src/levelc_*.rexx` | Level C DSLSH positive and negative fixtures |

The Level C files are intentionally separate from the Level B parser and
grammar. Shared code is limited to source-tree projection, diagnostics transport,
and parser-mode infrastructure.

## Parser-Mode Flow

1. `opt_pars()` performs the existing source pre-scan and selects the language
   level.
2. Parser mode dispatches Level C sources to `rexcpars()`.
3. `rxcpcscn.re` tokenizes source text without treating Classic instruction
   words as globally reserved.
4. `rxcpcpar.c` promotes words to contextual keyword tokens only where the
   current clause/parser state proves that role. Assignment and label contexts
   keep keyword-spelled variables as variables.
5. `rxcpcgmr.y` builds the Level C AST and adds grammar-level recovery nodes for
   malformed statement shapes.
6. `rxcp_levelc_prepare_source_ast()` attaches source locations, rewrites shared
   legacy string diagnostics to Level C standard codes where needed, builds the
   source tree, and runs Level C validation.
7. `rxcp_highlight_controller.c` serializes the source tree to DSLSH, overlays
   diagnostics, and stops.

## Implemented Syntax Coverage

The highlighter currently covers the Classic syntax needed for a substantial
first milestone:

- no general reserved words, including keyword-spelled variables in assignment,
  expression, template, and command contexts;
- labels, null clauses, explicit and inferred end-of-clause handling;
- `IF` / `THEN` / `ELSE`, including multiline clauses and missing-`THEN`
  recovery;
- `SELECT`, `WHEN`, `OTHERWISE`, and `END`;
- `DO` forms for simple blocks, controlled loops, repetitive options, and
  `WHILE` / `UNTIL`;
- `LEAVE` and `ITERATE` with loop-target validation;
- expression core for literals, symbols, parentheses, prefix operators, power,
  arithmetic, comparisons, strict comparisons, concat, AND, OR, and XOR;
- simple instruction families: `ADDRESS`, `ARG`, `CALL`, `DROP`, `EXIT`,
  `INTERPRET`, `NOP`, `NUMERIC`, `OPTIONS`, `PARSE`, `PROCEDURE`, `PULL`,
  `PUSH`, `QUEUE`, `RETURN`, `SAY`, `SIGNAL`, and `TRACE`;
- parse templates with targets, dot placeholders, string patterns,
  parenthesized variable patterns, absolute and relative positions, and
  comma-separated template lists;
- Classic command clauses as `IMPLICIT_CMD`, with a warning when a non-string
  command is likely a misspelled instruction.

## Implemented Diagnostic Families

Diagnostics use `RXC-LC-<standard-code>` plus key/value inserts. The rendering
format is intentionally simple for now so DSLSH tests can assert stable
identities before a shared formatter maps codes to final text.

Implemented coverage includes:

- bad comments and unterminated strings: `6.1`, `6.2`, `6.3`;
- malformed grouping structure: `7.*`, `8.*`, `9.*`, `10.*`, `14.*`, `18.*`;
- bad instruction targets and tails: `19.*`, `20.*`, `25.*`;
- invalid expressions and parentheses: `35.*`, `36`, `37.*`;
- parse-template diagnostics: `38.*`, `46.1`;
- loop-control validation: `27.1`, `28.*`;
- label/procedure validation: `16.*`, `17.1`;
- numeric validation for known constants: `26.5`, `26.6`, `33.6`;
- hex/binary string validation: `15.1`, `15.2`, `15.3`, `15.4`;
- static `TRACE` request validation: `24.1`;
- invalid number assignment: `31.1`;
- `ADDRESS WITH` stream/stem validation: `53.*`;
- warning-only implicit ADDRESS command detection:
  `RXC-LC-IMPLICIT_ADDRESS`.

## Test Workflow

Focused Level C parser-mode coverage:

```sh
ctest --test-dir /Users/adrian/CLionProjects/CREXX/cmake-build-release \
  -L levelc --output-on-failure
```

Manual DSLSH dump:

```sh
/Users/adrian/CLionProjects/CREXX/cmake-build-release/dslsyntax-tools/parser_tester \
  -q \
  -p /Users/adrian/CLionProjects/CREXX/cmake-build-release/bin/rxc \
  -s compiler/tests/rexx_src/levelc_deep_validation_ok.rexx \
  compiler/tests/highlighting/dump_ast.txt
```

Manual THE/editor smoke tests should configure parser mode with:

```sh
/Users/adrian/CLionProjects/CREXX/cmake-build-release/bin/rxc --syntaxhighlight
```

Use `OPTIONS LEVELC` at the top of ad hoc files unless intentionally testing
headerless parser-mode defaulting.

## Known Parser Gaps

The remaining front-end gaps are deliberately documented here so tree surgery
does not begin on shaky input:

- non-integer numeric token forms need a full Classic lexer pass, including
  exponent and period-start constants;
- period-start reserved symbols such as `.RC`, `.RESULT`, and invalid
  constant symbols still need scanner/parser support;
- function-call syntax in expressions is not complete enough for BIF/external
  function validation;
- continuation and nested-comment edge cases need another language-specification pass, especially
  where whitespace affects concatenation or string suffix recognition;
- `ADDRESS WITH` validation currently checks source-proven resource syntax but
  does not yet enforce every connection-order or duplicate-connection rule;
- parser-mode DSLSH does not emit grouped comment containers or populate
  `message_code`;
- multiple diagnostics on the same final character span are still constrained
  by current DSLSH flattening behaviour.

## Tree Surgery And Pipeline Integration Plan

The integration rule is: keep the authored Level C source tree stable, and
lower only the compiler work tree.

Recommended sequence:

1. Add a Level C-only lowering entry point, for example
   `rxcp_levelc_lower_to_canonical(Context *)`, called only after Level C
   source validation is clean and only when normal compilation is intentionally
   enabled for that slice. The first implementation is
   `rxcp_levelc_lower_slice1(Context *, const char **)`.
2. Introduce a conservative Level C symbol/pool model before rewriting:
   routine scopes, active variable pools, stems, compound variables, parse
   targets, loop-control names, labels, and expose lists.
3. Lower Classic syntax into canonical nodes and helper calls while preserving
   source provenance:
   - variables, stems, and compound variables become variable-pool get/set/drop
     operations;
   - `ARG`, `PULL`, and `PARSE` become parse-helper calls over the active pool;
   - `ADDRESS` and command clauses become environment protocol calls;
   - `CALL`, `SIGNAL`, `RETURN`, `EXIT`, condition traps, and `PROCEDURE`
     become routine/condition helper calls or canonical control nodes;
   - `NUMERIC`, `TRACE`, and condition state become explicit runtime context
     operations;
   - `DO`, `IF`, and `SELECT` lower to existing canonical control-flow shapes
     wherever the Level B tree already has a compatible representation.
4. Harden `rxcp_ast_rewrite.*` as needed before broad lowering:
   preserve source anchors, avoid symbol aliasing between cloned nodes, validate
   parent/child/sibling consistency after surgery, and keep source diagnostics
   reportable against authored clauses.
5. Feed the lowered canonical work tree into the existing validation pipeline.
   Level C remains gated as unsupported outside each proven slice.

The first executable milestone is deliberately narrower than the long-term
goal: a small Level C program with simple scalar variables, `RexxValue`
literals, binary `+`, and `SAY`. After that, widen by statement family while
keeping the syntax-highlighter fixtures as the static validation canary.
