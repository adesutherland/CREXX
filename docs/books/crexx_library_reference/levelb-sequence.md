## sequence

`sequence` builds an inclusive ascending range of Unicode characters:

```rexx
sequence(from=.string, tos=.string) = .string
```

Both endpoints must contain exactly one Unicode character. The result includes
both endpoints and every Unicode scalar value between them. Surrogate code
points U+D800 through U+DFFF are not Unicode scalar values and are skipped.

```rexx
sequence("a", "f") == "abcdef"
sequence("α", "γ") == "αβγ"
```

A descending range or an empty/multiple-character endpoint signals
`INVALID_ARGUMENTS`; SEQUENCE never wraps. Use the distinct Level B `xrange`
helper for the legacy wrapping U+0000 through U+00FF domain. Inputs are not
modified.

The implementation reads each endpoint code point once and appends the result
in one linear pass with no Level B helper call or intermediate range array.
`.Rexx.sequence(final)` forwards its receiver and `final` to this function.
There is no Level C BIF named SEQUENCE.

`lib/rxfnsb/tests_functional/ts_sequence.crexx` covers ASCII, Greek and
supplementary characters, the surrogate gap, the highest scalar value,
non-mutation, and every endpoint error family.
