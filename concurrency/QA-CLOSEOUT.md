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

## QA-A gap closure

- [x] Add a direct public-contract assertion for `.taskcontext.endpoint()`.
  `testTaskContextEndpoint.crexx` passes through the full linked toolchain on
  `rxbvm` and `rxtvm`, optimized and unoptimized.
- [x] Add a direct assertion that `.taskscope.ask()` signals
  `CHANNEL_ERROR`, code `26`, unsupported status `19`.
- [x] Replace the grep-derived concurrency test inventory with enduring
  `concurrency` and `concurrency-sp01` through `concurrency-sp09` CTest labels.
  [`TEST-MANIFEST.md`](TEST-MANIFEST.md) defines the matrix and the
  `concurrency-qa` build target constructs every declared artifact before
  starting CTest.
- [x] Audit platform registration, fixture isolation, timeouts and diagnostics.
  Persistent-carrier build dependencies now follow their macOS/Linux/Windows
  registration guard; every manifest test has a timeout and failure-output
  sentinel; the shared linked-runtime fixture is serial and built before
  CTest. Toolchain cases use unique workspaces or the existing runtime lock,
  child-process cases use no shared scratch files, and HTTP fixtures use
  kernel-assigned loopback ports. The last fixed-range HTTP fixture was moved
  to port `0` during this audit.
- [ ] Produce exact Linux and Windows validation commands and expected-result
  rules before either slow host is used.

Focused Mac evidence for the first two items: build targets
`testConcurrency` and `testTaskContextEndpoint`; then CTest selected
`testConcurrency_(noopt|opt)` and every `testTaskContextEndpoint_*` case. All
six executable cases passed on 2026-08-16 (the endpoint matrix covers both VM
variants and both optimization modes).

The labelled-manifest audit on Mac selected 179 unique correctness tests: all
nine solution-point labels were non-empty, every solution test carried the
umbrella label, and no umbrella test lacked a solution point. The
`concurrency-qa` entry point then built its declared artifacts and passed all
179 cases plus the required linked-runtime fixture (180/180) on 2026-08-16.
After the portability/isolation changes, the same entry point again passed
180/180 (89.24 seconds of CTest time after its dependency rebuild). Generated
CTest metadata reported zero manifest tests without a timeout and zero without
the `FAIL:`/`BAD:` diagnostic sentinel. A generated-command audit found no
direct shell, `/tmp`, Unix file-command or `.so` dependency in the labelled
test commands.

## Deferred feature proposals

Concrete services and `.taskscope.ask()`, provider type `3`, a public provider
plugin ABI, pool telemetry, server TLS/readiness/background lifecycle, HTTP/2
and WebSockets are outside closure. They remain possible future entries in the
central roadmap only after separate design approval.
