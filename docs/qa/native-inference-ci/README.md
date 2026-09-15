# Native inference binary delivery evidence

Live work package: [numbered pipeline plan](../../planning/native-inference-ci.md).
Candidate: `temp/llama-release-qa`. The accepted baseline is `12647a91aa7e9`,
merged with remote RXPP `b5b827489` by `c1de1670812a`. Publication is not authorized
by a partial or pending result.

Latest approved delivery sequence (15 September): separate core and plugin
downloads, one MSVC Windows base, all four llama-free core builds first, then
plugin-only builds/tests against the exact qualified core artifacts. Core
sanitizers exclude llama/CUDA; adapter checks remain separate and first-party.
Earlier combined jobs and the outstanding MSVC compile diagnostic were cancelled
for this sequence. Older proposed/pending wording below is historical; CI-D01
in the live plan records the approved contract. Core and plugin QA are separated
so backend variants do not rebuild or repeat the cREXX core suite.

## Local checks before remote qualification

| Check | Result | Retained evidence |
| --- | --- | --- |
| Isolated aggregate, normal Debug | Pass, 44.527 seconds | `local/measurement-debug/` |
| Same aggregate, maintained Apple ASan | Pass, 48.266 seconds | `local/measurement-asan/` |
| Registered serial CTest, normal Debug | Pass, 28.20 seconds | `local/registered-debug/` |
| Registered serial CTest, maintained Apple ASan | Pass, 45.57 seconds | `local/registered-asan/` |
| Public `rxvm` package smoke, normal Debug | Pass, 27.89 seconds | `local/public-entry-debug/` |
| Public `rxvm` package smoke, maintained Apple ASan | Pass, 44.03 seconds | `local/public-entry-asan/` |
| Workflow syntax and expressions | `actionlint` passes for Build, Deep Build and Sanitizer workflows. | Workflow definitions at candidate commit. |
| Publication, signing and matrix controls | 14 Python tests pass, including existing Windows publication guards. | `scripts/tests/test_*release*.py` |

Timing calibrates hang guards, not model performance. The isolated measurements
precede CTest registration. The registered aggregate has `RUN_SERIAL=TRUE`,
`TIMEOUT=3600`, the qualification tier and explicit prerequisite targets.
Child process guards are 1,800 seconds. Ordinary candidate packaging tests its
final staged payload once; the fixture and helper are never added to it.

The fixture is 4,763,872 bytes with SHA-256
`bb9b0debefdfec589f4c6d94c6bcc38daea64ab20b48eb95809e5c367863f5de`.
The raw engine checks two prompt rows, repeated-request isolation, finite logits
and embeddings, and two private contexts using one loaded model. Public-provider
checks cover configuration factories, device discovery, explicit rejection of
the fixture as an unqualified real model and cleanup in optimized/nonoptimized
VMs plus a relocated native application. Package controls cover corrupt CPU
hashes and a missing runtime manifest. No BGE/Smol weights are downloaded.

The initial test authoring run exposed a test-local Level G scope error: `state`
was declared inside a loop and read outside it as a symbol. Declaring it in the
enclosing scope made the intended assertion executable. No product or model
loader bypass was added. The successful logs above retain the exact checks.

The existing full local gate (2,349/2,349 Apple-ASan tests), normal Debug gate
(2,347/2,347) and real-model qualification remain in the STEP-05/06 ledgers.
Those results do not claim that new hosted inputs or platforms have passed.
Linux leak detection remains enabled in the forthcoming supported-host gate.

## Remote results

