# cREXX Language Performance Benchmarks

This directory adds a portable language-workload layer above the focused VM,
optimizer, collection, iterator, and library comparisons in
`tests/performance/`. The programs have deterministic reference results and no
timed I/O in their kernels.

## Included programs

| Program | Provenance | Main pressure |
| --- | --- | --- |
| `rexxcps_levelb.crexx` | RexxCPS 2.2d, the cREXX Level B port of Mike Cowlishaw's RexxCPS 2.2 | A historically representative classic Rexx clause mix |
| `awfy_sieve.crexx` | Are We Fast Yet? / SOM | integer arrays, indexed access, nested loops |
| `awfy_permute.crexx` | Are We Fast Yet? / SOM | recursion, method calls, array mutation |
| `awfy_mandelbrot.crexx` | Are We Fast Yet? / Benchmarks Game | floating-point arithmetic, branches, bit operations |
| `awfy_mandelbrot_hoisted_control.crexx` | separate v2 diagnostic derived from canonical Mandelbrot | source-hoisted invariant integer-to-float conversion |
| `awfy_towers.crexx` | Are We Fast Yet? / SOM | object allocation, object attributes, recursion |
| `awfy_bounce.crexx` | Are We Fast Yet? / SOM | objects, typed arrays and explicit references |
| `awfy_storage.crexx` | Are We Fast Yet? / SOM | sustained tree allocation; disclosed cREXX node wrapper |
| `awfy_list.crexx` | Are We Fast Yet? / SOM | linked-list recursion and explicit weak references |
| `awfy_richards.crexx` | Are We Fast Yet? / Richards | scheduler queues, state transitions and a larger call graph |
| `awfy_deltablue.crexx` | Are We Fast Yet? / DeltaBlue at pinned commit `74306fec151070fd07157cefeacf19e7e0bcdc89` | complete chain/projection constraint solving through a stable indexed Level B graph |
| `awfy_json.crexx` | Are We Fast Yet? / Json at pinned commit `74306fec151070fd07157cefeacf19e7e0bcdc89` | complete 25,820-byte RAP parse and verification through the indexed `rxjson` document surface |
| `awfy_queens.crexx` | Are We Fast Yet? / SOM | recursive search, object attributes and boolean arrays |
| `awfy_nbody.crexx` | Are We Fast Yet? / Benchmarks Game | floating-point object access and disclosed native `rxmath` square root |
| `json_parser.crexx` | deterministic RAP-shaped JSON fixture | parser/text processing through the current `rxjson` surface |
| `json_parse.crexx` | v2 deterministic RAP-shaped JSON fixture | fresh indexed-document construction and validation |
| `json_query.crexx` | v2 deterministic RAP-shaped JSON fixture | parse-once indexed member/count traversal |
| `base64_roundtrip.crexx` | deterministic RFC 4648 algorithm | `.binary`, pre-sized buffers, byte access and checksum observation |
| `base64_roundtrip_v2.crexx` | version-2 deterministic RFC 4648 algorithm | fastest current portable Level B byte access and arithmetic digit decoding |
| `parse_frozen.crexx` | NR-14 static/frozen PARSE workload | RexxCPS-shaped immutable three-word and positional templates plus adjacent fallback checks |
| `parse_frozen_generic.crexx` | NR-14 generic frozen-plan workload | longer odd/even word templates, literal delimiters, absolute positions and compact drops |

`cross-runtime/` contains the NR-02 ooRexx and NetRexx sources. Byte-exact
RexxCPS 2.2 and bundled NetRexx 2.1n remain unchanged; 2.2n, opaque-input,
result-observation and trace diagnostics use distinct names. All portfolio
ports preserve deterministic arguments/results and document every material
language-required representation or operation substitution in
`cross-runtime/README.md` and `performance/NR-02-WORKLIST.md`.

For the formal common comparison, NetRexx uses `options nobinary decimal` and
NetRexx `Rexx` numeric state; generated Java plus the default HotSpot JIT are
the normal substrate. Binary-typed NetRexx portfolio ports are labelled JVM
controls and do not enter the common Rexx aggregate.

The Are We Fast Yet? suite was published with the DLS 2016 paper
“Cross-Language Compiler Benchmarking—Are We Fast Yet?” (DOI
`10.1145/2989225.2989232`). Its porting rules prioritize deterministic,
well-typed workloads and comparable code structure.

### Full AWFY Json reserve lane

`awfy_json.crexx` is the separately named full-input lane. Its fixture is the
exact minified 25,820-byte RAP payload from the pinned upstream commit, with
SHA-256
`8f84f5fdc609a6d7179089249212a39588030852719d951db2d178820b70a7d8`.
The immutable text is loaded once before the repeated kernel. Every iteration
constructs a fresh document and verifies an object root, object member `head`,
array member `operations` and exactly 156 operations.

