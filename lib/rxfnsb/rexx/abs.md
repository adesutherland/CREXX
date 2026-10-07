## Level B `abs`

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
function first rounds with decimal addition to zero, then compares and,
for a negative value, subtracts from zero. It allocates no string, calls no helper, and does not
modify the caller's value.

Invalid dynamic conversion raises the catchable `CONVERSION_ERROR` signal.

This is the strongly typed foundation API. It does not perform Classic Rexx
numeric-text cleanup itself. The separate Level C `ABS` BIF accepts and
normalizes Classic numeric text through its `rNUM` contract.

Operands are first rounded as `number + 0` under the caller’s `NUMERIC DIGITS`
and `NUMERIC FORM`, following ANSI/Classic rules. Local numeric settings are
restored when the function returns.
