# Profiling cREXX VM Execution

cREXX can profile an RXBIN program at three complementary levels:

- **timing and counts** for VM instructions, dispatch transitions,
  procedures, methods, factories, native calls, and interrupt handling;
- **runtime storage counts** for VM values, frame allocation/reuse, attribute
  storage, and string/binary buffers;
- **dynamic instruction sequences** for finding frequently executed runs of
  two, three, or four sequential instructions.

These are development facilities. They are compiled into a dedicated VM build
and are absent from a normal build. They are also separate run modes: one run
collects timing data or instruction-sequence data, never both.

## Building the Profiling Tools

Configure an optimized profiling build from the repository root:

```sh
cmake -S . -B cmake-build-profile \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=ON
cmake --build cmake-build-profile --config Release \
  --target rxvm rxbvm rxvme rxbvme rxseq rxas
```

The profiling option is off by default. When it is off, the instrumentation
hooks compile to no code: the normal VM has no profiling branches, state, or
command-line options. A profiling build does contain the instrumentation even
when a particular run does not request a profile, so use the normal Release VM
for production execution and uninstrumented benchmark baselines.

Executable locations depend on the CMake generator and platform. The examples
below assume the profiling-build executables are installed or otherwise on
`PATH`.

All four command-line VM variants expose the same profiling options:

| VM | Execution mode | Runtime library |
|---|---|---|
| `rxvm` | threaded VM | load required RXBIN modules explicitly |
| `rxbvm` | bytecode VM | load required RXBIN modules explicitly |
| `rxvme` | threaded VM | standard library embedded |
| `rxbvme` | bytecode VM | standard library embedded |

Use the same VM variant when comparing profiles. Differences between VM modes
are often exactly what the instruction and transition views reveal.

## A Small Repeatable Example

The repository contains
[profiling_demo.rxas](examples/profiling_demo.rxas), reproduced here. Its
metadata gives the profiler human-readable callable names, and its worker
procedure is called twice.

```rxas
.globals=0

   .meta "profiling_demo.main"="b" ".int" main() ""
   .meta "profiling_demo.worker"="b" ".void" worker() ""

main() .locals=0
    call worker()
    call worker()
    ret 0

worker() .locals=5
    load r1, 0
    load r2, 5
loop:
    iadd r1, r1, 1
    copy r3, r1
    copy r4, r3
    ilt r0, r1, r2
    brt loop, r0
    ret
```

Assemble it from the repository root:

```sh
rxas -o profiling_demo \
  docs/books/crexx_programming_guide/examples/profiling_demo.rxas
```

This produces `profiling_demo.rxbin`. The same profiling commands work with
RXBIN files produced by `rxc` and `rxas`, by `rxlink`, or by the `crexx`
driver.

## Timing and Count Profiles

Run the example and write a human-readable report:

```sh
rxvm --profile-output profiling-demo.txt profiling_demo.rxbin
```

Write the same data as CSV by using a filename ending in `.csv`:

```sh
rxvm --profile-output profiling-demo.csv profiling_demo.rxbin
```

The extension check is case-insensitive. Any other filename selects the table
format. `--profile-output` enables timing profiling by itself; it does not need
to be combined with `--profile`.

For a table on standard error instead of a file, use:

```sh
rxvm --profile profiling_demo.rxbin
```

`--profile=timing`, `--profile-output=profiling-demo.txt`, and the equivalent
`--sequence-...=...` spellings are also accepted. Profiling options must occur
before the first binary filename. Program arguments still follow `-a`, for
example:

```sh
rxvme --profile-output application.csv application.rxbin -a first second
```

### Reading the Table Report

The heading identifies the VM mode and the program result. A result of `0`
normally means that the program completed successfully. The next lines state
the monotonic-clock calibration, hot-loop interrupt-poll count, invalid-event
count, counter-overflow state, and whether procedure and allocation tracking
were complete.

