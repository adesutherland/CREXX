## Level B `qwordpos`

`qwordpos(search=.string, text=.string [,start=.int]) = .int` finds an exact
sequence of quote-aware words at or after the positive word number `start`.
Partial-word matches do not count. It returns `0` for an empty or absent phrase.
See [the shared quote-aware contract](quote-aware.md).
