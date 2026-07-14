# `objectarraydelete` (Level B)

```rexx
objectarraydelete(array = .object[] expose,
                  from = .int,
                  count = .int) = .int
```

`objectarraydelete` removes up to `count` object values beginning at the
positive one-based index `from`, shifts the remaining tail down, and returns
the new high-water mark.

A zero count, empty array, or start beyond the high-water mark is a no-op. An
overlong count clamps to the available tail without overflowing native index
arithmetic. A position below one or negative count raises `INVALID_ARGUMENTS`.

The exposed array is mutated in place through one VM `delattrs1` bulk pointer
shift; object values in the retained tail are not moved by a Rexx-level loop.
The focused harness is
`lib/rxfnsb/tests_functional/ts_objectarraydelete.crexx`.
