# `b2d` (Level B)

```rexx
b2d(binary = .string) = .int
```

`b2d` converts binary digit text to a non-negative Level B integer. The empty
string and strings containing only zero digits return `0`.

Interior ASCII blanks may separate groups only when a multiple of four binary
digits appears to their right. Leading/trailing blanks, misplaced blanks, and
characters other than `0`, `1`, and blank signal `INVALID_ARGUMENTS`.

```rexx
b2d("01110")      /* 14 */
b2d("10000001")   /* 129 */
b2d("10 1010")    /* 42 */
```

The return type is the signed 64-bit `.int`, so the greatest accepted unsigned
result is `9223372036854775807` (63 one bits). A larger significant value
signals `OVERFLOW_UNDERFLOW`; any number of leading zero bits is harmless.

This is a Level B native helper, not a repository Level C BIF. It has one
argument and does not implement the optional signed-width extension described
by older imported class-reference material.
