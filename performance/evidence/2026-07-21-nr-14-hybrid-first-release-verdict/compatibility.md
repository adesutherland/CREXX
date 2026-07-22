# Hybrid compatibility contract

## Opcode and RXBIN contract

Opcode 410 is `PARSEPLAN_REG_REG_STRING`, RXAS `parseplan R,R,S`. Operand 1 is
the reusable `.string[]` result vector, operand 2 is the copied string source,
and operand 3 is an immutable constant descriptor. It is `FLOW_NEXT`, an
optimizer barrier, reads operand 2, writes/kills operand 1, and is opaque and
may-throw.

The descriptor is portable little-endian data:

- header: magic `0x50`, version 1, header size 8, 16-bit item count and 16-bit
  compact stored-result count;
- target item: kind and store/drop flag;
- literal item: kind, flags, 32-bit byte length, 32-bit character length and
  UTF-8 bytes;
- absolute/relative items: kind, normalization flag and 64-bit nonnegative
  movement; and
- implicit-word item: kind and zero flags.

All four NR-14 opcodes require RXBIN 007 feature bit
`RXBIN007_FEATURE_FROZEN_PARSE` (`0x00000002`). No new section or relocation is
introduced. Linker propagation, disassembly/reassembly and both VMs are covered
by the focused contract. Missing bits, unknown bits and malformed descriptors
fail closed.

## Compiler selection

- Exact three-word, three-word-plus-drop and positive positional shapes keep
  opcodes 405, 409 and 408 respectively.
- Longer odd implicit-word templates chain opcode 405 through internal source,
  word and tail temporaries. Ordered final assignments preserve repeated and
  compound targets, drops, source aliasing, case conversion and `TRIM`.
- Other mechanically frozen plans use opcode 410 and compact stored-result
  indexing. Literal names and target names are not decoded at runtime.
- Explicit `INTO`, `LOG`, `TRACE`, invalid and unsupported plans retain
  `parseExec`; regex behavior is untouched.

The source is evaluated/copied before any visible target assignment. The
existing semantic suite plus the new contract covers blank-delimited tails,
literal and empty fields, Unicode delimiters, absolute/relative positions,
overlay normalization, drops, repeated targets, modifiers and source aliases.
