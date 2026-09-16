# Native inference release pipeline and branch qualification

Status: implementation authorized by Adrian, 15 September 2026. This is the
STEP-06 pipeline work package under [the authoritative plan](native-inference-backlog.md),
preserving OUT-01–05 and AC-01–14. It does not replace real-device qualification
with a fixture pass. Main qualification branch: `origin/temp/llama-release-combined`; focused
retries use the named candidate branches in the live handoff.

## Vision and outcomes

1. **CI-OUT-01:** a release recipient can use `import llama` from the supplied
   cREXX binaries after provisioning a supported model. The release includes
   the provider, pinned engine, CPU/backend libraries, redistributable runtime
   dependencies, manifests, notices, guides and examples. No recipient C/C++
   build, separate llama.cpp installation or inference server is required.
2. **CI-OUT-02:** routine builds perform small, useful checks of the actual
   payload. A generated random-weight GGUF exercises a few engine computations;
   the public provider is checked for load, discovery, native factories, model
   rejection and cleanup. Public profile/hash checks are unchanged. Retained
   real BGE/Smol tests cover their successful full provider inference paths.
3. **CI-OUT-03:** the remote candidate can be rebuilt, inspected and cancelled
   independently of `develop`. Artifacts/logs identify the exact commit. Wider
   core regression and supported sanitizer workflows qualify that candidate;
   promotion is considered only after the required workflows are green.

## Numbered acceptance criteria

1. [x] **CI-AC-01:** the candidate contains the accepted implementation and
   current remote RXPP changes, with both parents retained. Neither remote
   `develop` nor a GitHub release is modified during qualification.
2. [ ] **CI-AC-02:** four independently usable llama-free core archives pass on
   the target Linux/GCC, Windows/MSVC, ARM Mac/Clang and Intel Mac/Clang
   configurations before llama is added. A fifth Windows/MinGW core-only gate
   verifies both VM variants and retains QA evidence without publishing binaries. Separate matching plugin archives
   supply CPU/Vulkan on Linux/Windows, CPU/Metal on ARM Mac and CPU on Intel Mac,
   with documentation,
   examples and notices. Verify actual extracted core and core-plus-plugin ZIPs.
3. [ ] **CI-AC-03:** separate Linux and Windows CUDA plugin archives include
   CPU fallback and redistributable dependencies. One Windows MSVC core with
   `rxvm` selecting `rxbvm` works with either Vulkan or CUDA, including native
   generation and relocated execution. MinGW source/regression support remains;
   a separate MinGW binary delivery is no longer required (approved CI-D01).
   The later approved Windows/MinGW core gate remains mandatory.
   GPU build/package success does not assert real-device execution.
4. [x] **CI-AC-04:** smoke uses a generated fixture smaller than 5 MiB, with
   generator/source/seed/toolchain/hash provenance, no large model download and
   no test bypass in the public model loader. Checks cover bounded generation
   and finite embedding extraction in the packaged engine, plus public provider
   behavior through installed VM and relocated native consumers.
5. [ ] **CI-AC-05:** smoke fails for a missing/altered required package file,
   verifies fixture identity and required backends, and runs outside the build
   tree using the staged runtime. Test helpers and fixture weights are not
   accidentally shipped as user runtime dependencies.
6. [x] **CI-AC-06:** branch-scoped cancellation and rerun controls work; failures
   preserve logs. Deadline-sensitive work is serial within each host, using
   wide hang guards. New smoke is measured alone in normal Debug and maintained
   ASan before CTest registration. No model-quality/performance workload enters
   the routine candidate pipeline.
7. [ ] **CI-AC-07:** the candidate release builds/smokes and wider core Deep
   Build/Sanitizer QA gates have terminal results for the exact candidate SHA.
   Linux ASan/LSan keeps leak detection enabled for first-party cREXX code.
   Core sanitizer jobs use `ENABLE_LLAMA=OFF` and build no CUDA backend/SDK.
   Separate first-party adapter checks may link an ordinary uninstrumented
   engine; ARM Mac is the practical local integration host. Do not instrument
   or qualify upstream llama.cpp/CUDA under this sanitizer work package.
   Retained unchanged local tests
   are reused; new RXPP inputs are covered by hosted qualification.
8. [ ] **CI-AC-08:** human/agent guides, parent acceptance status and artifact
   evidence describe exactly what is shipped and tested. Remaining real-GPU,
   model/provenance and release gates remain visible; green fixture checks do
   not silently close them.

9. [ ] **CI-AC-09:** CUDA jobs reuse unchanged compiler outputs without weakening
   source/toolchain/flag identity or the final package smoke. Retain cold and
   warm cache statistics on Linux/NVCC and Windows/MSVC/NVCC. A cache miss or
   eviction may require a cold build; it must never select a different engine,
   remove GPU architectures or require recipients to install a CUDA SDK.

## Numbered implementation steps

1. [x] **CI-01 — Freeze the candidate:** checkpoint accepted STEP-05/06/07 work
   (`12647a91aa7e91478b23961a68f7ff9fb492e1e1`), then merge the two remote RXPP
   commits through `b5b827489` (`c1de1670812a33a8121ab7f28b99b4e50ca5b208`).
   Serves CI-AC-01/07; local `develop` remains at its prior head.
