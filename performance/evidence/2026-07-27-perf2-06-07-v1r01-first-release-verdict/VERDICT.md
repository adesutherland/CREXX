# First Release verdict

## Decision

`PERF2-06-07-V1R01` in its retained linear-leaf form is **not acceptable for
production as measured**. Bounce hits the 3% comparable Tier A guard on both
VMs, so the mandatory action is to stop for Adrian's explicit
rework/revert/accept-trade-off decision. No candidate source is installed in the
main worktree.

All percentages below are paired candidate change in process elapsed time;
negative is favorable. The interval is the 95% Student-t interval around the
mean paired percentage. No recorded sample was removed.

| Workload | VM | Current median ms | Candidate median ms | Paired median | Mean 95% interval | Favorable pairs | Disposition |
| --- | --- | ---: | ---: | ---: | ---: | ---: | --- |
| Sieve | `rxvm` | 24.011 | 24.023 | +0.125% | [-2.328%, +1.197%] | 4/12 | layout guard, inconclusive |
| Sieve | `rxbvm` | 27.239 | 27.266 | +0.063% | [-1.556%, +1.942%] | 5/12 | layout guard, inconclusive |
| Permute | `rxvm` | 81.844 | 34.180 | -58.401% | [-59.030%, -57.862%] | 12/12 | clear favorable |
| Permute | `rxbvm` | 87.115 | 37.562 | -56.857% | [-57.288%, -56.731%] | 12/12 | clear favorable |
| Bounce | `rxvm` | 41.419 | 72.826 | **+75.743%** | **[+74.532%, +77.748%]** | 0/12 | **clear adverse; guard hit** |
| Bounce | `rxbvm` | 50.459 | 84.610 | **+67.267%** | **[+65.670%, +68.497%]** | 0/12 | **clear adverse; guard hit** |
| Richards | `rxvm` | 454.587 | 355.938 | -21.526% | [-21.954%, -21.300%] | 12/12 | clear favorable |
| Richards | `rxbvm` | 460.350 | 362.790 | -20.965% | [-21.468%, -20.495%] | 12/12 | clear favorable |
| Base64 | `rxvm` | 334.676 | 342.674 | +7.669% | [-4.382%, +9.922%] | 5/12 | noisy/inconclusive |
| Base64 | `rxbvm` | 331.296 | 351.908 | +3.387% | [-5.190%, +11.075%] | 4/12 | noisy/inconclusive |
| RexxCPS | `rxvm` | 947.530 | 930.490 | -0.695% | [-2.371%, +1.565%] | 7/12 | inconclusive |
| RexxCPS | `rxbvm` | 939.833 | 937.353 | +0.911% | [-1.809%, +1.242%] | 5/12 | inconclusive |

Base64's absolute distributions and the `rxvm` RexxCPS distribution meet the
formal append recommendation. They are retained and labelled inconclusive.
They were not appended because the unchanged implementation already produced a
large, clear, independently causal Bounce guard hit; further control sampling
cannot rescue this candidate's first verdict.

## Guard scorecard

| Dimension | Result |
| --- | --- |
| correctness | pass: 10/10 focused tests; 312/312 formal executions |
| five-workload common geometric mean | favorable: 1.111675x `rxvm`, 1.105427x `rxbvm` |
| comparable Tier A workload | **fail: Bounce +75.743%/+67.267% paired medians** |
| artifact size | pass: largest increase is 24 bytes for Bounce; library is +8 bytes |
| lifecycle | not run after decisive first-verdict throughput failure |
| peak RSS | not run after decisive first-verdict throughput failure |

## Causal review

The retained patch adds the leaf/control-flow restrictions in
`inline_method_receiver_can_share_actual()`, but places them in the predicate's
common tail. They therefore restrict both:

1. the new nested `§this` eligibility that needs the linear-leaf proof; and
2. the already accepted ordinary direct-object receiver eligibility that did
   not need that restriction.

In Bounce, the second effect changes the inlined `ball.bounce()` body from
direct receiver sharing to one `copy r16,r8` plus one `copy r8,r16`. The loop
executes the new copy-in exactly 1,000,000 times. Both count VMs report the same
result:

| Mechanism | Current | Candidate | Delta |
| --- | ---: | ---: | ---: |
| dynamic instructions | 21,139,726 | 22,139,726 | +1,000,000 |
| `COPY_REG_REG` | 10,201 | 1,010,201 | +1,000,000 |
| all value operations | 2,133,272 | 8,133,291 | +6,000,019 |
| value bytes | 11,149,175 | 51,149,295 | +40,000,120 |
| allocation requests | 260,520 | 260,533 | +13 |

This assigns the failure to compiler predicate placement. A possible rework is
to apply the leaf restrictions only when `receiver_symbol->is_this`, preserving
the existing ordinary receiver path. That rework has not been made or timed; it
requires Adrian's explicit selection as the next candidate identity.
