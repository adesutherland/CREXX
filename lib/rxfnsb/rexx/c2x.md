# `c2x` (Level B)

```rexx
c2x(from = .string) = .string
```

`c2x` converts every source character to two uppercase hexadecimal digits.
The empty string returns the empty string; multi-character strings produce two
digits per character and leading zero digits are retained.

```rexx
c2x("M")        /* 4D */
c2x("72s")      /* 373273 on the ASCII/Unicode build */
c2x("0123"x)   /* 0123 */
c2x("")         /* empty */
```

The current Level B contract preserves the established RXAS `hexchar`
behavior: for a Unicode code point above the single-byte range it formats the
low eight bits. For example, `c2x("é")` is `E9`. Changing C2X to emit the
underlying UTF-8 bytes would be a separate language-surface decision and is not
part of this library-only implementation change.

The input is a typed `.string`, so there is no value-domain error. The
implementation scans once and appends each two-digit group directly rather than
re-concatenating the growing result.

The focused harness is `lib/rxfnsb/tests_functional/ts_c2x.crexx`. The separate
Classic Level C contract is in [`lib/rxfnsc/c2x.md`](../../rxfnsc/c2x.md).
