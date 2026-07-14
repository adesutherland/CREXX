# Level B `countstr`

`countstr` counts case-sensitive, non-overlapping occurrences from left to
right:

```rexx
countstr(needle = .string, haystack = .string) = .int
```

A null needle, empty haystack, oversized needle, or absent needle returns zero.

```rexx
countstr("bc", "abcabcabc")  /* 3 */
countstr("aa", "aaaaa")      /* 2, not 4 */
countstr("", "anything")     /* 0 */
```

Both inputs are read-only and there are no value-domain signals. The
implementation computes lengths once and performs one direct VM substring
search per match, advancing by the full needle length.

`lib/rxfnsb/tests_functional/ts_countstr.crexx` covers empty, absent,
oversized, boundary, non-overlapping, Unicode, non-mutation, and 1,000-match
cases in optimized and unoptimized overlays.
