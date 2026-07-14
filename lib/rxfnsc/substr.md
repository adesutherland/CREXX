# Level C `SUBSTR`

The standalone direct entry point is:

```rexx
.rexxclassicbifsubstr..rexxclassicbif_substr(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `SUBSTR(string, start [,length [,pad]])` using 1-based Unicode
codepoint positions. With length omitted, the result runs through the end or is
empty when start is beyond it. A supplied length fixes the result width and
pads short input on the right; zero returns empty. Pad defaults to blank and a
supplied pad must contain one codepoint.

The CheckArgs contract is `rANY rWHOLE>0 oWHOLE>=0 oPAD`. Standard context
errors are `RXC-LC-40.3`, `40.4`, and `40.5` for argument count/presence,
`RXC-LC-40.12` for non-whole start/length, `RXC-LC-40.14` for non-positive
start, `RXC-LC-40.13` for negative length, and `RXC-LC-40.23` for invalid pad.

The implementation extracts RexxValue text once, caches its codepoint length,
takes at most one direct substring, and appends any padding directly. It calls
neither the name dispatcher nor the Level B LENGTH/SUBSTR/COPIES helpers.

`lib/rxfnsc/tests_functional/testRexxClassicBifSubstr.crexx` calls the qualified
standalone entry in optimized and unoptimized overlays and covers documented,
optional-hole, beyond-end, zero/equal, Unicode, substantial, non-mutation, and
standard argument-error behavior.
