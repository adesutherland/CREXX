## Level B quote-aware text contract

The `q*` string functions share one positional Unicode scanner. They are Level B
library functions, not classic Level C BIFs.

Single (`'`) and double (`"`) ASCII quotes can begin anywhere in a word. A
matching doubled quote represents an escaped quote. Quote delimiters remain part
of returned words and fields; an unmatched quote signals `INVALID_ARGUMENTS`.
Positions and lengths are Unicode codepoints. Word separation uses the Unicode
17.0 `White_Space` property, not grapheme boundaries.

Balanced-pair functions ignore delimiters inside quotes and support nested
pairs. Empty, identical, mismatched, or unclosed delimiters signal
`INVALID_ARGUMENTS`. Split and removal functions reconstruct results from source
spans, so repeated equal values, empty fields, whitespace, and line endings are
preserved exactly.

`ts_qpos.crexx`, `ts_qwordlength.crexx`, and `ts_qlibrary.crexx` run in optimized
and unoptimized Level B modes and cover the shared grammar, Unicode positions,
exact reconstruction, and signal paths.