The report then contains complementary views of the run:

| Section | What it reports |
|---|---|
| **Instructions** | Count and entry-to-retire or entry-to-terminal time for each executed opcode. Rows are ordered by total time. |
| **Transitions** | Time from an instruction retiring to the next instruction entering. Kinds distinguish sequential flow, a taken branch, call entry, return exit, interrupt entry/resume, external entry, and termination. |
| **Procedures and methods** | Runtime calls and inclusive elapsed/body/self time for bytecode callables; call count and total observed time for native callables. |
| **Call mechanics** | The measured VM work entering and leaving each bytecode callable. |
| **Runtime allocation and value/frame storage** | Successful profiling-scope allocation requests, requested bytes, maximum request size, frame reuse, and active-frame high water. |
| **Call-path census** | Dynamic call attempts by direct/dynamic/native/root/signal path, exact arity, callable kind, frame disposition, outcome, target, and site. |
| **Return placement** | The actual runtime `RET_REG` local-move/non-local-copy decision plus void, ignored, immediate, and terminal returns. |
| **Dynamic selection** | Method/factory selector attempts and their success/failure outcomes, kept separate from subsequent `DCALL` activity. |
| **Call-window attribution** | Effects-backed setup swaps and defensive argument copies, completed mapping-recovery swap sequences, and all remaining unclassified `SWAP`/`COPY` executions. |
| **Signal-unwind restoration** | Branch-unwind events, discarded bytecode frames, restored bytecode/native windows and slots, and restoration failures. |

The final **Interrupt sub-phases** section separates interrupt scans from the
mechanics of entering, resuming, or terminating an interrupt path. These
sub-phases overlap the interrupt transition rows and must not be added to them.

### Procedure, Method, Factory, and Native Rows

Callable identity comes from RXBIN `META_FUNC` metadata. The report includes
the fully qualified name and classifies each row as `procedure`, `method`,
`factory`, or `native`. Older binaries without function metadata still receive
a module/procedure fallback name, but do not have the richer signature data.

For bytecode callables the columns mean:

| Column | Meaning |
|---|---|
| `calls` | Number of runtime frame activations. |
| `complete` | Calls that returned normally. |
| `unwound` | Calls discarded by exceptional stack unwinding. |
| `total ns` | Elapsed call time, including measured entry, body, and exit spans. |
| `average ns` | Elapsed time divided by the number of elapsed samples. |
| `body ns` | Inclusive body time, including nested bytecode calls. |
| `self ns` | Time attributed only to that callable, excluding nested bytecode calls and observed native-call time. |
| `native child` | Observed native-call time removed from this bytecode caller's self time. It is a child component of body time, not extra elapsed time. |
| `self %` | Self time as a percentage of inclusive body time. |

The call-mechanics table exposes the entry and exit components separately:

```text
caller call-instruction entry
    -> callee first-instruction entry       entry overhead
    -> callee return-instruction entry      inclusive body
    -> caller next-instruction entry        exit overhead
```

The complete elapsed span runs from the first boundary to the last. External
entry and terminal return use the nearest VM boundary available. These are
observed VM spans, not an estimate of the benefit of inlining.

Native plugin calls are timed around the VM's native-call boundary. A native
row therefore has `calls`, `complete`, total time, and average time, while
body, self, and bytecode entry/exit columns are shown as unavailable. Native
time remains visible in the calling instruction's timing but is removed from
the bytecode caller's self time. The `native_child` metric preserves that
removed amount explicitly for each bytecode caller.

Dynamic calls are attributed to the concrete procedure selected at runtime.
An inlined procedure or method creates no runtime frame and consequently does
not appear as a separate callable row; its work belongs to the containing
procedure and its executed instructions.

### Call Census and Attribution Boundaries

