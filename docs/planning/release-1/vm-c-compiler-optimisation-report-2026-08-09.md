# VM and C Compiler Optimisation Report

Status: completed Apple ARM64 investigation; internal measurement framework
retained; no product-default policy selected

Date: 2026-08-09

## Purpose

This report records the complete `rxvm` instruction-handler refactoring and
code-generation investigation, including the unsuccessful intermediate forms.
It explains why apparently harmless C source changes produced large and
sometimes opposite performance movements under Apple Clang and GCC, what was
required to recover an equivalent all-inline control, and which source shapes
gave the best observed optimisation for each tested compiler.

The report covers both concrete interpreter engines:

- `rxtvm`, the direct-threaded computed-goto engine; and
- `rxbvm`, the portable numeric-opcode switch engine.

`rxvm` is a compiler-selected product alias, not a third engine. No result here
changes the public RXAS/RXBIN format, plugin ABI, language semantics, canonical
RXBIN image, default handler panel or product VM selection.

## Executive conclusion

The original profile-selected slowdown was not proportional to the number of
handlers called out of line. A single reachable but never-executed outlined
handler could materially slow Apple Clang. The primary cause was the C-level
address escape created by a pointer-rich facade over `run()` locals. It changed
register allocation, stack homes, alias analysis and, for `rxtvm`, the handling
of the function-local label table. The effect existed even when the outlined
edge was never taken.

Clang and GCC then required different repairs:

- Clang performs best for non-inline panels when `run()` retains scalar locals,
  takes a value snapshot only on the cold outlined edge, and sends all outlined
  public instructions through one cold trampoline.
- GCC performs better with the earlier per-instruction label/case and direct
  outlined-call shape using the pointer facade. Forcing the Clang repair onto
  GCC improved `rxbvm` but regressed `rxtvm`.
- The all-inline control must preserve the exact earlier R2 facade source shape,
  even though the compiler removes that facade from the optimized product.
  Deleting source that appeared dead changed compiler heuristics and made
  threaded execution about 5-6% slower in the first formal attempt.

There was no evidence that helper sub-inlining caused the regression. Across
the 176 selected public inline handlers, neither compiler changed a successful
helper-inlining decision. Converting the inner value-manipulation helpers to
macros is therefore not justified by this investigation and could remove useful
compiler discretion while enlarging the owner again.

The final experimental panel places 176 public handlers and the two existing
private fused handlers inline: 178 of 590 non-reserved public-plus-private
definitions, or 30.17%. Excluding noisy Base64, profile-30 versus the rebuilt
all-inline control is:

| Compiler | `rxtvm` | `rxbvm` |
| --- | ---: | ---: |
| Apple Clang 21 | -0.341% | +0.201% |
| Homebrew GCC 16.1 | +1.379% | +6.422% |

Positive values mean higher normalized throughput. The Clang result is
effectively neutral; the GCC result is favorable. The panel remains an
experimental measurement shape rather than a selected default.

## Interpreter and handler model

The R2 refactor moved 651 semantic handler definitions into five coherent
internal include files:

- 649 public opcode/sentinel handlers; and
- two process-private execution-image handlers,
  `PRIVATE_R1_RELINK` and `PRIVATE_R2_COPYATTR1`.

Each handler is written once behind `RXVM_HANDLER` or
`RXVM_PRIVATE_HANDLER`. `rxvmhandlerpolicy.h` independently chooses whether a
definition expands directly inside `rxvm_run_owned_core()` or is emitted as a
force-noinline callable function. `INTERRUPT` is an owner pseudo-label, not an
RXAS instruction handler.

Three reproducible policies provide the experimental boundaries:

| Policy | Meaning |
| --- | --- |
| `all-inline` | Every handler body expands inside `run()` |
| `all-outline` | Every handler is callable; minimum owner and maximum call boundary |
| `profile-30` | Frozen hot panel inline, remaining handlers callable |

The two dispatch engines consume the same handler bodies but present very
different control-flow structures to a C compiler. `rxtvm` requires labels to
remain owned by one stable noinline/noclone function and dispatches through
process-local label addresses. `rxbvm` dispatches numeric opcodes through a C
switch. Both execute a process-owned execution image while the canonical RXBIN
image remains immutable and authoritative for serialization, reflection,
source coordinates and debug identity.

## Baseline and equivalence contract

There are three distinct controls and they must not be conflated:

1. The untouched pre-refactor interpreter at
   `6a65b9c685b3776da211bcd209af14fcf23be445`.
2. The completed R2 macro framework at
   `fd54b616764ef880270f4bce9dd202b476bf559c` with every handler inline.
