## Level B numeric-context accessors

The `numeric` selector exports five zero-argument Level B functions:

```rexx
digits()   = .int
fuzz()     = .int
form()     = .string
numcase()  = .string
standard() = .string
```

Each accessor reads the corresponding setting inherited from its immediate
caller. `digits()` returns the positive significant-digit count and `fuzz()`
returns the non-negative ignored-digit count. The string accessors return:

| Function | Values |
|---|---|
| `form()` | `scientific`, `engineering`, `unknown` |
| `numcase()` | `lower`, `upper`, `unknown` |
| `standard()` | `common`, `classic`, `unknown` |

```rexx
example: procedure
  numeric digits 12
  numeric fuzz 2
  numeric form engineering
  numeric case upper
  numeric standard classic
  say digits() standard() form() fuzz() numcase()
  /* 12 classic engineering 2 upper */
```

All five functions are constant-time native context reads. Integer accessors
allocate no string; the other accessors only select a constant result name.
They have no arguments and no value-dependent error branch.

`numcase()` and `standard()` are cREXX Level B extensions. The repository's
Level C catalog requires only the distinct `DIGITS()`, `FORM()`, and `FUZZ()`
BIFs; their RexxValue contract is documented in `lib/rxfnsc/numeric.md`.
