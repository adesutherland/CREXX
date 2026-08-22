# RXVECTOR-01 first ordinary-Release verdict

Status: **positive verdict accepted by Adrian on 2026-08-22**.

This is the mandatory first profiling-off Release verdict for the frozen
RXVECTOR-01 candidate. No broad CTest, sanitizer, installed-package proof,
downstream cutover or documentation polish followed the measurement.

## Verdict

Lower elapsed time is better. Each cell has one discarded warmup followed by
12 serial recorded rounds in balanced, interleaved order. Every execution
returned zero and produced the exact checksum `1034.2`; no sample was removed.

| VM | Level B oracle median (range) | Public RXVECTOR median (range) | Direct shared-kernel median (range) | Paired public speedup over oracle | Public/direct median ratio |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxbvm` | 130,632.5 us (125,424-137,161) | **8,431 us** (8,291-8,657) | 8,540.5 us (8,418-8,758) | **15.622x** (14.733x-16.116x) | 0.9869x |
| `rxtvm` | 124,793 us (123,104-127,912) | **8,486 us** (8,273-8,899) | 8,510.5 us (8,398-8,737) | **14.682x** (14.374x-15.175x) | 0.9955x |

The Release product selects `rxbvm` (`rxvm -> rxbvm`), so its product result is
the first row; `rxtvm` is the required concrete-dispatch qualification. The
public/direct distributions straddle or sit marginally beyond parity by about
the observed noise, so the supported interpretation is that provider and
result-handling overhead is not measurable at this workload scale. It is not
evidence that the public route is intrinsically faster than the shared C
kernel.

The public kernel CV is 1.204% on `rxbvm` and 1.766% on `rxtvm`. Even the least
favorable recorded pair is more than 14.3x faster than the oracle, so the
verdict is not sensitive to the modest oracle/load variation.

## Workload and timing boundary

- optimized Level B caller; source and TRACE metadata stripped from the final
  linked image with `rxlink -s`;
- 11,684 row-major vectors by 768 dimensions: 8,973,312 host-native doubles
  (71,786,496 vector-payload bytes), plus packed identities and query;
- exact full scan, size-10 heap selection and deterministic final ordering;
- one public `rxvector.topkcosine` call or one direct-control call per measured
  kernel; and
- pure Level B uses the same packed values and requested result count.

No `f32le` conversion occurs in this comparison, so conversion time is zero
and the approved portable persistence boundary is not conflated with compute.
The matrix, identities and query are prepared before the benchmark-native
timer. `/usr/bin/time -p` separately records process-inclusive elapsed, which
includes VM launch, provider/direct-control loading, packed preparation and
teardown: medians are 0.14/0.13 seconds for the oracle and 0.02 seconds for
both public and direct on `rxbvm`/`rxtvm`. Its 0.01-second resolution makes the
process-minus-kernel remainder an upper-bound lifecycle observation, not a
precise preparation measurement.

This is an explicitly non-representative, single-capability comparison.
RexxCPS and the portfolio are technically inapplicable to selection of this
new vector primitive and are intentionally not claimed here.

## Source, host and build

- repository/branch/HEAD: `/Users/adrian/CLionProjects/CREXX`, `develop`,
  `6e6f0ea55d38db346bf78b6542635b2354ba2f04`;
- worktree: dirty only for the provisional RXVECTOR-01 implementation,
  focused concurrency integration, worklist/roadmap and benchmark evidence;
- host: `Mac17,3`, Apple M5, 10 physical/logical CPUs, arm64;
- OS: macOS 26.5.2 (25F84), Darwin 25.5.0;
- toolchain: Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2;
- build: ordinary `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`;
- power: AC attached and Low Power Mode off before and after;
- thermal: no thermal, performance or CPU-power warning before or after; and
- host state: user-declared clear; samples run serially.

The retained capture ran from 2026-08-22 14:23:02Z through 14:23:08Z. The
pre/post load averages and process snapshots are retained without attempting
to remove or correct any sample.

## Artifact identity

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| `rxbvm` | 1,430,088 | `55c772b19faec80645bde1154dd64f3f2749199c354a9305ae6ff847907a7f76` |
| `rxtvm` | 1,430,216 | `b788005396cc36dec20341bfb21934fb2c00e10df44d18972e4cb49251c691e9` |
| production `rxvector.rxplugin` | 34,096 | `6564401b446b39cad86bdeacc46c225ddeeaa9ff6560165d4e6ec4826376efb5` |
| direct-control plugin | 33,936 | `ba1439e0b6fb23c3ef212acf78b3676213df87cab79b20c98d31ab282b3ef750` |
| optimized RXAS | 63,842 | `3ee6ae5e01d3f859f71863b2fd2a6b8ed85610ca717495fffc1beb4a0bba3fca` |
| optimized RXBIN | 24,132 | `a7da1044bdd3cc10e756424f570add55ce4ba9ac3c6d44651b4b1c9e8067d3f2` |
| stripped linked RXBIN | 15,837 | `20bd6f6e837ea07338c2469492ce81b82134a197c72450793d1628fb507c0e7e` |

The source hashes are
`48e708e7541026d412785656455994dc8247155a180565d97e27fb5412a91925`
for `rxvector.c`,
`7553ffc92718d1b33276fd7c912ff283df393e346275d889baa3a2269020be02`
for the Level B comparison, and
`486e127b7549c57460fb04b202d487c88d6f8498f97e0477ce13c6aa2112e7aa`
for the direct control.

## Reproduction shape

The ordinary Release target and stripped image were built with:

```sh
cmake --build cmake-build-release --target performance_rxvector01 --parallel 10
cmake-build-release/bin/rxlink -s -o "$linked" \
  cmake-build-release/tests/performance/rxvector01_compare_opt \
  cmake-build-release/bin/library cmake-build-release/bin/rxfnsg
```

Each cell used this command shape, substituting `rxbvm`/`rxtvm` and
`pure`/`public`/`direct`:

```sh
/usr/bin/time -p -o "$timing" cmake-build-release/bin/$vm \
  --provider-path cmake-build-release/bin "$linked.rxbin" \
  cmake-build-release/bin/rx_rxvector01_direct.rxplugin -a "$variant"
```

Warmups used VM order `rxbvm`, `rxtvm` and variant order `pure`, `public`,
`direct`. Odd recorded rounds used that order; even rounds reversed both axes.
The first otherwise identical capture was discarded before retention because
pre-run thermal telemetry had not been recorded; it was not used in any
statistic here.

## Retained evidence

- `samples.csv`: all six discarded warmups and 72 recorded raw cells;
- `raw.log`: benchmark-native output and process timing for every cell;
- `summary.csv`: descriptive and paired distribution summaries with no
  outlier removal; and
- `host-pre.log` / `host-post.log`: power, low-power, thermal, load and process
  state.
