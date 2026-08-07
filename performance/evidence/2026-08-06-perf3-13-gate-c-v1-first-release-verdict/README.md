# PERF3-13 Gate C V1 first Release verdict

This is the mandatory short first verdict for removing the 32-byte inline
string from `value` on the accepted S0/R0 allocator. It is an isolated scratch
PoC, not a production ABI or final value-shape selection.

## Outcome

- V0 is exactly 240 bytes and V1 is exactly 208 bytes under the focused
  compiler-layout test.
- Both ordinary profiling-off Release products pass the three focused tests:
  `rxvm_product_entry_point`, `rxvmmemory_allocator` and
  `ts_regvalue_tester`.
- The fresh V0 `rxbvm` has an exactly identical 203,865-line `__text` body to
  retained S0. Both normalized disassemblies hash to
  `253af30970ff83d616923864ce9f029eaeaa88bda3bf44fe8150573d51490235`.
- All 70 timing executions pass: 14 cells across seven workloads, one warmup
  and four recorded rounds per cell. The 56 recorded observations are paired
  by workload and round with alternating position.
- The stable-six geometric aggregate is `1.010915174`, or about **+1.09%**
  for V1 over V0. Base64 remains recorded but non-selecting.

Positive percentages below are favorable to V1: reduced elapsed time except
for RexxCPS, where they mean increased benchmark rate.

| Workload | Paired median | 95% mean classification | Favorable pairs |
|---|---:|---|---:|
| Sieve | +1.593211% | clear favorable | 4/4 |
| Permute | -0.549362% | noisy/inconclusive | 1/4 |
| Bounce | -1.964423% | noisy/inconclusive | 0/4 |
| Richards | +3.589034% | clear favorable | 4/4 |
| Base64 | -0.063439% | noisy/inconclusive; non-selecting | 2/4 |
| Towers | +1.110094% | noisy/inconclusive | 3/4 |
| RexxCPS | +1.774387% | clear favorable | 4/4 |

There is no clear-adverse workload. This short screen supports promoting V1
to the formal V0/V1 survivor comparison; it does not yet select V1. Separate
bounded RSS/allocator telemetry, a formal timing panel and the simplicity
scorecard remain behind Adrian's acceptance of this first verdict. V2a,
`rxtvm`, reclamation and Gate D remain closed.

## Correctness finding

The first scratch timing attempt stopped at V1 RexxCPS. Removing inline storage
exposed a hidden dependency in the nine-digit integer-to-decimal zero path:
`extract_integer_decimal()` wrote `"0"` without first obtaining string
storage. A native backtrace identified a null store in `run`. The correction
prepares the minimum string sidecar only in the no-inline layout and adds a
direct zero-conversion regression. V0 therefore emits no new instruction and
retains exact S0 machine identity.

The failed capture also exposed separate runner debt: the compact runner
recorded exit code zero after the child actually terminated with status 139.
The runner is unchanged in this slice; the issue is retained for correction
before relying on it to classify abnormal child termination.

One later scratch panel was also rejected because an unconditional logical
no-op in V0 had not yet been proved machine-identical. Neither rejected panel
has timing authority or contributes samples here.

## Evidence map

- `timing/`: the one authoritative raw timing capture and paired reductions.
- `tests/`: focused V0 and V1 Release correctness logs.
- `diagnostics/rexxcps-zero-conversion/`: the pre-fix output, native register
  trace and passing post-fix output.
- `manifests/value-layout-v0-v1.txt`: exact workload commands.
- `source-identities.csv` and `build-configs.txt`: source, binary and build
  identities.
- `pre-state.txt` and `post-state.txt`: AC-power, thermal and process-state
  records.

