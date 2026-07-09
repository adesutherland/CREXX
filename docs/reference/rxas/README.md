# RXAS Human Reference Skeleton

This folder is the first pass at a human-focused RXAS reference. It is intentionally separate from `docs/ai-context` implementation notes and from the historical generated VM-spec instruction chapter.

The goal is to give maintainers and users a readable source that can later feed the generated documentation pipeline. The first pass is skeletal: every instruction mnemonic has a placeholder section, copied operand forms, and TODO slots for prose, examples, and signal behavior.

## Suggested Documentation Model

- Keep this folder as the human-authored source for RXAS semantics.
- Keep `docs/ai-context/RXAS_ASSEMBLER.md` focused on implementation architecture for agents and maintainers.
- Keep `docs/instructions/` as machine/generated instruction metadata.
- Later, teach the VM-spec generator to consume these files or a structured derivative of them, rather than hand-maintaining separate prose in multiple places.

## Current Inventory

- Unique mnemonics: 334
- Opcode/form rows: 529
- Source for this skeleton: `cmake-build-debug/bin/rxas -i`

## Source Syntax

- [RXAS Program Syntax](program-syntax.md): source layout, comments, header directives, procedures, labels, operands, literals, constants, and metadata.

## Instruction Sections

- [Program Control And Calls](instructions/01-program-control.md): 20 mnemonics
- [Data Movement And Conversion](instructions/02-data-movement-and-conversion.md): 7 mnemonics
- [Integer, Logical, And Boolean](instructions/03-integer-logical-and-boolean.md): 44 mnemonics
- [Floating Point](instructions/04-floating-point.md): 24 mnemonics
- [Decimal And Numeric Settings](instructions/05-decimal-and-numeric-settings.md): 46 mnemonics
- [Strings And Characters](instructions/06-strings-and-characters.md): 34 mnemonics
- [Binary Memory](instructions/07-binary-memory.md): 41 mnemonics
- [Arrays, Attributes, References, And Objects](instructions/08-arrays-attributes-references-and-objects.md): 32 mnemonics
- [I/O, Sockets, Processes, And Time](instructions/09-io-sockets-processes-and-time.md): 44 mnemonics
- [Signals, Breakpoints, And Runtime](instructions/10-signals-breakpoints-and-runtime.md): 19 mnemonics
- [Metadata And Introspection](instructions/11-metadata-and-introspection.md): 23 mnemonics
- [Uncategorized Review](instructions/99-uncategorized-review.md): 0 mnemonics

## Review Notes

- The section grouping is editorial and should be reviewed. Move placeholders as the human reference settles.
- Each mnemonic should appear exactly once across the section files; use [Instruction Inventory](instruction-inventory.md) to validate coverage.
- The binary-memory section should be filled first because it contains the newest Release 1 surface and the most nuanced constant/register behavior.
