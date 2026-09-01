# Level C `SUBSTR`

The standalone direct entry point is:

```rexx
.rexxclassicbifsubstr..rexxclassicbif_substr(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `SUBSTR(string, start [,length [,pad]])` using 1-based configured
unit positions. BYTE is the default and uses exact octets; UTF8 is opt-in and
uses Unicode codepoints. With length omitted, the result runs through the end
or is empty when start is beyond it. A supplied length fixes the result width
and pads short input on the right; zero returns empty. Pad defaults to blank and
a supplied pad must contain one configured unit. BYTE results remain
binary-authoritative.

The CheckArgs contract is `rANY rWHOLE>0 oWHOLE>=0 oPAD`. Standard context
errors are `RXC-LC-40.3`, `40.4`, and `40.5` for argument count/presence,
`RXC-LC-40.12` for non-whole start/length, `RXC-LC-40.14` for non-positive
start, `RXC-LC-40.13` for negative length, and `RXC-LC-40.23` for invalid pad.

The implementation caches the configured-unit length, takes at most one direct
string or binary slice, and appends any padding directly. It calls neither the
name dispatcher nor the Level B helpers.

`lib/rxfnsc/tests_functional/testRexxClassicBifSubstr.crexx` calls the qualified
standalone entry in optimized and unoptimized overlays and covers documented,
optional-hole, beyond-end, zero/equal, Unicode, substantial, non-mutation, and
standard argument-error behavior.
