# Level C `SPACE`

The standalone direct entry point is:

```rexx
.rexxclassicbifspace..rexxclassicbif_space(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `SPACE(string [,count [,pad]])`. Under default BYTE, ASCII space
and configured blank octets delimit words. Under opt-in UTF8, Unicode 17.0.0
`White_Space` and configured blank codepoints delimit words. Leading/trailing
blanks are removed and words are joined by `count` copies of the configured-unit
`pad`. The default count is one, zero removes inter-word spacing, and the
default pad is blank. BYTE results remain binary-authoritative.

The CheckArgs contract is `rANY oWHOLE>=0 oPAD`. Standard context errors are
`RXC-LC-40.3`, `40.4`, and `40.5` for argument count/presence,
`RXC-LC-40.12` for a non-whole count, `RXC-LC-40.13` for a negative count, and
`RXC-LC-40.23` for an invalid pad.

The implementation scans once with shared configured blank operations and
appends word slices directly. Its separator is built once and only if needed.
It calls neither the name dispatcher nor the Level B helpers.

`lib/rxfnsc/tests_functional/testRexxClassicBifSpace.crexx` calls the qualified
standalone entry in optimized and unoptimized overlays and covers documented,
optional-hole, empty/all-whitespace, Unicode, substantial, non-mutation, lazy
separator, and standard argument-error behavior.
