# Level B `overlay`

`overlay` writes formatted text over a target string:

```rexx
overlay(new_string = .string, target = .string, start = 1,
        overlay_length = 0, pad = " ") = .string
```

`start` is an optional positive 1-based character position. When
`overlay_length` is omitted it is the character length of `new_string`; an
explicitly supplied zero writes no new text. The formatted overlay is truncated
or padded to that width. If `start` is beyond the target, the target is padded
up to the preceding character. `pad` defaults to blank and must contain exactly
one Unicode codepoint.

Invalid start, length, or pad input raises `INVALID_ARGUMENTS`.

```rexx
overlay("qq", "abcd")             /* "qqcd" */
overlay(".", "abcdef", 3, 2)      /* "ab. ef" */
overlay("123", "abc", 5, 6, "+") /* "abc+123+++" */
overlay("foo", "abcdef", 3, 0)   /* "abcdef" */
```

Arguments are read-only. The implementation caches character lengths and uses
direct VM substring, append, and pad operations. It does not call `LEFT`,
`SUBSTR`, `COPIES`, or another Level B formatting helper.
