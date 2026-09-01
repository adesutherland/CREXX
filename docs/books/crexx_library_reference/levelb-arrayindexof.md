## arrayindexof

```rexx
arrayindexof(array = .string[], value = .string
             [, from = .int [, case = .int]]) = .int
```

`arrayindexof` returns the first one-based index at or after `from` whose
complete element equals `value`, or zero when no element matches. `from`
defaults to one; a lower value clamps to one, while a value above `array[0]`
returns zero. Empty strings are ordinary searchable values.

`case` defaults to `1` for case-sensitive string equality. Supply `0` for
uppercase-folded matching. Any other value signals `INVALID_ARGUMENTS`.

The array is aliased read-only rather than value-copied and is never mutated.
For insensitive search the implementation folds `value` once, reuses one
candidate buffer, and calls the VM uppercase operation directly. The focused
harness is `lib/rxfnsb/tests_functional/ts_arrayindexof.crexx`.
