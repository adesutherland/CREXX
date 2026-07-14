# DATATYPE — Level B

```text
datatype(value = .string [, type = .string]) = .string
```

With `type` omitted, `DATATYPE` returns `NUM` for a syntactically valid Level B
number and `CHAR` otherwise. An explicit type tests the first option codepoint
and returns `1` or `0`.

Level B recognizes `A`, `B`, `D`, `L`, `M`, `N`, `S`, `U`, `W`, and `X`.
`D` is the cREXX extension for ASCII digits. The other character classes are
also deliberately ASCII-based; traversal is codepoint-safe, but this foundation
function does not claim full Unicode property behavior.

`B` and `X` accept the empty string. Embedded ASCII spaces are allowed only at
right-counted group boundaries: multiples of four binary digits and pairs of
hexadecimal digits. Leading and trailing spaces are invalid. Other non-numeric
classes reject the empty string.

`N` supports surrounding blanks, an optional sign, decimal point, and signed
decimal exponent. `W` applies the same syntax and tests the exact mathematical
value. It does not convert through binary floating point or use a tolerance, so
`123.000` is whole while `123.0000003` is not.

An explicit empty or unsupported option signals `INVALID_ARGUMENTS`. The input
is never modified.

The separate Classic Level C BIF is documented in
`lib/rxfnsc/datatype.md`; it intentionally excludes the Level B `D` extension.
