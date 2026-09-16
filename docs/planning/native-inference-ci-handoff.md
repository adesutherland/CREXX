# Native inference CI restart handoff — 15 September 2026

Status: active qualification on `origin/temp/llama-release-combined`. The
authoritative [pipeline plan](native-inference-ci.md) retains the complete
vision, acceptance criteria and implementation steps. The older snapshots below
are chronological evidence; this live section supersedes their pending actions.

## Live continuation — 16 September

**Installer addition (CI-D04):** Adrian authorized implementing separate Windows
and Mac plugin installers now, locating/checking an installed matching core.
The canonical criteria are INST-AC-01–04 / INST-01–03 in the pipeline plan.
Mac local lifecycle/rollback and installed-provider controls pass; native hosted
installer and signed/offline Gatekeeper proof remain open. Mac concern explicitly
means Gatekeeper/notarization checks: staple the notarized package; never promise
macOS makes no Apple network requests. Windows CUDA/Vulkan coexistence is a
pending user choice (both stored/one active versus remove before switching).
Do not silently overlay their colliding filenames or select an answer.

**New approved release policy (CI-D03):** Vulkan is the general Windows/Linux
download; CUDA remains an optional separate plugin. Every full release build,
including beta, must build/package/smoke both CUDA variants against its exact
qualified core, with valid compiler-cache reuse. Otherwise CUDA runs only on an
explicit manual GitHub Actions request. Ordinary pushes/PRs/development snapshots
do not select CUDA, even for relevant input changes. Smoke describes the bounded
checks inside the selected CUDA build, not another trigger. This supersedes the
earlier automatic relevant-change proposal and is recorded in the authoritative
plan and human/agent guides; workflow selection is not yet changed. CI-06 and
CI-AC-10 remain open. Preserve the current candidate's complete qualification.

**Current integration candidate:** `2b897caa55fbe564c6d12966db6fea349f327986`
on `origin/temp/llama-release-combined`, merging repaired develop
`17e844441ed87e1f6e0d5f1f0d3bb4bee8db6187`. Adrian explicitly requested the
merge and affected retest. The prior policy/roadmap documentation is preserved
in `884247264`. Two overlapping MSVC test-registration conflicts preserve this
branch's capability-based VM selection and optional runner list.

