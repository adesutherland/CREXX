## `arraycopy` (Level B)

```rexx
arraycopy(array = .string[] expose [, from = .int [, count = .int]]) = .string[]
```

`arraycopy` returns a new array containing up to `count` strings starting at
`from`. `from` defaults to one. A negative start counts back from the end (`-1`
is the last element); zero or a start outside the source returns an empty array.

`count` defaults to zero, meaning copy through the end. A larger count clamps
to the available tail. A negative count signals `INVALID_ARGUMENTS`.

The source is aliased read-only rather than value-copied, and changing the
returned array does not change it. The implementation sizes the result once,
then copies each selected string through direct linked attributes without
per-element growth checks. The focused harness is
`lib/rxfnsb/tests_functional/ts_arraycopy.crexx`.
