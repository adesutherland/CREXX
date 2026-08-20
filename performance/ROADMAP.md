# cREXX PERF3 performance roadmap

Approved: 2026-07-31

Status: **complete historical PERF3 programme ledger — retired 2026-08-17**. The project-wide ordering
authority is [`docs/ROADMAP.md`](../docs/ROADMAP.md); this file retains
performance activity status, evidence links and idea dispositions without
creating a competing area roadmap. Adrian approved the programme
and transfer boundary on 2026-07-31, then accepted the PERF3-01 current-product
evidence boundary and ranked panel on 2026-07-31. PERF3-02's clean-host panel,
C1b detached-guard proof, C2E2 storage-identity infrastructure and
infrastructure-enabled R1 repanel are complete. Adrian selected the composed
C1abc production ladder on 2026-08-01. The disposable option mask and broad
correctness-negative branch are removed from the ordinary compiler while the
checksum-closed replay source preserves every investigated option.

Concurrency product scope, portability and publication are not governed here.
Their detailed status and evidence ledger is
[`concurrency/WORKLIST.md`](../concurrency/WORKLIST.md), while project ordering
remains in `docs/ROADMAP.md`. This file retains the allocator/worker performance
history, regression verdicts and evidence links that informed the
implementation.

Historical terminology note: the internal Gate E/F and F1-style labels below
name dated implementation slices, benchmarks and evidence directories. They do
not express current product status or documentation ownership. Use the
concurrency worklist, implementation matrix and enduring guides for those
questions; do not infer that a capability is open, closed or released from a
historical performance paragraph.

On 2026-08-17 the PERF3 closeout benchmark expansion exposed an optimized-code
correctness defect in the accepted NR-26 copy/dead-store fixed point. AWFY
Queens and a reduced nested-inline regression failed under the unmodified
`c38a5d184` compiler because a later fixed-point rung replaced a valid deep
substitution with an intermediate register whose defining store had been
omitted. The repair uses reaching-definition order to reject only that
unmaterialised path. It retains the supported inline shape, valid old-value
forwarding and the prior NR-26 reductions. Adrian accepted the first ordinary
profiling-off Release verdict; post-acceptance closeout passes full Debug
2,224/2,224 and focused Release 7/7. Evidence:
[`2026-08-17 rxc flow-copy repair`](evidence/2026-08-17-rxc-flow-copy-fixed-point-repair-first-release-verdict/).

PERF3 closed on 2026-08-17 with the accepted compiler repair, portfolio-v2
qualification, the static/private fusion registry and a fresh profiling-off
Release Apple scorecard. The product v2 common-five geometric means are
4.897751 versus ooRexx and 1.543319 versus decimal NetRexx; Richards remains a
deficit against both and Permute remains below NetRexx. Because Base64-v2 has a
new source identity, these figures start a new series rather than extending
the v1 aggregate. Evidence:
[`portfolio-v2 Mac closeout`](evidence/2026-08-17-perf3-closeout-current-mac-v2/).
The durable successor order now lives only in [`docs/ROADMAP.md`](../docs/ROADMAP.md).

On 2026-08-10 Adrian approved the PERF3-13 E3b-P1 isolated branch-free
load-binding comparison. Process-reentrant RXPA procedures remain permanently
direct. Legacy procedures may remain direct with one legacy-capable executor;
the second such executor causes one sticky, quiescent process-wide rebind of
legacy procedures to the recursive locked adapter. An executor containing only
process-reentrant plugins does not trigger that transition. This authorizes the
isolated ceiling/state-machine proof only; another production edit, P2 sessions
and Gate F remain closed.

The reserved-host E3b-P1 isolated comparison subsequently passed its
machine-level ceiling: the selected indirect direct invoker is inconclusive at
-0.489662% paired mean versus raw direct, with a 95% interval from -1.228728%
to +0.249404% and no 3% guard hit. The locked legacy invoker is clearly adverse
by +20.117255%, confirming that it should be introduced only by the cold sticky
second-legacy-executor transition. A production candidate is recommended but
still requires explicit approval and its own mandatory both-VM Release verdict.

Adrian approved that candidate and accepted its guard-clean ordinary-Release
verdict on 2026-08-10. The integrated branch-free form removes the rejected
14-20% call losses: product `rxbvm` process-reentrant calls are inconclusive at
+1.368096%, legacy calls are inconclusive at +0.104869%, and guard `rxtvm`
measures +2.175049% reentrant overhead, below the 3% guard, with legacy calls
inconclusive at -0.413910%. All 312/312 formal processes pass; Sieve, canonical
RexxCPS, lifecycle and artifact guards remain clear. Full Debug closeout passes
2,017/2,017 and focused Release passes 11/11. Rebuilt VMs are byte-identical to
the verdict artifacts. E3b-P1 is accepted for local commit.

Adrian subsequently approved and accepted E3b-P2. The separate optional V2
query adds load-bound per-procedure policy and nested-safe per-VM sessions while
preserving the installed initializer/call ABI and old-host legacy behavior.
`rxmath` proves mixed policy; ODBC proves isolated external-resource sessions,
opaque prepared statements and a default session for old hosts. The 156-process
Release verdict is guard-clean: the permanent direct path has no regression,
empty session calls add 2.92-4.08 ns, lifecycle is neutral and both VM files add
432 bytes. Mac closeout initially passes full Debug 2,032/2,032 and focused
Debug, Apple ASan and Release panels. After separately approved installation,
unixODBC and SQLite real-driver tests pass through both VMs in Debug, Apple ASan
and Release; the final ODBC-enabled full Debug suite passes 2,034/2,034 and the
deterministic mock retains concurrency/failure/teardown authority. E3 is
complete on Mac. Linux, Windows and clean-runner ODBC proof remains a
publication follow-up. Public workers/channels and Gate F remain closed pending
a separate approval.

On 2026-08-11 Adrian approved and completed E4a, the non-sharing control and
sealed-layout audit, then approved E4b and accepted its guard-clean first
ordinary-Release verdict. The internal bytecode-only implementation installs a
runtime-owned synchronized catalogue of reference-counted append-only sealed
generations. Canonical module files, instructions, constant pools, semantic
graphs and descriptors are shared; globals, procedures/frames, execution
images, bindings and caches remain worker-owned. Native/plugin modules remain
under the E3 lifecycle and outside this first sharing slice. The fixture removes
the complete 2,480-byte audited duplicate floor across two contexts while
retaining the 569-byte per-worker overlay floor. The same-session single-worker
matrix is neutral: only `rxbvm` Sieve is clearly adverse at +0.374%, inside the
3% guard; lifecycle, RSS and artifact guards remain clear. Mac closeout passes
focused Debug 11/11, Apple ASan 3/3, full Debug 2,037/2,037 and focused Release
11/11. Rebuilt Release VMs are byte-identical to the accepted timing artifacts.
E4 was complete on Mac at this point; portable proof, E5/E6, public
workers/channels and Gate F were then separately gated.

Adrian then approved a bounded E5 carrier investigation and selected the clean
macOS native-doorbell PoC for a separate `mthread` branch. A producer targets a
persistent worker with `pthread_kill(SIGURG)`; a bounded Apple handler ORs
`CANCEL` into that worker's existing E4 execution-local interrupt word. No
poll, targetability branch or loop selector is added to ordinary dispatch. The
clean first verdict passes all 156 processes with no 3% guard hit, focused
Debug/Release pass 3/3 and both engines complete 1,000 cancellations at 6 us
median latency. Full Debug passes 2,039/2,039, fresh supported Apple
AddressSanitizer passes 3/3 and the complete profiling-off Release build plus
combined E4/E5 focused panel passes 6/6. This proves physical delivery only.
The POSIX proof now also passes on Intel Linux with GCC and Clang: focused
stress, 4,000 total cancellation samples, handler/reachable-code audit and the
unchanged generated E4 `run` edge pass. The hot and loaded host makes both
E4-versus-PoC campaigns overall noisy/inconclusive; Adrian accepts them as a
physical-PoC pass with no demonstrated harm at PoC precision, not as a
guard-clean production claim. The affected GCC Debug targets rebuilt after
acceptance, but a broad 2,039-test closeout was stopped at 37 tests after
generated-artifact and parser-contract timeouts on the same stressed host; it
is incomplete host-capacity evidence, not an E5 failure or broad pass. Future
compiler/performance selection and broad Linux closeout begin after reboot on
a settled, quiet, reserved host. The later industrial E5 closure accepts this
Linux evidence without repetition; Linux ARM64 testing is not required for E5.

The Windows 11 investigation has now proved the special-APC physical carrier
and completed the corrected fallback-owner PoC. Non-targetable/local execution and
native-deliverable targetable workers retain E4. Only a targetable worker on a
host without prompt native delivery selects the sparse owner before
preparation. The first every-instruction outlined-switch reconstruction passed
focused cancellation correctness but is rejected at roughly 64%/102%
`rxbvm`/`rxtvm` forced-fallback slowdown and about 11% product growth. The
functionally effective macOS sparse learning is therefore restored as the
fallback direction: request entry, taken static/indirect backedges, call and
return boundaries, and native/plugin return. Its exact old source/opcode ledger
was not retained, so the current implementation carries an explicit semantic
instruction audit and RXAS progress fixture. GCC Release and Clang/MSVC-ABI
Debug pass 19/19 focused tests; MSVC Debug passes 13/13. GCC forced-fallback
latency is 2.9-3.0 us median over 1,000 samples per concrete engine. The
targetable fallback costs +16.09%/+14.89% paired mean/median in `rxbvml` and
+5.69%/+5.07% in `rxtvml`; the E4 owner for non-targetable and native-capable
  execution is unchanged. The duplicate owner adds the accepted 19.4-20.0%
  product size. All four core products build under GCC, MSVC and Clang/MSVC-ABI,
  and each `rxc` compiles the former access-violation reproducer optimized and
  no-opt. The two MSVC-ABI builds disable the optional parser-mode dependency,
  whose sibling source still includes POSIX `unistd.h`; MinGW GCC validates the
  default parser-enabled configuration.

Adrian accepted the industrial E5 implementation and its cleared-host Release
verdict on 2026-08-13. The private executor integrates correlated generation
mailboxes, copied logical integer/string requests, typed integer completion,
deterministic cancellation/deadline/kill/shutdown priority, quarantine and
join with the selected native and sparse carriers. `rxbvml` is favourable or
neutral; `rxtvml` records accepted +4.218% direct, +3.549% one-worker and
+3.013% two-worker paired-mean elapsed costs. The programme has exhausted the
carrier alternatives, and the sparse solution is slower and structurally less
desirable, so the computed-goto/multithreading loss is accepted rather than
reopened. Mac QA passes focused normal and Apple ASan checks, complete Debug
and Release builds, 2,055/2,056 broad Debug tests plus a clean serial recovery
of the sole parser-thread timeout, and 22/22 focused Release checks after an
initializer repair found by the broad sweep. Existing Linux and Windows
evidence is accepted; Linux/Windows ARM testing is not required. E5 closure
commit `9f5bb579a` is integrated into `develop` by merge `795e58edb` for the
authorized publication. E6, public workers/channels and Gate F remain closed
until the published develop GitHub build is green. Published head `5ba282129`
subsequently passed Build CREXX run `31733322358` across Windows x64, Linux
x64, macOS arm64 and macOS x86_64, plus CodeQL run `31733322413`. Adrian
approved E6 reclamation/scale selection on 2026-08-13 for direct execution on
`develop`, with the mandatory first Release verdict retained as an interactive
stop. Adrian selected strict C0 ownership and authorized E6 closure on
2026-08-14. The cleaned form removes C1/C2, passes focused
Debug/Apple-ASan/Release 49/49, complete Debug and Release builds, and full
Debug CTest 2,080/2,080. Gate F remained closed at that E6 checkpoint and was
subsequently approved for staged implementation on 2026-08-14 as recorded
below.
Evidence:
[`E5 industrial closeout`](evidence/2026-08-13-perf3-13-gate-e-e5-industrial-closeout/),
[`E6 C0 selection and closeout`](evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/).

The Gate F public design, user model and staged implementation were approved by
Adrian on 2026-08-14. F0-S has locked the exact specification, coherence matrix
and compile-checked Level B declaration oracle before the first opcode/runtime
edit. F1a-F1f are now complete: the five RXAS operations execute on both
concrete VMs with full canonical RXCV, typed register images,
generation-checked capabilities, private provider conformance,
deadline/scope/lifecycle control, explicit Level B classes, reusable byte
endpoints and structured child processes. ADDRESS now uses the common channel
operations and the six pre-release process/redirect mnemonics are retired with
slots `466..471` reserved. Type `2` supplies bounded warm isolated workers with
one fresh VM execution per task, preserves multi-graph callable identity in
bytecode-only snapshots and provides exactly-once crash/cancel/deadline
completion. F1f adds the Level G-gated task/parallel surface and sealed task
procedure, transferable receiver and `.taskwork` factory bindings. F3C1 then
recovers repeated sealed-binding validation with a bounded worker-local cache:
tiny-task latency improves +39.076017%/+40.340609% on `rxbvml`/`rxtvml`, no
cell hits the adverse guard and all broad Mac QA passes. Adrian activated F1g
on 2026-08-15. F1g-A closes typed codec-backed task results so the approved
task method can return an independent `.httpresponse`, preserves imported task
method/Level G semantics and reseals imported use-site bindings against the
final graph. F1g-B adds the bounded transferable `.httpclient.pooled` proxy,
long-lived single-socket taskwork owners and fixed canonical admission
descriptors over type-4 endpoints. F1g-C adds transferable safe headers,
bounded request policy, verified TLS, same-origin redirect/retry rules,
ambiguity diagnostics and response/header/request limits. Its receive-status
repair also prevents timeout/EOF/would-block from publishing uninitialised
socket buffers. F1g-D completes bounded fixed/chunked request and response
streams, pure-Level-B bounded gzip/deflate decoding, typed task arguments and
the concurrent `crexx-rag` generation/embedding fixture without adding an
HTTP opcode or provider type. Final Mac qualification passes Debug
2,175/2,175, Release Gate F 62/62 and focused ASan 39/39 after a complete
sanitizer build. Exact-final task and ordinary single-thread Release panels
have no 3% guard hit. Evidence:
[`F1a/F1b first Release verdict and closeout`](evidence/2026-08-14-perf3-13-gate-f-f1ab-first-release-verdict/),
[`F1c first Release verdict and closeout`](evidence/2026-08-14-perf3-13-gate-f-f1c-first-release-verdict/),
[`F1d first Release verdict and closeout`](evidence/2026-08-15-perf3-13-gate-f-f1d-first-release-verdict/),
[`F1e first Release verdict and closeout`](evidence/2026-08-15-perf3-13-gate-f-f1e-first-release-verdict/),
[`F1f first Release verdict and closeout`](evidence/2026-08-15-perf3-13-gate-f-f1f-first-release-verdict/),
[`F3C1 first Release verdict and closeout`](evidence/2026-08-15-perf3-13-gate-f-f3c1-task-binding-cache-first-release-verdict/), and
[`F1g-A first Release verdict and closeout`](evidence/2026-08-15-perf3-13-gate-f-f1g-a-typed-task-results-first-release-verdict/), and
[`F1g-B first Release verdict and closeout`](evidence/2026-08-15-perf3-13-gate-f-f1g-b-pooled-http-owner-first-release-verdict/), and
[`F1g-C first Release verdict and closeout`](evidence/2026-08-15-perf3-13-gate-f-f1g-c-http-policy-first-release-verdict/), and
[`F1g-D first Release verdict and local closeout`](evidence/2026-08-15-perf3-13-gate-f-f1g-d-streaming-integration-first-release-verdict/).
Gate F selects
execution-local module globals, structured
task scopes, single-owner service/actor identities for durable mutable state,
bounded channels, receiver-materialized `ChannelValue`, typed terminal
completion and immutable large-binary transfer. The approved Rexx surface adds
typed task declarations and methods,
independent task calls within normal expressions, `DO PARALLEL`,
`.taskwork`/`.taskcontext`, explicit class-configured pools/scopes and the
industrial concurrent HTTP consumer required by `crexx-rag`. Public Level B
classes wrap mandatory transport-neutral RXAS channel instructions implemented
by RXVM over the Gate E executor/provider substrate. There is no RXPA task path,
hidden native-payload contract or public angle-bracket task-intrinsic family.
The five conceptual instruction roles are open, start, wait, cancel and close;
`chanopen` separates provider type from required-capability flags and versioned
configuration, with core and registered plugin-extension code ranges.
Spawn/redirect behavior is migrated to reusable byte-endpoint and
child-process providers; pre-release RXAS spellings may be retired, their slots
remain reserved and compiler exits/Level B code are updated together.
Level B-over-RXAS local/process/endpoint conformance precedes cross-host
libraries; later profiling optimizes and stabilizes the mandatory boundary
rather than deciding whether it exists. Event hubs and replicated/versioned
state remain explicit libraries rather than transparent shared objects.
Design:
[`Gate F public concurrency surface`](../concurrency/history/PERF3-13-GATE-F-DESIGN.md). User model:
[`Gate F concurrency user guide`](../concurrency/history/PERF3-13-GATE-F-USER-GUIDE.md). Execution:
[`Gate F implementation plan`](../concurrency/history/PERF3-13-GATE-F-IMPLEMENTATION-PLAN.md).

