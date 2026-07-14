# Level B `left`

`left` returns an exact-width left-aligned string:

```rexx
left(string = .string, width = .int, pad = " ") = .string
```

`width` is required and must be non-negative. Longer input is truncated to its
leftmost `width` Unicode codepoints; shorter input is padded on the right. `pad`
defaults to blank and must contain exactly one codepoint.

Invalid width or pad input raises `INVALID_ARGUMENTS`.

```rexx
left("abc d", 8)      /* "abc d   " */
left("abc d", 8, ".") /* "abc d..." */
left("abc defg", 6)   /* "abc de" */
left("é", 3, "·")     /* "é··" */
```

The source argument is read-only. The implementation caches both character
lengths, uses one direct cursor/substring operation for truncation, and one VM
`padstr` plus bounded appends for extension. It makes no Level B helper call.
