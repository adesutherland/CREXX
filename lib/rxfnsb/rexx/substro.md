## Level B `substro`

`substro` is a cREXX-specific alternate name with the same Level B slicing
contract as `substr`:

```rexx
substro(string = .string, start = .int, length = 0, pad = " ") = .string
```

`start` is required and must be positive. When `length` is omitted, the result
runs through the end of the source, or is empty when `start` is beyond it. A
supplied `length` is an `.int` and must be non-negative; the result has exactly
that many codepoints and is padded on the right when the source is short. `pad`
defaults to blank and must contain exactly one codepoint. Invalid start, supplied
length, or pad input raises `INVALID_ARGUMENTS`.

```rexx
substro("abc", 2)          /* "bc" */
substro("abc", 2, 4)       /* "bc  " */
substro("abc", 5, 4)       /* "    " */
substro("abc", 2, 6, ".")  /* "bc...." */
```

The source is read-only. The implementation caches its character length, takes
at most one direct VM substring, and appends padding directly with `padstr` only
when needed. It makes no Level B helper call. `SUBSTRO` is not in the repository
Level C BIF catalog and therefore has no Level C implementation.