3. The rebuilt R3 all-inline shape containing the later compiler-specific
   machinery but required to reproduce R2 all-inline output and performance.

Semantic equivalence alone was insufficient. The all-inline design contract
also required:

- the same handler order and token expansion;
- the same opcode-to-case or opcode-to-label mapping;
- no callable handler symbol retained in the linked owner;
- the same owner extent as R2;
- identical observable workload output; and
- ordinary profiling-off Release throughput equivalent to R2 in a same-session
  comparison.

The final R3 all-inline owner extents are byte-for-byte the R2 extents. Excluding
Base64, R3 all-inline versus R2 all-inline is:

| Compiler | `rxtvm` | `rxbvm` |
| --- | ---: | ---: |
| Apple Clang 21 | -0.220% | -0.280% |
| Homebrew GCC 16.1 | -0.127% | +0.520% |

This is the required equivalent control. The first R3 formal attempt failed
this contract even though it was functionally correct.

## Measurement environment and interpretation

The retained runs used an Apple M5 with ten logical CPUs, a 128 KiB L1
instruction cache and a 6 MiB L2 cache, on AC power with low-power mode off and
no thermal warning. Products were ordinary profiling-off Release builds using
`-O3 -DNDEBUG`.

The compiler identities were:

- Apple Clang 21.0.0 targeting `arm64-apple-darwin25.5.0`; and
- Homebrew GCC 16.1.0 at `/opt/homebrew/opt/gcc/bin/gcc-16`.

On macOS, `/usr/bin/gcc` is Apple Clang and is not independent GCC evidence.
The GCC products used `CREXX_ENABLE_TLS=OFF` because the NETWORK backend uses
Clang blocks. This is a build limitation of the experiment, not a TLS policy
recommendation.

The final matrix covered Sieve, Permute, Bounce, Richards, Base64, Towers and
RexxCPS; both compilers; both engines; and R2 all-inline, rebuilt R3 all-inline
and R3 profile-30. Each of the 84 cells had two warmups and twelve recorded
serial rounds. All 1,176 executions passed their exact output checks and all
1,008 recorded samples were retained. Source, manifest and product hashes were
unchanged before and after measurement.

Base64 was retained, never discarded, but its known noise is why both all-seven
and without-Base64 aggregates are reported. Code size is evidence about
compiler shape, not proof of the active instruction-cache working set.

## Chronology of attempts and changes

### 1. R2 single-definition handler framework

The first production-shaped change replaced the monolithic list of instruction
bodies with one macro definition per handler, grouped into five files. The
selected callable ABI used a struct containing pointers to most mutable
`run()` locals. Inline expansion used the same body against direct owner
locals; outlined expansion used that pointer facade and returned an owner
continuation result.

This achieved the maintenance objective and passed the complete correctness
boundary. It also supplied all-inline and all-outline controls. Static shape
under Clang was:

| Shape | Engine | `run()` bytes | Product text bytes | Outlined symbols |
| --- | --- | ---: | ---: | ---: |
| Untouched | `rxtvm` | 535,556 | 831,532 | 0 |
| Untouched | `rxbvm` | 530,528 | 827,180 | 0 |
| R2 all-inline | `rxtvm` | 532,512 | 828,488 | 0 |
| R2 all-inline | `rxbvm` | 531,868 | 828,520 | 0 |
| R2 profile-30 | `rxtvm` | 200,160 | 888,228 | 475 |
| R2 profile-30 | `rxbvm` | 200,584 | 881,236 | 475 |
| R2 all-outline | `rxtvm` | 31,824 | 913,248 | 651 |
| R2 all-outline | `rxbvm` | 32,268 | 902,588 | 651 |

The frozen public heat panel put 176 of 588 non-reserved public handlers inline
and covered 99.9999969% of the measured public dynamic instruction count.
Nevertheless, profile-30 lost 9.35% on `rxtvm` and 12.08% on `rxbvm` versus
all-inline. All-outline lost 40.02% and 34.24% respectively.

The initial interpretation that six workloads executed no outlined handler was
later found incomplete. Profiling attributed private fused execution to the
first public opcode in its canonical sequence, hiding hot private handler calls.

Lesson: dynamic opcode coverage is necessary for placement, but is not a model
of C compiler behaviour. A frequency-perfect panel can still change the entire
hot owner's register allocation and dispatch layout.

### 2. Source and preprocessor sanity check

Before changing the ABI again, normalized all-inline, profile-30 and
all-outline expansions were compared. The ledger passed 368 checks:

- both engines contained the same 651 handlers in the same order;
- all 176 selected public inline bodies were token-identical;
- the non-handler owner skeleton was unchanged;
- the 650-entry threaded label mapping was unchanged; and
- opcode-to-label/case expansion was correct.

