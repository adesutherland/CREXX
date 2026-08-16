# cREXX initial concurrency QA closeout

Status date: 2026-08-16

Branch: `develop`

Frozen feature baseline: `b6a16dc3ae235eb959e926326fff2be8eb8e0ead`

This is the execution and evidence ledger for closing the Release 1 initial
concurrency surface. It does not authorize new concurrency capabilities and it
does not compete with the project ordering in `docs/ROADMAP.md`.

## Freeze and platform policy

- The task, channel, provider, Level B/G and HTTP surfaces are feature-frozen.
- QA may add tests, diagnostics, portability repairs and defect fixes. A
  language, RXAS/RXBIN, public ABI or architectural change still requires
  Adrian's separate approval.
- Linux and Windows execute a versioned, scripted candidate. They are not
  interactive development hosts. A platform failure is captured with a minimal
  reproducer and diagnostics, repaired and tested on Mac, then replayed on the
  affected host.
- Concurrency is called **initial** until an explicit later compatibility
  decision; passing QA does not silently make it stable.
- Every completed step receives focused QA and a local commit. No push is
  implied by this ledger.

## Closure gates

| Gate | Status | Exit evidence |
| --- | --- | --- |
| QA-A test readiness | active | solution-point review complete, direct gaps closed, enduring CTest manifest and platform commands ready |
| QA-B independent Mac closeout | pending | focused Debug/Release, both VMs/modes, sanitizer, stress and broad regression retained |
| QA-C Linux qualification | pending | clean frozen build, conformance matrix and install/package proof retained |
| QA-D Windows qualification | pending | clean frozen build, conformance matrix and install/package proof retained |
| QA-E publication decision | pending | limitations/packages reconciled and Adrian explicitly selects unpublished, published initial or deferred |

## Solution-point ledger

| ID | Solution point | Required review | Status |
| --- | --- | --- | --- |
| SP-01 | worker and VM ownership | execution state, allocator, RXPA policy, cancellation delivery and teardown | pending |
| SP-02 | channel machine contract | RXAS/RXBIN feature, opcodes, effects/signals, malformed images and both VM dispatches | pending |
| SP-03 | values, transfer and identity | canonical RXCV, bounds, receiver reconstruction, references, transfer buffers and sealed bindings | pending |
| SP-04 | providers, endpoints and redirects | local/process pools, crash replacement, byte endpoints, child processes and resource close ordering | pending |
| SP-05 | Level B control surface | pools, scopes, tasks, completions, channels, taskwork/context and explicit unsupported contracts | pending |
| SP-06 | Level G compiler surface | gating, lowering, recursion, short-circuiting, structured lifetime, imports and toolchain execution | pending |
| SP-07 | HTTP client, server and LLM | shared core, socket ownership, parsing/codec bounds, TLS, handler failure and provider integration | pending |
| SP-08 | cross-cutting failure and lifecycle | exactly one terminal completion, cancel/deadline/crash races, backpressure, no leak or hang | pending |
| SP-09 | build, installation and documentation | clean dependencies, installed linked images, packages, examples and stated limitations | pending |

Each row closes only with a `PASS`, `FIXED` or `DEFERRED` disposition naming
source reviewed, executable tests, commands/results and residual risk.

## Known QA-A gaps at freeze

- add a direct public-contract assertion for `.taskcontext.endpoint()`;
- add a direct assertion that `.taskscope.ask()` signals unsupported status
  `19`;
- replace the grep-derived concurrency test inventory with enduring CTest
  labels and one deterministic orchestration entry point;
- audit platform registration, fixture isolation, timeouts and diagnostics; and
- produce exact Linux and Windows validation commands and expected-result
  rules before either slow host is used.

## Deferred feature proposals

Concrete services and `.taskscope.ask()`, provider type `3`, a public provider
plugin ABI, pool telemetry, server TLS/readiness/background lifecycle, HTTP/2
and WebSockets are outside closure. They remain possible future entries in the
central roadmap only after separate design approval.
