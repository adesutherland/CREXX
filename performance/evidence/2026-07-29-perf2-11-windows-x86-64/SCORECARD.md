# PERF2-11 Windows x86-64 scorecard

Status: **complete Windows baseline; no final architecture selection**

Higher throughput ratios are better. All comparison ratios are same-session
medians from the profiling-off Release product.

## Correctness

| Gate | Result |
| --- | ---: |
| Focused regression set | 15/15 |
| Full Release CTest | 1,926/1,926 in 565.48 s |
| Initial formal timing | 348/348 correct samples |
| Timing variability append | 132/132 correct samples |
| Formal RSS | 87/87 correct samples |
| Lifecycle | 160/160 correct phase samples |

## Common timing

The common aggregate contains exactly Sieve, Permute, Bounce, Richards and
Base64.

| VM | versus ooRexx geometric mean | versus decimal NetRexx geometric mean |
| --- | ---: | ---: |
| `rxvm` | **2.363219x** | **0.749453x** |
| `rxbvm` | **1.984737x** | **0.629424x** |

| Workload | `rxvm` / ooRexx | `rxbvm` / ooRexx | `rxvm` / NetRexx | `rxbvm` / NetRexx |
| --- | ---: | ---: | ---: | ---: |
| Sieve | 6.879705x | 5.651418x | 1.933114x | 1.587980x |
| Permute | 9.687526x | 7.387701x | 0.597138x | 0.455377x |
| Bounce | 5.058100x | 3.669743x | 1.985833x | 1.440758x |
| Richards | 0.335528x | 0.330456x | 0.167660x | 0.165125x |
| Base64 | 0.651659x | 0.608274x | 0.615201x | 0.574243x |

The geometric mean clears 2.00x ooRexx for `rxvm`, but the per-workload final
claim gate does not pass: Richards and Base64 remain below ooRexx in both VMs,
and `rxbvm` is below 2.00x on the aggregate. NetRexx remains faster on Permute,
Richards and Base64.

Eleven cells received the policy-required ten-sample append. Their merged MAD
is 0.78-2.31%; several still carry the runner's min/max-span flag. Raw samples
are retained, and no sample was removed.

## Separate rows

Towers is a qualified object/allocation row outside the common aggregate.

| Comparison | `rxvm` | `rxbvm` |
| --- | ---: | ---: |
| Towers versus ooRexx | 0.385129x | 0.382994x |

RexxCPS is the separately disclosed cREXX 2.2d diagnostic against canonical
Classic 2.2 and the NetRexx 2.2n adaptation.

| Runtime / VM | Median clauses/s |
| --- | ---: |
| cREXX `rxvm` | 9,441,205.5 |
| cREXX `rxbvm` | 8,759,043.5 |
| ooRexx | 15,785,329.5 |
| Regina | 15,192,167.0 |
| NetRexx | 16,904,199.5 |

The cREXX/Regina ratios are 0.621452x for `rxvm` and 0.576550x for `rxbvm`.
Because the cREXX and NetRexx sources are disclosed adaptations, RexxCPS does
not enter the common aggregate.

## Peak working set

The common-five geometric means are:

| Runtime / VM | Peak working-set geometric mean |
| --- | ---: |
| cREXX `rxvm` | 13.81 MiB |
| cREXX `rxbvm` | 13.80 MiB |
| ooRexx | 23.30 MiB |
| NetRexx / HotSpot | 245.12 MiB |

Regina's RexxCPS median is 5.95 MiB. The valid Windows sampler measures the
direct child process's `PeakWorkingSet64` every 10 ms; PowerShell wrapper
memory is excluded.

## Lifecycle

Lifecycle phases are process-inclusive and separate from steady-state timing.

| Runtime / VM | Phase | Median |
| --- | --- | ---: |
| cREXX shared | compile | 223.818 ms |
| cREXX shared | assemble | 26.395 ms |
| cREXX `rxvm` | load to first result | 6.860 ms |
| cREXX `rxbvm` | load to first result | 6.710 ms |
| ooRexx | translate | 17.175 ms |
| ooRexx | load to first result | 16.530 ms |
| NetRexx | compile | 1,258.609 ms |
| NetRexx | load to first result | 59.412 ms |

## Artifacts

| cREXX Release product | Bytes |
| --- | ---: |
| `rxc.exe` | 6,021,583 |
| `rxas.exe` | 821,781 |
| `rxlink.exe` | 243,156 |
| `rxvm.exe` | 3,636,842 |
| `rxbvm.exe` | 3,609,194 |
| `library.rxbin` | 862,552 |

`rxvm.exe` is 27,648 bytes (0.766%) larger than `rxbvm.exe`. Exact hashes and
all benchmark RXAS/RXBIN, comparator and generated NetRexx artifact sizes are
in `artifacts.csv`.

## Boundary

This is a Windows scorecard, not a tuning verdict. No profiling was performed,
no candidate/default VM was selected, and absolute Windows timings are not
paired against Mac or Linux sessions. PERF2-11 Gate E remains open because
supported Linux ARM64 coverage is still required.
