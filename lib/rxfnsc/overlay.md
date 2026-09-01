# Level C `OVERLAY`

The standalone direct entry point is:

```rexx
rexxclassicbif_overlay(context = reference .RexxBifCallContext) = .RexxValue
```

It implements `OVERLAY(new, target [,start [,length [,pad]]])`. `start` is a
positive 1-based configured-unit position and defaults to one. An omitted
`length` uses the configured-unit length of `new`; a supplied zero writes no
new content. BYTE is the default and operates on exact octets; UTF8 is opt-in
and operates on Unicode codepoints. A target shorter than the units preceding
`start` and an overlay shorter than `length` are extended with the one-unit pad,
which defaults to blank. BYTE results remain binary-authoritative.

The CheckArgs contract is `rANY rANY oWHOLE>0 oWHOLE>=0 oPAD`. Standard context
errors are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole start or length;
- `RXC-LC-40.14` for a non-positive start;
- `RXC-LC-40.13` for a negative length; and
- `RXC-LC-40.23` unless a supplied pad is exactly one configured unit.

The implementation caches configured-unit lengths and uses direct VM string or
binary slice, append, and pad operations. It calls no name dispatcher or Level
B formatting helper.

`lib/rxfnsc/tests_functional/testRexxClassicBifOverlay.crexx` covers defaults,
omitted optional slots, explicit zero, padding, truncation, Unicode, substantial
padding, and every argument-error class in optimized and unoptimized direct
overlays.
