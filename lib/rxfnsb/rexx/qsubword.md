## Level B `qsubword`

`qsubword(text=.string, word_number=.int [,count=.int]) = .string` returns a
source span beginning at the positive word number. Omitted `count` selects
through the last word; explicit zero returns empty; a negative count signals
`INVALID_ARGUMENTS`. Original separators between selected words are preserved.
See [the shared quote-aware contract](quote-aware.md).
