# Measurements

## Refreshed post-P05 ranking

The complete optimized/no-opt dual-VM refresh confirms the ranking in
`CANDIDATE-PANEL.md`. Counts below are N=2 exact sequence counts from the
optimized product. Rows overlap and are not additive.

| Candidate | Executions | Sites | Modules | Optimized/no-opt reading |
| --- | ---: | ---: | ---: | --- |
| `UNLINK; LINKREF` | 4,337,800 | 11 | 2 | persists at the same count; not created by inlining |
| `LINKATTR1; COPY; UNLINK` | 3,929,948 | 14 | 4 | mostly created by optimized inlining; exact List reference-getter slice is 3,820,600 at 5 sites |
| `ITOS; CONCAT` | 2,885,410 | 16 | 11 | 2,885,400 executions and 6 sites belong to RexxCPS alone |
| `DCOPY; DTOS` | 2,541,900 | 5 | 1 | single-workload decimal representation case |
| `ICOPY; BR` | 2,321,562 | 59 | 5 | enlarged by inlining; Base64 owns 1,366,001 at 9 dynamic sites |

Both VM modes report identical counts. The complete raw profiles and RXSEQ
N=2/3/4 captures are retained under `profiles/post-p05/`.

## Bounded PoC machine-work gate

The scratch public forms are measurement controls for the single-handler
ceiling. They do not select public ISA placement.

| Pair | Static executable instructions | Dynamic instructions | Exact reduction |
| --- | ---: | ---: | ---: |
| List R1 relink | 256 -> 247 | 39,276,326 -> 35,448,526 | 9 static; 3,827,800 dynamic |
| List R2 direct attribute copy | 256 -> 246 | 39,276,326 -> 31,635,126 | 10 static; 7,641,200 dynamic |
| List R1+R2 | 256 -> 237 | 39,276,326 -> 27,807,326 | 19 static; 11,469,000 dynamic |
| Base64 B1 copy/branch | 671 -> 648 | 46,724,369 -> 45,358,368 | 23 static; 1,366,001 dynamic |

The direct attribute form replaces five static
`LINKATTR1; COPY; UNLINK` sites with five canonical `copy_value` operations.
It materializes the reference descriptor, not the target `ListElement`.

## Smallest decisive profiling-off Release comparison

Times are medians of 15 serial recorded process samples after two warmups.
The runner retained stdout/stderr and correctness for every sample. Profile
elapsed time was not used as product timing.

### `rxvm`

| Pair | Control median | Candidate median | Delta | Relative MAD, control/candidate |
| --- | ---: | ---: | ---: | ---: |
| List R1 relink | 76.918 ms | 75.185 ms | -2.253% | 0.711% / 0.348% |
| List R2 direct descriptor materialization | 77.357 ms | 72.582 ms | -6.173% | 1.065% / 0.409% |
| List R1+R2 | 77.828 ms | 71.041 ms | -8.721% | 0.714% / 0.343% |
| Base64 B1 copy/branch | 312.968 ms | 313.963 ms | +0.318% | 7.442% / 5.295% |

### `rxbvm`

| Pair | Control median | Candidate median | Delta | Relative MAD, control/candidate |
| --- | ---: | ---: | ---: | ---: |
| List R1 relink | 86.077 ms | 84.680 ms | -1.623% | 0.622% / 0.155% |
| List R2 direct descriptor materialization | 86.101 ms | 80.802 ms | -6.154% | 0.647% / 0.297% |
| List R1+R2 | 86.053 ms | 79.510 ms | -7.603% | 0.492% / 0.208% |
| Base64 B1 copy/branch | 317.608 ms | 304.680 ms | -4.070% | 8.609% / 8.253% |

R1 and R2 each pass correctness, instruction reduction and dual-VM product
timing. The combined cell is consistent with two separable gains, but it is
not evidence for making one inseparable production edit. B1 reduces
instructions but has high dispersion and disagrees across VMs; it is neutral
for selection.

## Interpretation boundary

These are candidate-specific first comparisons, not a full formal portfolio,
cross-platform result or production Release verdict. The List benchmark's
arena remains the owner of its elements. The measured optimization changes
reference-descriptor transfer and local alias setup only; it does not replace
weak references with owning references or extend target lifetime.
