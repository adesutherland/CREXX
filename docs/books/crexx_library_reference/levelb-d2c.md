## d2c

```rexx
d2c(codepoint = .int [, output_length = .int]) = .string
```

The Level B `d2c` helper converts a native Unicode scalar value to one
character. Valid scalar values are `0` through `1114111` (`U+10FFFF`), except
the surrogate range `55296` through `57343` (`U+D800` through `U+DFFF`).

```rexx
d2c(77)      /* "M" */
d2c(945)     /* "α" */
d2c(128293)  /* "🔥" */
d2c(0)       /* NUL */
```

When `output_length` is omitted or one, one character is returned. An explicit
zero returns the empty string without encoding the integer. Any other explicit
length, or an invalid scalar value when a character is requested, raises
`CONVERSION_ERROR`.

The arguments are native `.int` values. The implementation performs bounded
scalar validation and one `appendchar`; it does not convert through hexadecimal
or call another selector.

This is not the Classic Level C D2C contract. Level C converts a whole-number
RexxValue to configuration-coded characters, supports signed twos-complement
when a length is present, and pads or truncates in encoded-character units.
That separate contract is documented in
[`lib/rxfnsc/d2c.md`](../../rxfnsc/d2c.md).

The focused optimized/unoptimized harness is
`lib/rxfnsb/tests_functional/ts_d2c.crexx`.
