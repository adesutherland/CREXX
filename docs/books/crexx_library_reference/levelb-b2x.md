## b2x

```rexx
b2x(binary = .string) = .string
```

`b2x` converts binary digit text to uppercase hexadecimal text. The empty
string is valid and returns the empty string. A partial leading group produces
one hexadecimal digit; every complete four-bit group produces one further
digit, including zero nibbles.

Interior ASCII blanks may separate groups only when a multiple of four binary
digits appears to their right. Leading/trailing blanks, misplaced blanks, and
characters other than `0`, `1`, and blank signal `INVALID_ARGUMENTS`.

```rexx
b2x("11000011")  /* C3 */
b2x("10111")     /* 17 */
b2x("10 1010")   /* 2A */
b2x("")          /* empty */
```

The implementation scans the input directly and emits one nibble at a time. It
does not convert the complete bit string through an integer, so input length is
not constrained by integer or decimal precision.

Classic Level C callers use the RexxValue/error-context contract documented in
[`lib/rxfnsc/b2x.md`](../../rxfnsc/b2x.md).
