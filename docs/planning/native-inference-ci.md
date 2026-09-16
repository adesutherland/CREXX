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
   Vulkan is the general Windows/Linux plugin download; CUDA remains an
   optional download built and tested in every full release build, including
   beta builds, or by explicit manual GitHub Actions request. Ordinary pushes,
   pull requests and development snapshots do not select CUDA (CI-D03).
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

10. [x] **CI-AC-10:** workflow selection implements CI-D03: every full release
    build, including beta, requires both CUDA plugin packages from the release
    candidate's exact core/source identity and successful focused package smoke.
    Outside full release builds, CUDA runs only when explicitly selected through
    manual GitHub Actions dispatch. Retain selection controls for stable release,
    beta release, ordinary push/PR/development snapshot (including CUDA-related
    changes), and manual CUDA selection. An omitted lane is reported as untested,
    never passed; a partial run cannot publish a full release.

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
6. [x] **CI-06 — Apply the approved CUDA cadence:** implement and document the
   CI-D03 release-build/manual-only selection after the current candidate qualification,
   retaining mandatory complete-release gates and cache identity. Add focused
   workflow-selection controls and verify the full-release collector still
   requires both CUDA packages. Serves CI-AC-03/07/08/09/10; implemented and covered by the
   final-delivery selection/collector controls recorded below.

## Design boundaries

- **CI-D03, approved and clarified 16 September 2026 — optional CUDA download,
  release-build/manual-only CUDA testing:** Adrian selected options 1 and 3,
  then explicitly limited CUDA execution to every full release build (including
  beta builds) or an explicitly selected manual GitHub Actions request. Keep
  Vulkan as the general Windows/Linux choice and CUDA as a separate optional
  NVIDIA plugin download. Full release builds build, package and run focused
  smoke on Linux and Windows CUDA for convenience and QA assurance, using the
  same release identity and qualified core as the other plugin variants.
  Here "smoke" means bounded package/load/tiny-fixture checks within that CUDA
  build; it is not a separate routine trigger, large-model test or benchmark.
  Valid compiler-cache reuse remains encouraged; a full release build need not
  compile the engine cold. Ordinary pushes, pull requests and development
  snapshots do not automatically build or test CUDA, even after CUDA-related
  input changes. Such changes can motivate a manual run; the earlier automatic
  relevant-change proposal is superseded. Retain all six full-release plugin variants, their CPU
  fallback, notices/dependencies and separate real-device qualification gates.
  Do not move SDK installation onto recipients. This amends routine CI cadence,
  not the promised full-release payload or the current frozen qualification
  run. CI-06/CI-AC-10 now implement this selection; manual dispatch
  defaults to the non-CUDA `base` lanes. A skipped CUDA run cannot qualify or publish a
  full release.

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
   core installers remain core-only; CI-D04 adds separate optional plugin
   installers for Windows and Mac. Preserve existing Windows signing publication
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
2. [x] **F14-AC-02:** MSVC builds and runs the affected core controls and completes
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
2. [x] **F15-AC-02:** the actual MSVC fixture and Windows Vulkan package pass;
   distinct unresolved dependencies still fail rather than being ignored.
3. [x] **F15-01:** retain the failed package log, reproduce with tiny libraries,
   and correct the scan roots to the already-published paths (F15-AC-01).
4. [x] **F15-02:** run the tiny Windows package preflight before retrying the
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

1. [x] **F16-AC-01:** MSVC's HTTP optimized-copyback shape test reads an artifact
   actually generated for its portable VM and retains every existing assertion.
   Its focused run and the complete MSVC Deep gate pass.
2. [ ] **F17-AC-01:** core and plugin select the active developer environment's
   MSVC redistributables consistently. A pre-build comparison with the qualified
   core rejects missing/different runtime bytes, and both final Windows plugin
   archives pass with the core-overwrite guard intact.
3. [x] **D02-AC-01:** the Intel archive contains CPU backends and no Metal
   backend; the complete extracted archive smoke passes. Human/agent guides and
   published asset names identify Intel CPU-only support explicitly.
4. [ ] **R16-AC-01:** all required candidate Build, Deep and first-party core
   Sanitizer jobs reach terminal success on the final code/test inputs; record
   exact SHAs, archives and CUDA cache statistics. CI-AC-07/09 stay open until
   their required evidence is reconciled.
5. [x] **R16-01:** repair the MSVC artifact path (CI-F16), retain focused proof,
   and rerun MSVC Deep. Serves F16-AC-01; no compiler rewrite change.
6. [ ] **R16-02:** repair the reproduced 14.51 environment versus 14.44 CMake
   CRT selection (CI-F17), add cheap positive/mismatch checks before engine
   compilation, and retry Windows packaging. Serves F17-AC-01.
7. [x] **R16-03:** implement CI-D02 in matrix, package validation and guides;
   rerun Intel CPU packaging. Serves D02-AC-01.
8. [ ] **R16-04:** retain the now-green core sanitizer results and reconcile
   SAN-009's separate first-party bridge proof; dispatch/reconcile the complete
   candidate gates after focused repairs. Serves R16-AC-01 and CI-AC-07/08/09.
   Preserve the existing real-device/model qualification boundaries.

### CI-F15 follow-up — duplicate published runtime entries

At `31d4974f1`, both Windows CRT preflights pass. Windows Vulkan also compiles,
passes four controls, stages/finalizes the split archive, and passes engine,
provider and native-build checks. Relocation then fails with WinError 32.
The retained manifest contains two identical entries each for `msvcp140.dll`,
`vcruntime140.dll` and `vcruntime140_1.dll`: original SDK paths and copied paths
survive full-source-path deduplication but describe the same published file.
The relocation helper's second copy attempts to overwrite its own hard link.
This continues CI-F15's source-versus-published identity defect; it is not an
unexplained process lock or a new engine/runtime failure.

