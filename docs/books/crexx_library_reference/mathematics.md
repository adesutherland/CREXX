# Mathematics

The standard mathematics family separates binary floating-point, exact native
integer, and decimal-precision work. It is part of the normal Level G product;
it is not guaranteed by the minimal Level B bootstrap closure. The `rxint` and
`rxdecimal` modules are nevertheless authored in Level B, so installed callers
can use their signatures from Level B source.

## Binary float: `rxfloat`

Import `rxfloat` for scalar operations on the native `.float` type:

```rexx
options levelb
import rxfloat

hypotenuse = rxfloat..hypot(3.0, 4.0)
angle = rxfloat..atan2(1.0, 0.0)
```

`rxfloat` is a process-reentrant native provider backed by the host C maths
library. The compiler records the provider dependency in RXBIN metadata;
`rxvm` discovers the dynamic provider automatically, and `crexx -native`
selects the canonical static archive automatically. No Rexx wrapper or
remembered plugin argument is required.

The surface contains:

- trigonometric: `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`;
- hyperbolic: `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`;
- exponential and logarithmic: `exp`, `exp2`, `expm1`, `log`, `log2`,
  `log10`, `log1p`, `pow`, `pow10`;
- rounding and remainder: `ceil`, `floor`, `round`, `trunc`, `fmod`;
- roots and special functions: `sqrt`, `cbrt`, `hypot`, `erf`, `erfc`,
  `tgamma`, `lgamma`, `fabs`; and
- constants: `pi()` and `euler()`.

The same scalar procedures are published as direct `rxmath` compatibility
names by the `rxfloat` provider. The compatibility names do not load a second
library and add no Rexx call layer. New code should import `rxfloat`.
Historical `rxmath` statistics, hashes, UUID generation, and `inlinec` are not
part of this provider. RCC-5D supplies a separately loadable transitional
boxed-array `rxstats` surface. `BINARY-01` and RCC-5F will replace that
pre-release argument representation with aligned packed numeric storage.

Domain and range behaviour follows the platform C implementation: operations
such as a negative square root return IEEE NaN, poles can return infinity, and
signed zero is preserved where the corresponding C function specifies it.

The black-box float contract suite is
`lib/plugins/float/rxfloat_test.crexx`. Its typed assertions live in the pure
Level B `tests/support/numeric_test_support.crexx` module and are shared with
the integer and decimal suites. The helper is test machinery rather than a
source of expected values. The suite covers every canonical procedure, every
compatibility alias, and the representative IEEE/domain boundaries promised
above. The risk-weighted requirements for all three numeric families are in the
[RCC-5 mathematics validation strategy](../../planning/release-1/mathematics-validation-strategy.md).

## Native integer: `rxint`

`rxint` supplies checked algorithms over the signed native `.int` type:

```rexx
options levelb
import rxint

divisor = rxint..gcd(54, 24)
root = rxint..isqrt(9223372036854775807)
residue = rxint..powmod(2, 10, 1000)
```

| Procedure | Contract |
|---|---|
| `gcd(first, second)` | Non-negative greatest common divisor; `gcd(0,0)` is zero. |
| `lcm(first, second)` | Non-negative least common multiple with divide-before-multiply overflow control. |
| `isqrt(value)` | Floor of the square root of a non-negative integer. |
| `powmod(base, exponent, modulus)` | Normalized modular power using overflow-safe modular doubling. |
| `factorial(value)` | Exact factorial for values from 0 through 20. |

Invalid domains raise `INVALID_ARGUMENTS`. A mathematically valid result that
does not fit `.int` raises `OVERFLOW_UNDERFLOW`.

## Decimal: `rxdecimal`

`rxdecimal` operates directly on `.decimal`; it never converts through
binary `.float`:

```rexx
options levelb
import rxdecimal

calculate: procedure = .decimal
  numeric digits 32
  return rxdecimal..sqrt(2)
```

The initial surface is `sqrt`, `exp`, `ln`, `sin`, `cos`, `pi`, and `euler`.
Each public procedure inherits the caller's numeric context and rounds once
when returning to the caller. Work precision is 18 digits through caller 9,
32 through caller 18, 64 through caller 32, and 96 through caller 64. Wider
callers use their requested precision plus 32 guard digits; this is selected
dynamically and does not introduce another fixed ceiling. The surface is
qualified through a 128-digit caller context. That qualification boundary is
the tested assurance limit, not the implementation's runtime maximum.
Negative square-root arguments and non-positive logarithm arguments raise
`INVALID_ARGUMENTS`.

An ordinary dotted source literal is converted directly from its original
spelling when the surrounding typed context expects `.decimal`; it is not
first rounded to binary64. Explicit `.float(...)` conversion and `options
floats_binary` remain binary boundaries.

The exact-integer and decimal contract suites are
`lib/rxfnsg/tests_functional/ts_math_integer_contract.crexx` and
`ts_math_decimal_contract.crexx`. Their offline oracle commands, versions,
rounding policy, and reviewed SHA-256 identities are retained in
`math-reference-provenance.md` beside the suites.