2. [x] **CI-02 — Implement bounded smoke:** reuse pinned upstream generated
   fixture evidence, add package/engine and public-provider controls, measure
   the exact aggregate in normal/ASan isolation, then set scheduling properties.
   Serves CI-AC-04–06; depends on CI-01.
3. [ ] **CI-03 — Build the complete delivery:** enable provider/package targets,
   add CPU/GPU build prerequisites and complete CUDA variants, stage the same
   payload for branch artifacts and eventual release, and smoke it before upload.
   Serves CI-AC-02/03/05; depends on CI-02. Preserve signing/installer safeguards.
4. [ ] **CI-04 — Run remote qualification:** push only the candidate branch;
   exercise selected fast lanes first, repair failures with focused controls,
   then run required wider workflows on the resulting exact SHA. Retain logs
   and cancel superseded candidate runs. Serves CI-AC-01/06/07.
5. [ ] **CI-05 — Reconcile and report:** record terminal run/artifact identities,
   update this checklist and parent criteria, and make the green candidate
   reviewable before promotion. Serves CI-AC-08; depends on CI-01–04. Creating
   the candidate is not permission to publish an unqualified snapshot.

## Design boundaries

- **CI-D02, approved 16 September 2026:** ship the Intel Mac plugin CPU-only.
  Intel Metal is unsupported for this delivery; omit its backend at build time.
  Many Intel Macs support Metal, but the pinned engine's cold initialization
  stalls inside the hosted Metal compiler service (CI-F07), with no verified
  upstream remedy for that mechanism. This is a delivery scope amendment,
  not a claim that Metal cannot exist on Intel or that the engine defect was
  repaired. ARM Metal and Windows/Linux Vulkan/CUDA remain required. This
  amends CI-AC-02's Intel delivery and its application of parent OUT-02,
  AC-02/03/07; all other parent
  outcomes and criteria stay in force.

- The pinned upstream `test-llama-archs.cpp` generates a 4,763,872-byte dense
  llama fixture already exercised in STEP-02. BERT fixtures are not supported
  at this pin. Random-token engine embeddings are smoke plumbing, not BGE
  pooling/tokenizer or retrieval-quality qualification.
- Keep the actual public provider unchanged: its successful real-model paths
  remain covered by the retained local model campaign and named device gates.
  A fixture is deliberately rejected as an unqualified BGE/Smol artifact.
- Build and package GPU backends without requiring a GPU on every hosted
  runner. CPU fallback must execute on those hosts. Actual GPU computation is
  tested only when a real device is available and is reported separately.
- CI CUDA tooling may be larger than the smoke data; use pinned build inputs
  and caching. No real embedding or generation model is downloaded by default.
- `crexx --native` itself still needs a C toolchain when an application author
  creates a native executable. A release recipient running ordinary cREXX
  programs, or a recipient of a prepared native application, does not need one.
- Later idea from Adrian (15 September): a separate Cognitive example package
  could combine useful programs, model-download guidance and diagnostics. The
  random-weight fixture is developer QA data, has no useful model output, and
  is not included in the current user release. This idea does not expand the
  present delivery scope.

