# RXAS Assembler Notes

This root-level page is retained as a short wayfinder. The maintained RXAS
documentation for the Release 1 beta line is in:

- [rxas tool reference](books/crexx_programming_guide/rxas.md)
- [Assembler programming guide](books/crexx_programming_guide/rxas_programming_guide.md)
- [VM instruction characteristics](books/crexx_vm_spec/instruction_characteristics.md)

RXAS is the text assembly language emitted by `rxc` and assembled by `rxas`
into `.rxbin` bytecode. Most users should write Level B source and inspect RXAS
only when debugging, learning the VM, or writing low-level runtime support.