All governed profiles recorded zero actual interrupt selection, entry and
transition events. Compiler optimisation records showed no changed successful
helper-inlining decision across the selected handlers under either compiler.

This rejected two attractive but incorrect explanations: broken instruction
search/expansion and lost helper sub-inlining.

### 3. GCC baseline and compiler-output comparison

Real GCC builds showed immediately that reduced owner size was not inherently
adverse. Relative to all-inline, the original R2 profile-30 shape was +8.999%
on GCC `rxbvm` (+7.387% without Base64). GCC `rxtvm` was mixed: +1.238% with
Base64 but -2.239% without it, ranging from +18.834% on Sieve to -25.819% on
Bounce. GCC all-outline remained adverse at -5.225% for `rxbvm` and -23.539%
for `rxtvm` across all seven.

GCC's owners were much larger than Clang's:

| GCC shape at this stage | `rxtvm` bytes | `rxbvm` bytes |
| --- | ---: | ---: |
| All-inline | 1,493,900 | 1,478,368 |
| Profile-30 | 537,568 | 539,648 |
| All-outline | 104,512 | 110,304 |

The different speed direction despite much larger code proved that `run()`
extent and L1I capacity cannot be used as a standalone selection rule.

### 4. Address-escape and control-flow diagnostics under Clang

The diagnostic series held hot handler bodies constant and altered only the
reachable callable structure. The percentages below are elapsed-time changes
over all-inline for Sieve, Permute and Bounce; positive is slower in this table.

| Diagnostic shape | Observed range across engines/workloads | Finding |
| --- | ---: | --- |
| Keep all 651 unused wrappers, no reachable call | -1.6% to +0.6% | Wrapper definitions and cold call-graph population alone are neutral |
| Reach continuation funnel, but make no handler call | -1.8% to +1.2% | The continuation switch alone is neutral |
| One never-executed outlined public site | +9.6% to +35.9% | One reachable facade escape is enough |
| Eight never-executed outlined sites | +13.1% to +36.3% | Loss is not proportional to site count |
| Forty-nine reserved outlined sites | +15.1% to +47.4% | Coldness does not protect the owner from compile-time alias effects |
| Make threaded label map static | +13.3% to +37.9% | Label-map escape is secondary, not the shared root cause |
| Remove interrupt poll, semantic-invalid ceiling | +2.2% to +72.1% | Poll affects code shape, but removal damages threaded execution and is invalid |

Optimized IR confirmed the common cause. Once an outlined call was reachable,
the pointer facade retained 23 or 27 address-taking assignments in the GCC
owners and corresponding allocas/member pointers under Clang. Even a call that
never ran forced the compiler to respect possible mutation through those
pointers.

For Clang `rxtvm`, the same escape caused the 650-entry local label table to be
copied into the owner frame. The frame grew from approximately 2,192 bytes to
6,832 bytes and gained a Darwin stack-probe call. `rxbvm` has no label-address
table, so this amplified the threaded loss but did not explain the shared
slowdown.

### 5. Value snapshot and shared cold entry

The next form replaced pointers to owner locals with scalar values copied only
when control actually reached an outlined edge. Mutable fields were committed
back after the handler returned. This removed hot-path address escape.

A facade-only snapshot recovered most owner size but retained workload losses.
The decisive Clang change also removed outlined public identities from the hot
owner and mapped them to one shared cold entry. Only there did the interpreter:

1. recover the numeric dispatch identity;
2. select the callable function;
3. snapshot the scalar execution state;
4. call a `noinline,cold` trampoline; and
5. commit the returned state and continuation.

For direct threading, an outlined instruction cell contains a label address,
so the cold entry obtains the public opcode from the immutable canonical image.
For switch dispatch, the process-owned execution image already contains the
numeric opcode. Private outlined identities retain dedicated labels because
they have no canonical public opcode of their own.

This brought Sieve and Permute to roughly 2% of all-inline in the bounded Clang
pilot. A more complex family of narrowed per-handler ABIs was therefore not
justified.

### 6. Private-handler profiling correction

Bounce remained unexpectedly slow. Native samples showed execution in
`PRIVATE_R1_RELINK` and in the cold trampoline when private handlers were
outlined. The public semantic profiler had attributed that dispatch to
`UNLINK`, the first canonical opcode, and had therefore classified the actual
private callable identity incorrectly.

Both existing private fused handlers were made explicit inline members of the
profile panel. The resulting three-workload Clang pilot was within about 1.3%
of all-inline on both engines. This changed the denominator from 176/588 public
handlers to 178/590 non-reserved public-plus-private definitions, or 30.17%.

