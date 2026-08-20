# RCC-5 mathematics validation strategy

Status: approved strategy; RCC-5B float contract coverage is implemented and
RCC-5C integer/decimal expansion remains in progress.

Approved by Adrian: 2026-08-20.

## Purpose

This strategy defines the evidence required to show that the Level G
`rxfloat`, `rxint`, and `rxdecimal` mathematics surfaces implement their public
contracts. It deliberately separates three concerns:

1. `tests/support/numeric_test_support.crexx` supplies typed assertions and
   reports comparison failures. It is test machinery, not an oracle.
2. Each contract suite owns its inputs, independently derived expected values,
   tolerances, signal expectations, and public-surface calls.
3. This document owns the risk model, required scenarios, execution matrix,
   oracle provenance, and completion criteria.

The float provider delegates mathematical implementation to the host C maths
library. Its main cREXX risks are binding, signature, compatibility, provider
loading, and promised edge behaviour. The integer and decimal algorithms are
implemented by cREXX itself. They therefore need materially deeper algorithm,
boundary, and numerical coverage.

## Shared assertion tool

The test-only `numerictestsupport` namespace provides explicit typed checks:

- `check_bool(condition, label)`;
- `check_int_equal(actual, expected, label)`;
- `check_float_close(actual, expected, absolute_tolerance,
  relative_tolerance, label)`;
- float NaN, positive/negative infinity, and signed-zero checks;
- `check_decimal_equal(actual, expected, label)`; and
- `check_decimal_close(actual, expected, absolute_tolerance,
  relative_tolerance, label)`.

Integer results are exact. Float and approximate decimal cases state both
absolute and relative tolerances explicitly; zero is valid for either. Decimal
comparisons inherit the suite's numeric context. The support module contains no
function-specific expected result and must never import `rxfloat`, `rxint`, or
`rxdecimal`.

The module is compiled, assembled, linked, and executed through the normal
toolchain beside each owning suite. It is not installed as a product library.

## Expected-value independence and provenance

Expected results must not be calculated at runtime by the function under test
or by another cREXX routine sharing the same algorithm. Inverse and identity
checks are useful secondary properties, but cannot be the only oracle because
correlated errors can cancel.

- Integer expected values are exact checked-in literals or fixtures generated
  by an arbitrary-precision integer implementation.
- Decimal transcendental vectors are generated offline at least 32 guard
  digits beyond the 64-digit provider ceiling, then rounded independently for
  every tested caller context.
- Float finite values use independently established constants or
  higher-precision vectors rounded to binary64. The suite does not compare a C
  maths call with a second call to the same C maths routine.

Any generated fixture must record the generator implementation and version,
precision and rounding mode, exact command, and a content checksum. Generated
fixtures are reviewed and checked in; the test run has no external package or
network dependency. Particularly important decimal and integer boundary cases
should be corroborated by a second independent implementation or exact
mathematical derivation.

## Common contract coverage

Every public procedure requires:

- at least one independently expected ordinary result;
- every documented domain and signal boundary;
- zero, sign, and magnitude cases relevant to its contract;
- results on both sides of each internal algorithm-selection or reduction
  boundary that is observable through the public result;
- a structural guard that fails when an exported procedure has no contract
  case; and
- optimized and no-opt execution on `rxbvm` and `rxtvm` when both are built.

The suites must consume the public namespaces and normal RXBIN dependencies.
They must pass through `rxc`, `rxas`, `rxlink`, and `rxvm`; direct calls to a C
helper do not prove the library contract. Expected signals must verify both the
signal category and the operation that raised it.

## Binary-float contract: `rxfloat`

`lib/plugins/float/rxfloat_test.crexx` is the focused RCC-5B black-box contract
suite. The host C maths library is mature, so this suite validates the cREXX
surface and representative numerical behaviour rather than attempting to
requalify every host libm implementation exhaustively.

Required coverage is:

- one or more independent finite values for every canonical procedure;
- representative small, ordinary, negative, and scaled values where relevant;
- quadrants for inverse/two-argument trigonometry;
- cancellation-sensitive `expm1` and `log1p` inputs;
- rounding half cases, signed operands, and remainder sign;
- repaired two-argument and scaled `hypot` cases;
- NaN/domain, pole/infinity, propagation, and signed-zero behaviour promised by
  the public documentation;
- exact parity for every direct `rxmath` compatibility alias;
- nullary, unary, and binary native arity failures; and
- automatic provider discovery plus dynamic/static packaging evidence retained
  by RCC-5B qualification.

The source-surface guard requires every registered canonical procedure to have
both an expected-value call and a compatibility-alias comparison. This focused
contract coverage completes RCC-5B without claiming exhaustive libm accuracy.

## Exact-integer contract: `rxint`

