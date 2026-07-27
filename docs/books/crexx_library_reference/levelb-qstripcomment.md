## Level B `qstripcomment`

`qstripcomment(open=.string [,close=.string], text=.string) = .string` removes
comments outside quotes. Omitted or empty `close` selects line comments and
preserves CRLF, LF, and CR line endings exactly. A non-empty `close` selects
nested balanced block comments. See [the shared quote-aware contract](quote-aware.md).