C1abc emits the exact measured C1a+C1b Richards program and C1c Towers
program. The accepted R1 paired medians are 53.55%/52.57% lower Richards
elapsed and 18.92%/18.97% lower Towers elapsed on `rxvm`/`rxbvm`, with every
pair favorable. Production closeout passes 11/11 focused Release checks,
16/16 reviewed object structural/runtime pairs and 1,972/1,972 broad Debug
tests. The old clean-host C0/C1a/C1c timings remain historical authority. C2
still lacks ownership/lifetime proof, C3 is immaterial, C4 is met by the
selected C1 rows. P1A is complete: A1 demand-driven storage attachment is
retained, A3 is a correct replayable negative, and accepted closeout passes
24/24 focused plus 1,972/1,972 broad Debug tests.
PERF3-05 is complete: Adrian accepted retaining the ordinary L0 product,
rejecting the tested LTO/PGO/no-flatten runtime forms and leaving L4 unopened.
The separate VM-library link-interface cleanup is queued as PERF3-05-B1.
Adrian selected and accepted PERF3-03 C4 v3. The narrow private locale-aware
loose-comparison prefilter passes its 6/6 minimum gate and mandatory first
ordinary Release verdict: Base64 improves by 4.86%/5.78% on `rxvm`/`rxbvm`;
RexxCPS is +2.52% on `rxvm` and a guard-clean noisy -0.61% on `rxbvm`.
Accepted Apple closeout passes 1,972/1,972 full Debug tests, 6/6 focused ASan,
complete Release build/install and installed VM smoke 2/2. LSan is unavailable
on this macOS runtime; Windows/MSVC validation is queued before publication.
Adrian authorized the combined local closeout commit on 2026-08-01; push
remains a separate user-authorized action.
Adrian subsequently accepted PERF3-10: an ordered TRACE result-event batch and
a storage-identity/component-aware RXAS proof for redundant integer-to-string
materialization. Its first ordinary Release verdict improves RexxCPS median
CPS by 10.38% on `rxvm` and 10.61% on `rxbvm`, with 21/22 and 12/12 favourable
pairs. Closeout passes 59/59 focused and 1,982/1,982 broad Debug tests.
No tactical-rule deletion or public format change was made. The complete
PERF3-02/C1abc slice is committed locally as `4a3940395`; push remains a
separate user-authorized action.

PERF3-11 Gates 1-6 are now locked.  The reusable per-epoch proof service is
the sole authority for repeated `ITOS`; its stronger write-once/component SSA
proof produces a 19-`ITOS` RexxCPS image versus the retained old solver's 21.
Adrian accepted the mandatory Release verdict: median CPS improves 7.469% on
`rxvm` and 6.866% on `rxbvm`, with 12/12 favourable pairs on each VM.  A broad
test failure exposed and drove a precise caller-owned call-window argument
model; the corrected closeout passes 1,987/1,987 Debug tests.  The remaining
legacy-proof inventory and stable replay baseline are now locked.  M01 is
complete: the old one-register `ITOF` authority is deleted and the generic
metadata-driven `XTOY` consumer covers all 20 one-register conversions.  It
recovers the old `ITOF` floor and proves 11 additional focused deletions,
including four `ITOD` flow shapes; signalling and same-component-idempotence
gaps remain closed.  `ITOD` and `BTOD` now have a coherent total
non-signalling runtime/plugin contract.  M02 is also complete: repeated scalar
constants now require write-once value equivalence and already-absent reference
and native payloads.  It recovers the old floor and proves four stronger
focused deletions through equal phis, exact float bits, linked storage and
ordered TRACE.  Adrian accepted its output-neutral Release verdict and bounded
procedure-local 30.1 MB peak RSS on 2026-08-03.  Canonical images remain
unchanged, focused replay passes 53/53 and broad Debug passes 1,991/1,991.
M03 is also complete: repeated `NULL` now requires known storage and all eight
component leaves already absent. M04 is complete: canonical conditional
same-storage metadata and the proof service now own all seven full/typed copy
families, preserving the old floor and adding linked, agreeing-phi and
TRACE-safe deletions. Adrian accepted both output-neutral verdicts; canonical
images remain byte-identical and M04 broad Debug passes 1,995/1,995. M05 is
complete: a cached sparse use/dependency index and atomic typed-copy rewrite
plan replace the old dense availability/may-reach solver. The ten-case floor is
preserved and one stronger unrelated-ENDLIFE case is proved. Canonical images
remain byte-identical, Adrian accepted the 0.16-0.17 s/102.8 MB RexxCPS
assembly boundary, and broad Debug passes 1,995/1,995. M06 is complete: its
atomic SSA/use plan recovers all eleven current producer-forwarding cases,
deletes the last dense M08 semantic liveness authority and adds the missing
hidden-cleanup proof for producer-cleared reference/native payloads. Adrian
accepted the byte-identical first Release verdict; paired RexxCPS assembly
medians remain 0.18 s and median RSS rises 1.05% to 104,103,936 bytes. Focused
Debug/Release pass 8/8 and broad Debug passes 1,995/1,995. K04a completes
scalable atomic deletion of exact optimized-away Boolean TRACE events. K04b
replaces the procedure-global call-window veto with reusable exact/dependent-
`ValueId` visibility and is accepted as a neutral consolidation: focused
Debug/Release pass 5/5 and canonical output remains byte-identical. K04c proves
all five residual RexxCPS rejections are false positives from unknown CALL
retry metadata: each real argument window is only `r1`, while retry count phis
widen it to `r1..r69`. Broad Debug passes 1,995/1,995. K04d0 then found no
production retry caller and an existing fused-call retry mapping defect.
Adrian approved K04d1 retirement on 2026-08-03; propagated-call partial-state
metadata remains for skip/handler/unwind analysis. K04d1 is now implemented and
documented. The same 14 focused language, VM, metadata, flow, optimizer and
native-unwind checks pass in Debug and ordinary profiling-off Release. The
canonical candidate is 1,222 VM instructions and 1,249 TRACE events versus
frozen M06 at 1,241/1,252. The clean-host K04d3 verdict is runtime-neutral:
median RexxCPS changes by +0.021% on both VMs, with mixed 6/12 `rxvm` and 5/12
`rxbvm` pair directions. One retained `rxvm` candidate sample is 13% below its
pair and triggers the formal rerun recommendation. No sample was removed.
Adrian accepted K04d1 as a neutral semantic/infrastructure improvement without
a noisy-cell append on 2026-08-03. K04d4 then passed the complete Debug build
and 1,998/1,998 broad Debug tests in 297.92 seconds. The retirement audit finds
no obsolete production retry machinery. K04 is closed. K02/K03 are also
complete: one storage/component/path proof replaces all twelve duplicate
linked-read rules, focused K02/K03 passes 21/21 in Debug and Release, and
canonical Richards, Towers and RexxCPS remain byte-identical to frozen K04.
The final K02/K03 shared proof panel passes 28/28 in both builds and broad
Debug passes 2,010/2,010 in 678.89 seconds. K01 is also complete from committed
baseline `45e027685`: sparse storage-permutation and observation equivalence is
the sole authority for exact cancelling `SWAP` pairs and exact self-cancelling
ordered `SWAPN`. The inherited floor is byte-identical, the expanded focused
panel passes 9/9 and the shared proof panel passes 37/37 in Debug and Release,
and broad Debug passes 2,012/2,012 in 368.43 seconds. Canonical Richards,
Towers and RexxCPS have zero K01 accepts and byte-identical images, so runtime
timing was not warranted. K06 is also complete from committed K01 closeout
`78bd7f6f5`: metadata and VM handlers prove adjacent full `COPY` subsumes the
same-pair `ACOPY`, so the unchanged declarative rule is retained as mechanical
rather than moved into CFG/SSA. The metadata/optimizer panel passes 3/3 in
Debug and Release, the original focused outputs are byte-identical, and all
three canonical images remain exact with zero K06 accepts. No production code
or output changed, so timing and broad CTest were not warranted. K05 immutable
CFG branch threading exposed an unsupported assembler scale boundary rather
than a correctness failure. Committed K06 already requires approximately
1.87 GB peak RSS for generated `Parse.rxas` while `-n` requires approximately
26 MB; provisional batched K05 reaches approximately 2.47 GB. Adrian approved
D0.1-D0.5 as separately validated/committed infrastructure and consumer-
migration stages before K05 closeout. D0.1 is complete: every current local,
CFG, SSA/use and diagnostic optimisation has a stable executable route and
conservative pre-analysis candidate census. D0.2 is complete and gives one
immutable graph epoch to M00, every semantic proof and the batched K05 plan;
the seven queue-local K05 rules and the legacy graph/dense M07 matrices are
deleted. Focused Debug/Release proof checks, dual-VM flow fixtures and exact
accepted K05 canonical images pass. D0.3 is also complete: M01-M06 and K01-K04
request exact monotonic capabilities, undeclared use/loop queries fail closed,
loop analysis remains dormant, and explicit diagnostics retain their own
route. Its focused Debug/Release and dual-VM gates pass with exact K05 images.
The D0.3 diagnostic Parse probe remained approximately 2.46 GB because real
use-index candidates still trigger semantic analysis. D0.4 is now complete: a
sparse transactional manager batches compatible semantic plans, the final
Debug/Release optimizer and migrated-runtime panel passes 107/107, and
canonical images remain exact. Parse applies 63 semantic plans in 12 batches
but peaks at approximately 2.56 GB because longer epochs co-retain proof
capabilities. D0.5 is now complete: redundant storage phis collapse lazily,
an exact linear write-once/single-use typed-copy route handles mechanical
temporaries, and whole-procedure semantic analysis is bounded while local and
CFG consumers continue. Ordinary Release `Parse.rxas` now peaks at
142.7-142.9 MB versus 2.56 GB at D0.4, with all 261 K05 rewrites retained.
Six advanced M05 copies are conservatively retained and recorded for a future
candidate-sliced/region proof. Debug/Release optimizer/runtime checks pass
108/108, broad Debug passes 2,021/2,021, and canonical images remain exact.
D0.6 is complete: Adrian selected a permanent 100-record bounded
peephole-first stage and a standing rule that exact metadata-proved local
normalisations remain there rather than moving into CFG/SSA for consolidation.
The 20-record product is the control; procedure-length windows remain a
retained negative unless pre-SSA sparsity repays their measured scan cost.
The exact RexxCPS census/CFG/SSA/use facts and emitted images are unchanged at
100, while paired ordinary assembly moves from a 0.27 s to 0.28 s median with
0/12 favourable pairs and neutral RSS.  Adrian accepted the approximately
10 ms absolute cost on 2026-08-04; broad Debug passes 2,021/2,021.
K04e is complete and accepted.  It restores the one hot in-place
`ILT`/`BRF` comparison through the source's pre-write `ValueId` plus the
existing alias, cleanup, use and TRACE guards; the old raw-register rule does
not return.  Runtime-source-derived `STRLEN`, integer-subtract and loose-string
compare metadata plus exact fixed/range call ownership describe the existing
handlers without a VM/RXBIN semantic change.  RexxCPS moves from 1,222 to 1,221
static instructions and removes exactly 560,000 equal-work dispatches.
Thirty-six paired ordinary Release rounds remain noisy/inconclusive but have
positive point estimates and no 3% guard hit; Adrian accepted that verdict.
Broad Debug passes 2,021/2,021.

The authorized clean-product Mac refresh at `c44706350` passes `348/348`
initial plus `30/30` governed-append executions. Current common-five means are
`2.465740x/2.316900x` versus ooRexx and `0.894608x/0.840606x` versus decimal
NetRexx for `rxvm`/`rxbvm`; the high-level ranking is unchanged. Both VMs clear
the 2.00x ooRexx aggregate target, Richards and noisy Base64 remain common
deficits, Towers remains separate, and RexxCPS is above ooRexx parity but below
1.50x. No sample was removed; noisy `permute-netrexx` and both cREXX Base64
cells received the sole permitted append and remain labelled. Evidence:
[`2026-08-04-perf3-11-k04e-mac-scorecard`](evidence/2026-08-04-perf3-11-k04e-mac-scorecard/).

The accepted PERF3-12B merged-product refresh at `44d8b6a7e` is now the
current formal Apple scorecard. It passes 348/348 executions with no noise
append: common-five means are `2.375939x/2.376230x` versus ooRexx and
`0.852882x/0.852987x` versus decimal NetRexx for `rxvm`/`rxbvm`. RexxCPS is
47.203/47.093 MCPS, or `1.172472x/1.165701x` ooRexx. The high-level ranking is
unchanged and independent-session movement from K04e is descriptive only.
Evidence:
[`2026-08-05-perf3-12b-mac-scorecard`](evidence/2026-08-05-perf3-12b-mac-scorecard/).

The subsequent fixed-work PERF3-12 reassessment recorded optimized
RexxCPS totals of 53,660,581/53,660,552. K04e accounts for the exact 560,000
hot-site reduction, while every then-ranked clause opportunity retained its
original count. That dated queue is now historical: PERF3-12A completed
copied-XTOY placement, PERF3-12B completed compound-tail reuse, and PERF3-12D
completed exit-owned PARSE lowering. Late inlining/register finalization is a
post-PERF3 successor, not an unfinished PARSE transaction. Evidence:
[`2026-08-04-perf3-12-k04e-clause-reassessment`](evidence/2026-08-04-perf3-12-k04e-clause-reassessment/).

PERF3-06 is complete from accepted product `5fbe36049`. Its formal Apple M5
scorecard passes 348/348 initial plus 30/30 governed-append executions with no
sample removed. The common-five geometric means are 2.453066x/2.285744x
versus ooRexx and 0.912280x/0.850054x versus decimal NetRexx for
`rxvm`/`rxbvm`. Both VMs now clear the 2.00x ooRexx aggregate band, while
Richards and noisy Base64 remain below parity and the 1.50x per-cell band.
RexxCPS reaches ooRexx parity at 1.151301x/1.133307x but remains below its
separate 1.50x band; Towers remains a qualified separate deficit at
0.390842x/0.389933x. OoRexx Bounce and both cREXX Base64 cells remain
noise-labelled after the single permitted append. PERF3-12 is complete as an
analysis-only current RexxCPS clause-lowering evidence gate. It recommends
copied-XTOY component placement as the first separately approved implementation
slice; no product code changed in the gate.

PERF2 is closed and preserved in
[`ROADMAP-PERF2-2026-07-31.md`](ROADMAP-PERF2-2026-07-31.md). The initial
`NR-*` sweep remains closed in
[`ROADMAP-INITIAL-SWEEP-2026-07-23.md`](ROADMAP-INITIAL-SWEEP-2026-07-23.md).
The original dated charter remains a historical snapshot in
[`performance-programme-report-2026-07-15.md`](../docs/planning/release-1/performance-programme-report-2026-07-15.md).
Standing measurement, regression and publication rules remain normative in
[`PERFORMANCE-GOVERNANCE.md`](PERFORMANCE-GOVERNANCE.md) and
[`AGENTS.md`](AGENTS.md).

The accepted product baseline for the PERF3-06 scorecard is
`5fbe36049e26ee73ea0cf1720a7fc416f33d0fe2`; the earlier closeout tranche is
`4a3940395980dc40ea45917d71d99caa080e89bb`. The PERF3 planning baseline
remains `e38e514bf611ae3873513368c44742e2ae7332d1`, whose product-code parent is
`3f43a0014be10c930a12b8a636297b60f294c0a6`. The exact new local tip is kept in
repository history rather than self-referenced by the commit that contains
this roadmap. Push remains unauthorized.
The pre-existing worktree also contains five untracked
generated `lifecycle_probe.rxbin` files under retained evidence directories;
they are outside this documentation transition and must not be overwritten or
deleted casually. This is a planning baseline, not a benchmark baseline.

Status values are `queued`, `in progress`, `decision required`, `blocked`,
`deferred`, `complete`, `rejected` and `superseded`. `Complete` means the
activity's stated exit criterion and retained evidence both exist. A useful
prototype or a closed predecessor programme does not make an unresolved
mechanism complete.

## PERF3 mission

PERF3 turns the accepted PERF2 Mac, Linux x86-64 and Windows evidence into a
small number of evidence-selected, independently revertible product decisions
on the faster Apple ARM64 development host. It does not repeat PERF2 wholesale,
resume every historical idea or treat platform-dependent compiler movement as
a production mechanism.

The approved operating rule is:

> Re-establish exact current-product truth, remove the largest proved semantic
> or ownership cost at its earliest safe owner, and escalate to VM layout or a
> new execution architecture only when the remaining evidence requires it.

The PERF2 north star is retained as a programme target, not as permission to
change semantics or benchmark work:

1. the selected default VM should reach at least 1.50x ooRexx median throughput
   on every qualified equal-work common cell;
2. the common-five geometric mean should reach at least 2.00x ooRexx;
3. the separately governed cREXX RexxCPS 2.2d diagnostic should reach at least
   1.50x same-session canonical Classic ooRexx RexxCPS; and
4. the alternate VM should itself be clearly faster than ooRexx on every
   qualified cell.

Only semantically qualified, correctness-passing, same-session results count.
`rxvm` and `rxbvm`, throughput, lifecycle, RSS and artifacts remain separate.

## What PERF2 established

### Accepted product and evidence foundation

- The initial sweep, current-product profiling schema, governance, repeatable
  evidence tools and common-five comparison contract are complete.
- PERF2-01 through PERF2-05 are closed with accepted compiler, inlining, BIF,
  private execution-image and reference-path work.
- The Apple PERF2-06/07 slice is closed: V3-R01 corrected stale string-length
  state and V1R01-R1 removed the proved receiver-copy explosion.
- PERF2-08/09 closed the Mac qualification and same-session scorecard. The
  common aggregate is exactly Sieve, Permute, Bounce, Richards and Base64;
  RexxCPS remains a separately governed diagnostic.
- The initial Linux x86-64 GCC/Clang correctness, sanitizer, formal baseline,
  schema-5 and native-PMU campaign is checksum-closed and sufficient for
  mechanism selection.
