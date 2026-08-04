## subword

```rexx
subword(string = .string, start = .int [, count = .int]) = .string
```

`subword` returns up to `count` words beginning at the positive one-based
`start`. Omitted count selects through the final word; explicit zero returns
empty; and an oversized count clamps to the remaining words. A start beyond the
available words returns empty.

Leading and trailing whitespace is excluded while all whitespace between the
selected words is preserved. Words use the VM Unicode whitespace definition.
A non-positive start or supplied negative count signals `INVALID_ARGUMENTS`.

The implementation scans the source once to identify one span and extracts it
once, without other selector calls. The focused harness is
`lib/rxfnsb/tests_functional/ts_subword.crexx`.