References: [pinned fixture generator](https://github.com/ggml-org/llama.cpp/blob/5266f24da75dc449bd56cbed7addb9c8e4a6a73e/tests/test-llama-archs.cpp),
[GitHub branch concurrency](https://docs.github.com/en/actions/how-tos/write-workflows/choose-when-workflows-run/control-workflow-concurrency).

## Local checkpoint before the first remote run

The aggregate passed isolated Debug/Apple-ASan measurement (44.5/48.3 seconds),
then its registered serial CTest passed (28.20/45.57 seconds). Workflow validation
and 14 publication/signing/matrix controls pass.
[Retained evidence](../qa/native-inference-ci/README.md) records scope and inputs.
CI-03/04 remain open until the candidate packages and wider hosted jobs finish.

## Windows portability follow-up within CI-03/04

The main candidate's useful Linux/CUDA and Mac jobs continue while diagnostic
branches `temp/llama-release-windows` (MSVC/CUDA) and
`temp/llama-release-mingw` test ordinary portability repairs. These branches
cannot publish; the final combined candidate still needs CI-AC-07.

The Windows loader audit identifies an unmet existing CI-OUT-01/CI-AC-03/05
requirement: executable/plugin startup and backend dependencies must resolve
without a development SDK on PATH. Preserve the API and provider package:

1. Copy only the bridge/engine core DLLs and their runtime dependencies beside
   installed executables; keep the complete native/backend package in
   `bin/providers`. Do not duplicate the large CUDA libraries in `bin`.
2. Locate that trusted provider directory from the installed bootstrap copy;
   native applications retain their existing adjacent manifest/dependencies.
   Load verified Windows backends with dependency lookup scoped to their DLL
   directory, retaining the existing process-lifetime ownership.
3. Prove the final Windows package and relocated native application with the
   development SDK/toolchain PATH removed during execution. The native-build
   action retains its necessary compiler environment. Retain normal/ASan
   focused controls and target Windows results before closing this repair.

This is a source-audit finding until Windows execution reaches this boundary;
the earlier compiler failures do not themselves prove a loader failure.

CI-F07 subsequently identifies an Intel Mac engine-helper hang at `2bc56249d`.
The separate branch `temp/llama-release-intel-diagnostic` at `e39916f2a` runs
manual lane `macos-intel` in [35002901578](https://github.com/adesutherland/CREXX/actions/runs/35002901578).
Its only changes from shared diagnostic-support commit `dd6e3155d` are the
explicit 120-second stack-capture request and its branch-only note. Do not merge
that workflow override into the candidate. A diagnostic stop fails; the normal
qualification backstop and workload remain unchanged. Cause and platform closure
remain open in [the pipeline ledger](../qa/native-inference-ci/README.md).

CI-F09 reopens CI-AC-06: a delayed push event superseded a manual Windows-only
selection and launched the full matrix. Automatic package pushes now name only
`temp/llama-release-qa`; auxiliary candidate branches use manual dispatch.
The corrected trigger is verified by a subsequent diagnostic push at `78308cf38`:
no extra run starts and the selected job stays active. Windows dependency retry
`35004301149` runs the product inputs at `4a0924ee1`; a workflow/docs-only branch
update need not repeat its unchanged compiler/runtime checks.

## Combined candidate wider qualification

After all four base package lanes pass their recorded triage revisions, push
the combined fixes to `temp/llama-release-combined` for explicit Deep Build and
Sanitizer QA. This manual-only candidate branch preserves the still-useful
Linux CUDA run on `temp/llama-release-qa` and MSVC CUDA diagnostic run. It has no
diagnostic workflow override and no publication authority. Once CUDA triage
settles, run the complete Build matrix on the same combined candidate; any
further product/build change must be reflected in the final exact-head gates.
CI-AC-07 and CI-04 remain open until those terminal results are recorded.

## CUDA build cost and compiler reuse

The first complete Linux CUDA build takes 1 h 54 m 30 s with two build jobs;
its consumer smoke takes 33.524 s. NVIDIA's CUDA runtime/cuBLAS components are
already downloaded as pinned binary redistributables. The expensive work is
compiling the pinned llama.cpp/GGML CUDA kernels for its portable architecture
set. Adrian raised the recurring runner cost and explicitly approved caching the CUDA
engine on 15 September.

1. **CACHE-01:** keep source builds opt-in and standard CPU/Vulkan packages
   small; retain complete separate CUDA ZIPs and all existing CI outcomes.
2. **CACHE-02:** install pinned prebuilt sccache only in the CUDA jobs. Use
   explicit C/C++/CUDA compiler launchers so the pinned GGML auto-detection does
   not add a second wrapper. Use the GitHub Actions cache and preserve compiler,
   source/header and option identity; no permissive cache-key overrides.
3. **CACHE-03:** retain cache statistics, then verify reuse with a warm CUDA
   candidate run on the same branch. Final package checks still execute; cache
   reuse itself is not a correctness or GPU-device pass. Do not claim the
   recurring cost is resolved before actual Linux and Windows hit evidence.

This is compiler-output reuse in the existing build graph, not a new binary
engine ABI or a user-installed NVIDIA dependency. Cache misses, eviction or
changed engine/toolchain/settings can still cost a cold build. It extends
CI-03/04 with CI-AC-09 and leaves all earlier criteria open as recorded.


## Restart continuation — candidate 76df02be3

Adrian resumed after restart. The Windows Deep Build run at `cabc668cf` passes
2,250/2,251 checks; its only failure is the lifecycle helper's missing build-tree
DLL search location (CI-F06 follow-up). Scoped CTest PATH setup is repaired and
the unchanged probe passes local Debug/Apple-ASan checks. Linux and ARM Mac
pass all 2,329 comprehensive and four package tests. Their evidence is retained.

Combined candidate `76df02be3c4d89118bd8d4e4e68308c47f76366e` is pushed only to
`origin/temp/llama-release-combined`. [Build 35013130021](https://github.com/adesutherland/CREXX/actions/runs/35013130021)
starts all six complete packages with CUDA compiler caching enabled. This is
the first cache-populating run, not evidence of warm reuse. Prior useful Intel
Deep Build, Linux/Mac sanitizer and MSVC/CUDA triage runs continue; terminal
results and final exact-candidate gates remain open. No criterion is closed
by dispatch, and no develop/release publication occurs.


## CI-D01 — approved separate downloads and common Windows base

Status: approved by Adrian on 15 September: run the core without llama on every
target configuration first; only after those pass add llama, and implement the
agreed separate release packaging. Earlier pending-approval text is superseded.

### Vision and intended outcomes

1. Keep the ordinary cREXX download small. Users add one prebuilt llama.rexx
   package for their platform/backend and separately provision a model; they
   need no inference SDK, compiler or server to run their programs.
2. Supply one Windows MSVC base with `rxvm` selecting `rxbvm`, usable with either
   the MSVC CPU/Vulkan or CPU/CUDA plugin package. Adrian notes the accepted
   performance results favour the portable VM; no new performance programme
   is selected. Retain MinGW source-build support and its regression coverage.
3. Compile once per required toolchain/configuration, then split the verified
   outputs into core/plugin artifacts. Preserve CPU fallback, GPU support,
   native consumers, documentation, manifests, dependencies and notices.
   Adrian explicitly requested reuse of the already-built core: plugin jobs
   consume its exact artifact, build only plugin/dependency/helper targets, and
   test against those unchanged core binaries. Do not rebuild the cREXX product
   in each Vulkan/CUDA job. One Windows MSVC base serves both variants.
4. Keep QA aligned with the component, as Adrian explicitly requested: core
   jobs run cREXX tests; plugin jobs run only relevant llama adapter, lifecycle,
   packaging and fixture smoke checks. Reuse the core result instead of running
   its full suite again for each backend. Combined installed/native consumers
   remain the integration proof, without retesting model quality or throughput.

### Accepted acceptance amendments (stable existing IDs)

1. **CI-AC-02:** replace the four complete base archives with independently usable
   core archives plus matching CPU/Vulkan (Linux/Windows) or CPU/Metal (Mac)
   plugin archives; verify core-only use and installation from the actual ZIPs.
2. **CI-AC-03:** provide additional complete CUDA plugin archives for Linux and
   Windows, including CPU fallback and redistributable dependencies. The same
   Windows MSVC core must pass with either backend variant, including native
   generation and relocated execution. This replaces the separate complete
   MSVC CUDA base and the requirement to retain a separate MinGW binary
   delivery; MinGW source-build support and regression coverage remain.
3. **CI-AC-05/08:** verify installed core-plus-plugin artifacts through the
   existing restricted-environment smoke; retain package identities, version/
   platform compatibility, guides and unambiguous backend selection instructions.
   Every other CI/parent criterion remains unchanged and visibly open as before.

### Numbered implementation sequence

1. [x] **CI-D01-01:** build and smoke the real llama-free core on all four target
   configurations, with MSVC/`rxbvm` on Windows. Retain exact artifacts and
   semantic checks. No llama compilation or GPU SDK download occurs in this
   stage. All four delivery cores and the Windows/MinGW core-only gate must pass before llama work resumes (CI-AC-02/03/07).
2. [ ] **CI-D01-02:** after the core gate, qualify MSVC Vulkan and the other
   plugin variants; partition staged output into core and self-contained plugin
   archives, preserving native/static inputs and runtime DLL closure. Ensure
   installer behavior and human/agent guidance agree (CI-AC-02/03/05/08).
3. [ ] **CI-D01-03:** smoke core alone and the actual recombined downloads, including
   both Windows backend variants against one base. Complete the exact-candidate
   Build/Deep/Sanitizer gates and retain warm CUDA cache proof before proposing
   promotion (CI-AC-01 through CI-AC-09).

The approved distribution contract is now separate core/plugin downloads.
Implementation and target proof remain open; do not describe the split as
already delivered. Existing evidence is preserved, but previous combined CUDA,
sanitizer and MSVC adapter diagnostic runs were cancelled when Adrian selected
the core-first sequence. Cancellation is not a pass and cache hits remain unproven.

CI-04 execution refinement after Adrian's concern about repeated failures:
the core-first gate above supersedes the intervening isolated-diagnosis order.
Preserve CI-F07/11 evidence and resume those plugin findings after all four core
builds pass. No acceptance requirement is removed by this change of sequence.

Sanitizer scope clarification, explicitly requested by Adrian on 15 September:
the maintained gate concerns our code, not upstream llama.cpp or CUDA.
The core workflow must be independent of inference dependencies. Our adapter
ownership/failure checks remain separate and may use an uninstrumented engine;
record instrumented/uninstrumented boundaries and Mac capability limits.
SAN-009 remains open for its actual first-party closure evidence. This approved
scope does not require upstream-engine sanitizer cleanliness or GPU sanitizer
coverage, and does not waive a first-party finding.

## CI-F12 — Windows core KeyAccess compaction failure

Vision: the MSVC core must compact an ordinary KeyAccess database, preserve its
records, and close safely. This is core qualification under CI-D01-01; no llama
or GPU rebuild is involved. Core run `35018374738` passes 146/147 Windows checks
but `keyaccess_test_noopt` exits `0xc0000409` after printing "Closing database".
Inspection identifies a Windows CRT mismatch: `rename` refuses an existing
destination; compaction leaves both stream pointers null on replacement failure,
then close passes those null pointers to `fclose`. Retain the host failure and
verify the mechanism with focused controls before claiming repair closure.

1. [x] **F12-AC-01:** ordinary compaction returns success on Windows/MSVC,
   retains live key/value pairs and deletions, and survives close/reopen. The
   existing keyaccess opt/noopt tests must assert these outcomes explicitly.
2. [x] **F12-AC-02:** cleanup after a failed compaction never closes a null
   stream or reports the compaction itself as successful. Preserve the I/O
   error and prove cleanup with a focused failure control.
3. [x] **F12-AC-03:** matching focused normal/maintained first-party sanitizer
   controls pass, followed by the Windows-only core qualification retry. Reuse
   unaffected platform/core evidence; do not restart inference jobs.

1. [x] **F12-01:** retain the Windows failure and tighten the ordinary functional
   compaction assertions; establish a bounded replacement-failure control
   (F12-AC-01/02).
2. [x] **F12-02:** use the existing filesystem provider's Windows replace-file
   semantics and guard cleanup of closed streams. Preserve public signatures,
   error codes and ordinary database format (F12-AC-01/02).
3. [x] **F12-03:** run focused core controls and the Windows-only retry, then
   reconcile the four-core gate before resuming plugins (F12-AC-03).


F12 local evidence: the new failure control reports two null closes without the
repair; the three repaired controls pass normal Debug (0.44 s) and Apple ASan
(0.73 s), retained under `local/keyaccess-compaction/` in the CI evidence pack.
The initial ASan invocation used the runner's default leak option, which Apple
rejects before test execution; the completed run uses the documented Apple
`--build-leaks off`/`--leaks off` capability setting. Supported Linux leak
qualification remains required. Only KeyAccess and its core toolchain prerequisites
were built, with no engine/GPU build. F12-AC-01/03 remain open for Windows.

Cache evidence limit: GitHub caches are scoped to the current/default branch
(and the PR base for PR runs). This repository's default branch is `master`;
cache entries populated on the isolated candidate branch do not automatically
warm `develop` or a new release tag. The first build in a new eligible scope
can therefore be cold. Preserve the current same-branch cold/warm proof and
report this boundary; do not change branch publication policy to warm a cache.
See [GitHub cache access restrictions](https://docs.github.com/en/actions/reference/workflows-and-actions/dependency-caching#restrictions-for-accessing-a-cache).

### CI-D01-02 packaging implementation checklist

The existing release Build workflow must implement the approved split, not just
its diagnostic workflow. Preparation may proceed while the core retry runs;
no plugin compilation or plugin QA starts before the four-core gate passes.

1. [ ] **PKG-01:** preserve the version, signing, notarization and installer
   safeguards while making the four product jobs llama-free. Finalize and test
   the actual signed/staged core ZIP, with manifest hashes taken after signing.
2. [ ] **PKG-02:** six dependent plugin jobs download the exact matching core
   artifact from the same workflow/SHA. Build only adapter/engine/package/helper
   targets; use MSVC for both Windows backends and retain CUDA compiler caching.
   No core compilation, whole-core suite or upstream sanitizer is in these jobs.
3. [ ] **PKG-03:** sign only the plugin payload where configured, refresh its
   provider hashes, create a separate ZIP and test the actual combined downloads
   without changing any core bytes. Retain CPU fixture and relevant adapter/
   lifecycle controls, with no trained-model download. Core and plugin manifests
   identify the source, platform, toolchain and required companion archive.
4. [ ] **PKG-04:** publication collectors require four core plus six plugin ZIPs;
   installers remain core-only. Preserve existing Windows signing publication
   guards and optional notarized Mac installer rules. Human/agent download
   instructions and exact-candidate qualification must match this delivery.

This checklist serves CI-AC-02/03/05/07/08/09 and does not authorize publication.
The initial Windows-only repair retry is run `35021253099` at
`7425ff2417715b7e4f285d225867c0501fcb0002`, on `temp/llama-release-combined`.


### Windows MinGW core quality gate — subsequent approved addition

Adrian explicitly requested a fifth configuration on 15 September: Windows
MinGW builds and runs core-only QA, including optimized/nonoptimized semantic
consumer checks through both `rxtvm` and `rxbvm`. It uploads logs/identity/test
evidence only; no MinGW distributable or plugin is shipped. MSVC remains the
single Windows release core for both Vulkan and CUDA. This strengthens
CI-AC-02/03/07 and CI-D01-01 without changing the four-core/six-plugin asset count.

1. [x] **MINGW-AC-01:** Windows/MinGW `ENABLE_LLAMA=OFF` product and core smoke
   pass; extracted-core consumer checks explicitly execute both VM variants.
2. [x] **MINGW-AC-02:** retain exact source/toolchain and terminal logs; no
   distributable binary artifact is uploaded. The gate remains a prerequisite
   for plugin qualification and final publication.
3. [x] **MINGW-01:** add and run the focused Windows/MinGW diagnostic core job.
4. [x] **MINGW-02:** include the same non-shipping gate in the final release
   dependency graph; retain focused evidence without duplicating backend QA.


## CI-F13 — Windows core package environment lookup

MSVC retry `35021253099` builds and passes 148/148 core checks (73.18 s), including
both repaired KeyAccess smoke controls. Packaging then fails before consumer
execution because the Python environment dictionary contains `SYSTEMROOT`,
while the harness requested `SystemRoot`. This is a packaging harness defect,
not a new VM failure. Preserve this successful core result and the failing
trace under `remote/7425ff241/`.

The repair handles Windows environment names case-insensitively and retains
only the OS directories in execution PATH. A focused casing control covers
both spellings and prevents any development SDK PATH leakage. It passes with
the existing six split-package controls. Windows retries also retain any failed
archive as explicitly unqualified diagnostic data, so later package diagnosis
can reuse it; the requested MinGW gate retains no binary archive.

- [x] **F13-AC-01:** the Windows core ZIP completes restricted-environment VM
  and relocated native smoke. The final gate still requires the new MinGW run.
- [x] **F13-01:** repair the lookup and pass both casing controls without
  relaxing runtime PATH isolation.
- [x] **F13-02:** retry the Windows configurations and retain terminal evidence;
  also exercise the optimized KeyAccess control explicitly on those hosts.


### Packaging preparation and current core status

Windows/MSVC `35022668868` at `aade0fd0f` passes 148 core checks, all three focused
KeyAccess checks, and the extracted core ZIP smoke including relocated native
execution (2.718 s). This closes CI-F12; its matching Debug/Apple-ASan proof is
retained and it was a functional failure, not a new sanitizer finding. CI-F13's
MSVC execution proof passes; MinGW retry `35023122646` still owns its second
Windows configuration. Earlier MinGW `35021842079` passes 152 core checks but
fails on that same packaging dictionary lookup before ZIP consumer execution.
The four shipped core configurations now have passing evidence. MINGW-AC-01/02
and the fifth-configuration prerequisite remain open until the new run finishes.

PKG-01–04 implementation is prepared in the working tree: the main workflow
owns four core deliveries, calls the non-shipping MinGW gate, then builds six
plugin variants against exact matching archives. Twenty-five focused packaging,
signing and matrix controls pass, plus actionlint. The core finalizer's actual
ZIP harness passes using the retained hosted ARM core (no new core build).
The six selected plugin targets have zero core compiler/assembler/linker/VM
compile commands in the local dependency graph. This is preparation evidence;
PKG target qualification and the parent CI acceptance remain open. No engine
was built or plugin test executed during the core-first gate.

Windows Vulkan preparation pins LunarG SDK 1.4.357.0 and its published SHA256,
using its documented copy-only installation. Metadata source:
[SDK file index](https://vulkan.lunarg.com/sdk/files.json),
[unattended installation](https://vulkan.lunarg.com/doc/view/1.4.357.0/windows/getting_started.html).
The installer has not been downloaded or run in this preparation phase.


### Initial five-configuration core gate closed

MinGW retry `35023122646` at `aade0fd0f` passes 152 core checks, three focused
KeyAccess controls and all twelve extracted-core consumer commands (2.488 s).
The public `rxvm` copy is verified identical to `rxtvm`; both that entry point
and `rxbvm` execute opt/noopt programs. Relocated native execution also passes.
The run uploads exactly one 30,211-byte QA artifact and no binary archive.
This completes CI-D01-01, MINGW-AC-01/02 and CI-F13. All five core configurations
have green evidence; plugin qualification may now proceed. The final candidate
still needs the main split-pipeline result and wider first-party gates.


The prepared wider core matrix includes both Windows/MSVC (the shipped base)
and Windows/MinGW (both VM variants), plus Linux and the two Macs. All these
comprehensive, graph/stress and sanitizer core configurations explicitly disable
llama. Plugin jobs keep their four focused adapter/path controls and combined
fixture/package smoke. These wider gates are still pending; adding the matrix
rows does not claim a comprehensive pass.

### CI-F07 direct-library attribution control

The retained Intel stack waits inside `ggml_metal_library_compile_all` and
Apple's synchronous compiler service. Source inspection at the existing pin
confirms that `GGML_METAL_EMBED_LIBRARY=ON` embeds shader source and compiles it
at runtime. The alternate precompiled-library mode exists, but its packaging,
device compatibility and effect on this stall are unproven. No production build
policy or upstream shader code is changed by this diagnosis.

1. [x] **F07-AC-01:** a bounded standalone control loads only the exact archived
   GGML libraries, without the cREXX bridge, VM, model or compiler. Verify the
   archive and copied library hashes and retain loaded-image identities.
2. [x] **F07-AC-02:** run that control on Intel Mac with opt-in stack capture.
   A matching stalled direct call attributes the wait below the cREXX bridge;
   a passing replay does not explain or close the intermittent failure.
3. [x] **F07-D01:** validate the diagnostic harness locally against a retained
   ARM archive, then run it on the existing isolated Intel diagnostic branch.
   Reuse the compiled engine; neither a full build nor a model download is needed.
   Serves F07-AC-01/02 and the existing CI-AC-05/07 investigation.

### SAN-009 supported first-party closure within the agreed scope

The hosted core sanitizer deliberately disables llama. To cover the repaired
first-party probe without rebuilding or instrumenting the upstream engine,
reuse a SHA-verified ordinary Linux provider archive and the exact pinned
headers. A private standalone CMake test builds only `bridge.cpp`, first-party
SHA256 support and the existing permanent lifecycle/probe executable. This
adds no production API/build option or broad model workload.

1. [x] **LP-AC-01:** the probe's engine/backend files retain their ordinary
   archive hashes; only first-party targets are built/instrumented. Retain the
   build graph, source/header/archive identities and instrumentation inspection.
2. [x] **LP-AC-02:** the same permanent probe passes normal Debug and ASan/LSan
   on Linux, with leak detection enabled and logs retained through
   `tools/asan-run.sh`. No CUDA SDK, engine rebuild or model download is used.
3. [ ] **LP-AC-03:** combine this focused result with the retained original
   Apple trigger/full proof and the current full supported core sanitizer gate
   before disposing of SAN-009. A probe alone does not close the item or the
   remaining model/device criteria.
4. [x] **LP-01:** prepare the isolated harness and verify it locally against
   retained ordinary runtime libraries; then run the same source on a separate
   Linux diagnostic branch. Serves LP-AC-01/02.
5. [ ] **LP-02:** retain and inspect the focused and wider results, register any
   new first-party finding, and reconcile SAN-009's closure checks. Serves
   LP-AC-03 and existing CI-AC-07; no scope waiver is implied.

### Integrated Windows follow-ups: CI-F11 and CI-F14

Windows Vulkan at `21666b6bc` builds the production bridge, then finds the same
SDK `small` macro collision in the lifecycle QA helper's private runtime-handle
variable. Keep this under CI-F11: the affected helper and repair share the
established cause. The actual helper translation unit fails with `-Dsmall=char`
locally before its identifier is renamed. No runtime/API change is needed.

**CI-F14:** the new MSVC comprehensive core gate stops compiling
`cri17_attached_provider_control.c`: its C90 target includes the private VM's
C11 atomics. The full compile command lacks `/std:c11`; the SDK explicitly
rejects that mode. A preprocessing audit of all 172 local first-party C90
translation units finds `stdatomic.h` only in this harness's two target variants.
Declare their actual C11 requirement privately, following the neighboring
native-object test targets. Do not change the public C SDK or global C standard.

1. [x] **F14-AC-01:** both harness targets declare C11 and MSVC atomics support and retain their existing
   attached-provider lifetime/failure assertions. Verify generated commands and
   focused normal/maintained sanitizer results.
2. [ ] **F14-AC-02:** MSVC builds and runs the affected core controls and completes
   the broader core gate; the tests remain enabled.
3. [x] **F14-01:** retain the host failure/dependency audit, correct the two
   target requirements and validate focused controls (F14-AC-01).
4. [ ] **F14-02:** qualify the Windows correction, then reconcile the wider gate
   (F14-AC-02). Reuse unaffected core/plugin evidence and cached engine outputs.

Manual Deep QA now has an explicit core-platform selector so this MSVC retry
does not repeat all passing platforms, build-parallelism checks or stress work.
The default and scheduled gate retain all five core configurations and the
existing independent checks. A selected-platform run is reported as partial
qualification, never a complete Deep pass.

The direct GGML controls pass locally and in Intel runs `35026224152` and
`35026657758`; CI-F07 itself remains unresolved. Linux first-party probe
`35027428540` passes Debug/ASan-LSan (CTest 1.08/1.77 s) with ordinary engine
files; the broader gate still keeps LP-AC-03 and SAN-009 open. CI-F14 passes
normal/Apple-ASan 2/2 controls and generated C11 command inspection. Its Windows
retry is `35028634025`, while helper/package retry `35028632018` covers CI-F11.
Both use `b4d10adfb`; full combined qualification remains a separate gate.

**F07-D02 — isolate the archived bridge:** the direct GGML controls passed,
while the integrated Intel provider lifecycle check remains running unusually
long. Next compile only a tiny C executable calling the archived bridge's
existing permanent probe entry point, with no cREXX VM/compiler or model. Verify
the archive/library hashes, exercise the same entry point as the current CTest,
and retain an opt-in stack capture if it stalls. Run the local ARM control first,
then the isolated Intel diagnostic branch. This bounds the remaining
bridge-versus-process-context question; passing controls still do not close
CI-F07, and no production code or normal deadline changes are authorized by it.

CI-F14 follow-up: the first MSVC retry `35028634025` proves that declaring C11
alone was incomplete. MSVC also requires `/experimental:c11atomics`, already
used by neighboring VM-header consumers. Put both requirements in the shared
private harness configuration. Before another broad build, compile both actual
CMake-generated object commands: removing just the atomics switch must reproduce
the SDK error, and the unchanged command must pass. This preflight builds no
core executable or engine. Retain its commands and negative/positive logs.
F14-AC-01 and F14-01 are reopened until that Windows proof; unchanged normal and
Apple sanitizer evidence remains valid for their unaffected commands.

F07-D02 reproduces the wait in the archived bridge alone on Intel
(`35029475651`, `50b7bf8fa`), with the same Metal compiler-service stack and
no cREXX VM/model. The earlier direct control used a Python process.
**F07-D03:** compare tiny native-C direct GGML processes (quiet/default logger),
the earlier Python direct path and the archived bridge on one Intel runner,
using identical verified libraries. Retain process/library identities and
stacks for each bounded diagnostic stop. This controls for native process
context and logging before assigning the remaining cause; it changes neither
production code nor normal QA timeout/coverage. Validate the native harness
on the retained ARM archive first.

The actual MSVC preflight in `35030598584` (`3aca2dd27`) passes both negative
and positive object controls before starting the comprehensive gate. This
closes F14-AC-01/F14-01 again, with the unchanged normal/Apple-ASan evidence;
F14-AC-02 remains open. The preceding preflight-only stop `35030328193`
was its `/std:c11` assertion rejecting CMake's valid `-std:c11` spelling,
now corrected; it did not spend a broad-build cycle.

MinGW's full core Deep job at `21666b6bc` is green: 2,248 comprehensive tests
and three install/package/external-consumer checks. Both VM modes occur in
the retained log. MINGW-AC-01/02 stay closed; this strengthens the earlier
152-test smoke and actual-ZIP proof without adding any binary delivery.

### CI-F15 — package dependency scan mixes source and published locations

Windows Vulkan `35028632018` at `b4d10adfb` builds all plugin/helper targets,
then `WriteProviderPackage.cmake` reports conflicting `MSVCP140.dll` paths in
the Visual Studio redist and already-copied provider directory. The package
writer copies declared runtime files first but scans their original locations.
The intended outcome is to inspect dependencies of the actual published files
without weakening missing/conflicting dependency checks or choosing arbitrary
DLL versions. No production API or backend selection changes are needed.

1. [x] **F15-AC-01:** a small ordinary shared-library fixture reproduces the
   duplicate-location scan failure before the repair, then passes with all
   runtime hashes retained after scanning the published files.
2. [ ] **F15-AC-02:** the actual MSVC fixture and Windows Vulkan package pass;
   distinct unresolved dependencies still fail rather than being ignored.
3. [x] **F15-01:** retain the failed package log, reproduce with tiny libraries,
   and correct the scan roots to the already-published paths (F15-AC-01).
4. [ ] **F15-02:** run the tiny Windows package preflight before retrying the
   optional plugin build; retain actual archive/runtime smoke (F15-AC-02).
   Preserve the ongoing core, CUDA and Intel diagnostic results.

F15-AC-01 passes with a real dylib reproducer: the original scan reports
two conflicting paths to identical bytes, while scanning the published roots
passes and retains every file hash. The permanent tiny-library control also
requires an omitted dependency to fail. Normal and ASan-built fixture checks
each take 0.09 s; these are packaging scans, not sanitizer execution coverage.
Windows retry `35031332714` uses `99a28ce7d`; its package preflight runs before
the optional engine build. F15-AC-02 remains pending actual Windows evidence.

Intel core Deep completes 2,326 tests and three qualification checks at
`21666b6bc`. Intel process-context control `35030619064` passes native quiet,
native default, Python direct and archived bridge starts in
77.110/0.284/83.992/0.498 seconds. These are sequential controls on one host,
not an unconfounded speed comparison. The bridge ran after direct calls;
CI-F07's cold startup failure and captured compiler-service wait remain open.
No precompiled-Metal packaging or other engine/runtime change has been made.

Linux CUDA `35025115905` passes its split archive smoke at `21666b6bc`. Its
760,721,716-byte plugin ZIP is separate from the core; smoke takes 23.494 s.
Compiler caching shows 1,476 hits/1,254 misses (54.07%) and one write error.
This proves useful partial reuse; complete warm-repeat and Windows CUDA
evidence remain outstanding under CI-AC-09.

## 16 September repair and qualification continuation

Vision: complete the approved four-core/six-plugin delivery on the isolated
candidate branch, preserving unchanged core bytes and useful target-platform
QA. Fix concrete Windows packaging/test selection defects, deliver a working
Intel CPU package under CI-D02, and retain full exact-candidate hosted results
before any promotion. No engine performance campaign or upstream sanitizer
build is added.

1. [ ] **F16-AC-01:** MSVC's HTTP optimized-copyback shape test reads an artifact
   actually generated for its portable VM and retains every existing assertion.
   Its focused run and the complete MSVC Deep gate pass.
2. [ ] **F17-AC-01:** core and plugin select the active developer environment's
   MSVC redistributables consistently. A pre-build comparison with the qualified
   core rejects missing/different runtime bytes, and both final Windows plugin
   archives pass with the core-overwrite guard intact.
3. [ ] **D02-AC-01:** the Intel archive contains CPU backends and no Metal
   backend; the complete extracted archive smoke passes. Human/agent guides and
   published asset names identify Intel CPU-only support explicitly.
4. [ ] **R16-AC-01:** all required candidate Build, Deep and first-party core
   Sanitizer jobs reach terminal success on the final code/test inputs; record
   exact SHAs, archives and CUDA cache statistics. CI-AC-07/09 stay open until
   their required evidence is reconciled.
5. [ ] **R16-01:** repair the MSVC artifact path (CI-F16), retain focused proof,
   and rerun MSVC Deep. Serves F16-AC-01; no compiler rewrite change.
6. [ ] **R16-02:** repair the reproduced 14.51 environment versus 14.44 CMake
   CRT selection (CI-F17), add cheap positive/mismatch checks before engine
   compilation, and retry Windows packaging. Serves F17-AC-01.
7. [ ] **R16-03:** implement CI-D02 in matrix, package validation and guides;
   rerun Intel CPU packaging. Serves D02-AC-01.
8. [ ] **R16-04:** retain the now-green core sanitizer results and reconcile
   SAN-009's separate first-party bridge proof; dispatch/reconcile the complete
   candidate gates after focused repairs. Serves R16-AC-01 and CI-AC-07/08/09.
   Preserve the existing real-device/model qualification boundaries.
