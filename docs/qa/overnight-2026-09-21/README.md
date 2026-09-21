# Overnight rxvector QA repair — 21 September 2026

Authoritative scope and acceptance: [RXVECTOR-02 AC-09–12](../../planning/rxvector-binary-owner-20260919.md#overnight-qa-repair-21-september-2026).
Sanitizer infrastructure tracking: SAN-QA-017.

## Diagnosis

- Source actually checked out: `4468c7d75925cc158a568392f4abe97a59156df6`.
  Scheduled event metadata names master, but the assurance gate resolves develop.
- [Deep run 35555049283](https://github.com/adesutherland/CREXX/actions/runs/35555049283):
  all four scheduled comprehensive platforms fail the vector manifest assertion;
  `rxvector_index_core` is additionally Not Run because its executable is absent.
  Release parallel-build comparisons and isolated stress succeed.
- [Sanitizer run 35559006744](https://github.com/adesutherland/CREXX/actions/runs/35559006744):
  Linux x64 and macOS arm64 fail the same manifest assertion. Mac executes 2358
  tests with this single failure; Linux stops at the assertion after 1582 tests.
  Retained artifacts contain no ASan/LSan memory diagnostic. Unexecuted Linux
  tests are not passes. Apple LeakSanitizer is unavailable.
- The same two causes occur on 20 September in runs 35484487476 / 35487589362.
- `5949ef27e` adds the immutable owner and intentionally changes the provider to
  mixed V2 policy: four packed procedures stay process-reentrant; ten owner
  entry points require a VM session. The old blanket-capability assertion is
  stale; `handles=1 before=0` is valid accounting. The new codec executable is
  an ALL target but was omitted from the explicit comprehensive preparation.

## Local reproduction and repair

`baseline-vector.log` reproduces the exact assertion in the clean worktree.
`baseline-prep.txt` records absence of the codec executable from the transitive
`qa-prep-comprehensive` commands before repair. Failed hosted run metadata and
compressed logs are in `failed/`.

Repair changes only the vector-specific manifest test, the codec preparation
registration and documentation. No provider/runtime/compiler behavior changes.
The policy test keeps the existing two-VM binding, execution and teardown checks.
`policy-controls.py` builds temporary real-provider variants to verify that
incorrect stateless/owner policy and missing lifecycle hooks are rejected.

## Local qualification

- Clean Debug `qa-prep-comprehensive` succeeds and builds the codec executable
  through the repaired dependency. `stage-product` also succeeds.
- Focused Debug **21/21**, 3.86 s; same focused maintained Apple ASan **21/21**,
  6.84 s. Regex:
  `^(rxpa_bundled_.*_concurrency|rxpa_dynamic_session_factory_failure_rollback|rxvector_.*)$`.
  Both run serially. ASan uses `tools/asan-run.sh --phase ctest`, with
  `--build-leaks off --leaks off` solely for the Apple platform limitation.
- Instrumented preparation uses `tools/asan-run.sh --phase build --build-target
  test_rxpa_concurrency --build-target rxvector_test --build-target
  rxvector_index_core_test --build-jobs 10 --build-leaks off --leaks off`.
  No sanitizer diagnostic in build or test logs.
- Normal Debug correctness **2344/2344**, 817.05 s:
  `ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
  -L '^(essential|smoke|comprehensive)$'`.
- Policy controls: valid provider accepted; stateless procedure marked affine,
  owner marked reentrant, missing destroy hook and missing host-aware factory
  all rejected. Commands/script and individual diagnostics are retained.
- `qualified-inputs.json` records relevant source/test SHA-256 values. The main
  checkout's 47 pre-existing dirty/untracked files remain byte-identical.

## Hosted qualification

Publication and terminal exact-head Build/CodeQL/Deep/Sanitizer results pending.
The Deep and Sanitizer repeats resolve the observed overnight failures. No
suppression, test exclusion, product change or timeout increase is introduced.

