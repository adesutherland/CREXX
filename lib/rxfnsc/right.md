# Level C `RIGHT`

The standalone direct entry point is:

```rexx
.rexxclassicbifright..rexxclassicbif_right(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `RIGHT(string, length [,pad])`. `length` is a required
non-negative whole number. Longer values are truncated to their rightmost
configured-unit width and shorter values are padded on the left. BYTE is the
default and uses exact octets; UTF8 is opt-in and uses Unicode codepoints. A
supplied pad must contain exactly one configured unit; the default is blank.
BYTE results remain binary-authoritative.

The CheckArgs contract is `rANY rWHOLE>=0 oPAD`. Standard context errors are
`RXC-LC-40.3`, `40.4`, and `40.5` for argument count/presence,
`RXC-LC-40.12` for a non-whole length, `RXC-LC-40.13` for a negative length,
and `RXC-LC-40.23` for an invalid pad.

The implementation caches the configured-unit length and uses direct VM string
or binary slice/pad/append operations. It calls neither the name dispatcher nor
the Level B RIGHT helper.

`lib/rxfnsc/tests_functional/testRexxClassicBifRight.crexx` uses the qualified
standalone entry in optimized and unoptimized overlays and covers documented,
zero/equal, empty, Unicode, substantial-padding, non-mutation, and
argument-error behavior.
