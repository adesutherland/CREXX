## `objectarrayinsert` (Level B)

```rexx
objectarrayinsert(array = .object[] expose,
                  from = .int,
                  count = .int,
                  value = .object) = .int
```

`objectarrayinsert` opens `count` slots at the positive one-based position
`from`, shifts the existing tail upward, copies the `value` object into every
new slot, and returns the new high-water mark. Ordinary Level B object
assignment has value-copy semantics; this helper does not create shared
references.

A position beyond the end clamps to append. A zero count is a no-op. A position
below one or negative count raises `INVALID_ARGUMENTS`.

The exposed object array is mutated in place. The implementation uses one VM
`insattrs1` bulk pointer shift followed by one object-value assignment per new
slot; it does not move the existing tail in Rexx code. The focused harness is
`lib/rxfnsb/tests_functional/ts_objectarrayinsert.crexx`.
