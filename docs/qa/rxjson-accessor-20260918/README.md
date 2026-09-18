# JSON accessor repair: normal publication qualification

Adrian accepted the first Release speed verdict and authorized hotfix staging
and promotion to origin/develop on 18 September 2026. The owning
[worklist](../../planning/rxjson-accessor-repair-20260918.md) retains the design,
acceptance criteria and earlier 16-case baseline/candidate controls.

The clean hotfix checkout was fast-forwarded to
`e99136a1d5725d0c44128f64f505e1b46c64a51f`. Only the five-helper borrowing repair,
the immutable-document ownership regression and related documentation were
transferred. The main develop checkout's unrelated dirty work was preserved.
No compiler, ABI, JSON storage or public signature change is included.

## Build and checks

Ordinary profiling-off macOS ARM64 Release, AppleClang, native llama disabled
in this core build. The normal product compiler, assembler, linker, both VM
executables, library and standalone `crexx` driver were built successfully:

```sh
cmake -S . -B cmake-build-release
cmake --build cmake-build-release --target qa-prep-smoke testbifs --parallel 8
ctest --test-dir cmake-build-release --tests-from-file /tmp/crexx-json-hotfix-focused.txt --parallel 8 --output-on-failure
ctest --test-dir cmake-build-release --tests-from-file /tmp/crexx-json-hotfix-remaining.txt --parallel 30 --output-on-failure
cmake --build cmake-build-release --target ts_linein_stdin_fixtures --parallel 8
ctest --test-dir cmake-build-release -R '^ts_linein_stdin_crexx_tty$' --output-on-failure
```

The union of all Level-B functional cases and core `essential`/`smoke` cases is
451 unique tests, listed in `selected-cases.txt`. The 14 registered JSON cases
passed first (0.10 seconds); the remaining selection explicitly excluded them.
That run passed 436 tests and reported one **not run**, because
`linein_tty_harness` was absent (54.25 seconds). Building its existing supported
fixture target and running that one test passed (0.62 seconds). The compiler,
both VM executables and library hashes were unchanged across fixture
preparation, so all 450 previous passes remain valid. Final accounting is
**451/451 unique passing cases**, no disabled/skipped cases and no repeated
passing product test. Original not-run evidence is retained.

The JSON source and test are byte-identical to the accepted private candidate.
`artifact-hashes.txt` retains their hashes and the current runtime/library
hashes. `downstream-comparison.json` retains the combined JSON/RAG speed verdict,
including the slower first trial and RAG's separate model-load timeout followed
by a passing unchanged replay. It is not a JSON-only speed claim or a clean
full RAG qualification. No corpus or normal installed runtime was changed.

## Publication boundary

The user authorized commit on hotfix, push to origin/hotfix and promotion of
the identical repair commit to origin/develop. The automatic publication
workflows supply the hosted checks; their terminal result must be verified
against the pushed SHA. No extra overnight deep, stress or sanitizer workflow
is dispatched for this ordinary library change. This record establishes local
publication readiness and does not assert hosted success or release readiness.
