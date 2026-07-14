# Level C `WORDPOS`

The standalone direct entry point is:

```rexx
rexxclassicbif_wordpos(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `WORDPOS(phrase, string [,start])`. The result is the one-based
word number of the first exact phrase at or after positive `start`, defaulting
to `1`, or `0` when absent. Outer whitespace is ignored and nonempty whitespace
runs are equivalent separators. Word text is case-sensitive and exact; a word
prefix is not a match. Empty or blank-only phrases return `0`.

The CheckArgs contract is `rANY rANY oWHOLE>0`. Standard context errors are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole start; and
- `RXC-LC-40.14` for a nonpositive start.

The implementation scans word boundaries and compares source spans in the
active profile: exact bytes for BYTE, codepoints for UTF8. Both include their
configured blank sets; UTF8 retains the VM fast path with no additions. It
allocates no substrings and does not rely on compiler lowering.

`lib/rxfnsc/tests_functional/testRexxClassicBifWordpos.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers exact/prefix/case
behavior, BYTE/UTF8/configured blanks, start/duplicate matches, empty inputs,
non-mutation, omission, and all standard argument errors.
