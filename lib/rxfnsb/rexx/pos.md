# Level B `pos`

`pos` finds the first occurrence of text at or after a character position:

```rexx
pos(needle = .string, haystack = .string, start = 1) = .int
```

`start` is an optional positive 1-based integer. The result is the 1-based
Unicode character position of the match, or zero when no match exists. A null
needle or haystack also returns zero.

An invalid start raises `INVALID_ARGUMENTS`.

```rexx
pos("day", "Saturday")      /* 6 */
pos(" ", "abc def ghi", 5) /* 8 */
pos("→", "a→b→c", 3)       /* 4 */
pos("", "anything")        /* 0 */
```

Arguments are read-only. The implementation performs direct null checks and
one VM `strpos` search. It makes no LENGTH or other Level B helper call and
allocates no substring.

The VM search is length bounded: U+0000 can occur in either argument and does
not terminate matching.
