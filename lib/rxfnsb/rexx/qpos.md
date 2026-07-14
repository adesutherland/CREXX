# Level B `qpos`

`qpos(needle=.string, text=.string [,start=.int]) = .int` returns the first
one-based codepoint position of a non-empty `needle` outside quotes, or `0`.
`start` must be positive. Invalid arguments and unmatched quotes signal
`INVALID_ARGUMENTS`. See [the shared quote-aware contract](quote-aware.md).
