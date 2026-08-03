## qextractpair

`qextractpair(open=.string, close=.string, text=.string [,start=.int
[,mode=.string]]) = .string` returns the first balanced top-level span. `X`/`E`
returns its contents; `I`/`C` includes delimiters. No opening pair returns an
empty string. Invalid mode or grammar signals `INVALID_ARGUMENTS`. See
[the shared quote-aware contract](quote-aware.md).
