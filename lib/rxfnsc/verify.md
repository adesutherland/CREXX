# Level C `VERIFY`

The standalone direct entry point is:

```rexx
.rexxclassicbifverify..rexxclassicbif_verify(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `VERIFY(string, reference [,option [,start]])`. `option` defaults
to `N`; its first character is case-insensitive and may be `N` (find the first
character absent from `reference`) or `M` (find the first character present).
`start` defaults to one and is a positive 1-based character position. The
returned RexxValue contains the first qualifying position or `0`.

The CheckArgs contract is `rANY rANY oMN oWHOLE>0`. Standard context errors are
`RXC-LC-40.3`, `40.4`, and `40.5` for argument count/presence,
`RXC-LC-40.21`/`40.28` for an empty/invalid option, and
`RXC-LC-40.12`/`40.14` for a non-whole/non-positive start.

The implementation extracts RexxValue text once, caches both character
lengths, and uses one native VM character-table scan for each inspected source
character. It creates no substring or per-character text and calls neither the
name controller nor the Level B VERIFY helper.

`lib/rxfnsc/tests_functional/testRexxClassicBifVerify.crexx` calls this entry
directly in optimized and unoptimized overlays. It covers the documented,
empty, boundary, Unicode, NUL, substantial-input, non-mutation, optional-hole,
and standard argument-error behavior.
