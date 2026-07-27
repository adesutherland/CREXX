# PERF2-08 Mac capability and equivalence qualification

Status: **decision evidence complete; no formal PERF2-09 samples**

This bundle supports the PERF2-08 approval panel in
`performance/PERF2-08-09-WORKLIST.md`. It freezes the current Mac product,
external runtime identities, source/equivalence audit, bounded correctness
pilots, the ooRexx Towers replacement candidate and negative Mandelbrot
evidence. Pilot rates printed by RexxCPS are correctness/provenance smoke
output only. They are not governed formal measurements.

## Source and host identity

- Branch: `develop`
- starting HEAD: `057592681c0c68e90f436bf02d8c5a116111952a`
- accepted PERF2-06/07 product: `39d3c652e27860222f5d5ed43af71147589b1121`
- upstream at freeze: `origin/develop` at
  `53f7757c5b21c15d405b17920d4cd7f6c554c46b`
- qualification change: uncommitted ooRexx Towers source plus worklist/evidence
  documentation; no cREXX production source changed
- host: MacBook Air `Mac17,3`, Apple M5, 10 logical CPUs (4P/6E), 24 GB
- OS: macOS 26.5.2 (25F84), Darwin 25.5.0, Apple ARM64
- power/thermal at initial freeze: AC, low-power mode off, no thermal or
  performance warning
- initial load: `3.89 12.25 9.36`; explicitly rejected as formal timing state
- compiler/build tools: Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2

The clean detached source is retained at
`/private/tmp/crexx-perf2-0809.uMsNxm/src`. Independent qualification products
are under `build-release` (`Release`, `-O3 -DNDEBUG`, profiling off) and
`build-profile` (`Release`, profiling on). Only the profiling-off product was
built and used for the correctness pilot. The profiling build and later native
sampling capture remain separate.

## External products

- ooRexx 5.1.0 r12973; the official installation `bin` directory was prepended
  to `PATH` so its checksum-frozen `json.cls` resolves normally.
- Regina 3.9.7, used only for canonical RexxCPS.
- NetRexx 5.10-GA build 18-20260320-1410 with Temurin/OpenJDK 26.0.1+8.
- NetRexx invocation uses
  `java -cp GENERATED:NetRexxR.jar CLASS ARG`; the exact compiler/runtime JAR,
  Java executable and generated Java/class identities are frozen here.

See `product-identities.sha256` and `source-identities.sha256` for exact
identities. The generated files in `generated/netrexx-current` are retained so
the qualification product does not depend on mutable files outside this
bundle.

## Correctness execution

The isolated Release build passed all 39 benchmark-labelled CTest cases. The
selected non-common cREXX pilot then covered Mandelbrot sizes 1/500/750,
Towers/Storage/List/JSON repetitions 1/2 and RexxCPS smoke count 1, on both
`rxvm` and `rxbvm` and in optimized/no-opt form. Result: 48 PASS and zero
failures. RexxCPS smoke count is a correctness/provenance check, not a timing
input.

Current generated NetRexx classes passed all 11 selected Mandelbrot,
Towers/Storage/List/JSON cells. ooRexx passed all eight Towers, Storage, List
and JSON cells and the Mandelbrot size-1 guard. Its ordinary decimal arithmetic
produced the retained expected negative results at the common binary64 checks:

| Size | Required binary64 checksum | ooRexx result |
| ---: | ---: | ---: |
| 1 | 128 | 128 |
| 500 | 191 | 255 |
| 750 | 50 | 128 |

The bounded `NUMERIC DIGITS` probe tested 9, 15, 16, 17, 20, 34 and 50.
No setting produced the common size-500/750 contract. At digits 9 the results
were 255/192; all tested values from 15 through 50 produced 255/128.

Canonical RexxCPS 2.2 exited successfully on ooRexx and Regina. The disclosed
NetRexx 2.2n adaptation emitted its PASS marker. Their printed rates are kept
only to prove the pilot completed.

`raw/oorexx-noncommon-correctness.log` includes a repeated size-750 negative
line from a bounded retry after the first wrapper stopped on the intentional
nonzero exit. This duplication is not an additional sample and is not used as
one.

## Equivalence findings

| Row | Exact current distinction | Panel recommendation |
| --- | --- | --- |
| CAP-01 / JSON | cREXX reparses a string/path query; ooRexx and NetRexx build different retained DOMs. The shared answer `8` does not make construction, allocation or access equal. | Diagnostic only; defer any parse-once hierarchy/handle API. |
| CAP-02 / Storage | The common depth-7 four-way tree has 5,461 logical nodes. cREXX allocates 5,461 `StorageNode` owners plus 5,461 child arrays because the current Level B surface cannot own nested arrays as ordinary object values. External ports allocate 5,461 arrays. | Diagnostic only; defer the owned heterogeneous/nested container question to post-Release 1 Level G. |
| CAP-03 / Base64 | No reusable product API exists, but the benchmark itself retains the same RFC 4648 arithmetic, data, round trip and observations. | Keep benchmark qualified; defer library API independently. |
| CAP-04 / lifecycle | No common public load-without-execution CLI boundary exists. | Retain compile/translate and honestly named load-to-first-result; exclude pure load. |
| RexxCPS | cREXX 2.2d and NetRexx 2.2n are disclosed adaptations; ooRexx/Regina use canonical 2.2. | Separate closure target and labelled controls, never common-five membership. |
| Mandelbrot | ooRexx decimal numerics do not implement the binary64 work contract. | Approved exclusion; no cross-runtime ratio. |
| Towers | The replacement ooRexx form creates one benchmark plus 14 disk objects, preserves method/link operations and observes 8,191 recursive moves. | Qualify the replacement after Adrian approves the panel. |
| Storage | cREXX's additional owner object per logical node is material allocation/dispatch work. | Diagnostic exclusion tied to CAP-02. |
| List | All ports create 31 nodes, but cREXX additionally allocates an owning arena/typed array because links are weak; external object links own referents. | Diagnostic exclusion; timings may remain visible without a ratio. |
| JSON | Same payload/result, different parse and result model. | Diagnostic exclusion tied to CAP-01. |

No language, public API/ABI, RXAS/RXBIN, serialization or architecture change
was made. The formal matrix, cell arguments, RXAS/RXBIN/linked images,
steady-state rounds, lifecycle, RSS and artifact score remain intentionally
unfrozen until Adrian accepts the PERF2-08 panel.