- The supported Windows x86-64 baseline and bounded GCC, Clang and MSVC
  controls are retained. They selected no compiler, CRT, VM or production
  optimization.
- CAP-01's indexed `rxjson` document and accepted numeric projection work are
  closed. Their general residual mechanisms were extracted below rather than
  left hidden inside the capability activity.

### Current accepted Mac outcome snapshot

The 2026-07-27 PERF2-09 Mac scorecard at `d5f0827ca` is the last formal
same-session Apple comparison, not an automatic current-HEAD baseline.
Product-affecting compiler, VM and library work has landed since its source
snapshot.

| Workload | Qualification | `rxvm / ooRexx` | `rxbvm / ooRexx` | PERF3 meaning |
| --- | --- | ---: | ---: | --- |
| Sieve | common | 7.214291x | 5.338790x | Established win and zero-work/code-layout guard. |
| Permute | common | 8.005043x | 7.015322x | Established win; guard accepted call/value placement. |
| Bounce | common | 3.902513x | 2.963270x | Established win; do not reopen reference work without a new exact reduction. |
| Richards | common | 0.267262x | 0.264171x | Largest qualified common deficit and strongest current copy/value lead. |
| Base64 | common | 0.719817x | 0.724922x | Deficit remains noisy; require exact work reduction and stable same-session evidence. |
| RexxCPS | separate diagnostic | 0.995754x | 0.933193x | Near Mac parity but below the separate 1.50x band. |
| Towers | qualified separate lane | 0.328060x | 0.321343x | Large object/allocation deficit; not part of the common aggregate. |

The exact PERF2 Mac common-five geometric means were 2.125260x/1.842840x
versus ooRexx and 0.742985x/0.644251x versus decimal NetRexx for
`rxvm`/`rxbvm`. These remain historical observations until PERF3-01 decides
what current-HEAD refresh is required.

### Mechanisms selected by retained cross-platform evidence

| Evidence | Observation | Planning consequence |
| --- | --- | --- |
| Linux Richards | 56.9 million copy operations and 451.7 MB copied in the bounded profile; `copy_value` accounts for roughly 55-57% of GCC and 77% of Clang sampled cycles, with Clang also exposing attribute-storage trimming. | Make full-copy elimination/ownership the first planned Mac design panel. Do not assume the VM handler is the right owner. |
| Linux Towers | 26.8 million copies, 31.3 million clear/reset/destroy operations and 5.86 GB of allocation requests; front-end and indirect-branch pressure are also visible. | Separate exact copy/clear/attribute shapes from allocator or global-value-layout hypotheses. |
| Linux Base64 | 46.7 million VM instructions; `SCOPY_REG_REG` is the third-ranked opcode and 92-96% of sampled cycles remain in `run`. | Compare a semantic string/copy ceiling with code-layout alternatives before selecting either. |
| Linux RexxCPS | Decimal conversion/formatting and string movement remain visible; the GCC `rxvm` native cell is 42% front-end bound. | Keep conversion and layout as separate hypotheses; do not infer a single cause. |
| CRI-13 residual | The retained projection executes 6,144 full-source copies totalling 359,294,976 logical bytes; RXAS currently reports `full-value-ownership-unproved`. | Carry the byte-weighted proof question into PERF3-02. |
| Concrete scalar access | Current final/concrete wrapper reads are 4.56x-5.10x raw access and writes are 2.41x-3.67x across both VMs. | Preserve a generic accessor-proof lead, not JSON/vector-specific opcodes. |
| Apple/Linux/Windows compiler controls | Compiler direction reverses by workload; zero-work Apple controls moved with code layout, Clang helps some Linux/Windows cells and hurts others, and MSVC `/MT` improves one Windows control without closing the gap. | Treat compiler, CRT and layout results as qualified leads only. Require paired candidate evidence and semantic zero-work guards. |

## PERF2 to PERF3 transfer register

The following items are deliberately transferred. The old stable ID remains in
the source column so no unfinished item disappears during renumbering.

| PERF2 source | PERF3 owner | Closing disposition carried forward | PERF3 entry condition |
| --- | --- | --- | --- |
| `PERF2-07-B02` | PERF3-02 | C1abc selected and Apple closeout complete | Closed by the selected C1a+C1b Richards and C1c Towers production ladder; retain timing and replay evidence. |
| Linux Richards/Towers copy and attribute-trim findings | PERF3-02 | exact C0 attribution and selected C1abc proof complete | Closed for Apple; reuse retained evidence in the later platform-validation gate. |
| `PERF2-05-F01` | PERF3-02 | evidence-gated | A fresh profile attributes material residual reference-descriptor payload cost after accepted R2a. |
| `PERF2-03-F01` and `PERF2-03-F02` | PERF3-02 | evidence-gated | Current hot sites prove residual accessor/ownership/escape cost and exact alias/lifetime obligations. |
| `PERF2-07-C01` | PERF3-03 | Apple closeout complete | The material owner is implicit loose-comparison parsing. Adrian selected and accepted private C4 v3; first verdict and proportional Apple closeout pass. C1/C2/C3/cache/public-span alternatives remain rejected or deferred. Windows/MSVC validation is retained as PERF3-03-W1. |
| `PERF2-03-F06` | PERF3-04 | queued evidence only | Current profile plus hand-equivalent ceiling selects generic final/concrete scalar access. |
| `PERF2-06-D01` | PERF3-05 | open accepted debt | Paired Mac zero-work and target controls distinguish native code layout from semantic work. |
| PERF2-06 compact/hot-cold private stream and PERF2-10 LTO/PGO/layout | PERF3-05 | unstarted, no option selected | A bounded Mac panel identifies a repeatable supported mechanism before production selection. |
| PERF2-09 qualified gaps | PERF3-06 | outcome lane | An accepted product slice exists or PERF3-01 changes the ranking. |
| `CAP-02`, `CAP-03` and `CAP-04` | PERF3-07 | deferred or independent product/evidence tracks | Separate capability/API/use-case approval; they do not block qualified common cells. |
| PERF2-11 Gate E and final VM recommendation | PERF3-08 | incomplete late gate | A Mac-selected candidate is accepted and ready for batched platform validation. |
| PERF2-12 JIT/AOT/native backend | PERF3-09 | deferred | The accepted non-JIT programme cannot meet the target economically and Adrian approves a separate architecture decision. |

### Preserved conditional triggers, not queued work

These points remain discoverable but do not consume PERF3 capacity unless
their recorded trigger fires:

| Source | Preserved disposition |
| --- | --- |
| `PERF2-03-F03` | Admit remaining inline-exit/result/temporary cleanup only as a bounded companion to a currently selected hot site. |
| `PERF2-03-F04` | Reopen dynamic vararg/association/effect reconstruction only with a measured multi-site deficit. |
| `PERF2-03-F05` | Standing producer/consumer consistency requirement owned by any change that consumes new summary facts. |
| `PERF2-05-F02` | Reopen result forwarding only with mathematical equivalence and stable multi-workload dual-VM benefit. |
| `PERF2-06-C2R02` | Deferred; rejected reset evidence gives no reason to advance quickened clearing. |
| `PERF2-06-C2R03` | Analysis-only architecture candidate; it must first pass a current payload-capacity/high-water entrance gate. |
| Higher-arity call/frame forms | No `CALL5+` or embedded-argument work without a refreshed dynamic residual census after accepted inlining. |
| Legacy `FDIVSUB`, `ILOADSETUNLINKN` and frequency-only fusion ideas | Archive-only unless a current exact profile selects the mechanism. |
| Windows MSVC `/MT` | Experimental validation lead only; plugin/API allocator ownership must be proved before any product selection. |
| RexxCPS timer cross-check | Cheap cross-OS validation lead for a later platform campaign, not a Mac optimization. |

### Closed or rejected work that PERF3 must not silently repeat

- Do not retry C2-A/B, fixed-core reset R1/R2, exact reset lists, quickened
  clearing, C3R01 numeric synchronization or cleanup-only flattened-interpreter
  reshaping without materially different ownership evidence and zero-work
  controls.
- Do not reopen selector caches: accepted profiles observed zero attempts.
- Do not add a public RXAS/RXBIN form merely because a private or compiler
  form has a useful ceiling.
- Do not rerun Linux x86-64 or Windows baselines for questions answerable from
  the retained immutable products, profiles, samples or small external
  harnesses.
- Do not reopen CAP-01's accepted API or benchmark-local class probe while
  investigating the extracted generic copy, conversion or accessor questions.
- Do not edit the dated charter or either closed roadmap to reflect PERF3.

## Activity register

The PERF3-13 rows in this register are retained performance provenance. Their
internal stage labels and evidence paths are historical identifiers; current
concurrency implementation and publication status lives only in
[`concurrency/WORKLIST.md`](../concurrency/WORKLIST.md).

