## arrayset

```rexx
arrayset(array = .string[] expose, index = .int, value = .string
         [, fill = .string]) = .int
```

`arrayset` assigns `value` to the positive one-based `index` and returns the
resulting `array[0]` high-water mark. An existing element is overwritten in
place. When `index` is beyond the current end, the array grows through that
index and each intervening slot receives `fill`; the omitted fill is the empty
string.

A non-positive `index` signals `INVALID_ARGUMENTS`. The array is exposed and
mutated in place. Growth uses one bulk attribute insertion, skips explicit gap
initialisation for an empty fill, and never writes `fill` into the target slot
before overwriting it with `value`.

The focused harness is `lib/rxfnsb/tests_functional/ts_arrayset.crexx`.
