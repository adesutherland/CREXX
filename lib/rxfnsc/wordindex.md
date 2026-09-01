# Level C `WORDINDEX`

The standalone direct entry point is:

```rexx
rexxclassicbif_wordindex(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `WORDINDEX(string, n)`. `n` is a positive one-based word number.
The result is the one-based active-profile character position of that word, or
`0` when it does not exist. BYTE positions are bytes with space/configured byte
blanks. UTF8 positions are codepoints with Unicode 17.0.0 `White_Space` and
configured codepoint blanks; the default path uses the VM scanner.

The CheckArgs contract is `rANY rWHOLE>0`. Standard context errors are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole word number; and
- `RXC-LC-40.14` for a nonpositive word number.

The implementation scans only as far as the requested word and allocates no
substring. It has no compatibility-controller body and the direct harness does
not rely on compiler lowering.

`lib/rxfnsc/tests_functional/testRexxClassicBifWordindex.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers position boundaries,
BYTE/UTF8/default/configured blanks, absent words, empty input, non-mutation,
and all standard argument errors.