| ID | Priority | Activity | Status | Exit / next gate |
| --- | --- | --- | --- | --- |
| PERF3-00 | P0 | Archive PERF2 and approve the transfer boundary | complete | Adrian approved the roadmap and transfer boundary on 2026-07-31; no production work was bundled with approval. |
| PERF3-01 | P0 | Current-HEAD Mac evidence and baseline-validity gate | complete | Adrian accepted the current-product evidence boundary and ranked panel on 2026-07-31. No production edit was made. Evidence: [`2026-07-31-perf3-01-current-mac`](evidence/2026-07-31-perf3-01-current-mac/); control: [`PERF3-01-WORKLIST.md`](PERF3-01-WORKLIST.md). |
| PERF3-02 | P0 | Full-copy, ownership and attribute-storage panel | complete | Adrian selected C1abc. The ordinary compiler emits the exact measured C1ab Richards and C1c Towers programs; 11/11 focused Release and 1,972/1,972 broad Debug tests pass. Broad C1a-R1 remains rejected and replayable; C2 is ownership-deferred and C3 immaterial. Control: [`PERF3-02-R1-WORKLIST.md`](PERF3-02-R1-WORKLIST.md); timing: [`2026-08-01-perf3-02-r1-repanel`](evidence/2026-08-01-perf3-02-r1-repanel/); closeout: [`2026-08-01-perf3-02-c1abc-closeout`](evidence/2026-08-01-perf3-02-c1abc-closeout/). |
| PERF3-02-C1B | P0 | Multi-return receiver-link ownership feasibility | complete — correct and decisively faster | The exact detached scalar receiver-guard rule preserves canonical `23246/9297`, all fail-closed boundaries and link/unlink state. The R1 panel measures 44.28%/44.01% lower Richards elapsed alone and proves clean composition with C1a. The broad no-write route remains rejected. Control: [`PERF3-02-C1B-WORKLIST.md`](PERF3-02-C1B-WORKLIST.md); correctness: [`2026-08-01-perf3-02-c1b-correctness`](evidence/2026-08-01-perf3-02-c1b-correctness/); timing: [`2026-08-01-perf3-02-r1-repanel`](evidence/2026-08-01-perf3-02-r1-repanel/). |
| PERF3-02-C2E2 | P0 | RXAS symbolic register-storage identity | complete — core-infrastructure candidate | The diagnostic PoC safely follows direct link/swap/unlink identity, recovers exact point state at all 55 globally tainted Richards full-copy sites and 13/56 Towers sites, and proves a consolidated swap-round-trip route. P1 supplied split normal/signal failure edges; its original retry edge was later retired by approved K04d1. No rewrite or tactical-rule deletion is selected. Control: [`PERF3-02-C2E2-WORKLIST.md`](PERF3-02-C2E2-WORKLIST.md); evidence: [`2026-07-31-perf3-02-c2e2-storage-identity`](evidence/2026-07-31-perf3-02-c2e2-storage-identity/). |
| PERF3-02-C2E2-P1 | P0 | Core storage identity and signal continuations | locked — infrastructure retained, retry superseded | Typed normal/signal-skip edges and the graph-owned bounded storage service remain the R1 foundation. The original signal-retry edge is superseded by Adrian's approved K04d1 retirement; retained P1 evidence remains historical provenance. Control: [`PERF3-02-C2E2-P1-WORKLIST.md`](PERF3-02-C2E2-P1-WORKLIST.md). |
| PERF3-02-C2E2-P1A | P1 | Recover storage-analysis assembler cost | complete — A1 retained, A3 rejected | Adrian accepted the bounded disposition on 2026-08-01. A1 demand-driven storage attachment is retained; A3 remains a correct, replayable negative and is removed from production. Exact A1 restoration passes 24/24 focused and 1,972/1,972 broad Debug tests. Control: [`PERF3-02-C2E2-P1A-WORKLIST.md`](PERF3-02-C2E2-P1A-WORKLIST.md); A1 verdict: [`2026-08-01 A1 evidence`](evidence/2026-08-01-perf3-02-c2e2-p1a-first-release-verdict/); A3 verdict: [`2026-08-01 A3 evidence`](evidence/2026-08-01-perf3-02-c2e2-p1a-a3-first-release-verdict/); closeout: [`2026-08-01 closeout`](evidence/2026-08-01-perf3-02-c2e2-p1a-closeout/). |
| PERF3-02-R1 | P0 | Infrastructure-enabled copy/ownership option re-investigation | complete — C1abc selected | All positive, combined and rejected masks remain replayable in retained evidence; correctness and 156/156 formal timing executions pass for eligible rows. Production contains only C1a+C1b+C1c and passes proportional closeout. Control: [`PERF3-02-R1-WORKLIST.md`](PERF3-02-R1-WORKLIST.md); evidence: [`2026-08-01-perf3-02-r1-repanel`](evidence/2026-08-01-perf3-02-r1-repanel/); closeout: [`2026-08-01-perf3-02-c1abc-closeout`](evidence/2026-08-01-perf3-02-c1abc-closeout/). |
| PERF3-03 | P1 | Bounded string-to-number conversion review | complete on Apple — C4 v3 retained | Adrian selected and accepted the private locale-aware C4 v3 prefilter. Minimum validation, 212/212 first-verdict executions, 1,972/1,972 full Debug tests, 6/6 focused ASan, complete Release build/install and installed VM smoke 2/2 pass. Base64 improves 4.86%/5.78% and RexxCPS is +2.52%/-0.61% on `rxvm`/`rxbvm`; no cell hits the 3% guard. LSan is unsupported locally and Windows is separately queued. Control: [`PERF3-03-WORKLIST.md`](PERF3-03-WORKLIST.md); first verdict: [`2026-08-01-perf3-03-c4-first-release-verdict`](evidence/2026-08-01-perf3-03-c4-first-release-verdict/); closeout: [`2026-08-01-perf3-03-c4-closeout`](evidence/2026-08-01-perf3-03-c4-closeout/). |
| PERF3-03-W1 | P1 | C4 v3 Windows/MSVC validation | queued pre-publication gate | No Windows cross-toolchain is installed on the accepted Mac host. Before publication, build both VM variants under MSVC, run focused logic/conversion correctness and confirm material Base64/RexxCPS behavior. Do not reopen C4 design without a correctness or guard failure. |
| PERF3-04 | P1 | Generic final/concrete scalar accessor proof | complete — G1 and four-family guard proof accepted | The exact type-generic scalar-access lane covers `.boolean`, `.int` and `.float` while packed wrappers remain controls. The integrated 266-process verdict passes with no accessor guard hit and 32.75-35.41% lower float-write elapsed; the selected RXBIN is +2.07% versus G0. RXAS removes repeated proved-success initialization/type/range guards across the four families while preserving signal re-entry and mutation boundaries. Broad QA fixed exponential signal-policy resolution with a bounded worklist and passes 2,249/2,249. Adrian accepted the remaining `httpcodec` assembler lifecycle cost of +8.104172% (about 17.5 ms) to close the stage. Evidence: [`POSTPERF-04 verdict and closeout`](evidence/2026-08-18-postperf-04-generic-scalar-access-first-release-verdict/README.md). POSTPERF-05 subsequently completed the approved post-PERF3 sequence. |
| POSTPERF-05 | P1 | Bounded hoisting, register finalisation and late inlining | complete — H1-T20 accepted | RXC now applies a final Pareto profitability gate to supported callable bodies above 20 structural nodes while preserving exact scalar accessors and the ordinary small-call path. Against exact H0, paired medians improve DeltaBlue by 82.31%/82.42%, CD by 47.30%/47.15%, Richards by 83.93%/84.60% and List by 2.50%/5.07% on `rxtvm`/`rxbvm`; Sieve and RexxCPS remain accepted noisy/inconclusive controls at the 36-pair ceiling with no adverse median guard hit. The library shrinks 907,207 -> 719,809 bytes. Closeout corrected retained-call binary/decimal `ret` lowering and an NR-26 skipped-copy fixed-point reassignment defect, with focused structural/runtime regressions. Full Debug passes 2,251/2,251, focused Release 56/56, and all 17 rebuilt accepted RXAS/RXBIN/library artefacts are byte-identical to the frozen H1 set. No further post-PERF3 production stage is authorized. Evidence: [`POSTPERF-05 verdict and closeout`](evidence/2026-08-18-postperf-05-bounded-late-profitability-first-release-verdict/README.md). |
| PERFORMANCE-CLOSEOUT | P0 release evidence | Current-product performance closeout | Apple Stage 5 scorecard complete; formal Linux QA-C pending; Stage 6 started | Portfolio v3 is source-reviewed, capability-classified, calibrated and green at committed candidate `81f159186`. The fresh Apple scorecard qualifies all 89 cells: cREXX/ooRexx common-five is 5.467915x/5.679203x on `rxtvm`/`rxbvm`, while the separately named genuine-NetRexx common-four is 1.318977x/1.358087x and retains the Permute deficit. RexxCPS is 44.495/44.156 MCPS, above ooRexx and Regina but below genuine NetRexx. CD, DeltaBlue, Towers and Havlak receive bounded release reviews; a production change is made only if current evidence supports a safe, general and material improvement. Storage/List ownership, NBody, Permute and any unselected release-review item carry into the next release. DECIMAL retains `mc_decimal`; the Mac concurrency replay remains waived. Exact-commit formal Linux QA-C is the sole Stage 5 exit item. Current report: [`RESULTS.md`](RESULTS.md); control: [`closeout plan`](PERFORMANCE-CLOSEOUT-PLAN.md); v3 contract: [`manifest-v3`](portfolio/manifest-v3.md); Stage 4 evidence: [`2026-08-18 closeout Stage 4`](evidence/2026-08-18-performance-closeout-stage4/); Stage 5 scorecard: [`2026-08-18 Apple scorecard`](evidence/2026-08-18-performance-closeout-stage5/). |
| PERFORMANCE-CLOSEOUT-CD | P1 pre-release review | CD residual indexed-map/application deficit | release activity — current attribution required | Stage 5 cREXX is 4.797/4.774 s versus CPython 0.424 s and Java 0.068 s. The fair port retains all 200 frames through indexed red/black state and native `rxmath`; POSTPERF-05 already removed an unprofitable expansion and improved the old candidate about 47%. Before any production proposal, capture a current exact-work profile separating red/black/value copies, calls, allocation, attribute access and native-math time. Preserve algorithm, work and result; do not substitute no-opt or a benchmark shortcut. If no bounded material change is justified, carry the finding into the next release. |
| PERFORMANCE-CLOSEOUT-DELTABLUE | P1 pre-release review | DeltaBlue stable-indexed graph residual | release activity — current attribution required | Stage 5 cREXX is 0.448/0.457 s versus CPython 0.031 s and Java 0.053 s. Level B object assignment copies values, so the qualified port uses stable handles and planner-owned typed arrays. POSTPERF-05 recovered about 82% by rejecting unprofitable expansion; profile the remaining handle lookup, tagged constraint dispatch, value/copy and call/frame costs before selecting a compiler, VM, container or source-level mechanism. If no bounded material change is justified, carry the finding into the next release. |
| PERFORMANCE-CLOSEOUT-TOWERS | P1 pre-release review | Towers object/value/allocation deficit | release activity — refresh retained attribution | Stage 5 is about 45% slower than the qualified ooRexx object port and about 14x slower than CPython. Retained Linux evidence found 26.8M copies, 31.3M clear/reset/destroy operations and 5.86 GB of allocation requests. Reproduce the current counts first, keeping copy/clear/attribute traffic separate from allocation and dispatch; no pooling, layout or ownership change is inferred from the old profile alone. If no bounded material change is justified, carry the finding into the next release. |
| PERFORMANCE-CLOSEOUT-HAVLAK | P1 pre-release review | Havlak graph/container cost and optimized/no-opt inversion | release activity — paired attribution required | Stage 5 cREXX is 3.606/3.604 s versus CPython 1.474 s and Java 0.133 s. The source already passed the fair algorithm/capability review. Run a current same-work optimized/no-opt paired profile covering stable handles, VM-value `.int[]`, vector/set operations, calls/copies and generated expansion. Preserve insertion/traversal order and all 50 recognitions; do not tune the benchmark around the compiler. If no bounded material change is justified, carry the finding into the next release. |
| PERFORMANCE-CLOSEOUT-OWNERSHIP | P2 capability follow-up | Storage and List ownership/container adaptations | next release — post-Release capability owner | Storage adds one owner object around every logical child array because nested reference containers are unavailable; List adds a typed owning arena because Level B references are weak. These are confirmed extra work, not equal-work runtime deficits. Keep their timings visible but outside aggregates; reopen under the post-Release Level G ownership/container decision with lifetime and typed-array fast-path gates. |
| PERFORMANCE-CLOSEOUT-NBODY | P2 attribution | NBody float/object/native-math control | next release — profile before mechanism | Stage 5 cREXX is 1.728/1.658 s versus CPython 0.631 s, while cREXX remains about 16x faster than the ooRexx decimal/native-math adaptation. Both cREXX and CPython use a native square-root boundary. Attribute float/value operations, object attributes, loop dispatch and native calls before inferring a math-library or VM mechanism; keep NBody outside Rexx aggregates. |
| PERFORMANCE-CLOSEOUT-PERMUTE | P2 attribution | Permute genuine-NetRexx-specific deficit | next release — matched recursion/value review | cREXX is faster than ooRexx and CPython but only 0.540488x/0.549880x of the genuine-NetRexx port. The equal-work kernel stresses recursive calls, returns and six-slot mutation. Prior work removed a proved receiver-copy explosion; use current cREXX profiles plus generated-NetRexx inspection to identify any residual call/frame/value difference rather than treating this as a general cREXX failure or inferring a mechanism from HotSpot alone. |
| PERF3-05 | P1 | Compiler, native layout and private-stream panel | complete — retain L0 | Adrian accepted the 2026-08-01 panel. Exact C1abc+A1 baseline/drift products match; effective ThinLTO, merged/per-VM PGO and no-flatten layout fail representative or zero-work guards; L4 remains unopened. No production VM change was made. Control: [`PERF3-05-WORKLIST.md`](PERF3-05-WORKLIST.md); evidence: [`2026-08-01-perf3-05-compiler-layout-panel`](evidence/2026-08-01-perf3-05-compiler-layout-panel/). |
| PERF3-05-B1 | P2 | VM library link interface and static API granularity | queued build/API hygiene | Current Mac links complete in 61-71 ms, so the reported large delay is not reproduced. Export leakage is real but not causal in the isolated relink. Rework should make `crexxsaa` implementation archives/includes private, publish only the supported header/export surface, split the static phase API if narrow clients are supported, and remeasure on the reporting host. Evidence: [`link diagnostic`](evidence/2026-08-01-perf3-05-compiler-layout-panel/link-diagnostic/). |
| PERF3-05-R1 | P1 | RXVM `run()` hot/cold refactor and layout robustness | superseded — investigation completed by R2-R5 | R2-R5 built the per-handler framework, diagnosed compiler-specific owner lowering and selected profile-20. No further PERF3 VM refactor is active; regenerate low-level handler shape only at release-candidate freeze with current platform evidence. |
| PERF3-05-R2 | P1 | Profile-selected RXAS/RXBIN instruction-handler inline/call framework | complete at approved Apple 30% checkpoint — framework retained, no production policy selected | All 649 opcode/sentinel handlers plus two private handlers now have one grouped macro definition that emits direct owner code or a force-noinline call under an explicit per-instruction policy; the RXAS/RXBIN and plugin ABIs are unchanged. All-inline, all-outline and the common 176/588 (29.93%) profile panel pass 2,002/2,002 Release tests; all-outline also passes 2,002/2,002 fresh Debug tests. The framework moves `run()` from roughly 532 KiB all-inline to 32 KiB all-outline or 200 KiB profile-30. A balanced 588-execution Release matrix passes every oracle, but profile-30 loses 9.35%/12.08% geometric-mean throughput on `rxtvm`/`rxbvm`; all-outline loses 40.02%/34.24%. The R2 census reported zero outlined calls in six workloads and eight in RexxCPS, but R3 later proves that attribution omitted process-private fused handlers and retracts that completeness claim. Frequency identifies dynamic heat (29 public handlers cover 75.07%; 176 cover 99.9999969%) but does not alone choose a fast owner shape. Stop before production selection, VM-specific panels, layout optimization or cross-platform closure. Control: [`PERF3-05-R2-WORKLIST.md`](PERF3-05-R2-WORKLIST.md); evidence: [`2026-08-09-perf3-05-r2-handler-panel`](evidence/2026-08-09-perf3-05-r2-handler-panel/). |
| PERF3-05-R3 | P1 | Handler code-generation, interrupt-tail and cross-compiler defect analysis | locally complete — internal repair retained, no default selected | Normalized expansion rejects opcode/mapping and helper-sub-inlining defects; governed runs take no interrupt, but the poll still shapes compiler output. Clang's pointer facade is the primary avoidable loss: one reachable, never-executed outlined site changes hot owner allocation. A value snapshot plus one shared cold entry removes that effect, while real GCC performs better with the R2 per-identity pointer-facade lowering, so the experimental panel now lowers by compiler. Native sampling also proves hot `PRIVATE_R1_RELINK` execution hidden by R2 public-opcode attribution; both private fusions are inline. The final 178/590 (30.17%) panel passes 2,002/2,002 Release tests under each compiler and all 1,176 formal executions. Without noisy Base64, profile-30 versus rebuilt all-inline is -0.341%/+0.201% for Clang `rxtvm`/`rxbvm` and +1.379%/+6.422% for GCC; all-inline remains within -0.280% to +0.520% of R2. Clang `run()` is 205,548/205,444 bytes; GCC retains 547,808/549,632 bytes. No public ISA/ABI, product default or cross-platform selection changes. Control: [`PERF3-05-R3-WORKLIST.md`](PERF3-05-R3-WORKLIST.md); report: [`vm-c-compiler-optimisation-report-2026-08-09.md`](../docs/planning/release-1/vm-c-compiler-optimisation-report-2026-08-09.md); evidence: [`2026-08-09-perf3-05-r3-handler-codegen-analysis`](evidence/2026-08-09-perf3-05-r3-handler-codegen-analysis/). |
| PERF3-05-R4 | P2 | RXAS/RXVM fusion registry and quickening contract | complete — accepted mechanisms retained, adaptive quickening closed | The post-RXAS portfolio-v2 census records 1,248 public sites and 34 exact private-eligible sites. Both accepted private fusions remain immutable load-time execution-image specializations with canonical fallback and public semantic attribution. No residual exact miss, public-opcode promotion or adaptive state is justified. Registry: [`PERF3-FUSION-REGISTRY.md`](PERF3-FUSION-REGISTRY.md); evidence: [`2026-08-17 fusion registry`](evidence/2026-08-17-perf3-closeout-fusion-registry/). |
| PERF3-05-R5 | P1 | Handler-placement percentage, never-inline and platform panel | complete — profile-20 retained; further VM layout work closed until release-candidate finalisation | Adrian selected common profile-20 as the provisional default and accepted the known GCC threaded Bounce guard. Formal Clang improves +3.857%/+3.152% for `rxtvm`/`rxbvm`; GCC improves +3.175%/+9.646% overall but Bounce loses 10.072% on threaded dispatch. On this host Clang profile-20 is directionally faster than GCC in all 14 workload/engine cells, with unpaired all-seven point estimates of +23.8%/+41.5%. Fresh no-option and explicit profile-20 Release binaries are byte-identical for both engines; default Release and Debug each pass 2,002/2,002 tests and focused profile/documentation QA passes 6/6. Effective placement remains visible as `inline`/`outline`/`mixed`. PERF3 takes no further platform/layout candidate: at Release 1 candidate freeze, regenerate the exact panel from the wider current portfolio and current target platforms before reviewing low-level VM shape. Control: [`PERF3-05-R5-WORKLIST.md`](PERF3-05-R5-WORKLIST.md); evidence: [`R5 panel`](evidence/2026-08-10-perf3-05-r5-handler-percentage-panel/), [`R5a placement`](evidence/2026-08-10-perf3-05-r5a-handler-placement-profiling/), [`R5b closeout`](evidence/2026-08-10-perf3-05-r5b-profile20-default-closeout/). |
| PERF3-06 | P0 | Qualified-deficit closure and Mac scorecard | complete — accepted product scorecard retained | The formal Apple refresh passes 348/348 initial plus 30/30 append executions. Common-five means are 2.453066x/2.285744x versus ooRexx and 0.912280x/0.850054x versus NetRexx. Richards and noisy Base64 remain common deficits; RexxCPS clears parity but not 1.50x, and Towers remains a separate deficit. Control: [`PERF3-06-WORKLIST.md`](PERF3-06-WORKLIST.md); evidence: [`2026-08-04-perf3-06-mac-scorecard`](evidence/2026-08-04-perf3-06-mac-scorecard/). |
| PERF3-07 | P2 | Capability and lifecycle side lanes | deferred/independent | Each approved product/capability use case has its own scope and does not distort the common benchmark programme. |
| CREXXRAG-SHA256 | P1 capability decision | Binary SHA-256 native RXPA versus pure-Level-B comparison | Option A selected; RCC-4 production path implemented and first verdict accepted 2026-08-20 | Correctness passes standard, embedded-zero and padding-boundary vectors on both concrete VMs. In the original serial profiling-off Release pilot, including the required noise appends, integrated native is 196.5x-252.9x faster in-kernel than straightforward pure Level B, reaching 164-407 MiB/s versus 0.765-1.726 MiB/s; the bulk RXPA path is within about 3-7% of the direct-C ceiling. Production now publishes the B+G `rxhash.sha256(data = .binary) = .binary` surface from process-reentrant standard/default provider `rx_hash`; direct RXPA declarations need no Rexx wrapper. Declarative provider metadata gives ordinary VM autoload and automatic native static-archive selection. The production implementation emits the same SHA-256 call-path instructions as the prototype; its accepted first-Release comparison is neutral-to-favorable on both VMs. Pure Level B remains reference/explicit fallback. Existing RXAS opcode 513 is unchanged. Control: [`CREXXRAG-SHA256-WORKLIST.md`](CREXXRAG-SHA256-WORKLIST.md); [selection evidence](evidence/2026-08-19-crexxrag-sha256-a-vs-d-gate/); [production verdict](evidence/2026-08-20-rcc4-rx-hash-first-release-verdict/). |
| PERF3-08 | P1 | Selected-candidate platform validation and default-VM decision | transferred to release-candidate finalisation | No PERF3 VM candidate remains open. Revisit the default/private-stream and low-level handler shape only at release-candidate freeze using regenerated Apple, Linux and Windows evidence from the then-current portfolio; defer explicitly if that panel is not decisive. |
| PERF3-09 | P3 | JIT/AOT/native-backend architecture decision | deferred | Reopen only under the recorded economic and architecture gate. |
| PERF3-10 | P0 | Trace-safe storage/component conversion proof | complete — C1/T1 accepted | Closeout passes 59/59 focused and 1,982/1,982 broad Debug tests. Paired RexxCPS median CPS improves 10.38%/10.61% on `rxvm`/`rxbvm`; equal-work profiling removes 1,399,605 dynamic instructions and 1,400,000 `ITOS`. Control: [`PERF3-10-WORKLIST.md`](PERF3-10-WORKLIST.md); evidence: [`2026-08-01-perf3-10-trace-safe-itos-closeout`](evidence/2026-08-01-perf3-10-trace-safe-itos-closeout/). |
| PERF3-11 | P0 | Scalable RXAS flow, signal policy and sparse component SSA | complete — K04e accepted | Gates 1-6, M01-M06 and K01-K06 are locked. D0.1-D0.5 provide explicit routes, one immutable graph, capability-lazy proofs, sparse transactional rewrites and a 142.7-142.9 MB Parse boundary. D0.6 retains the peephole as the permanent cheap pre-SSA stage and records a standing ownership rule for exact local metadata-proved transformations. Its accepted 100-record bound leaves the exact RexxCPS analysis and image unchanged versus 20 at an approximately 10 ms ordinary assembly cost. K04e restores the hot in-place integer compare/branch fusion through the source's pre-write ValueId and existing SSA-owned liveness, alias, cleanup and TRACE proof, without restoring the tactical rule. It removes one static instruction and exactly 560,000 equal-work dispatches; the 36-pair runtime verdict is noisy/inconclusive with positive point estimates and no guard hit. Broad Debug passes 2,021/2,021. Procedure-length windows remain a retained negative unless sparsity repays scan cost. The future ledger retains RXC-to-RXAS ownership, inlining redesign, hoisting, register work and the bounded region-proof follow-on. Control: [`PERF3-11-WORKLIST.md`](PERF3-11-WORKLIST.md); K04e verdict: [`2026-08-04-perf3-11-k04e-first-release-verdict`](evidence/2026-08-04-perf3-11-k04e-first-release-verdict/); D0.6 verdict: [`2026-08-04-perf3-11-d06-pre-ssa-boundary`](evidence/2026-08-04-perf3-11-d06-pre-ssa-boundary/); migration: [`PERF3-11-MIGRATION-WORKLIST.md`](PERF3-11-MIGRATION-WORKLIST.md); K06: [`2026-08-03-perf3-11-k06-mechanical-classification`](evidence/2026-08-03-perf3-11-k06-mechanical-classification/); K01: [`2026-08-03-perf3-11-k01-storage-permutation`](evidence/2026-08-03-perf3-11-k01-storage-permutation/); K02/K03: [`2026-08-03-perf3-11-k02-k03-linked-reads`](evidence/2026-08-03-perf3-11-k02-k03-linked-reads/); K04: [`2026-08-03-perf3-11-k04-call-window`](evidence/2026-08-03-perf3-11-k04-call-window/); M06: [`2026-08-03-perf3-11-m06-producer-forwarding`](evidence/2026-08-03-perf3-11-m06-producer-forwarding/). |
| PERF3-11-R1 | P2 future | Proven signal-policy region specialization | roadmap only — no closeout implementation authorized | RXAS already carries per-opcode signal contracts and sparse per-name handler-policy facts, including exact `IGNORE`, terminal `HALT`/`RETURN`, branch and inherited/merged states. An ignored or OS-disabled signal is not proof that an instruction cannot fail: its skip/resume continuation and failure-write phase remain observable, so current consumers correctly do not convert it to `signal.state == NONE`. Reopen only from a current profile that attributes material cost to signal checks or handler setup. Compare bounded RXAS region proof and compiler-emitted scoped policy minimization, but require proof that default halts, nested handlers, native delivery, signal phase, reference/link cleanup, TRACE/debug and both VM dispatch variants are unchanged. Do not add blanket compiler `SIGIGNORE` or treat disabled delivery as non-signalling. |
| PERF3-12 | P1 | Current RexxCPS clause-lowering rereview | complete — implementation queue accepted | The checksum-closed analysis ranks transactional PARSE, compound tails, copied XTOY, and later inlining/register work while ordering implementation by proof readiness. Control: [`PERF3-12-WORKLIST.md`](PERF3-12-WORKLIST.md); evidence: [`2026-08-04-perf3-12-rexxcps-clause-rereview`](evidence/2026-08-04-perf3-12-rexxcps-clause-rereview/); reassessment: [`2026-08-04-perf3-12-k04e-clause-reassessment`](evidence/2026-08-04-perf3-12-k04e-clause-reassessment/). |
| PERF3-12A | P1 | Cursorless RXAS and copied-XTOY placement | complete — accepted and published | Cursorless RXAS removes all optimizer-visible cursor boundaries. X1 atomically removes two of five generated `DCOPY`/`DTOS` sites and fixed-work optimized RexxCPS falls from 53,659,088/53,659,041 to 52,839,051 instructions under `rxvm`/`rxbvm`: -820,037/-819,990 (-1.528235%/-1.528149%). Both VMs remove exactly 820,000 `DCOPY` and 36,080,000 copied bytes while retaining all 2,220,000 `DTOS`; full assembly remains sparse at 0.51 s/134.7 MB. Focused RXAS passes 78/78, broad Debug has 2,034/2,034 functional outcomes, and final implementation is `4a480bbfa` on published `develop`. Old build/worktree RXBIN must be rebuilt. Control: [`PERF3-12A-WORKLIST.md`](PERF3-12A-WORKLIST.md); X1 verdict: [`2026-08-04-perf3-12a-x1-first-release-verdict`](evidence/2026-08-04-perf3-12a-x1-first-release-verdict/); cursorless verdict: [`2026-08-04-perf3-12a-cursorless-first-release-verdict`](evidence/2026-08-04-perf3-12a-cursorless-first-release-verdict/). |
| PERF3-12B | P1 | Compound-tail representation and loop-scoped reuse | complete — H1 accepted and merged | The selected capability-lazy H1 proof retains one conditional joined-key seed in a fresh private local and redirects four later proved-equivalent uses. It removes 1.96M hot CONCAT dispatches with no setup instruction; B4 is clear favorable at +3.075212%/+4.274944% paired median CPS on `rxvm`/`rxbvm`, and the clean B5 production verdict is clear on `rxvm`. Canonical RexxCPS emits 1,210 static instructions, `main 380 -> 365`, and `.locals=104`; Sieve is a byte-identical zero-candidate guard. The fresh merged-product Apple scorecard passes 348/348 with no append and keeps both VMs above the 2.00x ooRexx aggregate target. S1 remains a replayable rejected fallback. Control: [`PERF3-12B-WORKLIST.md`](PERF3-12B-WORKLIST.md); evidence: [`B4 comparative panel`](evidence/2026-08-04-perf3-12b-b4-comparative-panel/), [`B5 first verdict`](evidence/2026-08-05-perf3-12b-b5-first-release-verdict/), [`current Mac scorecard`](evidence/2026-08-05-perf3-12b-mac-scorecard/). |
| PERF3-12C | P1 | Dynamic PARSE planning, invariant hoisting and transactional result placement | superseded by completed PERF3-12D production route | The comparative PoC established the large opportunity and rejected a weak hand-hoist. PERF3-12D subsequently integrated exit-owned compiled-pattern lowering using existing instructions and retired generated `parseExec`; no separate transactional PARSE production item remains. Historical control: [`PERF3-12C-WORKLIST.md`](PERF3-12C-WORKLIST.md); evidence: [`2026-08-08-perf3-12c-dynamic-parse-poc`](evidence/2026-08-08-perf3-12c-dynamic-parse-poc/). |
| PERF3-12D | P1 | Exit-owned compiled pattern processing for PARSE, regex and PEG foundations | complete — integrated on develop | The certified exit lowers bounded runtime-delimiter and issue-667 dynamic-position forms to existing `strpos`/`substring`/integer/branch operations with no new opcode and retires generated `parseExec` use. The opening PoC improved one million parses by 16.18x/16.44x and fresh canonical RexxCPS by 4.33x/4.23x versus retained E0. The reported 2.20%/2.62% H02 regression is retracted because it crossed rebuilt runtime products. A corrected frozen-product 40-round matrix now crosses optimized RXC before/after internal-binding metadata suppression with old/new RXAS and a matched `scopy` control. Metadata suppression alone is neutral. Direct reuse versus the matching metadata-free old RXAS improves median CPS by 0.571%/0.595% on `rxtvm`/`rxbvm`; the complete change versus the original metadata-retaining old RXAS is +0.436%/+0.147%, a small positive tendency. Direct reuse removes 420,000 executed instructions (-1.326581%) and shrinks the retained canonical RXBIN 76,033 -> 73,161 bytes (-3.777307%). An enlarged 80-pair fallback panel and 120-pair combined sensitivity check do not prove `link` faster than `scopy`. Adrian selected direct reuse for internal bindings and a fail-closed, alias-lifetime-proved `link` fallback for metadata-visible exact registers on 2026-08-08, because avoiding payload copies should scale better for large strings and later binary/object values. The isolated implementation is `ef6e3fd77`; develop integration retains the existing inline RXAS queue snapshots and growth-time re-pinning with no per-record allocation. Final merged qualification passes full Debug and Release builds, 20/20 focused Release PARSE/RXAS/RXQUEUE tests, and full Debug CTest at 2,002/2,002. All 720 new recorded performance processes pass. Current instructions remain sufficient; no new instruction is justified. A packed `.binary` E2, generic string/binary/object internal-binding reuse and any E3 instruction remain separate gates. Control: [`PERF3-12D-WORKLIST.md`](PERF3-12D-WORKLIST.md); opening evidence: [`2026-08-08-perf3-12d-existing-instruction-poc`](evidence/2026-08-08-perf3-12d-existing-instruction-poc/); register verdict: [`2026-08-08-perf3-12d-register-reuse-verdict`](evidence/2026-08-08-perf3-12d-register-reuse-verdict/). |
| DECIMAL-01 | P1 independent | Decimal-provider correctness and performance engineering | Stage 3 complete — retain current `mc_decimal`; D2/D3/D4 rejected | Individual and combined numeric-context state/parity remains green. Debug and profiling-off Release pass the focused provider/VM qualification, and the retained current-provider blocks establish the present cost baseline without treating old checksum identity as performance evidence. D4 libmpdec passed the provider/lifecycle contract but failed formal L1. D2 then passed all 48 provider contracts, but its best balanced tuning build was 0.23%/1.67% slower at Common-18/Classic-9 and its best isolated gain was only 2.70%, so no build had credible formal-screen headroom. D3 matched its admitted 9/18 checksums and lifecycle boundary, but adapter arithmetic was 44.40%/64.76% slower and direct-core arithmetic 49.28%/66.73% slower. Contexts above 34 and native power remain explicitly unsupported. The first D2 timing attempt was invalidated in full after XProtect overlap; the verdict uses only the clear-host replacement capture. No default, ABI, hybrid or production change is selected. Control: [`performance/decimal`](decimal/); worklist: [`DECIMAL-01-WORKLIST.md`](decimal/DECIMAL-01-WORKLIST.md); current-provider evidence: [`2026-08-18-decimal-01-gate1-current-provider`](evidence/2026-08-18-decimal-01-gate1-current-provider/); D4 evidence: [`2026-08-18-decimal-01-libmpdec-screen`](evidence/2026-08-18-decimal-01-libmpdec-screen/); D2/D3 evidence: [`2026-08-18-decimal-01-stage3-calibration`](evidence/2026-08-18-decimal-01-stage3-calibration/). |
| RCC5-MATH | P1 approved production | Level G mathematics composition and historical `rxmath` split | RCC-5A and focused RCC-5B complete; RCC-5B verdict accepted; RCC-5C in progress; RCC-5D+ not started | The Level G standard mathematics family separates a process-reentrant native `rxfloat` provider from Level-B-authored `rxint` and `rxdecimal` modules. `rxmath` scalar compatibility names share the native procedures; statistics waits for the separately approved native-packed `BINARY-01` representation. Correctness uses libm, Euclidean/divide-before-multiply/binary-search/modular algorithms and guarded decimal Newton/range-reduced series rather than the defective or dormant historical implementations. RCC-5B's black-box contract suite independently covers all 37 procedures plus compatibility and native arity boundaries. A shared typed assertion tool supports the risk-weighted [mathematics validation strategy](../docs/planning/release-1/mathematics-validation-strategy.md); RCC-5C requires deeper exact-integer and independent high-precision decimal scenarios. RCC-5 has one consolidated full-QA closeout after its final approved subphase; intermediate slices use focused correctness plus any mandatory first-Release verdict. Control: [`RCC5-MATH-WORKLIST.md`](RCC5-MATH-WORKLIST.md); RCC-5B evidence: [`2026-08-20-rcc5b-rxfloat-first-release-verdict`](evidence/2026-08-20-rcc5b-rxfloat-first-release-verdict/); packed prerequisite: [`BINARY-01-WORKLIST.md`](BINARY-01-WORKLIST.md). |
| PERF3-13 / CAP-06 | P0 | RXVM allocator, worker and concurrency performance record | historical record complete; live status moved | The dated implementation stages, guard verdicts and exact evidence links remain in [`PERF3-13-WORKLIST.md`](PERF3-13-WORKLIST.md) and `performance/evidence/`. Current capability, documentation, portability and publication status is governed by [`concurrency/WORKLIST.md`](../concurrency/WORKLIST.md) and its implementation matrix. |
| PERF3-13-F3C1 | P0 | Sealed task-binding validation and resolution cache | complete — accepted | The selected four-set, two-way worker-local cache preserves first-use validation, computes the worker graph digest lazily and keys successful resolved plans by the complete binding/result mode. Full Mac QA passes. Tiny-task latency improves +39.076017%/+40.340609% on `rxbvml`/`rxtvml`; `rxtvml` throughput improves +1.551257%, `rxbvml` throughput is inconclusive and no cell hits the adverse guard. It changes no RXBIN or public semantics. Control: [`PERF3-13-WORKLIST.md`](PERF3-13-WORKLIST.md); [evidence](evidence/2026-08-15-perf3-13-gate-f-f3c1-task-binding-cache-first-release-verdict/). |
| PERF3-13-F3C1-B1 | P0 verification | Full task-launch and single-thread post-cache baseline | complete — guard-clean at 36-pair ceiling | The exact retained F1f and F3C1 Release VMs share every current library/workload image across Sieve, Permute, Bounce, Richards, Base64, Towers and canonical RexxCPS. Governance appends take neutral/noisy cells to 36 pairs; 1,036/1,036 processes pass with no removed sample. Common-five is clearly favorable at +1.055583% `rxtvm` and +0.939592% product `rxbvm`. `rxtvm` Towers and RexxCPS are clear small adverse observations below 3%; no individual or aggregate guard fires. Task launch remains separately favorable/inconclusive as recorded in F3C1. Control: [`PERF3-13-WORKLIST.md`](PERF3-13-WORKLIST.md); [evidence](evidence/2026-08-15-perf3-13-f3c1-full-baseline/). |
| PERF3-13-E3B-I1 | P0 diagnostic | RXPA branch-free load binding and sticky legacy transition | complete — ceiling selected and production form accepted | The 65/65-process isolated proof puts the load-selected direct invoker at -0.489662% paired mean versus raw direct, with a 95% interval of -1.228728% to +0.249404% and no 3% guard hit. The locked invoker is clearly adverse by +20.117255%, supporting direct binding for one legacy-capable executor and one sticky quiescent rebind only when a second is published. The integrated candidate subsequently passed its 312-process guard set and Mac closeout. P2 sessions and Gate F remain outside this completed diagnostic. Control: [`PERF3-13-WORKLIST.md`](PERF3-13-WORKLIST.md); isolated evidence: [`branch-free invoker PoC`](evidence/2026-08-10-perf3-13-gate-e-e3b-p1-branch-free-invoker-poc/); production evidence: [`accepted branch-free verdict`](evidence/2026-08-10-perf3-13-gate-e-e3b-p1-branch-free-first-release-verdict/). |
| PERF3-13-F1 | P2 | Single-character `SCOPY` fast-path PoC | queued after M3; evidence gate only | First quantify one-character `SCOPY` incidence and static-site/value shapes across the representative portfolio. Only then compare the exact C ceiling and narrow implementation forms for an already-owned, already-capacious destination, preserving first-use allocation, empty/long strings, aliasing, UTF/cache/status/private flags, foreign/reference payloads and instrumentation. Measure branch cost on all copies, `run()` text/layout, ordinary Release timing, RSS and artifact size; do not optimize Base64 alone. V1-L01 did not select a fast path, and this entry authorizes no current VM change. |

