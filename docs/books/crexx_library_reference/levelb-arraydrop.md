## `arraydrop` (Level B)

```rexx
arraydrop(array = .string[] expose) = .int
```

`arraydrop` clears every logical element from a string array in place and
returns zero, the new high-water mark. Clearing an empty array is an idempotent
no-op, and the same array can be populated again afterward.

The implementation is one VM `setattrs array,0` operation. It does not walk or
copy string values in Rexx code. The focused harness is
`lib/rxfnsb/tests_functional/ts_arraydrop.crexx`.
