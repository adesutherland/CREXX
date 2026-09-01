## right

`right` returns an exact-width right-aligned string:

```rexx
right(string = .string, width = .int, pad = " ") = .string
```

`width` is required and must be non-negative. Longer input is truncated to its
rightmost `width` Unicode codepoints; shorter input is padded on the left. `pad`
defaults to blank and must contain exactly one codepoint.

Invalid width or pad input raises `INVALID_ARGUMENTS`.

```rexx
right("abc  d", 8)  /* "  abc  d" */
right("abc def", 5) /* "c def" */
right("12", 5, "0") /* "00012" */
right("é", 3, "·")  /* "··é" */
```

The source argument is read-only. The implementation caches both character
lengths, uses one direct cursor/substring operation for truncation, and one VM
`padstr` plus bounded appends for extension. It makes no Level B helper call.
