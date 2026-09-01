## qextractall

`qextractall(open=.string, close=.string, text=.string [,start=.int
[,mode=.string]]) = .string[]` returns every balanced top-level span in source
order. `X`/`E` returns contents and `I`/`C` includes delimiters. It scans once
and selects spans by position. See [the shared quote-aware contract](quote-aware.md).
