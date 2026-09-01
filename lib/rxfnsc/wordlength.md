# Level C `WORDLENGTH`

The standalone direct entry point is:

```rexx
rexxclassicbif_wordlength(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `WORDLENGTH(string, n)`, returning the active-profile character
length of the positive one-based word, or `0` when absent. BYTE counts exact
bytes and uses space/configured byte blanks. UTF8 counts codepoints and uses
Unicode 17.0.0 `White_Space` plus configured codepoint blanks; its default path
uses the VM scanner.

The CheckArgs contract is `rANY rWHOLE>0`. Standard context errors are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole word number; and
- `RXC-LC-40.14` for a nonpositive word number.

The implementation scans only as far as the requested word and subtracts its
source positions. It allocates no substring, has no controller body, and does
not rely on compiler lowering.

`lib/rxfnsc/tests_functional/testRexxClassicBifWordlength.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers BYTE/UTF8 lengths,
configured blanks, absent-word boundaries, source preservation, and errors.
