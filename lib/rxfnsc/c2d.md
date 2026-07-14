# `C2D` (Level C Classic BIF)

```text
C2D(string [, length])
CheckArgs: rANY oWHOLE>=0
```

Classic Level C C2D converts configuration-coded characters to a decimal
whole-number RexxValue. With `length`, it uses the rightmost requested number
of coded characters and interprets the high bit as signed twos-complement. The
conversion uses the caller's `NUMERIC DIGITS`; a result that cannot be
expressed under that setting reports `RXC-LC-40.35`.

Standard context errors also include `RXC-LC-40.3`/`40.4` for argument count,
`RXC-LC-40.5` for an omitted required argument, `RXC-LC-40.12` for a non-whole
length, and `RXC-LC-40.13` for a negative length.

This contract is intentionally distinct from the native Level B single-code-
point helper in [`lib/rxfnsb/rexx/c2d.md`](../rxfnsb/rexx/c2d.md). A direct
`RexxClassicBifC2d` implementation is parked until the repository defines the
`Config_C2B` coded-character-to-bits service required by C2D, C2X, BITAND,
BITOR, and BITXOR. Reusing Unicode code-point extraction here would not satisfy
the Classic contract.
