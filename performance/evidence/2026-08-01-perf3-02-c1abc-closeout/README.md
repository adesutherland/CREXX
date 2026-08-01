# PERF3-02 C1abc production closeout

Status: **complete Apple ARM64 production closeout**

Adrian selected the composed C1abc production ladder on 2026-08-01 after
accepting the R1 Release timing verdict. The ordinary compiler now contains
only the three bounded production proofs:

1. C1a-R2 directly uses an unused, call-free and attribute-free nested
   receiver;
2. C1b-R1 directly uses the exact detached indexed-Boolean receiver guard; and
3. C1c-R1 directly uses an eligible by-value scalar object formal.

The disposable option mask and the correctness-invalid broad no-write branch
have been removed. Their exact replay source remains checksum-closed in
`../2026-08-01-perf3-02-r1-repanel/compiler-option-source.diff`; rejected and
unselected variants therefore remain reproducible without retaining a dormant
production switch.

## Outcome

| Dimension | Closeout result |
| --- | --- |
| ordinary Release build | profiling-off `rxc`, `rxas`, `rxvm` and `rxbvm` build passed |
| selected-image identity | Richards RXAS `ac1737cc...59d9` and Towers RXAS `35ead6d2...3e4e`, byte-identical to measured mask 7 |
| focused Release | 11/11 pass: production ladder, receiver/reference canaries, by-value reuse and Richards/Towers opt/no-opt |
| focused Debug structural/runtime pairs | 16/16 pass after refreshing seven deliberately changed optimized-RXAS goldens |
| broad Debug | 1,972/1,972 pass in one final invocation |
| retained option replay | 31/31 R1 checksum entries verify; replay diff SHA-256 `7bf1f32e...fdbf` |
| protected lifecycle images | all five pre-existing RXBIN files retain their starting SHA-256 identities |

The first broad Debug invocation reported seven optimized-RXAS golden
mismatches. Every paired optimized runtime test already passed with its
established output. Review showed only the intended removal of one or two
receiver/formal locals and their copies, plus deterministic register
renumbering. The seven exact optimized goldens were refreshed, their structural
and runtime pairs passed 16/16, and the final broad invocation passed
1,972/1,972.

## Performance authority

This bundle contains no new benchmark samples. The accepted timing remains the
checksum-closed R1 same-session panel in
`../2026-08-01-perf3-02-r1-repanel/`:

| Target | Selected C1abc workload effect | `rxvm` | `rxbvm` |
| --- | --- | ---: | ---: |
| Richards | exact C1a+C1b image | -53.55% | -52.57% |
| Towers | exact C1c image | -18.92% | -18.97% |

Every timing row was favorable in all 12 pairs per VM with a wholly favorable
mean interval. The older 2026-07-31 clean-host C0/C1a/C1c panel remains the
historical baseline authority. Exact production RXAS identity makes another
timing rerun unnecessary for this closeout.

## Host and build boundary

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch/source anchor: `develop` at
  `e38e514bf611ae3873513368c44742e2ae7332d1` plus the recorded dirty
  PERF3-02 implementation and evidence scope
- Host: Darwin 25.5.0 arm64, Apple M5
- Toolchain: Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2
- Release: `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`
- Debug: `CMAKE_BUILD_TYPE=Debug`, `CREXX_VM_PROFILING=OFF`
- Test parallelism: focused Release 10; object pairs 15; broad Debug 30

## Evidence map

- `production-identity.csv`: ordinary Release product and selected benchmark
  program identities;
- `source-identity.csv`: locked infrastructure, production compiler logic,
  focused proof fixtures and protected lifecycle identities;
- `production-surface-audit.txt`: zero-match proof for the removed mask and
  broad investigation branch, linked to their retained replay source;
- `validation.csv`: exact gate results and retained logs;
- `COMMANDS.md`: reproduction commands;
- `logs/`: Release/Debug build logs, the reviewed first broad result, focused
  pair proof and final broad pass;
- `checksums.sha256`: recursive integrity for this bundle.

## Claim boundary

This closes the selected PERF3-02 Apple ARM64 slice. It does not claim
cross-platform performance, delete tactical assembler rules, select a C2
ownership rewrite, recover the separately queued P1A assembler cost, change a
public language/RXAS/RXBIN/ABI contract, commit, or push.