Lesson: semantic profiling and placement profiling are different views.
Semantic counts should retain canonical public identity, while placement
analysis must also expose the process-private dispatch identity that consumes
cycles.

### 7. Rejected first formal R3 candidate

The first formal candidate made two changes that looked reasonable in source:

- it used the Clang value-snapshot/shared-cold lowering under both compilers;
  and
- it deleted pointer-facade setup from all-inline because the optimized compiler
  had previously removed it anyway.

This verdict was rejected for two independent reasons.

First, the rebuilt all-inline control was no longer equivalent. Without
Base64, `rxtvm` lost 5.09% under Clang and 5.66% under GCC versus R2. The
source deletion changed compiler heuristics and the direct-threaded layout even
though the removed facade had generated no surviving handler calls.

Second, the universal shared-cold form produced opposite GCC outcomes versus
that rebuilt control: `rxtvm` lost 5.33% while `rxbvm` gained 13.67% without
Base64. A common source abstraction was not a common optimisation strategy.

The rejected run is retained because it demonstrates two essential review
rules: validate the baseline before interpreting a candidate, and never infer
cross-compiler equivalence from source-level neatness.

### 8. Restored all-inline source equivalence

The exact R2 all-inline pointer-facade source shape was restored behind a
compile-time choice. In all-inline builds it is allowed to optimize away just
as before. A three-hot-workload pilot returned all four rebuilt controls to
within about 1.3% of R2, and the final seven-workload run confirmed both exact
owner extents and performance equivalence.

This was not a request to preserve every historical line forever. It was proof
that, for this unusually large function, compiler heuristics are part of the
measured interface. Any future cleanup must establish a new equivalent control
before it can be called neutral.

### 9. Compiler-specific lowering

The final form selects the internal callable lowering by compiler and panel:

- all-inline always uses the exact R2 pointer-facade source shape;
- real GCC non-inline panels retain the R2 per-identity pointer-facade shape;
- Clang non-inline panels use the value snapshot and shared cold entry; and
- MSVC/other compiler panel policy remains unselected pending its own evidence.

The preprocessor test for real GCC must exclude Clang explicitly because Clang
defines `__GNUC__` for compatibility. `_MSC_VER` is also kept separate.

The two private fused handlers are inline in profile-30 for both compilers.
This design is implemented by `RXVM_HANDLER_USE_POINTER_FACADE`, not by a
public ABI or serialized instruction change.

## Why Clang and GCC differ

### Apple Clang 21

Clang aggressively centralizes the interpreter's normal direct-threaded
dispatch. The all-inline and profile owners contain only about six to eight
indirect branches in total, with normal computed-goto handlers converging on a
central dispatch branch. The classic benefit of one indirect branch site per
handler is therefore absent.

Clang also scalar-replaces the facade completely when no outlined call is
reachable. As soon as one such call exists, possible mutation through the
facade prevents the same scalar/register representation. In `rxtvm`, the label
array then escapes into a large stack object. Clang therefore benefits from a
late value snapshot and one unmistakably cold call edge.

Final Clang profile-30 owner extents are 205,548 bytes for `rxtvm` and 205,444
bytes for `rxbvm`, versus 532,512 and 531,868 bytes all-inline. The reduction
does not place the whole owner inside the 128 KiB L1I, and no claim is made that
whole-function extent equals resident hot footprint.

### Homebrew GCC 16.1

GCC preserves a distributed computed-goto shape: the comparison found about
1,520 indirect branches in all-inline `rxtvm` and 247 in profile-30. Its
`rxbvm` switch becomes a direct conditional decision tree rather than Clang's
central indirect switch dispatch. Reducing handler population can improve that
tree significantly.

GCC materializes the approximately 5,200-byte threaded label table in every
examined threaded shape, so the facade does not introduce the same binary
elimination-to-stack transition observed under Clang. Its register allocation,
branch prediction and outlining tradeoffs consequently differ.

For non-inline panels, GCC's per-identity pointer-facade form is faster than the
shared cold form for `rxtvm`. Final GCC profile-30 owner extents are 547,808
bytes for `rxtvm` and 549,632 bytes for `rxbvm`, compared with 1,493,900 and
1,478,368 bytes all-inline.

GCC emits 409 visible handler symbols in the final products versus Clang's 474,
despite the same policy. That is optimizer folding/elimination, not a different
handler selection.

### Shared findings

Both compilers showed that cold code can influence hot code at compile time.
Reachability, address-taking, alias sets, function-local labels and CFG
predecessors matter even when runtime profiles show zero traversals.

