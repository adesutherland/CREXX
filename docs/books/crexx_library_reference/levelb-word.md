## `word` (Level B)

```rexx
word(string = .string, wordnum = .int) = .string
```

`word` returns the positive one-based `wordnum` word from `string`. Words are
delimited by the VM Unicode whitespace definition. It returns the empty string
when the requested word does not exist, including for empty or blank-only
input. A non-positive word number signals `INVALID_ARGUMENTS`.

The implementation scans directly to the requested word and extracts it once;
it makes no other selector calls. The focused harness is
`lib/rxfnsb/tests_functional/tsword.crexx`.
