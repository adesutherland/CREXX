# RCC-5 mathematics and historical-provider composition worklist

Status: **complete and published**. RCC-5A through RCC-5F and the caller-relative
128-digit follow-up are implemented; all required first Release verdicts are
accepted. RCC-5F replaces the boxed statistics surface with the approved
`.packedfloat` contract and immutable `.linearfit`; its Release gate is clear
favorable on both VMs. Consolidated Debug, Release, install/package,
documentation, and Apple-ASan qualification is complete. Adrian assigned the
remaining supported Linux ASan/LSan proof for SAN-001 and SAN-002 to RCC-8
release QA; this is not a cross-platform sanitizer-clean or release-ready claim.

Approved by Adrian: 2026-08-20.

RCC-5D and RCC-5E approved by Adrian: 2026-08-21.

## Scope and stop boundary

This work may:

- document the Level B bootstrap and Level G availability distinction;
- split scalar float mathematics from the historical `rxmath` bundle;
- publish automatic dynamic/static provider forms and compatibility names;
- add Level-B-authored integer and decimal mathematics to the Level G standard
  library; and
- replace defective, dormant, or numerically weak implementations with known
  appropriate algorithms and focused correctness coverage.

RCC-5 did not itself implement `BINARY-01`; that independently accepted
prerequisite enabled the separately approved RCC-5F packed statistics section
below. RCC-5 does not retire an RXAS opcode, change numeric types or contexts,
or alter RXBIN format.
After the first production performance edit, minimum correctness is followed
immediately by the mandatory profiling-off Release verdict and a stop for
Adrian's direction.

Full broad QA, sanitizer sweeps, install/package qualification, and consolidated
documentation closeout run once at the end of RCC-5, not after every RCC-5
subphase. Each subphase still requires focused correctness proportional to its
surface and any mandatory first-Release performance verdict. The extensive
RCC-5B checks already completed remain useful retained evidence, but do not set
a precedent for repeating full QA after RCC-5C or another intermediate slice.

For RCC-5D, "does not promote the current statistics surface" means that the
boxed `.float[]` procedures are a deliberately transitional Level-G contract.
They make the historical statistics code independently loadable and testable;
they are not the final high-performance surface. `BINARY-01` and RCC-5F replace
that argument representation with aligned packed `rxfloat`/`rxinteger`
storage. No compatibility promise prevents that pre-release replacement.

## RCC-5D/E production design selection

### RCC-5D transitional statistics provider

**Selected:** publish a process-reentrant provider and namespace named
`rxstats` with boxed-array `mean`, sample `stddev`, sample `covariance`,
`correlation`, and `regression` procedures. Use compensated shifted-origin
accumulation for means, second moments, covariance, and regression components,
with a compensated second pass when the central moment is ill-conditioned.
Reject empty inputs, reject fewer than two observations where a sample
denominator is required, reject unequal paired lengths, and reject undefined
zero-variance correlation/regression with `INVALID_ARGUMENTS`. The
`regression` procedure retains the transitional exposed slope/intercept shape
only until RCC-5F.

Rejected alternatives:

1. Wait for `BINARY-01` before extracting any statistics provider. This leaves
   the historical bundle partition incomplete and provides no independently
   testable semantic oracle for RCC-5F.
2. Copy the historical sum/sum-of-squares algorithms unchanged. They have
   avoidable cancellation, an unqualified empty/singleton contract, an
   unchecked paired-length assumption, and a regression procedure already
   documented as not working.
3. Treat the boxed surface as final. Per-element RXPA attribute access is not
   the intended bulk vector contract and would pre-empt the separately approved
   aligned packed representation.

### RCC-5E provider split and pre-release removal boundary

**Selected:** finish the historical-bundle split with these canonical homes:

| Provider | Canonical public surface | Delivery/disposition |
|---|---|---|
| `rx_hash` | `rxhash.sha256`, `rxhash.djb2`, `rxhash.murmur3`, `rxhash.fnv1a`, `rxhash.crc32` | B+G standard/default; byte-oriented `.binary` inputs; no `rxmath` aliases. |
| `rxid` | `rxid.uuid4`, `rxid.uuid7`, `rxid.ulid`, `rxid.nanoid`, `rxid.snowflake`, `rxid.base58` | Bundled optional Level-G provider; UUIDv4 uses the same platform CSPRNG family as UUIDv7 rather than the historical pseudo-random generator. |
| `rxfs` | `cwd`, `loadpath`, `chdir`, `isdir`, `mkdir`, `rmdir`, `delete`, `rename`, `isfile`, `listdir`, `append` | B+G standard/default filesystem provider; the compiler driver links only this narrow native dependency. |
| `rxplatform` | `uptime`, `user`, `host`, `osname`, `sleep` | Bundled optional Level-G host-information/timing provider; no filesystem or UI calls. |

The public namespace is the namespace shown before the procedure name. Source
imports may call those procedures unqualified in the normal way. Provider
metadata, trusted dynamic autoload, and automatic native archive selection are
the RCC-1/RCC-2 route; none of these functions is hard-coded into `rxc` or
`rxvm`.

The dormant `inlinec`, parser, pipe, process-global key/value, clipboard, beep,
and binary-module-scanner entries have no Release-1 provider. `lmodules` is
removed in favour of the maintained RXBIN inspection tools. The broad
pre-release `system` provider and the private `id._*` declaration surface are
retired, not preserved as aliases. This follows the approved pre-release rule:
prefer a clear breaking surface to draft compatibility that would later need
support.

Rejected alternatives:

1. Keep `system` and add aliases in the narrow providers. This preserves the
   transitive dependency and ambiguous lifecycle/status that RCC-5E exists to
   remove.
2. Compile the complete historical `system.c` into multiple providers while
   registering different subsets. That duplicates dead code in static native
   products and makes the source ownership split cosmetic.
3. Move developer and legacy entries into `rxdev`/`rxlegacy`. The existing
   binary scanner is format-fragile, the parser and pipe paths are dormant, and
   the global/UI calls do not have approved standard contracts; a new provider
   identity would incorrectly bless them.

### RCC-5D/E first Release verdict

After minimum focused correctness, freeze the combined implementation and
compare profiling-off Release behavior before any broad QA:

1. compare the historical boxed statistics implementation with `rxstats` on
   the exact same deterministic arrays, reporting call/kernel and process-load
   time separately on both concrete VMs;
2. include a cancellation-sensitive dataset to confirm the selected stable
   algorithm is not traded away for speed;
3. compare unchanged SHA-256 throughput before/after adding the four
   checksum/table-hash procedures, proving provider growth does not regress the
   existing CREXX-RAG path; and
4. run a bounded `crexx` driver lifecycle drift cell because replacing its
   statically linked broad `system` provider with `rxfs` is the relevant
   end-to-end dependency result.

The result is reported to Adrian and implementation stops at the mandatory
first-Release gate. Consolidated Debug, sanitizer, install/package,
cross-platform, documentation, and RCC-5F work follow only after acceptance.

The initial combined gate accepted RCC-5E but rejected RCC-5D's first
Welford/Pébay numerical form. The approved shifted-origin rework exactly matches
the historical provider on the retained `1e12` cancellation probe. Its accepted
12-pair Release verdict improves ordinary median call rate by 3.140% on
`rxbvm` and 2.077% on `rxtvm`; the offset probe is +2.211% clear favorable on
`rxbvm` and +1.585% noisy favorable on `rxtvm`. No guard triggers. Evidence is
retained in
[`2026-08-21-rcc5de-provider-split-first-release-verdict`](evidence/2026-08-21-rcc5de-provider-split-first-release-verdict/).

Post-acceptance proportional qualification is complete. The final Debug tree
passes 2,324/2,324 tests; the frozen production implementation passes 37/37
focused profiling-off Release tests; and the provider, concurrency, installed
consumer/native-package, RexxDoc, and native-signal panel passes 42/42 under
Apple ASan. This is deliberately not a repeated consolidated RCC-5 sanitizer
or cross-platform closeout: that single broad gate runs after RCC-5F, in line
with the approved subphase-efficiency rule above.

## RCC-5F packed statistics production design selection

Approved by Adrian: 2026-08-22.

**Selected:** keep one production statistics implementation: the
process-reentrant native `rxstats` provider. Replace the five transitional
boxed `.float[]` signatures in place with dense `.packedfloat` arguments:

