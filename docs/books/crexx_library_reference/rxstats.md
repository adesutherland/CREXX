# Statistics with `rxstats`

`rxstats` is the process-reentrant Level G native provider for bulk statistics
over `.packedfloat` owners. The provider borrows their host-native payloads
read-only for each call: it does not box elements, call `binary()`, copy the
buffer, or retain a payload pointer.

| Procedure | Result |
|---|---|
| `mean(values = .packedfloat)` | Arithmetic mean; at least one value. |
| `stddev(values = .packedfloat)` | Sample standard deviation; at least two values. |
| `covariance(x = .packedfloat, y = .packedfloat)` | Sample covariance; equal lengths of at least two. |
| `correlation(x = .packedfloat, y = .packedfloat)` | Pearson correlation; equal lengths of at least two and non-zero variance. |
| `regression(x = .packedfloat, y = .packedfloat)` | Immutable `.linearfit`; equal lengths of at least two and non-zero x variance. |

```rexx
options levelg floats_binary
import rxfnsg
import rxstats

values = .packedfloat(3)
call values.set(0, 1.0)
call values.set(1, 2.0)
call values.set(2, 3.0)
say rxstats..mean(values)
```

`.linearfit` is a small immutable Rexx value class in the `rxstats`
namespace. Its public factory accepts slope and intercept values, and its
`slope()` and `intercept()` methods expose the named results. Its packed
payload remains private; the class is only a result carrier, not a second Rexx
implementation of the statistics algorithms.

The implementation uses Neumaier-compensated shifted-origin sums for means,
second moments, covariance, and regression. An ill-conditioned central moment
triggers a compensated second pass around the computed mean. This preserves
the accepted historical results on the retained `1e12` cancellation probe.
Every input value must be finite. NaN, infinity, invalid counts, unequal paired
lengths, and undefined zero-variance operations raise `INVALID_ARGUMENTS` with
a procedure-specific message. A finite input whose calculation cannot be
represented as a finite native float raises `OVERFLOW_UNDERFLOW`. Passing a
bare, uninitialized `.packedfloat` raises `OBJECT_NOT_INITIALIZED`.

The surface deliberately does not accept `.packedint`, raw `.binary`, boxed
`.float[]`, ranges, offsets, strides, weights, or missing-value masks. Integer
accumulation needs a separately approved precision contract. Views, vector
operations, and accelerator selection belong to the later `rxvector` design,
which must reuse the same packed storage boundary.

RXBIN provider metadata selects `rxstats.rxplugin` automatically for ordinary
execution and `rxstats.a` or the platform-equivalent archive for
`crexx -native`. No Rexx declaration wrapper or manual plugin list is needed.
