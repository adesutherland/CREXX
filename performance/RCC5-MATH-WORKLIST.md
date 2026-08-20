# RCC-5A/B/C mathematics composition worklist

Status: RCC-5A and focused RCC-5B are complete; the RCC-5B first Release
verdict is accepted. Consolidated full QA remains deferred to the end of
RCC-5. RCC-5C remains in progress; RCC-5D+ is not started.

Approved by Adrian: 2026-08-20.

## Scope and stop boundary

This work may:

- document the Level B bootstrap and Level G availability distinction;
- split scalar float mathematics from the historical `rxmath` bundle;
- publish automatic dynamic/static provider forms and compatibility names;
- add Level-B-authored integer and decimal mathematics to the Level G standard
  library; and
- replace defective, dormant, or numerically weak implementations with known
  appropriate algorithms and focused correctness coverage.

It does not implement `BINARY-01`, promote the current statistics surface,
retire an RXAS opcode, change numeric types or contexts, or alter RXBIN format.
After the first production performance edit, minimum correctness is followed
immediately by the mandatory profiling-off Release verdict and a stop for
Adrian's direction.

Full broad QA, sanitizer sweeps, install/package qualification, and consolidated
documentation closeout run once at the end of RCC-5, not after every RCC-5
subphase. Each subphase still requires focused correctness proportional to its
surface and any mandatory first-Release performance verdict. The extensive
RCC-5B checks already completed remain useful retained evidence, but do not set
a precedent for repeating full QA after RCC-5C or another intermediate slice.

## Availability and implementation decision

The public mathematics family is guaranteed by the normal Level G product, not
the Level B bootstrap closure. A member may nevertheless be authored in Level
B and callable from Level B source when installed. Such a caller is Level-B
compatible but no longer bootstrap-self-contained.

| Surface | Implementation | Availability |
|---|---|---|
| primitive `.int`, `.float`, `.decimal` arithmetic and `mc_decimal` | VM/runtime | Level B bootstrap core |
| `rxfloat` | process-reentrant native libm provider `rxfloat` | Level G standard |
| `rxint` | Level B algorithms | Level G standard |
| `rxdecimal` | Level B algorithms over `mc_decimal` | Level G standard |
| `rxstats` | later native bulk provider | Level G standard after `BINARY-01` |

## Production design selection

### Scalar float mathematics

**Selected:** extract one process-reentrant `rxfloat` provider, publish the
canonical `rxfloat` namespace, and register compatibility `rxmath` names from
the same native procedures. Use the platform C mathematical library rather
than handwritten approximations. Repair the current two-argument `hypot`
defect, add numerically stable standard primitives where the C library provides
them (`atan2`, `expm1`, and `log1p`), and test finite, signed-zero, infinity,
NaN and documented domain behavior.

Rejected alternatives:

1. Keep the broad historical provider: this retains unrelated statistics,
   hashes, UUID generation and executable-code tooling under one identity.
2. Implement binary64 transcendental functions in Level B: it duplicates
   mature platform libm work and adds accuracy, range-reduction and performance
   risk without a product requirement.
3. Add VM opcodes for scalar libm calls: normal typed provider calls already
   supply signatures, loading and native packaging.

### Integer mathematics

**Selected:** use Level B algorithms over checked native `.int` operations.
The initial family uses the Euclidean algorithm for `gcd`, divide-before-
multiply for `lcm`, overflow-safe binary search for `isqrt`, repeated squaring
with overflow-safe modular addition for `powmod`, and checked iterative
factorial. Native implementations remain a measured future option only for
hardware-shaped primitives or proved hot bulk work.

Rejected alternatives:

1. A blanket native integer plugin: small hot calls can cost more than inlined
   Level B and do not justify another bootstrap/runtime dependency.
2. Decimal intermediates: they weaken the exact `.int` contract and introduce
   avoidable representation conversion.

### Decimal mathematics

**Selected:** use Level B algorithms directly over `.decimal`, inherit the
caller's numeric context, compute with bounded guard digits up to the current
64-digit provider capacity, and round the returned value in the caller's
context. The initial foundation uses Newton iteration for square root,
halving plus a convergent series for exponential, repeated square-root range
reduction plus the atanh series for logarithm, and quadrant-reduced Taylor
recurrences for sine and cosine. Constants carry enough decimal digits for the
current provider limit.

Rejected alternatives:

1. Convert through `.float` and call `rxfloat`: this discards the reason for a
   decimal surface—caller-selected decimal precision and semantics.
2. Install another native decimal provider: DECIMAL-01 retained `mc_decimal`,
   and a new provider requires its own evidence-gated selection.
3. Promote the dormant historical Rexx snippets unchanged: they are untyped,
   incompletely built, and do not provide a coherent domain, convergence,
   context, or test contract.

## Compatibility boundary