Both compilers also rejected helper-sub-inlining as the cause: GCC reported 478
normalized helper decisions across the selected public handlers with zero
all-inline/profile differences; Clang showed no changed successful decision.
The large owner remains difficult code, but the evidence does not support
replacing value helpers with macros merely to force more expansion.

## Final static and performance result

### Static shape

| Build | Engine | `run()` bytes | Text bytes | File bytes | Visible handler symbols |
| --- | --- | ---: | ---: | ---: | ---: |
| Clang R2/R3 all-inline | `rxtvm` | 532,512 | 828,488 | 1,020,632 | 0 |
| Clang R2/R3 all-inline | `rxbvm` | 531,868 | 828,520 | 1,020,808 | 0 |
| Clang R3 profile-30 | `rxtvm` | 205,548 | 896,184 | 1,109,592 | 474 |
| Clang R3 profile-30 | `rxbvm` | 205,444 | 892,572 | 1,109,528 | 474 |
| GCC R2/R3 all-inline | `rxtvm` | 1,493,900 | 1,864,152 | 2,138,184 | 0 |
| GCC R2/R3 all-inline | `rxbvm` | 1,478,368 | 1,848,632 | 2,121,672 | 0 |
| GCC R3 profile-30 | `rxtvm` | 547,808 | 1,434,616 | 1,779,960 | 409 |
| GCC R3 profile-30 | `rxbvm` | 549,632 | 1,429,112 | 1,763,432 | 409 |

Outlined functions reduce the owner but can increase total Clang text because
the callable bodies remain elsewhere. Owner bytes, total text, file size,
branch layout and runtime throughput must therefore remain separate measures.

### Profile-30 versus rebuilt all-inline

| Compiler | Engine | All seven | Without Base64 |
| --- | --- | ---: | ---: |
| Clang | `rxtvm` | -0.541% | -0.341% |
| Clang | `rxbvm` | +0.274% | +0.201% |
| GCC | `rxtvm` | +3.841% | +1.379% |
| GCC | `rxbvm` | +8.053% | +6.422% |

### Direct-threaded versus switch dispatch

For final profile-30, `rxtvm` is 0.294% faster than `rxbvm` under Clang without
Base64, effectively a tie. Under GCC it is 14.615% faster. In the all-inline
control the corresponding advantages are 0.839% and 20.315%.

These comparisons do not make GCC globally faster than Clang and do not select
a product engine. They compare the two engines within each compiler build.

## Contract for maximum observed C optimisation

“Maximum” here means the best source/compiler result demonstrated on this Apple
ARM64 host, not a universal compiler theorem. The following rules are the
current engineering contract.

### Preserve the hot owner's scalar form

- Do not take addresses of `run()` locals on a hot path merely to support a
  cold callable handler.
- Under Clang non-inline panels, construct scalar snapshot state only after
  entering the shared cold edge and commit it once after the call.
- Do not place a pointer-rich state object in the common owner lifetime unless
  a new same-session measurement proves it neutral.

### Keep hot and cold ownership explicit

- Emit only inline handler labels/cases in the Clang owner.
- Route outlined public handlers through one cold owner label and one
  `noinline,cold` trampoline.
- Keep direct-threaded labels function-local and preserve the stable
  noinline/noclone label owner.
- Recover an outlined threaded opcode from the canonical image; never overwrite
  canonical RXBIN to simplify dispatch.
- Give private execution-image identities explicit placement accounting.

### Preserve compiler-specific lowering

- Keep the R2 pointer-facade source shape for all-inline under every compiler.
- Keep the per-identity pointer-facade lowering for tested real GCC non-inline
  panels.
- Keep the value-snapshot/shared-cold lowering for tested Clang non-inline
  panels.
- Test `__clang__`, `__GNUC__` and `_MSC_VER` deliberately; do not treat
  `__GNUC__` alone as proof of GCC.
- Do not generalize either non-inline policy to MSVC, Intel, Linux or other
  architectures without equivalent evidence.

### Let helpers remain compiler-visible functions

- Retain the existing inline policy of value and manipulation helpers in both
  inline and callable instruction bodies.
- Do not convert those helpers wholesale to macros based on owner size. This
  investigation found no loss of successful helper inlining, while forced
  expansion can increase code duplication and remove compiler cost choices.
- Consider a helper macro or forced-inline attribute only after a specific hot
  call is observed in linked owner assembly and a bounded control proves a
  benefit under both relevant engines and compilers.

### Preserve semantic hot-path checks

- Keep per-instruction interrupt/signal polling. Removing it changes semantics
  and produced opposite, sometimes severe, effects between engines.
