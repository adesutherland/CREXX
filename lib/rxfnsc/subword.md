# Level C `SUBWORD`

The standalone direct entry point is:

```rexx
rexxclassicbif_subword(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `SUBWORD(string, start [,count])`. Omitted count selects through
the final word, explicit zero returns empty, and an oversized count clamps to
the available words. The result excludes leading and trailing whitespace while
preserving all whitespace between selected words. The direct implementation
uses the VM's Unicode whitespace classification.

The CheckArgs contract is `rANY rWHOLE>0 oWHOLE>=0`. Standard context errors
are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole start or count;
- `RXC-LC-40.14` for nonpositive start; and
- `RXC-LC-40.13` for negative count.

The implementation scans once to identify one source span and extracts it once.
It has no controller body and the direct harness does not rely on lowering.

Supporting a configured `Config_OtherBlankCharacters` set is tracked as the
shared dependency for the word-family Level C BIFs.

`lib/rxfnsc/tests_functional/testRexxClassicBifSubword.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers the documented
examples, optional and boundary counts, blank preservation/trimming, Unicode,
source non-mutation, and every standard argument error.
