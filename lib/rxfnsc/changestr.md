# Level C `CHANGESTR`

The standalone direct entry point is:

```rexx
rexxclassicbif_changestr(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `CHANGESTR(needle, haystack, replacement)`. All
case-sensitive, non-overlapping matches in the original haystack are replaced
from left to right. Inserted text is not searched. A null needle or absent
needle returns the original haystack; a null replacement deletes matches.

The CheckArgs contract is `rANY rANY rANY`. It reports standard context errors
`RXC-LC-40.3`, `40.4`, and `40.5` for missing, extra, or omitted required
arguments. The three Classic arguments otherwise accept any RexxValue text.

The implementation uses direct VM search, cursor, substring, and append
operations. It scans the original haystack and grows one result instead of
rebuilding and re-searching the modified text after each match.

`lib/rxfnsc/tests_functional/testRexxClassicBifChangestr.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers ordinary and repeated
replacement, deletion, expansion, non-overlap, empty inputs, Unicode, and all
argument-presence errors without invoking the compatibility dispatcher.