The cREXX implementation uses the supported immutable indexed
`.jsondocument`, not AWFY's minimal-json object graph. It is labelled
`standard-library-indexed-document`, remains outside every cross-runtime
aggregate and must not replace `json_parser.crexx`, `json_parse.crexx` or
`json_query.crexx` in historical evidence. The runtime fixture path also keeps
the full parse outside compile-time constant folding; no separate opaque source
is required.

### AWFY DeltaBlue reserve lane

`awfy_deltablue.crexx` runs both parts of the pinned upstream
`innerBenchmarkLoop(n)`: a chain of `n` equality constraints with 100 edit
propagations, and the projection graph with forward, backward, scale and
offset verification. Assertion success is the upstream result contract; the
port does not substitute a checksum.

Level B object assignment has value semantics, so the port does not use copied
objects as graph identities. Planner-owned typed arrays hold variable and
constraint state, stable integer handles connect the graph, and one tagged
constraint representation preserves the upstream equality, scale, stay and
edit behaviours. The lane is labelled `stable-indexed-constraint-graph`, is
not Tier A and enters no aggregate.

The first qualification also records an optimizer boundary rather than hiding
it: at bounded size 500, the optimized image is substantially slower than the
unoptimized image. Optimized RXAS expands from 1,566 to 6,656 source
instructions and amplifies linked attribute/copy traffic. This is retained as
an input to the later generic scalar-access and late-inlining work; it is not a
reason to change the benchmark contract or report the no-opt result as the
product result.

RexxCPS is different: it reconstructs a dynamic mix derived from about 2.5
million lines of traced Rexx programs and reports clauses per second. Version
2.2d is a source port rather than a newly designed workload: its control flow,
1,000-clause dynamic mix, calibration, defaults, reporting shape, and stem-
default assignment follow upstream 2.2. The Level B type system still requires
the substitutions audited below. Quote its result as **RexxCPS 2.2d for
cREXX**, never as an unchanged RexxCPS 2.2 result.

The 2.2d numeric representation is explicit and intentional:

| Path | cREXX representation | Reason |
| --- | --- | --- |
| Authored benchmark procedures | `numeric digits 9` | Matches the Classic REXX default selected implicitly by upstream 2.2; generated trace handlers and imported modules keep their own contexts. |
| Counts, indices, loop controls, lengths and exact bounds | `.int` end to end | These paths are semantically integral and use native integer operations. |
| `j`, its 1.1-step loop and compound-variable arithmetic | `.decimal` plus explicit string storage conversions | This is genuine Classic decimal workload arithmetic, including the benchmark's intended string/number conversion exercise. |
| `time()` results, elapsed time and rate calculations | `.float` after the string-returning timer boundary | Disclosed cREXX port adaptation for binary64 timing; it does not replace clause-workload decimal arithmetic. |
| Timer calibration and display text | `floattrunc` / `floatformat`, then an explicit `.int` conversion where calibration needs a count | Preserves Classic textual BIF contracts without routing binary64 values through `.decimal`. |

NetRexx's separate 2.2n port continues to select 20 digits explicitly. The
2.2d cREXX decision does not alter that port or its historical evidence.

Run the canonical benchmark without arguments so it uses the upstream `100 x
100` defaults and one-second self-calibration:

```bash
cmake-build-release/bin/crexx --nokeep \
  -lcmake-build-release/bin/library.rxbin \
  tests/benchmarks/rexxcps_levelb.crexx
```

The only accepted override is the explicit `--smoke-count COUNT` form used by
CTest. It is always reported as `contract=noncanonical-smoke`; a count below
100 also prints a `NONCANONICAL` warning. Positional overrides are rejected so
a wrapper cannot silently change the baseline contract. Canonical retained
evidence must show `contract=canonical-default`, `argv_count=0`, and the
effective count/averaging provenance printed by the benchmark.

The direct `crexx` command and `run_benchmarks.crexx` execute the program and
library modules separately, retaining source/TRACE metadata. Optimized CTest
smoke tests instead create a linked image with `rxlink -s`. These are different
lifecycle modes: source stripping removes diagnostic metadata and currently
also avoids an accidental runtime metadata-scan cost in type/ADDRESS paths.
Published timing must state whether metadata was retained or stripped and must
not mix samples from the two modes.

### RexxCPS 2.2 timed-kernel equivalence audit

