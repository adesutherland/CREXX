# `d2b` (Level B)

```rexx
d2b(decimal = .int) = .string
```

`d2b` converts a non-negative native integer to minimal binary digit text.
There are no leading zeroes, except that zero itself returns `"0"`.

```rexx
d2b(0)   /* "0" */
d2b(9)   /* "1001" */
d2b(19)  /* "10011" */
d2b(129) /* "10000001" */
```

A negative input raises `INVALID_ARGUMENTS`. This native helper has no width
argument and therefore has no signed twos-complement form. Callers needing a
fixed-width encoded representation should use the appropriate typed binary
memory operation rather than relying on implicit truncation.

The implementation counts the significant bits with logical shifts, then emits
them directly from most significant to least significant. It does not call
`d2x` or `x2b`, allocate an intermediate representation, or inherit those
functions' grouping and diagnostic contracts.

There is no same-named Classic Level C BIF in the repository recognition
ledger. The focused optimized/unoptimized harness is
`lib/rxfnsb/tests_functional/ts_d2b.crexx`.