1. [x] **F15-AC-03:** the existing tiny native-library fixture fails when its
   manifest repeats a published dependency, then passes with one entry per
   identical published path/hash. Missing dependencies still fail. Distinct
   hashes must not be deduplicated into an arbitrary choice.
2. [x] **F15-03:** add the failing uniqueness assertion, deduplicate identical
   serialized runtime entries, and run the existing normal/sanitizer-built
   tiny fixture. Serves F15-AC-03, retaining its measured serial scheduling.
3. [x] **F15-04:** retry Windows Vulkan on an isolated branch and retain full
   relocated-consumer smoke. Keep the current CUDA/cache and wider core runs
   alive; unchanged core proof is reused. Final combined qualification remains
   distinct from this focused repair.

The uniqueness assertion reproduces the duplicate published dependency locally
before the repair. Exact serialized path/hash deduplication passes the unchanged
normal fixture in 0.10 s and the sanitizer-built fixture via the maintained
runner in 0.09 s. The missing-dependency negative still fails as required. These
are metadata/dependency scans, not instrumented-library execution. Evidence is
`local/published-runtime-identity/` in the pipeline ledger. Target Windows smoke
remains open; this repair changes neither core execution nor upstream engine
inputs, so keep the current broader runs and CUDA cache build alive.

MSVC and MinGW comprehensive QA now pass on `31d4974f1` in `35071052712`;
F16-AC-01/R16-01 are closed. Intel CPU `35071050120` passes its complete
archive smoke; the downloaded 9,141,464-byte ZIP has only CPU backends and no
Metal library or model fixture. D02-AC-01/R16-03 are closed. SHA256:
`aaacf714432c0497319ad3e6d68325eaeecb35066d949c30795f971c5cb8c740`.
Windows focused retry `35073667866` uses `f749203a7` with the published-entry
repair; final Windows relocation remains open. Other jobs are preserved.

Linux CUDA also passes at `31d4974f1`: 2,726 cache hits and four misses (99.85%),
zero read errors and one write error. CUDA, device-code and PTX cache categories
report 100% hits; the one CUBIN and three C/C++ misses remain visible. The
plugin-target build took 18m31s; this is build/cache evidence, not inference
performance. CI-AC-09's Windows warm result and final CUDA package remain open.

Full Deep workflow `35071052712` is now green on all five platforms at
`31d4974f1`. Windows CUDA's terminal failure is the same duplicate manifest
entry at native relocation; all earlier controls/package steps pass. It records
2,640 cache hits/five misses (99.81%) and three write errors. Both platforms
therefore have genuine warm-cache observations; final Windows CUDA delivery
and final combined qualification remain open. The generic writer is used by
other RXPA providers too: after the focused Windows repair passes, freeze the
combined revision and run the required final hosted gates once on that revision.

Windows Vulkan retry `35073667866` is green at `f749203a7`, including the
strengthened tiny fixture, CRT identity, all four provider controls, archive
hash/overlap guards and final relocated native execution without SDK paths.
F15-AC-02 and F15-04 are closed. Freeze the final combined revision after this
evidence checkpoint and run the complete required hosted gates once. Supersede
the older in-progress `31d4974f1` sanitizer run; its baseline closure evidence
already exists, and the final run must include the shared-writer repair. No
unchanged broad local suite or model/upstream sanitizer campaign is repeated.

### CI-F18 — MinGW project-build failure on the frozen candidate

Vision: finish the same four-core/six-plugin qualification without discarding
green exact-candidate evidence or masking an intermittent core failure.

Build `35076278681` attempt 1 at `f10e70ee5` passes all four shipped core
jobs. The non-shipping MinGW gate passes 151/152 smoke tests, then blocks all
plugin jobs. `crexx_project_build_contract` fails after 4.29 seconds during
the `--nooptimize` two-member wave: both members start, only `projectbeta`
reports completion, and the controller returns 1 without an operation-level
diagnostic. This is not a timeout. The test already uses `RUN_SERIAL` and a
900-second hang guard; neither weakening concurrency nor extending a timeout
is justified by this evidence.

The full Deep Build `35076281099` passes at the identical SHA, including
2,248 MinGW tests and three install/package checks. The same project-build
contract passes there in 34.35 seconds. All other comprehensive platforms,
stress, Release job-count builds and bytecode equivalence are green. These
are controls, not a diagnosed cause for the separate failure.

1. [x] **F18-01:** retain the failed job/CTest log, build configuration and
   same-SHA passing Deep control; inspect the actual error before changing
   product code or scheduling. Evidence: `remote/f10e70ee5/` in the CI ledger.
2. [x] **F18-AC-01:** the mandatory MinGW core gate succeeds on the frozen
   candidate. Retain the first failure and the diagnostic retry separately;
   a retry pass alone must not be described as a repaired root cause.
3. [x] **F18-02:** rerun only failed/dependent Build jobs once with diagnostic
   logging on the unchanged SHA. If the failure recurs, retain the failing
   member workspace and add operation-level diagnostics before a focused
   reproducer/repair; do not repeatedly rerun until green. Serves F18-AC-01
   and CI-AC-02/07. Attempt 2 was dispatched on 16 September.

