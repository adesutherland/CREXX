# cREXX concurrency worklist

Status date: 2026-08-16  
Branch: `develop`  
Frozen feature baseline: `b6a16dc3a`

This is the detailed status ledger for cREXX tasks, channels, worker/process
providers, reusable byte endpoints and concurrent HTTP. `docs/ROADMAP.md` is
the sole project roadmap. Release 1 closure execution and platform evidence
are tracked in [`QA-CLOSEOUT.md`](QA-CLOSEOUT.md); historical development
stages and performance evidence are linked rather than reproduced here.

## Capability ledger

| ID | Capability | Implementation | Qualification/publication | Next action |
| --- | --- | --- | --- | --- |
| CONC-01 | execution-owned VM state and worker lifecycle | complete | locally qualified; native-carrier evidence exists on Mac, Linux and Windows | retain as foundation; portable public surface is tracked by CONC-11 |
| CONC-02 | RXAS/RXBIN channel contract | complete | both VMs locally qualified | maintain opcode, feature, effect, signal and malformed-image conformance |
| CONC-03 | canonical `ChannelValue` and typed transfer | complete | locally qualified and documented | retain canonical validation and receiver-ownership coverage |
| CONC-04 | local-thread and isolated-process task providers | complete | Mac qualified; four-platform Actions matrix green | complete native Linux/Windows public conformance under CONC-11 |
| CONC-05 | Level B pool/scope/task/channel/endpoint classes | implemented | locally qualified with RexxDoc, generated API reference and direct task-context endpoint coverage | retain explicit unsupported-operation and endpoint reconstruction assertions |
| CONC-06 | Level G task and `DO PARALLEL` language surface | complete | locally qualified, Level G-gated and documented with checked examples | retain positive/negative compiler and runtime coverage |
| CONC-07 | reusable byte endpoints, child processes and ADDRESS redirects | complete | locally qualified | retain cross-platform endpoint/process regression coverage |
| CONC-08 | bounded concurrent HTTP/TLS client | complete | Mac qualified and documented; published as initial with four-platform Actions green | complete native-host conformance under CONC-11 |
| CONC-09 | sealed task-binding validation cache | complete | locally qualified and performance-guard clean | retain cache miss/hit and ordinary single-thread regression coverage |
| CONC-10 | enduring documentation | complete | locally checked with generated API, examples, links and broad Debug regression | maintain references with implementation; native-host qualification remains CONC-11 |
| CONC-11 | solution-point QA, portable conformance and initial publication | active | initial publication and four-platform Actions/CodeQL pass; QA-A and Mac correctness/sanitizer/install closeout pass, while the Mac AC replay and native Linux/Windows executions remain | complete QA-B through QA-D in `QA-CLOSEOUT.md` |
| CONC-12 | services, actors, events and projections | post-Release-1 proposal | `.taskscope.ask` is unsupported | require separate design approval after concurrency closure |
| CONC-13 | open-host and extension providers | post-Release-1 proposal | provider type `3` and public plugin ABI unavailable | require separate design approval after concurrency closure |
| CONC-14 | pool saturation telemetry | post-Release-1 proposal | `queued()` and `running()` are unsupported | require separate semantics and portability approval |
| CONC-15 | stabilization and transfer tuning | evidence-led follow-up only | binding cache complete; wider payload/copy comparisons are not closure requirements | act only on a defect or governed performance finding |
| CONC-16 | HTTP implementation rationalisation and server | complete | one private Level B core and one public Level G client/server are Mac-qualified; Release guard clean | retain the architecture and take portable/server follow-ups through CONC-11 and new approved work |

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
- [x] A bounded clear-text HTTP/1.1 server keeps sockets on its controller and
  dispatches complete request records to `.httpservice .taskwork` targets.
- [x] The Level G client, server and LLM providers share one private
  binary-oriented `_rxhttpcore`; no public Level B HTTP convenience client
  remains.

## Deliberately unsupported today

