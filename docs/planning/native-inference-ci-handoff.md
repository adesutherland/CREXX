# Native inference CI restart handoff — 15 September 2026

Status: Adrian resumed with "continue" after the restart. This is the historical
pause snapshot; the live pipeline ledger records subsequent work. Source/workflow changes were committed and
pushed to `origin/temp/llama-release-combined` at `c10b9115e` (resolve the full
SHA from git). This note is intentionally an uncommitted handoff. No new CI run
was dispatched after the pause request; existing remote jobs were left running.

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
