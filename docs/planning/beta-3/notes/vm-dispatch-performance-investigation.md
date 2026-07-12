# VM Dispatch Performance Investigation

Status: under investigation for Release 1

Date opened: 2026-07-11

## Question

Why is the switch-dispatch `rxbvm` faster than the computed-goto `rxvm` on
the measured Linux and macOS ARM64 hosts, when direct threading was previously
expected to be the faster execution mode?

This note is the handoff point for native Intel Linux investigation. It records
confirmed source history, bounded macOS experiments, external compiler
guidance, and the measurements still required. Do not change the default VM or
remove safety checks from the current evidence.

## Confirmed Facts

- Linux AArch64 under UTM with GCC 15.2 measured `rxbvm` approximately 10% to
  30% faster than `rxvm` for most current performance fixtures.
- Native Apple M5 macOS with Apple clang 21 measured the same direction, but a
  smaller 2% to 20% spread. UTM is therefore not the sole explanation.
- Both modes check the same pending-interrupt state before dispatch.
- The current computed-goto path is not the original in-place threaded layout.
  Commit `720d3253c` on 2026-04-22 removed `bin_code.impl_address` and added a
  separate per-module `prepared_dispatch` array for RXBIN/runtime-state
  separation.
- Before `720d3253c`, sequential dispatch loaded `(next_pc)->impl_address`.
  It now evaluates `next_pc - current_module->segment.binary`, loads
  `current_module->prepared_dispatch`, and indexes that second array on every
  instruction.
- The current `run()` body is also larger in `rxvm`: on the measured macOS
  Release build its text was 423,732 bytes versus 408,508 bytes for `rxbvm`, a
  15,224-byte or 3.6% difference.
- Apple clang merges the hundreds of source-level computed-goto dispatches into
  one shared main dispatch branch. Disassembly found six indirect `br`
  instructions in `run()`, but five belonged to other switch/jump structures;
  all normal VM handlers converge on the remaining dispatch site. The classic
  per-handler branch-predictor advantage is therefore absent in this build.

These facts make the 2026-04-22 dispatch-layout change a credible regression
candidate. They do not yet prove that it is the only cause or that Intel hosts
behave identically.

## Native macOS Experiments

The source baseline was `e5c912b93f70ca6c3dc7e03d889379352e5b3525`.
Each result is the median of seven serial in-process samples after one warmup,
using the same pre-linked optimized `.rxbin` with each experimental VM.

### Cache Module Dispatch Pointers

A disposable worktree changed only the computed-goto implementation. Whenever
`current_module` changed, it cached the module binary base and
`prepared_dispatch` pointer in `run()` locals. The per-instruction lookup then
used those locals. This preserves behavior and retains the separate dispatch
array.

The experimental shape was:

```c
#define SET_CURRENT_MODULE(module_) { \
    current_module = (module_); \
    current_binary = current_module->segment.binary; \
    current_dispatch = current_module->prepared_dispatch; \
}
#define CALC_DISPATCH(n) { \
    next_pc = pc + (n) + 1; \
    next_inst = current_dispatch[(size_t)(next_pc - current_binary)]; \
}
```

All 15 assignments to `current_module` in `run()` used
`SET_CURRENT_MODULE`. A production implementation should replace the macro
with the clearest repository-consistent helper form and test every transition.

| Hot section | Baseline `rxvm` us | Cached-pointer `rxvm` us | Change |
| --- | ---: | ---: | ---: |
| Binary `.u32` write | 1,262 | 1,061 | 15.9% faster |
| Binary `.u32` read | 1,528 | 1,293 | 15.4% faster |
| Binary `.i64` write | 1,255 | 1,054 | 16.0% faster |
| Binary `.i64` read | 1,537 | 1,293 | 15.9% faster |
| JSON validate | 16,107 | 15,532 | 3.6% faster |
| JSON count | 31,871 | 31,089 | 2.5% faster |
| Tinyexpr lex | 38,236 | 36,923 | 3.4% faster |
| Tinyexpr evaluate | 99,457 | 96,819 | 2.7% faster |
| Integer jump table | 22,854 | 20,747 | 9.2% faster |
| Padded-string jump table | 14,703 | 13,953 | 5.1% faster |
| Numeric-string jump table | 21,344 | 21,609 | 1.2% slower |

