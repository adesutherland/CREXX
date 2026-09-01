# `X2C` (Level C Classic BIF)

```text
X2C(hexadecimal)
CheckArgs: rHEX
```

The standalone `rexxclassicbif_x2c` entry removes valid grouping blanks,
left-pads an odd leading nibble, and converts each byte to exact configured
output. Empty input returns empty and encoded zero bytes are retained.

BYTE results are binary-authoritative. In UTF8, valid exact UTF-8 output gains a
text view; invalid sequences remain binary without reinterpretation. Standard
errors are `RXC-LC-40.3`, `40.4`, `40.5`, and `40.25`.

The optimized/unoptimized direct harness covers grouped/odd/empty input,
arbitrary bytes, UTF-8 validity flags, and every argument class without compiler
lowering.

The native Level B U+00xx mapping remains separate in
[`lib/rxfnsb/rexx/x2c.md`](../rxfnsb/rexx/x2c.md).
