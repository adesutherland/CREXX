# STEP-06 native inference qualification

Status: local qualification complete; remaining platform/acceptance work open,
15 September 2026, following Adrian's approval of the
completed STEP-07 documentation/examples. Authority: [parent OUT-01–05,
AC-01–14 and S6-01–08](../../planning/native-inference-backlog.md); coverage:
[approved STEP-07 handoff](../native-inference-step07/README.md).

Baseline: uncommitted implementation on `develop`, HEAD
`f9f87a8e8671a5e7894fbc187dbc93ee7bcf8466`. Preserve the STEP-05/07 evidence and
unrelated changes. Local hardware: Apple M5, Darwin 25.6.0 arm64, 10 logical
CPUs, 24 GiB RAM; CPU and Metal available. Linux/Windows connection details and
CUDA/Vulkan device availability have been requested from Adrian. Host absence
is not an acceptance waiver.

## Qualification ledger

| Area | Current state / next action |
| --- | --- |
| Ordinary Debug full regression | STEP-05 retains 2,347 passing tests. Compare inputs and reuse valid coverage; run affected focused checks for QA changes. |
| Accepted Release comparisons | STEP-04/05 verdicts retained. No replay or model/upstream tuning. |
| SAN-009 / sized output host / native objects | Matching focused Debug and Apple-ASan probe-cycle/counted-string controls pass 2/2. Local native-object, worker, package and broad ASan gates pass. Supported Linux leak/platform gates remain required; SAN-009 stays open. |
| Instrumented generation qualifier | S2-QA01 applied. Sustained normal and Apple-ASan CPU/Metal pass. RSS remains diagnostic under instrumentation; the 32 MiB retention and 4 GiB co-resident limits are unchanged. |
| New nested aggregates | Keep explicit/unregistered until normal and sanitizer isolation measurements establish scheduling/timeouts. |
| Complete local Apple-ASan | Full build/preparation pass; 2,349/2,349 CTests pass in 2,278.82 s through `tools/asan-run.sh`, `20260915-150827-full`. No sanitizer diagnostics in retained logs. Apple LSan unsupported; no supported leak check disabled. |
| Installed/native/offline/failure workflows | Both STEP-07 native examples pass under enforced network denial with live listener controls. Apple-ASan typed embedding and generation installed/native matrices pass, as do package dependency/fallback/cache/worker failure controls. Target OS/device proof remains open. |
| Linux / Windows / CUDA / Vulkan | Hosted package qualification is active on isolated candidate branches. Linux CPU/Vulkan and macOS arm64 CPU/Metal fixture packages pass at `2bc56249d`; Intel Mac also passes its isolated replay at `e39916f2a`, with the original stall still unexplained. Windows MinGW CPU/Vulkan package and restricted-PATH native consumers pass at `4a0924ee1`; Linux CUDA package/CPU fallback also passes at `2bc56249d`. Windows CUDA and combined exact-head wider gates remain pending; GCC sanitizer QA has a header-packaging repair under retry. Real BGE/Smol device cells remain open. See the [pipeline ledger](../native-inference-ci/README.md). |
| Model provenance | Exact GGUFs verified, including fresh documented Smol download. [S6-D01](../../planning/native-inference-model-provenance-proposal.md) reproducibility/compatibility QA is complete. Adrian directed a QA focus without overcomplication; approved pins/downloads unchanged, no adoption/distribution work. Historical original-artifact ancestry remains unproven. |
| Final acceptance | Parent AC-01–14 remain open. No sanitizer-clean or release-ready claim. |

All build/test output is retained in runner logs or bounded scratch logs and
indexed here. Initial scratch workspace:
`/tmp/crexx-ni-s6.zq0rk0ug`. No publication or user-prefix install has occurred.

## Final broad local gate

The maintained full run completed successfully on 15 September at 16:00 BST:
full build, explicit QA preparation and **2,349/2,349 CTests**. The CTest stage
took 2,278.82 seconds; the build/preparation/test invocation took approximately
52 minutes. [Summary and exact command](full-local/summary.json),
[CTest log](full-local/ctest.log), detailed `full-local/LastTest.log`, build and
preparation logs, runner options and selected build configuration are retained.
A scan of all four logs found no address/leak/undefined-behavior sanitizer
diagnostic. This establishes the macOS Apple-ASan result; it is not a supported
LeakSanitizer or other-platform pass.

