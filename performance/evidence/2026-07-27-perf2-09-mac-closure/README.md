# PERF2-09 formal Mac closure

This recursively checksum-closed bundle is the first complete PERF2-09
same-session Apple scorecard for the accepted PERF2-06/07 product. Adrian
approved the prerequisite PERF2-08 capability/equivalence panel on 2026-07-27.
No production code, public interface, language rule, RXAS/RXBIN format or VM
default was changed by this activity.

## Outcome

| Dimension | Result |
| --- | --- |
| Source | detached clean `057592681c0c68e90f436bf02d8c5a116111952a`; accepted product source is `39d3c652e27860222f5d5ed43af71147589b1121` and the intervening commit is test-only |
| Host | MacBook Air `Mac17,3`, Apple M5, 10 logical CPUs, 24 GB, macOS 26.5.2 (25F84) |
| Product | independent profiling-off Release, Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2 |
| Correctness prerequisite | PERF2-08 qualification: isolated cREXX 39/39, selected dual-VM opt/no-opt 48/48, NetRexx 11/11, ooRexx qualified rows 8/8; retained Mandelbrot decimal failures are deliberate exclusion evidence |
| Formal timing | 348/348 initial process samples plus 20/20 governed Base64 append samples; zero invalid recorded samples |
| Common five | `rxvm/ooRexx 2.125260`, `rxbvm/ooRexx 1.842840`, `rxvm/NetRexx 0.742985`, `rxbvm/NetRexx 0.644251` |
| Separate targets | Towers qualified object comparison; RexxCPS qualified disclosed native-rate comparison; lifecycle reported separately |
| Peak RSS | 87/87 initial plus 40/40 governed NetRexx append samples; zero invalid recorded samples |
| Lifecycle | 80/80 initial plus 80/80 governed append phase rows; zero invalid recorded samples |
| Artifact inventory | 84 exact path/size/SHA-256 rows |
| Largest qualified deficit | Richards: `0.267262/0.264171` versus ooRexx and `0.157815/0.155990` versus NetRexx (`rxvm/rxbvm`) |
| Stop | no PERF2-10/11 work, new performance candidate, cross-platform conclusion or final VM/default selection |

The common result is favorable versus ooRexx as a portfolio aggregate, but it
does not meet the programme's stronger per-benchmark band: Richards and
Base64 remain behind ooRexx in both VMs. It is also below NetRexx in aggregate.
See [scorecard.md](scorecard.md) and
[workload-dossiers.md](workload-dossiers.md) for the complete disposition.

## Evidence map

- `timing/`: all initial and governed-append samples, captured output,
  per-cell summaries, ratios and four exact `N=5` geometric means;
- `rss/`: independent zero-warmup peak-RSS samples and final summaries;
- `lifecycle/`: compile/translate, assemble and honestly named
  load-to-first-result rows plus the exact generated probe forms;
- `generated/netrexx/`: exact generated Java and class products used by the
  formal cells;
- `artifacts.csv`: source, RXAS, RXBIN, product, runtime, library, generated
  product, tool and manifest size/hash inventory;
- `manifests/`: immutable formal and one-append cell definitions;
- `provenance/`: pre/post Git, host, power, thermal and load freeze; and
- `checksums.sha256`: recursive evidence-root closure, excluding only the
  checksum file itself.

## Noise and inclusion

No correctness-passing observation was removed. The single governed append
was applied to the exact selected cells. The following remain labelled noisy
after it:

- timing: cREXX Base64 under both VMs (`n=20`);
- RSS: NetRexx Permute, Richards, Base64 and RexxCPS (`n=13`); and
- lifecycle: every series except cREXX compile (`n=20`).

They remain in their medians. A second retry is forbidden by the governed
one-append rule and was not taken.

## Claim boundary

This is a same-session Apple ARM64 closure and hardware-handover baseline. It
does not establish cross-platform superiority, close `PERF2-06-D01`, replace
the required Linux ARM64 lane, select compact/hot-cold VM layout, select a
final VM/default stream, or authorize capability/API/architecture work.
