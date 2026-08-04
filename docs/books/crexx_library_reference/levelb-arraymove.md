## arraymove

```rexx
arraymove(array = .string[] expose,
          from = .int, count = .int, to = .int) = .int
```

`arraymove` moves up to `count` strings beginning at the positive one-based
`from` index so they begin at insertion position `to` in the original array.
It mutates the array in place and returns its unchanged high-water mark.

The source count clamps to the available tail and a destination beyond the end
clamps to append. Count zero, an empty array, a source above the high-water
mark, and a destination within or immediately after the selected block are
no-ops. A source or destination below one, or a negative count, signals
`INVALID_ARGUMENTS`.

The implementation inserts the destination gap first, making the source and
target ranges non-overlapping. It then performs one direct string copy per
moved value and one bulk deletion, without a temporary array. The focused
harness is `lib/rxfnsb/tests_functional/ts_arraymove.crexx`.