First candidate `544d41f02de50d97598b21fb4a09a8a05756b7e7` is on `origin`.
[Build run 34993532373](https://github.com/adesutherland/CREXX/actions/runs/34993532373)
superseded the duplicate branch-create run 34993528940 through branch-scoped
concurrency. Remote `develop` remained `b5b827489d781f9e42d305ef264a22d2c1c42cb6`.

1. **CI-F01 — Missing Vulkan SDK component:** ordinary Linux and MinGW Windows
   configure fail at pinned `ggml-vulkan/CMakeLists.txt:14`, requiring
   `SPIRV-Headers`. Add the explicit development package on both hosts. Logs:
   `remote/544d41f02/linux.log`, `remote/544d41f02/windows.log`.
2. **CI-F02 — Unsupported concrete VM target in a regression:** MSVC configures
   CUDA successfully but cREXX test generation fails at
   `compiler/tests/CMakeLists.txt:3794`, which unconditionally names `rxtvm`.
   MSVC supplies the switch VM only. Preserve the regression on `rxbvm` and add
   `rxtvm` only when `CREXX_THREADED_VM_SUPPORTED` is true. Compiler test
   registration precedes interpreter target creation, so an early `TARGET`
   existence check would incorrectly drop threaded coverage. The same two-mode
   regression passes locally in normal Debug (2.58 seconds) and maintained Apple
   ASan (4.60 seconds); both generated CTest commands retain both supported VMs.
   Evidence: `local/receiver-debug/`, `local/receiver-asan/`. Remote MSVC
   generation is the target-platform closure check. Log:
   `remote/544d41f02/windows-cuda.log`.
3. **CI-F03 — CUDA host toolset lacks cREXX C11 atomics:** the corrected
   MSVC regression registration configures successfully in run
   [34995568477](https://github.com/adesutherland/CREXX/actions/runs/34995568477)
   at `58b7c749d`. Compilation then fails at `rxvmintp.h:43`: MSVC 14.29 has
   no `stdatomic.h` and ignores `/experimental:c11atomics`. Select the runner's
   installed 14.44 (v143) toolset. The pinned NVIDIA archive's actual
   `crt/host_config.h` accepts `_MSC_VER` from 1910 through 1949; 14.44 is
   within that check and supplies the required C11 implementation. No
   unsupported-compiler override or runtime code workaround is added.
   Evidence: `remote/58b7c749d/windows-cuda.log` and
   `cuda-host-compiler-control.txt`. The target build remains the closure check.
4. **CI-F04 — MSVC aliases in filesystem/platform providers:** run
   [34996258242](https://github.com/adesutherland/CREXX/actions/runs/34996258242)
   at `2bc56249d` confirms successful CUDA/MSVC 14.44 configuration and VM core
   compilation, then fails linking `rxfs.rxplugin` because MSVC lacks `S_ISDIR`
   and `S_ISREG`. Add guarded Windows mode-bit definitions, matching the existing
   file-I/O provider pattern. The same log also reports `FILE *`/`int` mismatches
   at the platform provider's two `popen` calls: MSVC requires `_popen` and
   `_pclose`. Add those MSVC aliases; no filesystem or UI contract changes.
   The existing eight filesystem/platform optimized/nonoptimized VM controls
   pass in normal Debug (0.79 s) and maintained Apple ASan (1.18 s), following
   matching focused builds. Evidence: `local/msvc-portability-debug/` and
   `local/msvc-portability-asan/`. The target MSVC build remains pending.
   Log: `remote/2bc56249d/windows-cuda.log`.

   Source inspection also found that the C++ bridge includes Windows headers
   before its `std::min(...)` calls without `NOMINMAX`. Define that conventional
   guard before `windows.h`; this is a source-audit correction, not an error
   already reached in the hosted log above. Normal/Apple-ASan bridge/package
   builds and smoke pass (28.54/44.57 s), retained under
   `local/windows-macros-debug/` and `local/windows-macros-asan/`. Windows
   compilation remains the relevant platform proof.

   The same failed job returned `-1`; CMD's `if errorlevel 1` tests only values
   at least 1, so it incorrectly continued into `qa-smoke` and retried the failed
   link. The MSVC configure/build/check step now exits on any nonzero code.
   This corrects workflow failure handling without excluding a test.

5. **CI-F05 — C++20 UTF-8 path boundary:** MinGW GCC 16.2 in the same
   `2bc56249d` run defaults to C++20 and rejects `char8_t*` arguments to the
   engine's UTF-8 `char*` APIs. Preserve the encoded path bytes in an explicit
   `std::string` helper, shared by bridge and native packager. The packager also
   reads its expected JSON hash explicitly as a string to avoid a C++20
   comparison overload incompatibility. Both actual translation units now pass
   a local C++20 compile control; the original failures are retained under
   `local/cxx20-before.log` and `local/cxx20-provider-comparison-before.log`.
   Permanent ASCII/non-ASCII path controls run as C++17 and C++20. Both controls
   and package smoke pass in normal Debug (28.43 s total) and maintained Apple
   ASan (45.38 s total): `local/cxx20-debug/`, `local/cxx20-asan/`. Target MinGW
   build/smoke is still required. Hosted failure: `remote/2bc56249d/windows.log`.

6. **CI-F06 — Windows executable/backend dependency lookup (source audit):**
   the complete provider package puts DLLs and MSVC redistributables in
   `bin/providers`, while the compiler and public plugin copy live in `bin`.
   RXPA uses `LoadLibraryA`; pinned GGML uses `LoadLibraryW`. A full DLL path
   alone does not make its directory a dependency search location. The repair
   copies only bridge/engine core DLLs and their runtime dependency closure to
   `bin`; GPU DLLs remain in `bin/providers`. The bridge finds the installed
   package from that bootstrap copy and preloads verified Windows backends with
   `LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR`, retaining process-lifetime ownership.
   Native consumers retain adjacent dependencies/manifests. Consumer smoke now
   removes SDK/toolchain PATH entries; native compilation keeps its compiler
   environment. A test-only bootstrap inventory checks the declared root DLLs.
   No new public API, provider manifest schema or model qualification is added.
   Normal/Apple-ASan builds and package smoke pass (28.48/44.07 s), retained
   under `local/windows-bootstrap-debug/` and `local/windows-bootstrap-asan/`.
   These validate the unchanged Unix paths and harness; Windows execution is
   still the platform closure check. The earlier
   hosted compiler failures did not reach this boundary, so this finding must
   not be described as a reproduced Windows loader failure yet.

7. **CI-F07 — Intel Mac engine-helper hang, cause unclassified:** job
   `104473761974` in run `34996258242` at `2bc56249d` completes Release build
   and fast QA, then the first isolated engine process reaches its 1,800-second
   child hang guard. No helper marker or sanitizer diagnostic was emitted; the
   retained log only contains its launch command. This does not establish a
   model-performance regression, a scheduling-only failure or a sanitizer defect.
   Evidence: `remote/2bc56249d/macos-intel.log` and `macos-intel-qa/`.
   Add flushed engine stage messages, then replay the same fixture workload on
   an isolated diagnostic branch. Its explicit `--capture-engine-after` option
   may sample and stop a stuck macOS engine for diagnosis; that stop fails the
   run and cannot be reported as qualification. Ordinary smoke retains its full
   workload and wide backstop. The permanent cause/repair and platform closure
   remain pending. Normal and Apple-ASan package smoke with stage markers pass
   (27.84/43.55 s), under `local/engine-stages-debug/` and
   `local/engine-stages-asan/`. A deliberately delayed helper proves the
   diagnostic mode captures a nonempty stack, stops its own child and reports
   failure rather than success: `local/stack-capture-control/`.

8. **CI-F08 — MinGW runtime scan locations and Windows separators:** run
   [35000816874](https://github.com/adesutherland/CREXX/actions/runs/35000816874)
   at `473fb6894` now compiles and links the C++ bridge, engine and native
   packager, closing CI-F05's target compiler failure. It fails in
   `WriteProviderPackage.cmake` because `libgcc_s_seh-1.dll`, `libstdc++-6.dll`
   and `libwinpthread-1.dll` are not found. Pass the actual C++ compiler's
   directory explicitly to the build-time dependency scanner; consumer PATH
   remains restricted. The same log exposes mixed Windows path separators in
   CMake's default old CMP0207 behavior. Share a filter that handles both forms,
   opt into normalization when supported, and retain positive compiler/SDK
   controls so redistributables are not excluded as system files. The permanent
   path control fails with the old filter (`local/windows-dependencies-before.log`)
   and passes with the corrected shared filter. Both this control and package
   smoke pass in normal Debug (27.98 s total) and Apple ASan (43.50 s total),
   under `local/windows-dependencies-debug/` and `local/windows-dependencies-asan/`.
   Target Windows package results remain pending. Hosted evidence:
   `remote/473fb6894/windows.log`;
   [CMake path-normalization policy](https://cmake.org/cmake/help/latest/policy/CMP0207.html).

9. **CI-F09 — Delayed push event overrides a diagnostic selection:** manual
   Windows run `35003939480` at `4a0924ee1` was cancelled by push run
   `35003947618`, whose event arrived five seconds later and selected all six
   lanes. The new selected retry is
   [35004301149](https://github.com/adesutherland/CREXX/actions/runs/35004301149).
   Restrict automatic candidate pushes to `temp/llama-release-qa`; auxiliary
   `temp/llama-release-*` branches remain available for explicit manual lane
   selection. Develop/master/tag safeguards and branch-scoped cancellation are
   unchanged. `actionlint` and the 14 publication/signing/matrix controls pass.
   Pushing `78308cf38` then starts no automatic run and leaves selected Windows
   run `35004301149` active; retained branch/run API records are in
   `remote/78308cf38/`. CI-AC-06 is checked again after this live control.

Adrian reaffirmed the public `rxvm` entry-point contract during triage. Package
smoke now calls `rxvm` and the alternate implementation when available, using
CMake's selected default so it does not duplicate the preferred VM execution.
Unix uses a symlink and Windows a copy; production selection is unchanged.
The follow-up archive inspection found Linux's `zip` invocation dereferenced
that link. Both candidate/release ZIP commands now use `-y`; macOS's existing
`ditto` path already retains links. A small archive/extract control passes for
both tools (`local/archive-links.json`). The smoke now verifies a relative
selected-VM symlink on Unix or identical selected-VM bytes on Windows, then
executes `rxvm`. The strengthened smoke passes Debug (28.17 s) and Apple ASan
(45.48 s): `local/entry-link-debug/`, `local/entry-link-asan/`.

The first-run **macOS arm64 package is green**: package smoke passed in 121.597
seconds, including actual `MTL0` computation, dynamic VM consumers and relocated
native execution. This generated-fixture GPU execution does not qualify the real
BGE/Smol profiles. Evidence: `remote/544d41f02/macos-arm64-qa/` and job log.
The uploaded user-test archive is artifact `10407098186`, 21,960,366 bytes, with
GitHub artifact digest
`6c18bbbbab425e16a0fef085af06e78863b7daf5ad5d586afc66b1cdb3c0b17b`.
Artifact metadata is retained in `remote/544d41f02/artifacts.json`.

The strengthened public-entry smoke also passes on macOS arm64 at `2bc56249d`.
The downloaded archive retains `bin/rxvm -> rxbvm`, all 16 declared provider
file hashes pass, and neither the fixture nor smoke helper is shipped.
Evidence: `remote/2bc56249d/macos-arm64-qa/` and
`macos-arm64-archive-inspection.json`; the latter records the user ZIP's own
size/hash separately from GitHub's artifact envelope digest.

The other unfinished first-run package jobs may be superseded by the corrected
candidate; cancellation is not a pass. Keep unavailable real-device
and model-provenance acceptance visible in the parent plan; fixture success
does not close those items. CUDA SDK caching now saves verified inputs before
compilation, allowing a failed build to reuse its SDK on retry.
The corrected CUDA matrix also preserves the pinned engine's portable
architecture defaults instead of supplying a narrower CI-specific target list.

The Linux CPU/Vulkan package at `2bc56249d` is also green: final package smoke
passes in 16.785 seconds, with CPU computation and zero GPU devices reported.
The downloaded archive preserves `bin/rxvm -> rxtvm`; all 31 declared provider
entries pass their hashes and no fixture/helper is included. Evidence:
`remote/2bc56249d/linux-qa/`, `linux.log`, `linux-archive-inspection.json`.
This is package/CPU proof, not real Vulkan-device qualification.

Diagnostic Windows branches retain useful other-platform work on the main
candidate. MSVC/CUDA run [34998653921](https://github.com/adesutherland/CREXX/actions/runs/34998653921)
is at `ae8c19681`; MinGW run [35000816874](https://github.com/adesutherland/CREXX/actions/runs/35000816874)
is at `473fb6894`, including CI-F05/06. The prior MinGW run at `fff73ce08` was
superseded before completion. These diagnostic revisions are not final combined
qualification. CI-AC-06 is checked: cancellation/rerun isolation, retained
failure logs, measured serial smoke, generous hang guards and absence of model
benchmarks are established. CI-AC-02/03/05/07/08 and CI-03–05 remain open.

### Intel Mac isolated replay

Diagnostic run [35002901578](https://github.com/adesutherland/CREXX/actions/runs/35002901578)
at `e39916f2a` passes the **entire** package smoke (224.287 s; 15 commands),
including CPU and actual `MTL0` computation, both optimization/VM paths and the
relocated native program. The engine completes in about 76 s, before the
explicit 120-second diagnostic capture condition; no stack is sampled and no
process is terminated. The later VM/native calls account for the rest of the
smoke. These are fixture/packaging durations, not model-performance findings.

The downloaded user-test ZIP preserves `rxvm -> rxbvm`, verifies all 29 declared
provider entries and excludes fixture/helper files. Raw logs, summary and archive
inspection are under `remote/e39916f2a/`. The original `2bc56249d` 30-minute
engine stall is not reproduced or explained. Do not claim a runtime repair or
silently relabel that first result as a pass. CI-F07 awaits the ordinary final
candidate run and retains its original evidence; no deadline, device exclusion
or inference algorithm was changed to obtain this replay. The diagnostic-only
workflow override remains outside the main candidate.

### Cold backend initialization guard

The permanent `rxllama_backend_probe_cycle` also calls `initialize_engine`.
At the pinned source, Metal registration constructs its device and shader
library before it returns (`ggml-metal.cpp`, `ggml-metal-device.m`). A first
Intel consumer process in the replay takes about 70 seconds; this is a whole
consumer observation, not an isolated shader/compiler performance measurement.
Its prior 30-second probe guard was therefore too narrow for this platform
risk. Widen it to the existing 1,800-second backend hang backstop and retain
`RUN_SERIAL=TRUE`, the exact probe/reopen calls and sanitizer diagnostics.

This preventive QA correction does **not** explain or repair CI-F07's original
1,800-second engine stall. The same permanent probe passes normal Debug
(0.66 s) and Apple ASan (1.02 s), with its scheduling properties inspected:
`local/probe-backstop-debug/`, `local/probe-backstop-asan/` and the input note.
No broad local replay is warranted for this guard/comment change; exact-head
hosted gates remain open.

### Windows MinGW package and archive

Run [35004301149](https://github.com/adesutherland/CREXX/actions/runs/35004301149)
at `4a0924ee1` passes the complete CPU/Vulkan package smoke in 13.483 seconds
(15 commands). All 14 consumer executions use the restricted Windows-only PATH;
the native compilation action retains its compiler environment. Public `rxvm`,
the alternate `rxbvm`, optimized/nonoptimized programs and relocated native
execution pass. The runner reports zero GPU devices: this is CPU/package proof,
not actual Vulkan-device qualification. CI-F05 and CI-F08 have target-platform
repair evidence, and CI-F06's audited loader requirement passes on MinGW.
The separate MSVC/CUDA package remains open.

The actual downloaded ZIP is 57,562,461 bytes, distinct from its GitHub envelope.
It preserves the identical `rxvm.exe` copy of preferred `rxtvm.exe`, passes all
29 provider entry hashes and contains matching bootstrap core DLLs plus the
three MinGW runtime DLLs beside the executables. No Windows OS DLL, fixture or
smoke helper is shipped. Evidence: `remote/4a0924ee1/windows.log`, `windows-qa/`,
`artifacts.json` and `windows-archive-inspection.json`.

All four base platforms now have passing triage packages on their recorded
revisions. This is not a combined exact-head qualification: both CUDA builds
and the final Build/Deep Build/Sanitizer workflows remain pending. CI-AC-02/03/
05/07/08 and CI-03–05 remain open until that evidence is reconciled.

### Combined wider gate dispatch

Combined candidate `cabc668cf242a969994dc750f6a71d41b15d385e` is frozen on
`origin/temp/llama-release-combined`. [Deep Build QA 35007946063](https://github.com/adesutherland/CREXX/actions/runs/35007946063)
and [Sanitizer QA 35007947861](https://github.com/adesutherland/CREXX/actions/runs/35007947861)
are running; no pass is claimed. Linux leak detection remains enabled.
The auxiliary branch's create-event Build run `35007934305` only executes
metadata/guard checks: every package job is skipped, so it supplies no package
qualification. The final complete Build matrix remains to run on the combined
candidate after CUDA triage. Initial run identities are retained under
`remote/cabc668cf/`; neither existing CUDA job was cancelled by this dispatch.

### CI-F10 — GCC sanitizer allocation header packaging

Linux ASan/LSan job `104512384642` in combined run `35007947861` fails during
compilation of the existing `rxllama_generation_bridge` QA executable:
`sanitizer/allocator_interface.h` is absent from the installed GCC 13 headers.
No test ran and no sanitizer memory diagnostic appears. Retained complete build
and driver logs are in `remote/cabc668cf/linux-sanitizer/` and
`linux-sanitizer.log`. This is a QA compiler/header portability finding, not a
new SAN memory finding or a reason to disable leak detection.

GCC 13's public source declares and implements
`__sanitizer_get_current_allocated_bytes`. Use its exact C declaration when the
header is unavailable; retain the installed header on toolchains that provide
it. The allocation measurement, 32 MiB bound and model workload are unchanged.
Normal/ASan focused proof and supported Linux retry remain required. Sources:
[GCC public declaration](https://github.com/gcc-mirror/gcc/blob/releases/gcc-13.3.0/libsanitizer/include/sanitizer/allocator_interface.h)
and [implementation](https://github.com/gcc-mirror/gcc/blob/releases/gcc-13.3.0/libsanitizer/asan/asan_stats.cpp).

Combined package run `35008896953` was started on `cabc668cf`, then cancelled
early after CI-F10 made a follow-up candidate necessary. Cancellation supplies
no pass. The already useful older Windows CUDA diagnosis and wider Deep/Mac
ASan checks are retained; complete exact-head gates still belong to the final
combined candidate after fixes.

The CI-F10 declaration change passes the same explicit generation qualification
target in normal Debug (83.1685 s) and Apple ASan (109.027 s), including 100
singles, 20 four-row batches, private/shared workers and co-resident models.
ASan records 1,456 bytes of retained live-allocation growth against the unchanged
32 MiB guard. These are correctness-run durations, not new performance verdicts.
Evidence: `local/allocator-debug/`, `local/allocator-asan/`, `allocator-inputs.json`.
The hosted GCC retry remains the missing-header branch's platform proof.

The Linux CUDA triage package at `2bc56249d` passes 161 smoke-tier checks and
the final consumer smoke (33.524 s, 15 commands, zero actual GPU devices).
Its full product build takes 1 h 54 m 30 s with two build jobs and all pinned
upstream portable CUDA architectures. The downloaded 786,190,716-byte user ZIP
retains `rxvm -> rxtvm`, passes 34 provider entry hashes (following relative
in-archive library links), and includes CUDA runtime/cuBLAS libraries and notices.
No fixture/helper is shipped. See `remote/2bc56249d/linux-cuda*`.

CI-F10 repair commit `489559ec4` and its retained evidence/docs form combined
candidate `0f35b970e` on `origin/temp/llama-release-combined`.
[Sanitizer retry 35009830830](https://github.com/adesutherland/CREXX/actions/runs/35009830830)
is dispatched with the complete Linux/Mac matrix and Linux leaks enabled.
This retry supersedes the earlier sanitizer candidate; cancellation is not a
Mac pass. Earlier Deep Build and MSVC CUDA runs remain useful triage. Final
Build/Deep/Sanitizer exact-head closure is still open.

### CUDA compiler cache

Adrian explicitly approved caching the CUDA engine on 15 September after the
1 h 54 m 30 s Linux cold build. Both CUDA jobs now use the prebuilt sccache
v0.16.0 through the action's pinned v0.0.11 commit
`fc920bf0ec8de6ee65d409111f7ec508035751ba`. Explicit C/C++/CUDA launchers prevent
GGML's automatic wrapper from being layered a second time. GitHub Actions stores
the compiler outputs; source/header, compiler and option keys remain intact.
Cache server I/O failure falls back to the ordinary compiler. This does not
remove GPU architectures, alter the engine pin or change recipient dependencies.

Text/JSON statistics are retained with package QA. `actionlint` and the same
14 publication/signing/matrix controls pass (`local/compiler-cache-validation.log`).
A first populated run and subsequent warm Linux/Windows CUDA evidence remain
required by CI-AC-09; no build-time saving is claimed yet. The ordinary source
build still defaults to `ENABLE_LLAMA=OFF`.


### Resumed wider QA and Windows build-tree probe follow-up

Adrian resumed after restart. Deep run `35007946063` at `cabc668cf` passes
2,329 comprehensive checks plus all four install/package qualification tests
on Linux and ARM Mac. Retained logs and artifacts are `remote/cabc668cf/deep-linux*`
and `deep-macos-arm64*`. Intel Mac has reached package qualification.
Windows passes 2,250/2,251 comprehensive checks; its sole failure is
`rxllama_backend_probe_cycle`, exit `0xc0000135` before any helper output.
Its command launches `lib/plugins/llama/tests/rxllama_bridge_lifecycle.exe`,
while its imported DLLs are in `bin/providers`, outside executable-directory
and PATH lookup. The exact command and empty startup failure are retained in
`deep-windows.log` and `deep-windows-qa/Testing/Temporary/LastTest.log`.

This extends CI-F06's Windows dependency-lookup finding to the build-tree QA
executable. Set a per-test Windows PATH prepend to the bridge target directory
for the probe and other CTests using the same lifecycle helper. The package
prerequisite remains mandatory. This does not change the installed consumer
PATH, package layout, model workload or wide timeout. The existing failing
probe is the permanent regression; hosted Windows confirmation remains required.
CMake's scoped [environment modification](https://cmake.org/cmake/help/latest/prop_test/ENVIRONMENT_MODIFICATION.html)
affects only the named tests. No sanitizer memory finding is present in this
Windows result. Normal/Apple-ASan focused results will be retained below.

The same probe passes after focused normal/Apple-ASan builds (0.71/0.82 s),
retained under `local/windows-probe-debug/` and `local/windows-probe-asan/`.
The Windows-only environment branch still requires the hosted target result;
these local checks establish that the ownership workload remains intact.


### Download size and possible post-build split — discussion only

Adrian asked whether the already-built plugin should be a separate download.
The current `dev-snapshot` (`b5b827489`) ZIPs are 19.9 MB ARM Mac, 20.9 MB Intel
Mac, 26.2 MB Linux and 25.4 MB Windows. Retained candidate archives are 22.1,
29.8, 49.7 and 57.6 MB respectively; Linux CUDA is 786.2 MB. These are decimal
compressed MB without models; differences also include other revision/build
inputs. Exact sizes and an indicative provider-entry breakdown are retained in
`local/download-size-comparison.json`. Windows CUDA remains unmeasured.

Recommendation for Adrian's decision: retain a single build per toolchain,
then create a small core archive and an optional matching llama.rexx archive
containing its engine, backends, dependencies, examples and notices. Preserve
the version/platform/toolchain match, including the MSVC CUDA variant. Verify
core-only execution and reconstruction of the complete installed payload from
core plus plugin before the existing restricted-environment smoke. This would
reduce ordinary download size, not CUDA compilation work. It is a proposed
delivery change only; CI-AC-02/03 and the currently authorized complete archives
remain unchanged until Adrian decides. Existing running builds remain useful.

### Combined 76df02be3 package results (in progress)

ARM Mac job `104529899383` in Build `35013130021` passes 161 fast checks and
all 15 package commands (113.861 s), with actual CPU/MTL0 fixture computation.
The downloaded ZIP is 22,144,155 bytes, retains `rxvm -> rxbvm`, passes all 16
provider entry hashes and contains no fixture/helper. Log, package QA and
archive inspection are retained under `remote/76df02be3/macos-arm64*`. Other
platforms and cache warm-reuse evidence remain pending.

Adrian additionally asked whether the same Windows base would work with both
Vulkan and CUDA. This is the intended interchangeability criterion for the
proposed split, not an already demonstrated property: the present Vulkan lane
uses MinGW and the CUDA lane uses MSVC. Before adopting the split, prove the
same base with either complete plugin package, including native executable
generation and its static/import-library toolchain requirements. Existing
separate complete-ZIP passes do not establish cross-toolchain interchangeability.

Adrian asked whether Vulkan can also use MSVC. The pinned upstream Vulkan
CMake file has explicit MSVC support and its build guide includes Visual Studio
with the Windows Vulkan SDK. A consistent MSVC base plus MSVC Vulkan/CUDA
plugin variants is feasible to qualify. Switching the default Windows base
would also select cREXX's portable `rxbvm` instead of MinGW's default `rxtvm`;
this execution tradeoff was reported to Adrian. No default-toolchain change or
MinGW removal has been approved or implemented.

Adrian corrected the framing of that VM difference: `rxbvm` is the faster
option in the accepted performance results. Do not present portability as an
inherent slowdown or reopen the accepted performance programme. Reuse retained
platform/compiler-labelled evidence; this clarification does not itself supply
a new MSVC-versus-MinGW timing result. Focus the proposed MSVC Vulkan follow-up
on build, package and consumer compatibility.


### CI-F07 follow-up — intermittent relocated native stall

Deep Intel job `104512381773` at `cabc668cf` passes all 2,329 comprehensive
checks and three of four qualification tests. Package smoke reaches all 15
preceding commands, including CPU/Metal engine, both optimized/nonoptimized VMs
and native generation, then its relocated native process reaches the unchanged
1,800-second guard with no program output. Retained full logs and individual
command records are in `remote/cabc668cf/deep-macos-intel*`. No sanitizer
diagnostic is present. Keep this on CI-F07 while its cause is unclassified.

The separately scheduled `76df02be3` Intel package run passes the same unchanged
smoke workload (188.948 s, 15 commands) and 161 fast checks. Product sources and
the smoke program/harness are unchanged between these revisions; the intervening
CMake change is Windows test lookup and the QA header change is ASan-only. This
is useful isolated replay evidence, not an explanation of the intermittent stall.

Extend the existing opt-in macOS diagnostic capture to any named smoke command,
so the next stalled relocated consumer yields a stack before termination. The
normal workload/backstops remain unchanged; a diagnostic stop fails explicitly.
A separate Intel diagnostic branch supplies the opt-in flag, never the ordinary
release workflow. Do not start another complete Deep run merely to repeat a
timeout with no additional diagnostic evidence.

The new non-engine diagnostic path has a causal control: a temporary staged
compiler deliberately sleeps, while the real engine checks complete first.
`--capture-command-after 5` captures a 7,992-byte stack for `opt-compile`, stops
that process and returns a diagnostic failure. The control passes only when
that failure and nonempty stack are observed; see `local/command-capture-control/`.
The ordinary package workload also passes in normal Debug (27.866 s) and
maintained Apple ASan (45.068 s) after this option is added. Exact runner logs,
input hashes and smoke records are under `local/command-capture-{debug,asan}/`
and `local/command-capture-inputs.json`. No default timeout or package behavior
changes, and no ordinary full QA is repeated for this diagnostic option.

All four standard package lanes at `76df02be3` are now green. Windows passes
151 fast checks (parser-mode checks are disabled in that lane as configured)
and 15 smoke commands, with 14 restricted-PATH executions (13.487 s). Linux
passes 161 plus 15 smoke commands (17.190 s). Both Mac lanes pass 161 plus 15
commands, including real Metal fixture execution. All four actual downloaded
ZIPs pass retained manifest, public-VM and fixture/helper-exclusion inspection.
This does not close the separate Deep Intel intermittent stall, Windows probe
retry, CUDA package/cache proof or exact-head sanitizer gates.

The isolated Intel native replay is branch `temp/llama-release-intel-native`,
commit `3875270c6`, run `35016898785`. It reuses the SHA-checked Intel archive
from Build `35013130021`, compiles only the tiny native consumer, then relocates
and repeats it up to five times. A 180-second diagnostic stop captures a stack
and fails; the ordinary qualification guard is unchanged. The helper's local
ARM control passes native compilation and two relocated executions
(`local/native-replay-control/`). No engine rebuild or model download is used.

### CI-F11 — MSVC Windows SDK macro collision

The old MSVC/CUDA diagnostic `34998653921` at `ae8c19681` finishes CUDA
compilation, then fails compiling our adapter at `generation.h:39`. The SDK
defines `small` as `char`; our `char small[256]` becomes an invalid declaration.
Rename this private buffer to `token_piece`, retaining its size and every
generation operation. The actual bridge translation unit reproduces the
failure with the SDK macro before the rename and passes after it; ordinary
Debug and maintained Apple-ASan bridge builds also pass. Evidence and the
Microsoft header reference are in `local/msvc-small-control/README.md`.

The 190,722,959-byte remote log is preserved losslessly as
`remote/ae8c19681/windows-cuda.log.gz`, with original byte count/hash alongside
it and the QA artifact in `windows-cuda-qa/`. This is a first-party compile
portability defect, not a CUDA computation or sanitizer finding. Its permanent
regression is the actual MSVC provider build; target confirmation remains open.

Adrian challenged the repeated failures on 15 September. The next work is
narrowed to this MSVC compile check and isolated Intel diagnosis, preserving
useful running CUDA/cache and sanitizer jobs. Do not launch another broad
qualification cycle until those failures are understood. The branch-only
MSVC diagnostic `temp/llama-release-msvc-compile` at `63a6e1004` configures a CPU
build and compiles only the actual adapter object before any engine build.
This does not adopt CI-D01, change the release matrix or close its criteria.

Subsequent instruction supersedes the diagnostic order above: CI-D01 is now
approved with the core-first gate and existing combined jobs cancelled.
The Intel replay did capture the stall. Its main thread waits in
`initialize_engine -> ggml_metal_library_init -> dispatch_group_wait`, while
Metal compiler threads wait for Apple's synchronous compiler service reply.
The stack in `remote/3875270c6/intel-native/relocated-1-stack.txt` locates the
wait during backend initialization, before model inference. It does not alone
prove why the service failed to respond. Defer this plugin investigation until
the four independent core builds pass; no timeout relaxation or GPU removal
is implied.

### Approved core-first restart

Core-only run `35018374738` at `9caac6dcb` builds with `ENABLE_LLAMA=OFF` on
Linux/GCC, Windows/MSVC 14.44, ARM Mac and Intel Mac. Linux and both Macs are green; Windows compiles but fails the KeyAccess
compaction test (146/147 pass, CI-F12 below). Their actual extracted archives
exercise optimized/nonoptimized compilation, assembly, linking, the public
selected VM and applicable alternative, plus relocated native execution.
No inference SDK or model is downloaded. Logs/artifacts are retained under
`remote/9caac6dcb/`; this does not mark the four-core gate complete.

ARM Mac passes 161 core checks (66.90 s), followed by the extracted-core smoke
(2.206 s). Its ZIP is 19,671,021 bytes, SHA256
`79aa05859a0dc80281bef9bc967cfa8d26f8a2f1cbdae5718146cb1990757788`, and selects
`rxbvm`. The smoke harness itself was checked locally against copied Release
tools (`local/core-package-harness/`); that is harness proof, not another fresh
core build.

`local/plugin-only-build-graph.json` confirms the three selected plugin targets
do not compile core compiler/assembler/linker/VM objects. Six package controls
verify core identity, rejection of changed binaries, refusal to overwrite core
files, sharing of identical runtime DLLs, ZIP executable/symlink preservation
and rejection of nonlocal entries (`local/split-package-controls.log`). The
plugin packager emits an add-on ZIP and reconstructs both downloads for smoke;
its workflow integration and target qualification are still pending.

Cancelled combined run `35013130021` retains partial compiler-cache data:
Linux has 1,692 misses/17 hits, Windows 1,256 misses/12 hits. There are no compiler
failures in those cache snapshots; one/two cache writes failed respectively.
These interrupted cold-population observations are not warm-run savings or
completed CUDA package proof. Cancelled sanitizer `35009830830` artifacts have
no observed ASan/LSan or compile diagnostic; they remain cancelled, not passed.
Their logs are preserved under `remote/0f35b970e/` and `remote/76df02be3/`.


### CI-F12 — Windows core KeyAccess compaction

The first core-only run `35018374738` is terminal. All three Unix core jobs pass
161/161 tests: Linux 67.82 s, ARM Mac 66.90 s, Intel Mac 112.02 s. Intel's actual
extracted ZIP smoke also passes (3.712 s); its archive is 20,302,974 bytes,
SHA256 `8a7bbae4ad346e7a9f5ca1bbe678df0ebb5358bf96aaad78b7eccdc336b4a8fe`.
The Windows/MSVC job `104547463919` builds successfully, then its unasserted
compaction/close sequence aborts with `0xc0000409`. Logs and CTest artifacts are
retained under `remote/9caac6dcb/`.

MSVC CRT `rename` refuses an existing destination. The compaction replacement
therefore fails after closing both original streams, leaving null pointers;
`closekey` passed those to `fclose`. A permanent C control uses the actual plugin
with real failing filesystem replacement and intercepts only null closes so it
can report the defect without aborting its test host. Before repair it reports
two null closes. The ordinary opt/noopt Rexx test now checks compaction's return,
49 live records, a retained value and deletion, then closes/reopens and checks
again to bypass cached values. Windows replacement now uses the existing FS
provider's `MoveFileExA(..., MOVEFILE_REPLACE_EXISTING)` pattern. Cleanup checks
whether streams remain open and reports actual close errors. Public signatures
and database format are unchanged; two-file replacement is not newly atomic.

The three controls share a CTest resource lock because existing compaction uses
working-directory temporary names, with a 300-second hang backstop. The C test
stays outside the shipped bin directory. Normal Debug passes all three in 0.44 s, and Apple ASan passes the same three
in 0.73 s (`local/keyaccess-compaction/`). The initial ASan build stopped on
Apple's unsupported leak option; the completed run uses the documented Apple
leak-off setting. Supported Linux leak qualification remains open. Windows-only
hosted qualification is next. No upstream engine,
GPU SDK or CUDA is built for this repair.

The operating-system behavior matches Microsoft's [rename documentation](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/rename-wrename?view=msvc-170)
and [fclose null-argument contract](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fclose-fcloseall?view=msvc-170).


### Windows/MSVC core retry and CI-F13 packaging harness

Run `35021253099`, SHA `7425ff241`, passes all 148 core tests (73.18 s), including
`keyaccess_test_noopt` and the deliberate replacement-failure cleanup control.
The ZIP smoke then stops before consumer execution on `KeyError: SystemRoot`:
Python's Windows environment dictionary uses uppercase `SYSTEMROOT`. The casing
repair and focused isolation control preserve the restricted runtime PATH.
Seven packaging controls pass. Raw build/test/failure evidence is retained under
`remote/7425ff241/`. Core correctness is green; extracted Windows ZIP proof is
still pending, as is the newly requested MinGW/both-VM gate (`35021842079`).


### Completed MSVC core and prepared split pipeline

Run `35022668868` at `aade0fd0f` passes 148 core checks and the additional three
focused KeyAccess controls. Its extracted ZIP passes ten consumer commands,
including optimized/nonoptimized compilation/assembly/linking/public VM and
relocated native execution (2.718 s). Archive: 21,836,575 bytes, SHA256
`ca62526dfed26f1c0d4b1c60ce86a998179e6f538111195a27b93490ba1cbcec`.
Raw evidence is under `remote/aade0fd0f/`. Earlier MinGW `35021842079` passes
152 core checks in 56.47 s, then fails the known CI-F13 lookup; its evidence is
under `remote/25f14aabf/`, and retry `35023122646` is pending.

`local/core-finalization-harness/` checks the staged/signed-payload finalization
path with the retained real hosted ARM core archive, then executes its extracted
ZIP and relocated native consumer. It is harness proof, not a fresh current-head
core build. `local/split-package-controls.log` has 25 passing controls, including
signed plugin finalization, stale-hash rejection and core-overwrite rejection.
The updated `local/plugin-only-build-graph.json` covers all six selected plugin
build/helper targets and finds no core toolchain compilation. The prepared main
workflow reuses exact core artifacts, keeps MinGW non-shipping, preserves core
installers and signing guards, and publishes copies of the same tested ZIP bytes.
Actual split-plugin execution and the Windows Vulkan SDK setup remain untested.


### MinGW gate completed

`35023122646` at `aade0fd0f` passes 152 core tests (73.25 s), three focused
KeyAccess tests (0.22 s) and all twelve actual-ZIP consumer checks (2.488 s).
`rxvm` is byte-identical to `rxtvm`, and both that entry point and `rxbvm` run
opt/noopt programs. The relocated native program passes too. The GitHub artifact
inventory contains only `core-qa-windows-mingw-aade0fd0fe8ae97f5130383165bcf1b835145903`
(30,211 bytes), with no distributable ZIP. Logs and summaries are retained under
`remote/aade0fd0f/core-mingw*`. This closes the initial five-configuration core
prerequisite; it does not yet qualify the new split-plugin jobs or wider gates.