The binary result nearly closes the previously measured native macOS switch
gap: `rxbvm` medians were 1,037/1,299 us for `.u32` write/read and
1,027/1,296 us for `.i64` write/read. The mixed larger-workload results mean
that branch prediction, code layout, and the compiler remain relevant.

The cached-pointer `run()` body was 422,216 bytes, 1,516 bytes smaller than the
baseline despite retaining the same dispatch representation.

### Prevent Compiler Dispatch-Site Merging

A third measurement-only variant added a unique empty `asm volatile` marker at
each computed dispatch expansion, following the technique investigated by
CPython. This is not portable C and is not a proposed default. Apple clang then
emitted 729 indirect branches in `run()` instead of six, while `run()` grew
from 422,216 to 487,164 bytes, or 15.4%.

The marker inserted immediately before `goto *next_inst` was:

```c
__asm__ volatile ("" : : "i" (__COUNTER__));
```

| Hot section | Baseline `rxvm` | Cached pointers | Cache plus barrier | `rxbvm` |
| --- | ---: | ---: | ---: | ---: |
| Binary `.u32` write | 1,275 | 1,050 | 1,109 | 1,030 |
| Binary `.u32` read | 1,526 | 1,279 | 1,376 | 1,243 |
| JSON count | 31,839 | 30,975 | 31,191 | 30,201 |
| Tinyexpr evaluate | 99,542 | 98,847 | 97,003 | 94,752 |
| Integer jump table | 22,534 | 20,999 | 22,487 | 20,978 |
| Padded-string jump table | 14,602 | 13,695 | 13,356 | 14,365 |
| Numeric-string jump table | 21,812 | 21,394 | 20,757 | 21,807 |

The replicated sites help some opcode sequences, especially string/numeric
jump and Tinyexpr dispatch, but hurt binary, JSON, and integer jump workloads
relative to pointer caching alone. This is consistent with a tradeoff between
branch-target prediction and instruction-cache/code-size pressure. A global
anti-merging barrier is not justified by these results.

### Remove Pending-Interrupt Check

A second disposable worktree removed the computed-goto pending-interrupt check
only to measure its cost. This changes VM semantics and is not a candidate fix.
It improved the sampled sections by approximately 1% to 5%, including 3.6% for
`.u32` write, 2.0% for `.u32` read, 1.6% to 2.4% for JSON, 4.6% to 4.7% for
Tinyexpr, and 1.2% for integer jump dispatch.

The interrupt check therefore has measurable cost and may affect compiler code
layout, but it does not account for the main binary dispatch gap. Since both VM
modes perform the check, it is not evidence that interrupt handling alone
caused the mode reversal.

## Extended Hot-Loop Review

The follow-up review covered 513 source handler dispatch points, every manual
dispatch recalculation, all 15 frame/module activation sites, operand lookup,
branch target calculation, interrupt polling, compiler flattening, runtime
instruction representation, and generated ARM64 code.

Two additional hot-path costs were confirmed:

- register operands repeatedly reach through `current_frame->locals`, while
  branch targets repeatedly reach through
  `current_frame->procedure->binarySpace->binary`;
- taken conditional branches in `rxvm` calculate and load the fall-through
  handler first, then calculate and load the selected target. This is most
  visible on loop backedges and successful jump-table lookups.

Moving handler resolution to a lockstep cursor removed the second lookup but
was substantially slower. Resolving the next handler before executing the
current instruction hides load latency; preserving that overlap matters more
than eliminating the occasional duplicate lookup on this host.

### Coherent Frame-State Cache

A common `rxvm`/`rxbvm` experiment refreshed the active frame's locals array,
binary base, constant-pool base, module, and binary-space pointers only when the
frame changed. Operand and branch macros then used those local pointers.

Caching only the binary base or only operand state produced inconsistent code
generation and regressed some `rxbvm` cases. Caching the complete state
together consistently improved both modes. This must therefore be one atomic
activation operation, not a collection of unrelated local substitutions.