**Bounded retest:** the existing plan's F18-RAC-01–03 / F18-R01–03 own this
integration. The nine qualified launcher/file-open/worker diagnostic and
regression source blobs match the hotfix exactly; identity evidence is under
`local/f18-merged-retest/`. Reuse hotfix Child Inheritance QA `35088226165`
(Linux/macOS/MSVC/MinGW) and focused Debug/Apple-ASan evidence for those
unchanged mechanisms. The combined tree rebuilt successfully and all nine focused ordinary Debug
checks pass (91.90 seconds), including the project-build contract. Manual [MinGW core run 35104282036](https://github.com/adesutherland/CREXX/actions/runs/35104282036)
passes the original F18 environment with llama disabled: 155/155 core smoke,
three KeyAccess checks and extracted-package/relocated-native smoke. The
project-build contract passes in 31.59 seconds; source identity and terminal
artifacts are retained under `local/f18-merged-retest/` and `remote/2b897caa5/`. No CUDA/backend,
Deep or full sanitizer run was dispatched for this retest. Preserve deliberate
worker concurrency; no runtime redesign is introduced.

**Previous candidate evidence:** at `f10e70ee5`, Build `35076278681` attempt 2,
Deep `35076281099` and core Sanitizer `35076283269` are all terminal success.
Build includes all six plugins; Deep covers five comprehensive configurations,
stress, Release jobs 1/5/30 and RXBIN equivalence. These remain valid historical
results on that SHA, not new full qualification of `2b897caa5`.

**Defect disposition:** the separate hotfix session reproduced and repaired
POSIX resource inheritance and Windows unrelated-handle inheritance/blocked
rename, preserving standard streams and adding worker publication diagnostics.
Its authoritative record is `issue-701-resource-inheritance.md`. This closes
the reproduced mechanism and its integration checks; it does not prove
that mechanism caused the original silent F18 run. GitHub #701 still has no
posted closure update; do not post one from this task.

**Retest complete:** F18-RAC-01–03/F18-R01–03 and F18-AC-02/F18-03 are checked
with retained evidence. No more testing is needed for this bounded request.
Parent real-device/model criteria and CI-D03 workflow cadence remain separately
open; do not repeat unchanged broad tests or infer develop/release publication
from this integration result. Documentation-only closeout reuses the tested
`2b897caa5` runtime/test/build inputs.

**Earlier repair evidence:** `f749203a7` passes the complete Windows Vulkan
run `35073667866`, including relocated native execution. At `31d4974f1`, all
five cores and Deep configurations pass; ARM Metal, Intel CPU, Linux Vulkan
and Linux CUDA archives pass. Both Windows plugins stop only at the duplicate
manifest-entry relocation defect repaired in `f749203a7`. Linux CUDA records
2,726 hits/four misses and Windows CUDA 2,640 hits/five misses, with one/three
cache write errors retained. Intel CPU excludes Metal and the fixture. F16,
D02 and F15's targeted repair proof are closed; final Windows CUDA and combined
delivery remain open.

**Sanitizer boundary:** SAN-009 is closed by the retained `21666b6bc` broad
core results and separate Linux first-party bridge probe. The superseded
`35071054942` run was cancelled, not passed. That closure does not replace the
current final-candidate sanitizer gate or establish upstream/real-device/model
qualification. Parent acceptance and unavailable hardware cells remain open.

## Live continuation — 15 September, after restart

**Latest packaging repair:** code HEAD `99a28ce7d` is on
`origin/temp/llama-release-windows-retry`; Build `35031332714` retries Windows
Vulkan with both core gates first and a tiny package preflight before the engine.
CI-F15 reproduced the scanner inspecting original SDK/source locations after
copying files into the package. The writer now scans those published roots;
no conflict/missing-dependency checks are weakened. Normal/ASan-built tiny
fixtures pass both complete-package and missing-dependency controls in 0.09 s.
The prior Windows Vulkan run compiled every plugin/helper binary successfully,
then stopped in this scanner. No new runtime/API change was required.

The paragraphs below retain earlier exact retry heads; this packaging-only
change does not invalidate their unaffected core runtime results.

**Newest source / focused retries:** local HEAD is
`3aca2dd2797ace911ff0eab8e48db41376687ced`, pushed to
`origin/temp/llama-release-windows-retry`. CI-F11's helper rename is in
`0be70cd69`. CI-F14 required both private C11 and MSVC atomics settings:
`23fd6b8e0` alone was incomplete; `b965c0716` adds the atomics switch and
`3aca2dd27` accepts CMake's `-std:c11` spelling in the preflight. The quick
real-MSVC negative/positive compiler gate passes both objects in
`35030598584`; comprehensive Windows QA is now running. Earlier retry
`35028634025` retained the missing-atomics error; `35030328193` stopped only
in the preflight's flag-spelling assertion before any broad build.

Windows Build `35028632018` remains on `b4d10adfb`, with both MSVC and MinGW
cores green and Windows Vulkan building. Its unchanged production/helper inputs
remain useful. These are focused retries, not a complete new exact-head gate.
Normal/Apple-ASan attached-provider controls pass 2/2 in 2.30/3.07 seconds;
the latest extra flag is MSVC-only. Linux first-party probe `35027428540` passes
with leak detection enabled and an ordinary verified engine. SAN-009 still
awaits the broader supported gate. No develop/release publication is authorized.

**Current candidate:** `21666b6bcf0c5a14986652ef0b0a76bddc43611b` is committed
and pushed. Integrated [Build run 35025115905](https://github.com/adesutherland/CREXX/actions/runs/35025115905)
was dispatched with `lane=all`. All four shipped core jobs and the non-shipping
MinGW gate pass on this SHA. Six plugin jobs now consume the matching core
artifacts from this same run. ARM Metal and Linux Vulkan plugin packages pass.
Windows Vulkan stops in the QA helper's CI-F11 macro collision; its production
bridge compiled. Linux CUDA passes (760,721,716-byte plugin ZIP; 23.494 s smoke). It records
1,476 cache hits/1,254 misses and one write error: useful partial reuse, with
full warm evidence still pending. Windows CUDA remains building on the older
head, which still contains the known helper/scanner issues; preserve its cache
work until terminal. Intel Metal fails only its serial permanent
probe after the unchanged 1,800-second backstop; the other three checks pass.
The bounded archived-bridge replay captures the same Metal compiler-service wait.
Wider core [Deep Build 35026467548](https://github.com/adesutherland/CREXX/actions/runs/35026467548)
and [Sanitizer QA 35026470003](https://github.com/adesutherland/CREXX/actions/runs/35026470003)
are running on this head with llama disabled. Deep Linux, ARM and MinGW pass, including external-consumer checks. MinGW
passes 2,248 comprehensive tests (881.48 s) and three qualification checks
(22.72 s), with both VM modes represented. MSVC stops at CI-F14 and has its focused retry. Intel also passes all
2,326 comprehensive tests and three qualification checks.
Stress and the 1/5/30-job product equivalence checks pass. Do not publish or move
`develop`.

Next: retain the integrated core/plugin results, diagnose any concrete failures,
complete CUDA cache evidence and the required wider core QA on the final
candidate. CI-F07's Intel Metal compiler-service wait and SAN-009's supported
Linux first-party closure remain open. CI-F11's private-buffer repair still
needs actual MSVC plugin build proof. Real-device/model/provenance criteria in
the parent plan are unchanged and are not closed by a fixture pass.

Direct Intel GGML-only registration passes three starts in `35026224152`
(`1a2d3dbc8`): 53.938/0.266/0.233 seconds, no bridge/VM/model present. This does
not reproduce or close CI-F07. A follow-up matching production's CPU-first
registration is `35026657758` at `da1a430b8`, using the same archived libraries
and opt-in capture. It also passes three Intel starts (56.737/0.312/0.249 s);
its local ARM control passes. CI-F07 remains unresolved. Archived-bridge replay `35029475651` at `50b7bf8fa` reproduces the wait
without a cREXX VM/compiler/model. Native-C direct GGML (quiet/default logger),
Python direct GGML and the archived bridge all pass the local ARM control.
The same-host Intel comparison `35030619064` at `f29a8d3b9` passes all four
controls in 77.110/0.284/83.992/0.498 seconds. The bridge follows earlier direct
initialization, so this does not close its intermittent cold-start failure or
prove a speed comparison. Captured failing stacks remain the evidence of the
Metal compiler-service wait. No production Metal change has
been made. Diagnostic changes remain on `temp/llama-release-intel-native`,
outside the candidate pipeline.

**Latest completed gate:** all five core configurations are green. MinGW
`35023122646` at `aade0fd0f` passes 152 core tests, three KeyAccess controls and
twelve extracted-ZIP commands covering `rxvm` (verified `rxtvm` copy), `rxbvm`,
both optimization modes and relocated native execution. Only the 30,211-byte QA
artifact was uploaded; no MinGW binaries. CI-D01-01, MINGW-AC-01/02, CI-F12 and
CI-F13 are closed. Plugin qualification is now authorized to proceed using the
prepared split pipeline. The paragraphs below retain the intervening history.

**Current checkpoint:** Windows/MSVC run `35022668868` is green at
`aade0fd0fe8ae97f5130383165bcf1b835145903`: 148 core tests, three focused KeyAccess
controls (including opt/noopt), and the actual extracted ZIP/native smoke.
Archive SHA256 is `ca62526dfed26f1c0d4b1c60ce86a998179e6f538111195a27b93490ba1cbcec`
(21,836,575 bytes). Linux and both Mac core gates remain green at `9caac6dcb`.
MinGW run `35021842079` passed 152 core tests but hit the same CI-F13 packaging
lookup already fixed; `35023122646` retries MinGW on `aade0fd0f` and is still
building. No plugin job has started. Await MinGW's extracted/both-VM proof before
dispatching inference builds. The main split-pipeline preparation is ready for its candidate commit and
integrated run; do not mistake static checks for hosted plugin qualification.

The prepared main Build workflow now has four core jobs, a reusable non-shipping
MinGW gate, and six dependent plugin jobs consuming exact matching core ZIPs.
Core jobs run core QA; plugin jobs build six provider/helper targets and run
four focused lifecycle/path controls plus the combined-archive fixture smoke.
No core compile commands appear in those plugin targets' local Ninja graph.
Twenty-five artifact/signing/matrix controls and actionlint pass. The new
`--payload` core finalizer passes its real archived-ARM-core harness; no fresh
core build is claimed by that local test. Plugin staging/finalization preserves
core bytes and signed-provider hashes; synthetic ZIP controls pass, but actual
plugin builds remain gated. Publication collectors now require four core plus
six plugin ZIPs; copies retain the exact tested ZIP bytes. Core installers and
existing signing/notarization safeguards remain. MSVC Vulkan has a pinned,
hash-checked LunarG SDK preparation script; only metadata/HEAD were fetched,
not the SDK, and it has not run on Windows yet. The wider core matrix now covers both MSVC and MinGW with llama disabled.
Commit the prepared pipeline and dispatch its integrated candidate run; do not
publish or move develop while its required gates remain open.

**Latest addition:** Adrian also requires a non-shipping Windows/MinGW core
quality gate, including both VM variants. Retain logs, not distributable binaries.
It is a fifth configuration alongside the four release cores and must pass before
plugin qualification. MSVC remains the one Windows release base. See MINGW-AC-01/02
and MINGW-01/02 in the authoritative plan. The Windows/MSVC repair retry is
`35021253099` at `7425ff241`; MinGW dispatch follows this workflow update.

**Latest authority supersedes the older continuation bullets below:** Adrian
approved separate core/plugin downloads and one MSVC Windows base. First build
and qualify llama-free cores on all four target platforms; only after all pass
resume plugin work. Plugin jobs must consume the already-built exact core and
run llama-only QA, avoiding duplicate core compilation/tests. Core sanitizer
jobs use `ENABLE_LLAMA=OFF`, no CUDA and no upstream engine instrumentation;
first-party adapter checks are separate with an ordinary engine where practical.
Build `35013130021`, sanitizer `35009830830` and MSVC diagnostic `35017440732`
were cancelled to implement that sequence. Preserve their evidence; cancellation
is not a pass. `.github/workflows/release-core.yml` and
`scripts/package-core-release.py` implement the new initial core gate; their
hosted result is recorded below. No further approval is needed for CI-D01.

Core-only run `35018374738` at `9caac6dcb47dbacc9f8011ab0f9a2e53acf5feb2`
is on `origin/temp/llama-release-core`. Jobs: Linux `104547463711`, Intel Mac
`104547463876`, Windows/MSVC `104547463919`, ARM Mac `104547463953`. All configured
successfully. Linux and both Mac jobs pass all 161 checks and the extracted
core ZIP consumer smoke. Windows compiles but passes only 146/147 checks:
`keyaccess_test_noopt` aborts after compaction. CI-F12 in the authoritative plan
owns the MSVC replace-file and null-stream cleanup repair. The new ordinary
failure control reproduces two null closes before repair; the repaired Debug
control and both Rexx modes pass. Matching Apple-ASan also passes all three controls (0.73 s). A Windows-only
retry is next. No inference build should start yet. The workflow now permits a
single platform on manual dispatch, avoiding unnecessary passing-platform reruns.

Post-core-checkpoint preparation in the main worktree: Deep QA now explicitly
disables llama; installation/human/agent guidance describes the approved split;
`scripts/package-llama-release.py` stages only provider files onto an extracted
qualified core, verifies identity and unchanged core hashes, emits a separate
plugin ZIP and re-extracts both ZIPs for consumer testing. Six new packaging
controls pass (20 total release-related controls pass). **The plugin packaging
script is not yet wired into Build CREXX or target-platform tested.** That work
must replace the old combined-product build/QA and six-complete-ZIP publication
assumptions, preserve signing/installers, and provide the Windows MSVC Vulkan
SDK. Do not describe release packaging as completed yet. No plugin build/test
should start before the four core jobs pass.

Intel replay `35016898785` now has a captured stack: our consumer is waiting
inside GGML Metal library initialization and Apple's synchronous Metal shader
compiler service. Retained evidence: `remote/3875270c6/intel-native/`. This
locates the stall but does not yet prove its underlying driver/service cause.
Leave this plugin investigation until the core gate passes.

- Current committed local/origin-combined source: `76df02be3c4d89118bd8d4e4e68308c47f76366e`.
- Build `35013130021` at that SHA passes all four standard packages. Linux and
  each Mac pass 161 fast checks; Windows passes 151. All pass 15 package commands;
  Windows has 14 restricted-PATH executions. All four downloaded archives were
  inspected; evidence is under `remote/76df02be3/` (new evidence not yet committed).
- Linux/Windows CUDA jobs in that run are the first sccache-populating builds.
  Cache setup succeeded on both and GitHub already contains compiler objects;
  cold statistics and a warm run remain required. Old MSVC/CUDA `34998653921`
  now fails adapter compilation due to the SDK `small` macro (CI-F11).
- Older Deep `35007946063` is terminal: Linux/ARM pass 2329+4; Intel passes 2329
  then 3/4 qualification tests; Windows passes 2250/2251. Windows lifecycle
  missing-DLL startup is fixed in `76df02be3` with per-test PATH lookup and local
  Debug/ASan probe proof (0.71/0.82 s), but Windows comprehensive retry is pending.
- Intel Deep's failure is the final relocated native smoke process timing out
  at 1800 seconds after all preceding commands pass. Same unchanged workload
  separately passes in the new Intel package (188.948 s). Cause remains CI-F07,
  unclassified; no sanitizer diagnostic. Do not dismiss it because the retry passes.
- `release_smoke.py` adds opt-in `--capture-command-after` to extend
  existing macOS sample-and-stop diagnostics beyond just the engine. Normal
  execution/guards are unchanged. A diagnostic stop remains failure. Local
  normal/Apple-ASan smoke targets pass (27.866/45.068 s); a deliberately delayed
  compiler confirms stack capture and diagnostic failure. Evidence is under
  `local/command-capture-*`. Isolated Intel native replay uses the already-built
  archive on branch `temp/llama-release-intel-native`, commit `3875270c6`, run
  `35016898785`. No engine build or ordinary full QA replay is selected.
- CI-F11 repair `935e9bbfd` renames only the private buffer `small` to
  `token_piece`. Identical actual-TU macro control fails before and passes after;
  the bridge builds in normal Debug and Apple ASan. Branch-only MSVC object
  check `temp/llama-release-msvc-compile` at `63a6e1004` verifies the target
  compiler without rebuilding the CUDA engine. Hosted confirmation is pending.
- Adrian challenged the repeated failures. Keep the next work narrow: MSVC
  adapter compilation and Intel stack diagnosis. Preserve useful active jobs;
  do not start another broad qualification cycle before these are understood.
- Sanitizer `35009830830` at `0f35b970e` is still running Linux/Mac full QA;
  Linux leaks are ON. SAN-009 stays open. Inspect failures before unrelated work.
- **Pending user decision:** CI-D01 is drafted in the existing pipeline plan.
  Adrian asked about separating the already-built plugin, one Windows base for
  Vulkan/CUDA, and MSVC Vulkan. Pinned upstream supports MSVC Vulkan. Proposed
  delivery is separate core/plugin archives and one MSVC Windows base, retaining
  MinGW source/regression support. This changes the existing separate MinGW
  binary requirement, so an async approval question is pending. Do not infer
  approval from elapsed time or the user's performance correction.
- Adrian corrected the VM framing: accepted performance favours `rxbvm`; do not
  call portability a slowdown or reopen benchmarks. `performance/RESULTS.md`
  confirms the Apple scorecard direction; it is not a new MSVC/GCC comparison.
- Size comparison against current dev-snapshot: ARM Mac 19.9->22.1 MB, Intel
  20.9->29.8, Linux 26.2->49.7, Windows 25.4->57.6, Linux CUDA 786.2 MB total.
  Models excluded. Exact baseline/candidate sizes are retained. Splitting is
  proposed only; current running builds remain useful either way.

The older pause snapshot below is historical. Read the live ledger and pending
CI-D01 decision before acting on its earlier next steps.

## Scope and authority

Read `AGENTS.md`, the complete vision/criteria in
`docs/planning/native-inference-backlog.md`, and
`docs/planning/native-inference-ci.md` before resuming. Preserve all parent
OUT-01–05 / AC-01–14 and pipeline CI-OUT-01–03 / CI-AC-01–09 / CI-01–05.
The pipeline ledger is `docs/qa/native-inference-ci/README.md`.

The authorized work is complete ready-built optional llama.rexx binary delivery,
small fixture smoke, and wider qualification on separate origin branches.
No develop promotion, tag, snapshot or GitHub release is authorized by pending
or partial results. Remote develop remains `b5b827489`; local develop remains
`f9f87a8e8`. Local working branch remains `temp/llama-release-qa`; its remote
counterpart is older because useful CUDA triage was preserved there.

## Latest user decisions

1. Preserve public `rxvm`: relative selected-VM link on Unix/Mac, identical
   executable copy on Windows. GCC selects rxtvm; Clang/MSVC selects rxbvm.
   ZIP inspection and smoke now exercise/verify this actual entry point.
2. Ordinary source builds already default `ENABLE_LLAMA=OFF`; opt-in includes
   CPU, Metal defaults on for Apple, CUDA/Vulkan are explicit SDK options.
   Complete downloadable packages include llama.rexx. Standard Vulkan packages
   still contain llama.rexx and allow CPU fallback; no llama-free release ZIP
   has been requested. Models remain separate downloads.
3. Keep separate complete CUDA ZIPs. NVIDIA runtime/cuBLAS are downloaded as
   pinned binary redistributables; the expensive part is compiling GGML CUDA
   kernels for the full upstream portable architecture set.
4. Adrian explicitly approved: "Yes lets cache cuda engine." Implemented
   sccache in CUDA jobs only; do not remove architectures or require recipients
   to install NVIDIA SDKs. CI-AC-09 requires actual cold/warm cache evidence.
5. Fixture remains developer QA only, absent from user archives. A separate
   Cognitive example package remains a later idea.

## Committed changes since earlier triage

- `3c7f77048`: widen serial backend probe guard 30 -> 1800 seconds; same probe
  passes normal 0.66 s / Apple ASan 1.02 s. Not a repair/explanation of Intel's
  earlier 1800-second hang.
- Windows MinGW package at `4a0924ee1` passed restricted-PATH smoke and archive
  checks; loader/bootstrap and runtime scan fixes have real target evidence.
- `489559ec4`: GCC ASan QA header portability. GCC 13 omits installed
  `sanitizer/allocator_interface.h`; use the exact public C declaration when
  absent. Same allocation API, 32 MiB guard and model workload retained.
  Normal generation qualification 83.1685 s / Apple ASan 109.027 s pass;
  live allocation growth 1456 bytes. Supported GCC retry is pending.
- `0f35b970e`: retained GCC repair/CUDA evidence and truthful source/binary
  installation/status guides.
- `3a677be5a`: CUDA compiler cache. Pinned Mozilla action commit
  `fc920bf0ec8de6ee65d409111f7ec508035751ba` (v0.0.11), prebuilt sccache v0.16.0,
  explicit C/C++/CUDA launchers in the release matrix, GitHub cache enabled,
  ordinary compiler fallback for server I/O errors, text/JSON cache statistics
  uploaded with package QA. Explicit launchers prevent GGML double wrapping.
  `actionlint` and 14 publication/signing/matrix controls pass. No cache run or
  warm saving has yet been demonstrated.
- `c10b9115e`: retained wider/cache evidence. All indexed evidence hashes were
  checked against SHA256SUMS before commit; raw logs retain their original bytes.

## Remote runs to inspect on resume

1. **Windows MSVC/CUDA diagnostic** run `34998653921`, job `104481333824`,
   SHA `ae8c19681`, branch `temp/llama-release-windows`: last seen building.
   This old revision lacks later bootstrap/runtime-path/UTF-8 fixes. Keep its
   compiler evidence but do not claim final packaging closure from it.
2. **Deep Build QA** run `35007946063`, SHA `cabc668cf`, combined branch:
   macOS ARM job `104512381741` and Linux job `104512381775` are SUCCESS,
   including comprehensive + install/package qualification. Windows
   `104512381634` and Intel Mac `104512381773` were still running correctness.
   Release graph jobs 1/5/30, bytecode comparison and isolated stress all pass.
   Some graph/stress evidence is retained under `remote/cabc668cf`; download
   terminal comprehensive logs/artifacts and reconcile counts.
3. **Sanitizer retry** run `35009830830`, SHA `0f35b970e`, combined branch:
   Linux ASan/LSan job `104519136540` and Mac ARM ASan `104519136517` were running
   full instrumented build/CTest. Linux leak detection ON, Mac OFF because
   unsupported. This is the GCC missing-header repair retry. Do not repeat
   unchanged local full QA. Inspect any failures and first-party diagnostics.
4. Earlier sanitizer run `35007947861` at `cabc668cf`: Linux failed compiling
   the absent GCC allocator header (CI-F10), not a memory diagnostic. Mac was
   cancelled during build by the retry. Both raw logs/artifacts retained;
   cancelled Mac logs contain no observed sanitizer/compile finding.
5. Combined Build `35008896953` at `cabc668cf` was cancelled early after CI-F10
   required a follow-up candidate. Cancellation is not a pass. Branch-create
   Build `35007934305` only ran metadata/guards and skipped every package.
6. **No Build run has yet been dispatched for cache candidate `c10b9115e`.**
   Auxiliary branches are manual-only. When resumed and ready, use explicit
   Build dispatch (all six packages), retain cold cache statistics, then prove
   actual warm CUDA reuse on the same branch without confusing it with model
   performance. Final exact-candidate Build/Deep/Sanitizer gates remain open.

## Passing triage package/archive evidence

- Linux CPU/Vulkan `2bc56249d`: smoke 16.785 s, CPU/no GPU, ZIP 49,740,278 bytes,
  rxvm -> rxtvm, 31 manifest entries checked.
- Windows MinGW CPU/Vulkan `4a0924ee1`: smoke 13.483 s, 15 commands with all 14
  consumer executions on Windows-only PATH; CPU/no GPU. ZIP 57,562,461 bytes,
  rxvm.exe identical to rxtvm.exe; 29 manifest entries and seven bootstrap/runtime
  DLL copies checked; no Windows OS DLLs shipped.
- Mac ARM CPU/Metal `2bc56249d`: smoke 87.884 s, actual Metal, ZIP 22,143,164
  bytes; rxvm -> rxbvm, 16 entries checked.
- Mac Intel CPU/Metal diagnostic `e39916f2a`: entire smoke 224.287 s, actual Metal,
  ZIP 29,847,924 bytes; rxvm -> rxbvm, 29 entries checked. Engine finished before
  the diagnostic 120-second sampler threshold; no sampling or termination.
  Original `2bc56249d` 1800-second engine hang remains unexplained (CI-F07).
  Do not merge the diagnostic branch's workflow override into the main candidate.
- Linux CPU/CUDA `2bc56249d`: 161 smoke-tier checks plus final smoke 33.524 s,
  CPU/no GPU. Cold product build 1 h 54 m 30 s. ZIP 786,190,716 bytes, SHA256
  `c3a59f239e92e5e243009c5bdb68adc8a12837ef480218fbc8572fa99a61c184`;
  rxvm -> rxtvm, 34 entries (relative library symlinks resolved) and NVIDIA
  libraries/notices checked. cuBLAS/cuBLASLt ~854 MB and GGML CUDA ~191 MB
  uncompressed explain the size. All archives exclude fixture/helper.

Actual downloaded archives remain in `/tmp/crexx-ci-<sha>-<platform>` directories.
Archive inspections and their own size/hash (distinct from GitHub envelopes)
are retained in `docs/qa/native-inference-ci/remote/`.

## Remaining acceptance and next work

CI-AC-01/04/06 and CI-01/02 are checked. CI-AC-02/03/05/07/08/09 and CI-03–05
remain open. Inspect running jobs first, repair actual failures, finish CUDA
package/cache proof, then reconcile the final combined exact-head gates and
reviewable candidate. Do not silently narrow the plan to the latest cache work.

SAN-009 remains open and release-blocking until required supported Linux proof
and all closure checks are retained. No new sanitizer memory finding was observed
before pause. Parent real BGE/Smol Windows/Linux/CUDA/Vulkan device, memory,
driver/failure, provenance and final acceptance cells remain open. Original
approved model pins remain unchanged; generated fixture success does not close
those cells. No publication or develop promotion has occurred.
