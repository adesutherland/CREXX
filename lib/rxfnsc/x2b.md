# `X2B` (Level C Classic BIF)

```text
X2B(hexadecimal)
CheckArgs: rHEX
```

Classic Level C X2B converts every hexadecimal digit to exactly four binary
digits. Input is case-insensitive, the empty string returns empty, and leading
zero nibbles are retained.

Interior ASCII blanks are ignored after validation and are valid only when an
even number of hexadecimal digits lies to their right. Leading/trailing blanks,
mis-grouped blanks, and non-hexadecimal characters report `RXC-LC-40.25`.

```rexx
X2B("C3")     /* "11000011" */
X2B("7")      /* "0111" */
X2B("1 C1")   /* "000111000001" */
X2B("0001")   /* "0000000000000001" */
X2B("")       /* empty */
```

Argument-count and presence errors are `RXC-LC-40.3`, `RXC-LC-40.4`, and
`RXC-LC-40.5`. The standalone
`rexxclassicbifx2b.rexxclassicbif_x2b(context_ref)` entry uses the shared HEX
validator and emits bits directly; it does not use the compatibility name
controller or compiler lowering.

The distinct typed Level B API is documented in
[`lib/rxfnsb/rexx/x2b.md`](../rxfnsb/rexx/x2b.md). The direct Level C harness is
`lib/rxfnsc/tests_functional/testRexxClassicBifX2b.crexx`.
