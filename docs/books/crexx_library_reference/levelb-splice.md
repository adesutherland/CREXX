## `splice` (Level B)

```rexx
splice(replacement = .string,
       source = .string,
       at = .int,
       remove_length = .int) = .string
```

`splice` removes up to `remove_length` characters from `source` beginning at
the positive one-based position `at`, then inserts `replacement` at that
position. It does not pad. The result may grow, shrink, or keep the same
length.

```rexx
splice("X", "abcdef", 3, 2)    /* "abXef" */
splice("-", "abcdef", 4, 0)    /* "abc-def" */
splice("", "abcdef", 4, 99)    /* "abc" */
```

A position beyond `length(source) + 1` clamps to append. An overlong removal
clamps to the available tail. A position below one or a negative removal
length raises `INVALID_ARGUMENTS`.

All offsets count Unicode characters rather than UTF-8 bytes. The inputs are
not mutated. The implementation uses direct bounded cursor slices and does not
call `length`, `left`, `substr`, or another selector. The focused harness is
`lib/rxfnsb/tests_functional/ts_splice.crexx`.