| Procedure | Result | Contract |
|---|---|---|
| `mean(values = .packedfloat)` | `.float` | Arithmetic mean of at least one finite value. |
| `stddev(values = .packedfloat)` | `.float` | Sample standard deviation of at least two finite values. |
| `covariance(x = .packedfloat, y = .packedfloat)` | `.float` | Sample covariance of equal-length inputs containing at least two finite pairs. |
| `correlation(x = .packedfloat, y = .packedfloat)` | `.float` | Pearson correlation of equal-length finite inputs with non-zero variance on both sides. |
| `regression(x = .packedfloat, y = .packedfloat)` | `.linearfit` | Ordinary least-squares linear fit of equal-length finite inputs with non-zero x variance. |

`.linearfit` is an immutable Level-G value class in the `rxstats` namespace.
It exposes `slope()` and `intercept()` plus a two-scalar value factory. Its
small packed payload is private: it publishes no positional, mutation, resize,
or binary-reference surface. This Rexx value class is not a second Rexx
implementation of the statistics algorithms.

The native procedures declare `.packedfloat` arguments and consume each
owner's exact `register.0.binary` payload directly. They borrow it read-only
for the duration of the call, never retain its pointer, and perform no
`binary()` call, reference/dereference, Rexx wrapper, boxed element access, or
whole-buffer copy. The caller must not mutate or resize an input concurrently
with a call. Payload length determines the item count and must be an exact
multiple of the native `rxfloat` width. The implementation uses alias-safe
native loads and retains the RCC-5D compensated shifted-origin and conditional
second-pass algorithms.

All input values must be finite. NaN or infinity raises `INVALID_ARGUMENTS`;
finite inputs that produce a non-finite statistical result raise
`OVERFLOW_UNDERFLOW`. Existing count, paired-length, and zero-variance
rejections retain `INVALID_ARGUMENTS`. Bare uninitialized owners retain the
ordinary `OBJECT_NOT_INITIALIZED` language signal.

The production provider accepts `.packedfloat`, not `.packedint`. BINARY-01
deliberately has no hidden numeric-kind tag, and integer statistics would need
a separately approved conversion and accumulation-precision contract. RCC-5F
also does not publish offset, range, stride, weights, missing-value masks,
multidimensional layouts, or accelerator selection. Those view and backend
questions belong to the later `rxvector` design; it must reuse the accepted
packed storage rather than create another representation.

The boxed signatures and their exposed regression outputs are removed without
production aliases or conversion wrappers. The old implementation and its
datasets remain test-only oracle material.

Rejected alternatives:

1. Keep boxed overloads or conversion wrappers. This preserves the draft
   surface and makes accidental whole-buffer conversion look supported.
2. Accept raw `.binary` plus a kind flag. This discards the approved class
   type check and recreates a hidden representation contract in each caller.
3. Accept both packed integer and float values through one generic signature.
   The payload has no kind tag, and silently reinterpreting integer bytes as
   float is not a valid numeric conversion.
4. Add offset/count/stride arguments now. This would pre-empt the ownership and
   view contract that RCC-5F is explicitly required to leave to `rxvector`.
5. Retain regression's exposed slope/intercept arguments or return a positional
   two-item buffer. The immutable named result is clearer and does not expose
   its private storage layout.

The focused Release gate compares packed production calls with the retained
boxed oracle and a direct native payload-scan control under both concrete VMs.
The packed path must be clearly favorable to boxed access in both VM modes;
any neutral/adverse result, whole-buffer copy, or residual boxed element loop
requires rework rather than qualification. Consolidated Debug, sanitizer,
install/package, cross-platform, documentation, and SAN closeout is the single
end-of-RCC-5 gate.

