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

## External pools

When argument 3 is supplied, the ANSI contract changes argument 1 to `rANY`
and routes both lookup and assignment through a named
`RexxExternalValuePool` adapter registered on `RexxClassicConfig`. Pool names
are trimmed and case-insensitive. The subject text is passed to the selected
adapter unchanged.

External assignment always performs a successful get before set and returns
that old value. A missing subject is not implicitly created. A missing or
rejected subject reports `RXC-LC-40.36`; an unknown or blank pool name reports
`RXC-LC-40.37`. `RexxInMemoryExternalValuePool` is the exact-subject adapter
used by direct harnesses and embedded users. Host/SAA adapters can implement
the same two-method interface without changing VALUE.

The direct harness
`lib/rxfnsc/tests_functional/testRexxClassicBifValue.crexx` covers scalars,
missing values, old-value assignment, empty values, reserved variables, stems,
derived and empty compound components, stem defaults, argument errors,
successful external get/set, pool-name normalization, unchanged subject text,
get-before-set, no implicit creation, adapter rejection, `40.36`, and `40.37`.
It calls `rexxclassicbif_value` directly and does not use the deprecated name
dispatcher.
