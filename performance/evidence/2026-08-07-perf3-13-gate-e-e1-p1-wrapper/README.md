# PERF3-13 Gate E E1-P1 lifecycle-wrapper evidence

Date: 2026-08-07

Branch: `develop`

Source state: committed E1 `b5e1d05651eab6c4be20f7a953a6840e9a13f98c`
plus the uncommitted E1-P1 changes to `interpreter/rxvmintp.c`, this evidence
bundle and the live PERF3 control documents. The exact comparison control is
post-EF0 `19802842e0655b2f2ae011f911e909a2ded7233b`; it already includes the
spawn recovery, so the measured E1 loss is not attributable to EF-0.

## Selected implementation

Public `run()` is a small worker-lifecycle wrapper. It begins worker execution,
calls the private flattened interpreter core, ends worker execution and retains
the existing failure diagnostic. The flattened core still owns allocator
enter/leave and all instruction semantics. Nested same-owner calls continue to
use the existing execution-depth count. No language, RXAS/RXBIN, plugin ABI,
channel or allocator-hot-path semantic changes.

This form was selected after a 60-pair diagnostic isolated the regression to
placing lifecycle/error code inside the 500+ KiB flattened function. The
counterfactual wrapper retained the enlarged E1 context layout while restoring
the core's former stack/register-allocation pattern.

## Accepted first ordinary-Release verdict

Both products are ordinary profiling-off Release builds. The candidate was
compared with the exact post-EF0 control using identical retained linked RXBIN
and library inputs, one warmup, 20 pairwise-balanced serial recorded rounds per
workload, no sample removal and no append/rerun. Positive percentages favour
E1-P1.

| Workload | Pairs | Paired mean | 95% interval | Median | Favourable | Verdict |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Sieve | 20 | -0.190379% | -0.700736% to +0.319979% | -0.223705% | 8 | neutral |
| Richards | 20 | -0.286721% | -1.832747% to +1.259306% | -0.235605% | 8 | neutral |
| Towers | 20 | -0.059737% | -0.638991% to +0.519517% | -0.132069% | 9 | neutral |
| RexxCPS | 20 | -0.231775% | -0.985820% to +0.522270% | -0.492743% | 9 | neutral |
| Pooled core four | 80 | -0.192153% | -0.632127% to +0.247821% | -0.235605% | 34 | neutral |

All 168 processes passed: eight warmups and 160 recorded executions. The
committed-E1 RexxCPS diagnostic was clear adverse at -0.780187%; the production
wrapper is neutral, so the targeted regression is removed. No 3% workload
guard fires. Raw samples, output checks and capture metadata are retained under
`timing/`.

The candidate `rxbvm` is 1,002,392 bytes with SHA-256
`cc9eb05c6b232ed7fb42f52b95b6ca067b61d214e45342cca90cf441fb870996`:
1,264 bytes larger than the post-EF0 control and 176 bytes smaller than the
committed E1 product.

## Accepted closeout QA

| Gate | Result |
| --- | --- |
| Pre-verdict focused Debug | 13/13 passed |
| Pre-verdict focused Release | 6/6 passed |
| Focused Apple AddressSanitizer | 3/3 passed after rebuilding allocator, worker and reentrancy targets |
| Full Debug | 1,997/1,997 passed with `--parallel 30` in 210.77 seconds |
| Ordinary Release/concrete products | `rxbvm`, `rxtvm`, `rxvml`, linked artifacts and static archive consumer built |
| Focused Release closeout | 14/14 passed |
| Diff hygiene | `git diff --check` passed |

Apple LeakSanitizer remains unsupported, so the focused supported sanitizer
run used `detect_leaks=0`; exact allocator teardown assertions remain enabled
in Debug. The previously retained broad-ASan RXAS use-after-free is outside
this VM-only slice and was neither suppressed nor reclassified. No native
Windows cross-toolchain is installed on this Mac. Native Windows, Intel Linux
and Linux ARM64 evidence remain later Gate E platform work.

Host: Apple M5, 10 logical CPUs, Darwin 25.5.0 arm64. Toolchain: Apple Clang
21.0.0, Ninja. Release configuration: `CMAKE_BUILD_TYPE=Release`,
`CREXX_VM_PROFILING=OFF`.
