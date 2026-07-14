# Level C `SUBWORD`

The standalone direct entry point is:

```rexx
rexxclassicbif_subword(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `SUBWORD(string, start [,count])`. Omitted count selects through
the final word, explicit zero returns empty, and an oversized count clamps to
the available words. The result excludes leading and trailing whitespace while
preserving all whitespace between selected words. BYTE uses byte positions and
space plus configured blank bytes. UTF8 uses codepoints and Unicode 17.0.0
`White_Space` plus configured blank codepoints, retaining the VM fast path when
no additions are configured.

The CheckArgs contract is `rANY rWHOLE>0 oWHOLE>=0`. Standard context errors
are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole start or count;
- `RXC-LC-40.14` for nonpositive start; and
- `RXC-LC-40.13` for negative count.

The implementation scans once to identify one source span and extracts it once.
It has no controller body and the direct harness does not rely on lowering.

`lib/rxfnsc/tests_functional/testRexxClassicBifSubword.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers the documented
examples, optional and boundary counts, blank preservation/trimming,
BYTE/UTF8/configured blanks, source non-mutation, and every standard error.
