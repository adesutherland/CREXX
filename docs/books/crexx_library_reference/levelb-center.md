## center

`center` pads or centrally truncates text to an exact character width:

```rexx
center(string = .string, width = .int, pad = " ") = .string
```

- `width` must be non-negative.
- `pad` must contain exactly one Unicode codepoint.
- When padding is uneven, the extra pad character is placed on the right.
- When truncation is uneven, the extra removed character comes from the right;
  equivalently, the retained central slice is biased one character left.

Invalid width or pad input raises `INVALID_ARGUMENTS`.

```rexx
center("ABC", 7)           /* "  ABC  " */
center("ABC", 8, "-")      /* "--ABC---" */
center("The blue sky", 8)  /* "e blue s" */
center("anything", 0)      /* "" */
```

The source argument is read-only. The implementation computes character
lengths once, uses VM cursor/subslice operations for truncation, and creates one
half-padding string which is appended on both sides.

`lib/rxfnsb/tests_functional/ts_center.crexx` is CENTER-only; the separate
CENTRE alias is reviewed and tested in its own selector row.