## Approved execution order

1. **Approve the control plane.** Review PERF3-00 and amend scope, priorities,
   target bands or transfer dispositions before activity work begins.
2. **Establish current Mac truth.** PERF3-01 audits every product-affecting
   change since the accepted Mac scorecard, replays the evidence manifests
   actually used, and decides exactly which current-HEAD timing/profile cells
   must be refreshed. It stops with a ranked panel and no production edit.
3. **Lead with proved work, not subsystem preference.** Unless PERF3-01
   overturns the retained evidence, PERF3-02 is the first design/PoC activity.
   It compares compiler, RXAS and runtime ownership for exact copy/value shapes.
4. **Keep orthogonal risks separate.** PERF3-03 conversion semantics,
   PERF3-04 accessor proof and PERF3-05 native layout/stream work do not ride
   inside a copy candidate. Each receives its own entry criterion and stop.
5. **Apply the mandatory first Release verdict.** After Adrian selects one
   production candidate, run only the minimum focused correctness needed,
   freeze implementation, build the ordinary profiling-off Release product,
   run the smallest decisive paired target plus guards, report and stop.
6. **Close accepted slices proportionally.** Broad QA, sanitizer,
   install/package, compatibility and documentation follow only after the
   first verdict is accepted and in proportion to the changed surface.
7. **Refresh outcomes deliberately.** PERF3-06 runs the formal Mac scorecard
   after an accepted group of slices or when a ranking decision requires it,
   not after every small edit.
8. **Validate, then select architecture.** PERF3-08 reuses retained Linux and
   Windows evidence, adds the selected candidate and still-required supported
   Linux ARM64 lane, and only then recommends the default VM/private stream.

The dependency shape is:

```text
PERF3-00 roadmap approval
└── PERF3-01 current Mac truth and accepted ranking
    ├── PERF3-02 copy / ownership / attribute storage
    ├── PERF3-03 conversion contract and ceiling
    ├── PERF3-04 concrete scalar accessor proof
    └── PERF3-05 compiler / layout / private stream

accepted independently gated production slices
└── PERF3-06 formal Mac outcome scorecard
    └── PERF3-08 Linux x86-64 -> Linux ARM64 -> Windows validation
        └── default VM / private execution recommendation

PERF3-07 capability lanes remain independent.
PERF3-09 remains deferred unless the non-JIT economic gate fires.
```

## PERF3-01 — current-HEAD Mac truth gate

Started 2026-07-31 at local `develop` commit `3f43a0014`. The resumable
evidence-only control plane is
[`PERF3-01-WORKLIST.md`](PERF3-01-WORKLIST.md). No production edit or candidate
selection is authorized in this activity.

Evidence collection is complete and Adrian accepted it on 2026-07-31. Clean current
Release timing records a common-five `2.139811x/1.818954x` versus ooRexx and
`0.779920x/0.662974x` versus decimal NetRexx for `rxvm`/`rxbvm`. Richards
remains the dominant common deficit; current deterministic counts plus retained
Linux native attribution keep PERF3-02 first. The evidence panel ranks
PERF3-05 second, PERF3-03 third and keeps PERF3-04 evidence-gated. See the
[`decision summary`](evidence/2026-07-31-perf3-01-current-mac/decision-summary.md).

### Question

Which retained PERF2 baseline and attribution conclusions remain valid for
exact current HEAD, and which mechanism should receive the first PERF3 design
panel?

### Required work

1. Freeze branch, exact commit, dirty scope, host/power/toolchain and ordinary
   profiling-off Release product identity.
2. Audit product-affecting changes since the 2026-07-27 Mac scorecard. Compare
   compiler, VM, library, workload, tool and manifest hashes before deciding a
   retained cell is reusable.
3. Replay only the retained checksum manifests actually used for decisions.
   Preserve and reconcile the untracked generated lifecycle files as a named
   repository-state issue; do not normalize, delete or regenerate evidence
   casually.
4. If the old Mac scorecard is not valid for current-product ranking, capture
   the smallest governed refresh that restores authority. A representative
   multi-workload set includes RexxCPS; common aggregate claims require all
   five common workloads.
5. Refresh diagnostic counts/native samples only where existing Linux and Mac
   artifacts cannot distinguish the candidate owners. The expected focused
   set is Richards, Towers, Base64, RexxCPS and Sieve as a zero-work/layout
   guard, in both VMs; this is the expected set, not an automatic requirement
   to rerun every profile.
6. Produce a current gap/mechanism ledger with exact operation counts, bytes,
   native footprint, machine ceiling, semantic risk and earliest safe owner.

### Exit and stop

Adrian accepts the current-product evidence boundary and a ranked PERF3-02/03/
04/05 panel. No performance production source is edited, no candidate is
silently selected and no broad platform rerun occurs in PERF3-01.

## PERF3-02 — full-copy, ownership and attribute-storage panel

