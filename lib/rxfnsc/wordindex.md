# Level C `WORDINDEX`

The standalone direct entry point is:

```rexx
rexxclassicbif_wordindex(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `WORDINDEX(string, n)`. `n` is a positive one-based word number.
The result is the one-based Unicode character position of that word, or `0`
when the word does not exist. The direct implementation uses the VM's Unicode
whitespace classification.

The CheckArgs contract is `rANY rWHOLE>0`. Standard context errors are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole word number; and
- `RXC-LC-40.14` for a nonpositive word number.

The implementation scans only as far as the requested word and allocates no
substring. It has no compatibility-controller body and the direct harness does
not rely on compiler lowering.

Supporting a configured `Config_OtherBlankCharacters` set is tracked as the
shared dependency for the word-family Level C BIFs.

`lib/rxfnsc/tests_functional/testRexxClassicBifWordindex.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers position boundaries,
multiple and Unicode whitespace, absent words, empty input, non-mutation, and
all standard argument errors.
