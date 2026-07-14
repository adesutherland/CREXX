# `d2x` (Level B)

```rexx
d2x(number = .int [, output_length = .int]) = .string
```

The Level B `d2x` helper converts a native signed 64-bit integer to uppercase
hexadecimal digit text. Without `output_length`, `number` must be non-negative
and the result uses the minimum width (`d2x(0)` is `"0"`).

With a non-negative `output_length`, the result has exactly that many digits.
Positive values are padded with `0`, negative values are represented in
twos-complement and padded with `F`, and excess high digits are discarded.

```rexx
d2x(129)       /* "81" */
d2x(129, 4)    /* "0081" */
d2x(257, 2)    /* "01" */
d2x(-127, 2)   /* "81" */
d2x(-127, 4)   /* "FF81" */
d2x(12, 0)     /* empty */
```

A negative value without a supplied width or a negative width raises
`INVALID_ARGUMENTS`. Both arguments are native `.int` values.

The implementation extracts the requested nibbles directly from the native
integer. It performs at most 16 nibble extractions plus one bulk left pad and
does not call `right`, divide per digit, or build and reverse an intermediate
string.

The separate Classic Level C BIF accepts caller-context Rexx whole numbers and
is documented in [`lib/rxfnsc/d2x.md`](../../rxfnsc/d2x.md). The focused native
harness is `lib/rxfnsb/tests_functional/ts_d2x.crexx`.
