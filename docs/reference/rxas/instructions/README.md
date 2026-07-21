# RXAS Instruction Sections

These files contain one section for every RXAS mnemonic, grouped for human
reading rather than opcode order.

## Human Reference Format

Instruction pages should use a stable Markdown shape so they can be read by
people and processed by documentation tooling:

1. Keep one second-level heading per mnemonic: ``## `mnemonic` ``.
2. Begin with one short paragraph that says what the instruction is for.
3. Use a `### Forms` table for opcode, syntax, and effect.
4. Use `### Operands`, `### Semantics`, `### Signals`, `### Example`, and
   `### Related` when those sections add useful detail.
5. Keep assembler syntax examples in fenced code blocks whose language is
   exactly `rxas`. Put test metadata in the preceding HTML comment, not in the
   fence info string, so syntax highlighters only need to recognise `rxas`.

Example block convention:

<!-- rxas-example name="example-name" test="run" -->
```rxas
.globals=0

main() .locals=1
    ret
```

Optional expected output should immediately follow the source block:

<!-- rxas-output for="example-name" -->
```text
expected output
```

Use `test="assemble"` for examples that should only be assembled, and
`test="run"` for examples that should be assembled and executed. A testable
example must be a complete RXAS source file unless the surrounding text
explicitly marks it as a fragment.

| Section | Mnemonics | File |
| --- | ---: | --- |
| Program Control And Calls | 30 | [01-program-control.md](01-program-control.md) |
| Data Movement And Conversion | 7 | [02-data-movement-and-conversion.md](02-data-movement-and-conversion.md) |
| Integer, Logical, And Boolean | 44 | [03-integer-logical-and-boolean.md](03-integer-logical-and-boolean.md) |
| Floating Point | 24 | [04-floating-point.md](04-floating-point.md) |
| Decimal And Numeric Settings | 46 | [05-decimal-and-numeric-settings.md](05-decimal-and-numeric-settings.md) |
| Strings And Characters | 34 | [06-strings-and-characters.md](06-strings-and-characters.md) |
| Binary Memory | 43 | [07-binary-memory.md](07-binary-memory.md) |
| Arrays, Attributes, References, And Objects | 32 | [08-arrays-attributes-references-and-objects.md](08-arrays-attributes-references-and-objects.md) |
| I/O, Sockets, Processes, And Time | 44 | [09-io-sockets-processes-and-time.md](09-io-sockets-processes-and-time.md) |
| Signals, Breakpoints, And Runtime | 19 | [10-signals-breakpoints-and-runtime.md](10-signals-breakpoints-and-runtime.md) |
| Metadata And Introspection | 23 | [11-metadata-and-introspection.md](11-metadata-and-introspection.md) |
| Large And Fused Instructions | 29 | [12-large-instructions.md](12-large-instructions.md) |
| Uncategorized Review | 0 | [99-uncategorized-review.md](99-uncategorized-review.md) |