The frame's `locals` pointer is fixed by `frame_f()`; link/reference operations
replace entries but do not replace the array itself. Procedure binary-space and
module pointers are fixed for an active resolved procedure. The existing frame
changes are bounded and auditable, but a production implementation must route
all of them through one activation macro and include Debug-only state
invariants.

### Interleaved Clang And GCC Results

The final comparison rotated VM order between samples to reduce thermal/order
bias. Values are medians of seven retained samples after one warmup, in
microseconds:

| Variant | Binary write | Binary read | JSON count | Tinyexpr eval | Integer jump | String jump | Numeric jump |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Clang `rxvm` baseline | 1,263 | 1,513 | 31,970 | 99,325 | 22,281 | 14,661 | 20,977 |
| Clang `rxbvm` baseline | 1,025 | 1,251 | 30,173 | 91,034 | 20,578 | 13,732 | 20,998 |
| Clang frame cache + opcode dispatch | 952 | 1,160 | 29,401 | 90,408 | 19,915 | 14,416 | 20,988 |
| Clang frame cache + in-place upper bound | 925 | 1,176 | 28,898 | 87,561 | 18,158 | 13,589 | 20,574 |
| Clang frame-cached `rxbvm` | 941 | 1,170 | 29,279 | 88,161 | 18,552 | 13,373 | 21,673 |
| GCC `rxvm` baseline | 1,492 | 1,863 | 33,365 | 106,594 | 26,772 | 17,172 | 20,857 |
| GCC frame cache + in-place upper bound | 956 | 1,193 | 29,268 | 90,187 | 16,034 | 12,678 | 17,769 |
| GCC `rxbvm` baseline | 1,996 | 2,333 | 35,271 | 124,604 | 34,891 | 22,366 | 27,880 |

For Apple clang, frame caching plus opcode-indexed computed goto improves the
current `rxvm` by about 2% to 25% except for the already-tied numeric jump.
JSON, Tinyexpr, and integer jump improve by about 8% to 11%. The result is close
to or faster than baseline `rxbvm` in six of the seven sections. It also removes
`prepared_dispatch` and its per-module allocation entirely.

The in-place upper bound overwrote the instruction cell with its handler
pointer, deliberately breaking runtime opcode reflection. It is not shippable,
but it proves the original one-load dispatch advantage: with frame caching it
improves current Clang `rxvm` by 2% to 27% across this table and is approximately
tied with frame-cached `rxbvm`.

Under Homebrew GCC 16.1, the same upper bound improves current GCC `rxvm` by
12% to 40%. GCC retains approximately 1,364 indirect branches in `run()` and
therefore preserves the classic per-handler prediction shape, while Apple
clang merges normal dispatch to one site. GCC `rxvm` is much faster than GCC
`rxbvm`, but unmodified GCC binaries are slower than Apple-clang binaries on
this M5. GCC is useful and competitive after the architecture change, not a
blanket replacement for clang.

The high-level planning interpretation is:

| Change | Gain against that compiler's current `rxvm` |
| --- | ---: |
| Macro simplification and disabled instrumentation hooks | 0% target; any material change is a regression to investigate |
| Clang frame cache plus opcode-indexed fallback | measured -0.1% to 24.6%; 9.0% median |
| Clang frame cache plus runtime-image hot-path projection | measured upper bound 1.9% to 26.8%; 11.8% median |
| GCC frame cache plus runtime-image hot-path projection | measured upper bound 12.3% to 40.1%; 26.2% median |

The Clang runtime-image upper bound versus frame-cached `rxbvm` is close to
parity: `rxvm` wins five of seven sections, with a 1.3% median advantage and a
range from 1.6% slower to 5.1% faster. It therefore predicts recovery of the
current regression and a small workload-dependent `rxvm` advantage, not a
universal win. The GCC upper bound is 17% to 54% faster than unmodified GCC
`rxbvm`, but that is not a like-for-like comparison because the GCC `rxbvm`
frame-cache variant was not measured. Native Intel and Windows measurements
remain necessary.