The gate passes. Against the retained boxed oracle, packed production improves
paired median call rate by 2054.025% on `rxbvm` (12/12 favorable) and
2062.894% on `rxtvm` (24/24 retained pairs favorable). The `1e12`
cancellation probe improves by 2055.093% and 2080.291% respectively, with
identical mean, deviation, covariance, correlation, and checksum output. The
public packed path reaches 90.757-91.562% of the direct native control on the
ordinary dataset and 90.922-91.221% on the offset dataset. One permitted
unchanged `rxtvm` rerun is retained alongside the original noisy samples; no
sample was removed and no guard fired. Evidence is in
[`2026-08-22-rcc5f-packed-stats-first-release-verdict`](evidence/2026-08-22-rcc5f-packed-stats-first-release-verdict/).

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
| `rxstats` | process-reentrant native bulk provider over borrowed `.packedfloat` payloads | Level G standard; boxed surface removed pre-release |

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
caller's numeric context, qualify caller precision through 64 digits, compute
with bounded 18-, 32-, 64-, and 96-digit work tiers, and round the returned
value in the caller's context. The 96-digit tier retains 32 guard digits at the
qualified ceiling. The initial foundation uses Newton iteration for square root,
halving plus a convergent series for exponential, repeated square-root range
reduction plus the atanh series for logarithm, and quadrant-reduced Taylor
recurrences for sine and cosine. Constants carry independently checked guard
digits beyond the qualified caller ceiling.

Rejected alternatives:

1. Convert through `.float` and call `rxfloat`: this discards the reason for a
   decimal surface—caller-selected decimal precision and semantics.
2. Install another native decimal provider: DECIMAL-01 retained `mc_decimal`,
   and a new provider requires its own evidence-gated selection.
3. Promote the dormant historical Rexx snippets unchanged: they are untyped,
   incompletely built, and do not provide a coherent domain, convergence,
   context, or test contract.

### Decimal caller-relative precision follow-up

**Approved 2026-08-20:** remove the accidental 64-digit assurance ceiling
without introducing a new runtime ceiling. The original RCC-5C evidence remains
an explicit 64-caller-digit milestone; this follow-up qualifies every public
`rxdecimal` procedure through 128 caller digits.

The selected implementation uses the existing register-form numeric-context
instructions inside the private Level B library implementation. Each public
entry point records the inherited caller precision, selects a wider work
precision, performs the calculation in that same invocation frame, restores
the caller precision, and rounds the result on return. Existing work levels are
preserved through the formerly qualified range: 18 work digits through caller
9, 32 through caller 18, 64 through caller 32, and 96 through caller 64. Above
64, work precision is caller precision plus 32 guard digits. This changes no
language syntax, opcode, RXBIN structure, provider ABI, or public function
signature.

The selected constant strategy computes `pi` with the quadratically convergent
Gauss-Legendre algorithm and computes Euler's number through the decimal
exponential implementation. Precision-aware values are cached once per mutable
module instance and replaced only when a wider work context is requested.

Rejected alternatives:

1. Retain 64 as the qualification boundary: this would not exercise a public
   result beyond the provider's 64-digit inline capacity and would not prove
   that the old ceiling had been removed.
2. Add more fixed 128/160-digit adapter procedures and longer literal
   constants: this merely moves the hidden ceiling and retains duplicated
   procedure surfaces.
3. Add dynamic `NUMERIC DIGITS <expression>` source syntax: the VM already has
   the required register-form operation and the library can scope it to its own
   invocation frame, so a public language change is unnecessary for this
   requirement.

The retained precision cells 9, 10, 18, 19, 32, 33, and 64 remain regression
coverage. New 65, 96, 97, and 128 cells cross the provider inline capacity,
the former internal work tier, and the new qualification boundary. The 128
limit is a documented assurance boundary, not a runtime maximum.

Adrian accepted the first profiling-off Release verdict on 2026-08-20. All
four steady cells are clearly favorable by 0.55-1.31%; four one-call cold
cells remain noisy at the governed 36-pair ceiling, with effectively flat
medians and no 3% workload guard hit. Exact artifacts, raw samples, appends,
and the final paired table are retained in
[`2026-08-20-rcc5c-dynamic-precision-first-release-verdict`](evidence/2026-08-20-rcc5c-dynamic-precision-first-release-verdict/).

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
artifact fixture. It is outside RCC-5B and is retained as SAN-001 rather than
being conflated with the provider-lifetime fix. SAN-001 is now repaired and its
profiling-off Release verdict was accepted on 2026-08-21.

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

## RCC-5C integer and decimal libraries — complete

The reusable RCC-5C suites now cover every `rxint` and `rxdecimal` export in
optimized and no-opt images on both concrete VMs. Integer coverage includes
bounded exhaustive independent-oracle grids, exact limit-adjacent results,
`INT64_MIN`, overflow-safe modular multiplication, all factorials through 20,
and operation-specific domain/overflow signals. Decimal coverage exercises
caller digits 9, 10, 18, 19, 32, 33, and 64; exact constants and roots;
exponential/logarithmic convergence and reduction boundaries; all
trigonometric quadrants; values on both sides of `pi/2`, `pi`, and a full
period; and positive/negative multi-period reduction.

