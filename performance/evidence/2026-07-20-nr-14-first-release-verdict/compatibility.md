# NR-14 semantic and compatibility record

## Compiler eligibility and fallback

Direct lowering is fail-closed and requires an exact compiler-certified token
and frozen-plan shape:

- plain expression, `VALUE` or `VAR` source represented by one source token;
- no `ARG`, modifier, explicit `INTO`, compound target or repeated target;
- one of the exact three result shapes represented by opcodes 405, 408 or 409;
  opcode 408 requires a strictly positive relative split.

`ARG`, `UPPER`, `LOWER`, `LOG`, `TRACE`, `TRIM`, `INTO`, literal delimiters,
unmatched delimiters, repeated targets and all uncertain/dynamic shapes retain
the existing generic `parseExec` path. `PULL`, `SOURCE`, `LINEIN`, `VERSION` and
`CASELESS` are not implemented selectors/modifiers in the authoritative exit
and are not reinterpreted by NR-14. Relative `+0` is explicitly regression-
tested as generic because its non-advancing word-capture semantics differ from
the positive-position primitive.

The source is evaluated and converted to an internal `.string` temporary before
any result write. Direct results use separate internal `.string` temporaries;
ordinary ordered assignments remain the visible target writes. The focused
contract includes a non-string numeric source, Unicode character positioning
and raw source/result aliasing. The existing 140-case generic runtime suite
remains unchanged and passes in optimized and no-opt modes.

## Opcode and RXBIN contract

| ID | RXAS | Operands | Effect |
| ---: | --- | --- | --- |
| 405 | `parsewords3` | `R,R,R,R` | first word, second word, remaining tail from source op4 |
| 408 | `parsepos2` | `R,R,R,I` | fixed character prefix and next word from source op3 |
| 409 | `parsewords3d` | `R,R,R,R` | first three words; discard remaining tail |

All are `FLOW_NEXT`, optimizer barriers, opaque/may-throw, read only the source
register and write/kill the explicit result registers. Opcode 407 remains
reserved after the rejected array-result PoC was removed.

RXBIN 007 feature declarations are:

- bit 0, `0x00000001`: fixed calls (existing NR-21);
- bit 1, `0x00000002`: frozen PARSE; and
- supported mask: `0x00000003`.

The opcodes use the existing portable RXBIN operand varints. No section,
relocation, pointer, host byte order, prepared graph or retained descriptor is
added. The linker propagates the feature; disassembly/reassembly preserves the
forms. Missing and unknown bits fail closed.

## Runtime, allocation and observability

Both `rxvm` and `rxbvm` share the handlers in `rxvmintp.c`. Blank scanning is
byte-based for ASCII space and therefore cannot split a UTF-8 continuation
byte. Position length is mapped by UTF-8 codepoint boundaries. The normal
compiler path has distinct source/results and takes no handler allocation. A
raw aliased bytecode form snapshots the source before ordered writes and may
signal out-of-memory; the focused contract exercises it in both VMs.

Each form uses the ordinary `START_INSTRUCTION` path and has a canonical opcode
identity, so standard instruction profiling and RXSEQ capture observe it rather
than hiding work in `parseExec`. Disassembly and candidate RXAS prove the same
identity. Dynamic profiling/RXSEQ campaign expansion is outside the frozen
first-verdict gate.

Candidate RexxCPS optimized RXAS contains seven direct sites: five
`parsewords3`, one `parsewords3d` and one `parsepos2`; the seven baseline sites
were generic. Candidate focused PARSE has nine direct sites in both optimized
and no-opt RXAS plus five deliberately retained generic fallback statements.

The `TRACE REXX` probe preserves source-step metadata before stripping and has
identical baseline/candidate output under both VMs. The linked formal workload
images intentionally strip source/TRACE debug metadata and run the benchmark's
normal TRACE-off execution contract.