- Keep instrumentation, TRACE/source identity, breakpoint behavior, signal
  continuations and terminal cleanup balanced in inline and callable forms.
- Treat a never-taken branch as compile-time-active until assembly proves
  otherwise.

### Optimize from measured code, not source aesthetics

- Smaller `run()` is not automatically faster, and fitting a whole symbol into
  a nominal cache size is not a sufficient model of its active footprint.
- Inspect optimized IR/assembly, owner stack size, indirect-branch distribution,
  symbol retention, text size and runtime together.
- Preserve rejected variants and negative measurements; compiler-specific
  reversals are design evidence, not noise to average away.

## Required review procedure for future VM changes

Any further handler, dispatch-tail, helper or interrupt-path change should use
this order:

1. Freeze the exact prior all-inline binaries, source hashes and compiler
   identities.
2. Normalize preprocessing and prove handler order, selected-body tokens and
   opcode/label/case mappings before interpreting timing.
3. Build both `rxtvm` and `rxbvm` with Apple Clang and real GCC; label any TLS
   or platform difference.
4. Confirm the rebuilt all-inline owner extent and ordinary Release throughput
   against the prior control. Reject the candidate baseline if equivalence is
   not recovered.
5. Inspect address-taken locals, stack frame, label-table materialization,
   helper decisions, indirect branches and cold symbols.
6. Run focused dispatch, signal, breakpoint, late-load, worker, reentrancy and
   instrumentation correctness before performance timing.
7. Use bounded one-mechanism controls. Keep semantic-invalid ceilings clearly
   labelled and never select them as repairs.
8. Measure both engines separately and report each compiler separately. Do not
   average away a compiler/engine reversal.
9. Run the balanced profiling-off Release portfolio with exact output oracles,
   pre/post hashes and retained raw samples.
10. Treat product-default selection, public opcodes and cross-platform policy as
    later explicit decisions.

## Lessons learned

1. Source that optimizes away can still change the compiler's earlier
   heuristics and final layout.
2. A never-executed call edge can damage a hot function through address escape
   and aliasing.
3. The number of outlined handlers did not predict the loss; one site was
   already sufficient.
4. Dynamic opcode frequency does not describe whole-owner compiler effects.
5. Semantic profiler identity can hide private runtime work; placement needs a
   separate execution-identity view.
6. Clang and GCC do not merely produce faster and slower versions of one code
   shape; they construct materially different dispatch CFGs.
7. One portable C lowering need not be the fastest portable design. A small,
   explicit compiler selection can be safer than forcing a common adverse
   shape, provided every form preserves semantics and is tested.
8. Large-function code size is diagnostic, not a cache verdict. Total product
   text and hot-owner extent can move in opposite directions.
9. Interrupt polling influences compiler layout even when no interrupt occurs;
   correctness checks cannot be optimized away from timing evidence.
10. Helper macro conversion should be evidence-led. The suspected helper
    sub-inlining failure was not present.
11. A candidate cannot be evaluated against a broken rebuilt control. Baseline
    equivalence is a first-class performance gate.
12. Rejected formal runs are valuable: the first R3 verdict found both the
    all-inline source-shape defect and the GCC policy reversal.

## Validation and remaining boundaries

The retained final Clang and GCC profile-30 trees each pass 14/14 focused tests
and 2,002/2,002 full profiling-off Release tests. Fresh all-outline trees pass
14/14 focused tests under each compiler. The final timing matrix and all
diagnostic/pilot executions passed exact output checks.

The remaining boundaries are deliberate:

- Apple ARM64 is the only performance-selection host in this report.
- GCC used the no-TLS build described above.
- MSVC supports only `rxbvm` and has no selected non-inline policy here.
- No product-default panel has been selected.
- The private fused handlers remain immutable load-time quickening with guarded
  canonical fallback. A shared RXAS/RXVM fusion registry and any decision to
  expose normal serialized instructions are tracked separately as
  `PERF3-05-R4`.

## Evidence and related documents

- [R2 handler-panel worklist](../../../performance/PERF3-05-R2-WORKLIST.md)
- [R3 code-generation worklist](../../../performance/PERF3-05-R3-WORKLIST.md)
- [Checksum-closed R3 evidence](../../../performance/evidence/2026-08-09-perf3-05-r3-handler-codegen-analysis/README.md)
- [Durable interpreter architecture](../../ai-context/RXVM_INTERPRETER.md)
- [Earlier VM dispatch investigation](../beta-3/notes/vm-dispatch-performance-investigation.md)
- [Performance governance](../../../performance/PERFORMANCE-GOVERNANCE.md)

