## Level B `qsplitsafe`

`qsplitsafe(text=.string, separator=.string [,start=.int [,pairs=.string]]) =
.string[]` additionally suppresses splits inside nested one-codepoint delimiter
pairs. `pairs` is an opener/closer sequence such as `()[]`; the default is `()`.
The prefix before `start` stays in the first field. Invalid pair grammar signals
`INVALID_ARGUMENTS`. See [the shared quote-aware contract](quote-aware.md).
