# Slab-era VM dispatch and inlining evidence

Date: 2026-08-05 (Europe/London)

## Executive decision

The slab/worker-arena architecture remains mandated. None of this work tests or
recommends returning to per-object libc allocation.

On Apple M5 with Apple Clang 21, the current switch-dispatch `rxbvm` is the
clear winner. Against the current threaded `rxvm`, it is:

- +9.147% on the formal common-five geometric mean;
- +7.576% across all seven rows;
- +8.870% across the six stable rows when noisy Base64 is excluded.

A selective hot-inline prototype improves threaded `rxvm` substantially, but
still trails current `rxbvm` by 1.299% on the common five, 1.525% on all seven,
and 1.945% on the six stable rows. Current `rxbvm` wins five of those six stable
rows; selective `rxvm` wins only Sieve.

Therefore:

1. Stop treating threaded `rxvm` as a performance asset on Apple Clang.
2. Prefer/build `rxbvm` only on Apple Clang (and continue the existing
   switch-only MSVC approach), subject to the normal production approval gate.
3. Do **not** delete threaded dispatch globally yet. Homebrew GCC 16 reverses
   the result: threaded `rxvm` is +16.452% over GCC `rxbvm` on the six stable
   rows.
4. Decide global retirement only after formal Intel Linux Clang/GCC and Linux
   ARM64 evidence. If Clang is standardized as the Unix release compiler, the
   evidence strongly favors global retirement; if GCC remains a performance
   compiler, retain threaded dispatch behind a compiler/platform build option.
5. Keep the current forced-inline/flatten policy for production `rxbvm` for
   now. Both broad compiler-led and five-helper selective policies regress it.

No production worktree was changed. Everything below is disposable scratch
evidence.

## Provenance and scope

- Gate B source worktree:
  `/Users/adrian/CLionProjects/CREXX-rxvm-worker-memory-gatea`
- Gate B base commit:
  `4813e98d1dca1ac77d5899dd6c5787e4b83f4772`
- Exact dirty-candidate snapshot commit:
  `d032f27a3d5c3dfbecf1a803c11462b45f4839d6`
- Snapshot tree:
  `78427a7c62d00d752d0b2e386afedbbe6ba96d2b`
- Scratch source:
  `/tmp/crexx-rxvm-inline.yvLywZ/source`
- Final scratch experiment commit:
  `a1f40a1966600bc5e88de3dc227e33c50b907280`
- Scratch results:
  `/tmp/crexx-rxvm-inline.yvLywZ/results`

The control preserves the Gate B allocator/value shape: 64 KiB slabs,
power-of-two byte classes, typed value/reference silos, and one worker per VM
context. The 240-byte value shape is unchanged.

Spawn was deliberately excluded. Its current transitional allocator/lifecycle
breakage was accepted for this gate and does not contaminate VM timing.

## Measurement validity

- Ordinary Release, profiling off, `-O3 -DNDEBUG`.
- Apple formal lanes: Apple Clang 21, AC attached, low load, no thermal warning.
- Common benchmark images and `library.rxbin` for every compared VM binary.
- No outlier removal.
- Decisive comparisons use separate two-cell `workload × VM` groups, one
  warmup and 12 recorded rounds. This gives exact 6:6 A/B versus B/A order.
- RexxCPS is evaluated using its higher-is-better benchmark rate, not elapsed
  process time.
- Towers and RexxCPS are reported separately as well as in the descriptive
  all-seven aggregate.
- Base64 remained noisy even in a separate 24-pair extension and is not used
  alone to select a policy.

An initial four-cell formal capture was retained but demoted to diagnostic
evidence. Rotating four cells balances absolute position but leaves each pair's
relative order 9:3. RexxCPS exposed a sign disagreement, so the decisive lane
was rerun as fourteen independent two-cell groups. This is an important harness
lesson: rotation is not pairwise balance when group size exceeds two.

## Clang structural matrix

`F` means `run()` flatten; `A` means global `always_inline` on `RX_INLINE`.

| Policy | rxvm run span | rxbvm run span | rxvm file | rxbvm file | focused build | peak RSS |
|---|---:|---:|---:|---:|---:|---:|
| F1A1 current control | 536,188 | 535,480 | 1,000,872 | 1,001,032 | 38.11 s | 711,737,344 |
| F0A1 no flatten | 492,964 | 490,308 | 967,544 | 967,800 | 32.01 s | 646,971,392 |
| F1A0 compiler-led | 307,308 | 300,356 | 787,480 | 771,240 | 14.65 s | 479,051,776 |
| F0A0 compiler-led/no flatten | 109,440 | 106,088 | 588,232 | 571,720 | 4.19 s | 232,620,032 |
| F1S five hot helpers forced | 463,748 | 459,524 | 935,832 | 936,088 | 24.16 s | 639,156,224 |

