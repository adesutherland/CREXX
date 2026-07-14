# Level C `COMPARE`

The standalone direct entry point is:

```rexx
rexxclassicbif_compare(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `COMPARE(left, right [,pad])`. The result is the first 1-based
configured character position that differs after the shorter value is
virtually right-padded, or `0` when equal. BYTE is the default and compares
exact octets; UTF8 is opt-in and compares Unicode codepoints. The default pad
is blank.

The CheckArgs contract is `rANY rANY oPAD`. Standard context errors are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count; and
- `RXC-LC-40.23` unless an explicit pad contains one configured unit.

The implementation computes each character length once, compares the common
prefix directly, then compares only the longer tail with the cached pad
codepoint. It allocates no padded string.

`lib/rxfnsc/tests_functional/testRexxClassicBifCompare.crexx` calls this entry
directly in optimized and unoptimized overlays. It covers ordinary, empty,
both-direction, Unicode, pad, and standard-error behavior without invoking the
compatibility dispatcher.