Started 2026-07-31 at local `develop` commit `e38e514bf`; its product-code
parent remains `3f43a0014`. The resumable control plane is
[`PERF3-02-WORKLIST.md`](PERF3-02-WORKLIST.md). Adrian authorized the bounded
evidence/design and isolated C1-C4 PoC comparison, not a production candidate,
architecture/ISA/ABI change, commit of later evidence or push.

The initial panel completed with C1a-R2 removing 4,910,249 total Richards copy operations
and authoritative clean-host paired Release elapsed improves 9.18%/9.33% on
`rxvm`/`rxbvm`. C1c-R1 removes 7,140,440 total Towers copy operations,
55,158,560 logical bytes and 202,314 attribute blocks, improving 19.42%/19.65%.
Every target pair is favorable and all target mean intervals exclude zero.
Both pass the focused optimized/no-opt dual-VM matrix and keep their opposite
workload image byte-identical; those four guards pass the +3% budget at the
36-pair cap while remaining noisy around zero. C1c-R1 is recommended first;
C1a-R2 remains an independent alternative. The original remote-terminal timing,
exact products, rejected variants and every clean-host sample are preserved.
C1a-R1 is correctness-invalid, C2 accepts zero full copies with current facts,
and C3's zero-byte scalar residual is immaterial. At that initial gate no
production candidate was selected. See the
[`decision summary`](evidence/2026-07-31-perf3-02-copy-ownership-panel/decision-summary.md).

The approved follow-on analysis-only C1b gate is also complete. It proves the
C1a-R1 failure is caused by four taken Boolean-guard returns bypassing
receiver-derived `unlinkn` cleanup; later register reuse writes through stale
aliases into the scheduler object. Common-exit normalization is already
present and insufficient, while retaining private storage with per-exit
copyback removes no target copy. A narrow candidate-local rewrite that first
snapshots every receiver-derived scalar guard, then directly binds the
receiver only under a fail-closed structural and same-frame-signal gate, is a
bounded isolated PoC candidate. Its ceiling is two static/172,394 dynamic full
receiver copies, 25,341,738 recursive operations and 201,354,752 bytes, with
small scalar snapshot/cleanup work retained. Adrian approved the isolated
correctness PoC on 2026-08-01. Its exact structural recognizer and post-clone
validation preserve canonical `23246/9297` on `rxvm` and `rxbvm` with
optimization on and off, reduce the total static copy count by two with both
target receiver copies absent, retain six scalar snapshots, and leave Towers
plus the class-method control
byte-identical. The independent P1 storage proof keeps exact link/unlink
balance, removes one full-copy event from each target procedure and reduces
unknown join state to zero. That correctness-only gate performed no timing;
the later R1 repanel measures C1b alone at 44.28%/44.01% lower Richards elapsed
and the safe C1a+C1b composition at 53.55%/52.57%, with 12/12 favorable pairs
in both VMs. See the
[`C1b analysis`](evidence/2026-07-31-perf3-02-c1b-analysis/analysis.md) and
[`R1 evidence`](evidence/2026-08-01-perf3-02-r1-repanel/README.md).

The R1 replay retains eight build masks and rejects the broad rule both alone
and in combination. C1abc emits the exact C1ab Richards image and exact C1c
Towers image. The locked storage service finds 59 remaining exact-base full
copies in combined Richards and 18 in C1c Towers, but independent ownership,
destruction, value, lifetime, TRACE and continuation proofs remain absent, so
C2 installs no rewrite. C3 remains a zero-byte scalar residual and C4 is met
by the correct C1 paths. Adrian subsequently selected C1abc; the ordinary
compiler now contains that one ladder, the disposable replay mask is removed,
and the checksum-closed replay source preserves all investigated options.

### Question

Can cREXX remove or narrow the current high-cost full-value copies before they
reach `copy_value`, while preserving by-value isolation, reference identity,
recursive attributes, native payloads, unwind and observable intermediate
state?

### Required comparison

| Variant | Owner | Question |
| --- | --- | --- |
| C0 | current product | Exact caller, payload-shape, byte and lifetime baseline. |
| C1 | `rxc` semantic proof | Can typed flow/inline facts eliminate the copy or directly place the result? |
| C2 | RXAS whole-procedure proof | Can existing CFG/liveness/effects prove a full-copy projection without compiler-only knowledge? |
| C3 | narrow typed/payload operation | Can a proved scalar, binary or no-payload shape avoid general recursive copy work? |
| C4 | runtime machine ceiling | What is the exact direct operation cost after every eligible semantic decision is preproved? |

Richards, Towers and the CRI-13 residual are evidence sources, not permission
for benchmark-specific handling. Compare `rxvm` and `rxbvm`; include existing
Sieve/Permute/Bounce wins as appropriate guards. Keep copy elimination,
attribute-storage trimming and teardown independently attributable.

Before any production edit, create a resumable PERF3-02 worklist with at least
two viable production approaches, exact semantic obligations, machine-level
ceilings and a declared first Release stop. Present the PoC panel to Adrian for
selection; do not install a production ladder automatically.

## PERF3-03 — bounded conversion review

Freeze the current `rx_string_to_double` and `string2integer` allocation,
grammar, whitespace/sign, locale, range, exception and rounding contracts.
Compare the existing temporary NUL-terminated copy with bounded no-copy,
locale-independent and correctly rounded controls across signed zero, halfway
rounding, normal/subnormal, finite/overflow, long input and `inf`/`nan`
compatibility.

This activity starts as evidence/design only. A public RXAS operation, changed
conversion signal, ABI or language contract is a separate decision. It may
advance only if PERF3-01 confirms material current cost and the candidate is
general beyond CRI-13 or RexxCPS.

## PERF3-04 — generic final/concrete scalar access

Test whether statically resolved final/concrete scalar getters and setters can
approach direct typed-memory cost using generic compiler proof. Cover receiver
initialization and identity, writable ownership, signals, source/debug
identity, imports, optimized/no-opt output and both VMs. The hand-equivalent
ceiling precedes any production form.

Do not add JSON-, vector- or numeric-width-specific methods/opcodes. Missing
proof rejects only the affected candidate site; the ordinary call and
materialized path remain valid.

The post-PERF3 Havlak audit adds `POSTPERF-04-PACKED-01`. Compare the existing
checked canonical little-endian `BGETI64` path with a compiler-proved fast path
and an explicitly selectable aligned `rxinteger` accessor/view. The latter
checks alignment, extent, bounds and compatible signed-64 host representation
once, then targets one alias-safe native load/store per element. Its public
syntax and whether it is backed by `.binary` or a future packed container are
not selected. Unproved or incompatible sites retain the portable checked path.

## PERF3-05 — compiler, layout and private-stream panel

This activity owns `PERF2-06-D01`, the unstarted PERF2-10 options and the
compact/hot-cold private execution question. It does not own semantic copy or
conversion work that can be removed earlier.

The bounded Mac panel may compare:

- current source/product as a zero-work drift control;
- semantic Base64 string/copy ceilings before native layout changes;
- hot/cold handler/helper outlining and source ordering;
- supported LTO/interprocedural and representative PGO controls;
- private compact operand/stream or decoded hot/cold representations while
  canonical RXBIN remains unchanged; and
- `rxvm` and `rxbvm`-specific forms only when a common semantic fallback and
  maintenance boundary are explicit.

Record build time/memory, `run()` text/data, retired instructions, front-end,
branch/I-cache evidence, startup/preparation, RSS, artifacts and both VM modes.
A compiler or flag movement without an exact causal mechanism is a control, not
a candidate. A compact stream requires representative benefit on at least two
architectures before adoption.

The 2026-08-01 panel retains the ordinary L0 product. ThinLTO changes `__text`
by roughly -6.3% but fails Sieve and Base64 guards with VM reversals.
Representative PGO changes `__text` by roughly -25% but over-specializes the
layout, regressing Sieve 33.7-40.4% and Base64 27.7-48.8%. The isolated
no-flatten PoC improves focused compile time and build RSS but regresses the
non-noisy `rxbvm` Sieve guard by 3.639%. These are retained replayable
negatives, not production candidates. The semantic Base64 controls instead
show a 2.69-10.69x ceiling in generated work, so L4 is not opened without a
new mechanism.

The adjacent VM-library link question is separated from runtime layout. The
Mac build does expose implementation details: `libcrexxsaa` exports 367 globals
for a 16-symbol intended surface, `crexxsaa` propagates static implementation
archives and internal include paths as `PUBLIC`, and the static phase-API
object pulls the monolithic interpreter even for lifecycle-only use. Export
filtering does not change measured dylib relink time; removing propagated
archives reduces the downstream test link from a 35.095 ms median to 21.706
ms. `PERF3-05-B1` therefore owns optional build/API cleanup and must first
reproduce the reported slow client/host.

## PERF3-06 — qualified-deficit closure

Track each accepted product slice against the qualified score rather than one
headline benchmark:

- guard Sieve, Permute and Bounce;
- close Richards and Base64 only with general mechanisms;
- keep RexxCPS separate and first-class in sampling;
- keep Towers as a qualified separate object/allocation lane; and
- retain Mandelbrot, Storage, List and JSON under their approved explicit
  no-ratio dispositions unless a separately approved equivalence decision
  changes them.

The formal Mac refresh uses the ordinary profiling-off Release product, exact
current sources/images, two warmups and ten serial recorded observations per
absolute cell. A before/after verdict uses at least one warmup and twelve
balanced/interleaved pairs, with governance append rules. No unmatched
historical ratio becomes a regression claim.

Completed 2026-08-04 at clean accepted product `5fbe36049`. The initial
29-cell matrix passes `348/348`; the governed ooRexx Bounce and dual-VM Base64
append passes `30/30`. All three appended cells remain noise-labelled, no
sample is removed and no second append is taken. The exact common-five means
are `2.453066x/2.285744x` versus ooRexx and `0.912280x/0.850054x` versus
decimal NetRexx for `rxvm`/`rxbvm`. Richards remains the largest qualified
common deficit; Base64 remains noisy and below parity. RexxCPS clears parity
at `1.151301x/1.133307x` but not its 1.50x band, while Towers remains a
separate qualified deficit at `0.390842x/0.389933x`. Evidence:
[`2026-08-04-perf3-06-mac-scorecard`](evidence/2026-08-04-perf3-06-mac-scorecard/).

## PERF3-07 — capability and lifecycle side lanes

- `CAP-02` owned heterogeneous/nested containers remains a separate
  post-Release 1 Level G or explicitly approved library/runtime decision.
- Carry Base64 into `CAP-03` as a pure Level B library/API task: specify the
  standard Level B API, provide a reference implementation and add focused
  correctness/algorithm tests. It remains separate from the qualified common
  codec-loop benchmark and does not imply native, VM or opcode work without
  separate approval.
- `CAP-04` pure-load lifecycle remains a measurement/API-use-case question and
  enters public API work only with an approved product need.
- `CAP-05` explicit RXBIN module initialization is logged for a separate
  lifecycle design. The linker would record declared module initializers and
  VM prepare would invoke each exactly once after linking and before the
  application `main`. Selection must define ordering, idempotency, signals and
  failure, re-entrant calls, late-loaded modules and optional teardown. A bare
  `_init` name scan and a library-owned wrapper `main` are not selected; the
  current `rxvm_prepare()` only prepares execution images, so libraries must
  continue to initialize shared cREXX state lazily meanwhile.
- `CAP-06` is active as PERF3-13. Gate A M0 audited every allocator-eligible
  RXVM and bundled-plugin allocation, retained size/lifetime evidence and
  foreign/OS exceptions without changing runtime code or running a timed cell.
  It selects typed fixed-size silos plus power-of-two variable-byte classes,
  served by one worker heap and a central 64 KiB whole-slab depot; a universal
  power-of-two object allocator is retained only as a control. Gate B was
  approved on 2026-08-05 and completed/accepted on 2026-08-06. The corrected
  pairwise-balanced Release verdict selects the unchanged `value`
  single-worker allocator as the Gate C baseline; the selected Apple Clang
  product lane is +20.060456% on the stable-six geometric performance ratio,
  with no material RSS concern. Gate C was approved on 2026-08-06 and opened
  with an unchanged-value census before freezing its slab/oversize and
  stepwise compact-value panel. C1 completed on 2026-08-06 and now stops for
  C2 approval before PoCs. C2 was approved on 2026-08-06 and first compares
  S0/S1, the 128/16 KiB bridge and S2 on unchanged V0/R0 with byte-normalized
  depot reserves, then stops at its first Release verdict. Its initial capture
  was invalidated before retention because the S0 selector added a hot branch.
  Corrected S0 now matches all 22 allocator text symbols; the valid short screen is
  timing-neutral, identifies S1 as allocator-memory lean, S1b as the best
  balanced survivor and S2 as an RSS/retention reject. Adrian approved the
  formal S0/S1/S1b survivor panel on 2026-08-06. It completes without a guard
  hit; Adrian accepts S0 as the balanced V0/R0 substrate and authorizes V1.
  V1 now removes only the inline string and stops at its first Release verdict
  before V2a, `rxtvm` or reclamation. It
  rejects the 1 MiB variants, and confirms exact 240/208/192/160/120-byte
  value models. Gate C subsequently selected and Adrian accepted 176-byte
  L32SDH on 2026-08-07. Gate D cleanly industrialises that one layout and R0
  sticky reuse; its Mac local closeout is complete with final correctness,
  sanitizer, install and two-block Release evidence. Automatic reclamation is
  deferred to a separately approved Gate D-R, and Gate E adapts the policy then
  in force to worker ownership. The spawn diagnostic proves a worker is
  single-thread-owned and cannot be shared by stdout/stderr capture threads.
  Adrian opened and accepted EF-0 on 2026-08-07 as a deliberately narrow
  combined Gate E/F recovery slice. It re-engineers spawn redirects around
  immutable input and independently owned single-shot byte completions,
  synchronized by join and converted only by the receiver worker. Full Debug
  is restored to 1,996/1,996; focused ASan passes 34/34 and the combined
  ordinary Release closeout passes 38/38. This is the first vertical slice of
  the coherent multithreading architecture and uses the existing low-level
  native/cREXX redirect boundary; it does not open general public channel
  instructions. The preferred later Rexx-visible channel value remains
  register-centric: one logical scalar/binary register image with optional
  child-register images, materialized into receiver-owned VM registers.
  Cross-worker ownership,
  registration/teardown, depot synchronization, allocation failure, late load,
  native/plugin boundaries, deterministic cleanup and Windows CRT compatibility
  remain mandatory proofs. EF-0 is published in `642e1b697` on synchronized
  `develop` base `19802842e`. Adrian accepted the E1 single-worker ownership
  shell on 2026-08-07. Its first Release verdict is pooled-neutral with no
  guard hit, and Mac closeout passes focused ASan 3/3, full Debug 1,997/1,997
  and ordinary Release 14/14. A 60-pair post-closeout review isolates the
  RexxCPS loss to lifecycle/error code inside flattened `run()`; the equivalent
  lifecycle wrapper is neutral against the post-EF-0 control. Adrian approved
  E1-P1 to retain ownership semantics while restoring the flattened core's
  register/stack pattern. Adrian accepted its neutral core-four verdict and
  full QA passes focused ASan 3/3, Debug 1,997/1,997 and Release 14/14. The E1
  series is published through `84d406904`; GitHub Actions passes all 1,999
  Windows-MinGW tests and the complete build matrix. Adrian approved E2
  explicit-active-state implementation on 2026-08-07. Its first Release
  verdict isolated an adverse two-word interrupt poll; Adrian approved the
  bounded single-word counterfactual in which the product main VM owns the OS
  interrupt target and every other context polls only its own word. The first
  context-field form restored RexxCPS but left Sieve clearly adverse. The
  approved direct execution-slot form restores Sieve to neutral; RexxCPS is
  clearly adverse at `-1.206404%` but remains inside the 3% guard. Adrian
  accepted that verdict and the bounded loss. Mac closeout passes Debug
  1,999/1,999, full AddressSanitizer 1,999/1,999 and focused Release 49/49.
  The sanitizer sweep also exposed and repaired an independent RXAS sparse-batch
  snapshot UAF without per-record allocation. E3 subsequently completed the
  process-reentrant, mixed-policy and per-VM-session plugin model on Mac, with
  ODBC as the industrial session example. E4a retains the independent-load
  correctness control, and E4b now implements the internal bytecode-only sealed
  immutable-generation/worker-overlay boundary with a guard-clean verdict and
  Mac Debug/ASan/Release closeout. E5 integrates the native POSIX and Windows
  carriers plus the targetable-only sparse fallback with the private
  correlated mailbox, copied logical register image and deterministic terminal
  lifecycle. Adrian accepts the cleared-host computed-goto slowdown and the
  existing Linux/Windows evidence; Mac QA is complete after its guarded
  initializer repair. E6 selects strict allocator C0 ownership, rejects and
  removes C1/C2, retains private 1/2/4/8-worker compute/churn qualification and
  passes Mac Debug/ASan/Release closeout. Gate F and transport-neutral public
  channel semantics remain deferred.
  Control:
  [`PERF3-13-WORKLIST.md`](PERF3-13-WORKLIST.md); E1 evidence:
  [`2026-08-07-perf3-13-gate-e-e1-worker-shell`](evidence/2026-08-07-perf3-13-gate-e-e1-worker-shell/);
  E1-P1 evidence:
  [`2026-08-07-perf3-13-gate-e-e1-p1-wrapper`](evidence/2026-08-07-perf3-13-gate-e-e1-p1-wrapper/);
  EF-0 evidence:
  [`2026-08-07-perf3-13-ef0-spawn-recovery`](evidence/2026-08-07-perf3-13-ef0-spawn-recovery/).
  E5 closure evidence:
  [`2026-08-13-perf3-13-gate-e-e5-industrial-closeout`](evidence/2026-08-13-perf3-13-gate-e-e5-industrial-closeout/).
  E6 closure evidence:
  [`2026-08-13-perf3-13-gate-e-e6-first-release-verdict`](evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/).

