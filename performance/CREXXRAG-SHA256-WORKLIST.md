# CREXXRAG-SHA256: native RXPA surface versus pure Level B gate

Status: Option A selected by Adrian on 2026-08-19; production
`rxhash.sha256()` implemented and its first-Release verdict accepted on
2026-08-20

Approved by Adrian: 2026-08-19

## Decision question

Should the first public binary SHA-256 capability required by `crexx-rag` be:

- **A:** a public Level G-usable RXPA surface from a small process-reentrant
  native hash provider; or
- **D:** a portable pure Level B implementation?

This began as a focused, explicitly non-representative capability experiment.
Its selection evidence does not alter the common performance portfolio. The
subsequent RCC-4 production change was separately approved and qualified.

## Selection

Adrian selected **A** on 2026-08-19. The production signature is
`rxhash.sha256(data = .binary) = .binary`, published directly through RXPA;
there is no Rexx declaration wrapper. Provider `rx_hash` is recorded in RXBIN
metadata, resolved automatically by ordinary VMs, and selected automatically
as a static archive by native packaging.

## Hypothesis and risks

Hypothesis: pure Level B will be usable for small control-plane values but the
existing C SHA-256 implementation, reached through the real RXPA boundary,
will have enough steady-state advantage on asset-sized binary payloads to
justify Option A.

Risks to control:

- comparing different byte payloads, padding rules, or digest encodings;
- charging only one implementation for binary construction or hex conversion;
- measuring compiler/build/profiling instrumentation instead of the ordinary
  profiling-off Release product;
- hiding process startup inside a short kernel; and
- treating a native-C ceiling as the integrated Level G result.

## Locked comparison

1. Implement one test-only pure Level B SHA-256 and one test-only
   process-reentrant RXPA function backed by the existing `rx_sha256()`.
2. Give both variants the same in-memory `.binary`, return the same lowercase
   hexadecimal digest, and keep payload construction outside the timed loop.
3. Qualify empty, `abc`, boundary-length, embedded-zero, and deterministic
   generated payloads against published SHA-256 vectors or an independent host
   implementation before timing.
4. Measure the integrated native call and pure Level B path in the same
   optimized linked image on both concrete Release VMs (`rxtvm` and `rxbvm`).
5. Retain a standalone direct-C ceiling separately so RXPA/VM boundary cost is
   visible rather than folded into the algorithm claim.
6. Run all samples serially. Report benchmark-native steady-state time and
   process-inclusive elapsed separately. Use at least one warmup and three
   recorded observations per qualification-pilot cell.

The initial payload lanes are small request material (64 bytes), a typical
chunk (4 KiB), and an asset-sized buffer (1 MiB). Iteration counts may differ by
lane but must be identical across A and D within a lane.

## Decision gate

- Select **D** only if it is correct, portable, and its asset-sized throughput
  is close enough that the native deployment/package surface has no material
  operational payoff (working threshold: no worse than 2x integrated A).
- Select **A** if D is materially slower on 4 KiB or 1 MiB payloads, especially
  if the gap is an order of magnitude, while integrated A remains close enough
  to the direct-C ceiling that RXPA overhead is not dominant.
- If small-message RXPA overhead dominates but bulk A wins, select A with a
  single public facade and document that batching/incremental use is the normal
  asset path; do not create separate public APIs merely for the benchmark.
- If correctness, equivalence, or Release execution is not clean on both VMs,
  make no selection and repair the benchmark before more timing.

## Work log

| Gate | State | Evidence / next action |
| --- | --- | --- |
| G0 governance and scope | complete | Approved 2026-08-19; this worklist and `PERFORMANCE-GOVERNANCE.md` define the boundary. |
| G1 implementations and vectors | complete | Empty, `abc`, embedded-zero, padding-boundary, 4 KiB and 1 MiB vectors pass in pure and native form on both concrete VMs. Test-only sources live under `tests/performance/`; no production source changed. |
| G2 optimized dual-VM pilot | complete | One warmup plus three recorded serial observations per cell under profiling-off Release, with the required ten-sample append for five short cells that crossed the noise trigger. Integrated A is 196.5x-252.9x faster in-kernel; bulk RXPA cost is within about 3-7% of direct C. Evidence: [`2026-08-19-crexxrag-sha256-a-vs-d-gate`](evidence/2026-08-19-crexxrag-sha256-a-vs-d-gate/). |
| G3 recommendation and selection | complete | Adrian selected A on 2026-08-19: a public Level G-usable RXPA hash surface over a small process-reentrant `rx_hash` provider. Retain D only as reference/explicit fallback. |
| G4 runtime capability composition and production | complete | RCC-1 through RCC-4 implement `rx_hash` as standard/default rather than core, with declarative provider dependencies, static-first trusted autoload, automatic native-package linkage, and explicit module initialization before `main`. The accepted [first-Release verdict](evidence/2026-08-20-rcc4-rx-hash-first-release-verdict/) found no production-path slowdown versus the prototype. |
