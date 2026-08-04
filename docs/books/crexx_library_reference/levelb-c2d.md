## c2d

```rexx
c2d(from = .string) = .int
```

The Level B `c2d` helper returns the native integer Unicode code point of
exactly one character. Its typed integer result is suitable for comparisons,
indexes, byte/code-point classification, and `d2c` round trips without a
string-to-number conversion.

```rexx
c2d("M")    /* 77 */
c2d("α")    /* 945 */
c2d("🔥")   /* 128293 */
c2d("00"x) /* 0 */
```

An empty or multi-character string raises `CONVERSION_ERROR`. The
implementation computes the Unicode character length once and extracts the
single code point directly.

This is not the Classic Level C C2D contract. Level C accepts an optional
length and interprets configuration-coded characters as a signed
twos-complement value. That separate contract is documented in
[`lib/rxfnsc/c2d.md`](../../rxfnsc/c2d.md).

The focused optimized/unoptimized harness is
`lib/rxfnsb/tests_functional/ts_c2d.crexx`.
