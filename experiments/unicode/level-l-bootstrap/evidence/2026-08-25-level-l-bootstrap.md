# Level L Bootstrap And Unicode-Lexer Evidence — 2026-08-25

Revision under test: local `unicode` branch work following `48bace22f`.

Command, from the repository root:

```sh
experiments/unicode/level-l-bootstrap/run.sh
```

Retained result:

```text
Level B frontend: deterministic Token and AST dumps
parser policy: single cursor, no backtracking, first-fault panic
TinyExpr cREXX: byte-identical generation
TinyExpr differential oracle: PASS
ICU gennorm2 nfc.txt lexer: PASS (2500 lines)
Unicode rule parser: PASS (2485 typed records with byte/token spans)
C++/re2c semantic oracle: byte-identical (2485 records)
generated dispatch: .jtable and jumpi
binary scan/stores: bgetu8, bsetu16, bsetu32
rxtvm: PASS
rxbvm: PASS
```

The command compiled and assembled the Level B bootstrap frontend, linked it,
and used both VM families to parse `specs/tinyexpr.levell` and
`specs/gennorm2.levell`. Token dumps, structural AST dumps, and emitted re2c
adapter input were identical between the VMs and byte-identical to the retained
files. Vendored re2c 4.5.1 then emitted the retained cREXX scanners, which were
compiled with `rxc`, assembled with `rxas`, and linked with `rxlink`.

TinyExpr matched the maintained `rxfnsl` lexer for token kinds, names, starts,
and byte lengths across empty, whitespace, identifier, number, punctuation,
expression, and invalid-prefix fixtures. The Unicode scanner accepted the
complete frozen ICU 78.3 `nfc.txt` and checked 163 range operators, 403 CCC
records, 961 two-way mappings, 1,120 one-way mappings, one version declaration,
2,500 line endings, and one EOF token. The UTF-8 comment fixture retained the
newline's byte offset after a two-byte scalar.

The generated Unicode tokens then passed through the hand-written Level B
consumer parser on both VM families. It constructed 2,485 typed records: one
version declaration, 403 CCC records covering 968 code points, 961 two-way
mappings, and 1,120 one-way mappings containing 3,127 values. The retained
record dump includes exact source lines, byte spans, and token spans and was
byte-identical between VMs. The canonical semantic dump was byte-identical to
the complete dump produced by the independent C++/re2c parser. Focused tests
also exercised UTF-8/CRLF span accounting and first-fault panics for malformed
CCC syntax, an unsupported version, invalid two-way arity, a non-scalar value,
and a missing version.

No production compiler, RXAS, linker, VM, library, or CMake source changed.
The result proves the experimental authored Level L path, cREXX output adapter,
and typed Unicode rule parsing. It does not prove normalization-table
generation, NFD/NFC runtime behavior, malformed binary UTF-8 handling, a
native Level L automaton core, packed layout superiority, or self-hosting.
