# Native inference binary delivery evidence

Live work package: [numbered pipeline plan](../../planning/native-inference-ci.md).
Candidate: `temp/llama-release-qa`. The accepted baseline is `12647a91aa7e9`,
merged with remote RXPP `b5b827489` by `c1de1670812a`. Publication is not authorized
by a partial or pending result.

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

The other unfinished first-run package jobs may be superseded by the corrected
candidate; cancellation is not a pass. Keep unavailable real-device
and model-provenance acceptance visible in the parent plan; fixture success
does not close those items. CUDA SDK caching now saves verified inputs before
compilation, allowing a failed build to reuse its SDK on retry.
The corrected CUDA matrix also preserves the pinned engine's portable
architecture defaults instead of supplying a narrower CI-specific target list.