Marking `run()` hot produced byte-for-byte identical `rxvm` and `rxbvm`
binaries under Apple Clang. It is a no-op and should not be added.

The neutral harness control has the same `run()` addresses, span, and
disassembly as the untouched slab snapshot; the sole disassembly difference is
the embedded scratch Git commit text.

## Apple Clang formal result: broad compiler-led policy

The broad F1A0 policy is rejected as a global change.

- `rxvm`: +3.133% common five; +0.961% all seven.
- `rxbvm`: -3.965% common five; -4.925% all seven.
- Stable `rxbvm` losses include Richards -9.371%, Towers -9.979%, Bounce
  -6.137%, Permute -2.099%, Sieve -2.079%, and RexxCPS -4.510%.

It demonstrates that Apple Clang's default inline choices are not globally
safe for this interpreter, even though build time and code size improve
dramatically.

## Apple Clang formal result: selective hot helpers

The evidence-selected helpers were:

- `value_zero`
- `clear_binary_payload`
- `prep_string_buffer`
- `refresh_utf8_flags`
- `power_of_two_size`

They are the most frequent slab/value lifecycle helpers that the compiler-led
build declined to inline, especially in `rxbvm`.

### Selective policy versus current policy

| Workload | rxvm | rxbvm |
|---|---:|---:|
| Sieve | +6.445% | -0.035% |
| Permute | +29.703% | -0.558% |
| Bounce | +9.225% | -2.016% |
| Richards | -4.885% | -7.112% |
| Base64 | +1.161% noisy | -13.214% noisy |
| Towers | -0.757% | -2.448% |
| RexxCPS | +3.973% | -5.114% |
| Common five | +7.729% | -4.721% |
| All seven | +5.935% | -4.456% |
| Six stable rows | +6.753% | -2.913% |

Most stable effects are strongly paired: selective `rxvm` has 12/12 wins for
Permute, Bounce and RexxCPS, and 12/12 losses for Richards; selective `rxbvm`
has 12/12 losses for Permute, Richards, Towers and RexxCPS.

This is not a viable global policy. It is useful evidence that slab/value hot
boundaries matter, but the two dispatch engines need different code-generation
policies.

### Dispatch decision on Apple Clang

Current `rxbvm` versus current threaded `rxvm`:

| Workload | rxbvm advantage |
|---|---:|
| Sieve | +2.424% |
| Permute | +33.738% |
| Bounce | +11.418% |
| Richards | +1.363% |
| Base64 | +0.129% noisy |
| Towers | +0.521% |
| RexxCPS | +7.078% |

Even selective threaded `rxvm` remains behind current `rxbvm` on five of the
six stable rows: Permute -3.017%, Bounce -1.968%, Richards -6.164%, Towers
-1.271%, RexxCPS -2.899%; only Sieve is +3.926%.

## Base64 extension

The separate 24-pair broad-policy extension remained unstable:

- `rxvm`: paired median -1.669%, 11 wins / 13 losses, range -25.441% to
  +29.391%.
- `rxbvm`: paired median +0.957%, 13 wins / 11 losses, range -26.816% to
  +20.834%.

Base64 should remain in recorded panels but should not choose an inline or
dispatch policy until its variance source is understood.

## GCC 16 portability control

GCC was built on the same Mac with TLS disabled because Network.framework is a
Clang-specific backend. This is a bounded portability screen, not a second
formal platform verdict.

### Structure and build cost

| Policy | rxvm run span | rxbvm run span | focused build | peak RSS |
|---|---:|---:|---:|---:|
| GCC current | 2,102,120 | 2,080,668 | 478.33 s | 4,505,878,528 |
| GCC selective | 2,102,112 | 2,081,316 | 493.39 s | 4,992,876,544 |

With GCC plus `flatten`, the five-helper selective boundary is effectively a
no-op and does not reduce build cost.

### GCC `rxbvm` versus threaded `rxvm`

| Workload | rxbvm effect versus rxvm |
|---|---:|
| Sieve | -29.469% |
| Permute | -23.373% |
| Bounce | -23.011% |
| Richards | -2.160% |
| Base64 | -7.338% |
| Towers | -0.777% |
| RexxCPS | -15.803% |
| Six stable rows | -16.452% |

Thus direct-threaded dispatch remains a major GCC optimization. A global source
deletion would sacrifice GCC runtime performance.

Nevertheless, in sequential same-host descriptive evidence, Apple-Clang
`rxbvm` is +14.809% over GCC's best engine (`rxvm`) on the six stable rows.
Clang wins Sieve, Permute, Bounce, Towers and RexxCPS; GCC wins Richards.
This supports Adrian's view that Clang is the stronger release-performance
compiler, but Linux formal evidence is still required before making that a
cross-platform rule.

## Stack and compiler evidence