These lanes can proceed under their own authority but do not borrow PERF3
performance approval or alter benchmark equivalence silently.

### DECIMAL-01 — independent decimal-provider engineering

Adrian opened a separate decimal-library review on 2026-08-05. Its control
plane is [`performance/decimal`](decimal/), and its detailed evidence/design
contract is
[`DECIMAL-01-ENGINEERING-PLAN.md`](decimal/DECIMAL-01-ENGINEERING-PLAN.md).

The first gate inventories all decimal operations and Mike Cowlishaw numeric
options, classifies current `FUZZ`, Common/Classic, quotient, signal and
platform-precision gaps, and freezes an independent correctness oracle. It
also resolves the unconfirmed report about the individual RXAS numeric setters
and getters and the compiler-used combined `NUMSCI`/`NUMENG` path. The
recommended first measured panel then compares current/tuned `decNumber`,
`libmpdec` and already-vendored fixed-34-digit `decQuad`. `db_decimal` is split
between an unrestricted diagnostic ceiling and a separately labelled,
workload-qualified Classic-9 speed control for the ooRexx comparison. Intel,
Boost, GCC decimal, a native-64-bit-limb fork and a hybrid fixed/arbitrary
provider are later gated candidates, not selected work.

Candidate sources and disposable native comparators remain outside the product
tree; maintained orchestration is Level B cREXX. Any value tag, plugin ABI,
default-provider or production edit requires a separate decision and the
mandatory first ordinary Release verdict.

Before the extended candidate panel, DECIMAL-01 must retain a public-evidence
dossier covering upstream claims, independent research, reproducibility,
versions, platforms, correctness suites, licences and maintenance. Public
figures prioritize local PoCs but never become CREXX results. This Mac is a
shared performance host: every timing session requires Adrian to clear and
reserve it first; correctness-test elapsed time is not performance evidence.

The 2026-08-18 first candidate panel is now closed. D4 libmpdec failed formal
L1; D2's 48-build tuning grid showed no credible headroom; and D3 decQuad was
materially slower in both adapter and direct-core arithmetic. Retain the
current 8/64/64 `mc_decimal`. Extended candidates D5-D9 remain rejected ideas
or future roadmap hypotheses rather than next work: Stage 3 produced no
material question that would justify opening them, and no provider, ABI,
fixed/arbitrary hybrid or production integration change is selected. The
enduring no-repeat rationale and explicit reopening triggers are in
[`DECISIONS.md`](DECISIONS.md).

## PERF3-08 — platform validation and architecture selection

The approved order is:

1. Apple ARM64 design, PoC and mandatory first Release verdict;
2. retained Linux x86-64 evidence audit, followed by a batched selected
   candidate run only when required;
3. supported Linux ARM64 correctness, timing and relevant counter coverage;
4. supported Windows x86-64 correctness, scorecard and artifact validation;
5. whole-scorecard default VM/private representation decision.

The existing Windows GCC/Clang/MSVC and `/MT` results remain controls. A static
CRT cannot become the default until allocator ownership across executables,
DLLs, plugins and public APIs is proved. A later cross-OS RexxCPS validation
should compare the benchmark-reported timer with an external monotonic timer.

The final decision must state whether `rxvm`, `rxbvm` or a platform-specific
choice is the default; which private execution representation is selected;
which supported compilers/options are normative; and which deficits remain.
Canonical RXBIN portability, public ABI, TRACE/source/debug identity, late
load, plugins, lifecycle, RSS and artifacts remain hard dimensions.

## PERF3-09 — JIT/AOT/native-backend decision

Keep this deferred. Reopening requires current residual gaps after accepted
PERF3 non-JIT work, a comparison of native AOT, baseline JIT, tracing,
quickening and existing-VM maintenance, and explicit treatment of debugger/
TRACE, signals, dynamic loading, plugins, portability, packaging, sandboxing
and reproducible builds. Adrian must approve a separate architecture programme
before implementation.

## Worklist and evidence contract

Before the first production edit in any PERF3 activity, create a resumable
worklist recording:

1. exact baseline commit, branch/upstream/dirty state and artifact hashes;
2. one falsifiable hypothesis and named current mechanism footprint;
3. status quo plus at least two plausible implementation owners/forms when
   two exist, or a recorded reason why only one is viable;
4. machine-level ceiling and exact semantic proof obligations;
5. focused correctness and regression-guard matrix;
6. ordinary profiling-off Release comparison and hard first-verdict stop;
7. proportional closeout only after Adrian accepts the verdict;
8. accepted, rejected, neutral and invalidated evidence; and
9. dated status links back to this roadmap.

All maintained performance orchestration remains cREXX Level B under
`performance/tools/`. Temporary host-side analysis may be used outside the
repository, but it does not become the maintained control plane.

## Approval record

Adrian approved these five points on 2026-07-31:

1. PERF2 is closed at its recorded state and this file is the live PERF3
   control plane.
2. PERF3-01 is the only authorized next activity, with no production edit and
   a hard evidence/ranking stop.
3. Full-copy/ownership is the provisional first design panel unless PERF3-01
   current-product evidence overturns it.
4. Conversion, accessor, layout/stream, capability and JIT/AOT work remain
   behind their separate entry and decision gates.
5. Mac iteration is followed by selected Linux x86-64, required Linux ARM64
   and Windows validation before the default-VM/final architecture decision.

On 2026-08-05 Adrian requested the independent DECIMAL-01 review and
performance-engineering plan. He subsequently approved the seven-step gated
plan and opened Gate 0 correctness work only. Gate 0 must freshly validate the
reported individual/combined RXAS numeric-context concern. The later public
candidate panel must be informed by source-attributed public evidence. No
candidate, experimental provider, default-provider change or production
decimal edit is selected. Before any performance run Adrian must be asked to
clear and reserve the shared host, and any pause request is binding before the
next cell.

Adrian subsequently accepted the PERF3-01 current-product evidence boundary
and ranked PERF3-02/03/04/05 panel on 2026-07-31. This closes PERF3-01 and
opens PERF3-02 for its bounded evidence/design and PoC comparison only.

On 2026-08-01 Adrian approved P1A followed, after its mandatory Release
verdict, by the bounded PERF3-05 and PERF3-03 evidence/design gates. Each
production candidate still requires its recorded selection and first-Release
stop; this approval does not preselect a public format, layout or changed
conversion contract.

The first P1A A1 verdict was correct but neutral. Adrian authorized the bounded
A3 rework on 2026-08-01; A1 evidence and binaries remain retained as an oracle,
and PERF3-05 stays blocked until the repeated A3 Release verdict is accepted.

The repeated A3 verdict is correct but shows no selectable gain at the governed
36-pair maximum. It remains frozen for Adrian's revert-or-accept decision;
PERF3-05 has not begun.

Adrian accepted the recommended disposition on 2026-08-01: retain A1, reject
and remove A3, complete P1A closeout, then open PERF3-05.

P1A closeout is complete and PERF3-05 is open at its bounded Mac
evidence/design gate. This authorizes isolated comparators, not a production
architecture edit or default/private-stream selection.

The PERF3-05 bounded panel is complete and stopped at selection on 2026-08-01.
Its recommendation is to retain the ordinary product, reject LTO, PGO and the
no-flatten runtime form, and leave L4 unopened. The separately diagnosed VM
library link-interface cleanup is queued as `PERF3-05-B1`; no CMake/API change
is bundled into the runtime decision. PERF3-03 has not begun.

Adrian accepted that PERF3-05 disposition on 2026-08-01. PERF3-05 is closed
with no production VM edit, PERF3-05-B1 remains independently queued, and the
already approved PERF3-03 bounded evidence/design gate is open.

The PERF3-03 evidence/design gate reached selection on 2026-08-01. The
recommendation is the private locale-aware C4 v3 loose-comparison prefilter:
retain the exact current converter on numeric/uncertain spans, add no public
opcode/API/serialized form, and defer the value cache. C4 v1 is rejected for a
confirmed Base64 `rxvm` guard regression after lost caller inlining; v2 is
rejected unchanged for incomplete active-locale proof. No production edit has
been made at that selection gate. A subsequent governed common-layout guard retained 348/348 correct
executions. Permute, Bounce and Richards execute zero loose comparisons; their
six VM cells show four favourable results, one -0.446% small adverse median
inside guard and one +0.147% neutral median at the 36-pair cap.

Adrian selected C4 v3 on 2026-08-01. The production edit is limited to the
portable no-inline private rejector and exact current-converter fallback.
Minimum Debug validation passes 6/6. The mandatory ordinary Release verdict
retains 212/212 correct executions: Base64 paired medians are +4.859%/+5.780%
and RexxCPS is +2.517%/-0.609% on `rxvm`/`rxbvm`; the latter is noisy/neutral
at cap and no cell reaches the -3% guard. The provisional implementation is
stopped for Adrian's verdict acceptance before broad closeout.

Adrian accepted the first Release verdict on 2026-08-01. Proportional Apple
closeout then passed 1,972/1,972 full Debug tests, 6/6 focused ASan tests, the
complete ordinary Release build and 136-file isolated install, plus installed
VM smoke 2/2. The leak-enabled sanitizer attempt is retained as unsupported on
this macOS runtime, and no local Windows cross-toolchain is available; real
MSVC validation is queued as PERF3-03-W1 before publication. PERF3-03 is
complete on Apple and PERF3-04 remains queued. Adrian subsequently authorized
the combined local closeout commit; no push is authorized.

Adrian then approved PERF3-10 on 2026-08-01. The selected C1/T1 candidate is
ordered TRACE result-event batching plus a reusable storage-identity and
component-aware RXAS fact for redundant `ITOS`. C0-C4 and T0-T2 remain
recorded for replay. This candidate preserves TRACE event count/order/value
because the same proved value remains available; that is its selected local
quality property, not a universal restriction on optimisation. It must fail
closed across unproved writes, calls and signal phases, and stop after its
minimum correctness gate and mandatory first ordinary Release runtime verdict.

That stop gate was reached and Adrian accepted C1/T1 on 2026-08-01. The
ordinary Release comparison passes 72/72 executions. Combined paired median
CPS changes are +10.376% on `rxvm` (21/22 favourable) and +10.612% on `rxbvm`
(12/12 favourable). An equal 200 x 100 counts-only diagnostic removes
1,399,605 total dynamic instructions (2.504%) and 1,400,000 `ITOS` executions
(55.555%). No sample was removed. The affected Debug product then passes 59/59
focused and 1,982/1,982 broad tests. Exact C0/C1 artifacts, timing, profiles,
the reviewed legacy TRACE expectation update and closeout logs are retained in
the checksum-closed
[`PERF3-10 evidence bundle`](evidence/2026-08-01-perf3-10-trace-safe-itos-closeout/).
T2 and wider conversions remain outside the accepted slice; no push is
authorized.

## PERF3-11 — component generations and signal-phase flow

Adrian approved the scalable per-procedure graph, dedicated signal-policy and
sparse component-SSA architecture on 2026-08-02.  The complete staged plan,
semantic-change boundary, clean-base oracle, scaling gates and mandatory first
consumer verdict are controlled by
[`PERF3-11-WORKLIST.md`](PERF3-11-WORKLIST.md).  Production execution is
authorized subject to its mandatory gates; no push is authorized.

PERF3-10 proves the architectural direction but intentionally implements one
consumer. The reusable metadata now distinguishes register components,
derivation context and an explicit signal-phase type; the production consumer
uses storage identity and invalidates an `ITOS` fact when its integer/string
components or numeric context change. Two limitations are now the entry gate
for the next mechanism slice:

1. derivation availability is solved separately for each candidate generator,
   so it should become one worklist-driven multi-fact forward analysis before
   adding many conversions or paying avoidable assembler cost;
2. signal phase is currently proved only as `NONE` for the safe `ITOS` case
   and otherwise fails closed as unknown. Common pre-write, post-write and
   partial-write locations must be classified against actual VM handlers so
   normal, skip and handler continuations receive the right component
   generation.

The proposed fact is `(storage identity, component, value generation,
derivation, context generation)`. A component write creates a new generation;
derived string/number views name the source generation they represent. Direct
link/swap/unlink mapping, joins, loop entries/backedges and typed signal edges
then operate on the same fact instead of accumulating tactical exceptions.

The current equal-work RexxCPS diagnostic leaves 1,120,006 `ITOS`, 1,660,000
`STOD` and 2,220,000 `DTOS` executions. These are opportunity counts, not
authorization to remove them. PERF3-11 first measures assembler processing
time, attributes the two retained hot-loop `ITOS` sites and proves signal
locations for the decimal handlers. Only then should it compare remaining
`ITOS`, decimal/string round trips, compiler loop hoisting and consolidated
swap/swap cleanup as separate consumers. No public format, ABI or production
rewrite is selected by this queue entry.

Stage 1 closes the signal-contract entry gate. Adrian selected S1-S5 and
retirement of `RXOP_SEM_MAY_THROW` on 2026-08-02. Generic effect flags now
describe only call/alias/reference/indirect/opaque behavior and retain their
existing numeric values; the separate aligned signal inventory is authoritative
for capability, phase, source, dependencies and continuations. The selected
total `DCOPY`, non-signalling/plugin-consistent `DTOS`, portable pre-write
checked `INC`/`DEC`, and pre-write invalid literal `SETNUMFUZ` contracts pass a
permanent four-way runtime matrix. Focused correctness is 68/68, both ordinary
Release VMs build, the live decision ledger consumes 650 effect plus 650 signal
rows, and all three Gate 0 benchmark RXBIN hashes are unchanged. Evidence:
[`Stage 1 analysis`](evidence/2026-08-02-perf3-11-stage1-signal-contract/) and
[`Stage 1 lock`](evidence/2026-08-02-perf3-11-stage1-contract-lock/).

Stage 2 closes the immutable graph gate. The new consumer-free sidecar owns
stable queue-record, instruction, code-block and pre-emission address IDs for
one epoch, plus typed normal, branch, signal skip, handler, unwind,
terminal and unknown edges through synthetic roots/exits. Its label index and
edge construction are expected linear in records plus edges. The first cost
check exposed and rejected a duplicate opcode-resolution pass; the locked
orchestration reuses the final legacy `OpInfo` view, frees the legacy graph and
then constructs the sidecar. Focused correctness is 113/113, all three Gate 0
images are exact, and final same-session 30-round assembler medians are
+0.411% Richards, +0.463% Towers and -2.784% RexxCPS with no RSS escalation.
Evidence: [`Stage 2 flow graph`](evidence/2026-08-02-perf3-11-stage2-flow-graph/).

Stage 3 closes the reusable structural-analysis gate. The procedure epoch now
owns a demand-driven cached result containing unique predecessor sets,
multi-root RPO, dominators and sparse frontiers, SCC/backedge classification
and a loop hierarchy for source/control-flow candidates. K04d1 later removes
the now-retired synthetic signal-retry cycles. Work and retained-memory budgets
fail closed; deterministic dumps
make the scale auditable. The first eager integration was rejected after it
crossed the Richards RSS guard by 1,155,072 bytes. The accepted demand-driven
form retains identical analysis results under `-d` and future consumers while
ordinary consumer-free assembly stays guard-clean. Correctness passes 113/113
and all Gate 0 images remain exact.

Stage 4 closes the signal-policy/effect gate. Handler policy is an inherited
procedure parameter with sparse writes and edge-multiset phis; normal, skip,
handler and exit edges select policy versions using the authoritative
failure phase. Calls do not leak callee-local copy-on-write handler changes,
but do advance call/reference/external/plugin/locale identities because VM
argument slots point at caller-owned values. TRACE and numeric-context effects
remain independently versioned. Silent `sigpush` allocation failure prevents
an invented exact `sigpop` restoration. Strict GNU90 checks pass, focused
correctness is 113/113, all Gate 0 images are exact and the ordinary assembler
cost/RSS guard passes. Stage 5 now consumes these edge-state APIs.
Evidence: [`Stage 4 signal policy`](evidence/2026-08-02-perf3-11-stage4-signal-policy/).

Stage 5 closes the symbolic storage/component gate.  The third demand-driven
epoch cache uses sparse persistent mapping/value definitions and lazy phis;
local, argument and global registers are names for `StorageId`s rather than
the identity itself.  Link/swap/unlink and fused failure edges preserve exact
mapping state, caller arguments remain caller-owned, and reference/effect
versions prevent calls or indirect writes from becoming false unchanged-value
proofs.  Null/absent is distinct from unknown, copies retain presence, and
ITOS/FTOS/DTOS plus two-register ITOF name their actual source `ValueId` and
effect dependencies.

The initial recursive dynamic-storage query was rejected at 82.51 s and the
first eager component materialization was rejected at about 305 MB peak RSS.
The locked generation-marked, derivation-site-demanded form completes canonical
RexxCPS diagnostics in 0.28 s at 18.7 MB peak RSS.  Adrian explicitly accepted
a seconds-scale proof-analysis budget rather than requiring the roughly 50 ms
ordinary baseline.  Focused correctness is 113/113, all Gate 0 images are
exact, and ordinary RexxCPS assembly remains 54.526 ms (+0.009%).  Evidence:
[`Stage 5 sparse SSA`](evidence/2026-08-02-perf3-11-stage5-sparse-ssa/).

