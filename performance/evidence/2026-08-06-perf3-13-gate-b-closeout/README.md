# PERF3-13 Gate B closeout

Date: 2026-08-06

Status: accepted by Adrian; Gate B closed; Gates C-F remain closed

## Decision

The unchanged-`value` single-worker allocator is accepted as the baseline for
later value-shape work. It retains the 240-byte public/internal value shape,
one logical arena per RXVM context, typed value/reference silos, power-of-two
byte classes, a central 64 KiB whole-slab depot and separately tracked
oversized extents.

This does not approve a compact value, allocator-service ABI, worker threads,
cross-worker transfer, processes, channels or RXAS instructions. Attribute
reclamation is absent from the hot path. The explicit
`reclaim_attribute_storage()` operation has test coverage but no production
caller; later gates own reclamation and memory-pressure policy.

## Corrected formal allocator verdict

The original 2026-08-05 four-cell allocator capture used simple rotation. The
subsequent runner audit proved that schedule did not balance every pair's
relative order, so it is not the formal closeout authority. The accepted
2026-08-06 rerun uses the corrected Level B pairwise scheduler.

- Host: Apple M5 MacBook Air, Darwin arm64 25.5.0.
- Toolchain: Apple Clang 21.0.0, Ninja, ordinary Release
  `-O3 -DNDEBUG`, `CREXX_VM_PROFILING=OFF`.
- Power: AC attached, low-power mode off, no recorded thermal/performance
  warning.
- Workloads: Sieve, Permute, Bounce, Richards, Base64, Towers and canonical
  RexxCPS.
- Lifecycle: startup included; exact control-built RXBIN images and
  `library.rxbin` for every cell; source/TRACE form therefore identical
  between control and candidate.
- Sampling: one warmup and 12 serial, pairwise-balanced recorded rounds per
  workload across libc control and worker/slab candidate for both concrete
  engines.
- Correctness: all 364 processes passed; no sample was removed.

The compiler-selected Apple Clang product is `rxvm -> rxbvm`. Its stable-six
geometric performance ratio is `1.200604559` (+20.060456%). Every stable
workload is clear favorable with 12/12 favorable pairs.

| Workload | `rxbvm` paired median | Mean 95% interval | Favorable |
|---|---:|---:|---:|
| Sieve | +11.615% | +11.245%..+12.241% | 12/12 |
| Permute | +19.005% | +17.987%..+20.666% | 12/12 |
| Bounce | +15.025% | +13.613%..+17.522% | 12/12 |
| Richards | +26.462% | +25.762%..+27.994% | 12/12 |
| Towers | +21.773% | +21.264%..+22.232% | 12/12 |
| RexxCPS | +3.893% | +2.878%..+4.560% | 12/12 |

The direct-threaded diagnostic lane is also favorable at `1.136287881`
(+13.628788%) on the stable six. The governed common-five ratios are
`1.269177` for `rxbvm` and `1.177466` for `rxtvm`, but those figures
remain descriptive because Base64 is materially noisy (10/12 and 11/12
favorable respectively, with wide ranges). Base64 does not select the
allocator or dispatch policy.

## RSS materiality check

A separate four-round serial pairwise RSS panel used the same binaries,
images and workloads. It is a bounded materiality check, not a formal
throughput aggregate.

For the selected `rxbvm` lane, median peak RSS changes are:

| Workload | Candidate minus control | Change |
|---|---:|---:|
| Sieve | +163,840 bytes | +0.899% |
| Permute | +348,160 bytes | +1.914% |
| Bounce | 0 bytes | 0.000% |
| Richards | -901,120 bytes | -4.592% |
| Base64 | -716,800 bytes | -3.723% |
| Towers | -1,757,184 bytes | -8.642% |
| RexxCPS | -520,192 bytes | -2.655% |

There is no material RSS blocker for Gate B.

## Dispatch and build-product decision

The retained slab-era dispatch study is supporting evidence for the stable
product path introduced in the closeout:

- Apple Clang: `rxvm` selects portable switch `rxbvm`; the current
  `rxbvm` beats the old direct-threaded engine by +8.870% on the study's six
  stable rows.
- GCC: `rxvm` selects direct-threaded `rxtvm`; the GCC 16 screen found the
  threaded engine +16.452% over GCC `rxbvm` on the six stable rows.
- MSVC: only portable `rxbvm` is built and `rxvm.exe` is a copy of it.

The Apple compiler-led and selective-inline prototypes are rejected as global
policies. They improve some threaded rows but regress `rxbvm`; current
forced-inline/flatten defaults therefore remain. The GCC screen is not a
formal Linux platform verdict and does not approve global threaded-engine
retirement.

## Proportional validation

- full Clang Debug build completed;
- focused allocator/value/product checks passed 7/7;
- representative non-spawn Clang Debug smoke passed 129/129 after adding the
  allocator library to native link metadata;
- installed SDK consumers passed 2/2 and the isolated installed Clang product
  resolves `rxvm -> rxbvm`;
- GCC 16 Release built and ran both concrete engines and resolves
  `rxvm -> rxtvm`; and
- Base64 correctness matched exact length/checksum under `rxvm`, `rxbvm`
  and `rxtvm`.

Spawn tests are deliberately excluded from this transitional baseline by
Adrian's decision. Existing spawn remains important and must migrate to the
final worker-ownership architecture; no claim is made that its current tests
pass. Base64 parser/API redesign remains separate under CAP-03.

## Provenance

- Control source: `4813e98d1dca1ac77d5899dd6c5787e4b83f4772`,
  clean detached source under the retained Gate B scratch workspace.
- Allocator candidate snapshot:
  `d032f27a3d5c3dfbecf1a803c11462b45f4839d6`.
- Closeout branch: `codex/rxvm-default-and-base64-review`.
- Closeout parent before this evidence:
  `3471a1d4e` (`Require even rounds for exact pairwise balance`).
- Final source snapshot: the enclosing Gate B closeout commit; its dirty scope
  before commit is the VM product/build/test/documentation integration and
  this evidence bundle.

## Evidence map

- `allocator/pairwise-manifest-v2.txt`: exact control/candidate commands.
- `allocator/pre-state.txt`, `post-state.txt`: host, toolchain, power,
  process and identity capture.
- `allocator/timing/`: raw timing samples, process output, absolute summary
  and pairwise comparison.
- `allocator/rss/`: raw RSS samples, process output and absolute summary.
- `allocator/summarize_paired_v2.crexx`: Level B pairwise summary used for
  this concrete-engine naming transition.
- `dispatch/STUDY.md` and child directories: the Apple/GCC inline/dispatch
  study, raw samples, process outputs and host state.
- `logs/`: final build, smoke, install and driver results.
- `COMMANDS.md`: exact closeout commands and interpretation boundaries.
- `checksums.sha256`: recursive identity closure, excluding itself.

All performance percentages above are observations from the retained host and
toolchain. The compiler-specific product selection is an evidence-backed local
policy, not a claim that another compiler/platform will reproduce the same
ratios.
