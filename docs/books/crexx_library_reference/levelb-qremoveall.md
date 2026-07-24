## Level B `qremoveall`

`qremoveall(open=.string, close=.string, text=.string [,mode=.string]) =
.string` removes every balanced top-level span. `I`/`C` (the default) removes
delimiters and contents; `X`/`E` removes contents only. Positional reconstruction
handles repeated equal spans correctly. See [the shared quote-aware contract](quote-aware.md).
