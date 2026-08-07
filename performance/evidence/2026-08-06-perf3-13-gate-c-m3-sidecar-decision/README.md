# PERF3-13 Gate C M3 typed-sidecar decision

Date: 2026-08-06  
Status: **decision point — recommend 176-byte L32SDH; Gate D remains closed**

## Decision

Keep the frozen 192-byte L32S product as the control and carry the 176-byte
L32SDH representation into Gate D industrialisation:

- retain the decimal payload pointer directly in `value`;
- place raw `size_t` decimal length and capacity in a fixed header immediately
  before the worker-slab-owned payload;
- keep the header and payload in one sticky allocation;
- keep object-growth and native-payload metadata direct in `value`;
- retain raw `size_t` capacities and the accepted 64 KiB slab geometry.

This is a recommendation at the final Gate C M3 evidence point, not a product
default change. Gate D, reclamation, `rxtvm`, worker execution and channel
semantics remain closed.

## Host and controls

- Source: private worktree branch `codex/rxvm-default-and-base64-review` at
  `0d1fe884782ff369960b1c67c38127407ce54588` plus the Gate C research changes.
- Host: Apple M5, Darwin arm64, 10 logical CPUs, Apple Clang 21 Release.
- Power: AC attached at 80%; no thermal or performance warning was recorded.
- The host was reserved exclusively for this work.
- Frozen control: L32S 192-byte `rxbvm`, SHA-256
  `2d3dc6c8df5e260cf6100413ee11d0bf4d497ba6ca775e12ebb9494abf138174`.
- Recommended candidate: L32SDH 176-byte `rxbvm`, SHA-256
  `81c9b018e1ee106996b52c65d3f946ad0d435e5fd953dc305abb55add97b0710`.
- Maximum viable comparator: L32SDHO 160-byte `rxbvm`, SHA-256
  `85864990ece0e60d3ae5f42106488be7471ddeba690d2d9cb2dc667933db6e5d`.

## Maximise and back off

The complete D/N/O factorial first tested every combination from 192 down to
152 bytes. Targeted backoffs then kept the decimal payload pointer or the hot
object maximum direct. Finally, metadata/payload co-allocation removed the
decimal descriptor allocation. The material outcomes were:

| Layout | Bytes | Outcome |
|---|---:|---|
| L32S | 192 | Frozen control |
| L32SDNO | 152 | Reject: core-four screen -1.069%; about 24 KiB more text |
| L32SN family | 168-184 | Reject: 8 bytes costs about 20 KiB text and repeated regressions |
| L32SO1 | 184 | Reject: Towers -4.056% in the 12-pair backoff panel |
| L32SDPO1 | 176 | Reject: Towers -3.536% |
| L32SDP | 184 | Reject representation: separate decimal metadata caused 10.74M tracked RexxCPS allocations and 66.2 MB cumulative internal fragmentation |
| L32SDH | 176 | Recommend: one header+payload allocation, direct hot payload pointer |
| L32SDHO1 | 168 | Reject: Towers -3.004% in the corrected four-cell screen |
| L32SDHO | 160 | Timing-neutral, but reject on allocation churn and complexity |

## Recommended 176-byte evidence

Two independent 12-pair core-four blocks were combined. Positive percentages
favor L32SDH.

| Workload | Pairs | Paired mean | 95% interval | Paired median |
|---|---:|---:|---:|---:|
| Sieve | 24 | +0.474% | -0.085% to +1.033% | +0.595% |
| Richards | 24 | -0.342% | -1.233% to +0.548% | +0.215% |
| Towers | 24 | +2.893% | +2.444% to +3.341% | +2.959% |
| RexxCPS | 24 | +0.363% | -0.734% to +1.460% | +0.696% |
| Pooled core four | 96 | +0.847% | +0.398% to +1.296% | +1.018% |

JSON was neutral at +0.384% (-0.438%, +1.205%). Base64 measured -7.194%
(-12.940%, -1.448%) for L32SDH but +3.204% (-3.703%, +10.112%) for the
nearby L32SDHO layout. That sign reversal supports its prior noisy/non-selecting
classification; CAP-03 retains the separate Level B library/API investigation.

The 176-byte form is 8.333% smaller than L32S. Artifact movement is bounded:
the file is +48 bytes, `__text` is +2,104 bytes (+0.259%) and flattened `run()`
is +2,276 bytes (+0.428%). Four-run direct RSS medians are +0.043% to +0.221%,
well below the 5% materiality guard.

Peak live allocator capacity falls in every measured workload:

| Workload | L32S | L32SDH | Change |
|---|---:|---:|---:|
| Sieve | 3,681,732 | 3,549,284 | -3.597% |
| Richards | 2,317,036 | 2,299,020 | -0.778% |
| Base64 | 2,045,220 | 2,035,780 | -0.462% |
| JSON | 2,320,068 | 2,296,244 | -1.027% |
| Towers | 2,882,388 | 2,823,028 | -2.059% |
| RexxCPS | 2,192,716 | 2,179,468 | -0.604% |

Retained slab bytes are unchanged in all six cells. RexxCPS deliberately moves
decimal payload allocation from the plugin's untracked libc heap into the
worker slabs; tracked allocation calls therefore rise from 4.14M to 8.68M,
while peak capacity falls and retained slabs remain 1,310,720 bytes.

## Why not stop at 160 bytes

L32SDHO is timing-neutral: its formal core-four result is +0.590%
(-0.454%, +1.633%), with Sieve +0.901%, Richards +0.723%, Towers -1.077%
and RexxCPS +1.813%. RSS improves by 0.215-0.757% in five cells and is flat in
Base64. However, Towers allocates 12,071,103 additional object descriptors,
raising allocator calls from 48.29M to 60.36M (+25.0%). That buys only another
44,800 bytes of peak-capacity reduction beyond L32SDH while reversing Towers
from +2.893% to -1.077%. It also grows `__text` by 7,152 bytes and `run()` by
6,300 bytes versus L32S and introduces a second sidecar lifecycle.

The 160-byte form clears the numerical guards, but it is not the simplicity
sweet spot. L32SDH captures the value-density benefit without object-lifecycle
churn and is the safer foundation for per-worker ownership.

## Correctness and implementation cost

- All factorial and backoff builds passed the value-layout test and all six
  benchmark correctness markers.
- L32SDH passes the focused value, allocator, stem and allocator-family tests.
- The compiler, static and dynamic `mc_decimal`, dynamic/static `db_decimal`,
  decimal archive-link harness and decimal arithmetic sanity path build/pass
  with the shared layout contract.
- Gate D must replace the research selectors with one clean production
  representation and migrate the legacy full decimal test fixtures from raw
  `free(decimal_value)` to `value_init`/`clear_value` ownership.
- The rejected N/O selector code and research-only compatibility macros are
  disposable and should not be industrialised.

Raw captures remain in the reserved scratch root under
`/private/tmp/crexx-rxvm-inline.yvLywZ/results/`, notably
`gatec-sidecar-factorial-screen`, `gatec-sidecar-backoff-formal`,
`gatec-sidecar-dp-confirm`, `gatec-sidecar-dh-confirm`,
`gatec-sidecar-dh-confirm-2`, `gatec-sidecar-dh-diagnostic`,
`gatec-sidecar-dho-confirm`, `gatec-sidecar-dho-diagnostic`,
`gatec-sidecar-dh-rss-direct`, `gatec-sidecar-dho-rss-direct` and
`gatec-sidecar-memory-stats`.