GCC also stresses build scalability: the unmodified switch `rxbvm` build took
378.93 seconds and the experimental state/in-place `rxvm` build took 249.19
seconds. GCC `run()` was 1,596,064 bytes for baseline `rxvm` and 1,571,616 bytes
for `rxbvm`, compared with 423,732 and 408,508 bytes under Apple clang.

The Homebrew GCC formula supports Apple Silicon. The default CREXX macOS
`NETWORK` TLS backend is Clang-specific because it requires `-fblocks`; these
VM-only GCC measurements used `CREXX_ENABLE_TLS=OFF`. A supported GCC macOS
build would need `OPENSSL`, no TLS, or an explicit CMake compatibility policy.

### Runtime Instruction Image

The production form of the one-load result must not mutate
`module->segment.binary`. Instead:

1. Keep `segment.binary` as the immutable canonical RXBIN/reflection image.
2. Replace `prepared_dispatch` with an equally sized `bin_code` execution copy
   for computed-goto `rxvm`.
3. Copy operands unchanged and replace only instruction cells in the execution
   copy with handler pointers prepared from validated opcodes.
4. Point `pc`, `next_pc`, branch targets, and return addresses into the execution
   image. Metadata instructions continue reading the canonical image.
5. Convert between execution pointers and canonical instruction indexes through
   explicit helpers for signals, retries, caller addresses, and diagnostics.

This has the same instruction-memory footprint as today: canonical binary plus
one pointer-sized slot per binary slot. Unlike `prepared_dispatch`, every byte
of the second image is useful to execution. It also leaves the canonical image
read-only and is compatible with a future mmap-backed RXBIN.

Handler addresses are process-local and must never be serialized. Because
`rxvm_prepare()` can prepare in one `run()` invocation and execute in another,
the interpreter entry should explicitly prevent inlining/cloning where the
compiler supports that attribute, matching GCC's labels-as-values guidance.

### Rejected Or Deferred Variants

| Variant | Assessment |
| --- | --- |
| Cache binary base and `prepared_dispatch` only | Safe fallback; recovers about 15% on binary handlers but retains the parallel side table |
| Lockstep dispatch cursor | Rejected; 15% to 30% slower from lost early target loading and greater register pressure |
| Store a 32-bit handler-relative offset in `no_ops` | Rejected; slower dependent load/add and mutates canonical runtime metadata |
| Opcode-indexed computed goto | Safe fallback; broadly fast, simplest representation, and halves current runtime instruction memory |
| Remove `RX_FLATTEN` | Rejected for now; shrinks `run()` by roughly 9% to 10% but is neutral or slower at runtime |
| Add an `interrupts` unlikely hint | Rejected; severe code-layout regressions under Apple clang |
| Remove interrupt polling | Invalid; only an upper-bound measurement and changes signal/breakpoint semantics |
| Replicate dispatch sites with an inline-assembly barrier | Rejected globally; helps some branch-heavy shapes and hurts binary/JSON through code growth |
| Split synchronous/asynchronous interrupt polling | Post-Release 1; maximum observed opportunity is only about 1% to 5% and semantics are delicate |
| Tail-call handlers, cold-path outlining, or generated superinstructions | Post-Release 1 architecture work requiring counters and opcode-frequency evidence |

## Proposed Implementation Slices

### Slice 1: Simplify The Dispatch Macro Contract

Perform a semantics-preserving macro cleanup before changing representation.
Keep the early next-handler load: the delayed/lockstep experiment proved that
moving resolution to the end of an instruction is slower.

Replace the current `CALC_DISPATCH`/`CALC_DISPATCH_MANUAL` pairing and scattered
raw `next_pc` assignments with a small intent-based surface:

- activate a non-null frame and all of its cached execution state;
- advance by a compile-time operand count and resolve the sequential target
  early;
- select an indexed branch/jump target and resolve it early;
- select an existing execution pointer for call/return/interrupt resume;
- dispatch with the common pending-interrupt rule;
- resume an interrupt while preserving breakpoint semantics;
- convert execution index to pointer and execution pointer to canonical index.

