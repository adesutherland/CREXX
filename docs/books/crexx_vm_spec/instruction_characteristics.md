# Virtual Machine Instruction Set

This chapter describes RXVM instructions by functional characteristic. It is a
reference for RXAS authors and for readers inspecting compiler output.

Read the assembler guide before writing non-trivial RXAS by hand; many details
that look obvious at the instruction level are normally handled by the compiler
and linker.

## Characteristics of RXVM Instructions

Most RXVM instructions share these properties:

- logical data flow is written from right to left
- comparisons write their boolean result into an explicit register
- there is no separate condition-code register
- constants and symbols are represented through the RXBIN constant pool
- modules carry metadata records for source mapping, imports, classes,
  interfaces, factories, methods, and linker/runtime support
- typed conversion instructions make representation changes explicit

## Instruction Argument Types

Instruction definitions use compact argument-type names. Common ones include:

| Type | Description |
| --- | --- |
| `REG` | VM register |
| `ID` | label or symbolic identifier |
| `FUNC` | global or local callable reference |
| `INT` | integer literal or constant-pool integer |
| `FLOAT` | floating-point literal or constant-pool float |
| `DECIMAL` | decimal literal or constant-pool decimal |
| `STRING` | string literal or constant-pool string |
| `BINARY` | binary literal or constant-pool binary payload where supported |

Related instructions often share a mnemonic and differ by opcode and operand
types. The generated instruction tables later in this book are the detailed
source for individual opcode forms.
