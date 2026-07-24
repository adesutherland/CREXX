## `x2d` (Level B)

```rexx
x2d(hexadecimal = .string [, signed_length = .int]) = .int
```

The Level B `x2d` helper converts grouped hexadecimal text to an exact native
signed 64-bit integer. Without `signed_length`, the text is unsigned; empty
input returns zero and values above `INT64_MAX` raise `OVERFLOW_UNDERFLOW`.

With a non-negative `signed_length`, only the rightmost requested hexadecimal
digits are used. Shorter input is left-padded with zero, and the selected high
bit determines signed twos-complement. A zero length returns zero. Results that
cannot be sign-extended into the native range raise `OVERFLOW_UNDERFLOW`.

Interior ASCII blanks are valid only when an even number of hexadecimal digits
lies to their right. Invalid text or a negative length raises
`INVALID_ARGUMENTS`.

```rexx
x2d("81")       /* 129 */
x2d("81", 2)    /* -127 */
x2d("81", 4)    /* 129 */
x2d("F081", 3)  /* 129 */
x2d("F081", 2)  /* -127 */
x2d("0031", 0)  /* 0 */
```

The implementation validates/counts once and scans only the selected digits.
It accepts arbitrarily many leading zeros or valid sign-extension digits
without allocating a cleaned/right-aligned copy, and makes no selector call.

Classic Level C X2D returns a Rexx whole number under caller numeric settings;
that separate contract is documented in
[`lib/rxfnsc/x2d.md`](../../rxfnsc/x2d.md). The focused native harness is
`lib/rxfnsb/tests_functional/ts_x2d.crexx`.
