# Current combined attribution

The selected matrix uses current optimized and no-opt linked images for Sieve,
Permute, Bounce, Richards, Base64 and RexxCPS on both VMs. The ordinary Release
product has `CREXX_VM_PROFILING=OFF`; the separately built counts product has
`CREXX_VM_PROFILING=ON`. All 24 profile cells are schema-5 complete. Operation
counts below are deterministic for the frozen image and arguments.

## Steady-state attribution control

One warmup and three serial recorded samples were taken per cell. These small
controls rank mechanisms; they do not meet the 12-pair first-Release-verdict
protocol.

| Workload | `rxvm` no-opt ms | `rxvm` opt ms | delta | `rxbvm` no-opt ms | `rxbvm` opt ms | delta |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sieve | 13.751 | 12.739 | -7.359% | 16.579 | 16.542 | -0.223% |
| Permute | 31.519 | 69.229 | +119.642% | 37.781 | 76.286 | +101.916% |
| Bounce | 43.554 | 29.230 | -32.888% | 55.064 | 41.064 | -25.425% |
| Richards | 24.326 | 436.204 | +1,693.160% | 29.453 | 448.990 | +1,424.429% |
| Base64 | 370.151 | 321.907 | -13.034% | 464.125 | 325.763 | -29.811% |
| RexxCPS | 987.452 | 940.089 | -4.796% | 971.191 | 955.879 | -1.577% |

## Current exact mechanism counts

Except for RexxCPS's VM-dependent timed-loop count, `rxvm` and `rxbvm` execute
the same frozen streams and report the same mechanism counts. The table shows
optimized `rxvm`; the raw summary contains both VMs and all no-opt controls.

| Workload | instructions | calls | copy ops / bytes | move ops / bytes | clear/reset/destroy ops / bytes | allocations / bytes | frame high-water |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Sieve | 6,825,375 | 0 | 52 / 10 | 0 / 0 | 506,584 / 120,209 | 8,319 / 4,231,352 | 1 |
| Permute | 12,906,826 | 432,950 | 10,259,602 / 74,012,810 | 0 / 0 | 2,893 / 2,645 | 866,526 / 239,912 | 7 |
| Bounce | 21,139,726 | 10,100 | 1,203,002 / 9,542,411 | 10,100 / 400,000 | 920,170 / 1,206,764 | 260,520 / 83,328,648 | 3 |
| Richards | 8,726,851 | 119,157 | 73,307,574 / 582,076,729 | 99,035 / 0 | 865,839 / 4,011,486 | 738,482 / 173,528,696 | 6 |
| Base64 | 46,724,369 | 500 | 2,221,507 / 18,789,128 | 500 / 512,000 | 1,382 / 1,075,088 | 1,700 / 608,160 | 2 |
| RexxCPS | 27,841,418 | 143,642 | 1,109,124 / 48,425,897 | 2,624 / 8,046 | 1,641,006 / 71,902,353 | 1,515,811 / 274,924,728 | 5 |

RexxCPS optimized `rxbvm` reports 26,448,437 instructions, 136,642 calls,
1,053,624 copies/45,983,897 bytes, 1,558,506 clear/reset/destroy operations/
68,272,386 bytes, 3,009,511 conversions and 1,439,811 allocation requests/
261,060,728 bytes. RexxCPS is a self-calibrating smoke cell; these exact counts
belong to the retained final run rather than a cross-session invariant.

The optimized/no-opt contrast identifies the owner:

- Richards grows from 119,629 copies/83,745 bytes to 73,307,574/
  582,076,729. Final native samples put 633/619 top-of-stack samples in
  `copy_value` for `rxvm`/`rxbvm`; high-water malloc
  histories put the largest live high-water stack under recursive
  `copy_value`. The optimized RXAS copies the inlined receiver into and back
  out of `§this` around receiver-mutating paths.
- Permute grows from 181,052 copies/1,448,010 bytes to 10,259,602/
  74,012,810. Its optimized RXAS likewise copies the object receiver before
  and after the inlined `swap` body. `COPY_REG_REG` itself executes 1,008,001
  times; recursive payload copying expands the exact helper work.
- Bounce improves despite growing from 171,302 copies/400,011 bytes to
  1,203,002/9,542,411. Residual reference traffic remains large in both
  modes: optimized `MKREF` executes 510,000 times, `LINKATTR1` 4,222,600,
  `UNLINK` 3,065,000, `UNLINKN` 1,000,000 and `MINLINK*` 510,000. The native
  sample still sees reference identity/release helpers, but current evidence
  does not isolate one redundant, statically safe root/tree operation.
