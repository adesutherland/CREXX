# PERF3-12 RexxCPS clause-lowering rereview

Date: 2026-08-04

Status: complete analysis-only evidence gate; no product source changed

## Outcome

The accepted RXAS graph/component-SSA infrastructure exposes three material,
general optimization families in current RexxCPS.  The largest measured
opportunity is transactional direct-destination PARSE lowering.  The safest
first implementation slice is the narrower XTOY component-placement case,
because its required decimal/string component, use, observation and signal
facts already exist and it needs no ISA change.

The implementation order recommended by this gate is:

1. `PERF3-12A / R12-C01`: redirect a copied XTOY temporary back to the
   original storage when the input component remains valid and the new result
   component is otherwise unobserved; delete the typed copy atomically.
2. `PERF3-12B / R12-S01+H01`: compare existing segmented native-stem
   operations with loop-scoped tail value reuse before selecting one route.
3. `PERF3-12C / R12-P01`: correct the conditional PARSE signal contract, then
   generalize producer forwarding to a transaction with multiple results.
4. `PERF3-12D / R12-I01+R01`: review late inlining and final register
   compaction only after the first three consumers have simplified the actual
   call body and temporary set.

Each slice remains separately approvable and must stop at its first ordinary,
profiling-off Release verdict.  This evidence does not authorize any of them.

## Authority and interpretation

- Repository control commit: `209206f0f0f07cd709dcdd04665489756e0b433e`.
- Accepted product commit: `5fbe36049e26ee73ea0cf1720a7fc416f33d0fe2`.
- The intervening commit changes only PERF3-06 documentation and evidence.
- Runtime authority remains the checksum-closed ordinary Release PERF3-06 Mac
  scorecard.  Nothing here is a wall-clock result.
- Counts use the canonical source and exact accepted optimized/no-opt images,
  fixed at `--smoke-count 200` so all four profile cells perform the same
  work: 200 counts x 100 averages x 14 `lvar` iterations = 280,000 hot
  `lvar` iterations and `cps_subroutine` calls.
- Both VMs ran schema-5 `counts` profiles serially with result 0, PASS output,
  all seven profile domains complete, and zero invalid events or overflow.

## Profile summary

| Cell | VM instructions | Change from no-opt | Status |
| --- | ---: | ---: | --- |
| `rxvm` no-opt | 148,701,541 | reference | complete |
| `rxvm` optimized | 54,221,210 | -94,480,331 (-63.536888%) | complete |
| `rxbvm` no-opt | 148,701,541 | reference | complete |
| `rxbvm` optimized | 54,221,182 | -94,480,359 (-63.536906%) | complete |

No-opt instruction counts are identical.  The optimized hot-kernel opcodes
are also identical.  Nineteen low-frequency final formatting/control opcodes
differ by an absolute total of 28 instructions because the two VM runs obtain
slightly different final timer text; none is in a ranked candidate family.

## Ranked measured opportunities

### 1. PARSE result/source transactions (`R12-P01`)

The fixed-count optimized profile executes exactly 1,960,000 PARSE
instructions: 1,400,000 `PARSEWORDS3`, 280,000 `PARSEWORDS3D`, and 280,000
`PARSEPOS2`.  Current compiler lowering creates source and result temporaries,
initializes them inside the hot loops, parses into them, then copies results to
the user variables.

Static source-to-RXAS weighting derives 7,280,000 PARSE-only `SCOPY`
instructions: 5,040,000 in `main` and 2,240,000 in `cps_subroutine`.  The same
mapping derives 1,960,000 grouped temporary-null instructions.  Removing both
classes while retaining the PARSE primitive gives an upper bound of 9,240,000
dispatches, 17.041302% of the optimized `rxvm` profile.  This is a derived
upper bound, not an attributed wall-clock percentage.

The subroutine has eight parse-only temporaries in `.locals=22`; their later
removal could also avoid up to 2,240,000 reused-frame local-relink operations.
That internal operation domain is separate and is not added to the instruction
ceiling.

