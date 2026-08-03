## lastpos

`lastpos` finds the last match ending at or before a character position:

```rexx
lastpos(needle = .string, haystack = .string, search_end = 0) = .int
```

`search_end` is optional. When omitted, the complete haystack is considered; a
supplied value must be a positive 1-based integer. A match is eligible only when
its final character is at or before `search_end`. The result is the match's
1-based Unicode character position, or zero for no match or a null needle.

An invalid supplied search end raises `INVALID_ARGUMENTS`.

```rexx
lastpos(" ", "abc def ghi")    /* 8 */
lastpos(" ", "abc def ghi", 7) /* 4 */
lastpos("abc", "abc abc", 6)   /* 1: the later match ends at 7 */
lastpos("aa", "aaa")           /* 2: overlapping matches count */
```

Arguments are read-only. The VM has no reverse substring-search instruction,
so the implementation caches both character lengths and performs monotonic
`strpos` searches from successive matches. It creates no substrings and permits
overlapping candidates.
