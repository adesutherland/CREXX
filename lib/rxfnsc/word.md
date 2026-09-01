# Level C `WORD`

The standalone direct entry point is:

```rexx
rexxclassicbif_word(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `WORD(string, n)`. `n` is a positive one-based word number. The
requested word is returned, or the empty string when it does not exist. BYTE
uses exact bytes and space plus configured blank bytes. UTF8 uses codepoints
and Unicode 17.0.0 `White_Space` plus configured blank codepoints, retaining
the VM fast path when no additions are configured.

The CheckArgs contract is `rANY rWHOLE>0`. Standard context errors are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole word number; and
- `RXC-LC-40.14` for a nonpositive word number.

The implementation scans only as far as the requested word and extracts that
word once. It does not call the Level B selector or dispatch through the
compatibility controller. The old common body remains deprecated only for
current generated artifacts.

`lib/rxfnsc/tests_functional/testRexxClassicBifWord.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers boundaries, multiple
default/configured blanks, arbitrary BYTE data, UTF8 rejection, absent words,
empty input, source non-mutation, and all standard argument errors.
