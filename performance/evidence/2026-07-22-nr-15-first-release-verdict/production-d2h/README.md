# NR-15 D2-hybrid first ordinary-Release verdict

Status: **favorable; stopped for Adrian's accept/rework/revert decision**

The selected D2-hybrid is integrated end to end: versioned little-endian hash,
chain and generation metadata lives in the stem receiver's ordinary binary;
VM attributes own keys, values and the current default. Canonical opcodes
641-649 are protected by RXBIN 007 feature bit `0x00000004`. The compiler emits
the direct path only for a proved concrete `rxfnsb.stem` in simple storage;
class attributes and complex receivers retain ordinary call/copyback behavior.

## Correctness and compatibility gate

- Focused Debug: 22/22 pass across reviewed compiler goldens, TRACE ordering,
  classic stem/multi-tail compatibility, NR-15 semantics/access, optimized and
  `-n`, and both VMs.
- Focused Release: 10/10 pass for canonical RexxCPS smoke plus the NR-15
  semantic/access matrix; opcode metadata reports 650 total, 585 classified,
  six conservative and no missing new form.
- Public `stem.hash()` retains its polynomial `% 256` result. Explicit empty
  keys, omitted-key reset, generations, insertion order, iterator snapshots,
  references, deep copy/move, Unicode byte equality, two-segment native ops,
  source/TRACE order and invalid-index signals are covered.
- Optimized linked images carry feature words `0x00000005` for the access
  workload and `0x00000007` for RexxCPS; the retained Candidate A images remain
  executable in the new VMs.

## Formal profiling-off Release timing

The Level B matrix driver ran one warmup and 12 recorded serial rounds per
cell. Candidate A and D2-hybrid, `rxvm` and `rxbvm`, rotate starting position
within every workload. No result failed, no sample was removed, no absolute
noise trigger fired, and every mean 95% interval is wholly favorable.

| Workload | Metric | `rxvm` paired median | `rxbvm` paired median | Favorable |
| --- | --- | ---: | ---: | ---: |
| 60,000,000 get hits, 64 entries | process elapsed | -76.839% | -75.438% | 12/12 each |
| 400,000 histogram passes | process elapsed | -32.027% | -31.885% | 12/12 each |
| canonical RexxCPS 2.2d | native CPS | +10.888% | +10.919% | 12/12 each |
| canonical RexxCPS 2.2d | process elapsed | -9.768% | -9.828% | 12/12 each |

RexxCPS median native rates are 5,581,482.5 to 6,183,779.5 CPS (`rxvm`) and
5,052,647 to 5,617,810 CPS (`rxbvm`). These are same-host observations for the
named dirty frozen product, not release-wide claims.

## Static and dynamic work

The optimized access RXAS falls from 503 to 417 executable instructions and
25 to six call sites; `-n` falls from 486 to 381. Canonical optimized RexxCPS
falls from 1,618 to 1,564 static instructions and 89 to 77 call sites.

Schema-4 profiles use identical work on each before/after pair and are not
wall-clock claims:

| Profile | Instructions | Direct bytecode calls | Copy opcodes |
| --- | ---: | ---: | ---: |
| get hit | 53,874 -> 15,396 (-71.422%) | 1,067 -> 3 | 2,460 -> 1,259 |
| histogram | 1,851,581 -> 1,117,409 (-39.651%) | 21,071 -> 1,004 | 107,466 -> 77,259 |
| canonical RexxCPS | 317,993,134 -> 251,052,811 (-21.051%) | 8,663,479 -> 6,583,479 | 25,742,969 -> 24,912,952 |

The get-hit profile also reduces frame activations 1,068 to four and allocated
value slots 754 to 325. The candidate adds two ordinary binary-buffer
allocations (6,144 profiled bytes) for that stem while substantially reducing
the former attribute/value object graph. A discarded smoke-profile setup is
not retained because RexxCPS calibration selected unequal work; the retained
profiles all prove canonical `100 x 100` effective work.

## Lifecycle, RSS and artifacts

The process-inclusive load/64-entry construction/first-access diagnostic has
paired medians -1.365% (`rxvm`, noisy interval crossing zero) and -2.659%
(`rxbvm`, clear favorable). The required ten-sample absolute-noise append is
retained; its high min/max span persists, but the candidate is faster and the
median differences are only 45-70 microseconds, so the lifecycle guard of both
5% and 1 ms is not hit.

Canonical three-sample peak-RSS medians fall 81,920 bytes (`rxvm`) and 98,304
bytes (`rxbvm`), so the RSS guard is not hit. The optimized access image shrinks
1,200 bytes (-7.004%), canonical RexxCPS shrinks 1,192 bytes (-0.613%), and the
linked library shrinks 1,528 bytes (-0.178%); no artifact guard is hit.

## Recommendation and stop

Recommendation: **accept D2-hybrid and proceed to the post-verdict quality
closeout**. It is mathematically semantics-preserving on the covered contract,
strictly reduces relevant static and dynamic work, materially improves both
focused and independent end-to-end workloads in both VMs, and introduces no
lifecycle, RSS or artifact regression guard.

Per the performance programme gate, broad Debug CTest, sanitizer/failure
injection, install/package proof, opcode/reference documentation polish,
cleanup, commit and push have not run. The implementation remains provisional
and revertable until Adrian accepts this verdict.
