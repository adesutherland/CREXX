# `X2C` (Level C Classic BIF)

```text
X2C(hexadecimal)
CheckArgs: rHEX
```

The standalone `rexxclassicbif_x2c` entry removes valid grouping blanks,
left-pads an odd leading nibble, and converts each byte to a character ordinal.
Empty input returns empty and zero ordinals are retained.

Compiled Level C selects the text profile and maps every byte `00` through
`FF` to one Unicode scalar `U+0000` through `U+00FF`. BYTE results remain
binary-authoritative for direct binary consumers. Standard errors are
`RXC-LC-40.3`, `40.4`, `40.5`, and `40.25`.

The optimized/unoptimized direct harness covers grouped/odd/empty input,
all byte ordinals, text results, and every argument class without compiler
lowering.

The native Level B U+00xx mapping remains separate in
[`lib/rxfnsb/rexx/x2c.md`](../rxfnsb/rexx/x2c.md).
