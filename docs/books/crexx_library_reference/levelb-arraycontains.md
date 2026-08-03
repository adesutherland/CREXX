## arraycontains

```rexx
arraycontains(array = .string[], value = .string [, case = .int]) = .int
```

`arraycontains` returns `1` when a complete array element equals `value`, and
`0` when no element matches. It searches from index one through `array[0]` and
returns on the first match. Empty strings are ordinary searchable values.

`case` defaults to `1` for case-sensitive string equality. Supply `0` for
uppercase-folded matching. Any other value signals `INVALID_ARGUMENTS`.

The array is aliased read-only rather than value-copied and is never mutated.
For insensitive search the implementation folds `value` once, reuses one
candidate buffer, and calls the VM uppercase operation directly. The focused
harness is `lib/rxfnsb/tests_functional/ts_arraycontains.crexx`.