Core Sanitizer QA `35076283269` remains running on both hosts; it is not
cancelled or counted as passing. Deep is not repeated. CI-AC-02/03/05/07/08/09
and R16-AC-01 stay open while the mandatory retry and plugin delivery remain
incomplete. No change to `develop`, release publication or model/GPU acceptance
is implied.

#### CI-F18 race review and unchanged retry — 16 September

Attempt 2's MinGW job `104749506781` passes all 152 smoke tests, three
KeyAccess checks and the package smoke. The project-build contract passes in
33.93 seconds. All six dependent plugin jobs have now started. Retain both
attempts; the retry does not establish a repaired root cause.

The review finds no competing-test explanation in this gate: all 152 remote
smoke names match the inspected local registrations, no explicit writable
`WORK_ROOT`/`WORK_DIR`/`OUTPUT_DIR`/`BINARY_DIR` is shared by those registrations,
and the project test has a single registration, private root, resource lock and
`RUN_SERIAL=TRUE`. The workflow runs preparation, CTest and packaging in sequence.
This scan is not proof that every dynamically constructed test path is unique.
Within the project test, each `execute_process` finishes before the next edit;
member output/stamp paths are distinct; `scope.join`, `scope.close` and
`pool.close` precede controller publication and the next invocation. The Unix
moving-input control deliberately changes a source during compilation, but it
does not execute on the failing Windows host. Do not remove the intended
two-worker concurrency or add sleeps to make this test green.

Two source findings warrant targeted follow-up, without asserting that either
has reproduced the runner failure:

- **Windows handle-inheritance race window:** `rxspawn.c:4141` enables the
  handle allowlist only when stdin, stdout and stderr are all redirected.
  `crexx_build_worker.crexx:54` redirects only stdout/stderr; the argv path
  preserves absent stdin at `rxspawn.c:2032`, and `CreateProcessW` still receives
  `bInheritHandles=TRUE` at line 4364. That permits unrelated inheritable
  handles from another worker to enter the child. The VM FOPEN handler clears
  inheritance only after `fopen` (`rxvmhandlers_string.inc:3008–3018`), leaving
  an open-to-clear window. If another worker launches in that window, the child
  can retain a temporary stamp file after its owner closes it, potentially
  making the Windows rename fail. The code/OS mechanism is identified; its
  attribution to this particular CI failure remains unproved.
- **Diagnostic gap:** missing assembled output and stamp write/rename failure
  return the same silent 1 (`crexx_build_worker.crexx:78–82,173–175`); the
  controller suppresses the failing task's integer value (`crexx.crexx:1290`).
  Therefore the existing failure log cannot distinguish these branches.

Microsoft documents unrestricted inheritance as a multithreaded launch hazard
and provides the handle allowlist for this purpose:
[CreateProcessW](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw).
The CRT also supports opening files without inheritance:
[fopen](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fopen-wfopen).
The source references above are against the unchanged `f10e70ee5` candidate.

4. [x] **F18-AC-02:** a deterministic Windows regression establishes whether
   partial-redirection launches inherit another owner's handle; both MSVC and
   MinGW controls identify the child handle and rename outcome. Any repair
   preserves inherited standard-stream behavior and intentional worker
   concurrency; do not claim causal closure solely from a green retry.
5. [x] **F18-03:** construct a bounded Windows reproducer with explicit
   synchronization and a fully redirected control, then repair the proved
   mechanism with focused tests. Add operation-level worker diagnostics and
   retain the failing workspace when reproducing. Serves F18-AC-02 and
   CI-AC-07. This review changes no runtime/test/build input and leaves current
   plugin and sanitizer runs undisturbed.

Review evidence is `local/f18-race-review/`; the successful retry log/artifact
is retained alongside attempt 1 under `remote/f10e70ee5/`.

#### Existing related issue — #701

Adrian's issue review identifies open
[#701: POSIX child launches inherit unrelated pipe ends, delaying exit completion until sibling workers close](https://github.com/adesutherland/CREXX/issues/701),
filed 15 September. It records actual macOS cross-worker pipe descriptors and
delayed child completion in cREXX-RAG. Its generic-launcher diagnosis is closely
related to CI-F18: an unrelated child retains a resource owned by another
worker. On POSIX the documented mechanism is an inheritable child pipe end
surviving exec; on Windows the reviewed risk is incomplete handle allowlisting
and the file-open/clear-inheritance window. The symptoms and platform mechanisms
must remain distinct until reproduced; #701 does not prove the Windows stamp
rename explanation.

The entire `interpreter/rxspawn.c` blob in #701's observed revision
`037e7939bc29eb91b29ed41e9b1b8debdef6353d` is identical to this candidate:
Git blob `c88ab671291d5ddf8f5a7d276c553a44babba3b0`. Thus the reported POSIX
source path remains present, rather than being a historical repair already
absorbed here. The issue is open with no comments at this review.

Coordinate F18-03 with #701's generic launcher ownership work, retaining
separate deterministic Windows and POSIX controls. Do not create a duplicate
public issue or mark either platform repaired from the unchanged green retry.
Closed #695 (repeated observation during task cleanup), #697 (Darwin exit-time
EPERM) and #698 (timeout-only sanitizer replay) describe different mechanisms.
Older #646/#669 stream-inheritance fixes must remain intact: unredirected
standard streams still need normal prompt/output behavior.

The read-only issue snapshot and source identity comparison are retained in
`local/f18-race-review/`. No GitHub issue/comment was changed by this review.

#### CI-F18 repair integration and focused retest — 16 September

