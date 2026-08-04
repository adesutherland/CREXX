## xrange

```rexx
xrange(from = .string, tos = .string) = .string
```

The native Level B `xrange` helper returns the inclusive byte-domain character
range from `from` through `tos`. Each endpoint must be exactly one Unicode
character from U+0000 through U+00FF. The range wraps from U+00FF to U+0000
when `from` is greater than `tos`, and therefore contains between one and 256
characters.

```rexx
xrange("a", "f")  /* "abcdef" */
```

An empty, multi-character, or out-of-domain endpoint raises
`INVALID_ARGUMENTS`. The implementation directly appends at most 256
codepoints and never prints a deprecation warning. It remains a legacy
byte-domain helper; use `sequence` for a non-wrapping Unicode range.

Classic Level C XRANGE is a distinct configuration-coded BIF documented in
[`lib/rxfnsc/xrange.md`](../../rxfnsc/xrange.md). The focused native harness is
`lib/rxfnsb/tests_functional/ts_xrange.crexx`.
