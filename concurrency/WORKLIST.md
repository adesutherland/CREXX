# cREXX concurrency worklist

Status date: 2026-08-16  
Branch: `develop`  
Implementation baseline: `183a730dc`

This is the sole live worklist for cREXX tasks, channels, worker/process
providers, reusable byte endpoints, concurrent HTTP and future service/host
providers. Historical development stages and performance evidence are linked,
not reproduced as the current control model.

## Capability ledger

| ID | Capability | Implementation | Qualification/publication | Next action |
| --- | --- | --- | --- | --- |
| CONC-01 | execution-owned VM state and worker lifecycle | complete | locally qualified; native-carrier evidence exists on Mac, Linux and Windows | retain as foundation; portable public surface is tracked by CONC-11 |
| CONC-02 | RXAS/RXBIN channel contract | complete | both VMs locally qualified | maintain opcode, feature, effect, signal and malformed-image conformance |
| CONC-03 | canonical `ChannelValue` and typed transfer | complete | locally qualified | add enduring API/reference documentation |
| CONC-04 | local-thread and isolated-process task providers | complete | Mac qualified | complete Linux/Windows public conformance under CONC-11 |
| CONC-05 | Level B pool/scope/task/channel/endpoint classes | implemented | locally qualified; documentation incomplete | finish RexxDoc and generated API reference; keep unsupported operations explicit |
| CONC-06 | Level G task and `DO PARALLEL` language surface | complete | locally qualified and Level G-gated | publish formal syntax and compile-checked examples |
| CONC-07 | reusable byte endpoints, child processes and ADDRESS redirects | complete | locally qualified | retain cross-platform endpoint/process regression coverage |
| CONC-08 | bounded concurrent HTTP/TLS | complete | Mac qualified; experimental publication pending | correct docs, complete portable conformance and resolve HTTP architecture under CONC-16 |
| CONC-09 | sealed task-binding validation cache | complete | locally qualified and performance-guard clean | retain cache miss/hit and ordinary single-thread regression coverage |
| CONC-10 | enduring documentation | active | not complete | execute the documentation plan below |
| CONC-11 | portable conformance and experimental publication | pending | Mac complete; public local/process/HTTP matrix incomplete | qualify Linux and Windows, then make the release/package decision |
| CONC-12 | services, actors, events and projections | surface reserved | `.taskscope.ask` is unsupported | specify and implement one single-owner service before typed proxies/events |
| CONC-13 | open-host and extension providers | reserved | provider type `3` and public plugin ABI unavailable | select protocol from interoperability evidence and prove a non-Rexx actor |
| CONC-14 | pool saturation telemetry | reserved | `queued()` and `running()` are unsupported | specify snapshot semantics and provider portability before implementation |
| CONC-15 | stabilization and transfer tuning | active as evidence-led follow-up | binding cache complete; wider payload/copy work open | route timing through performance governance without moving product ownership there |
| CONC-16 | HTTP implementation rationalisation | decision pending | two independent clients exist; LLM providers use the synchronous one | compare retention, common-core extraction and migration options before code changes |

The evidence behind each status is reconciled in
[`IMPLEMENTATION-STATUS.md`](IMPLEMENTATION-STATUS.md). A declaration by itself
does not count as an implemented or supported capability.

## Implemented surface

- [x] `chanopen`, `chanstart`, `chanwait`, `chancancel` and `chanclose` use
  opcodes `650..654` with RXBIN 007 channel feature bit `1 << 3`.
- [x] Core type `1` provides bounded local task execution.
- [x] Core type `2` provides bounded isolated-process task execution.
- [x] Core type `4` provides reusable bounded byte endpoints.
- [x] Core type `5` provides structured child-process execution.
- [x] RXCV covers null, boolean, integer, float, decimal, string, binary,
  arrays, records and provider references with canonical validation.
- [x] Level B exposes pools, scopes, tasks, targets, task work/context,
  completions, channels, requests, values/codecs, endpoints, service references
  and transfer buffers.
- [x] Level G exposes task procedures, transferable task methods, explicit task
  targets, ordinary task calls and both `DO PARALLEL` forms.
- [x] Typed primitive and transferable object arguments/results materialize in
  receiver/controller-owned storage.
- [x] Child-process redirects use the common endpoint/channel foundation; the
  retired pre-release spawn/redirect instruction slots remain reserved.
