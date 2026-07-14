# `D2C` (Level C Classic BIF)

```text
D2C(number [, length])
CheckArgs: rWHOLENUM>=0
           rWHOLENUM rWHOLE>=0
```

The standalone `rexxclassicbif_d2c` entry converts an arbitrary-precision
Classic whole number to exact configured bytes. Without `length`, the number
must be non-negative and the minimum byte width is returned. With `length`, the
result is padded or truncated to exactly that many bytes and negative values
use twos-complement/sign extension. Length zero returns empty.

BYTE results are binary-authoritative. In UTF8, exact output bytes are marked as
text only when they form canonical valid UTF-8; otherwise they remain binary.
No decoding, fallback, or normalization occurs.

Standard errors include `RXC-LC-40.3`, `40.4`, `40.5`, `40.12`, and `40.13`.
The direct optimized/unoptimized harness covers positive/negative widths,
padding, truncation, empty output, UTF-8 validity flags, and argument errors.

This differs from the native Unicode-scalar Level B helper in
[`lib/rxfnsb/rexx/d2c.md`](../rxfnsb/rexx/d2c.md).
