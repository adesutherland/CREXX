# Level B `insert`

`insert` adds formatted text after a character position in a target string:

```rexx
insert(new_string = .string, target = .string, before = 0,
       insert_length = 0, pad = " ") = .string
```

`before` and `insert_length` are optional non-negative integers. The default
position is zero, before the first target character. When `insert_length` is
omitted it is the character length of `new_string`; an explicitly supplied zero
inserts no new text. `pad` defaults to blank and must contain exactly one
Unicode codepoint.

If `before` is beyond the target, padding extends the target up to the insertion
point. The inserted text is truncated or padded to `insert_length`.

Invalid position, length, or pad input raises `INVALID_ARGUMENTS`.

```rexx
insert("123", "abc")              /* "123abc" */
insert(" ", "abcdef", 3)          /* "abc def" */
insert("123", "abc", 5, 6, "+")  /* "abc++123+++" */
insert("abc", "def", 2, 1)       /* "deaf" */
```

Arguments are read-only. The implementation caches character lengths and uses
direct VM substring, append, and pad operations. It does not call `LEFT`,
`SUBSTR`, `COPIES`, or another Level B formatting helper.