The current input comparison remains exact: 30 existing QA/example/build inputs
changed from the initial STEP-06 snapshot, plus two new explicit qualification
dispatch files. None of the 7,415 non-document input hashes changed during the
final gate. `inputs-before.json.gz`, `inputs-after.json.gz` and
`qa-input-diff.json` distinguish this work from the accepted implementation.
The unchanged 2,347-test ordinary Debug result and accepted Release panels
remain valid. No further local broad replay is required without a relevant
input change or a newly reproduced concern.

S6-01–04 and S6-07 are complete. S6-05 remains open for the outstanding target
delivery/failure scenarios, S6-06 for actual Linux/Windows/real CUDA/Vulkan and
supported Linux ASan/LSan proof, and S6-08 for final whole-plan reconciliation.
Codex owns those gates under Adrian's direction. GitHub host/package testing is
active; remaining real-device access and full supported-platform proof are
pending. The [current parent disposition](../../planning/native-inference-backlog.md#step-06-parent-acceptance-disposition--15-september-2026)
preserves each AC-01–14 and its unmet condition. SAN-009 remains release-blocking.

## Initial preparation

- Normal Debug `rxpa_host_text_services` and `rxllama_backend_probe_cycle` pass
  2/2 after explicit prerequisite builds; runner record
  `cmake-build-debug/asan-logs/20260915-125105-ctest`.
- The generation qualifier now applies the already approved S2-QA01 distinction:
  the same 32 MiB retention limit uses process RSS normally and sanitizer live
  allocated bytes under ASan. RSS, the selected metric and all samples remain
  visible. The 100 singles / 20 batches / 1/2/4 owners and normal co-resident
  4 GiB guard are unchanged; sanitizer timing is diagnostic. Normal build passes.
- Initial ASan targeted build could not find the newly added host-service target
  in the older Makefiles. No source compiled and no sanitizer diagnostic occurred.
  `rebuild_cache` through `tools/asan-run.sh` regenerated the same configured
  tree successfully; the exact targeted build then passed. Retain both
  runner logs; this is stale build preparation, not a product sanitizer finding.

## Focused results and QA adaptations

| Isolated control | Normal Debug | Apple ASan |
| --- | --- | --- |
| `rxllama_backend_probe_cycle` and `rxpa_host_text_services` | 2/2, 0.74 s; `20260915-125105-ctest` | 2/2, 1.80 s; `20260915-125357-ctest` |
| Sustained generation CPU | 124.283 s; `20260915-125508-build` | 176.338 s; `20260915-130452-build` |
| Sustained generation required GPU/Metal | 52.196 s; `20260915-125900-build` | 69.462 s; `20260915-130921-build` |
| Typed embeddings, opt/noopt and both VMs, CPU/Metal | 28 executions, 132.412 s; `20260915-131244-build` | 28 executions, 205.531 s after prerequisite build; `20260915-132237-build` |
| Low-level embedding numeric/boundary controls, CPU/Metal/cross-device | All three pass; retained queue records | All three pass; retained queue records |
| Original complete-text boundary and sustained legacy embeddings | 12 VM executions, 35.722 s; `20260915-133516-build` | 12 VM executions, 69.780 s; `20260915-134123-build` |
| Minimum public generation matrix | Unchanged STEP-05 normal evidence reused | 16 VM executions, 247.351 s; `20260915-134235-build` |
| Generation lifecycle, persistent and shared-worker closeout matrix | Unchanged STEP-05 normal evidence reused | 24 VM executions, 920.165 s; `20260915-134645-build` |
| Installed/relocated typed embeddings, including persistent/shared workers | Unchanged STEP-04 normal evidence reused | 28 installed VM + 14 relocated native executions, 534.181 s after prerequisite build; `20260915-140207-build` |
| Installed/relocated generation, including persistent/shared workers | STEP-05 normal coverage reused; changed shared-worker scope additionally passes both Debug VMs | 32 installed VM + 16 relocated native executions: 18 retained passes plus 30 successful continuation executions after S6-QA02; continuation 1,360.051 s |
| Package metadata, cache, fallback and native/dynamic worker lifecycle | STEP-03 expanded normal matrix reused | Complete expanded matrix, 385.991 s; `20260915-150153-build` |

Times describe isolated QA scheduling costs, not a product-performance verdict.
The broad Apple-ASan selection contains 2,349 tests: the same 2,347 names from
the retained normal run plus the two real SQLite-ODBC VM tests, enabled by this
tree's `ENABLE_ODBC=ON` and available driver. The normal tree has ODBC disabled;
no prior test name disappeared. See `test-selection-comparison.json`.
Generation controls retain 100 singles, 20 four-row batches, 1/2/4 simultaneous
VM owners each doing 20 batches, identity/output isolation, cancellation,
boundaries and co-resident BGE/Smol checks. ASan CPU live retention grew by
1,456 bytes; co-resident peak RSS was 3,314,335,744 bytes, below 4 GiB.
ASan Metal's live retention did not grow and its co-resident peak was
2,593,177,600 bytes. Its larger diagnostic RSS growth did not correspond to
retained live allocations, as anticipated by S2-QA01.

The explicit `rxllama_generation_qualify_*` and `rxllama_qualify_*` targets
dispatch correctness checks through the maintained runner, keeping their fresh
per-suite logs and install directories. They are not yet registered in CTest.
`CREXX_LLAMA_QUALIFICATION_MODES=cpu,required-gpu` selects both actual Mac paths;
the portable default is CPU only and does not imply GPU qualification.

The embedding installed-consumer dispatcher now resolves `.exe` names on
Windows like the generation dispatcher. Workloads/assertions are unchanged;
local execution and actual Windows qualification are still required for this
QA adaptation. Both package dispatcher docstrings now permit their STEP-06
instrumented use through the runner.
The legacy and installed dispatchers now also require the portable VM to exist;
a missing executable cannot produce a vacuous passing VM matrix. The existing
normal and sanitizer trees and retained installed product satisfy this guard.

Current hosted core sanitizer/deep workflows do not enable the optional llama
provider or provision models. Their normal platform gates remain necessary but
cannot substitute for explicit native-inference platform evidence.
The repository runner API currently reports zero registered self-hosted runners;
this is recorded in [the inventory](github-runner-inventory.json), not evidence
that Adrian's machines do not exist. Direct host connection details are pending.

## Enforced offline delivery

The unchanged scratch-installed STEP-07 native `embeddings` and `generation`
examples both pass in `auto` mode under macOS `sandbox-exec` with
`(version 1)(allow default)(deny network*)`. Each completes its persistent
20-batch workload. A local HTTP listener is reachable without the sandbox both
before and after the denied child; the sandboxed network request fails. This
distinguishes an effective network restriction from an unavailable network.
No system network/firewall setting changed. Provider/library override variables
are removed for the example children.

[Commands/results](offline/results.json), [binary/model identities](offline/identities.json)
and raw per-child logs are retained under `offline/`; the reproducible
[driver](offline-check.py) uses the STEP-07 installed binaries. This closes the
local enforced-offline gap for these actual programs, not all OS/package cells.

The [current input comparison](qa-input-diff.json) records QA dispatch/measurement
files, test/example hang guards and CMake test inclusion/scheduling properties.
Product inference, RXPA, compiler/runtime logic and approved model pins are
unchanged from the STEP-06 baseline. Reuse the full 2,347-test normal result and
matching STEP-04/05 normal consumer coverage; only changed QA paths need a fresh
normal control before the corresponding sanitizer execution. No accepted timing
panel is replayed.

## S6-QA01 — Complete the old-host fixture's existing callback table

The new explicit `rxllama_qualify_old_host` entry point reproduced a normal
Debug null callback crash in `text_old_host.c` during plugin initialization.
The [backtrace](old-host-fixture/old-host-debug-backtrace.log) reaches
`_initfuncs` at `rxllama.c:161`; the fixture supplied procedure registration but
left class/interface/implementation/member registration callbacks null. Those
callbacks already existed in the legacy initializer at baseline `c2cf28a4f`.
The typed provider now uses them during declaration registration, before an
unsupported legacy call is rejected. This incomplete mock did not represent
that host contract.

The fixture now supplies the four metadata callbacks and asserts declaration
registration occurred. Every existing rejection assertion is retained: legacy
factory, absent/null text-service tables, bounded `-7` result, cleared signal,
and no published handle. The exact explicit target passes in normal Debug and
Apple ASan after this fixture-only correction. No product code was changed and
no sanitizer diagnostic was observed for this finding. The original normal
failure is retained; this is not a sanitizer suppression or an accepted crash.
The first passing normal queue run shared a second-resolution runner directory
with the next control. Its log was replaced, so the tiny unchanged old-host
target was rerun alone at `20260915-135735-build` to retain an unambiguous pass.
The replacement log and retention note are under
`runner-results/debug-rxllama_qualify_old_host/`; the queue JSON still preserves
the original successful invocation.

## S6-D01 candidate conversion

Adrian approved producing reproducible candidates. Both models now reproduce
byte-for-byte and pass the five existing CPU/Metal compatibility controls in a
separate scratch bridge with only their two hash substitutions. Full input,
environment, artifact and compatibility records are in the
[candidate evidence](model-conversion/README.md). Adrian then directed a QA focus
without overcomplication. Current approved pins/downloads remain, and the
proposed adoption/distribution work is not pursued.
The original BGE/Smol model files have not been overwritten. No publication took
place. This progress does not close current full-product AC-10 by itself.

## S6-QA02 — Functional deadlines are hang backstops

The installed ASan `opt-shared_generation-installed-rxtvm-cpu` case failed with
`PANIC: deadline exceeded status=6 (SIGNAL TASK_FAILURE)`. An unchanged isolated
replay reproduced it after 137.166 seconds. There is no ASan memory diagnostic.
The example imposed a 120-second scope for all four workers' twenty batches;
the VM/provider/library hashes match the earlier successful build-tree run.
The prior raw-tree sharing cases took about 90–100 seconds; installed cases
took 133–140 seconds. Host load was also elevated. These observations identify
the deadline that failed, not a demonstrated inference or concurrency defect.
Original/replay records are in `shared-generation-deadline/`.

Adrian explicitly requested removal or wide backstops for hard deadlines because
they undermine GitHub repeatability. The intended outcome is identical functional
coverage on slower or busy hosts, with runaway jobs still bounded. No workload,
output, sharing, memory or cancellation assertion is removed. Accepted ordinary
Release performance criteria remain separate.

1. [x] **D-AC-01:** sustained shared-worker scopes have no whole-workload timer;
   operation/barrier waits use a wide ten-minute hang backstop.
2. [x] **D-AC-02:** model-bearing QA subprocesses have a thirty-minute outer
   backstop; serial aggregate scheduling and diagnostic logs remain.
3. [x] **D-AC-03:** the reproduced example passes in normal Debug and Apple ASan,
   then the interrupted package matrix resumes with completed evidence reused.

1. [x] **D-01:** retain original and unchanged replay failures and artifact checks.
2. [x] **D-02:** adjust inference functional examples/controls and their harness
   limits; document the distinction from performance acceptance.
3. [x] **D-03:** validate the changed shared-generation path in normal Debug,
   then ASan; continue pending package cases without repeating completed cases.
4. [x] **D-04:** finish the full local gate and update the parent acceptance ledger.

The four example files retain every source documentation tag. Their finite
workloads, output/identity checks, cleanup and memory assertions are unchanged.
`AGENTS.md` now records Adrian's wide-backstop/serialization direction. Registered
real-model tests retain `RUN_SERIAL`; the explicit qualification queue executes
one case at a time. Normal Debug passes the changed shared-generation example
in both VMs (106.158 seconds for compilation/linking and the two executions).
Matching Apple ASan also passes both VMs (161.468 seconds for the same shape).
The interrupted installed/native matrix resumed at the shared-generation
case and passed its remaining cases. The observed failure is dispositioned as the example/QA deadline defect;
no inference, model or runtime production change was needed.

The continuation now passes all thirty remaining installed/native executions,
including the formerly failing installed threaded VM, CPU/Metal and both
optimization modes. Combined with the eighteen retained prefix passes this
completes all forty-eight generation package executions. See
`package-generation-continuation/combined-coverage.json`. The smaller direct
Debug/ASan repair controls are additional evidence, not counted in that total.

Earlier successful cases met the previous stricter hang limits and remain useful
evidence for unchanged functional work. The package dispatcher can continue at
an explicit whole-case label in a fresh directory; it does not silently skip
failures or claim the omitted prefix passed. STEP-06 combines the retained
three completed optimized generation cases with the continued remaining cases.
Accepted Release timing payloads and measured thresholds are unchanged; their
unused hang guards also widen under Adrian's direction, without another panel.
