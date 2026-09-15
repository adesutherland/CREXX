# Native inference CI restart handoff — 15 September 2026

Status: Adrian resumed with "continue" after the restart. This is the historical
pause snapshot; the live pipeline ledger records subsequent work. Source/workflow changes were committed and
pushed to `origin/temp/llama-release-combined` at `c10b9115e` (resolve the full
SHA from git). This note is intentionally an uncommitted handoff. No new CI run
was dispatched after the pause request; existing remote jobs were left running.

## Live continuation — 15 September, after restart

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
