# `arrayget` (Level B)

```rexx
arrayget(array = .string[], index = .int [, default = .string]) = .string
```

`arrayget` returns `array[index]` when `index` is within the one-based range
`1..array[0]`. It returns `default` when the array is empty or the index is
outside that range. The omitted default is the empty string.

An out-of-range index is a successful fallback lookup, not an error. The input
array is not exposed or modified. The implementation performs two bounds
checks and reads the array element only on the in-range path.

The focused harness is `lib/rxfnsb/tests_functional/ts_arrayget.crexx`.