- [x] Concurrent HTTP provides bounded admission, reusable single-owner
  connections, verified TLS, safe headers, explicit retry/redirect/ambiguity
  policy, fixed/chunked request streams, response streams and bounded
  gzip/zlib/DEFLATE decoding.
- [x] The concurrent generation/embedding fixture matches the required
  `crexx-rag` request, authentication, idempotency and response shapes.

## Deliberately unsupported today

- [ ] `.taskscope.ask` and service execution;
- [ ] `.taskpool.queued` and `.taskpool.running` statistics;
- [ ] cross-host provider type `3`;
- [ ] public channel-provider plugin registration ABI;
- [ ] typed service proxies, event hubs, topics and projections;
- [ ] detached ordinary tasks and blocking nested task waits; and
- [ ] public thread IDs, worker affinity or shared mutable VM values.

`.taskcontext.endpoint` has an implementation that reconstructs a byte endpoint
from a provider reference. CONC-10 must verify its executable coverage before
the enduring reference marks it supported.

## CONC-10 documentation plan

- [x] Reconcile every public declaration and advertised behavior against source
  and executable tests.
- [x] Publish `docs/ai-context/CREXX_CONCURRENCY.md` as the holistic maintainer
  and AI authority.
- [x] Publish a programming-guide chapter with approachable, runnable examples.
- [x] Publish a formal language-reference chapter with positive and negative
  syntax examples.
- [x] Publish the Level B concurrency class guide and complete RexxDoc coverage.
- [x] Publish the current concurrent HTTP guide, including streaming and
  compression boundaries.
- [x] Add all chapters to `docs/index.md` and their book structures.
- [ ] Replace internal development-stage terminology in enduring documents.
- [ ] Reconcile the Level G catalogue, project roadmap and beta release notes
  without claiming portable publication before CONC-11.
- [ ] Compile examples through `rxc`, `rxas`, `rxlink` and both VMs as
  applicable; build/check generated documentation and links.

## CONC-11 portable conformance and publication

- [ ] Build the ordinary product and concurrency libraries on supported Linux,
  Windows and Mac toolchains.
- [ ] Run the same local-provider, process-provider, endpoint, ADDRESS, task
  lowering and HTTP fixture matrix on `rxbvm` and `rxtvm` where supported.
- [ ] Prove cancellation, deadline, crash replacement, teardown, TLS and
  bounded streaming behavior on each platform.
- [ ] Validate install/package contents and rebuilt linked images.
- [ ] Record limitations and make an explicit experimental-publication
  decision. Do not call the surface stable as a side effect of passing QA.

## CONC-12 service and event work

- [ ] Specify service creation, discovery, identity lifetime and serialized
  accepted-call ordering.
- [ ] Define cancellation, deadline, idempotency and ambiguous-outcome behavior.
- [ ] Implement one local single-owner service behind `.taskscope.ask`.
- [ ] Prove state isolation, ordering and teardown before adding typed proxies.
- [ ] Design events/projections as explicit eventually consistent libraries,
  not transparent shared globals.

## CONC-13 open-host and provider extensions

- [ ] Compare open framing, encoding, authentication and transport options.
- [ ] Specify version, capability, identity, backpressure, cancellation and
  reconnect behavior.
- [ ] Implement provider type `3` and one independent non-Rexx actor.
- [ ] Review a public provider-plugin descriptor only after the core protocol is
  interoperable and unload/lifetime behavior is proven.

## CONC-16 HTTP architecture decision

The repository currently has:

- synchronous Level B `.rxhttp..httpclient`, used by the existing Level G LLM
  provider classes; and
- concurrent Level G `.rxfnsg..httpclient`, implemented independently over
  tasks, endpoints and `rxsocket`.

Before changing either implementation:

- [ ] compare retaining two stacks with extracting a shared Level B
  request/response framing and codec core;
- [ ] identify source-compatibility and package layering constraints;
- [ ] decide whether the LLM providers should remain synchronous, accept an
  injected transport, or migrate to the concurrent client; and
- [ ] require equivalent protocol, TLS, malformed-response and portability QA
  for the selected architecture.

## Historical evidence

The former development plan and exact Mac closeout records are indexed in
[`history/README.md`](history/README.md). Raw timing and QA artifacts remain
under `performance/evidence/` because they were produced under performance
governance and must retain their original paths.