The R3 retained implementation is commit
`adf96256d709bd6fe61cb6638a055fff2caa89d9` on
`codex/perf3-05-r3-handler-codegen-analysis`. It is local and unpushed at the
time of this report.

## 2026-08-10 R5 percentage and never-inline addendum

R5 tested whether the repaired framework has a stable size/speed compromise,
rather than assuming 30% is correct. It also separated the literal all-inline
equivalence control from the practical maximum requested for production use.

### Policy representation and corrected denominator

The repeated per-panel policy blocks are replaced by one central tier per
handler. Tiers record the first frozen heat panel at which the handler is
eligible, `MAXIMUM`, `NEVER`, or `RESERVED`; the run-owned `INTERRUPT` pseudo-op
has its own always-inline `OWNER` tier. Panel mappings alone convert these tiers
to direct owner bodies or callable handlers. This keeps semantic handler files
unchanged and makes counts mechanically auditable.

The 56-entry `NEVER` class covers sockets, console operations, clocks and
environment access, spawn/redirection, file I/O and `METALOADMODULE`. These are
host-bound operations whose external cost normally dominates the VM call and
whose bodies bring large cold interfaces into `run()`. Literal `all-inline`
still expands them for exact equivalence; every practical profile honors the
attribute.

The audit corrected an old reporting shorthand. The 588 non-reserved public
opcode slots include `INTERRUPT`, but `INTERRUPT` is an owner target rather
than one of the 649 public/sentinel handler definitions. The placement
denominator is therefore 589 non-reserved public-plus-private definitions, not
590. The profile totals are 31, 61, 90, 120 and 175, and `max-eligible` is
531/589 (90.15%). Normalized R5 `all-inline` preprocessing is byte-identical to
the R3 starting commit for both engines.

### Percentage screen

The Apple Clang and real GCC screens each ran all-inline, 5%, 10%, 15%, 20%,
30%, max-eligible and all-outline under both concrete engines and seven
governed workloads. Each screen passed 560/560 exact-output executions. The
all-seven geometric-mean throughput changes versus literal all-inline were:

| panel | Clang `rxtvm` | Clang `rxbvm` | GCC `rxtvm` | GCC `rxbvm` |
|---|---:|---:|---:|---:|
| 5% | -24.087% | -16.915% | -11.058% | +7.984% |
| 10% | -3.275% | -3.681% | -3.117% | +9.694% |
| 15% | +1.850% | +3.292% | +2.071% | +11.301% |
| 20% | +3.438% | +5.682% | +2.624% | +11.189% |
| 30% | -0.204% | +1.722% | +3.472% | +9.923% |
| max eligible | -0.724% | +0.069% | +1.306% | +12.743% |
| all outline | -64.195% | -60.217% | -29.211% | -6.526% |

There is no monotonic relationship between inline percentage and throughput.
Clang has a clear optimum around 20%; GCC switch dispatch benefits throughout,
while GCC threaded dispatch retains a workload-specific Bounce sensitivity.

### Formal verdict and decision stop

The formal Clang contender matrix ran all-inline, 15%, 20% and 30% with two
warmups and twelve recorded balanced rounds. All 784 executions and 672
recorded samples passed. Twenty percent is guard-clean:

| compiler | engine | all seven | common five | worst cell |
|---|---|---:|---:|---:|
| Clang | `rxtvm` | +3.857% | +5.475% | Towers -0.915% |
| Clang | `rxbvm` | +3.152% | +4.697% | Richards -1.714% |

Fifteen percent retains Base64 and RexxCPS guard failures. Thirty percent
fires the Clang `rxtvm` common-five guard and is slower than 20%.

The decisive formal GCC all-inline/20% matrix passed all 392 executions and
336 recorded samples. It confirms the compiler/engine split:

| compiler | engine | all seven | common five | worst cell |
|---|---|---:|---:|---:|
| GCC | `rxtvm` | +3.175% | +3.974% | Bounce **-10.072%** |
| GCC | `rxbvm` | +9.646% | +12.464% | Towers -0.428% |

RexxCPS is a higher-is-better benchmark rate; the retained derivation handles
that direction separately from elapsed time. GCC `rxbvm` RexxCPS improves
6.352% at 20%. The only formal GCC 20% guard is threaded Bounce, but it is
large. Every requested GCC non-inline panel fires the same Bounce guard in the
screen; `max-eligible` reduces it only to -4.693% and adds a -5.604% Base64
loss.

No common percentage is therefore acceptable under the standing guards. The
default remains all-inline and Linux x86-64, Windows Intel and Linux sanitizer
selection work has not started. The explicit choices are:

