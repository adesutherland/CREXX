# RCC-3 module initializers: first Release verdict

Status: accepted by Adrian on 2026-08-19.

## Question and boundary

RCC-3 adds a published-module-count comparison to bytecode call paths so calls
made while initialization is pending can force the callee module ready or fail
closed. When every loaded module is published, the handler skips the slow
initializer helper. This bounded verdict asks whether that steady-state check
causes a material call/argument regression.

The baseline is local commit
`6a7960e8c1cc89fa3272a60605cc52f50883a369` before the RCC-3 production edit.
The candidate is the approved RCC-3 working tree later committed in repository
history. Both products are ordinary profiling-off Release builds on macOS
ARM64, using the same exact optimized call/argument control image and runtime
library image. Source/TRACE metadata was retained exactly as produced in that
shared image; it was not rebuilt between variants.

## Host and method

- host: Apple M5 MacBook Air, 10 logical CPUs;
- OS: Darwin 25.5.0 ARM64;
- build: `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`;
- workload: `rexxcps_family_controls.crexx`, separately named `call-arg`
  attribution control, 5,000,000 iterations;
- correctness: every process reported checksum `220000000` and
  `PASS: RexxCPS family control`;
- sampling: one warmup per VM/variant, then 12 serial balanced pairs per
  concrete VM, alternating baseline/candidate order;
- metric: high-resolution parent-process elapsed time, including process
  startup and image load; no benchmark-native time is claimed.

The host was explicitly available for performance work, but this bounded first
verdict did not record a formal AC/low-power/thermal panel. The paired result is
therefore an implementation gate, not a new absolute baseline or release
scorecard.

## Result

| VM | Baseline median (s) | Candidate median (s) | Median change | Paired mean | 95% CI |
|---|---:|---:|---:|---:|---:|
| `rxbvm` | 0.973712325 | 0.969355464 | -0.447% | -0.353% | -1.882% to +1.176% |
| `rxtvm` | 0.973719954 | 0.979417443 | +0.585% | +0.179% | -0.775% to +1.133% |

Both rows are neutral/inconclusive around zero and remain well inside the 3%
individual-workload guard. Adrian accepted the guard-clean verdict.

The candidate executable growth also remains below 3%: `rxbvm` grows from
1,379,592 to 1,413,512 bytes (+2.459%), and `rxtvm` from 1,396,232 to
1,430,152 bytes (+2.429%). The growth includes the initializer lifecycle and
failure machinery, not only the steady-state comparison.

## Artifact identities

| Artifact | SHA-256 |
|---|---|
| shared call-argument RXBIN | `20b8e606325abe10ae0921c968923f862eb601f0e633c567e96ae55833d3e4c1` |
| shared runtime library RXBIN | `148e7c04b5646d1f2f5dc1e60efe9cf05c325fb4f02d529fad80c13da5b5b00a` |
| baseline `rxbvm` | `d854a90d0181ad0d370ff04e3307b1753623dda88b4a1f1e57aa225e952cd3b1` |
| candidate `rxbvm` | `1d3629d507675d4a9b8b96c5c024b4a5ce521b62d85a53764421b58cd1492c61` |
| baseline `rxtvm` | `1626a4b044a3979cfd6ddaf92168fe3c41084108ed98eca75b806e393119b655` |
| candidate `rxtvm` | `c5fffff850d636e2c013d8f2be04025bf3e9714f9805a615701e3b6ef9b3dde8` |

Raw paired samples are in `call-arg-hires-samples.csv`; the derived result is
in `call-arg-summary.csv`. Exact commands are retained in `COMMANDS.md`.