The census is entirely dynamic. A call row is recorded at the VM boundary
where the opcode path, concrete target, actual argument count, caller site, and
activation result are authoritative. `external_root`,
`signal_bytecode`, and `signal_native` keep non-instruction
entry distinct. Failed or unresolved attempts have an explicit outcome and
`none_failed` frame disposition. Native calls use
`no_child_native`; successful bytecode and root/signal frames use
`fresh` or `reused`.

Argument mechanics are not inferred from opcode adjacency. At each successful
argument-bearing call, the profiler performs a backward slice of the executed
straight-line trace from the actual call window. It uses the NR-04
`rxop_effects()` overwrite/flow contract as the definition boundary.
Only reached `SWAP_REG_REG` instructions are setup swaps and only
reached whole-value `COPY_REG_REG` definitions are defensive
argument copies. Normal restoration reconstructs the setup permutation and
buffers subsequent swaps until their combined effect recovers the pre-call
mapping. Only a completed recovery sequence is credited; a mapping carried
into another call or to frame completion is not treated as a tracking failure.
Every other executed `SWAP_REG_REG` or `COPY_REG_REG`
remains explicitly unclassified.

`RET_REG` placement is measured inside its real VM branch. A true
local with unchanged base mapping is moved; an argument, global, linked/swapped
local, or other non-local source is copied. This internal copy decision is not
a `COPY_REG_REG` instruction and is therefore separate from
call-window copy counts. Exceptional unwind is also separate from ordinary
return placement.

All counters saturate. Non-zero `census_tracking_unavailable`,
`attribution_degraded`, `restoration_failures`, or an
`overflowed`/`degraded` status means the affected census
is incomplete. Zero-count categories remain in CSV so absence is not confused
with parser loss.

### CSV Format

CSV is the best input for spreadsheets, scripts, and comparisons between runs.
The current format identifies itself as schema version 4 and preserves the
schema-3 24-column header:

```text
section,name,value,id,count,total_ns,average_ns,min_ns,max_ns,percent,selected,entries,resumes,terminals,module,kind,completed,unwound,return_type,args,bytes,max_bytes,high_water,status
```

Columns that do not apply to a row are empty or zero. Interpret rows by their
`section` value:

| `section` | Row use |
|---|---|
| `summary` | Schema, VM mode, result, timer calibration, poll/error state, and overflow/tracking status. |
| `instruction` | One row per executed opcode. |
| `transition` | One row per observed transition kind. |
| `interrupt` | Scan totals and per-signal selection/entry/resume/terminal data. |
| `procedure` | One or more metric rows per called procedure, method, factory, or native routine. |
| `allocation` | One row per allocation/value/frame counter, including byte and high-water fields where they apply. |
| `census` | Aggregate call path, exact arity, callable kind, frame disposition, and outcome rows; zero path/kind/disposition/outcome categories are retained. |
| `call` | One dynamic target/site/path/arity/kind/frame/outcome row. `bytes`, `max_bytes`, and `high_water` carry setup swaps, normal restoration swaps, and defensive argument copies for this section only. |
| `return` | Dynamic return-placement decisions. |
| `dynamic` | Method/factory selection attempts, successes, and failures. |
| `mechanics` | Global call-window setup/restoration/unclassified swap and defensive/unclassified copy counts. |
| `unwind` | Signal-unwind frame/window/slot/restoration counters. |

For a bytecode `procedure` row, `value` is one of `elapsed`,
`inclusive_body`, `self`, `native_child`, `entry_overhead`, or
`exit_overhead`. A native row uses `native_total`. Callable rows also carry
`module`, `kind`, `completed`, `unwound`, `return_type`, and `args` metadata.
Do not assume that one CSV row represents all metrics for a callable.

### Allocation and Frame Counter Definitions

The allocation view counts successful allocation requests made while the
timing-profile run is active. `bytes` is the sum of requested capacities and
`max_bytes` is the largest single requested capacity. A successful `realloc`
is one request for its full new capacity, not a net-growth byte count. These
are request counters, not retained/live-heap measurements.

