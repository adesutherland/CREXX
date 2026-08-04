## arrayprepend

```rexx
arrayprepend(array = .string[] expose,
             value = .string,
             count = .int optional) = .int
```

`arrayprepend` adds `value` to the front of `array` `count` times and returns
the new high-water mark. `count` defaults to one. The array uses `array[0]` as
its automatically maintained high-water mark.

A zero count is a no-op. A negative count raises `INVALID_ARGUMENTS`.

The exposed array is mutated in place. The implementation uses one VM
`insattrs1` bulk shift followed by exactly one value assignment per new slot;
it does not move the existing tail in Rexx code. The focused harness is
`lib/rxfnsb/tests_functional/ts_arrayprepend.crexx`.
