# STEP05 first cleanup Release verdict — 15 October 2026

**Cleanup verdict accepted by Adrian on 15 October 2026.** This is the
first F5-STEP01 gate, not completed STEP05 qualification or publication.
The [authoritative plan](../../PLAN.md#step05-proposal-level-l-services-and-shared-optimisation)
retains all F5 acceptance criteria and the remaining shared-pass work.

## Scope and frozen inputs

Baseline: `afcde7f6c2b2dcce6a8a6e89705457aafcac8344`. Candidate: the uncommitted
Level B source/test inputs in [candidate-input-hashes.json](candidate-input-hashes.json).
Changes provide a reusable bounded `llsource.index`, use it in diagnostics and
RXAS source emission, avoid unused frontend defaults, size frames from actual
instruction-selection needs, and omit next-block jumps. The existing Level L
writer remains the output path. No shared IR optimisation pass is implemented
or enabled, and no C, VM, assembler or linker source changed.

Both ordinary Release `stage-product` builds passed with VM profiling off.
[Baseline](baseline-hashes.json) and [candidate](candidate-hashes.json) binaries
were retained separately before/after the production edits. Their native VMs,
assembler, linker, `library.rxbin`, `llctarget.rxbin` and `rxfnsc.rxbin` match
byte-for-byte. Frozen source/test hashes were rechecked after measurement.
The same unchanged `classlib.rxbin` was supplied for Classic runtime autoload.

Temporary full build logs, frozen products and per-command logs remain in
`/tmp/levell-step05-gate-t04nsyx5`. This receipt retains hashes, raw paired
measurements, RSS logs, command records and the actual emitted RXAS. Temporary
binaries are not repository deliverables.

## Minimum correctness before the gate

- Independent Level L source-index consumer: **123 assertions**, including
  Unicode columns, CRLF/CR/LF, empty input/EOF, invalid offsets, exact budgets,
  ownership, malformed-byte diagnostics and repeated lookup. Four host-mode/VM
  CTests passed; the separately retained [probe](source-probe.log) also passed.
- C/G service consumer: **58 assertions** in both host compilation modes,
  including unused/default construction, minimum register budget, frame shape,
  provenance and output ownership.
- Existing no-opt-host backend corpus: **31 programs, 232 commands**, both
  assembler modes and both concrete VMs. Existing provenance aggregate:
  **90 commands**. Both [focused CTests passed](focused-ctest.log).
- New `cleanup-c.crexx` and `cleanup-g.crexx` end-to-end fixtures:
  **18 commands**, both assembler modes and both VMs. They cover bare SAY,
  required defaults, ten arguments, void calls and Classic numeric state;
  [commands](focused-commands.json). They are not yet registered in the nested
  aggregate: scheduling measurement and registration remain after this gate.
- All four frozen Release measurement outputs assembled with `rxas -n`, linked
  and executed, printing the expected sequence 1 through 80; see
  [execution commands and expected output](release-execution.json).

The first isolated baseline C execution omitted `classlib.rxbin` from its
autoload directory. Its [error](baseline-c-run.stderr) is retained. Supplying
the same runtime package to both frozen variants resolved this harness setup
error without changing compiler inputs or discarding timing samples.

## Release compiler lifecycle measurements

The unchanged Level B consumer
[`tests/performance/levell_compiler_cleanup.crexx`](../../../../../tests/performance/levell_compiler_cleanup.crexx)
was linked separately to the two frozen library sets. It builds C/G source
containing **80 assignment/SAY pairs (160 clauses)**; G also declares `x` as an
integer. Its output key `statements=80` counts pairs. The timer covers compile
request construction, parsing, IR construction, RXAS selection/emission and
output text materialisation. It excludes VM process startup, source generation,
file writing, assembly, linking and target execution. Target execution is
checked for correctness here; no target execution-speed claim is made.

Serial, balanced alternating baseline/candidate pairs began with 12 rounds per
language. Noise/interval overlap justified bounded extensions to 24 then the
maximum 36, with no candidate changes and **no excluded samples**. The
[raw observations](pairs.json) contain commands and values; summaries retain
medians, MADs, quantiles, ranges, paired changes and Student-t 95% intervals for
the mean paired percentage change. Negative change means faster compilation.

| Language | Baseline median | Candidate median | Paired median change | Paired mean change and 95% interval | Favourable pairs |
|---|---:|---:|---:|---:|---:|
| G | 1,016,650 us | 999,743 us | -0.72% | -0.81% [-2.65%, +1.04%] | 25/36 |
| C | 1,084,842.5 us | 1,061,952.5 us | -2.78% | -3.53% [-5.64%, -1.41%] | 34/36 |

**Verdict:** modest C compile-time improvement in this bounded cell; G timing
is inconclusive. The wide sample ranges remain visible in
[summary-36.json](summary-36.json); this is not a general speed or replacement
claim. No measured lifecycle regression exceeds the programme guard.

Three process peak-RSS samples per cell include the VM, linked compiler and
request lifecycle. Median G RSS changed from 608,911,360 to 608,485,376 bytes
(-0.07%); C changed from 639,205,376 to 625,704,960 bytes (-2.11%). Neither hits
the >5% and >1 MiB regression guard. [Raw RSS](rss.json) and time logs are
retained. These are process measurements, not isolated source-index allocation.

## Emitted-code quality

Counts cover the complete emitted module, including wrappers and cold handlers.
See [shapes.json](shapes.json) and the four `baseline-*.rxas` /
`candidate-*.rxas` files here. Static reductions do not establish runtime speed.

| Measure | G before | G after | C before | C after |
|---|---:|---:|---:|---:|
| RXAS bytes | 30,165 | 24,437 | 101,072 | 73,271 |
| Executable instructions | 569 | 488 | 3,992 | 3,030 |
| Calls | 1 | 1 | 487 | 327 |
| Source records | 404 | 324 | 485 | 325 |
| Main-wrapper registers | 16 | 2 | 16 | 6 |
| Body registers | 335 | 244 | 499 | 334 |

Fewer source records follow removal of unused constructions; source mapping is
not stripped. The C helper-call reduction comes from avoiding unused defaults,
not from dropping required Classic semantics. Activation, numeric state,
argument ownership and origin-preserving conditions remain covered by the
focused regressions.

## Decision and remaining work

Recommend retaining this cleanup based on structural improvements, passing
focused semantics and absence of a measured resource regression. G has no
established speed improvement. The implementation remains provisional and
part of the accepted research direction. Adrian subsequently clarified that
formal performance testing is not required for this Level L research project;
root `AGENTS.md` records the exception. These historical observations are retained.

F5-STEP02/03 shared reconstruction, analyses, passes, semantic evaluators,
controls and independent optimisation oracle were unimplemented at this
measurement checkpoint; the current plan records their subsequent work. Expanded
aggregate scheduling/registration, remaining normal tests, B-only bootstrap and
installed-consumer proof belong after the gate. Architecture/user-guide source
index and frame descriptions are updated, but full STEP05 documentation and
acceptance closeout remain open. No broad sanitizer, hosted or package
qualification is claimed, and nothing from this increment has been committed
or pushed.
