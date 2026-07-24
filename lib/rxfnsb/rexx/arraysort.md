## Level B `arraysort`

`arraysort` sorts a one-based Level B string array in place:

```rexx
arraysort(expose array=.string[] [,offset=.int [,order=.string [,debug=.int]]]) = .int
```

`offset` is the positive one-based character position at which each lexical,
case-sensitive key begins and defaults to `1`. Text before it remains part of
the moved value but not the comparison. A position beyond a value yields an
empty key. `order` is case-insensitive `ASC` (default) or `DESC`. `debug` is `0`
by default; `1` prints one summary line. The return is the unchanged high-water
mark. Equal-key order is not stable.

Invalid offset, order, or debug values signal `INVALID_ARGUMENTS`. Because the
array is exposed, caught signals currently share the repository's parked
exposed-argument unwind dependency; run each error assertion in a fresh VM.

The implementation extracts every substring key once, then moves values and
keys together through an in-place binary heap. Runtime is O(n log n), auxiliary
key storage is O(n), and comparisons allocate no substrings. It performs no
general selector calls in the sort core. There is no Level C BIF or class method
named ARRAYSORT.

`ts_arraysort.crexx` covers valid orders, substring keys, lexical numeric text,
Unicode, empty/single arrays, and return state. `ts_arraysort_errors.crexx`
covers each signal in a separate process.
