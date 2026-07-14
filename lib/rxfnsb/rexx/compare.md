# Level B `compare`

`compare` returns the first 1-based character position at which two strings
differ after the shorter string is virtually padded:

```rexx
compare(left = .string, right = .string, pad = " ") = .int
```

The result is zero when the strings compare equal. `pad` must contain exactly
one Unicode codepoint; an invalid pad raises `INVALID_ARGUMENTS`.

```rexx
compare("abc", "abc")        /* 0 */
compare("abc", "ak")         /* 2 */
compare("ab ", "ab")         /* 0: default blank padding */
compare("ab-- ", "ab", "-")  /* 5 */
```

Both source strings are read-only. The implementation compares their common
prefix directly by codepoint and compares only the longer tail with the cached
pad codepoint. It allocates no padded copy of either string.

`lib/rxfnsb/tests_functional/ts_compare.crexx` covers equal and differing
strings, both longer-side directions, blank/nonblank/Unicode padding, Unicode
positions, invalid-pad signals, and non-mutation in optimized and unoptimized
overlays.