The exact macro names are implementation details, but each macro must evaluate
arguments once, use `do { ... } while (0)` or an equally safe statement form,
and have one documented ownership/state contract. Keep mode-specific code
inside the macro definitions rather than throughout handlers. Do not combine a
rename of the historical `NTHREADED` build define with this performance change;
the simplified macro boundary can hide that legacy naming for now.

The simplified surface must also provide compile-time instrumentation extension
points. At minimum, instrumentation builds need hooks for:

- VM execution begin and end;
- instruction begin and retire, including the executed opcode, canonical module
  and instruction location, selected target, and transition reason;
- active-frame changes, including call, return, interrupt, and external-entry
  transitions;
- interrupt selection, handler entry, resume, and terminal signal paths.

Every instruction-begin event must have exactly one retire or terminal event,
including instructions that signal, branch, call, return, stop the VM, or leave
through an error path. The intent-based transfer macros should enforce this
balance rather than asking individual handlers to remember instrumentation.
Runtime-image builds must report canonical RXBIN coordinates through the common
execution address/index helpers; instrumentation must never expose or persist
process-local handler pointers.

Hooks are internal compile-time extension points, not a new public VM callback
ABI. Their default definitions must expand to no code, introduce no runtime
condition, and leave the ordinary generated dispatch shape and performance
unchanged. Define the hook contract, default no-op definitions, transition
reason values, and compile-time backend binding in one internal header shared
by `rxvm` and `rxbvm`; handlers must not acquire backend-specific `#ifdef`
blocks. An enabled build may keep per-context state and select an
instrumentation backend, but a hot hook must not allocate, format output, take
a process-wide lock, or invoke a general function pointer for every
instruction. Suitable first backends are per-opcode counters and a bounded
per-context event buffer; timestamps and sampling policy belong to the backend
because reading a clock for every instruction can materially perturb the result
being measured. Buffer draining and human-readable output happen outside the
dispatch loop.

Run focused semantics and the complete performance matrix after this cleanup.
Add an instrumented test backend that verifies balanced events and canonical
locations across sequential instructions, branches, calls/returns, interrupts,
signals, and termination in both VM modes. Also compare the non-instrumented
generated code and timings with the pre-cleanup baseline. Any unexplained
generated-code or timing change must be resolved before Slice 2.

### Slice 2: Common Active-Frame State

- Add one local activation macro/helper in `run()` that updates
  `current_frame`, binary space, module, execution binary, constant pool, and
  locals together.
- Route every initial frame, call, return, interrupt unwind, branch-to-handler,
  and breakpoint return through it.
- Use cached state in operand macros and branch target calculation; keep setup
  and ownership code explicit where the current frame is intentionally changing.
- Add Debug-only assertions that cached pointers match the active frame.
- Build and test both `rxvm` and `rxbvm`, then repeat the performance matrix
  before starting Slice 3.

### Slice 3: Computed-Goto Runtime Image

- Replace `module->prepared_dispatch` with an owned execution image for
  computed-goto builds; retain the canonical image unchanged.
- Add checked allocation/copy/preparation and cleanup for initial and
  late-loaded modules. Keep `rxvm_prepare()` idempotent.
- Add execution address/index helpers and audit signal source addresses,
  retry/skip behavior, caller metadata, calls, returns, branches, jump tables,
  panic locations, and external `rxvml` calls.
- Keep `METALOADINST` and operand reflection on the canonical image.
- Add compile-time pointer/slot-size assertions and no-inline/no-clone protection
  for stored label addresses.
- Remove the old side-table allocation only after full semantics and sanitizer
  coverage passes.

### Slice 4: QA And Compiler Matrix

- Run focused call/return, reflection, `rxvml`, dynamic load, interface,
  signal/action, breakpoint, corruption, binary, jump-table, and performance
  tests under both VM modes.
- Run full Debug CTest, the supported ASan/LSan runner, Release CTest, and the
  cross-platform pipeline.
- Repeat Apple clang and Homebrew GCC with interleaved raw samples. Then repeat
  GCC/clang plus hardware counters on native Intel Linux before changing any
  compiler or default-VM policy.

## External Evidence

- The [GCC labels-as-values documentation](https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html)
  describes storing handler labels in threaded code as a fast interpreter
  technique, but it does not promise that computed goto beats a compiler-built
  switch on every target.
