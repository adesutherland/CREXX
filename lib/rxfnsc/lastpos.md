# Level C `LASTPOS`

The standalone direct entry point is:

```rexx
rexxclassicbif_lastpos(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `LASTPOS(needle, haystack [,start])`. When `start` is omitted,
the complete haystack is considered. A supplied start is a positive 1-based
final configured character position: a match is eligible only when it ends at
or before that position. BYTE is the default and positions exact octets; UTF8
is opt-in and positions Unicode codepoints. The returned RexxValue contains the
last eligible match's position, or `0` when none exists or the needle is null.

The CheckArgs contract is `rANY rANY oWHOLE>0`. Standard context errors are
`RXC-LC-40.3`, `40.4`, and `40.5` for argument count/presence,
`RXC-LC-40.12` for a non-whole start, and `RXC-LC-40.14` for a non-positive
start.

The VM has no reverse substring search. This direct implementation caches both
configured-unit lengths and scans monotonically, retaining the last eligible
(including overlapping) match. It calls no name dispatcher or Level B helper
and creates no substring.

`lib/rxfnsc/tests_functional/testRexxClassicBifLastpos.crexx` covers ordinary,
limited, empty, overlapping, Unicode, substantial-search, non-mutation, and
argument-error behavior in optimized and unoptimized direct overlays.
