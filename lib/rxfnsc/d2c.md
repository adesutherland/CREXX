# `D2C` (Level C Classic BIF)

```text
D2C(number [, length])
CheckArgs: rWHOLENUM>=0
           rWHOLENUM rWHOLE>=0
```

Classic Level C D2C converts a decimal whole-number RexxValue to
configuration-coded characters. Without `length`, `number` must be
non-negative and the result has the minimum encoded width. With a non-negative
`length`, negative numbers are represented in twos-complement and the result is
padded or truncated to exactly that many coded characters. Positive values use
the configuration's zero character for padding; negative values use its
highest-value character.

Standard context errors include `RXC-LC-40.3`/`40.4` for argument count,
`RXC-LC-40.5` for an omitted required argument, `RXC-LC-40.11` for a non-whole
number, `RXC-LC-40.12` for a non-whole length, and `RXC-LC-40.13` for a
negative length or for a negative number without `length`.

This contract is intentionally distinct from the native Unicode-scalar helper
in [`lib/rxfnsb/rexx/d2c.md`](../rxfnsb/rexx/d2c.md). A direct
`RexxClassicBifD2c` implementation and harness are parked until the repository
defines the `Config_B2C` bits-to-coded-character service required by D2C and
X2C. Hard-coding Unicode `appendchar` would not satisfy the Classic contract.