- [ ] `.taskscope.ask` and service execution;
- [ ] `.taskpool.queued` and `.taskpool.running` statistics;
- [ ] cross-host provider type `3`;
- [ ] public channel-provider plugin registration ABI;
- [ ] typed service proxies, event hubs, topics and projections;
- [ ] detached ordinary tasks and blocking nested task waits; and
- [ ] public thread IDs, worker affinity or shared mutable VM values.

`.taskcontext.endpoint` reconstructs a worker-local byte endpoint from a
transferable type-4 provider reference. Its direct public contract is exercised
inside `.taskwork` through both VMs and optimization modes.

## CONC-10 documentation plan

- [x] Reconcile every public declaration and advertised behavior against source
  and executable tests.
- [x] Publish `docs/ai-context/CREXX_CONCURRENCY.md` as the holistic maintainer
  and AI authority.
- [x] Publish a programming-guide chapter with approachable, runnable examples.
- [x] Publish a formal language-reference chapter with positive and negative
  syntax examples.
- [x] Publish the Level B concurrency class guide and complete RexxDoc coverage.
- [x] Publish the current concurrent HTTP client/server guide, including
  handler examples, streaming and compression boundaries.
- [x] Add all chapters to `docs/index.md` and their book structures.
- [x] Replace internal development-stage terminology in enduring documents.
- [x] Reconcile the Level G catalogue, project roadmap and beta release notes
  without claiming portable publication before CONC-11.
- [x] Compile examples through `rxc`, `rxas`, `rxlink` and both VMs as
  applicable; build/check generated documentation and links.

### CONC-10 closeout evidence

Local macOS closeout on 2026-08-16 recorded:

- a 200-action focused rebuild of classlib, Level G and all documented/HTTP
  test artifacts;
- 41/41 class, documentation, generated-API and concurrent-HTTP tests across
  both VMs and both optimization modes;
- 41/41 compiler/channel concurrency tests, including all 19 fail-closed
  language cases and imported task-method/`.taskwork` routes;
- deterministic regeneration of `classlib-api.tex` with an unchanged SHA-256;
- local-link resolution for every Markdown file changed by CONC-10; and
- full Debug CTest, 2,192/2,192 passing in 340.09 seconds.

No TeX engine is installed on this Mac, so no book PDF was rendered. The book
structure includes the new chapters, and the Markdown sources, generated API,
links and executable examples were checked directly. This local documentation
closeout does not satisfy the portable conformance or release-publication work
in CONC-11.

## CONC-11 solution-point QA, portable conformance and publication

The frozen implementation is reviewed and qualified through QA-A to QA-E in
[`QA-CLOSEOUT.md`](QA-CLOSEOUT.md). Linux and Windows are validation hosts:
failures are reduced and repaired on the primary Mac development host, then
the corrected frozen candidate is replayed on the affected platform.

The exact clean-commit runners and expected-result rules are versioned in
[`qa/`](qa/). They configure fresh out-of-tree builds, enable live HTTP/TLS
verification, execute the maintained labels plus stress and broad CTest, and
prove installed and extracted-package toolchains without editing the target
checkout. The next-agent execution and independent review brief is
[`qa/INDEPENDENT-REVIEW-PROMPT.md`](qa/INDEPENDENT-REVIEW-PROMPT.md).

- [ ] Build the ordinary product and concurrency libraries on supported Linux,
  Windows and Mac toolchains.
- [ ] Run the same local-provider, process-provider, endpoint, ADDRESS, task
  lowering and HTTP fixture matrix on `rxbvm` and `rxtvm` where supported.
- [ ] Prove cancellation, deadline, crash replacement, teardown, TLS and
  bounded streaming behavior on each platform.
- [ ] Validate install/package contents and rebuilt linked images.
- [ ] Record limitations and make an explicit initial-publication
  decision. Do not call the surface stable as a side effect of passing QA.

## CONC-12 service and event work

This and the following provider/telemetry capabilities are post-Release-1
proposals, not an approved sequence after CONC-11. They remain recorded so the
frozen names and architectural positions are not mistaken for implementations.

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

The implemented architecture is:

