# SAN-001 RXAS SSA first Release verdict

Status: **accepted by Adrian on 2026-08-21**

## Scope

This is the mandatory first profiling-off Release verdict for the SAN-001
memory-correctness repair in `rxas_flow_value_node()`.  It is a local
same-host regression decision, not a release-wide performance claim.

- Branch and HEAD: `develop`,
  `298f412dc0e40ef12b4957df4b5f8b57a8a14d9f`.
- Worktree: deliberately dirty with Adrian's existing RCC-5, sanitizer-policy
  and regression work preserved.
- Production delta: retain stable source fields across lazy SSA resolution and
  reacquire `value_versions[value_id]` after the growth-capable call.
- Control: the existing pre-fix ordinary Release `rxas`, copied before the
  target rebuild.
- Candidate: the same Release tree rebuilt only through target `rxas`.
- Configuration: `CMAKE_BUILD_TYPE=Release`,
  `CREXX_VM_PROFILING=OFF`.
- Host: Apple M5, Darwin 25.5.0 arm64, 10 logical CPUs, 24 GiB RAM,
  Apple clang 21.0.0, CMake 4.3.2 and Ninja 1.13.2.
- Session: AC attached, low-power mode off, no thermal or performance warning.
- Temporary binaries and full transient logs:
  `/private/tmp/crexx-san001-release-verdict.FkyUC9`.

## Correctness qualification

The permanent 64-value growth-boundary regression first failed under
Apple-ASan with a heap-use-after-free write, then passed unchanged in normal
Debug and Apple-ASan after the repair.

| Check | Result | Retained log |
| --- | --- | --- |
| pre-fix focused Apple-ASan | intended heap-use-after-free | `cmake-build-debugasan/asan-logs/20260821-101842-ctest/ctest.log` |
| post-fix focused Debug | 1/1 pass | `cmake-build-debug/asan-logs/20260821-101929-ctest/ctest.log` |
| post-fix focused Apple-ASan | 1/1 pass | `cmake-build-debugasan/asan-logs/20260821-101939-ctest/ctest.log` |
| applicable RXAS flow/proof Debug panel | 70/70 pass | `cmake-build-debug/asan-logs/20260821-102446-ctest/ctest.log` |
| identical Apple-ASan panel | 70/70 pass | `cmake-build-debugasan/asan-logs/20260821-102451-ctest/ctest.log` |
| original optimized Towers Apple-ASan artifact | pass | `cmake-build-debugasan/asan-logs/20260821-102139-build/build.log` |
| Release Towers artifact plus semantic test | 2/2 including required fixture | `release-towers-ctest.log` |

Control and candidate emit byte-identical direct images:

| Input | Input SHA-256 | Output SHA-256 | Size |
| --- | --- | --- | ---: |
| optimized Towers | `3d1226677941025fb9ce768745123a3f9153afe0d82ac6b828fb33150cd69df9` | `d96588145fe9e5926557151c044d9873f4f0cdd6778db6acc2945d82dc6ee635` | 19,142 B |
| optimized RexxCPS | `41f094d349a7a6195a82d294cd82740a688e1d78a97a4d8d6bb90730af7ccb6a` | `0039da82b4bcb7b8829318ea6dd0ac62be3f1ad338b026eeb5e2096d39a9c857` | 51,234 B |

The direct hashes include the common absolute temporary module path.  The
candidate rebuilt by the normal Towers target is also exact against the
pre-fix build-tree artifact:
`3e49e28149b5778ebf1b064aa1873c74fcb52cb06746377e0adcf0734ebf82b2`,
19,110 B.

## Profiling-off Release verdict

Canonical optimized RexxCPS is the smallest decisive assembler-cost cell: it
exercises the SSA proof service substantially more than Towers while the
byte-identical outputs mechanically exclude a VM-runtime change.  One warmup
per binary preceded serial, pairwise-balanced control/candidate rounds.
Because the 12- and 24-pair mean intervals crossed zero, sampling continued
without removal to the governed 36-pair cap.

| Metric | Control | Candidate | Change |
| --- | ---: | ---: | ---: |
| median process elapsed | 0.259790063 s | 0.258436561 s | paired median -0.513520% |
| mean paired change, 95% interval | — | — | -0.551549% [-1.865052%, +0.761954%] |
| favourable elapsed pairs | — | 21/36 | noisy/inconclusive |
| median peak RSS, 4 paired records | 123,551,744 B | 123,551,744 B | 0 B / 0.000% |
| `rxas` executable size | 779,272 B | 779,272 B | 0 B |
| optimized RexxCPS image size | 51,234 B | 51,234 B | 0 B |
| optimized Towers image size | 19,142 B | 19,142 B | 0 B |

The elapsed interval crosses zero, so the correct label is
**noisy/inconclusive around neutral**.  Its adverse bound is +0.762%, wholly
inside the +3% individual assembler-workload guard.  Relative MAD is 0.713%
for the control and 0.896% for the candidate.  No sample was removed.

## Decision boundary

The evidence supports accepting the SAN-001 production repair: it removes the
proved use-after-free without changing bytecode, semantic output, peak RSS or
artifact size, and it shows no Release assembler regression signal.

Adrian accepted this neutral, guard-clean verdict on 2026-08-21.  SAN-001 is
repaired but is not closed by this performance evidence alone.  The complete
Apple-ASan build and 2,310/2,310 CTest gate subsequently passed in
`cmake-build-debugasan/asan-logs/20260821-112920-full`; supported Linux
ASan/LSan remains required before the live worklist entry can move to closed
history.
