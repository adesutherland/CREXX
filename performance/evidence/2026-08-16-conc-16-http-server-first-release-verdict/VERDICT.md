# CONC-16 Level G HTTP server first Release verdict

## Decision

**ACCEPTED under the approved unattended rule.** Neither 12-pair ordinary
single-thread campaign has a 3% guard hit. The first campaign was entirely
noisy/inconclusive; the unchanged confirmation makes `rxtvm` Sieve clearly
favourable and leaves the other three cells noisy. The small first-run
`rxbvm` RexxCPS adverse direction reverses in confirmation.

This Step 3 slice changes Level G/private HTTP bytecode, tests, build metadata
and documentation only. It does not change concurrency source or semantics,
`.serviceref`, `ask()`, the compiler, RXAS, RXBIN, linker, socket instructions,
or RXVM execution source.

## Ordinary single-thread result

Percentages are paired candidate changes. Negative Sieve elapsed and positive
RexxCPS rate are favourable. Each cell has 12 recorded pairs after one warm-up;
no sample was removed.

| Run | Workload | VM | Mean change | Mean 95% interval | Result | 3% guard |
| --- | --- | --- | ---: | ---: | --- | --- |
| first | Sieve | `rxtvm` | -0.336% elapsed | [-0.964%, +0.293%] | noisy | no |
| first | Sieve | `rxbvm` | -0.289% elapsed | [-1.220%, +0.641%] | noisy | no |
| first | RexxCPS | `rxtvm` | -0.170% rate | [-0.766%, +0.426%] | noisy | no |
| first | RexxCPS | `rxbvm` | -0.466% rate | [-0.973%, +0.040%] | noisy | no |
| confirmation | Sieve | `rxtvm` | -0.681% elapsed | [-1.342%, -0.020%] | clear favourable | no |
| confirmation | Sieve | `rxbvm` | +0.247% elapsed | [-0.361%, +0.855%] | noisy | no |
| confirmation | RexxCPS | `rxtvm` | -0.504% rate | [-1.101%, +0.093%] | noisy | no |
| confirmation | RexxCPS | `rxbvm` | +0.306% rate | [-0.621%, +1.233%] | noisy | no |

## Server scenario observation

All 48 recorded server processes passed. Internal elapsed time is the whole
fixture server interval, not per-request latency: it includes the deliberate
200 ms slow-client timeout before two parallel valid requests. Process elapsed
also includes VM startup and teardown.

| Mode | Internal median | Internal range | Process median | Median scans | Median idle waits |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxbvm` no-opt | 330.890 ms | 315.084-332.320 ms | 539.163 ms | 265 | 260 |
| `rxbvm` opt | 328.916 ms | 309.776-331.051 ms | 534.269 ms | 263 | 258 |
| `rxtvm` no-opt | 330.988 ms | 329.605-333.248 ms | 538.836 ms | 265 | 260 |
| `rxtvm` opt | 329.712 ms | 319.315-331.045 ms | 534.118 ms | 264 | 259 |

About 98.1% of controller scans took the private 1 ms blocking pacer path in
this deliberately idle-heavy scenario. This proves deadline progress without
an idle busy-spin. It is not a direct CPU-utilisation measurement and does not
claim production throughput or network latency.

## Correctness and artifacts

- Focused Debug HTTP/client/server/LLM matrix: 35/35 passed.
- Focused Release HTTP/client/server/LLM matrix: 35/35 passed.
- The four server modes pass concurrently, including the service/taskwork
  bridge, binary body, incomplete-request 408, framing negatives and cleanup.
- The separate four-mode failure matrix proves oversized responses and raised
  handlers are bounded and returned as HTTP 500 responses.
- Apple ASan passed both four-mode server matrices with leak detection disabled;
  LeakSanitizer is unavailable on this host.
- `rxtvm` and `rxbvm` retain their exact control sizes. Their hashes differ
  because rebuilt executables record dirty build metadata; no VM source changed.
- `library.rxbin` is byte-identical to control at 906,019 bytes.
- Both benchmark images are byte-identical to control.
- `rxfnsg.rxbin` grows from 259,097 to 317,136 bytes (+58,039) for the server,
  inbound parser, typed request/service surface and response builders.

## Boundary

This is a profiling-off Release verdict on Darwin 25.5.0 arm64, Apple M5. It
supports the bounded clear-text buffered server slice only. TLS termination,
HTTP/2, WebSockets, detached/background lifecycle, request/response streaming,
portable-host qualification and production capacity/throughput remain outside
this result.
