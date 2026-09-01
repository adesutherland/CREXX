## `objectarraydrop` (Level B)

```rexx
objectarraydrop(array = .object[] expose) = .int
```

`objectarraydrop` clears every logical element from an object array in place and
returns zero, the new high-water mark. Clearing an empty array is an idempotent
no-op, and the same array can be populated again afterward.

The implementation is one VM `setattrs array,0` operation. It does not walk or
copy object values in Rexx code. The focused harness is
`lib/rxfnsb/tests_functional/ts_objectarraydrop.crexx`.
