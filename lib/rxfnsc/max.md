# Level C `MAX`

The standalone direct entry point is:

```rexx
.rexxclassicbifmax..rexxclassicbif_max(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements variadic `MAX(number, ...)`. At least one argument is required;
every supplied argument is validated and normalized by the repeated `rNUM`
rule, then compared as a RexxValue decimal under the active numeric context.
When values compare equal, the first argument remains selected, including its
normalized text representation.

Standard context errors are `RXC-LC-40.3` for no arguments,
`RXC-LC-40.5` for an omitted variadic argument, and `RXC-LC-40.11` for
nonnumeric text.

The implementation performs one validation/conversion pass and one linear
comparison pass. It calls neither the name controller nor the Level B MAX
helper and allocates no comparison substrings.

`lib/rxfnsc/tests_functional/testRexxClassicBifMax.crexx` calls the standalone
entry directly in optimized and unoptimized overlays. It covers one/many
arguments, signs, decimal precision, representation-preserving ties,
blank-separated signs, non-mutation, and standard errors.
