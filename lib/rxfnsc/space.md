# Level C `SPACE`

The standalone direct entry point is:

```rexx
.rexxclassicbifspace..rexxclassicbif_space(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `SPACE(string [,count [,pad]])`. Leading and trailing Unicode
whitespace is removed, whitespace runs delimit words, and the words are joined
by `count` copies of `pad`. The default count is one, a zero count removes
inter-word spacing, and the default pad is blank.

The CheckArgs contract is `rANY oWHOLE>=0 oPAD`. Standard context errors are
`RXC-LC-40.3`, `40.4`, and `40.5` for argument count/presence,
`RXC-LC-40.12` for a non-whole count, `RXC-LC-40.13` for a negative count, and
`RXC-LC-40.23` for an invalid pad.

The implementation extracts RexxValue text once, scans it once with direct VM
Unicode blank/nonblank operations, and appends word slices directly. Its
separator is built once and only if needed. It calls neither the name dispatcher
nor the Level B `SPACE`, `WORDS`, `WORD`, or `COPIES` helpers.

`lib/rxfnsc/tests_functional/testRexxClassicBifSpace.crexx` calls the qualified
standalone entry in optimized and unoptimized overlays and covers documented,
optional-hole, empty/all-whitespace, Unicode, substantial, non-mutation, lazy
separator, and standard argument-error behavior.
