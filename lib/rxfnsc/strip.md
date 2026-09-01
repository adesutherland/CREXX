# Level C `STRIP`

The standalone direct entry point is:

```rexx
.rexxclassicbifstrip..rexxclassicbif_strip(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `STRIP(string [,option [,char]])`. The first codepoint of option
selects leading (`L`), trailing (`T`), or both (`B`) removal, case-insensitively;
the default is both. Under default BYTE, omission removes ASCII space and
configured blank octets. Under opt-in UTF8, omission removes Unicode 17.0.0
`White_Space` and configured blank codepoints. A supplied char removes only one
configured unit, so an explicit blank is distinct from omission. BYTE results
remain binary-authoritative.

The CheckArgs contract is `rANY oLTB oPAD`. Standard context errors are
`RXC-LC-40.3`, `40.4`, and `40.5` for argument count/presence,
`RXC-LC-40.21` for an empty option, `RXC-LC-40.28` for an invalid option, and
`RXC-LC-40.23` for an invalid char.

The implementation computes one start/end slice using shared configured blank
scans or direct unit comparisons. It performs at most one result slice and calls
neither the name dispatcher nor the Level B STRIP/SUBSTR helpers.

`lib/rxfnsc/tests_functional/testRexxClassicBifStrip.crexx` calls the qualified
standalone entry in optimized and unoptimized overlays and covers documented,
optional-hole, omitted-versus-explicit blank, Unicode, all-trimmed, substantial,
non-mutation, and standard argument-error behavior.