The execution counts below are for one outer iteration labelled “1,000
clauses” by RexxCPS. Counts come from the unchanged loop/select paths, not from
wall-clock inference. “Preserved” means the source clause count and intended
value effect are the same; it does not claim identical VM instructions.

| Upstream RexxCPS 2.2 | cREXX 2.2d port | Executions | Assessment |
| --- | --- | ---: | --- |
| `flag=0` | `flag = "0"` | 1 | Preserved; the explicit string spelling fixes the Level B type. |
| `do loop=1 to 14` | `do lvar = 1 to 14` | 1 loop / 14 bodies | Preserved; `loop` is a Level B statement keyword, so only the identifier changes. |
| Implicit conversions in comparisons and arithmetic | Decimal `j` under digits 9 plus local `as .decimal` / `as .string` conversions in the same clauses | 28 inner-loop bodies; 27 stem multiplications | Clause counts and value effects preserved; the explicit conversions retain the Classic string/decimal exercise. |
| `avar.=1.0''loop` | `avar. = "1.0" \|\| lvar` | 14 | Preserved, including the Classic operation that replaces existing tail values and establishes the new default. `.stem` implements this reset in constant time using generations. |
| `when flag` | `when flag <> "0"` | 28 predicate checks / 27 true paths | Preserved truth test; Level B requires a Boolean expression. |
| `trace value trace()` | `trace value _trace_current_mode()` | 14 | Preserved query/set clause; uses the Level B trace-state surface. |
| `address value address()` | `address value addressenv().environment_name()` | 14 | Preserved query/set clause; uses the Level B ADDRESS environment object. |
| Two-argument call plus `parse upper arg a1 a2 a3 ., a4` | Four typed call arguments plus one typed `arg` clause | 14 calls / 14 argument clauses | Clause count and resulting argument values preserved. Argument splitting and uppercasing are expressed at the typed call boundary, so internal work is not instruction-identical. |
| Four assignments to `rc` | Four assignments to `text` | 56 | Preserved; Level B reserves typed `RC` for host-command status. |
| Failure-path `say` clauses | The same `say` clauses | 0 on a valid run | Preserved; CTest treats any `Failed...` output as a test failure. |

**Net timed source-clause change: zero.** The score therefore retains the
upstream numerator of exactly 1,000 nominal RexxCPS clauses per outer
iteration; it is not an estimate derived from cREXX instructions or trace-line
counts.

This table makes the comparability decision reviewable: rows marked preserved
have the same dynamic source-clause count and intended result; the typed
argument and conversion rows are disclosed differences. Cross-implementation
scores are therefore meaningful community comparisons of the recognizable
RexxCPS workload, but not proof that the implementations perform identical
lower-level work per clause.

RexxCPS measures **Rexx clauses**, not cREXX VM instructions. For VM
instructions per second, sum the `count` column of the profiler's `instruction`
rows and divide by the non-profiled elapsed time for the identical workload.
Do not use the instrumented elapsed time: per-instruction timestamps perturb
the run.

## Build and correctness checks

The CMake target builds optimized and unoptimized bytecode for every program.
CTest smoke-runs both forms and verifies reference results without asserting
machine-specific timing thresholds:

```bash
cmake --build cmake-build-release --target language_benchmarks --parallel
ctest --test-dir cmake-build-release -L benchmark --output-on-failure
```

The separately named `rexxcps_levelb_opaque.crexx` target is an NR-02
diagnostic, not a replacement canonical score. Its optimized/unoptimized smoke
uses fixed `1 x 1` counts and validates the externally observed final state.
Full-default diagnostic pilots must state their A/B variant and remain
separate from `rexxcps_levelb.crexx` results.

## Convenience process smoke

`run_benchmarks.crexx` is a convenience process-smoke runner. It checks every
result and can expose gross timing changes, but it is not formal baseline or
aggregate authority. Formal evidence uses
`performance/tools/run_cross_runtime_matrix.crexx` under the standing
performance governance.

```bash
cmake-build-release/bin/crexx --nokeep tests/benchmarks/run_benchmarks.crexx \
  --args --build-dir cmake-build-release --vm rxvm --mode opt \
  --warmups 2 --runs 10 --output rxvm-opt.csv
```

The runner itself is Level B cREXX. It accepts the compiler-selected `rxvm`
product and the `rxtvm`/`rxbvm` concrete-engine controls, records their roles
explicitly, and labels every result `process-smoke`. It records microsecond-resolution wall
time as nanoseconds in the CSV columns, validates every child run by exit code
and `PASS:` output, and identifies the runner with `crexx_version`. For
RexxCPS it additionally requires the child's
`REXXCPS-PROVENANCE contract=canonical-default` marker, proving that the formal
baseline path supplied no count override.

