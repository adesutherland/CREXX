# Level C `COUNTSTR`

The standalone direct entry point is:

```rexx
rexxclassicbif_countstr(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `COUNTSTR(needle, haystack)`, returning the number of
case-sensitive, non-overlapping matches in the original haystack. Null,
oversized, and absent needles return `0`.

The CheckArgs contract is `rANY rANY`. It reports standard context errors
`RXC-LC-40.3`, `40.4`, and `40.5` for missing, extra, or omitted required
arguments. Both Classic arguments otherwise accept any RexxValue text.

The implementation caches both character lengths and advances a direct VM
search by the full needle length after every match. It allocates no substring.

`lib/rxfnsc/tests_functional/testRexxClassicBifCountstr.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers empty, absent,
oversized, boundary, non-overlap, Unicode, substantial, and all
argument-presence behavior without invoking the compatibility dispatcher.
