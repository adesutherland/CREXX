# STEP05 local qualification — 15 October 2026

STEP05 implements the approved Level L/emitter cleanup and shared optimisation
pipeline over develop `afcde7f6c2b2dcce6a8a6e89705457aafcac8344`. The compiler
and library algorithms remain Level B. No native compiler, assembler, linker
or VM source changed. The authoritative scope is [F5-AC01–08](../../PLAN.md).
This receipt closes that bounded local increment, not the complete compiler
replacement plan. Adrian authorised committing and publishing this qualified
increment to `develop` and switching the lightweight CI monitor on 15 October.

Adrian explicitly removed formal performance testing as a requirement for this
research project. Root `AGENTS.md` records that standing exception, also linked
from `performance/AGENTS.md`. There is no AC-power, sample-count or separate
Release-verdict gate here. Functional/resource tests and emitted-code quality
remain acceptance conditions. The earlier accepted cleanup measurement is
[historical evidence](../step05-cleanup-20261015/README.md); it is not repeated.

## Qualified behaviour

- `llsource.index` owns a bounded line index and preserves exact byte offsets,
  Unicode scalar columns, CRLF/CR/LF, EOF and malformed-byte diagnostics.
- The frontend avoids unused default construction. The RXAS selector reserves
  required scratch/diagnostic/call windows, including cold calls and ten-argument
  cases, and omits jumps to the next emitted block. These fixes work with IR
  optimisation disabled. The existing Level L writer remains the output path.
- `llirservices..optimise` uses numeric indexes for exact facts, same-block
  private-slot substitutions, known Boolean edges, entry-rooted reachability
  and reverse demand. One checked reconstruction returns the candidate and
  operation/value/block maps. Source graphs remain unchanged; failed work,
  index or byte budgets return the original graph, including failures during
  reconstruction. Source/type/slot/constant tables and byte pools are retained;
  this increment does not compact every stored table.
- Generic services contain no C/G names or imports. C/G supply explicit
  evaluators: checked G int64 add/subtract/multiply/comparison, int64-to-text,
  canonical Boolean conversion/truth, and exact C literal `0`/`1` truth. Unknown
  effects, possible failures, state/call barriers, divergence, overflow,
  division, Classic arithmetic/context/pools and user calls retain runtime
  behaviour unless an admitted exact evaluation proves that instance safe.
- Compiler `-n` and the trailing service `optimise=0` select the cleaned control.
  `graph()` retains the frontend view; `emissiongraph()` exposes the successful
  selected graph. B-host and assembler optimisation controls stay independent.

## Functional and delivery evidence

| Check | Result and evidence |
|---|---|
| Normal Debug and ordinary Release product builds | Both passed. Logs `/tmp/levell-step05-final-debug-build.log` and `/tmp/levell-step05-passes-release-build.log`; [product hashes](product-hashes.json). |
| Independent Level L IR/services/source/optimisation | 16 CTests passed across both host modes and VMs: [log](units.log). Source index has 123 assertions; the non-C/G optimiser fixture has 113, including its independent interpreter, exact attributes, barriers, divergence, budgets, callback refusals, multiple results, origins, mappings, all function roots and convergence. Existing IR/services tests retain codec, stale/foreign handle, rewrite/lowering and writer contracts. |
| C/G admission and output writer | 77 assertions per new consumer, including integer boundaries, refusal cases, an independent branch oracle, original/emission graph comparison, remapped chunk origins, sink failure and bounded atomic append. Executed in the paired aggregate and installed consumer. |
| New paired compiler-mode aggregate | Four CTests passed: both B host compilation modes and both VMs, with IR on/off and assembler `-n`; 33 commands per invocation. Branch and cleanup fixtures cover defaults, void calls, ten arguments, Classic numeric state and logical source filenames. [Log](registered.log). |
| Existing backend and provenance suites | Optimised B host: all four CTests passed, covering both VMs and assembler modes: [log](backend-provenance-opt.log). No-opt B host: 31 programs / 232 commands and 90 provenance commands passed on rxbvm: [backend](backend-noopt.log), [provenance](provenance-noopt.log). Earlier no-opt cleanup evidence remains valid for unchanged cleanup inputs. |
| Existing native compiler controls | 120 commands passed against independently stated expected output: [log](native-controls.log). Existing documented profile exclusions/control discrepancies are unchanged; no claim that every existing compiler is an oracle. |
| Standalone B-only bootstrap | Both opt/no-opt builds reached L0/L1/L2 verification with only native seed tools and `library.rxbin` as prerequisites: [receipt](bootstrap.json). No Level G import/target enters that standalone graph. |
| Fresh install into a prefix with spaces | Complete `cmake --install` passed. 25 installed-only checks passed: C/G IR on/off compile/assemble/link/execute on both VMs, plus compiling/linking and executing the 77-assertion B service consumer on both VMs: [commands](installed.json). |
| Focused maintained Apple ASan | New 33-command aggregate and six source/services/optimiser CTests passed: [aggregate log](isolated-asan.log), [unit log](units-asan.log). No sanitizer finding occurred in these checks. Apple LSan is unsupported; this is not broad or cross-platform sanitizer qualification. |

