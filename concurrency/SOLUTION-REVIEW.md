# Initial concurrency solution-point review

Status date: 2026-08-16

Scope: SP-01 through SP-09 at the initial concurrency feature freeze. This is
the source-and-test review required by QA-A; it does not substitute for the
clean Release, sanitizer, performance, installation or non-Mac qualification
in QA-B through QA-D.

## Review method and common verdict

The review traced each public claim from its enduring documentation through
the implementation seam and its labelled executable tests. It checked
ownership and teardown paths, bounds and invalid inputs, terminal-state races,
Level G gating/lowering, the complete `rxc` -> `rxas` -> `rxlink` -> VM path,
and HTTP resource ownership. Historical plans were used only to locate design
decisions; current source, API documentation and executable tests determined
the dispositions below.

After the two repairs recorded below, the deterministic Debug entry point

```sh
cmake --build cmake-build-debug --target concurrency-qa --parallel 10
```

completed a 1,079-step dependency rebuild and passed 180/180 CTest executions
on Mac on 2026-08-16. The total is 179 unique concurrency tests plus
the serial linked-runtime fixture. The umbrella matrix includes six explicit
stress cases and de-duplicates tests that support more than one solution point.

## Dispositions

| ID | Disposition | Source reviewed | Executable evidence and residual risk |
| --- | --- | --- | --- |
| SP-01 | PASS | `rxvmworker.c`, `rxvmmemory.c`, `rxvmprogram.c`, `rxvmexecutor.c`, RXPA loading/session paths and platform doorbells | `concurrency-sp01`: 41/41 within the full pass. Covers worker ownership, allocator families, generation isolation, bounded queues, cancellation/deadline/kill/shutdown priority, quarantine, join-before-destroy, native return and sparse fallback. Residual: Windows APC and Linux carrier behavior require QA-C/D replay. |
| SP-02 | PASS | RXOP declarations/effects/signals, RXBIN 007 channel feature handling, assembler validation, both VM dispatch policies/handlers and `rxvmchannel.c` | `concurrency-sp02`: 18/18. Opcodes 650..654 retain one common shape, optimizer barriers and no signal side channel; both dispatches execute them, malformed/unsupported images fail closed and retired spawn/redirect opcodes remain rejected. Residual: binary portability replay remains QA-C/D. |
| SP-03 | FIXED | RXCV validation/reconstruction, transfer buffers, provider references, semantic graph sealing, linker resealing and executor task-plan caching | `concurrency-sp03`: 11/11. Direct graph tamper tests, typed transfer tests and repeated identical sealed submissions now prove integrity and one retained per-worker/per-result plan. Completion wrapping now accounts for its two added depth levels and the final member/document budget; depth 64 is accepted and depth 65 rejected. Residual: cache performance is measured separately in QA-B. |
| SP-04 | PASS | provider registry, local/process providers, byte endpoints, child-process provider, task-context endpoint adapter and ADDRESS redirect conversion | `concurrency-sp04`: 24/24. Covers provider pinning, owner/generation capabilities, process crash replacement and outcome classification, endpoint rights/half-close/cancel, task-local adaptation and spawn/redirect arrays. Residual: process creation, pipe and path branches require Linux/Windows qualification. The extension-provider ABI remains private. |
| SP-05 | PASS | `lib/classlib/Concurrency.crexx` declarations, RexxDoc and implementations down to the five RXAS instructions | `concurrency-sp05`: 8/8. Covers pools, scopes, tasks, completions, channel values, transfer buffers, endpoints and direct `.taskcontext.endpoint()`. `.taskpool.queued()`, `.taskpool.running()` and `.taskscope.ask()` deterministically signal unsupported status 19. Residual: no automatic finalizers are promised; user code must close lifecycle owners. |
| SP-06 | PASS | parser level selection, task validation/binding, `rxcp_task_lower.c`, signal cleanup and imported target handling | `concurrency-sp06`: 52/52. Covers Level G-only gating, sibling submission/join, `DO PARALLEL`, explicit targets, imports, methods, `.taskwork`, pending-result restrictions, scope reuse, signal/control exits and optimizer/no-optimizer output through both VMs. Direct self-recursion remains an ordinary procedure call; cross-task blocking waits remain rejected. Residual: unrelated compiler regressions are covered by the broad QA-B sweep. |
| SP-07 | PASS | `_rxhttpcore`, `.httpclient`, connection-owner taskwork, `.httpserver`, `.httpservice`, LLM providers and ADDRESS integration | `concurrency-sp07`: 39/39. The one private Level B protocol core is shared by Level G client/server/LLM users. Tests cover bounded pooling, binary/streaming bodies, policy, redirects/retries, gzip/zlib/raw DEFLATE, loopback server failures, handler task failure and `crexx-rag` shapes. Remaining `word()` uses parse actual HTTP words; no integer-index table remains. Residual: live external TLS is opt-in and portable TLS/package proof remains QA-C/D. |
| SP-08 | FIXED | terminal publication and observation across executor/channel/process/byte/child providers, structured compiler unwind and HTTP teardown | `concurrency-sp08`: 49/49, including six stress cases. The review confirmed one terminal completion, correlated cancellation, completion ordering, backpressure and close accounting. The provider-completion depth repair prevents a provider from emitting a wrapper that the receiver must reject. Residual: sanitizer and longer repetitions remain QA-B; platform race replay remains QA-C/D. |
| SP-09 | PASS | CMake registration/artifact dependencies, installation declarations, current user/language/AI/API documents and executable examples | `concurrency-sp09`: 19/19. Docs examples compile, assemble, link and run optimized/unoptimized on both local VMs, and the manifest has complete timeouts/failure sentinels. Enduring documents use “initial”; historical Gate F names remain only in the history archive and executable test identifiers. Residual: clean install/package proof on each platform remains QA-B through QA-D. |

## Repairs made during review

1. The implementation-status matrix claimed direct cache coverage although the
   retained evidence directly proved binding validation and performance, not
   cache reuse. A private quiescent-worker cache-entry observation and repeated
   sealed-submission test now prove reuse without adding any instruction or
   counter to the hot submission path.
2. Provider result/details nodes were validated as standalone trees. The
   completion record adds two semantic depth levels, so a standalone-legal
   tree could produce a noncanonical completed document. Validation now starts
   at the inserted depth, shares the final record's member budget and rejects a
   completed document above the 16 MiB ceiling. The provider-registry test owns
   the exact maximum/excessive boundary regression.

Neither repair changes Rexx syntax, the public class surface, RXAS/RXBIN, the
installed ABI or the approved ownership model.

## Deferred, not failed

Concrete services and `.taskscope.ask()`, pool telemetry, provider type 3, a
public provider plugin ABI, server TLS/readiness/background lifecycle, HTTP/2
and WebSockets remain outside the frozen initial scope. Cross-platform
qualification and publication are later closeout gates, not implicit claims
of this Mac source review.
