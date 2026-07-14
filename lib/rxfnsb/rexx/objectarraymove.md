# `objectarraymove` (Level B)

```rexx
objectarraymove(array = .object[] expose,
                from = .int,
                count = .int,
                to = .int) = .int
```

`objectarraymove` moves up to `count` object values beginning at the positive
one-based `from` position so they are inserted at `to`, interpreted in the
original array. The order of moved and retained values is preserved, and the
unchanged high-water mark is returned.

An overlong count clamps to the source tail. A destination beyond the end
clamps to append. A zero count, empty array, source beyond the end, or
destination within/immediately after the selected block is a no-op. A source
or destination below one or negative count raises `INVALID_ARGUMENTS`.

The implementation inserts the destination gap first, copies each object value
directly once into a non-overlapping target, then removes the original block
with one `delattrs1`. It avoids the former temporary object array and second
deep-copy pass. The focused harness is
`lib/rxfnsb/tests_functional/ts_objectarraymove.crexx`.
