## space

`space` normalizes whitespace-delimited words:

```rexx
space(string = .string, count = 1, pad = " ") = .string
```

`count` is an optional `.int` and must be non-negative. `pad` defaults to blank
and must contain exactly one Unicode codepoint. Leading and trailing Unicode
whitespace is removed, internal whitespace runs delimit words, and the words
are joined with exactly `count` copies of `pad`. A zero count joins the words
without a separator. Invalid count or pad input raises `INVALID_ARGUMENTS`.

```rexx
space("abc  def  ")        /* "abc def" */
space("  abc def ", 3)     /* "abc   def" */
space("abc  def  ", 0)     /* "abcdef" */
space("a　b", 1, "·")      /* "a·b" */
```

The source is read-only. The implementation scans it once with the VM Unicode
blank/nonblank instructions and appends each word slice directly. It constructs
the separator once with `padstr`, lazily when a second word needs it; there are
no Level B `WORDS`, `WORD`, or `COPIES` calls and no repeated result
concatenation.