- The [GCC optimization documentation](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html)
  explicitly notes that computed-goto programs may run faster with
  `-fno-gcse`. Release builds use `-O3`, which enables GCSE through `-O2`.
- Ertl and Gregg's [interpreter dispatch study](https://jilp.org/vol5/v5paper12.pdf)
  shows that indirect-branch prediction and misprediction penalties dominate
  many interpreter designs.
- CPython's [computed-goto dispatch investigation](https://github.com/python/cpython/issues/129987)
  records that GCC and LLVM may merge or duplicate dispatch sites during
  optimization, changing the branch-prediction benefit.
- CPython's [tail-call interpreter investigation](https://github.com/python/cpython/issues/128563)
  likewise treats dispatch performance as compiler, ABI, and architecture
  dependent, with AArch64 requiring separate validation.

There is no sound general rule that Intel is simply better at computed gotos.
The relevant variables are the exact microarchitecture's indirect predictor,
compiler transformations, generated code size/layout, and the cost of obtaining
the next handler address.

Homebrew GCC 16.1 was available as `/opt/homebrew/opt/gcc/bin/gcc-16` and was
included in the extended comparison above. `/usr/bin/gcc` remains an Apple
clang driver and must not be used to claim a GCC result.

## Native Ubuntu Intel Investigation

Use the cross-platform protocol in
`docs/planning/beta-3/reports/linux-vm-sanitizer-performance-review.md` and
retain all raw samples. Build and measure from the same commit as the report or
record the newer commit explicitly.

1. Establish the unmodified `rxvm` versus `rxbvm` ratios for the complete
   workload matrix. Use one warmup and seven serial samples; do not overlap
   builds, CTests, or benchmark runs.
2. Run `perf stat` for at least binary, JSON, Tinyexpr, and integer jump-table
   workloads. Record cycles, instructions, branches, branch misses, cache
   references/misses, and supported L1 instruction-cache and iTLB events.
3. Use `perf record`/`perf report` and retain annotated disassembly of `run()`.
   Confirm whether the computed-goto handlers keep distinct indirect dispatch
   sites or the compiler merges them.
4. In disposable worktrees, repeat the coherent frame-state cache, opcode-indexed
   dispatch, and runtime-image upper-bound experiments. Run focused semantics
   tests before timing and distinguish the shippable runtime-copy design from
   the reflection-breaking in-place measurement.
5. Compare GCC `-O2`, `-O3`, and `-O3 -fno-gcse` for both VM modes. Report code
   size and counters as well as elapsed time. Do not select a non-default flag
   from one fixture.
6. If clang is installed, repeat the paired Release comparison with clang using
   otherwise equivalent flags. Record exact compiler versions.
7. Repeat the normal Debug and supported ASan/LSan sweep independently of the
   performance tree. Sanitizer binaries are validation tools, not benchmark
   inputs.

If counters show poor indirect prediction from a merged dispatch site, an
anti-merging barrier may be measured in the disposable worktree, but it must be
reported with `run()` size and instruction-cache counters. Do not treat more
dispatch sites as inherently better.

For each variant, report the seven raw samples, median, `rxbvm / rxvm` ratio,
`run()` text size, cycles/instruction, branch-miss rate, and instruction-cache
evidence. Absolute time is secondary to paired ratios on the same host.

## Release 1 Decision Gate

This is a Release 1 investigation because the current default is documented as
the fast VM and the measured architecture candidates recover substantial
performance in both VM modes. Before Release 1:

- complete native Intel Linux counters and at least one Windows x86-64 paired
  timing run;
- implement and validate the coherent frame-state cache, then decide between
  the opcode-indexed fallback and the separate runtime instruction image for
  computed-goto execution;
- run full `rxvm`/`rxbvm` semantics, late-load, interrupt, metadata, sanitizer,
  and cross-platform tests for the chosen change;
- update VM documentation and default-mode claims from measured results.

Handler pointers must not be reintroduced into the canonical/serialized
instruction image. The separate runtime image is the measured design that
restores one-load dispatch while preserving reflection and runtime-state
separation.
