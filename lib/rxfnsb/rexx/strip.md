# Level B `strip`

`strip` removes leading, trailing, or both runs of unwanted codepoints:

```rexx
strip(string = .string, option = "B", char = " ") = .string
```

Only the first codepoint of `option` is significant, case-insensitively: `L`
removes a leading run, `T` removes a trailing run, and `B` removes both. An
invalid or empty option raises `INVALID_ARGUMENTS`.

When `char` is omitted, the configured Unicode whitespace set is removed. When
it is supplied, it must contain exactly one codepoint and only that codepoint is
removed; consequently, an explicitly supplied blank differs from omission for
other Unicode whitespace. An invalid supplied char raises `INVALID_ARGUMENTS`.

```rexx
strip("  ab c  ")         /* "ab c" */
strip("  ab c  ", "L")    /* "ab c  " */
strip("12.7000",, "0")    /* "12.7" */
strip("　ab c　")          /* "ab c" */
strip(" x　",, " ")       /* "x　" */
```

The source is read-only. The implementation calculates one start/end slice,
using direct VM Unicode blank scans when `char` is omitted and direct codepoint
comparisons when it is supplied. It performs at most one result substring and
makes no Level B helper call.
