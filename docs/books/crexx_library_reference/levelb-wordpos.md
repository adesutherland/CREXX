## wordpos

```rexx
wordpos(phrase = .string, string = .string [, start = .int]) = .int
```

`wordpos` returns the positive one-based word number of the first exact word
sequence matching `phrase` at or after `start`, which defaults to `1`.
Whitespace at either edge is ignored and any nonempty whitespace run separates
words, so different run lengths compare equally. Word text remains case-
sensitive and must match exactly; prefixes are not matches.

Empty/blank-only phrases, empty targets, and absent phrases return `0`. Words
use the VM Unicode whitespace definition. A non-positive start signals
`INVALID_ARGUMENTS`.

The implementation scans word boundaries directly and compares word spans by
codepoint without allocating substrings or calling another selector. The
focused harness is `lib/rxfnsb/tests_functional/ts_wordpos.crexx`.
