## Level B `qword`

`qword(text=.string, word_number=.int) = .string` returns a positive one-based
quote-aware word, including its quote delimiters, or empty when absent. Quotes
may be attached to unquoted text. See [the shared quote-aware contract](quote-aware.md).
