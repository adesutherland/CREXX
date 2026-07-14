# Level C `COMPARE`

The standalone direct entry point is:

```rexx
rexxclassicbif_compare(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `COMPARE(left, right [,pad])`. The result is the first 1-based
Unicode character position that differs after the shorter string is virtually
right-padded, or `0` when equal. The default pad is blank.

The CheckArgs contract is `rANY rANY oPAD`. Standard context errors are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count; and
- `RXC-LC-40.23` unless an explicit pad contains exactly one codepoint.

The implementation computes each character length once, compares the common
prefix directly, then compares only the longer tail with the cached pad
codepoint. It allocates no padded string.

`lib/rxfnsc/tests_functional/testRexxClassicBifCompare.crexx` calls this entry
directly in optimized and unoptimized overlays. It covers ordinary, empty,
both-direction, Unicode, pad, and standard-error behavior without invoking the
compatibility dispatcher.
