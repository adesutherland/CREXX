# `C2X` (Level C Classic BIF)

```text
C2X(string)
CheckArgs: rANY
```

The standalone `rexxclassicbif_c2x` entry converts each exact configured byte
to two uppercase hexadecimal digits. Empty and multi-byte inputs are valid and
encoded leading zeros are retained.

BYTE accepts arbitrary bytes. UTF8 requires valid UTF-8 text, then converts its
exact UTF-8 representation. Standard errors are `RXC-LC-40.3`, `40.4`, `40.5`,
and UTF8 invalid-data `23.1`.

The optimized/unoptimized direct harness covers text, multibyte UTF-8, arbitrary
binary bytes, empty input, argument errors, and source preservation without
compiler lowering.

The native Level B low-codepoint helper remains a different API, documented in
[`lib/rxfnsb/rexx/c2x.md`](../rxfnsb/rexx/c2x.md).
