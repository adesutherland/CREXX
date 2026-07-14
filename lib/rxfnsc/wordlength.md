# Level C `WORDLENGTH`

The standalone direct entry point is:

```rexx
rexxclassicbif_wordlength(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `WORDLENGTH(string, n)`, returning the Unicode character length
of the positive one-based word, or `0` when that word does not exist. The direct
implementation uses the VM's Unicode whitespace classification.

The CheckArgs contract is `rANY rWHOLE>0`. Standard context errors are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole word number; and
- `RXC-LC-40.14` for a nonpositive word number.

The implementation scans only as far as the requested word and subtracts its
source positions. It allocates no substring, has no controller body, and does
not rely on compiler lowering.

Supporting a configured `Config_OtherBlankCharacters` set is tracked as the
shared dependency for the word-family Level C BIFs.

`lib/rxfnsc/tests_functional/testRexxClassicBifWordlength.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers Unicode character
length, whitespace and absent-word boundaries, source non-mutation, and all
standard argument errors.
