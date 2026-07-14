# Level C `POS`

The standalone direct entry point is:

```rexx
.rexxclassicbifpos..rexxclassicbif_pos(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `POS(needle, haystack [,start])`. `start` defaults to one and is a
positive 1-based character position. The returned RexxValue contains the first
matching position at or after `start`, or `0` when there is no match. A null
needle or haystack returns `0`.

The CheckArgs contract is `rANY rANY oWHOLE>0`. Standard context errors are
`RXC-LC-40.3`, `40.4`, and `40.5` for argument count/presence,
`RXC-LC-40.12` for a non-whole start, and `RXC-LC-40.14` for a non-positive
start.

The implementation extracts RexxValue text once, performs direct null checks,
and executes one VM `strpos` search. It calls neither the name dispatcher nor
the Level B POS helper.

`lib/rxfnsc/tests_functional/testRexxClassicBifPos.crexx` uses the qualified
standalone entry in optimized and unoptimized overlays and covers documented,
empty, boundary, Unicode, substantial-search, non-mutation, and argument-error
behavior.