| Counter | Exact scope |
|---|---|
| `frame_blocks` | Fresh combined `stack_frame` blocks allocated when the selected procedure has no reusable block large enough. `bytes` includes the frame, pointer arrays, and inline local/a0 `value` storage. |
| `standalone_values` | Successful `value_f()` heap allocations. Each request also contributes one `value_slots` slot. |
| `attribute_value_blocks` | Fresh or replacement blocks containing object-attribute `value` structs. Their element counts contribute to `value_slots`. |
| `attribute_pointer_storage` | Successful allocations or capacity changes for attribute pointer arrays, unlinked-attribute pointer arrays, and attribute-buffer pointer arrays. |
| `string_buffers` | Successful heap buffer allocations/capacity changes made by VM string growth or alias-safe string concatenation. Inline small strings do not count. |
| `binary_buffers` | Successful heap buffer allocations/capacity changes made by VM binary growth or binary concatenation. |
| `value_slots` | Total `value` structs supplied by counted standalone, frame-local/a0, and attribute-value allocations. `bytes` is slots times `sizeof(value)`; `max_bytes` is the largest counted value block. It deliberately overlaps the containing allocation byte totals. |
| `frame_activations` | Every activated bytecode frame, fresh or recycled. `high_water` is the maximum simultaneously active bytecode-frame count observed in the run. |
| `frame_reuses` | Activations satisfied from a procedure's frame recycler rather than a fresh `frame_blocks` request. |

The scope excludes module loading, profiler/RXSEQ bookkeeping, plugin-private
and native-payload ownership, OS/TLS support, reference-lifetime payloads, and
temporary native conversion buffers. It is intentionally a focused VM
value/frame/storage view, not a replacement for a heap profiler. Allocation
rows report `complete`, `overflowed`, or `degraded`; the summary also carries
`allocation_tracking_unavailable`. A non-zero active-frame balance at report
time degrades `frame_activations` because its high water cannot then be treated
as a balanced run.

### Interpreting Timing Responsibly

The timings are raw, instrumented monotonic wall times. The profiler calibrates
the minimum positive interval between adjacent clock reads and reports how
often calibration reads had a zero delta, but it does not subtract a guessed
timer cost from short instructions.

For useful results:

- profile an optimized build and a representative workload;
- repeat the run when scheduler noise or very short samples could dominate;
- compare percentages and like-for-like profiled runs before comparing
  absolute nanoseconds;
- use call counts and larger totals to decide where finer investigation is
  worthwhile;
- use the ordinary, non-profiling Release VM for final performance numbers.

Instruction, transition, procedure, and interrupt sections are overlapping
views. Procedure body times also overlap their nested callees. Do not add the
sections or all procedure rows together as if they partition elapsed time.

`invalid events`, `counter overflow`, `procedure tracking=degraded`, or
`allocation tracking=degraded` in the table heading—and their CSV summary or
allocation-row equivalents—mean that the affected view is incomplete.
Preserve those status fields whenever profiles are processed automatically.

## Dynamic Instruction-Sequence Profiles

Sequence mode finds frequently executed straight-line instruction patterns.
It records where sequential windows occurred during the run; the separate
`rxseq` program then decodes the original RXBIN modules, normalizes operands,
and clusters equivalent patterns.

Capture two-instruction windows from the example:

```sh
rxvm --sequence-count=2 \
  --sequence-output profiling-demo.rxseq \
  profiling_demo.rxbin
```

`--sequence-count` accepts only `2`, `3`, or `4`, and it must be paired with
`--sequence-output`. The output is a compact binary execution profile, not a
text report.

Analyze it as a table:

```sh
rxseq profiling-demo.rxseq profiling_demo.rxbin
```

Or write candidate data as CSV:

```sh
rxseq profiling-demo.rxseq profiling_demo.rxbin \
  --output profiling-demo-candidates.csv
```

