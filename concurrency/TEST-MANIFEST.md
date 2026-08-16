# Initial concurrency test manifest

Status date: 2026-08-16

This is the maintained executable manifest for the frozen initial concurrency
surface. CMake attaches two enduring labels to every included test:

- `concurrency` selects the complete correctness matrix;
- `concurrency-sp01` through `concurrency-sp09` select one reviewed solution
  point.

Historical development labels and test names may remain useful diagnostics,
but they are not the orchestration contract.

## Deterministic entry point

Configure the build normally, then use one target from the repository root:

```sh
cmake --build cmake-build-debug --target concurrency-qa --parallel 10
```

`concurrency-qa` first completes the explicit
`concurrency_test_artifacts` dependency target. Only after every compiler,
assembler, linker, VM, library, fixture and linked image is ready does it run:

```sh
ctest --test-dir cmake-build-debug --parallel 10 \
  --output-on-failure -L '^concurrency'
```

The prefix selector includes the umbrella and solution/stress labels. The
manifest helper guarantees that every such test also carries `concurrency`, so
CTest still executes each declared case exactly once.

The CTest job count is controlled at configure time by
`CREXX_CONCURRENCY_CTEST_JOBS`. It defaults to `10` on Unix-like hosts and `1`
on Windows, where build/test overlap and fixture cleanup can conflict with
open files. The build command's `--parallel` value controls only artifact
construction; it does not overlap construction with CTest.

To inspect rather than execute a solution point:

```sh
ctest --test-dir cmake-build-debug -N -L '^concurrency-sp04$'
```

## Solution-point labels

| Solution point | CTest label | Maintained coverage |
| --- | --- | --- |
| SP-01 worker and VM ownership | `concurrency-sp01` | allocator and worker lifecycle, active execution isolation, program generations, RXPA policy/ownership, persistent carriers, native return, sparse fallback and race stress |
| SP-02 channel machine contract | `concurrency-sp02` | RXAS/RXBIN feature contract, both concrete dispatches, provider registry, local channel lifecycle and explicit rejection of retired opcodes |
| SP-03 values, transfer and identity | `concurrency-sp03` | semantic graph/task-binding integrity, linker resealing, canonical channel values, transfer buffers, endpoint references and transferable task methods |
| SP-04 providers, endpoints and redirects | `concurrency-sp04` | local/process providers, crash replacement, byte endpoints, task-context adaptation, ADDRESS array/spawn transfer and both applicable VMs |
| SP-05 Level B control surface | `concurrency-sp05` | bridge inspection, interface metadata, class behavior, unsupported operations and direct task-context endpoint contract |
| SP-06 Level G compiler surface | `concurrency-sp06` | Level G gating, lowering, positive/negative semantics, imported methods/taskwork, signal cleanup and checked examples through the full toolchain |
| SP-07 HTTP client, server and LLM | `concurrency-sp07` | service interface, pooled/streaming/policy/TLS/codec clients, bounded server and failures, `crexx-rag` shapes, providers and ADDRESS integration |
| SP-08 failure and lifecycle | `concurrency-sp08` | cancellation, deadlines, crash/race replacement, sparse progress, signal unwind, backpressure, malformed/failing handlers and terminal close behavior |
| SP-09 build and documentation | `concurrency-sp09` | RexxDoc/API exposure, Level B bridge inspection and executable user examples on both VMs and optimization modes |

Tests can carry more than one solution-point label where one executable is
evidence for several invariants. CTest de-duplicates them in the umbrella
matrix.

## Deliberate exclusions

The manifest is the focused correctness matrix, not the whole release verdict.
E6 scaling measurements, governed Release performance comparisons, sanitizer
runs, full Debug CTest, installation, packaging and platform-specific package
smokes remain separate QA-B through QA-D evidence. Stress cases that are part
of correctness also carry `concurrency-stress` so they can be repeated without
using a test-name expression.
