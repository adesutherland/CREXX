# PERF3-05-R5a effective handler-placement profiling

This bundle retains the focused diagnosis requested after the R5 Apple GCC
threaded Bounce guard. It adds effective `inline`, `outline`, or `mixed`
handler placement to each executed instruction row and uses that attribution
to test whether Bounce simply depends on an instruction omitted from the 20%
panel.

## Result

The profile addition works and preserves canonical opcode counts. CSV schema 5
uses the existing `value` column on `instruction` rows; the human table adds a
`handler` column. Placement is recorded at the handler-entry hook, so a private
fused handler can produce `mixed` rather than inheriting a false label from its
serialized opcode.

GCC profile-20 `rxtvm` and `rxbvm` produced identical exact counts for the
canonical Bounce input (`4200` repetitions):

| placement | dynamic instructions | share |
|---|---:|---:|
| inline | 887,443,222 | 99.952222146% |
| outline | 424,204 | 0.047777854% |
| total | 887,867,426 | 100% |

`CALL1_REG_FUNC_REG` accounts for 424,200 outlined executions. The remaining
four are one execution each of `SCONCAT_REG_REG_STRING`, `STOI_REG`, `SAY_REG`
and `SAY_STRING`. No instruction row is `mixed`; private fusion therefore does
not hide an outlined hot path in this build.

The missing `CALL1` is not the principal explanation. `CALL1`, `SCONCAT` and
`STOI` all enter at the 30% tier. Applying the frozen tier ledger to the exact
profile means profile-30 outlines only the two one-off `SAY` executions, or
0.000000225% of dynamic instructions. `max-eligible` has the same executed
inline/outline split. Nevertheless, retained profiling-off GCC `rxtvm` Bounce
timing is -8.691% at profile-30 and -4.693% at max-eligible versus literal
all-inline; formal profile-20 is -10.072%.

The slowdown is therefore primarily a compiler owner/code-layout effect, not
runtime call overhead proportional to the number of outlined executions. The
fact that profile-30 and max-eligible have the same dynamic placement but
materially different speed is direct evidence that inlining otherwise
unexecuted handlers reshapes the hot threaded owner.

## Correctness and isolation

- The unit profiler, switch and threaded profile output, CSV/table formats,
  counts determinism, and documentation example pass 6/6 focused tests.
- Signal and breakpoint instrumentation passes 4/4 CTests; a direct concrete
  `rxtvm_instrumented` signal/breakpoint check also passes.
- Both GCC profile-20 Bounce executions pass the exact output oracle with
  empty stderr, complete profile domains, and identical instruction counts.
- Normalized profiling-off profile-20 preprocessing is byte-identical before
  and after this change for both engines. The added hook argument is not
  evaluated in an ordinary build.

An additional GCC profiling-enabled all-inline compilation was stopped after
8 minutes 28 seconds at approximately 6.6 GiB RSS. It was a redundant
diagnostic, not a failed product build: all-inline placement is mechanically
all inline, and ordinary all-inline timing is already retained. The unusually
high diagnostic compilation cost further illustrates the compiler-complexity
problem but is not used as a benchmark result.

## Decision boundary

Do not promote `CALL1` alone or refresh the hot panel from this single
workload. The next useful GCC threaded investigation is binary/assembly layout
comparison of profile-30, max-eligible and all-inline, which have effectively
the same executed placement but different speed. No default or tier changes
are made by R5a.
