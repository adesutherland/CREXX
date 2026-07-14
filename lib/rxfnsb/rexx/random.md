# Level B `random`

The typed Level B function returns a pseudo-random `.int`:

```rexx
random([minimum = .int [, maximum = .int [, seed = .int]]]) = .int
```

With no arguments the range is `0..999`. One argument is the inclusive maximum
and therefore selects `0..maximum`. In the positioned three-argument form an
omitted minimum defaults to `0`, an omitted maximum defaults to `999`, and an
optional seed resets the module-scoped sequence before the value is drawn.

Arguments must be non-negative, the minimum must not exceed the maximum, and
their difference must not exceed `100000`. Violations signal
`INVALID_ARGUMENTS`.

The generator is deterministic Park-Miller state. Seed zero is normalized to
seed one; omitted seeding continues the current module sequence. Bounded values
use rejection sampling rather than `%` over a C-library `rand()` result, so the
range is unbiased. This generator is suitable for language-level repeatable
sequences, not cryptography.

The implementation has no VM-global RNG dependency. `RexxRandomState` is also
the state object used by the separately scoped Level C configuration service.
