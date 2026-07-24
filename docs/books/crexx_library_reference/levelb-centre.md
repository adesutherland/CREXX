## Level B `centre`

`centre` is the British-spelling Level B entry for exact-width centring and
central truncation:

```rexx
centre(string = .string, width = .int, pad = " ") = .string
```

- `width` must be non-negative.
- `pad` must contain exactly one Unicode codepoint.
- An uneven extra pad character is placed on the right.
- Uneven truncation removes the extra character from the right, retaining the
  left-biased central slice.

Invalid width or pad input raises `INVALID_ARGUMENTS`.

```rexx
centre("ABC", 7)           /* "  ABC  " */
centre("ABC", 8, "-")      /* "--ABC---" */
centre("The blue sky", 8)  /* "e blue s" */
centre("anything", 0)      /* "" */
```

The source argument is read-only. This performance-critical Level B entry uses
the same reviewed direct VM algorithm as `center`: lengths are computed once,
truncation uses cursor/subslice operations, and one half-padding allocation is
reused on both sides.

`lib/rxfnsb/tests_functional/ts_centre.crexx` is CENTRE-only. The American
spelling has separate source, tests, and documentation.