`rxint` has moderate algorithmic risk and high boundary/overflow risk. All
results are exact; tolerances are prohibited.

### `gcd`

- exhaustive sign combinations over a bounded small grid, including zero;
- equal, coprime, shared-factor, and consecutive-Fibonacci inputs;
- `gcd(0, 0)`, `gcd(value, 0)`, and argument-order symmetry;
- inputs containing `INT64_MIN`, `INT64_MAX`, `-1`, and `1`; and
- the unrepresentable mathematical result `2**63` signal.

### `lcm`

- zero in either position and every input-sign combination;
- coprime and shared-factor values, with exact relation to independently known
  GCD values;
- representable products adjacent to the integer limit;
- cases proving divide-before-multiply avoids a false intermediate overflow;
  and
- positive and negative inputs whose exact LCM is unrepresentable.

### `isqrt`

- exhaustive values across a bounded initial range;
- `k**2 - 1`, `k**2`, and `k**2 + 1` around representative small, medium, and
  maximum representable roots;
- zero, one, `INT64_MAX`, and the `3037000499` upper-root boundary; and
- every negative-domain signal path.

### `powmod`

- a bounded cross-product of positive/negative bases, exponents, and moduli
  compared with an arbitrary-precision modular-power oracle;
- exponent zero, base zero, modulus one, and negative-base normalization;
- large bases/moduli that force repeated modular doubling and would overflow a
  direct intermediate multiplication;
- even and odd exponent paths; and
- negative exponent plus zero/negative modulus signals.

### `factorial`

- every exact result from 0 through 20;
- the identities for zero and one; and
- negative input and every value above 20 through representative large-input
  overflow signals.

## Decimal contract: `rxdecimal`

`rxdecimal` has the highest risk because cREXX owns the iterative algorithms,
range reduction, termination, work-context selection, and final rounding. The
primary oracle is independent high-precision data, not float conversion and not
an inverse call through another `rxdecimal` function.

All public procedures are tested at caller precisions 9, 18, 32, and 64. The
work-context switching boundaries additionally require cases at 10 and 19
digits. Expected values are supplied as decimal strings so they never pass
through binary float.

### Constants and context

- independently rounded `pi()` and `euler()` strings at every context;
- proof that the result observes the caller's digits and is rounded once on
  return; and
- adjacent rounding cases where the first discarded digit is below, equal to,
  and above five when such cases can be selected from the function vectors.

### `sqrt`

- zero, one, exact integer/decimal squares, irrational roots, and sub-unit
  values;
- very small and large decimal exponents within the provider range;
- values immediately below and above exact squares;
- inputs below and above the initial-estimate branch at one; and
- negative-domain signalling plus convergence at every caller precision.

### `exp`

- zero, small positive/negative cancellation-sensitive values, `1`, `-1`, and
  ordinary positive/negative arguments;
- values around each halving threshold, including `0.5` and its neighbours;
- large finite results, small reciprocal results, and the documented
  unsupported-range signal boundary; and
- independent results at every caller precision.

### `ln`

- one, independently supplied `e`, values just below/above one, powers of ten,
  and non-power ordinary values;
- values around both reduction-band boundaries `0.75` and `1.5`;
- very small and large positive magnitudes;
- zero and negative-domain signals; and
- independent results at every caller precision.

### `sin` and `cos`

- zero, positive/negative ordinary angles, and independently supplied values at
  common fractions of pi;
- every quadrant and values immediately on both sides of `pi/2`, `pi`, and the
  period boundary;
- positive and negative multi-period inputs, including magnitudes large enough
  to stress reduction without exceeding the documented precision contract;
- near-zero results where absolute tolerance dominates and near-unit results
  where relative tolerance is meaningful; and
- odd/even symmetry and `sin(x)**2 + cos(x)**2` as secondary properties only.

Every iterative decimal routine must also have a bounded non-convergence or
range failure test where that failure is part of its public contract. A hang or
silent low-precision result is never an acceptable outcome.

## Suite layout and completion gates

- `lib/plugins/float/rxfloat_test.crexx` remains the RCC-5B float contract
  suite.
- `lib/rxfnsg/tests_functional/ts_math_numeric.crexx` is the current RCC-5C
  integration smoke suite and exercises the shared integer/decimal assertions.
- The deeper integer and decimal cases defined above may be split into focused
  source files for maintainability, but their CTest labels and structural
  guards must make omitted procedures or contexts visible.

RCC-5C is complete only when the integer and decimal scenario sets above pass
in the four optimized/no-opt and concrete-VM cells, their independent fixture
provenance is retained, and every exported procedure is structurally covered.
The one consolidated broad CTest/sanitizer/install closeout remains an
end-of-RCC-5 activity; this strategy does not require rerunning it after each
focused test increment.
