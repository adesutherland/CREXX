# Validation record

- Source product: committed `develop` F3C1 `9e7d600c8`; evidence/roadmap-only
  dirty scope during timing.
- Build: ordinary Release, profiling off, `profile-20`; retained control and
  candidate VM hashes were unchanged before and after all blocks.
- Workloads: current optimized Sieve, Permute, Bounce, Richards, Base64,
  Towers and canonical RexxCPS images; one common current `library.rxbin`.
- Sampling: initial one warmup and 12 balanced pairs; two unchanged 12-pair
  governance appends; serial execution and no removed sample.
- Correctness: 1,036/1,036 processes pass; runner and reducer stderr logs are
  empty.
- Uncertainty: final 36-pair Student-t intervals use 35 degrees of freedom;
  remaining zero-crossing cells are final noisy/inconclusive observations.
- Guards: no 3% individual-workload adverse hit and no 1% common-five adverse
  hit. Task launch remains separately reported in the F3C1 first verdict.
- Environment: Apple M5/Darwin 25.5.0 arm64, AC power, low-power mode off, no
  recorded thermal/performance/CPU-power warning, no overlapping build/test or
  second benchmark campaign.
- Interpretation: same-host task-cache verification only; no cross-platform,
  release-wide, HTTP/TLS, lifecycle or RSS claim.