Adrian confirms the reported defect is fixed and requests a retest, explicitly
authorizing the merge. Vision: incorporate the qualified #701 repair from
`origin/develop` at `17e844441ed87e1f6e0d5f1f0d3bb4bee8db6187` into this
llama candidate and verify the affected project-build behavior. Preserve
concurrent workers and prior full-suite evidence; do not repeat CUDA, model,
Deep or full sanitizer qualification for this bounded integration check.

The hotfix's [authoritative record](issue-701-resource-inheritance.md) retains
deterministic baseline-negative/repaired-positive controls on Linux, macOS,
MSVC and MinGW (run `35088226165`, code `135b9254f`). Windows reproduces unwanted
handle inheritance and blocked rename, then proves exclusion/success after the
repair. This establishes the mechanism, not attribution of the original silent
F18 failure. Worker diagnostics now identify missing output and stamp write/
rename failures. Reuse those exact-source controls rather than rerunning them
solely for a merge SHA.

1. [x] **F18-RAC-01:** the merge includes the unchanged qualified launcher,
   atomic file-open helper, worker diagnostics and permanent regressions.
   Retain source-blob identity comparison. Resolve the two overlapping MSVC
   test-registration repairs by preserving this branch's capability-based
   selection and optional runner list, then run that test locally.
2. [x] **F18-RAC-02:** focused ordinary Debug inheritance/file-open, stream,
   lifecycle, worker diagnostics, project-build and merge-resolution checks
   pass on the combined tree, with retained command/log evidence.
3. [x] **F18-RAC-03:** the previously failing Windows/MinGW core qualification
   passes on the merged candidate, including the project-build contract and
   new inheritance/diagnostic regressions. Retain terminal hosted artifacts.
   Other platform mechanism controls reuse the unchanged hotfix evidence.
4. [x] **F18-R01:** merge develop and verify repaired input identities;
   serves F18-RAC-01. No develop publication or API change is part of this work.
5. [x] **F18-R02:** run the focused local panel and manually dispatch only
   `release-core.yml` with `platform=windows-mingw` on the candidate branch;
   serves F18-RAC-02/03. This is the smallest existing hosted lane that repeats
   F18's original environment without rebuilding any inference backend.
6. [x] **F18-R03:** retain results, reconcile F18-AC-02/F18-03 and the handoff;
   serves F18-RAC-01–03. Keep original incident attribution qualified.

Previous candidate `f10e70ee5` now has terminal success for Build `35076278681`,
Deep `35076281099` and core Sanitizer `35076283269`; these remain that revision's
results, not new full-suite results on the merged tree. CI-D03's future workflow
cadence and parent real-device/model criteria remain separately open.

