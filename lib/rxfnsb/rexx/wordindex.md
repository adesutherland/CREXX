## `wordindex` (Level B)

```rexx
wordindex(string = .string, wordnum = .int) = .int
```

`wordindex` returns the positive one-based character position of `wordnum` in
`string`. Words are delimited by the VM Unicode whitespace definition. It
returns `0` when the requested word does not exist. A non-positive word number
signals `INVALID_ARGUMENTS`.

The implementation scans only as far as the requested word and allocates no
strings. The focused harness is `lib/rxfnsb/tests_functional/ts_wrdix.crexx`.
