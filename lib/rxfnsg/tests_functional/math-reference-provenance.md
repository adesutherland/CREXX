# RCC-5C mathematics reference provenance

Status: retained offline oracle evidence for the integer and decimal contract
suites.

The contract tests have no runtime dependency on Python, `bc`, a network
service, or the implementation under test. The tools below generated or
corroborated checked-in literals before the suites were run through `rxc`,
`rxas`, `rxlink`, and both VMs.

## Decimal vectors

The transcendental reference implementation is GNU `bc` 7.0.3, invoked with
its standard mathematics library and `scale=110`. This retains at least 46
fractional guard digits beyond the 64-digit caller context qualified by the
suite. The checked-in generator contains the exact inputs for constants,
roots, exponentials, logarithms, quadrants, reduction boundaries, and
multi-period angles.

Run it from the repository root with:

```sh
/usr/bin/bc -l lib/rxfnsg/tests_functional/math_decimal_reference_vectors.bc
```

`bc` truncates the generated high-precision values at the requested scale.
Approximate contract cases retain those guard digits and compare them with
explicit absolute-plus-relative tolerances. Exact `pi` and `euler` expectations
are independently rounded for every caller context by Python 3.14.6's decimal
module using `ROUND_HALF_UP`; none of the selected constant boundaries is a
tie.

Run the rounding step with:

```sh
python3 lib/rxfnsg/tests_functional/math_decimal_rounding.py
```

The public suite covers caller digits 9, 10, 18, 19, 32, 33, and 64. The 10,
19, and 33 cases straddle all internal work-tier changes. The implementation's
96-digit internal tier supplies 32 guard digits at the qualified 64-digit
caller ceiling.

## Integer vectors

Python 3.14.6 supplied the arbitrary-precision integer boundary values through
`math.gcd`, `math.lcm`, `math.isqrt`, `math.factorial`, and the built-in
three-argument modular `pow`. Run the retained generator with:

```sh
python3 lib/rxfnsg/tests_functional/math_integer_reference.py
```

The suite's bounded exhaustive grids do not call those Python routines at test
time. They use deliberately simple brute-divisor, brute-multiple, linear-root,
and direct-multiplication oracles whose shapes differ from the production
Euclidean, divide-before-multiply, binary-search, and modular-doubling
algorithms. Limit-adjacent multiplication values can also be checked directly
as exact integer products.

## Reviewed content identities

The following SHA-256 identities bind this provenance record to the reviewed
generators, suites, and shared typed comparator as of 2026-08-20:

```text
21a6327f3f0a89b38c295a1e2f460486887fc4268c66bd25cb3a8facf7e6270c  math_decimal_reference_vectors.bc
8e7186061524c7f84e592858230f9d4bc3ab8826e354fd46f1ca7e03cf7e0fdd  math_decimal_rounding.py
f4cc7c1c3146f04c544f9c15e7ad67821a458da2321081c573dea3f677434fdd  math_integer_reference.py
ce1e12f2b5c14eed63a485b5a90a94ca186ae4d8a32122b302d8789dbf4d94f0  ts_math_decimal_contract.crexx
7fa05892d5e7cb43209b5c94cae1c98b90772c4a099500323c842e2edf01226a  ts_math_integer_contract.crexx
ffdb923106b0837ff4970aa55b1335f962b453078c5a6608940fc80f8c86e7f6  numeric_test_support.crexx
```

Recalculate these identities with `shasum -a 256` whenever a generator,
checked-in vector, scenario, tolerance, or comparator changes, and update this
record in the same commit.
