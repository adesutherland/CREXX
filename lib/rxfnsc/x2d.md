# `X2D` (Level C Classic BIF)

```text
X2D(hexadecimal [, length])
CheckArgs: rHEX oWHOLE>=0
```

Classic Level C X2D removes valid hexadecimal grouping blanks and returns the
exact decimal whole number in a RexxValue. Without `length`, the hexadecimal
field is unsigned. With a non-negative `length`, only the rightmost requested
hexadecimal digits are used, a shorter field is left-padded with zero, and the
selected high bit gives the signed twos-complement interpretation. A zero
length returns zero.

```rexx
X2D("81")       /* 129 */
X2D("81", 2)    /* -127 */
X2D("81", 4)    /* 129 */
X2D("F081", 3)  /* 129 */
X2D("F081", 2)  /* -127 */
X2D("0031", 0)  /* 0 */
```

The standalone `rexxclassicbifx2d.rexxclassicbif_x2d(context_ref)` entry
converts arbitrary-width fields through mutable base-1e9 limbs. It neither
narrows the result to a native `.int` nor calls the compatibility name
controller. A result whose magnitude has more significant digits than the
caller's `NUMERIC DIGITS` reports `RXC-LC-40.35`.

Standard context errors are `RXC-LC-40.3`/`40.4` for argument count,
`RXC-LC-40.5` for an omitted required argument, `RXC-LC-40.12` for a
non-whole length, `RXC-LC-40.13` for a negative length, and `RXC-LC-40.25`
for invalid hexadecimal text or grouping.

The distinct native signed-64-bit API is documented in
[`lib/rxfnsb/rexx/x2d.md`](../rxfnsb/rexx/x2d.md). The direct Level C harness
is `lib/rxfnsc/tests_functional/testRexxClassicBifX2d.crexx`.