Local integration passes all nine focused Debug checks in 91.90 seconds on
`2b897caa5`; build log, exact commands and source identities are retained in
`local/f18-merged-retest/` under the pipeline QA ledger. No code/test input
changed after that run. Hosted MinGW retest
[35104282036](https://github.com/adesutherland/CREXX/actions/runs/35104282036)
is terminal **success** on the same candidate: 155/155 core smoke checks,
including resource inheritance, atomic private file opening, worker diagnostics
and the project-build contract (31.59 seconds); three KeyAccess checks; and
actual extracted-core execution across both optimization/VM modes plus a
relocated native consumer. Evidence is retained under `remote/2b897caa5/`.

F18-RAC-01–03 and F18-R01–03 are complete. F18-AC-02/F18-03 now close against
the retained deterministic MSVC/MinGW hotfix controls, unchanged repaired input
identities and this combined-candidate retest. Original silent-incident
attribution remains unproved; do not describe its green rerun as causal proof.
The requested merge/retest is complete. No new broad/real-device qualification,
CUDA workflow implementation, develop promotion or release publication is
claimed. Documentation-only closeout preserves the tested runtime/build inputs.

### CI-F19 — Release policy, signing and installer audit — 16 September

Adrian requests current status against CI-D03, separate distribution, signing,
installers and remaining product tests. This is a read-only implementation audit
plus this durable finding; no workflow, signer, installer or product was changed.
Vision: the agreed split delivery must survive final signing and installation,
with CUDA restricted to full release builds (including beta) or explicit manual
GitHub Actions requests. Green unsigned candidate ZIPs alone do not verify it.

Findings against `d612e66c2` (runtime/build inputs unchanged from `2b897caa5`):

Current-status reconciliation at `09d4ffaf2`, 16 September: the findings below
are the original audit snapshot. CI-D04 subsequently implemented the native
Mac/Windows plugin installers and Rexx manager; their actual unsigned lifecycle
checks pass (`35110318683` and `35119980116`), satisfying F19-AC-03. F19-AC-01/02
remain open: current workflow selection still includes routine CUDA, and the
maintainer Windows signer still assumes one ZIP and omits core/plugin manifest
refresh after signing. The latest develop #699 fix (`f786b15d8`, plus evidence
at `94f2f228c`) has green Build/CodeQL but is not in this candidate. Integrate it
and finish these existing delivery steps before one combined final Build/package
check; reuse unchanged broad Deep/sanitizer evidence and focus integration tests
on affected import/provider behavior. The [live status table](../qa/native-inference-ci/README.md#current-overall-status--16-september)
records exact run/revision boundaries. Parent hardware/model criteria and actual
signed/offline proof remain open; no scope change or promotion is implied.

**Execution approved 16 September:** Adrian approved this continuation after
the overall status review. Preserve the existing vision and ACs above. Execute
F19-01/02/03 and INST-02, merging current develop #699 into the candidate first
and retaining a focused import/provider integration check. Then run one combined
Build/package qualification with all six plugins explicitly selected, including
non-publishing Mac signing/notarization checks and Windows retained-artifact
signing where credentials permit. Reuse unchanged Deep/sanitizer evidence.
No develop promotion, release/tag publication or silent hardware waiver is
authorized by this approval. Credential or device limitations keep their named
criteria open, without blocking independent implementation and build proof.

- The four-core/six-plugin split, unchanged core reuse, packaged dependencies,
  fixture smoke and CUDA compiler cache are implemented. All six variants pass
  Build `35076278681` at `f10e70ee5`; the later inheritance merge has its bounded
  local/MinGW proof above. This is not a new full release qualification.
- CI-D03 is still documentation only. A direct call of
  `scripts/ci-release-matrix.py:select` returns six plugins/two CUDA lanes for
  ordinary develop pushes, PRs and even manual `base` selection on develop.
  Candidate-branch manual `base` correctly selects four. The dev-snapshot
  collector also requires both CUDA ZIPs; changing only the matrix will break
  snapshot publication. Full release collection must still require all six.
- Mac core/plugin signing, signature verification, post-signing provider hash
  refresh and notarization are wired. The candidate's successful manual Build
  run skipped actual signing, notarization and installer steps, as verified
  from its terminal job metadata. Trusted non-publishing candidate qualification
  of these publication-only paths is still needed.
- Windows signing uses maintainer scripts rather than runner credentials.
  `sign-windows-asset-common.sh` selects a single Windows ZIP and identifies its
  root by `BUILDINFO`; `package-windows-installer-common.sh` also selects one
  Windows ZIP. Split releases make automatic selection ambiguous, and plugin
  archives contain `llama-package.json` rather than `BUILDINFO`.
  `windows-signing-common.sh` refreshes provider manifests after verified
  signatures, but does not refresh `core-package.json` or `llama-package.json`.
  A temporary positive/negative control confirms that unchanged core bytes pass
  `verify_core`, while a simulated signing byte change fails `Core file changed`.
  This control tests hash handling, not a real Authenticode signature.
- Installers are correctly core-only under PKG-04. Windows install/reinstall/
  uninstall QA exists, but its standalone workflow always selects current
  develop; it cannot yet select this retained candidate. Its smoke does not add
  either optional plugin. Plugin ZIP instructions cover a portable shared
  directory, not the installed Windows/Mac/Linux prefix and upgrade lifecycle.
- Remaining trained-model/real-device coverage stays as recorded in the parent
  plan. Tiny fixture CPU execution does not prove Windows/Linux BGE/Smol or
  actual CUDA/Vulkan computation. Reuse accepted local model, numeric and glue
  performance evidence; no new model benchmark or provenance project follows.

1. [x] **F19-AC-01:** CI-D03 selection and snapshot/release collection agree;
   stable/beta/manual and push/PR/snapshot controls pass. Serves CI-AC-10/CI-06.
2. [ ] **F19-AC-02:** final signed core and plugin archives have consistent
   identities/hashes and pass a combined consumer smoke. Retain actual Mac
   signature/notarization and Windows signing evidence, distinguishing unsigned
   and signed assets. Serves PKG-01/03/04 and CI-AC-05/08.
3. [x] **F19-AC-03:** a retained candidate can be installed as core-only, extended
   with its matching optional plugin, used, upgraded/reinstalled and removed
   according to documented ownership rules. Retain platform-specific evidence,
   VM selection, runtime dependency and mismatched-version diagnostics.
   Serves PKG-04 and CI-AC-02/03/05/08.
4. [x] **F19-01:** finish CI-06's event/asset-selection implementation and focused
   controls; do not rebuild models or CUDA for the policy controls (F19-AC-01).
5. [ ] **F19-02:** adapt Windows signing to explicit core/plugin artifacts and
   verified final manifest identities, then provide non-publishing candidate
   signing/installer checks that reuse compiled artifacts (F19-AC-02/03).
6. [ ] **F19-03:** reconcile the package checklist, installed guides and remaining
   hardware cells with retained evidence. Repeat only affected checks; broad
   unchanged core/sanitizer suites are not needed for this audit or documentation.
   Normal final release build gates remain separate (F19-AC-01–03, CI-AC-08).

### CI-D04 — Approved optional plugin installers — 16 September

Adrian explicitly requests implementation now. Preserve the small core and
separate portable ZIPs; additionally supply Windows and Mac plugin installers
which find/check an existing cREXX installation and copy the matching plugin
there. The intended outcome is an ordinary install experience with no compiler,
Python, inference SDK, server or model download required by the installer.
Models remain separately provisioned user data. This extends PKG-04 and
F19-AC-03; it does not change CUDA cadence or authorize publication.

On Windows use the registered core location, with an explicit directory choice
for portable installs. On Mac discover the core package receipt/default location
or an explicitly selected installation. Never silently choose between ambiguous
installations. Check exact source/platform/toolchain identity and packaged hashes
before mutation. Copy only plugin-owned files, preserve core/VM selection and
environment settings, and reject overlapping or incompatible payloads. Support
reinstall and removal of the plugin without deleting core files or models.

Adrian's Mac concern is Gatekeeper/notarization network checks. Sign contained
code and the installer, notarize and staple the ticket to the final `.pkg`;
retain signature/ticket validation and an offline installation/execution check.
Stapling supplies local notarization evidence; it does not promise that macOS
will never contact Apple or disable its security policies. Do not remove user
quarantine attributes or bypass Gatekeeper to claim success.

1. [x] **INST-AC-01:** Windows and Mac installers discover/select a matching
   installed core; absent, ambiguous, altered or incompatible cores fail before
   copying. Retain positive and negative controls, including paths with spaces.
2. [x] **INST-AC-02:** install/reinstall/remove affects only declared plugin
   files, preserves core hashes/VM selection, PATH and models, and detects mixed
   backends. Copy failures preserve/recover the previous installation. Retain
   focused lifecycle and failure controls; document core upgrade ordering.
3. [x] **INST-AC-03:** actual installer outputs contain complete dependencies,
   notices and final matching manifests, with no models/fixture/SDK. Installed
   provider/toolchain smoke succeeds using retained candidate binaries on Mac
   and Windows; no core or CUDA rebuild is required for these packaging tests.
4. [ ] **INST-AC-04:** release Mac plugin packages are signed, notarized and
   stapled; Windows installers/payload are signed through the maintained signing
   path. Signature/hash and offline Mac evidence are retained separately from
   unsigned lifecycle tests; unavailable credentials leave that proof open.
5. [x] **INST-01:** implement package validation, discovery and safe plugin
   lifecycle helpers and native installer wrappers (INST-AC-01/02).
6. [ ] **INST-02:** connect retained-artifact packaging/QA and release signing
   paths; run the focused controls on both platforms (INST-AC-03/04).
7. [x] **INST-03:** update user/agent guides and handoff with installation,
   removal, core-upgrade and Gatekeeper behavior, then reconcile evidence and
   open delivery gates without claiming broader product closure (all INST ACs).

Implementation checkpoint: Mac package generation, receipt/environment
discovery, exact core/file checks, transactional copying, reinstall and removal
are implemented. Twelve local controls pass, including ambiguous discovery,
concurrent-operation rejection, tampering and injected copy-failure rollback.
An actual generated unsigned `.pkg` was expanded and its embedded installer
installed retained `f10e70ee5` ARM binaries into a temporary prefix; the existing
four-tool public-provider smoke and reinstall/removal pass. This is not yet a
native Installer.app, signed/notarized or offline Gatekeeper pass. The new
`llama-installer-qa.yml` reuses retained binaries for native Mac installer QA.

Adrian then raised Windows users installing both CUDA and Vulkan and approved
keeping both separately with one explicitly active. The Windows installer stores
complete verified variants under `.llama-backends/<backend>` and projects only
the active variant into the existing canonical runtime paths. Use hard links on
the same volume where supported, with a verified copy fallback; do not duplicate
CUDA files unnecessarily or change the runtime/ABI. This does not enable both
backends in one process. Existing separately packaged native applications retain
their own dependencies and are unaffected by this installation's selection.

The installed `crexx-llama status`, `crexx-llama use vulkan` and
`crexx-llama use cuda` commands are the switching surface. Installing a second
variant preserves the current selection unless its activation option is chosen.
Switching verifies core/variant identity and hashes, serializes installer work,
refuses in-use files, and rolls back failures. A system-wide install requires an
administrator terminal. No model download or environment-variable change occurs.
Removing the active backend leaves no active plugin until the user selects
another installed variant; it must not silently switch. INST-AC-02/03 additionally
require coexistence, explicit switching, failed/in-use switching, independent
removal and unchanged core/model controls. Implement within INST-01–03; retain
all other criteria and signing/offline gates.

**cREXX switcher requirement, 16 September:** Adrian requires the shared
switcher to be a cREXX program, supplied by either Windows plugin installer.
The draft PowerShell command wrapper is superseded; a cREXX wrapper that merely
delegates the whole switch to that script would not satisfy this requirement.
Keep command parsing, backend selection, manifest/ownership validation and
transaction orchestration in cREXX. Reuse existing libraries, with narrowly
scoped native filesystem/locking primitives only where the operating system
requires them. Keep installation-specific registry/elevation/signing actions
in the native installer. Both installers supply the same versioned management
program, preserve it while either backend remains, and never overwrite a newer
incompatible management format silently.

The approved delivery is the standalone native `crexx-llama` executable written
in cREXX, plus its source. Adrian explicitly authorized replacing PowerShell
before publication. Either backend installer supplies it; recipients need
neither a C compiler nor a first-run compilation cache. No core-driver dispatch
change is required. The current `crexx` driver compiles
ordinary sources and requires `--args` for program arguments; it has no automatic
`crexxsaa` source-cache delegation. `crexxsaa_run_source()` does cache hosted
source, but the `crexxsaa` executable is a cache-maintenance tool, not a general
script runner. Do not introduce a new compilation cache for this small utility.

8. [x] **INST-AC-05:** the delivered switcher is implemented in cREXX and
   installed identically by either backend installer; installing/removing one
   variant preserves the shared tool while the other remains. Retain source,
   compiled-tool identity and native install/switch/remove proof with no Python,
   SDK or first-run compilation requirement. An installed tool must work even
   when no inference backend is active; it must not import `llama` itself.
9. [x] **INST-04:** replace the draft Windows shell switcher with the cREXX
   management program, reuse the existing compile/link/native-packaging path,
   and qualify the public command plus safe switching controls. Use the approved
   standalone native command. Serves INST-AC-02/03/05; preserve
   the completed Mac lifecycle evidence and separate signing/offline gates.

The PowerShell manager is removed. The native Rexx command and standard `rxfs`
additions are committed; actual unsigned Windows installer lifecycle passes in
the focused retained-artifact runner recorded below. Local native-manager
controls pass 14/14. The `rxfs` contract passes all four VM/optimization cells
in ordinary Debug and maintained Apple ASan; Apple LeakSanitizer is unavailable.
Retained inputs and logs are under
`docs/qa/native-inference-ci/local/native-manager/`.
Windows native install/provider/reinstall succeeded, but removal exposed a
reinstall error: the manager ignored the returned `jsonmembers` count on a reused
array and retired stale entries. The application fix at `182516b4a` has a
failing-before/passing-after regression and complete payload lifecycle proof
locally. Focused manual Windows run `35119980116` passes both actual Vulkan and
CUDA installers using retained binaries. See the pipeline evidence ledger and
closure below. INST-04 is complete; signing/release acceptance remains open.

INST-04 implementation sequence, amended at Adrian's request to fill gaps in
the main libraries: (1) implement CLI, JSON/hash verification, ownership and
rollback in Level B using existing `rxjson`/`rxhash`; (2) reuse `rxfs` and add its
missing reusable filesystem primitives and an owning `fileguard` class for
nonblocking exclusive locks/Windows file leases. Use native RXPA factories and
finalization, not raw process-global handles or an installer-private provider.
Ordinary copies share the guard; explicit close releases it for all aliases;
last-reference/VM teardown releases any remaining OS handle. Locks stay VM-local.
The filesystem provider owns no backend-selection/installation policy and
introduces no runtime ABI. (3) compile/package this utility with the expanded
static `rxfs` against the retained core runtime and SDK, replacing every Windows installer call to the
PowerShell helper; (4) retain focused mutation/failure controls and actual
Windows native installer/provider lifecycle evidence. The packaging toolchain
may use Python/PowerShell on runners; the delivered switcher must not invoke
them or require them. Only the small filesystem provider/tool are rebuilt for
this focused check; no engine/VM rebuild or model download follows. Normal core
builds will acquire the library additions through the existing `rxfs` targets.
Retain ordinary and sanitizer-focused `rxfs` lifecycle controls (including
copy/close/finalization and contention) separately from Windows installer QA.

Hosted Mac run `35109939178` at `68f4a7564` passes actual native core/plugin
installation, installed public-provider smoke, reinstall and removal on both
ARM and Intel. Its final core-only demonstration fails because the harness
tries to compile beside a root-owned installed example. This is a reproduced QA
working-directory error, not an installer/provider failure. Copy the example to
a writable directory for that check; the core installer's completion text needs
the same correction. Rerun only this retained-artifact installer workflow. Signed
and offline Gatekeeper proof remain separate; no product/engine rebuild follows.

**Mac native lifecycle qualified:** the corrected focused run
[35110318683](https://github.com/adesutherland/CREXX/actions/runs/35110318683) at
`be8fbf4e5` passes on both ARM and Intel Mac. Each runs 13 focused controls and
actual core/plugin `.pkg` installation, installed compile/assemble/link/provider
execution, reinstall, plugin-only removal and core execution afterward from a
writable directory. Evidence and checksums are retained under
`docs/qa/native-inference-ci/remote/be8fbf4e5-installers/`; product binaries are
unchanged retained `f10e70ee5` artifacts. No product/engine/CUDA build, model
download or sanitizer campaign was run. The Mac portions of INST-AC-01–03 now
have native runner proof; Windows was still open at that checkpoint and is
qualified below. INST-AC-04 remains open. The code and evidence
are on the candidate branches; no develop promotion or release is authorized.

**Windows native Rexx lifecycle qualified:** focused manual run
[35119980116](https://github.com/adesutherland/CREXX/actions/runs/35119980116)
at `182516b4a` passes the native rxfs contract and 14 manager controls, plus
actual Vulkan/CUDA installer coexistence, preserved active choice, SDK-free
native command switching, installed four-tool public-provider smoke for each
variant, active reinstall with all declared hashes intact, independent backend
removal and shared-tool removal only after the last backend. Core/model/environment
preservation, core execution afterward, and backend registration cleanup on
core uninstall also pass. Evidence and checksums are under
`docs/qa/native-inference-ci/remote/182516b4a-installers/`. The run identifies the
Windows compiler/linker and source revision; local compiled-tool hashes and
before/after controls are retained under `local/native-manager/`. Future workflow
artifacts additionally retain the generated Windows manager manifest.

This closes INST-01/03/04 and the unsigned functional INST-AC-01/02/03/05.
INST-02 and INST-AC-04 remain open for the release signing/publishing path and
real offline Gatekeeper proof. CI-06/F19 and parent hardware/model qualification
remain separate open work. Retained product binaries are still `f10e70ee5`;
the source additions enter the next ordinary core build through standard rxfs.
No engine/VM rebuild, large-model download, broad sanitizer repeat, develop
promotion or release publication was performed for this closure.


### Approved final-delivery execution — 16 September

The clean merge `85a9f5cffdbbec31a816ab9ad347e658f1fd6b40` includes
`origin/develop` through `94f2f228c` (#699). No merge conflicts or compiler
redesign were required. Normal product build and 20 focused import/RXPA/rxfs
checks pass. The package smoke initially found stale local staged provider
hashes; rebuilding its declared prerequisite target resolved those. It then
exposed a CTest registration omission: the default `cpu` expectation rejected a
Metal-enabled package. ReleaseSmoke.cmake now passes configured backends, as
hosted callers already did. The unchanged workload passes in 26.72 seconds;
serial scheduling and its wide timeout remain intact. No engine behavior changed.

F19-AC-01/F19-01 are verified: 37 release/publication/signing controls pass,
including actual collector scripts with complete and missing-input sets.
Routine push/PR/snapshot selection excludes CUDA; stable/beta tags require both
CUDA variants, and manual branch dispatch respects explicit lane selection.
Snapshot refresh removes obsolete CUDA/signed derivatives; complete release
collection explicitly requires all six named plugin ZIPs.

F19-02/INST-02 implementation adds final package/manager hash refresh and a
non-publishing paired Windows signer, retains one native manager artifact for
both installers, and enables explicit candidate Mac signing/notarization.
SimplySign's initial CKR_FUNCTION_FAILED probe was retried after Adrian logged
in; real Authenticode signing and verification then passed. This credential
probe is not full signed-delivery qualification. A dedicated disposable-host Mac
check installs and runs the signed/stapled payload with active network interfaces
down, restores them in `finally`, and has an independent restoration watchdog.
Actual signed/offline results remain pending the combined candidate run.

The Windows signing key remains local. Signed-artifact runner QA uses a
temporary **private draft** staging release named `qa-llama-signing-<commit>`
solely for artifact transfer; never publish it or create its tag. The runner
checks draft status, target commit, GitHub digests, signed-output digests and
the exact unsigned input archives from the qualified Build run. It then verifies
Authenticode on setup/payload/uninstallers and exercises the existing installed
provider/coexistence/reinstall/removal workload. Delete the staging draft after
retaining terminal evidence. This is QA transport, not a user release.

Evidence: `docs/qa/native-inference-ci/local/final-delivery/`. Reuse #699's
retained full normal/focused ASan and the unchanged previous Deep/core sanitizer
results. No develop promotion, release publication or hardware waiver occurs.

**Signed Mac packaging repair, 16 September:** Build `35143588581` passes ARM
Metal package smoke, then fails constructing its signed installer: `codesign -R`
treats `anchor apple generic` as a filename. Native `codesign` documentation
requires `=anchor apple generic` for literal requirement text. A permanent native
control reproduces the failure with Apple's signed `/bin/ls`; the correction
passes that control and still rejects an ad-hoc signature (14 controls pass).
This is an installer verification invocation defect, not an inference failure.
F19-02/INST-02 remain open. Retry only retained-artifact packaging: accept a failed
source Build for this explicit retry only if all core/plugin smoke jobs passed
and its only failed steps are the named Mac installer packaging step. Require
terminal source metadata and retain that validation. Repackage/notarize/staple
the unchanged signed Mac payload in the installer QA workflow, then perform the
same offline check. Do not rebuild core/engines or call the original run green;
the terminal packaging retry must be recorded as its replacement evidence.

**Packaging retry `35149300574`:** both Mac packages now pass real signing,
notarization, stapling and online Gatekeeper. Both offline QA hosts correctly
disable networking, reject the unsigned control and validate the core signature,
but the harness incorrectly invokes `stapler validate` while offline. Apple's
installed `stapler(1)` documentation explicitly requires Internet access for
validation against the latest ticket; both logs show the expected CloudKit
network error 68. Move that online comparison after the offline proof. Retry on
fresh hosts using these already signed packages, without online Gatekeeper or
ticket validation beforehand, to avoid warming their notarization cache. Retain
quarantine, unsigned rejection, actual offline assessment/install/execution and
network restoration. INST-AC-04 stays open until that full proof passes.

### Development promotion and light monitoring approved — 16 September

Adrian authorizes merging the sufficiently qualified candidate into `develop`,
then monitoring its normal automatic checks lightly and fixing failures until
green. He explicitly confirms that **Windows signing is an optional final
distribution step done later**, not a development integration or testing gate.
Do not request further signing login tonight. This supersedes the earlier
no-promotion boundary; it does not authorize a public version tag or full release,
waive known sanitizer defects, or close the parent real-device/model criteria.

Vision: integrate the working four-core/six-plugin delivery and native installers
into ordinary development, with normal CI feedback and truthful signing status.
Avoid repeated Deep/sanitizer/CUDA work when unchanged qualified evidence exists.

1. [x] **PROM-AC-01:** all compiled delivery lanes and relevant integration tests
   have retained passing evidence. Build `35143588581` passes four cores, MinGW
   and all six plugin smoke checks. Its Mac packaging-only failures are repaired:
   `35149300574` signs/notarizes/staples both plugin installers; fresh-host
   `35149778131` passes offline Gatekeeper/install/consumer and subsequent ticket
   checks on ARM and Intel. Installed Linux rxfs passes `35149445867`.
2. [x] **PROM-AC-02:** promote without overwriting newer develop changes; retain
   the exact promoted commit and inherited qualification boundaries.
3. [ ] **PROM-AC-03:** the promoted revision's normal automatic Build and CodeQL
   checks reach green, with any actual failures diagnosed and repaired using
   focused regressions. Do not dispatch extra overnight assurance by default.
4. [x] **PROM-AC-04:** optional Windows signing is visibly separate from
   integration acceptance. The candidate's local signed archives/installers
   completed after reconnection; actual signed installer QA remains a separate
   follow-up. Future develop signing may wait until morning without blocking CI.
5. [x] **PROM-01:** reconcile the accepted evidence and promote the clean
   candidate to develop (PROM-AC-01/02/04).
6. [ ] **PROM-02:** follow the automatic runs at a light cadence, repair failures,
   retain final results and stop monitoring when green (PROM-AC-03).


Promotion completed: `8ed983afdc2dad723b14f7a84d985c1c5ea88b32` fast-forwarded
remote develop from `94f2f228c` on 16 September, preserving all history and no
concurrent changes. Normal automatic Build `35150686647` and CodeQL
`35150686250` are running. PROM-AC-03/PROM-02 remain open until terminal green.
The thread heartbeat `qualify-llama-develop-integration` checks every 15 minutes,
remediates actionable failures and stays quiet on unchanged status. Local final
Windows signing succeeded; its optional retained-artifact installer QA and
private draft cleanup continue independently, without gating this promotion.
