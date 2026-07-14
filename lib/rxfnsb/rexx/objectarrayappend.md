# `objectarrayappend` (Level B)

```rexx
objectarrayappend(array = .object[] expose,
                  value = .object,
                  count = .int optional) = .int
```

`objectarrayappend` copies `value` to the end of `array` `count` times and
returns the new high-water mark. `count` defaults to one. Ordinary Level B
object assignment has value-copy semantics; appended slots are not shared
references to the caller's object.

A zero count is a no-op. A negative count raises `INVALID_ARGUMENTS`.

The exposed object array is mutated in place. The implementation uses one VM
`insattrs1` bulk growth operation followed by one object-value assignment per
new slot; existing values are not moved in Rexx code. The focused harness is
`lib/rxfnsb/tests_functional/ts_objectarrayappend.crexx`.