1. retain common all-inline and its build/code-size cost;
2. accept common 20% with the measured GCC threaded Bounce regression;
3. approve compiler/engine-specific policy (Clang both engines and GCC switch
   dispatch can use 20%, while GCC threaded remains all-inline); or
4. open a focused GCC threaded repair before default/platform selection.

Option 4 is the evidence-led recommendation. It preserves the strong Clang and
GCC switch result without accepting a 10% governed regression or embedding a
premature default split.

### Size and compiler effort

Clang 20% reduces `run()` by about 72.4% to 146,824/145,608 bytes and the first
two-VM target build by 81.5%, from 40.02 s to 7.42 s. GCC reduces the owner by
about 70% to 438,816/442,304 bytes and the diagnostic target build by 87.2%,
from 310.30 s to 39.84 s. This is the material compiler-effort/build-cost gain
the refactor sought.

The build-cost reduction is measured; improved response to arbitrary future
source changes is still an inference. R5 retained one clean first target build
per shape, not a repeated perturbation series. A controlled sequence of small
hot, cold and unrelated edits is still needed to quantify build-time variance
and binary-layout stability. The much smaller owner reduces the compiler work
and heuristic surface exposed to those edits, but this report does not promote
that expectation to a measured predictability claim.

The practical maximum is not a useful default ceiling. Its 56 never-inline
handlers reduce Clang build time only 10.7% and leave a 510-515 KiB owner; GCC
build time falls 24.9% but remains 233.04 s with a roughly 1.386-1.388 MiB
owner. It supplies a legitimate control and proves the never-inline attribute
has headroom, but 20% is the meaningful code-size/build-cost region.

Total product text must remain separate from owner size. Clang's callable
wrappers make total text about 8% larger at 20% even while `run()` shrinks;
GCC total text shrinks. Both complete 20% compiler trees pass the 14/14 focused
dispatch, signal, interrupt, breakpoint, worker, reentrancy and late-load
suite. Full broad and cross-platform validation correctly remains after the
default decision.

R5 evidence is retained in
[`2026-08-10-perf3-05-r5-handler-percentage-panel`](../../../performance/evidence/2026-08-10-perf3-05-r5-handler-percentage-panel/).

## 2026-08-10 R5a effective-placement and Bounce addendum

R5a added effective handler placement to the existing VM instruction profile.
The human instruction table now reports `inline`, `outline` or `mixed`; CSV
schema 5 uses its existing `value` column. Placement is sampled at actual
handler entry while counts and timings retain canonical public-opcode
attribution. This closes the earlier private-fusion blind spot: if a canonical
opcode is observed through handlers with different placement, its row becomes
`mixed` rather than receiving a possibly false static label.

The ordinary profiling-off contract remains exact. The added hook argument is
discarded without evaluation by the no-backend macro, and normalized
profile-20 `rxvmintp.c` preprocessing remains byte-identical before/after for
both engines.

Exact counts-only GCC profile-20 Bounce runs produced identical `rxtvm` and
`rxbvm` instruction counts:

| placement | instructions | dynamic share |
|---|---:|---:|
| inline | 887,443,222 | 99.952222146% |
| outline | 424,204 | 0.047777854% |

`CALL1_REG_FUNC_REG` accounts for 424,200 outlined executions. The four other
executions are one each of `SCONCAT_REG_REG_STRING`, `STOI_REG`, `SAY_REG` and
`SAY_STRING`. No row is mixed. `CALL1`, `SCONCAT` and `STOI` all enter at the
30% tier, so replaying the frozen policy over these exact counts leaves only
the two one-off never-inline `SAY` executions callable at both profile-30 and
max-eligible: 0.000000225% of all instructions.

This disproves outlined dynamic frequency as the main GCC threaded cause.
Despite identical effective placement for virtually every executed Bounce
instruction, retained profiling-off `rxtvm` results are -8.691% at profile-30
and -4.693% at max-eligible versus all-inline. Inlining handlers that Bounce
does not execute changes its speed materially. The residual defect is
therefore GCC owner layout/code shape—branch reach, label placement, hot/cold
partitioning, register allocation, or a related whole-function heuristic—not
the direct cost of omitted hot calls.

This also explains why simply promoting `CALL1` would be an incomplete and
potentially misleading repair. The hot panel still needs a pre-release
portfolio refresh, but frequency selection alone cannot determine the fastest
threaded owner. The next GCC investigation should compare profile-30,
max-eligible and all-inline assembly/layout because they execute the same
placement mix while producing materially different Bounce timing. No tier or
default changes are made by R5a.

R5a evidence is retained in
[`2026-08-10-perf3-05-r5a-handler-placement-profiling`](../../../performance/evidence/2026-08-10-perf3-05-r5a-handler-placement-profiling/).
