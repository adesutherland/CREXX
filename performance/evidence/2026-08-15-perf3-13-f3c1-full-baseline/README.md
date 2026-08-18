# PERF3-13 F3C1 task-launch and single-thread baseline

Date: 2026-08-15

Branch/source: `develop` at committed F3C1 product
`9e7d600c8108be9c3f1f568e6bd5b7109e4bf62b`. The only timing-time dirty
scope was this evidence bundle and its live roadmap row.

Status: **complete; task launch and ordinary single-thread performance are
guard-clean**.

## Result

Task launch remains a distinct measurement in the accepted
[`F3C1 first Release verdict`](../2026-08-15-perf3-13-gate-f-f3c1-task-binding-cache-first-release-verdict/):

| Cell | `rxbvml` mean | `rxtvml` mean | Disposition |
| --- | ---: | ---: | --- |
| tiny-task latency jobs/s | +39.076017% | +40.340609% | both clear favorable |
| throughput jobs/s | -0.099356% | +1.551257% | `rxbvml` inconclusive; `rxtvml` clear favorable |

The separate single-thread panel compares the exact retained pre-cache F1f
Release VMs with the committed F3C1 VMs. Every pair shares the same current
optimized workload image and `library.rxbin`. The governance noise rule grew
the initial 12 pairs to 24 and then the 36-pair ceiling. All 1,036 processes
passed: 28 warmups and 1,008 recorded executions. No sample was removed.

Final paired means follow the source metric orientation: elapsed-time changes
below zero are faster; RexxCPS rate changes above zero are faster.

| Workload | `rxtvm` mean and interval | Result | `rxbvm` mean and interval | Result |
| --- | ---: | --- | ---: | --- |
| Sieve elapsed | -0.148155% [-0.587162%, +0.290853%] | inconclusive | -0.310996% [-0.608381%, -0.013610%] | favorable |
| Permute elapsed | +0.409260% [-0.109825%, +0.928345%] | inconclusive | +0.175053% [-0.274954%, +0.625060%] | inconclusive |
| Bounce elapsed | -0.175584% [-1.010993%, +0.659825%] | inconclusive | -0.219033% [-0.677310%, +0.239244%] | inconclusive |
| Richards elapsed | -0.658579% [-1.716323%, +0.399166%] | inconclusive | -0.571116% [-1.421987%, +0.279755%] | inconclusive |
| Base64 elapsed | -4.202705% [-6.320139%, -2.085271%] | favorable | -3.166449% [-6.091361%, -0.241538%] | favorable |
| Towers elapsed | +0.986414% [+0.134681%, +1.838146%] | adverse, below guard | -0.049646% [-0.548861%, +0.449570%] | inconclusive |
| RexxCPS rate | -1.012927% [-1.390582%, -0.635273%] | adverse, below guard | +0.056966% [-0.935898%, +1.049830%] | inconclusive |

The common-five higher-is-better geometric means are both clearly favorable:

| VM | Paired mean | Mean 95% interval | Favorable pairs | Guard |
| --- | ---: | ---: | ---: | --- |
| `rxtvm` | +1.055583% | +0.465799% to +1.645367% | 26/36 | clear |
| `rxbvm` product | +0.939592% | +0.265290% to +1.613894% | 25/36 | clear |

No individual workload hits the 3% adverse guard and neither common-five
aggregate hits the 1% adverse guard. Remaining zero-crossing cells are final
noisy/inconclusive results at the 36-pair ceiling.

## Interpretation boundary

The cache is not entered by these ordinary single-thread workloads. Their
paired changes measure complete-product binary/code-layout and host-noise
effects of the F3C1 build, not direct cache execution. The Base64 improvement
therefore must not be attributed to the cache algorithm. The result supports
the narrower claim that the accepted task-launch gain does not conceal a
governance-level ordinary-product regression on this Apple M5 host.

This is a same-host F3C1 verification result, not a cross-platform or Release
claim. It adds no HTTP/TLS, lifecycle, RSS or public-provider conclusion.

## Product and host identity

- pre-cache `rxbvm`: 1,379,048 bytes,
  `a47c32bded25ef20924ccb1919d43d61734fc4118ee5c3d9af763cfb7fe2c00b`
- F3C1 `rxbvm`: 1,379,128 bytes,
  `0b46867f13175464722ab23f51b1376859b6881eea84d17452d8e87368e2d8ae`
- pre-cache `rxtvm`: 1,379,176 bytes,
  `9d549eca1b956cdb9238740cbbfd14c312f96ed58cfa1e82c4ef00ef9d242be2`
- F3C1 `rxtvm`: 1,379,240 bytes,
  `5e9339948c38ac8fcdd26dce60cfd2630190d6613385917e7f3d833d223e3394`
- ordinary profiling-off Release, `profile-20`, Apple Clang 21.0.0
- Apple M5, Darwin 25.5.0 arm64, 10 logical CPUs
- AC power, low-power mode off, no recorded thermal/performance/CPU-power
  warning before, during or after the campaign

## Bundle map

- `manifest.txt`: exact 28-cell control/candidate schedule;
- `timing/`, `timing/append-1/`, `timing/append-2/`: raw samples, outputs,
  capture manifests and per-block summaries;
- `timing/combined/summary.csv`: final 36-sample absolute cell summary;
- `paired-summary-36.csv` and `common-five-summary-36.csv`: final decision
  tables; earlier 12/24-pair tables retain the governed append decisions;
- `summarize_paired_36.crexx` and `summarize_common_five_36.crexx`: final
  Level B reducers;
- `pre-state.txt`, `post-state.txt` and `logs/`: provenance and command output;
- `COMMANDS.md`, `VALIDATION.md` and `SHA256SUMS`: replay and integrity.