The tests exposed and closed two production defects: `rxint.lcm` now checks
the representable magnitude before multiplication so overflow carries the
`RXINT.LCM` identity, and the 64-digit decimal caller path now computes in a
96-digit work tier instead of losing trailing precision without guard digits.
The shared test helper also stopped applying file-wide binary-float treatment
to explicitly declared decimal assertions.

Offline GNU `bc` 7.0.3 and Python 3.14.6 generators, exact commands, rounding
policy, and reviewed content hashes are retained in
[`math-reference-provenance.md`](../lib/rxfnsg/tests_functional/math-reference-provenance.md).
The eight execution cells plus structural guard passed 10/10 including their
artifact fixture. Following a complete Debug build, Adrian's requested broad
review run passed 2,302/2,302 CTests in 213 seconds.

This closes RCC-5C only. The broad Debug result is useful retained evidence,
but it predates the separately implemented RCC-5D/E provider split and is not
its post-acceptance closeout evidence.

## RCC-5C caller-relative precision follow-up — complete

The 64-digit assurance limit has been removed without adding another runtime
ceiling. All seven exports pass independent constant/transcendental checks at
65, 96, 97, and 128 caller digits in optimized/no-opt images on both concrete
VMs. The final sequence also proves a widest cached `pi`/`euler` value rounds
back correctly into a later 65-digit caller and preserves that caller's
numeric context.

Post-verdict qualification passes a complete 2,302/2,302 Debug run, final
focused Debug 5/5, complete profiling-off Release 2,302/2,302, targeted
Apple-ASan build and 5/5 decimal cells, and fresh installed bytecode/native
128-digit smokes. The repository-wide Apple-ASan build originally reproduced
the retained SAN-001 `rxas_flow_value_node()` heap-use-after-free while
assembling optimized AWFY Towers before CTest.

## Consolidated RCC-5 qualification and publication — complete

The current profiling-off Release implementation passes its focused 19/19
provider, both-VM, optimized/no-opt, compiler import, concurrency, RexxDoc,
fresh scratch install/autoload, and native-package panel. Normal Debug broad
closure ran all 2,356 tests: 2,290 passed immediately and 66 compiler/codegen
goldens exposed stale expectations from the already accepted BINARY-01
lowering. After reviewed mechanical expectation updates, the exact failed
surface plus its build fixture passed 67/67. This is broad-plus-targeted
closure, not a second monolithic 2,356-test invocation.

The complete current Apple-ASan instrumented build and all 2,356 CTests pass
with no sanitizer report in
`cmake-build-debugasan/asan-logs/20260822-104056-full`. The gate includes the
new packed statistics, native result-class import, concurrent provider load and
call, installed consumer/native package, and all permanent SAN-002 cells.

SAN-001 now retains the exact 64-value RXAS growth regression, a repaired
Debug/Apple-ASan proof, optimized Towers and 70-test flow/proof qualification,
and Adrian's accepted neutral Release verdict. SAN-002 retains two minimal
wide-return/narrow-caller decimal cases across optimized/unoptimized `rxbvm`
and `rxtvm`; a counterfactual run reproduces both original use-after-poison
stacks, establishing one value-aware DEXTR capacity defect rather than SAN-003.

The earlier complete Apple-ASan 2,310/2,310 result remains useful repair
evidence; the current 2,356/2,356 result supersedes it for consolidated RCC-5
Mac qualification. Automatic Linux x64 ASan/LSan and macOS arm64 ASan jobs are
defined in `.github/workflows/sanitizers.yml` and use the full runner phase,
including instrumented build-time execution. Their stable names still require
repository-level required-check configuration. No local Linux runtime is
available. On 2026-08-22 Adrian approved RCC-5 closure and publication, with
that supported Linux ASan/LSan proof explicitly transferred to RCC-8 release
QA. SAN-001 and SAN-002 remain live and release-blocking until the exact later
release-QA candidate passes that gate; the handoff does not qualify the
repository as cross-platform sanitizer-clean.