- Base64 optimized executes 2,907,503 `SCOPY` and 1,370,502 `STRLEN` operations.
  RexxCPS optimized `rxvm` executes 4,258,578 `SCOPY`, 1,054,500 each of
  `DCOPY` and `DTOS`, 788,500 `STOD`, 282,105 `STRLEN` and 1,197,006 `ITOS`.
  These conversion opcodes materialize; the product has no broad retained-
  representation hit/miss mechanism to count.

## Allocation, lifetime and ownership

Full-mode malloc high-water captures separate churn from retained capacity:

| Cell | VM | high water | current | cumulative | allocations | frees |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| Richards opt, argument 30 | `rxvm` | 1.5 MB | 1.5 MB | 187.0 MB | 356,293 | 354,058 |
| Richards opt, argument 30 | `rxbvm` | 1.5 MB | 1.5 MB | 195.6 MB | 372,789 | 370,554 |
| RexxCPS opt, smoke 5 | `rxvm` | 1.7 MB | 1.2 MB | 16.7 MB | 68,076 | 67,181 |
| RexxCPS opt, smoke 5 | `rxbvm` | 1.7 MB | 1.2 MB | 14.9 MB | 58,524 | 57,628 |

The high-water capture places Richards' largest live allocation stack under
recursive `copy_value` (393 calls/779,712 bytes in both VMs). The general
`value` object is 248 bytes and appears in the allocator as a rounded 256-byte
request. Cumulative operations are the capture-time history, while the
schema-5 profile above supplies full-cell request counts. High churn with low
stable high water does not prove a new procedure-affine capacity owner; it
instead strengthens the narrower case for preventing the full-object copies
that create the churn.

Frame entry/return work is material in call-heavy streams, but this panel found
no new value/reference contract that authorizes another reset or mapping
ledger. Sieve's zero calls remain the generic frame/layout guard.

## Native hardware, RSS and lifecycle controls

The most decisive Apple hardware control is Richards:

| VM | mode | instructions retired | cycles | max RSS bytes |
| --- | --- | ---: | ---: | ---: |
| `rxvm` | no-opt | 438,325,102 | 105,421,499 | 7,340,032 |
| `rxvm` | optimized | 7,024,243,601 | 1,895,751,474 | 8,896,512 |
| `rxbvm` | no-opt | 504,969,275 | 116,773,916 | 7,323,648 |
| `rxbvm` | optimized | 7,085,663,399 | 1,927,360,918 | 8,880,128 |

The same control shows the optimized Permute instruction count roughly
doubles. Bounce reference helpers, RexxCPS decimal/string helpers and Base64
string work remain visible but do not beat the Richards/Permute compiler-owned
ceiling. RSS for the full selected matrix is retained separately under
`raw/rss/` and was not blended into steady-state timing.

Five final-product maintained Level B lifecycle rounds produced medians of
76.688 ms to compile, 7.146 ms to assemble, 2.999 ms for `rxvm`
load-to-first-result and 2.892 ms for `rxbvm`. Assemble and both VM load
samples are marked for rerun if lifecycle becomes decision-critical. Teardown is represented by
deterministic clear/reset/destroy counts and the nearly balanced malloc
histories; no standalone teardown-latency claim is made.

## Artifacts

| Workload | source bytes | no-opt RXAS / RXBIN / linked | opt RXAS / RXBIN / linked |
| --- | ---: | ---: | ---: |
| Sieve | 983 | 10,190 / 5,372 / 5,356 | 9,774 / 4,864 / 4,840 |
| Permute | 1,313 | 14,291 / 7,544 / 7,592 | 29,984 / 11,925 / 11,909 |
| Bounce | 2,230 | 23,081 / 11,323 / 11,763 | 45,362 / 17,175 / 17,215 |
| Richards | 9,403 | 106,117 / 39,724 / 39,644 | 255,919 / 79,158 / 79,150 |
| Base64 | 4,437 | 46,933 / 19,161 / 31,232 | 114,978 / 38,233 / 38,417 |
| RexxCPS | 12,180 | 127,325 / 48,931 / 142,651 | 195,576 / 68,446 / 159,739 |

The ordinary Release executables are 998,904 bytes (`rxvm`) and 999,064 bytes
(`rxbvm`). Each has 884,736-byte `__TEXT`, 49,152-byte `__DATA_CONST` and
16,384-byte `__DATA` segments. Product, source and image hashes are in
`PROVENANCE.md` and `SHA256SUMS`.
