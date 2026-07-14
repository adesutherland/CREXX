# Level B `abs`

The native Level B `abs` function returns a non-negative decimal value:

```rexx
abs(number = .decimal) = .decimal
```

```rexx
abs(12.3)           /* 12.3 */
abs(-12.345)        /* 12.345 */
abs("-123.45E+16") /* 1.2345E+18 after typed conversion */
```

The Level B call boundary performs the ordinary `.decimal` conversion. The
function itself needs only one comparison and, for a negative value, one
decimal subtraction. It allocates no string, calls no helper, and does not
modify the caller's value.

Invalid dynamic conversion raises the catchable `CONVERSION_ERROR` signal.

This is the strongly typed foundation API. It does not perform Classic Rexx
numeric-text cleanup itself. The separate Level C `ABS` BIF accepts and
normalizes Classic numeric text through its `rNUM` contract.
