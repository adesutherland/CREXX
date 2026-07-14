# Level B `sign`

The native Level B function returns the sign of a decimal value:

```rexx
sign(number = .decimal) = .int
```

It returns `1` for a positive value, `-1` for a negative value, and `0` for
either signed representation of zero. The decimal boundary preserves signs for
values outside binary floating-point range:

```rexx
sign("1E-999")  /*  1 */
sign("-1E+999") /* -1 */
sign("-0")      /*  0 */
```

The implementation performs at most two decimal comparisons and allocates
nothing. It does not format or modify the argument.

The Level B call boundary performs ordinary `.decimal` conversion. Invalid
dynamic conversion raises the catchable `CONVERSION_ERROR` signal.

This is the strongly typed foundation API. The distinct Level C `SIGN` BIF
normalizes Classic numeric text and reports standard context errors.
