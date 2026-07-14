# Level C `VALUE`

The standalone Level C entry point is:

```rexx
rexxclassicbif_value(context = reference .RexxBifCallContext) = .RexxValue
```

It implements the Classic contract:

```rexx
VALUE(name [, newvalue [, pool]])
```

For the internal form, `name` must be a valid Classic variable symbol. The
function expands compound-variable tail components through the caller's
`RexxVariablePool`, returns the value in effect before the call, and assigns
`newvalue` when argument 2 is present. Missing scalars return their uppercase
name. Missing compound tails return their expanded name unless the stem has an
assigned default value.

Examples of internal behavior:

```text
MARY = 10             VALUE("mary")       -> 10
J = 3; A.3 = "three"  VALUE("A.J")        -> three
                      VALUE("new", "x")   -> NEW
```

The CheckArgs contract is `rSYM oANY oANY` for the internal form. Invalid
symbols report `RXC-LC-40.26`; ordinary missing, extra, and omitted arguments
use the shared `RXC-LC-40.*` errors.

## Parked external-pool dependency

When argument 3 is supplied, the ANSI contract changes argument 1 to `rANY`
and routes both lookup and assignment through a configuration-named external
pool. The repository does not yet provide that configuration service. The
direct entry point therefore validates `rANY oANY oANY` and reports
`RXC-LC-40.37` for every external pool name. Completing successful external
get/set behavior is parked on a concrete configuration-pool interface; it does
not require compiler lowering work.

The direct harness
`lib/rxfnsc/tests_functional/testRexxClassicBifValue.crexx` covers scalars,
missing values, old-value assignment, empty values, reserved variables, stems,
derived and empty compound components, stem defaults, argument errors, and the
parked external-pool error path. It calls `rexxclassicbif_value` directly and
does not use the deprecated name dispatcher.