As with the timing report, `.csv` is recognized case-insensitively; another
output extension selects the human-readable table.

### What Forms a Window

A window continues only across an actual sequential fall-through transition
in the same module and runtime frame. Bytecode calls and returns, taken
branches, interrupt entry or resume, external frame entry, and program
termination break the window. A native call that returns normally stays within
its CALL instruction and can participate in a sequential window. A branch
instruction can be the last instruction in a window, but a window never crosses
a branch when it is taken.

Loops increase the dynamic count of a site instead of enlarging the `.rxseq`
file. The capture keeps one counter for each possible starting slot in the
loaded modules and writes only non-zero sites.

### Exact Modules Are Required

The `.rxseq` file deliberately stores module identity and counts, not a second
copy of the bytecode. Supply `rxseq` with the complete module set from the
profiled run. Argument order does not matter, but names, expanded content
hashes, and instruction sizes must match exactly, and no extra modules may be
added.

For example, a two-module run is captured and analyzed as follows:

```sh
rxvm --sequence-count=2 --sequence-output application.rxseq \
  application.rxbin library.rxbin
rxseq application.rxseq application.rxbin library.rxbin
```

An `rxvme` or `rxbvme` run also records its embedded standard-library modules.
Analysis therefore needs the exact corresponding library RXBIN image. For the
simplest reproducible workflow, use `rxvm`/`rxbvm` with an explicit module set
or profile a single linked RXBIN image.

`rxseq` rejects a missing module, an extra module, a content-hash mismatch, or
an instruction-size mismatch. Keep the `.rxseq` file and its exact RXBIN inputs
together.

### Reading Candidate Output

Operands are alpha-renamed by first occurrence across the whole window.
Registers become `r1`, `r2`, and so on. Every other encoded operand—such as a
literal, pool constant, label, or procedure reference—becomes `c1`, `c2`, and
so on. Reuse remains visible, so these two sites cluster even if their physical
register numbers differ:

```text
IADD_REG_REG_REG(R17,R5,R9) | COPY_REG_REG(R5,R22)
    -> IADD_REG_REG_REG(r1,r2,r3) | COPY_REG_REG(r2,r4)
```

The table heading reports the selected window length, number of clusters,
number of static sites, and whether a counter overflowed. Each candidate row
contains:

| Field | Meaning |
|---|---|
| `Count` | Sum of dynamic executions across all clustered sites. |
| `Sites` | Number of distinct starting sites with the normalized pattern. |
| `Modules` | Number of modules containing those sites. |
| `Args` | Number of distinct normalized register/constant symbols. |
| `Status` | `candidate` when the window decoded successfully. |
| `Pattern` | Normalized instruction and operand-reuse pattern. |
| `mapping` / `example` | One concrete operand mapping and module/start-slot location. |

CSV output uses these columns:

```text
rank,count,sites,modules,symbols,status,pattern,mapping,example_module,example_start
```

Candidate extraction is a discovery aid, not an automatic bytecode rewrite.
Before combining instructions or adding an optimizer rule, separately review
control flow, liveness, aliasing, exceptions, interrupts, and observable VM
semantics. Treat `overflow=yes` as an incomplete count rather than silently
ranking the saturated data.

## Common Errors

| Symptom | Resolution |
|---|---|
| `rxvm -h` does not list profiling options | Reconfigure and rebuild with `-DCREXX_VM_PROFILING=ON`; normal builds intentionally omit them. |
| `--profile and --sequence-count are separate run modes` | Run timing and sequence collection separately. |
| A sequence option says its partner is required | Supply both `--sequence-count` and `--sequence-output`. |
| `profiled module is missing` or `module content hash mismatch` | Analyze with every exact RXBIN module used by the captured run. |
| An expected procedure is absent | Check whether it was inlined and whether the RXBIN contains `META_FUNC` metadata. |
| Procedure tracking is degraded | Treat callable data as incomplete; instruction and transition data may still be useful. |
