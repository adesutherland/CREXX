# PERF2-06/07 V1R01-R1 Apple closeout

Adrian accepted the favorable first ordinary Release verdict for
`PERF2-06-07-V1R01-R1`. This bundle retains the proportional Apple ARM64
closeout for that exact proof-wide compiler candidate and the prerequisite
V3-R01 representation-correctness fix. No other candidate was implemented.

The compiler directly places an enclosing method's `§this` receiver through
arbitrary internal branches and calls when the reconstructed body has at most
one explicit return; fallthrough is included and already-proved receiver aliases
survive later clone passes. Multiple explicit returns retain materialisation
because per-exit receiver-owned link balance is not yet proved. The ordinary
direct-object path remains unrestricted by this nested-`§this` exit guard.

## Outcome

| Dimension | Closeout result |
| --- | --- |
| focused Debug | 10/10 pass |
| broad Debug | 1,925/1,925 pass after refreshing two exact optimized-RXAS goldens for the deliberately removed locals |
| focused Release | 10/10 pass |
| broad profiling-off Release | 1,925/1,925 pass |
| sanitizer | focused ASan 10/10 pass with fresh artifacts; Apple LSan is unavailable and the runner's exact unsupported-host abort is retained |
| lifecycle | 10 observations per phase; compile -0.037752%, assemble -2.190540%, `rxvm` load-to-first-result -6.301653%, `rxbvm` -1.710234%; no adverse lifecycle guard |
| peak RSS | 3 observations per cell across six workloads and both VMs; largest adverse median is +49,152 bytes / +0.284091%; no RSS guard |
| RXBIN compatibility | 12/12 retained pre-V1 linked-image cells pass under the rebuilt `rxvm`/`rxbvm` |
| install/package | isolated-prefix install passes with 133 files and both installed VMs pass; the configured project has no CPack `package` target |
| first Release result | retained unchanged: Richards -21.224%/-21.076%, Permute -58.019%/-56.466%, exact-work Bounce neutral, common ratios 1.244352x/1.242301x |

The first broad Debug run passed 1,923 tests and reported only the expected
optimized-RXAS changes in `18_array_class_opt` and
`inline_test_class_methods_opt`: one and two receiver-materialisation locals
respectively disappeared. After the exact goldens were refreshed, the two tests
passed and the final broad Debug run passed 1,925/1,925 in one invocation.

## Evidence map

- `logs/`: independently identifiable build, focused/broad CTest, sanitizer,
  compatibility, lifecycle, package-target and install logs;
- `lifecycle-current/` and `lifecycle-candidate/`: raw 10-sample phase rows and
  maintained Level B summaries;
- `rss/`: maintained Level B 72-sample RSS matrix and raw output;
- `lifecycle-delta.csv` and `rss-delta.csv`: exact median guard calculations;
- `source-product-workload.sha256` and `artifact-sizes.csv`: final source,
  product, workload, tool and artifact identities;
- `live-freeze.txt`, `host-build-freeze.txt` and `host-post.txt`: Git, host,
  power and build context;
- `COMMANDS.md`: closeout reproduction commands;
- `HARDWARE-HANDOVER.md`: frozen successor order, commands and stop rules.

The current-attribution panel remains in
`../2026-07-27-perf2-06-07-selection-panel/`. The accepted timing and exact
operation proof remain immutable in
`../2026-07-27-perf2-06-07-v1r01-r1-first-release-verdict/`; this closeout does
not duplicate or reinterpret those raw samples.

## Claim boundary

This is an Apple ARM64 result for one local source candidate. It is not a
cross-platform claim, a final VM/default-stream selection, a closure of
`PERF2-06-D01`, an architecture approval for C2R03/V6R01, or authorization to
start PERF2-08/09 in this session.