The optimiser tests use a non-C/G semantic family and an independent bounded
IR interpreter, rather than calling the folding callback as their expected-
result oracle. C/G error and provenance cases retain reached/skipped arithmetic
failures, short-circuiting, calls, recursion, loop/control joins, multiline and
Unicode locations, file failures and previous-output preservation.

The compiler host import closure remains B/L only. The unchanged Classic target
adapter is compiled in Level B but links its existing Classic runtime for the
emitted program. Existing generated native launchers package bytecode; they do
not implement compiler logic. Python scripts only orchestrate tools and compare
results; transformation algorithms and reference IR interpreters are Level B.

## Scheduling and preparation

Before permanent registration, the final 33-command aggregate passed in
isolation in Debug (32.213 seconds) and the maintained Apple ASan build
(64.69 seconds): [Debug commands](isolated-debug.json),
[ASan commands](isolated-asan.json). These are scheduling observations under the
then-current host load, not formal performance results. Tests use `RUN_SERIAL`,
the `llcompiler_backend` resource lock, a 900-second whole-test hang guard and
300-second nested-command hang guards. Explicit prep dependencies include both
compiler commands, the admission consumer, target adapter, B/Classic/class
runtime images, assembler, linker and selected VM.

An initial ASan build encountered an unconfigured new target; regenerating the
existing build tree corrected that setup error. Its refreshed native tool
build emitted existing Apple SDK `sprintf` deprecation warnings. There was no
new source warning suppression, native change or sanitizer defect. Retained
build log: `cmake-build-debugasan/asan-logs/20261015-202746-build/build.log`.

## Emitted code and qualification identities

The [complete worked examples](../../examples/step05/README.md) retain actual
IR/RXAS before and after the shared passes, with cleanup present on both sides.
All four examples execute `yes` with assembler optimisation disabled.

| Complete module | G before | G after | C before | C after |
|---|---:|---:|---:|---:|
| Executable instructions | 17 | 8 | 111 | 83 |
| Calls | 1 | 1 | 13 | 9 |
| Body registers | 7 | 3 | 17 | 14 |
| Source records | 11 | 7 | 13 | 9 |

Wrappers and cold handlers are included. Labels, metadata and external
signatures are excluded from instruction counts. C retains its activation,
argument/numeric packets, pool and condition handler. Smaller counts establish
code shape, not speed. Before Adrian waived the formal campaign, one battery-
powered Release pilot per language/candidate had executed correctly; those
single timings do not support a performance conclusion or gate this work.

[Input hashes](input-hashes.json) retain the library/compiler/test/build closure;
[product hashes](product-hashes.json) identify the Debug/Release tools and
installed compiler packages. The final CMake registration is included. No
source/test code changed after the recorded qualification. Compact prior-run
identities and retained temporary command records are indexed in
[retained-runs.json](retained-runs.json). Documentation changes do not invalidate
those unchanged tested inputs.

## Acceptance boundary

| Criteria | Evidence |
|---|---|
| F5-AC01 / F5-AC08 | Accepted cleanup receipt, independent source-index tests, C/G ABI/default/provenance regressions and new writer checks. |
| F5-AC02 / F5-AC04 / F5-AC05 | Independent generic fixture/oracle, existing services regressions, exact budgets and verified atomic reconstruction. |
| F5-AC03 / F5-AC06 | C/G semantic admission, independent expected values, paired IR/RXAS and actual execution in both modes/VMs. |
| F5-AC07 | Explicit opt-off controls, both B-only bootstraps, complete install, installed consumer, source/product hashes and architecture/user-guide updates under the approved research qualification scope. |

F5-AC01–08 and F5-STEP01–04 are verified locally. STEP06 and full replacement,
proper B self-hosting, INTERPRET, broader TRACE/condition work, SSA/register
reuse, inlining and contextual Classic arithmetic specialisation remain open.
Develop publication and CI-monitor retargeting are authorised. Ordinary
automatic CI is expected to pass after publication; hosted results are reported
separately, without a status-only repository commit. This receipt does not claim
an observed hosted pass for STEP05.
