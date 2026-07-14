# `X2C` (Level C Classic BIF)

```text
X2C(hexadecimal)
CheckArgs: rHEX
```

Classic Level C X2C removes valid grouping blanks, left-pads an odd leading
nibble to a full byte, and converts the resulting bits to configuration-coded
characters. Empty input returns empty and all encoded leading zero bytes are
retained.

Standard context errors are `RXC-LC-40.3`/`40.4` for argument count,
`RXC-LC-40.5` for an omitted required argument, and `RXC-LC-40.25` for invalid
hexadecimal text or blank grouping.

This contract is intentionally distinct from the native Unicode U+00xx helper
in [`lib/rxfnsb/rexx/x2c.md`](../rxfnsb/rexx/x2c.md). Shared HEX validation is
implemented, but a direct `RexxClassicBifX2c` implementation and harness are
parked until the repository defines the `Config_B2C` bits-to-coded-character
service shared by X2C and D2C. Hard-coding Unicode `appendchar` would not satisfy
the Classic contract.
