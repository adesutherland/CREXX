# `wordlength` (Level B)

```rexx
wordlength(string = .string, wordnum = .int) = .int
```

`wordlength` returns the Unicode character length of the positive one-based
`wordnum` in `string`. Words use the VM Unicode whitespace definition. It
returns `0` when the requested word does not exist. A non-positive word number
signals `INVALID_ARGUMENTS`.

The implementation scans only as far as the requested word and computes its
length from source positions without allocating a substring or calling another
selector. The focused harness is
`lib/rxfnsb/tests_functional/ts_wordlength.crexx`.