The qualified Full AWFY Json reserve lane is available explicitly as
`--benchmark awfy-json`; it is not part of the runner's historical default
set. Each process-smoke sample performs 50 complete parse/verify iterations
and supplies the exact build-tree fixture path.

The qualified DeltaBlue reserve lane is available explicitly as
`--benchmark deltablue`; it is also excluded from the historical default set.
Each process-smoke sample runs the complete chain and projection contracts at
bounded size 500. Retain optimized and unoptimized cells separately because
the current optimizer expansion is an observed result, not comparable noise.

For a diagnostic sample stream, also request the serial samples:

```bash
cmake-build-release/bin/crexx --nokeep tests/benchmarks/run_benchmarks.crexx \
  --args --build-dir cmake-build-release --vm rxvm --mode opt \
  --warmups 2 --runs 10 --output summary.csv \
  --samples-output samples.csv
```

The samples file contains every warmup and recorded run in execution order.
Its lifecycle is labelled `process`: every sample starts a new VM and loads the
workload and library modules, so these measurements include startup and are not
steady-state kernel timings. Keep the samples and summary from the same
invocation together with the host, commit, dirty state, toolchain, build flags,
image set and metadata-retention mode. The operational manifest and retained
evidence layout are in `performance/`.

For a non-cREXX process, run the Level B cREXX tool
`performance/tools/run_cross_runtime.crexx` through the Release `crexx`
driver. It retains the exact argv, serial warmup/recorded process samples,
every raw stdout/stderr stream, correctness result and optional
benchmark-native metric. It does not compile ports or decide equivalence;
those remain explicit NR-02 cell steps.

Repeat with `rxtvm`, `rxbvm` and `noopt` when concrete-engine diagnosis is
needed. Do not duplicate a concrete engine when `rxvm` selects the same
executable. Compare one change at a time on the same host,
power mode, compiler, build type, and commit. Keep raw samples; do not compare
single best-case numbers from different machines. Process-level results include
VM and module startup, which is intentional for cold-run measurements. For
steady-state kernel work, increase the matching argument in
`benchmark_command()` in `run_benchmarks.crexx` rather than subtracting startup
time.

## cREXX profiling workflow

Configure a separate profiling build because profiling hooks are compiled out
of normal builds:

```bash
cmake -S . -B cmake-build-profile -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=ON
cmake --build cmake-build-profile --target language_benchmarks rxseq --parallel
```

Instruction/transition timing (diagnostic only; do not use the instrumented
elapsed time as the benchmark result):

```bash
cmake-build-profile/bin/rxvm \
  --profile-output sieve-profile.csv \
  cmake-build-profile/tests/benchmarks/benchmark_awfy_sieve_opt.rxbin \
  cmake-build-profile/bin/library.rxbin -a 50
```

Dynamic three-instruction sequences, followed by offline clustering:

```bash
cmake-build-profile/bin/rxvm \
  --sequence-count=3 --sequence-output sieve.rxseq \
  cmake-build-profile/tests/benchmarks/benchmark_awfy_sieve_opt.rxbin \
  cmake-build-profile/bin/library.rxbin -a 50
cmake-build-profile/bin/rxseq sieve.rxseq \
  cmake-build-profile/tests/benchmarks/benchmark_awfy_sieve_opt.rxbin \
  cmake-build-profile/bin/library.rxbin --output sieve-sequences.csv
```

Use the timing profile to find costly opcodes and transitions, the sequence
profile to find high-volume instruction windows, and optimized `.rxas`/`rxdas`
output to map those costs back to source. Confirm every optimization with the
ordinary non-profiled Release build and the correctness tests.

## Approved portfolio

The approved bounded portfolio contains eleven steady-state workloads plus the
separate compile/load/first-result lifecycle lane. `Bounce` and `List` exercise
cREXX references directly. `Storage` exposes the current array/object and
nested-reference-container boundary; JSON exposes the absence of a full cREXX
JSON object model. Those adaptations are never silently aggregated. The
lifecycle lane is reported separately from steady-state workload aggregates.
Queens and NBody have separately named Tier B cREXX sources. Full AWFY Json and
DeltaBlue are qualified, separately named post-PERF3 reserve lanes; neither
changes the v2 aggregate. CD, Havlak, filesystem-I/O workloads and focused
Classic-semantics probes remain pending or reserve work.

The confirmed capability gaps and audit candidates uncovered while porting are
kept in `performance/capability-gaps.md` so they can be checked and considered
for measured closure work.

See `THIRD_PARTY_NOTICES.md` for attribution and license terms.