The rewrite is not a simple copy fold today.  The three frozen PARSE opcodes
have precise read/write masks but conservative `RXSC_UNKNOWN` signal metadata.
Their VM handlers can signal `FAILURE` before writes when a result aliases the
source and the required snapshot allocation fails.  Current compiler
temporaries make assignment to user variables transactional.  Direct result
placement therefore requires either a proof that source/results are suitably
disjoint and non-signalling or an explicitly atomic runtime form; otherwise a
handler could observe a user variable changed too early.

### 2. Compound-tail construction (`R12-S01` and `R12-H01`)

The profile executes 2,520,002 `CONCAT string,reg` instructions.  Exact source
weighting assigns 2,240,000 of them to the five `"Key Bee." || lvar` sites;
the remaining 280,000 are the distinct `"1.0" || lvar` default-tail site.

Two general options should be compared rather than combined initially:

- loop-scoped value reuse materializes the stable `"Key Bee." || lvar` once
  per `lvar` and can remove up to 1,960,000 concatenations (3.614822% of the
  optimized instruction stream);
- the existing `STEMGET2`/`STEMSET2` instructions stream two segments without
  materializing a lookup key and can replace all 2,240,000 matching
  concat-plus-stem shapes, removing up to 2,240,000 concatenations (4.131225%)
  while retaining the stem operation itself.

The segmented route needs a reusable left-segment register for the current
constant-folded `key1`; the loop route needs lazy loop/value-numbering facts.
Both require exact signal/TRACE equivalence and storage/component stability.
The existing instructions make this a representation and proof comparison,
not an ISA request.

### 3. Copied XTOY temporaries (`R12-C01`)

The optimized stream contains exactly 2,220,000 adjacent `DCOPY`/`DTOS`
pairs, and the value-operation profile records exactly 2,220,000 decimal
copies totalling 97,680,000 logical bytes.  Deleting the copies is a
2,220,000-instruction ceiling, 4.094339% of the optimized stream.  `DTOS`
itself remains required.

The VM already stores decimal and string components in the same value. `DTOS`
reads the decimal component and writes the string component without changing
the decimal component.  Current metadata records that exact derivation and a
non-signalling, success-stable contract.  The general candidate is therefore:

```text
DCOPY temporary, source
DTOS  temporary
<string-only uses of temporary>

        ->

DTOS  source
<the same string uses redirected to source>
```

The new proof must show the source string value/cursor is unobserved, the
decimal input remains the right `ValueId`, the temporary has no other live
components or cleanup obligation, numeric/plugin dependencies agree, and the
TRACE event batch remains valid.  This is an atomic component-SSA/use rewrite;
it does not require a two-register opcode, a runtime representation flag, or
an `rxc` semantic optimizer.

### 4. Calls, registers, TRACE, and ADDRESS

- `cps_subroutine` is called 280,000 times.  Removing its direct call and
  return has a 560,000-dispatch ceiling (1.032806%) plus separate frame work,
  but its body should first be simplified by PARSE/result placement.  Late
  RXAS or hybrid inlining remains an `AT04` design activity, not a tactical RXC
  inline expansion.
- `main` uses `.locals=103` and `cps_subroutine` `.locals=22`.  Final register
  compaction follows temporary deletion; register reuse still requires an
  interference proof.  Neither should widen SSA merely to close numeric gaps.
- Inactive TRACE is not a hot-kernel cost: `_trace_current_mode` executes 100
  times and trace setup/teardown procedures 100-200 times, versus 280,000 hot
  iterations.  T1's reached-event batching remains the correct model.  The
  generated trace handler is an artifact/assembly-size question, not this
  runtime candidate.
- The source `ADDRESS VALUE addressenv().environment_name()` emits no runtime
  instruction or procedure call in the optimized main path.  Only metadata
  remains, so `R12-A01` has zero measured runtime work in this slice.

## Files

- `analysis/ranked-general-shape-ledger.csv`: full family dispositions and
  proof/scale/guard requirements.
- `analysis/clause-rxas-map.csv`: source-to-lowering and dynamic weighting.
- `implementation-queue.md`: separately gated implementation designs.
- `VALIDATION.md`: commands, calibration exclusions, parity and checksum
  proof.
- `profiles/rxvm` and `profiles/rxbvm`: complete runner bundles, including
  raw profiles, stdout/stderr, RXSEQ captures, summaries and local checksums.
- `checksums.sha256`: root-relative recursive evidence verification.
