# RCC-5B `rxfloat`: first Release verdict

Status: accepted by Adrian on 2026-08-20.

## Question and boundary

RCC-5B replaces the historical mixed `rxmath` native module with the
process-reentrant scalar `rxfloat` provider. It publishes canonical `rxfloat`
procedures plus direct `rxmath` scalar compatibility names, fixes the
two-argument `hypot` implementation, and adds `atan2`, `expm1`, and `log1p`.
RXBIN provider metadata selects the dynamic provider automatically; native
packaging uses its canonical static archive.

This verdict covers RCC-5B only. It does not close the separately implemented
RCC-5C integer/decimal libraries, start packed numeric storage, qualify
`rxstats`, or authorize RCC-5D+.

The retained control is the exact pre-edit Release `rxmath` provider, concrete
VMs, and NBody/CD images. The candidate uses the exact post-edit Release
`rxfloat` provider, concrete VMs, and images whose `rxmath` compatibility
declarations name `rxfloat` as their provider. Both variants use the same
current `library.rxbin`. Neither command supplies a native-plugin argument, so
all 208 executions exercise metadata-driven provider discovery.

## Host and method

- host: Apple M5 MacBook Air, 10 logical CPUs;
- OS: Darwin 25.5.0 ARM64;
- toolchain: Apple Clang 21.0.0;
- build: `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`;
- power: AC throughout, low-power mode off;
- thermal: no thermal, performance, or CPU-power warning before or after;
- pre-run load averages: 2.42, 2.74, 2.52;
- post-run load averages: 2.81, 2.65, 2.52;
- workloads: full NBody at 250,000 steps and CD at 10 aircraft across all 200
  frames, optimized and unoptimized, with `rxbvm` and `rxtvm` reported
  separately;
- sampling: one warmup and 12 serial pairwise-balanced recorded rounds for
  every control/candidate pair; and
- metric: parent-process elapsed time including process startup, image load,
  provider discovery/load, execution, and teardown.

The first attempted run overlapped material host activity, including sustained
AV-conferencing and Spotlight work. Adrian identified the host as not clear and
directed a complete rerun. That observation was discarded, was not merged with
this result, and is not retained as formal evidence. The exact candidate and
control hashes were rechecked before the clean-host rerun.

## Correctness

All 208 recorded and warmup executions exited successfully and printed the
expected `PASS: AWFY NBody` or `PASS: AWFY CD` result. Provider discovery
resolved `rx_rxmath` for the retained control and `rxfloat` for the candidate
without an explicit provider argument.

## Result

Negative percentages mean the candidate is faster.

| Workload / VM / mode | Paired mean | 95% CI | Paired median | 3% guard |
|---|---:|---:|---:|---:|
| NBody `rxbvm` opt | -0.111% | -0.716% to +0.494% | -0.309% | clear |
| NBody `rxbvm` noopt | +0.214% | -0.307% to +0.735% | +0.229% | clear |
| NBody `rxtvm` opt | -0.211% | -0.950% to +0.528% | -0.155% | clear |
| NBody `rxtvm` noopt | +0.584% | -0.551% to +1.718% | +0.569% | clear |
| CD `rxbvm` opt | -0.576% | -3.699% to +2.547% | -1.515% | clear |
| CD `rxbvm` noopt | +2.676% | -1.716% to +7.067% | +5.677% | clear |
| CD `rxtvm` opt | -2.141% | -5.999% to +1.716% | -2.316% | clear |
| CD `rxtvm` noopt | +0.302% | -3.913% to +4.518% | +0.961% | clear |

Every interval crosses zero and no paired-mean workload guard fires. CD is
noisier than NBody; in particular its unoptimized `rxbvm` paired median is
adverse while its paired mean remains below the guard and its interval spans
both directions. The selected optimized product `rxbvm` is neutral on both
workloads. Adrian accepted the result.

The concrete VM executable sizes are byte-identical before and after:
`rxbvm` is 1,413,512 bytes and `rxtvm` is 1,430,152 bytes. The canonical
dynamic provider falls from 53,088 bytes for `rx_rxmath.rxplugin` to 35,568
bytes for `rxfloat.rxplugin` (-33.0%). Compatibility artifact copies are
reported separately from the canonical provider and are not additional loaded
code.

## Retained files

- `manifest.txt`: exact pairwise matrix and command vectors;
- `samples.csv`: every warmup and recorded elapsed observation;
- `outputs.csv`: every captured child-output line;
- `cell-summary.csv`: absolute per-cell distribution;
- `paired-summary.csv`: paired deltas, intervals, and guard decisions;
- `capture-manifest.json`: capture timing and completion status;
- `artifact-sha256.txt`: exact control, candidate, and shared artifact identities;
- `QUALIFICATION.md`: retained post-verdict qualification and the RCC-5 QA
  cadence boundary;
- `summarize_paired.crexx`: Level B paired reducer; and
- `COMMANDS.md`: build, capture, reduction, and identity commands.

This is a bounded RCC-5B implementation verdict, not a new absolute release
baseline or an aggregate performance claim.
