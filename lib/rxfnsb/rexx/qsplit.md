# Level B `qsplit`

`qsplit(text=.string, separator=.string) = .string[]` splits only outside
quotes. The separator must be non-empty. Results preserve source whitespace,
quote delimiters, adjacent empty fields, and a trailing empty field. See
[the shared quote-aware contract](quote-aware.md).