Two-second Permute stack samples put 87-93% of top samples inside `run()` for
current `rxvm`, current `rxbvm`, and selective `rxvm`. The recurring out-of-line
costs are:

- `maybe_trim_attribute_storage`: roughly 6-9%;
- `syncNumericContext`: roughly 2-3%.

Libc `malloc` is not a top stack. The slab-era crossover is therefore best
explained by changed hot helper/code-layout interactions, not by a conclusion
that slabs are inherently slower.

Clang optimization records show the broad compiler-led policy makes materially
different decisions for the two engines. For example, direct `run()` callsites
for `clear_binary_payload` are mostly inlined in compiler-led `rxvm`, but far
more are missed in compiler-led `rxbvm`; `prep_string_buffer`, `value_zero`,
`refresh_utf8_flags`, and `power_of_two_size` show similar partial-inline
boundaries. This matches the runtime split.

## Source-structure conclusions

1. `run()` is about 8,190 source lines (actual scratch lines 4,959-14,148) and
   dominates execution. Its size alone is not a defect: shrinking it from ~536
   KiB to ~109 KiB can make runtime worse.
2. Core interpreter source does not include project `.c` files. The `.c`
   inclusions found are vendored decNumber implementation composition, not the
   VM handler structure.
3. `rxops.h` is included twice as an X-macro inventory. Handler binding macros
   are lexical because computed-goto labels belong to `run()`. Converting these
   mechanically to functions is not byte-identical cleanup.
4. `rxvmvars.h` has about 102 `RX_INLINE` definitions and `rxvmstem.h` about
   29. Moving them wholesale to `.c` changes code generation and must be
   performance-gated; it is not a cosmetic refactor.
5. The existing object-library design compiles the same giant TU twice for
   `NTHREADED` and non-`NTHREADED`. On GCC this is an especially large build
   cost. Removing an engine would simplify source ownership and build cost
   materially.
6. `interpreter/CMakeLists.txt` unconditionally overwrites
   `CMAKE_C_FLAGS_RELEASE`. It silently discarded requested Clang diagnostic
   flags despite the cache containing them. Production cleanup should compose
   caller/toolchain flags and use target-scoped optimization options instead.
7. `run()`'s `hot` attribute is useless under the tested Clang. Keep semantic
   no-inline fences for computed-goto label ownership and
   `rxvm_loose_string2float`; do not conflate those with tuning no-inline.

## Recommended next gate

1. Keep Gate B slab/value work intact and measure future value-shape work
   primarily with Apple-Clang `rxbvm` on this Mac. Retain `rxvm` only as a
   comparison lane until the dispatch decision is approved.
2. Before global engine deletion, run the same exact two-cell 12-pair dispatch
   verdict on Intel Linux Clang/GCC and Linux ARM64. Windows MSVC already lacks
   threaded dispatch; a Windows Clang confirmation is useful but not blocking
   for MSVC structure.
3. If Clang wins overall on Linux as it does here, standardize the Release
   compiler and retire threaded dispatch. If GCC remains supported for peak
   runtime, make the threaded engine an explicit GCC-only build option rather
   than a universally built peer.
4. For the remaining `rxbvm`, keep current flatten/forced-inline defaults until
   an `rxbvm`-specific hot set passes a formal panel. The tested five-function
   set is insufficient.
5. Investigate `maybe_trim_attribute_storage` as the next allocator/value hot
   boundary. It connects directly to reclamation policy and is the only
   allocator-adjacent helper visible as a substantial out-of-line stack cost.
6. Fix performance harness grouping so multi-candidate captures either create
   explicit pair groups or use a true pairwise-balanced schedule.

## Evidence index

- Decisive selective formal summary:
  `formal-selective-timing/summary.csv`
- Decisive selective formal samples:
  `formal-selective-timing/samples.csv`
- Broad compiler-led balanced formal summary:
  `formal-f1a0-balanced-timing/summary.csv`
- Broad compiler-led balanced formal samples:
  `formal-f1a0-balanced-timing/samples.csv`
- 24-pair Base64 extension:
  `base64-f1a0-timing/summary.csv`, `base64-f1a0-timing/samples.csv`
- GCC dispatch screen:
  `gcc16-dispatch-timing/summary.csv`, `gcc16-dispatch-timing/samples.csv`
- Static source/size inventory:
  `static-structure-inventory.txt`
- Clang inline remarks:
  `remarks2-control-build.log`, `remarks2-f1a0-build.log`
- Stack samples:
  `sample-clang-control-rxvm.txt`, `sample-clang-control-rxbvm.txt`,
  `sample-clang-selective-rxvm.txt`
- Power/load/binary provenance:
  `formal-selective-pre-state.txt`, `formal-selective-post-state.txt`,
  `gcc16-dispatch-pre-state.txt`, `gcc16-dispatch-post-state.txt`
