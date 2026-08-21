# Statistics with `rxstats`

`rxstats` is a process-reentrant Level G native provider. Its current
`.float[]` argument contract is deliberately transitional: it provides an
independent semantic and numerical reference for RCC-5F, but the pre-release
boxed-array shape will be replaced after `BINARY-01` supplies aligned packed
`rxfloat` storage.

| Procedure | Result |
|---|---|
| `mean(values = .float[])` | Arithmetic mean; at least one value. |
| `stddev(values = .float[])` | Sample standard deviation; at least two values. |
| `covariance(x = .float[], y = .float[])` | Sample covariance; equal lengths of at least two. |
| `correlation(x = .float[], y = .float[])` | Pearson correlation; equal lengths of at least two and non-zero variance. |
| `regression(x, y, expose slope, expose intercept)` | Least-squares slope, also returned, and exposed intercept; non-zero x variance. |

```rexx
options levelg floats_binary
import rxstats

values = .float[]
values[1] = 1.0
values[2] = 2.0
values[3] = 3.0
say rxstats..mean(values)
```

The implementation uses Neumaier-compensated shifted-origin sums for means,
second moments, covariance, and regression. An ill-conditioned central moment
triggers a compensated second pass around the computed mean. This preserves
the accepted historical results on the retained `1e12` cancellation probe.
Invalid counts, unequal paired lengths, and undefined zero-variance operations
raise `INVALID_ARGUMENTS` with a procedure-specific message.

RXBIN provider metadata selects `rxstats.rxplugin` automatically for ordinary
execution and `rxstats.a` or the platform-equivalent archive for
`crexx -native`. No Rexx declaration wrapper or manual plugin list is needed.