- Canonical provider ID: `rxfloat`.
- Canonical public namespace: `rxfloat`.
- Existing `rxmath` scalar names remain compatibility aliases from the same
  provider; no Rexx declaration wrapper is needed.
- Historical statistics, hash, UUID and `inlinec` draft names are removed from
  the pre-release product rather than preserved in a residual compatibility
  provider. Their later RCC-5 replacements require deliberately named,
  separately qualified surfaces.
- Existing type-prefixed Level B BIFs such as `intabs` and `floatformat` remain
  unchanged in this slice.

## Correctness gate

- Optimized/no-opt and `rxbvm`/`rxtvm` scalar float tests.
- Exact compatibility-name parity for every retained scalar procedure.
- Integer limits, signs, zero, `INT64_MIN`, overflow, invalid modulus/exponent
  and square-root boundaries.
- Decimal digits 9, 18 and the provider ceiling where practical; zero, signs,
  domain failures, range-reduction quadrants and independent high-precision
  expected values.
- Provider autoload, missing/wrong provider diagnostics, static concurrency,
  native package and scratch-install proof proportional to `rx_hash`.

## First Release verdict

Freeze the current `rxmath` provider and exact Release workload images before
the production edit. After minimum focused correctness:

1. compare old `rxmath` and new compatibility `rxmath` scalar calls with the
   same steady-state call kernel under both concrete VMs;
2. compare canonical `rxfloat` and compatibility `rxmath` calls within the new
   provider to prove alias neutrality;
3. run exact-work NBody and CD cells because they are current real consumers of
   `sqrt`, `sin`, and `cos`;
4. report startup/load separately from steady-state execution; and
5. apply the normal 3% workload guard and stop for Adrian's decision.

No broad CTest, sanitizer, install/package closeout, packed storage work, stats
implementation or further algorithm expansion precedes acceptance of that
verdict.

## RCC-5A closeout — complete

The Level B bootstrap and Level G availability distinction is documented and
the mathematics family has explicit implementation, availability, precision,
algorithm, compatibility, and performance boundaries. RCC-5A changes no
runtime code and needs no independent full-QA cycle; its contracts are the
criteria applied by the later production subphases.

## RCC-5B post-verdict qualification — retained

Adrian accepted the first Release verdict on 2026-08-20. The retained result
and exact artifact identities are in
[`2026-08-20-rcc5b-rxfloat-first-release-verdict`](evidence/2026-08-20-rcc5b-rxfloat-first-release-verdict/).

Post-acceptance qualification established:

- canonical `rxfloat` provider metadata and removal of stale draft `rx_float`
  artifacts from incremental build trees;
- 21/21 focused Debug tests and a repeated full Debug pass of 2,292/2,292 after
  one unrelated process-channel test reported `SIGPIPE` and passed its
  immediate serial retry;
- scratch-installed optimized/no-opt bytecode execution on both concrete VMs
  without an explicit provider argument;
- scratch-installed `crexx -native` packaging through the canonical static
  archive; and
- the exact former Apple-ASan provider-reload reproducer plus 6/6 focused
  `rxfloat`/RXPA sanitizer tests.

The sanitizer investigation also exposed a separate RXAS SSA
heap-use-after-free while assembling optimized AWFY Towers during a broad
artifact fixture. It is outside RCC-5B and is retained as an independent
follow-up rather than being conflated with the provider-lifetime fix.

These results qualify the implementation and accepted performance verdict; they
are not the consolidated RCC-5 full-QA closeout.

## RCC-5B focused mathematics coverage — complete

The reusable black-box suite now executes every one of the 37 canonical
procedures against independent expected values and proves every `rxmath`
compatibility name resolves to the same result. It covers trigonometric,
hyperbolic, exponential/logarithmic, rounding/remainder, root, special-function
and constant families, including representative domains, poles, NaN/infinity,
signed zero, negative inputs, half-way rounding, negative remainder, and the
repaired/scaled `hypot` cases.

The shared pure Level B `numerictestsupport` module applies typed
absolute-plus-relative binary64 assertions. It is test machinery rather than a
mathematical oracle: expected values and tolerances remain owned by the float
contract suite. The same module supplies exact integer and typed decimal
assertions for RCC-5C. A source-surface guard fails if a future registered
procedure lacks either an expected-value canonical call or an `rxmath`
compatibility call. Native-boundary tests separately prove missing and extra
argument signals for nullary, unary, and binary RXPA procedures because typed
source rejects those malformed calls before execution.

The optimized/no-opt `rxbvm`/`rxtvm` contract cells, structural surface guard,
and native arity test pass 6/6. This completes focused RCC-5B without claiming
exhaustive host-libm qualification. The approved risk-weighted float,
integer, and decimal scenario requirements are defined in
[`mathematics-validation-strategy.md`](../docs/planning/release-1/mathematics-validation-strategy.md).
They govern RCC-5C completion and do not run or replace the single consolidated
RCC-5 full-QA closeout.
