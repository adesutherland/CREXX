# Level B `qwordlength`

`qwordlength(text=.string, word_number=.int) = .int` returns the codepoint length
of a positive one-based quote-aware word, including its quote delimiters. An
absent word returns `0`; an invalid word number or unmatched quote signals
`INVALID_ARGUMENTS`.

The implementation reads the selected span length from the shared scanner and
does not copy the word. There is no Level C BIF or class method named
QWORDLENGTH. See [the shared quote-aware contract](quote-aware.md).

`ts_qwordlength.crexx` covers quoted and unquoted lengths, Unicode whitespace,
missing input, non-mutation, and signals in both Level B build modes.
