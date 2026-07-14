# `C2X` (Level C Classic BIF)

```text
C2X(string)
CheckArgs: rANY
```

Classic Level C C2X accepts one present RexxValue and converts its coded
characters to uppercase hexadecimal through the configuration's character-to-
bits encoding. It does not inherit the native Level B low-code-point-byte
behavior.

The empty string is valid, multi-character input is valid, and encoded leading
zero digits are retained.

```rexx
C2X("M") /* configuration encoding of M, commonly 4D or D4 */
C2X("")  /* empty */
```

Standard context errors are `RXC-LC-40.3` for too few arguments,
`RXC-LC-40.4` for too many, and `RXC-LC-40.5` for an omitted required
argument.

A direct `RexxClassicBifC2x` implementation and harness are parked until the
repository defines the `Config_C2B` coded-character-to-bits service shared by
C2X, C2D, BITAND, BITOR, and BITXOR. Hard-coding the Level B low-byte behavior
would not satisfy this contract. The native Level B API is documented
separately in [`lib/rxfnsb/rexx/c2x.md`](../rxfnsb/rexx/c2x.md).
