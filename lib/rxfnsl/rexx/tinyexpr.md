# TinyExpr Level L Generated-Shape Demo

`tinyexpr.crexx` is a Level L proving slice. It is not a lexer generator or a
parser generator yet. It is hand-written in the shape that a future generator
might emit, so the project can test whether the Rexx and RXAS binary-memory
surface is usable for generated language tooling.

The demo exposes:

- a packed ASCII character-class table stored as a `.binary` constant;
- a table-driven lexer for a tiny arithmetic-expression language;
- a packed binary token stream with fixed 16-byte token records;
- a single exposed constant declaration procedure for token ids, token layout,
  and generated lookup tables;
- a small precedence parser/evaluator that reads the packed token stream;
- helper functions for tests and diagnostics.

The expression language supports integer literals, identifiers as lexer tokens,
`+`, `-`, `*`, `/`, unary minus, and parentheses. Identifiers are lexed so that
the lexer exercises nontrivial spans, but the evaluator accepts only numeric
expressions.

Token records use zero-based byte offsets:

| Field | Offset | Type | Meaning |
| --- | ---: | --- | --- |
| `kind` | 0 | `.u16` | Token id. |
| `start` | 2 | `.u32` | Byte offset in the source. |
| `length` | 6 | `.u16` | ASCII byte/codepoint length. |
| `value` | 8 | `.int` | Parsed integer value for number tokens. |

The public helpers are intentionally small:

- `tinyexpr_lex(source)` returns a packed `.binary` token stream.
- `tinyexpr_eval(source)` evaluates a numeric expression and stores status.
- `tinyexpr_status()`, `tinyexpr_error_offset()`, and
  `tinyexpr_error_message()` expose the last operation status.
- `tinyexpr_token_*` functions inspect token records for tests and examples,
  without materializing source lexemes as strings.
- `tinyexpr_token_is(source, tokens, index, needle)` compares a source slice
  with a binary needle without materializing the slice.
- `TINY_TOK_*` and `TINY_TOKEN_*` constants are defined once in an exposed
  declaration procedure and reused throughout the source module. They are not
  exported to importers in Release 1.

The lessons to watch are:

- whether the declaration-procedure pattern is sufficient for generated
  source-module-local constants, and whether Release 2 should add cross-module
  constants plus wildcard expose forms such as `TINY_TOK_*`;
- whether multiline or chunked binary constants should be accepted directly by
  the lexer/parser so generated tables remain readable without manual joining;
- whether fixed token records are clear enough in Rexx source;
- whether variable-length lexeme handling should stay binary-first, with string
  materialization reserved for diagnostics and NUL-terminated packed fields;
- whether Release 2 memory structs can remove repeated field-offset arithmetic
  without hiding the RXAS-friendly layout;
- whether future generator output should call helper functions or emit direct
  binary-memory intrinsics;
- whether re2c should be ported, or whether changing/replacing its emitter is
  easier once this target shape is better understood.
