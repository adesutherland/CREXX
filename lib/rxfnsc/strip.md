# Level C `STRIP`

The standalone direct entry point is:

```rexx
.rexxclassicbifstrip..rexxclassicbif_strip(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `STRIP(string [,option [,char]])`. The first codepoint of option
selects leading (`L`), trailing (`T`), or both (`B`) removal, case-insensitively;
the default is both. When char is omitted the configured Unicode whitespace set
is removed. A supplied char removes only that one codepoint, so an explicit
blank is distinct from omission for other Unicode whitespace.

The CheckArgs contract is `rANY oLTB oPAD`. Standard context errors are
`RXC-LC-40.3`, `40.4`, and `40.5` for argument count/presence,
`RXC-LC-40.21` for an empty option, `RXC-LC-40.28` for an invalid option, and
`RXC-LC-40.23` for an invalid char.

The implementation extracts RexxValue text once and computes one start/end
slice using direct Unicode blank scans or direct codepoint comparisons. It
performs at most one result substring and calls neither the name dispatcher nor
the Level B STRIP/SUBSTR helpers.

`lib/rxfnsc/tests_functional/testRexxClassicBifStrip.crexx` calls the qualified
standalone entry in optimized and unoptimized overlays and covers documented,
optional-hole, omitted-versus-explicit blank, Unicode, all-trimmed, substantial,
non-mutation, and standard argument-error behavior.
