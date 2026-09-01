# Level C `COUNTSTR`

The standalone direct entry point is:

```rexx
rexxclassicbif_countstr(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `COUNTSTR(needle, haystack)`, returning the number of
case-sensitive, non-overlapping matches in the original haystack. Null,
oversized, and absent needles return `0`. BYTE is the default and searches
exact octet sequences; UTF8 is opt-in and searches by Unicode codepoint
position.

The CheckArgs contract is `rANY rANY`. It reports standard context errors
`RXC-LC-40.3`, `40.4`, and `40.5` for missing, extra, or omitted required
arguments. BYTE accepts arbitrary RexxValue bytes; UTF8 requires valid text.

The implementation caches both configured-unit lengths and advances a direct
search by the full needle length after every match. It allocates no substring.

`lib/rxfnsc/tests_functional/testRexxClassicBifCountstr.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers empty, absent,
oversized, boundary, non-overlap, Unicode, substantial, and all
argument-presence behavior without invoking the compatibility dispatcher.
