# PERF3-13 Gate E E4b first ordinary-Release verdict

Date: 2026-08-11

Branch: `develop`

Source: `99753ba544277ccbf1baecedd35b00a859d543f2` plus the frozen,
uncommitted E4a/E4b implementation and tests.

Status: **accepted; Mac closeout complete**. Adrian accepted the guard-clean
first ordinary-Release verdict and authorized proportional closeout on
2026-08-11.

## Candidate boundary and structural result

E4b adds an internal runtime-owned, reference-counted program-generation
catalogue for bytecode modules. Sealed generations share immutable module
files, canonical instructions, constant pools, semantic graphs, names and
descriptions. Per-worker module overlays, globals, procedure runtimes, frames,
execution images, graph bindings and caches remain private. Generations are
append-only and are reclaimed only after the current reference and all worker
pins are gone. Native/plugin modules remain outside this first sharing slice.

The retained control found 2,480 bytes of duplicated immutable instruction and
constant data across two contexts for the test image. E4b removes all 2,480
audited duplicate bytes while retaining the 569-byte per-context overlay floor.
Shared identity, overlay isolation, late generation transition and teardown
pass under product `rxvml`, explicit `rxbvml` and test direct-threaded
`rxtvml`. The test RXBIN hashes are unchanged from E4a.

## First ordinary-Release verdict

The exact same-session controls are the accepted E3b-P2/E4a VMs. Control and
candidate are ordinary profiling-off `Release`, `profile-20` products. One
warmup and 12 pairwise-balanced recorded rounds covered VM lifecycle, Sieve and
canonical RexxCPS under both concrete engines. All 156 processes passed their
correctness oracle.

Positive elapsed percentages are adverse; positive RexxCPS rates are
favourable.

| Workload | VM | Paired mean | Mean 95% interval | Result | Guard |
| --- | --- | ---: | ---: | --- | --- |
| VM lifecycle | `rxbvm` | -2.155% | -6.011% to +1.701% | append required | clear |
| VM lifecycle | `rxtvm` | -0.640% | -1.783% to +0.503% | append required | clear |
| Sieve | `rxbvm` | +0.374% | +0.140% to +0.609% | clear adverse, small | clear |
| Sieve | `rxtvm` | -1.020% | -2.161% to +0.122% | inconclusive | clear |
| RexxCPS rate | `rxbvm` | +0.584% | -0.081% to +1.248% | inconclusive | clear |
| RexxCPS rate | `rxtvm` | -1.757% | -4.492% to +0.979% | append required | clear |

The reducer mechanically requested an append for the complete lifecycle groups
and the `rxtvm` RexxCPS pair. Twelve additional balanced rounds, without a
second warmup, all passed and removed every rerun recommendation:

| Workload | VM | Append paired mean | Mean 95% interval | Result | Guard |
| --- | --- | ---: | ---: | --- | --- |
| VM lifecycle | `rxbvm` | -0.180% | -0.868% to +0.507% | inconclusive | clear |
| VM lifecycle | `rxtvm` | -0.577% | -1.025% to -0.129% | clear favourable | clear |
| RexxCPS rate | `rxtvm` | +0.454% | -0.956% to +1.864% | inconclusive | clear |

The only clear adverse hot result is the `rxbvm` Sieve point at +0.374%, far
inside the 3% guard. The single-worker verdict is therefore neutral and no
performance guard fires.

## RSS and artifact cost

The lifecycle-only RSS panel ran 12 recorded processes per cell. `rxbvm`
control and candidate medians are identical at 18,096,128 bytes. The `rxtvm`
candidate median is 49,152 bytes lower than control (18,104,320 versus
18,153,472 bytes), about 0.27% favourable.

The candidate files grow by 1,328 bytes for `rxbvm` and 1,312 bytes for
`rxtvm`, about 0.12%. Each `__text` section grows by 3,244 bytes; aligned
Mach-O segment sizes do not change. This is a small explicit code-size cost and
does not trigger the artifact guard.

## Provenance and interpretation boundary

Control hashes are `4d3dd2b1...fac1b` (`rxbvm`) and `ab25c6ca...dbf0`
(`rxtvm`). Candidate hashes are `cbf43248...39b88` and
`3a5cd290...7e68`. Full hashes and sizes are retained in `ARTIFACTS.txt`.

Host: Apple M5, 10 logical CPUs, on AC power with low-power mode disabled. No
thermal, performance or CPU-power warning and no competing build, test or VM
process was observed before or after capture.

The capture manifest's launcher version comes from the already-built `crexx`
orchestrator. Candidate identity is established by the exact candidate VM
hashes above; the launcher neither supplies nor replaces the measured VM.

This is a same-host, single-worker result. It does not claim multi-worker
scaling and it does not authorize commit or publication. Raw samples, outputs,
manifests, reducer output, correctness, host state and binary inspection are
retained in this directory.

## Mac closeout

- The complete normal-Debug build passed. Focused Debug passed 11/11 across
  the E4 controls, worker lifecycle, reentrancy, both dispatch contracts,
  Level B late load and the optimizer barrier.
- The supported Apple AddressSanitizer E4 panel passed 3/3 through
  `tools/asan-run.sh`. Apple LeakSanitizer is unavailable, so the runner used
  `detect_leaks=0`; normal Debug teardown retained the exact zero-live-
  allocation assertion.
- The full Debug suite passed 2,037/2,037 with `--parallel 30` in 247.69
  seconds.
- The complete ordinary profiling-off Release rebuild and focused Release
  panel passed 11/11.
- Rebuilt Release hashes remain exactly
  `cbf432480553c2956e751d9d419562c2f5ce3151a7442e8b0ab4c7252d339b88`
  for `rxbvm` and
  `3a5cd290f5e20c1db797fe15e921679a290f74a4cf6236196b7d4c63bd0b7e68`
  for `rxtvm`. They are byte-identical to the accepted timing candidate, so no
  timing rerun is warranted.

E4 is complete on Mac. Portable proof, E5 persistent trusted workers, E6
scale/reclamation selection, public workers/channels and Gate F remain
separately gated.
