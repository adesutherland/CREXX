## `x2c` (Level B)

```rexx
x2c(hexadecimal = .string) = .string
```

The Level B `x2c` helper parses hexadecimal bytes and maps each byte value to
the Unicode code point U+0000 through U+00FF. An odd number of digits is
left-padded with one zero nibble; empty input returns empty and leading zero
bytes are retained.

This produces valid Level B UTF-8 text rather than a raw byte buffer. For
example, hexadecimal `FF` becomes U+00FF (UTF-8 bytes `C3 BF`), not an invalid
single `FF` byte. Use `.binary` and the binary-memory helpers for untyped bytes.

Interior ASCII blanks are permitted only when an even number of hexadecimal
digits lies to their right. Leading/trailing blanks, mis-grouped blanks, and
non-hexadecimal characters raise `INVALID_ARGUMENTS`.

```rexx
x2c("416263") /* "Abc" */
x2c("F")      /* U+000F */
x2c("004D")   /* U+0000 followed by "M" */
x2c("FF")     /* U+00FF */
x2c("")       /* empty */
```

The implementation validates once and converts once with direct ASCII nibble
arithmetic. It does not allocate a blank-stripped copy, search conversion
tables, or call another selector.

Classic Level C X2C instead uses configuration-coded characters; its separate
contract and current dependency are documented in
[`lib/rxfnsc/x2c.md`](../../rxfnsc/x2c.md). The focused native harness is
`lib/rxfnsb/tests_functional/ts_x2c.crexx`.
