# PERF3-12D existing-instruction pattern PoC

## Verdict

The compiler exit can compile the opening dynamic PARSE forms to the existing
instruction set. No new RXAS/RXBIN operation is needed or justified by this
slice.

The PoC handles:

- two captures separated by a runtime `(identifier)` delimiter; and
- issue 667's exact `=(start)` then `+(span)` three-capture form.

The exit preserves a source snapshot, evaluates runtime operands at their
authored trigger points, assigns results in order, and resolves a later
`(target)` from the most recently completed assignment. Other supported dynamic
delimiter shapes continue to use the generic compatibility path; unproved
dynamic-position shapes fail compiler validation.

## Regina and ooRexx source findings

Both interpreters compile template topology once and retain runtime operands in
that compiled structure rather than rebuilding a textual template on every
execution.

- Regina's grammar creates typed `X_TPL_VAR`, `X_POS_OFFS`, `X_NEG_OFFS` and
  `X_ABS_OFFS` tree nodes, storing the parenthesized symbol tree separately from
  literal offsets. `doparse()` walks those nodes, evaluates dynamic values when
  their anchor is reached, advances the cursor and immediately calls
  `doparse3()` to assign the preceding segment. Its search path already uses a
  one-character `memchr()` fast path and Boyer-Moore for longer delimiters.
- ooRexx's parser creates `ParseTrigger` objects for string, absolute, relative
  and length triggers. Parenthesized forms retain an expression object. At
  runtime, `RexxInstructionParse::execute()` walks the prebuilt trigger array;
  each trigger evaluates its operand, updates `RexxTarget`, materializes the
  segment and assigns its variables before the next trigger.

This is the semantic lead adopted here: compiled topology plus ordered runtime
operand evaluation and assignment. cREXX can go further because its exit can
lower small deterministic templates directly to ordinary bytecode.

## Correctness

The direct exit protocol test proves that eligible fragments contain
`strpos`/`substring` and contain neither `parseExec` nor `parseplan`. The runtime
fixture covers runtime, missing, empty and multibyte delimiters, source aliasing,
issue-667 dynamic positions, latest-completed and completed-before-pending
target reuse, and a same-trigger external value.

Focused Release qualification passes 19/19, including optimized/no-opt runtime
tests, both PARSE suites, frozen PARSE controls and the new PoC. Direct manual
runs pass under `rxtvm` and `rxbvm` in both optimized and no-opt forms. A
standalone semantic fixture also passes under both concrete VMs.

## Equal-work measurement

The microkernel receives its delimiter and source as runtime arguments and
performs one million parses. One warm-up was discarded, followed by eight
interleaved profiling-off Release samples per VM and variant. All samples
reported `CHECKSUM=9000000`.

| VM | variant | elapsed samples (s) | median (s) | speedup |
| --- | --- | --- | ---: | ---: |
| `rxtvm` | E0 | 1.35, 1.39, 1.36, 1.39, 1.40, 1.37, 1.37, 1.38 | 1.375 | control |
| `rxtvm` | E1 | 0.08, 0.09, 0.08, 0.09, 0.08, 0.09, 0.09, 0.09 | 0.085 | 16.18x |
| `rxbvm` | E0 | 1.32, 1.33, 1.30, 1.31, 1.31, 1.32, 1.32, 1.29 | 1.315 | control |
| `rxbvm` | E1 | 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08 | 0.080 | 16.44x |

Counts-only profiling at 100,000 parses reports the same totals for both VMs:

| metric | E0 | E1 | change |
| --- | ---: | ---: | ---: |
| dynamic instructions | 67,700,030 | 3,400,030 | -94.98% |
| frame activations | 800,001 | 1 | -800,000 |
| frame reuses | 799,997 | 0 | -799,997 |
| string buffers | 600,034 | 14 | -600,020 |

## Canonical RexxCPS score

The canonical `tests/benchmarks/rexxcps_levelb.crexx` source was unchanged
(SHA-256 `2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6`).
It was compiled directly with the current E1 compiler exit because the existing
CMake benchmark artifact was stale and still contained four `parseExec` calls.
The fresh RXAS contains four `strpos` sites and no `parseExec` call or
`parseplan` instruction.

One warm-up was discarded. Eight profiling-off Release runs per VM were then
recorded in alternating VM order. Every run reported the canonical-default
contract, `effective_count=500`, `calibrated=1` and
`PASS: RexxCPS 2.2d cREXX port`.

| VM | E1 CPS samples | median CPS | retained E0 median | ratio |
| --- | --- | ---: | ---: | ---: |
| `rxtvm` | 43,910,994; 43,840,459; 43,525,344; 43,421,322; 43,900,585; 43,438,486; 42,998,953; 43,485,483 | 43,505,413.5 | 10,039,724.5 | 4.33x (+333.33%) |
| `rxbvm` | 46,309,246; 46,777,236; 45,880,273; 45,609,951; 46,065,033; 45,791,068; 46,456,269; 45,854,228 | 45,972,653.0 | 10,877,602.0 | 4.23x (+322.64%) |

The retained E0 medians are the same-day PERF3-12C eight-run current-product
control, not a newly paired panel. The E1 result is therefore a valid bounded
PoC score but not the mandatory first Release verdict for an approved
production implementation.

The generated benchmark artifact records the expansion cost:

| artifact | E0 | E1 | change |
| --- | ---: | ---: | ---: |
| RXAS bytes | 11,578 | 16,359 | +4,781 (+41.29%) |
| RXBIN bytes | 5,682 | 6,953 | +1,271 (+22.37%) |
| top-level static instructions | 68 | 63 | -5 |

The static-instruction row excludes the library body called by E0. The larger
E1 artifacts despite fewer top-level instructions come from the inline control
flow and its source/trace metadata.

## Decision boundary

E1 is a strong candidate for small deterministic PARSE templates, but remains
a PoC pending approval and the failure-visibility decision in the worklist.
This evidence does not justify a new instruction. It identifies one next
question: how RXBIN size scales as trigger/capture complexity grows.

If approved, the next bounded PoC should compare E1 expansion against an E2
packed Pattern Program stored as a read-only `.binary` constant. Only a
measured crossover or a recurring residual sequence may nominate a generic E3
instruction. Regex and PEG syntax/semantics remain out of scope for this PoC.

## Contents

- `COMMANDS.md`: reproducible command shapes and baseline identity.
- `raw/pattern_dynamic_benchmark.crexx`: runtime-unknown delimiter kernel.
- `raw/pattern_semantics_poc.crexx`: standalone semantic fixture.
- `raw/timing-samples.csv`: all recorded elapsed/checksum rows.
- `raw/profile-summary.csv`: equal-work count and allocation summary.
- `raw/artifact-summary.csv`: hashes, sizes and static instruction counts.
- `raw/rexxcps-timing-samples.csv`: canonical E1 CPS samples and pass controls.