Stage 6 closes the first proof-authority gate.  The fourth per-epoch cache
provides dominated-success repetition, speculatability, loop must-execute and
component-invariance queries with cached diagnostic reasons.  Value/effect phi
reduction permits safe proofs through joins without source-order assumptions.
The private ITOS availability solver is deleted and the service is sole
authority.  Its 19-`ITOS` RexxCPS image removes two more operations than the
retained 21-`ITOS` old-solver image; Adrian accepted the resulting +7.469% and
+6.866% median CPS verdict on `rxvm`/`rxbvm`.

The first broad run exposed an unsound proof across a caller-owned range-call
argument.  Sparse SSA now gives explicit and range-call actual arguments
unknown component definitions on normal and failure paths, while preserving
unrelated locals.  Final focused correctness is 10/10, broad Debug is
1,987/1,987, and the accepted RexxCPS image hash is unchanged by the fix.
Diagnostic proof analysis completes in 0.39 s at 20.2 MB peak RSS.  Evidence:
[`Stage 6 proof service`](evidence/2026-08-02-perf3-11-stage6-proof-service/).

The migration principle is basic-to-advanced, not like-for-like parity.  The
old solver is a retained minimum safe-capability baseline.  Each remaining
legacy proof must be inventoried and replayed, then replaced one authority at a
time; a stronger new acceptance is valid only with its own positive proof,
adversarial correctness and output-changing Release gate.

The inventory is locked in
[`PERF3-11-MIGRATION-WORKLIST.md`](PERF3-11-MIGRATION-WORKLIST.md).  M01 is
complete: the old repeated-`ITOF` authority is deleted and the generic
one-register `XTOY` consumer selects all 20 conversions from canonical
metadata.  Twelve focused deletions prove: the old `ITOF` floor plus 11
stronger cases, including four `ITOD` flow shapes.  `ITOD` and `BTOD` now have
a coherent total non-signalling plugin/runtime contract; signalling families
and same-component `BTOI`/`ITOB` normalization remain closed.  Richards,
Towers and RexxCPS are byte-identical to Stage 6, focused replay passes 51/51,
broad Debug passes 1,989/1,989, and diagnostic RexxCPS assembly remains within
the seconds-scale budget. Evidence:
[`M01 XTOY migration`](evidence/2026-08-02-perf3-11-m01-xtoy/).

M02 is complete.  The old repeated integer/bitwise-float load availability
solver is deleted and the proof service now requires equal storage, equal
write-once scalar leaves and already-absent reference/native payloads.  The
focused image recovers the old floor and adds equal-phi, exact-float,
linked-storage and ordered-TRACE deletions; different phis, signed zero and
hidden cleanup remain closed.  Canonical images are byte-identical, ordinary
RexxCPS assembly retains a 0.05 s median, and the accepted procedure-local peak
is 30.1 MB.  Focused replay passes 53/53, Release hidden-cleanup execution is
4/4 and broad Debug passes 1,991/1,991.  Evidence:
[`M02 equivalent constants`](evidence/2026-08-03-perf3-11-m02-constant-write/).
M03 is complete in
[`M03 repeated absence`](evidence/2026-08-03-perf3-11-m03-absent-write/).
Its known-storage/all-component proof recovers the old repeated-`NULL` floor
and adds equal-phi, linked-storage and ordered-TRACE cases while preserving
cleanup.  Canonical images are unchanged and broad Debug passes 1,993/1,993.
M04 is complete in
[`M04 exact same-storage copy`](evidence/2026-08-03-perf3-11-m04-self-copy/).
Its conditional opcode metadata and StorageId proof recover the old raw-copy
floor and add decimal/attribute/binary, LINK, agreeing-phi and TRACE-safe
deletions. Divergent and different storage remain closed. Canonical images are
unchanged and broad Debug passes 1,995/1,995.

M05 is complete in
[`M05 sparse use/liveness`](evidence/2026-08-03-perf3-11-m05-sparse-use-liveness/).
One per-epoch use/dependency index replaces repeated dense candidate scans and
feeds immutable typed-copy rewrite plans. The ten old accepts are recovered;
the unrelated-ENDLIFE case is one stronger acceptance, while metadata, TRACE,
mixed-entry, cursor, caller-window and handler observations remain closed.
Canonical images are unchanged. Adrian accepted the ordinary RexxCPS assembly
boundary of 0.16-0.17 seconds and 102.8 MB peak RSS, and broad Debug passes
1,995/1,995.

M06 is complete in
[`M06 producer forwarding`](evidence/2026-08-03-perf3-11-m06-producer-forwarding/).
Its immutable proof follows exact storage/component identities and sparse uses,
recovers all eleven current accepts and deletes the old dense liveness solver.
Hidden producer cleanup is now explicit: every reference/native payload
cleared by the retargeted scalar producer must already be absent in both
storages. Frozen M05 and M06 focused/canonical images are byte-identical.
Adrian accepted the 0.18 s equal RexxCPS assembly median and +1.05% median RSS
verdict; focused Debug/Release pass 8/8 and broad Debug passes 1,995/1,995.
K04a compare/branch fusion owns atomic exact result-event deletion. K04b now
uses exact/dependent-`ValueId` call-window visibility and is accepted as a
neutral consolidation. K04c proves all five remaining canonical rejections are
unknown CALL retry-metadata false positives, not actual argument observations.
K04d0 found no production retry user and exposed an existing fused-call retry
mapping defect. K04d1 is implemented with propagated-call partial-state
metadata retained for non-retry failure paths, and K04d2 focused Debug/Release
passes 14/14. K04d3's revised first Release verdict is neutral at +0.021%
median RexxCPS on both VMs; mixed pair directions and one retained low `rxvm`
sample are retained. Adrian accepted the neutral semantic/infrastructure result
without an append, and K04d4 passes the complete Debug build plus 1,998/1,998
broad Debug tests. K04 is closed.

The PERF3-10 closeout also audited surviving tactical guards. Loads,
one-register XTOY repetition, same-storage copies and typed-copy redirection
now use the proof service; their superseded TRACE/barrier scans were deleted
with the corresponding authority. Producer forwarding now uses M06's atomic
SSA/use, hidden-cleanup and address-observation proof. The local duplicate-link
and swap/call-window rules still transform
code while storage identity currently only analyses their mappings. The
adjacent `cnop` rule is not a trace-anchor workaround. PERF3-11 replaces these
one consumer at a time with structural/runtime equivalence, rather than
deleting guards in a batch.

## PERF3-12 — current RexxCPS clause-lowering rereview

This compiler-facing evidence lane completed on 2026-08-04 without a product
edit. Fixed `200 x 100` schema-5 profiles use the exact accepted optimized and
no-opt images. No-opt is exactly 148,701,541 instructions under both VMs;
optimized is 54,221,210/54,221,182 under `rxvm`/`rxbvm`, with only 28
low-frequency final formatting/control instructions differing between the
otherwise matching optimized profiles. These are diagnostic counts; the
profiling-off PERF3-06 scorecard remains runtime authority.

The ranked mechanism evidence is:

1. `R12-P01` PARSE direct-destination transactions derive a maximum 7.28M
   PARSE-only `SCOPY` plus 1.96M grouped-null removals, a 9.24M-dispatch
   ceiling. The frozen PARSE opcodes first need exact conditional signal and
   failure-write metadata because source/result alias snapshot allocation can
   signal before current user-variable assignments.
2. `R12-S01/H01` maps 2.24M exact `"Key Bee." || lvar` constructions. Existing
   `STEMGET2`/`STEMSET2` selection can remove up to 2.24M concat dispatches;
   loop-scoped reuse can remove 1.96M. They require a comparative PoC rather
   than an assumed combination.
3. `R12-C01` observes exactly 2.22M `DCOPY`/`DTOS` pairs and 97.68 MB of
   decimal-copy traffic. Existing multi-component values and metadata permit
   an atomic RXAS plan that materializes the string component on the source,
   redirects string-only uses and deletes `DCOPY`; no two-register opcode,
   runtime flag or RXC semantic optimizer is required.
4. `R12-I01/R01` defers inlining and final register work until these consumers
   reduce the body and temporaries. Inactive `R12-T01` TRACE has only 100-200
   setup/teardown calls and no hot-loop procedure call; `R12-A01` emits no
   runtime work.

The opportunity ranking puts PARSE first, but the implementation queue starts
with `PERF3-12A / R12-C01` because it is a bounded consumer of already accepted
component SSA/use facts. `PERF3-12B` compares segmented stems with loop-scoped
reuse; `PERF3-12C` installs the PARSE contract and multi-result transaction;
`PERF3-12D` later revisits late/hybrid inlining and register finalization. Each
requires separate approval and its own mandatory first ordinary Release
verdict. T1 remains the trace principle: a reached event drains prior ordered
events, optimized trace output may differ, and users disable optimization for
source-accurate tracing.

PERF3-12A was approved on 2026-08-04 and is controlled by
[`PERF3-12A-WORKLIST.md`](PERF3-12A-WORKLIST.md). Adrian subsequently selected
a breaking cursorless string/binary RXAS prerequisite: explicit-position slice
operations replace public cursor state, old assembler/bytecode compatibility
is not required, and removal of the cursor fields from the VM value structure
is an intentional compile-time cross-check for missed instructions. The
cross-check found and removed all production dependencies plus one stale
current register diagram and obsolete generated operation assets. The
cursorless first verdict was accepted: fixed-work RexxCPS removes 5,601,469
no-opt instructions per VM plus 1,493/1,511 optimized instructions under
`rxvm`/`rxbvm`, and all former setter dispatches are zero. Adrian then accepted
X1 copied-XTOY placement. It removes exactly 820,000 optimized `DCOPY`
dispatches and 36,080,000 copied bytes while retaining all 2,220,000 `DTOS`, a
further 1.528235%/1.528149% optimized instruction reduction. Combined closeout
adds exact negative and injected-allocation slice coverage and rejects
component placement when `MKREF` exposes either storage without a second
register mapping. Broad Debug has 2,034/2,034 functional outcomes: 2,033 pass
in the parallel-30 run and its only host-load timeout passes isolated in 12.11
seconds. Old build/worktree RXBIN files must be deleted or rebuilt because
compatibility was deliberately broken. Wall-clock claims remain deferred until
the remote terminal is absent. Evidence:
[`cursorless first verdict`](evidence/2026-08-04-perf3-12a-cursorless-first-release-verdict/)
and
[`X1 first verdict`](evidence/2026-08-04-perf3-12a-x1-first-release-verdict/).

PERF3-12B was approved on 2026-08-04 and is controlled by
[`PERF3-12B-WORKLIST.md`](PERF3-12B-WORKLIST.md). It first audits the exact
one-/two-segment native-stem signal contracts and the five current generated
sites, then compares two separately replayable PoCs: existing segmented
`STEMGET2`/`STEMSET2` selection and capability-lazy loop-scoped joined-key
reuse. The stable left-segment register, individual UTF-8 validation, joined
tail TRACE event, storage/effect invariance, `.locals` growth and assembler
scale are explicit costs rather than assumed details. The routes retain their
overlapping 2.24M/1.96M concat ceilings and cannot be combined before an
ordinary profiling-off Release panel is reported for selection.

The 2026-08-04 B1 audit verifies all 185 retained PERF3-12/X1 checksums and
closes the four native stem access contracts to pre-write
`UNICODE_ERROR|FAILURE`; `INVALID_ARGUMENTS` is unreachable for get/set parts.
Injected initialization and segmented GET allocation failures preserve the
logical stem and destination. Its first profiling-off Release verdict is
mechanism-neutral but not byte-identical: exact signal edges move one accepted
X1 `DCOPY` deletion from RexxCPS source line 163 to the equally weighted line
164, leaving the code-segment size, two static `DCOPY`, four static `DTOS` and
fixed-work hot dispatches unchanged. The optimized image grows 48 bytes from
two retained TRACE records; no-opt remains byte-identical and all four
fixed-work dual-VM cells pass. Evidence:
[`B1 contract audit`](evidence/2026-08-04-perf3-12b-b1-contract-audit/).
Adrian accepted the neutral verdict on 2026-08-04. B2 then completed the
isolated S1 segmented route at commit `888fa94eb`: four of five generated
sites become three `STEMGET2` plus one `STEMSET2`, removing 1,960,000 CONCAT
dispatches at a 280,000 stable-left LOAD cost. The fifth site is correctly
rejected because a later failure-atomic get's signal-skip continuation can
expose the old joined register to user-visible TRACE. Exact string-component
metadata also unlocks one independent X01 placement; that F1 foundation is
factored out of S1 and must be common to the later comparison. The focused
proof/native-stem panel and six dual-VM smoke cells pass, while matched
assembler time/RSS and first-epoch SSA size are neutral. Evidence:
[`B2 S1 PoC`](evidence/2026-08-04-perf3-12b-b2-s1-poc/). B3 then completes the
separately replayable H1 PoC at `80c78fcee`: the first conditional concat stays
as a lazy cache seed and all four later target uses reuse it, removing exactly
1,960,000 dispatches with no setup instruction and deriving 50,879,051 total
fixed work (-3.709378%). All preheader candidates fail the required
must-execute, loop-invariance and ordered-TRACE gates, while adversarial
loop/storage/reference/call/signal tests, 16/16 native-stem checks and six
dual-VM smoke cells pass. First-epoch SSA bytes remain 83,902,504; elapsed
assembly on battery is retained only as non-authoritative raw evidence.
Evidence:
[`B3 H1 PoC`](evidence/2026-08-04-perf3-12b-b3-h1-poc/).

B4 is complete on AC with S1 and H1 still unlayered. The governed timing panel
reaches 36 pairs: H1 is clear favorable at +3.075212%/+4.274944% paired median
CPS on `rxvm`/`rxbvm`, while S1 is +0.673386% noisy/inconclusive on `rxvm` and
+0.523554% clear favorable on `rxbvm`. Exact counts confirm the shared 1.96M
CONCAT removal, S1's additional ~0.28M LOAD cost and H1's absence of hot setup;
allocation, RSS and assembler scale remain neutral. H1 is recommended, but the
programme is stopped for Adrian to select S0, S1 or H1 before B5 production
reimplementation. Evidence:
[`B4 comparative panel`](evidence/2026-08-04-perf3-12b-b4-comparative-panel/).

Adrian selected H1 on 2026-08-05. B5 reimplemented the proof and consumer in
production rather than importing the PoC decision wholesale: immutable
ValueIds, exact signal/use plans and one transactionally provisioned private
local authorize four later joined-key reuses. Canonical `main` emits
`380 -> 365` with `.locals=104`; focused Debug/Release, native-stem, dual-VM and
zero-candidate gates pass. The first ordinary Release verdict is independently
clear favorable on `rxvm` at +2.557920% paired median; its small `rxbvm` panel
is +3.169497% but interval-inconclusive after one retained low observation.
Adrian accepted the verdict; the B4 36-pair result remains the both-VM causal
authority. Evidence:
[`B5 first verdict`](evidence/2026-08-05-perf3-12b-b5-first-release-verdict/).

B6 merges the accepted work with current `origin/develop` at clean product
`44d8b6a7e` and retains a fresh formal Mac scorecard. All 348 executions pass
without a noise append. Common-five means remain above the 2.00x ooRexx target
at `2.375939x/2.376230x`; RexxCPS is 47.203/47.093 MCPS and the static image is
exactly the selected H1 count of 1,210 instructions. Independent-session
movement from K04e is descriptive, while B4/B5 retain causal ownership. S1 and
H1 replay material remains in checksum-closed evidence after disposable PoC
worktrees are removed. PERF3-13 Gate A completed the RXVM allocator/value-shape
audit and Gate B's unchanged-value worker/slab baseline was accepted on
2026-08-06 after the corrected formal Release and bounded RSS verdicts.
Gate C completed with accepted 176-byte L32SDH on 2026-08-07. Gate D's first
ordinary-Release verdict is accepted and its Mac local closeout is complete;
Intel Linux, Linux ARM64 and same-machine Windows rebuild-together validation
remain before global closure. PERF3-12C transactional PARSE
remains separate and queued. Evidence:
[`current Mac scorecard`](evidence/2026-08-05-perf3-12b-mac-scorecard/);
[`PERF3-13 Gate B closeout`](evidence/2026-08-06-perf3-13-gate-b-closeout/);
[`PERF3-13 Gate D local closeout`](evidence/2026-08-07-perf3-13-gate-d-local-closeout/).

## Authoritative references

- standing instructions and governance:
  [`AGENTS.md`](AGENTS.md) and
  [`PERFORMANCE-GOVERNANCE.md`](PERFORMANCE-GOVERNANCE.md)
- closed PERF2 register:
  [`ROADMAP-PERF2-2026-07-31.md`](ROADMAP-PERF2-2026-07-31.md)
- closed initial register:
  [`ROADMAP-INITIAL-SWEEP-2026-07-23.md`](ROADMAP-INITIAL-SWEEP-2026-07-23.md)
- retained Mac scorecard:
  [`2026-07-27-perf2-09-mac-closure`](evidence/2026-07-27-perf2-09-mac-closure/)
- retained Linux x86-64 attribution:
  [`2026-07-28-perf2-10-11-intel-linux`](evidence/2026-07-28-perf2-10-11-intel-linux/)
- retained Windows baseline and controls:
  [`2026-07-29-perf2-11-windows-x86-64`](evidence/2026-07-29-perf2-11-windows-x86-64/),
  [`2026-07-30 compiler comparison`](evidence/2026-07-30-perf2-11-windows-compiler-comparison/) and
  [`2026-07-30 MSVC rxbvm`](evidence/2026-07-30-perf2-11-windows-msvc-rxbvm/)
- extracted copy/conversion/accessor evidence:
  [`CRI-13 RXAS trace`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-R1-RXAS-TRACE.md),
  [`bounded conversion decision`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-BOUNDED-NUMERIC-CONVERSION-DECISION.md) and
  [`class-access verdict`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-C-CLASS-RELEASE-VERDICT.md)
