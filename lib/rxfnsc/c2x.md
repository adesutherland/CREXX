# `C2X` (Level C Classic BIF)

```text
C2X(string)
CheckArgs: rANY
```

The standalone `rexxclassicbif_c2x` entry converts each character ordinal to
two uppercase hexadecimal digits. Empty input and leading zero ordinals are
retained.

Compiled Level C selects the text profile: `U+0000` through `U+00FF` map to
`00` through `FF`, and a higher scalar raises `RXC-LC-23.1`. The direct BYTE
profile remains for binary consumers such as RexxScript. Standard argument
errors are `RXC-LC-40.3`, `40.4`, and `40.5`.

The optimized/unoptimized direct harness covers all 256 ordinals, arbitrary
binary in BYTE mode, empty input, argument errors, and source preservation.

The native Level B low-codepoint helper remains a different API, documented in
[`lib/rxfnsb/rexx/c2x.md`](../rxfnsb/rexx/c2x.md).
