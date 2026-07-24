## `arrayhi` (Level B)

```rexx
arrayhi(array = .string[] expose
        [, mode = .string [, newhi = .int]]) = .int
```

`arrayhi(array)` and case-insensitive `arrayhi(array, "GET")` return the
current `array[0]` high-water mark without changing the array.
`arrayhi(array, "SET", newhi)` shrinks the array when `newhi` is positive and
smaller than its current mark. A same/larger or non-positive `newhi` is a
documented no-op; `arrayhi` never grows an array.

An unknown mode, `SET` without `newhi`, or `GET` with `newhi` signals
`INVALID_ARGUMENTS`. The omitted GET path returns immediately; explicit modes
use one direct VM uppercase operation, and an actual shrink uses one `setattrs`.

The focused harness is `lib/rxfnsb/tests_functional/ts_arrayhi.crexx`.