- [x] retain one private, binary-oriented Level B request/response framing,
  parsing and codec core;
- [x] expose HTTP only through the Level G client and server classes, removing
  the independent public Level B convenience client after its Level G LLM
  consumers migrate;
- [x] let ordinary Level G client calls use the same task methods and bounded
  connection owners as explicitly parallel calls;
- [x] implement a controller-owned clear-text HTTP/1.1 server whose accepted
  sockets never cross executions;
- [x] dispatch complete `.httprequest` values to sealed factories whose user
  classes implement `.httpservice .taskwork`; `.httpservice` supplies the
  canonical typed dispatch adapter, users implement `handle`, and the class
  binds `.taskwork.run` to that adapter with one explicit method because the
  language does not provide interface inheritance;
- [x] leave `.taskscope.ask`, `.serviceref`, compiler, RXAS/RXBIN, RXVM and the
  socket instruction/API surface unchanged; and
- [x] require equivalent protocol, TLS-client, malformed-message, client/server
  integration, both-VM, optimizer, sanitizer and governed Release evidence.

The initial server is bounded, controller-owned and buffered. It supports
clear-text HTTP/1.1 plus limited HTTP/1.0 compatibility. Server TLS, HTTP/2,
WebSockets, detached/background serving and server request/response streaming
require later proposals. Without a
multi-socket readiness primitive, the first server uses bounded nonblocking
scans plus short blocking waits; idle CPU and response latency are explicit
acceptance evidence rather than assumed properties.

### CONC-16 closeout evidence

The client/LLM consolidation is committed at `229b3e8b4`; the bounded server
and protocol extensions are committed at `2212cf427`. Focused Debug and Release
HTTP/client/server/LLM matrices each passed 35/35. The public server and its
negative handler fixture each passed on `rxbvm` and `rxtvm`, optimized and
unoptimized, under Apple ASan; LeakSanitizer is unavailable on that host.

The mandatory profiling-off Release comparison found no single-thread guard
breach. Confirmation deltas were between -0.681% and +0.306% across Sieve and
RexxCPS on both VMs, with only the -0.681% result statistically clear and
favourable. All 48 recorded server scenarios passed. Exact commands, raw
samples, artifact checks and limitations are retained in
[`2026-08-16-conc-16-http-server-first-release-verdict`](../performance/evidence/2026-08-16-conc-16-http-server-first-release-verdict/).

Remaining HTTP work is not an unfinished part of CONC-16. Linux/Windows and
package qualification belong to CONC-11. Server TLS, a readiness primitive,
detached/background lifecycle, server request/response streaming, HTTP/2 and
WebSockets require separately approved proposals.

The enduring-documentation closeout then added the server classes to generated
API output and checked 99 local Markdown links. Its focused Debug documentation,
HTTP and LLM matrix passed 51/51; the final Release HTTP/LLM/ADDRESS matrix
passed 36/36. Repeated API generation produced SHA-256
`1c90e1ad4ae48132cf635b8aa8d80bc64fa016f8eed2503fd7d1b20ad6f7a310`;
no TeX engine is installed on this Mac. A broad Debug run exposed that the LLM
ADDRESS demonstrator still
constructed transient providers without closing their new HTTP pools. The
demonstrator now closes every provider and its integration fixture is linked
through `rxc`, `rxas`, `rxlink` and `rxvm`; that test passes in Debug, Release
and Apple ASan.

The corrected 2,200-test Debug sweep passed 2,198 and timed out two tests under
the concurrent generated-artifact rebuild: `ts_http_server_rxtvm_opt` at its
90-second limit and unrelated `rxpa_signature_diagnostics` at 120 seconds.
Serial confirmation passed them in 1.82 and 12.67 seconds respectively. No
RXVM source changed in this slice, so these are recorded as rebuild-load
slowdowns rather than product failures.

## Historical evidence

The former development plan and exact Mac closeout records are indexed in
[`history/README.md`](history/README.md). Raw timing and QA artifacts remain
under `performance/evidence/` because they were produced under performance
governance and must retain their original paths.
