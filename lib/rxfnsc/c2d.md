# `C2D` (Level C Classic BIF)

```text
C2D(string [, length])
CheckArgs: rANY oWHOLE>=0
```

The standalone `rexxclassicbif_c2d` entry converts the exact configured bytes
of `string` to a decimal whole-number `RexxValue`. Without `length`, all bytes
are unsigned. With `length`, the rightmost requested bytes are selected and
interpreted as signed twos-complement; a width larger than the input is
zero-extended and therefore non-negative. Zero returns `0`.

The result uses the caller's `NUMERIC DIGITS`; an unrepresentable result reports
`RXC-LC-40.35`. Standard argument errors are `40.3`, `40.4`, `40.5`, `40.12`,
and `40.13`.

BYTE accepts arbitrary bytes. UTF8 requires valid UTF-8 input, but conversion
still uses its exact encoded bytes. The direct optimized/unoptimized RexxValue
harness covers unsigned/signed widths, arbitrary-size values, zero, invalid
arguments, and the digits limit without compiler lowering.

This is distinct from the one-codepoint native Level B helper documented in
[`lib/rxfnsb/rexx/c2d.md`](../rxfnsb/rexx/c2d.md).
