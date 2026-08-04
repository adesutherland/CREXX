## arrayinsert

```rexx
arrayinsert(array = .string[] expose,
            from = .int,
            count = .int,
            default = .string optional) = .int
```

`arrayinsert` opens `count` slots at the positive one-based position `from`,
shifts the existing tail upward, assigns `default` to every new slot, and
returns the new high-water mark. `default` is the empty string when omitted.
The array uses `array[0]` as its automatically maintained high-water mark.

A position beyond the end clamps to append. A zero count is a no-op. A position
below one or negative count raises `INVALID_ARGUMENTS`.

The exposed array is mutated in place. The implementation uses one VM
`insattrs1` bulk pointer shift followed by exactly one assignment per inserted
slot; it does not copy the existing tail in Rexx code. The focused harness is
`lib/rxfnsb/tests_functional/ts_arrayinsert.crexx`.
