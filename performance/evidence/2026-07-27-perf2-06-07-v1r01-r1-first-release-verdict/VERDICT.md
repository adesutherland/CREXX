# First Release verdict

## Decision

**ACCEPT is recommended** for `PERF2-06-07-V1R01-R1`, pending Adrian's
mandatory decision. The rework restores the established ordinary-object path,
widens nested `§this` direct placement to the complete currently proved
single-return/fallthrough boundary, and preserves an already-proved receiver
alias when an inlined body is cloned again. Multiple explicit returns retain
the conservative materialisation/copyback path because the current summary
does not prove receiver-link balance on every exit.

The candidate remains isolated, provisional and uncommitted. No broad QA,
lifecycle/RSS sweep, package proof, production installation or follow-on
candidate has started.

## Exact mechanism proof

- Richards removes 111,606 dynamic instructions, 16,404,842 copy operations
  (22.378100%) and 130,345,888 copy bytes (22.393248%) per `-a 1` run on both
  VMs; static `copy` sites fall from 115 to 71.
- Permute `-a 50` removes 1,007,900 dynamic instructions, 10,078,400 copy
  operations (98.233830%) and 72,564,000 copy bytes (98.042488%) on both VMs;
  static `copy` sites fall from 19 to 3.
- Bounce `-a 100` is exact status quo: dynamic instructions, copy operations,
  copy bytes, allocations and allocation bytes are identical on both VMs.
  This dispositions the first candidate's accidental extra 1,000,000
  `COPY_REG_REG` operations.

## Capped formal result

All percentages are paired candidate change in process elapsed time; negative
is favorable. The interval is the two-sided 95% Student-t interval around the
mean paired percentage. No sample was removed. Governance-required unchanged
append blocks took every cell to the 36-pair cap.

| Workload | VM | Current median ms | Candidate median ms | Paired median | Mean 95% interval | Favorable pairs | Disposition |
| --- | --- | ---: | ---: | ---: | ---: | ---: | --- |
| Sieve | `rxvm` | 24.795 | 24.819 | -0.414% | [-0.759%, +0.522%] | 19/36 | layout guard, inconclusive |
| Sieve | `rxbvm` | 28.382 | 28.038 | -0.942% | [-4.350%, -0.322%] | 26/36 | clear favorable |
| Permute | `rxvm` | 83.033 | 34.658 | **-58.019%** | **[-59.175%, -54.785%]** | 36/36 | clear favorable |
| Permute | `rxbvm` | 88.502 | 38.397 | **-56.466%** | **[-57.387%, -56.325%]** | 36/36 | clear favorable |
| Bounce | `rxvm` | 41.887 | 41.760 | -0.085% | [-1.104%, +0.359%] | 20/36 | exact-work guard, inconclusive |
| Bounce | `rxbvm` | 52.162 | 51.652 | -1.251% | [-2.451%, -0.605%] | 26/36 | clear favorable |
| Richards | `rxvm` | 463.361 | 367.511 | **-21.224%** | **[-22.004%, -20.088%]** | 36/36 | clear favorable |
| Richards | `rxbvm` | 476.326 | 372.900 | **-21.076%** | **[-22.013%, -20.779%]** | 36/36 | clear favorable |
| Base64 | `rxvm` | 345.778 | 350.829 | +1.289% | [-1.836%, +4.935%] | 16/36 | noisy/inconclusive at cap |
| Base64 | `rxbvm` | 347.824 | 353.797 | -0.261% | [-3.439%, +6.006%] | 18/36 | noisy/inconclusive at cap |
| RexxCPS | `rxvm` | 960.379 | 930.104 | -3.657% | [-4.542%, -0.921%] | 24/36 | clear favorable |
| RexxCPS | `rxbvm` | 976.968 | 955.472 | -1.441% | [-3.569%, +1.068%] | 23/36 | noisy/inconclusive at cap |

The five-workload current/candidate geometric mean is favorable by `1.244352x`
for `rxvm` and `1.242301x` for `rxbvm`. All 888 formal executions (24 warmups
and 864 recorded) returned zero and matched the expected correctness marker.

## Guard scorecard

| Dimension | Result |
| --- | --- |
| correctness | pass: focused 10/10; formal 888/888 |
| five-workload common aggregate | pass: 1.244352x `rxvm`, 1.242301x `rxbvm` |
| comparable Tier A workload | pass: no clear adverse interval or paired median worse than 3% |
| artifact size | pass: changes range from -400 to +80 bytes |
| lifecycle | not run before the mandatory first-verdict stop |
| peak RSS | not run before the mandatory first-verdict stop |

## Decision boundary

This is one first ordinary profiling-off Release verdict on one Apple ARM64
host. Acceptance would authorize only the proportional closeout Adrian chooses.
It is not a cross-platform claim, final VM/default selection, `PERF2-06-D01`
decision, architecture approval for C2R03/V6R01, or authorization for
PERF2-08/09.
