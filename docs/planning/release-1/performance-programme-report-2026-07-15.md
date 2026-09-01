# CREXX performance programme: evidence, issues and response

Date: 2026-07-15

Status: proposed Release 1 programme and terms of reference

## Executive decision

The new RexxCPS result exposes a real and material performance problem. On the
same Apple M5 host, the current cREXX Level B port averages about 0.893 million
nominal RexxCPS clauses per second while ooRexx 5.2.0 averages about 40.8
million. The approximately 45.7-fold headline gap is not explained by a
RexxCPS version change, a simple numerator error, the choice between the two
cREXX virtual-machine (VM) dispatch variants, `rxvm` and `rxbvm`, or one
removable interpreter instruction.

RexxCPS parity with ooRexx is **not** a Release 1 ship/no-ship gate. RexxCPS is
one historically important, deliberately synthetic Classic Rexx workload; it
cannot represent all of CREXX's intended uses or benefits. It is nevertheless
a serious diagnostic and a community-credibility issue. CREXX should report it
honestly, understand it, improve it materially, and avoid allowing a broad
benchmark portfolio to hide an unexplained pathological result.

The quoted Apple M1 ooRexx score is also about 4.52 times the NetRexx 2.1n
score. That is unexpectedly strong for ooRexx relative to a Java Virtual
Machine (JVM)-hosted compiled language and makes ooRexx execution/optimizer
forensics an immediate (P0) requirement. A validated NetRexx-equivalent
performance band is a more plausible long-term
CREXX objective than immediate ooRexx parity, especially if CREXX can provide
similar throughput with a smaller native deployment and no mandatory JVM. The
deployment advantage must be measured, not merely asserted.

The recommended response is therefore a performance programme, not a
benchmark-specific patch:

1. establish a correctness-gated, cross-language benchmark portfolio;
2. use the existing profiler and dynamic instruction-sequence analyser,
   `rxseq`, to remove general overhead;
3. attack the largest semantic lowering costs, especially general `PARSE`,
   stems, TRACE and ADDRESS paths;
4. improve compiler and cREXX assembler (RXAS) flow analysis, call placement
   and instruction selection;
5. prototype compact execution, quickening and mapped calls without changing
   the canonical portable cREXX bytecode format (RXBIN) prematurely;
6. make the Release 1 date and scope conditional on an architecture selection
   gate rather than the current fixed 2026-08-31 plan.

There is useful P0 (immediate) and P1 (next-phase) work that is no-regrets
across almost every plausible VM direction. In particular, compiler-side
call-window placement can remove avoidable `SWAP`s without changing the call
application binary interface (ABI); compare-to-branch lowering,
dead initialization/lifetime elimination, repeatable profile capture and
portfolio benchmarking remain valuable whether the eventual runtime uses
switch dispatch, computed-goto direct dispatch, a compact runtime stream,
runtime instruction specialization (quickening), just-in-time compilation
(JIT) or ahead-of-time compilation (AOT).

## 1. Scope and terminology

This report consolidates the current RexxCPS review and the proposed strategic
tracks for the `rxc` compiler, `rxas` assembler, `rxlink` linker, RXBIN bytecode
and `rxvm` runtime. It is a programme charter, not approval for an instruction
set architecture (ISA), ABI, language or concurrency change. The ISA is the
set and encoding of operations understood by the VM; the ABI is the contract
for calls, registers, values and interoperability. Those changes retain their
normal design and cross-platform approval gates.

Priority labels in this report mean:

- **P0**: start immediately because it is required evidence, removes a known
  general inefficiency with bounded semantic risk, or unblocks several later
  decisions;
- **P1**: execute in the first performance-foundations phase after its P0
  prerequisites; this includes reversible architecture-neutral prototypes;
- **P2**: research or later implementation whose value depends on earlier
  measurements or an architecture decision.

“No-regrets” does not mean “guaranteed speedup.” It means that the deliverable
is useful under all credible architecture outcomes, is independently
reversible, and does not force a premature serialized ISA, public ABI or
language commitment.

## 2. Release and measurement snapshot

### 2.1 Release status

The measured source tree identifies itself as
`crexx-1.0.0-beta.3+local.gca275cdea9f3.dirty 20260714` and its Release build
uses `-O3 -DNDEBUG`. It is a local dirty beta 3 work-in-progress (WIP) build,
not a released beta.
The latest completed release tag is `v1.0.0-beta.2`; `v1.0.0-beta.3` does not
yet exist. Performance results from this tree must therefore not be labelled
as beta 3 release results.

The existing Release 1 plan targets the end of August 2026. This programme
changes that scope materially. The date should no longer be treated as fixed
until the architecture selection gate in section 10 has completed.

### 2.2 Current RexxCPS comparison

RexxCPS reports nominal Rexx clauses per second (CPS). The formal comparison
under review is:

| Implementation | Workload | Same-host result |
| --- | --- | ---: |
| cREXX Release `rxvme` | RexxCPS 2.2c Level B port | 887,419; 900,841; 892,064 CPS |
| cREXX Release `rxvme` | Mean of the three runs | **893,441 CPS** |
| cREXX Release `rxbvm` | Same generated workload | about **895,948 CPS** |
| ooRexx 5.2.0 | Upstream RexxCPS 2.2 | about **40.8 million CPS** |

The ooRexx result is approximately 45.7 times the cREXX mean. The `rxbvm`
result is effectively the same as `rxvme`, so dispatch selection by itself does
not explain the gap.

These values were collected on the same Apple M5 machine. Rene's quoted Apple
M1 results are useful external context but are not used to calculate this
ratio. They were the trigger for the investigation:

| Rene's Apple M1 sample | Reported result | Status in this report |
| --- | ---: | --- |
| earlier cREXX Level B adaptation, `count=100`, `averaging=10` | 411,879 estimated CPS | historical trigger; predates the audited 2.2c port/output |
| ooRexx 5.2.0, RexxCPS 2.2 | 24,550,963 CPS | external same-M1 context |
| NetRexx 5.10, RexxCPS 2.1n | 5,428,558 CPS | external context; equivalence and optimizer-resistance review still required |

Those results reinforce the need for investigation but must not be blended
into a formal ratio: the cREXX workload was the earlier adaptation, the
NetRexx workload identifies itself as 2.1n, and the sampling protocols differ.
A current Regina result and same-host NetRexx/Java portfolio results have not
yet been collected.

The M1 ooRexx result is about 4.52 times the quoted NetRexx result. That is a
surprising result because NetRexx normally compiles through Java source to Java
bytecode executed by a JVM, which may in turn JIT-compile
hot bytecode to native machine code. It is not evidence that “ooRexx is faster
than C” or that an interpreter is inherently faster than compiled Java. The
numbers compare two complete language implementations, two different RexxCPS
ports and their runtime services—not C instructions with Java instructions.
They make both the NetRexx port and the exceptionally strong ooRexx path
high-priority forensic subjects.

### 2.3 RexxCPS version and arithmetic review

The cREXX source is explicitly a **RexxCPS 2.2c Level B port**. It is not an
unchanged copy of 2.2 and must not be described as one. The port preserves the
upstream timed control-flow shape and the historical numerator of exactly
1,000 nominal source clauses per outer iteration. The necessary typed
substitutions and their dynamic counts are audited in
[`tests/benchmarks/README.md`](../../../tests/benchmarks/README.md).

The score calculation is the same as upstream 2.2:

```text
empty     = average time for the empty count loop
innertime = full_total / averaging - empty
thousand  = innertime / count
CPS       = 1000 / thousand
```

`count` and `averaging` therefore are not multiplied into the numerator a
second time. Empty-loop subtraction, averaging and per-iteration division are
in the correct order. The port reports 1,000 because the numerator is the
historical RexxCPS source-clause unit, not the number of cREXX VM instructions.

RexxCPS 2.1 and 2.2 were also run ten times each on the same ooRexx
installation:

| ooRexx workload | Ten-run mean |
| --- | ---: |
| RexxCPS 2.1 | 39,561,716 CPS |
| RexxCPS 2.2 | 39,599,811 CPS |
| 2.2 relative to 2.1 | +0.096% |

That difference is measurement noise. The timed kernels and subroutine are
identical after whitespace for this purpose; 2.2 mainly adds modern
self-calibration. A 2.1-versus-2.2 version effect cannot explain the cREXX
gap.

An ooRexx source trace emitted 985 trace records between the kernel markers,
not 1,000. This does not create the headline ratio: upstream RexxCPS has always
used the nominal 1,000-clause unit, and both quoted scores use that same unit.
It is still worth retaining the trace count as provenance so that “nominal
clauses” is not mistaken for a physical trace-record count.

### 2.4 Equivalence conclusion

The timed-kernel equivalence table is evidence, not a judgement-free proof.
The dynamic source-clause counts and intended value effects can be checked
objectively. Whether an explicit typed substitution represents sufficiently
equivalent implementation work remains a disclosed comparison judgement.

The current conclusion is:

- the port is recognizably and structurally a real RexxCPS port;
- the 1,000-clause numerator and score arithmetic are valid within RexxCPS's
  historical convention;
- typed conversions, typed four-argument construction, explicit stem objects,
  and Level B TRACE/ADDRESS surfaces mean lower-level work is not identical;
- the result is suitable for a transparent community comparison labelled
  **RexxCPS 2.2c for cREXX**;
- it is not sufficient by itself to rank whole implementations or to set a
  Release 1 parity gate.

### 2.5 Optimizer-resistance challenge tests

It is not yet known whether ooRexx removes or specializes material parts of the
timed kernel. The 985-record source trace is evidence that the clause stream is
not wholesale deleted in the traced run; it does not prove that the untraced
execution path is identical. Stable ooRexx 2.1 and 2.2 scores also make an
obvious version-specific accident unlikely. Neither observation rules out
partial constant folding, dead failure-branch removal, cached PARSE templates,
direct compound-variable specialization, loop specialization or other work
reduction.

This requires a P0 optimizer-resistance challenge suite. It must **not** replace
or silently modify canonical RexxCPS 2.2/2.2c, because that would destroy the
community comparison. It should add separately named diagnostic variants:

1. **Opaque-input variant.** Read selected values from command-line arguments
   or another runtime boundary before timing, choosing values that preserve the
   original branch paths and dynamic source-clause counts. This prevents the
   translator from treating all important operands as source constants.
2. **Result-observation variant.** Validate a deterministic digest or selected
   final stem/scalar state outside the timed region. Where observing every
   outer iteration is necessary, report the added checksum cost separately
   rather than folding it into the canonical score.
3. **Perturbation family.** Change opaque values across runs while preserving
   the intended taken/not-taken paths. A score that changes unexpectedly with
   semantically equivalent opaque inputs is evidence for deeper inspection.
4. **Trace-count variant.** Retain the source-clause census as structural
   evidence, while recognizing that TRACE materially perturbs execution and
   may itself change optimization behavior.
5. **Runtime-instrumented variant.** Build or instrument ooRexx to count its
   internal instruction and operation handlers. This is stronger evidence than
   source changes because it can test the canonical source without adding
   timed clauses.
6. **Native-C ceiling controls.** Implement both a mechanical C control-flow
   kernel and, separately, a more faithful dynamic-value kernel. Prevent the C
   compiler from deleting results. These are labelled ceilings/controls, not
   RexxCPS scores, and answer whether the ooRexx rate is physically surprising
   without pretending that C and Rexx do identical work.

For ooRexx, also inspect the translated/internal instruction representation and
any optimizer passes, and compare source execution with any supported
precompiled form. For NetRexx, retain generated Java source, disassemble the
class file, record JVM/JIT settings and warmup state, and prove that observable
results depend on the timed loop. The output is an elimination/specialization
ledger: each candidate transformation is either observed, ruled out, or left
open with a concrete next test.

The motivating optimizer question is ooRexx's unexpectedly high result,
especially relative to NetRexx. NetRexx dead-code and warmup checks remain
necessary benchmark hygiene, but they are not being proposed as the leading
explanation for ooRexx's score.

Only the unchanged canonical workload produces the quotable community score.
The challenge variants answer whether that score represents the intended work
and why implementations differ.

### 2.6 TRACE-off and metadata controls

The `TRACE` clause is not instrumentation used to count this benchmark.
Historical trace output was used to derive the workload mix;
`trace value trace()` is itself one clause in that mix and executes 14 times
per nominal 1,000-clause outer iteration. The cREXX port's
`trace value _trace_current_mode()` preserves that source-clause position.

Three runtime states must not be conflated:

1. With TRACE active, `BPON` raises the breakpoint interrupt bit. The VM then
   takes its interrupt path after every instruction and the handler consumes
   source/TRACE metadata. This is intentionally and substantially slower.
2. Canonical RexxCPS sets TRACE to `OFF`. Its TRACE clauses still call the Level
   B TRACE helpers and execute `BPOFF`, but no per-instruction breakpoint
   handler runs. The dispatch loop retains the generic `if (interrupts)` poll
   used by all VM signals, not TRACE alone.
3. A separately named no-TRACE diagnostic removes all executable TRACE clauses.
   It contains 986 remaining timed clauses, so its fair work-rate numerator is
   986, not 1,000. It is not a quotable RexxCPS score.

On the same Release `-O3 -DNDEBUG` build and Apple M5 host, five interleaved
`rxvme` pairs at `count=200`, `averaging=10`, after one warmup per image,
produced:

| Direct modular execution, source/TRACE metadata retained | Five-run mean |
| --- | ---: |
| Canonical RexxCPS 2.2c | 798,475 nominal CPS |
| No-TRACE diagnostic, 986-clause-normalized rate | 923,529 clauses/s |
| No-TRACE diagnostic, 1,000-unit time equivalent | 936,642 outer units/s |
| Paired outer-iteration time improvement | **17.33%** |

The absolute values drifted below the formal baseline during the extended test
session, so this table is a paired diagnostic, not a replacement baseline. The
same comparison after `rxlink -s` had a 4.66% paired time improvement. That is
the better estimate of executable TRACE-clause/helper cost when source/TRACE
metadata is already absent. Either result is far too small to explain the
ooRexx gap.

The larger retained-metadata effect initially appeared to imply that TRACE
events were being checked while TRACE was off. They are not executable
instructions, and disassembly confirmed the same instruction sequence after
normalizing rewritten pool/jump-table addresses. The actual issue is an
accidental metadata coupling:

- source/TRACE metadata, runtime function/type/class/interface metadata and
  other records share one linked list;
- runtime contract/type fallbacks traverse that list and skip irrelevant
  record kinds;
- keeping 930 source/TRACE records slowed an otherwise identical no-TRACE
  linked image by 8.61%; keeping 2,414 slowed the canonical image by 21.22%;
- retaining 107 large inline-metadata records made the constant pool larger
  than the unstripped no-TRACE pool but changed paired time by only -0.60%, so
  constant-pool byte size is not the explanation;
- removing the timed ADDRESS path reduced the 930-record strip advantage to
  0.76%; caching only `environment_name()` before timing reduced it to 4.37%,
  locating most of the sensitivity in the generic ADDRESS/object path;
- a 10-outer instruction profile preserved dynamic counts but measured
  `ASSERTTYPE_REG_STRING` at 988 ns average stripped versus 4,910 ns retained,
  and `ISTYPE_REG_REG_STRING` at 2,452 ns versus 6,250 ns. `SRCMETHODSEL`
  changed only from 64 ns to 74 ns.

The production response is therefore not to disable TRACE or require stripped
artifacts. Runtime contract and type lookup needs a kind-specific index or
separate chain so source/TRACE records are never visited by ordinary type and
ADDRESS operations. Stripped and unstripped lifecycle results must remain
separately labelled because stripping removes debugging capability as well as
this accidental scan cost. A compile-time VM with the generic interrupt poll
removed would be a different upper-bound experiment: it disables all signal
handling, not merely inactive TRACE, and has not been measured here.

## 3. What the cREXX profiles show

### 3.1 Dynamic instruction expansion

The optimized benchmark executes approximately 193,690.58 VM instructions for
one nominal 1,000-clause outer iteration, or 193.69 VM instructions per nominal
RexxCPS clause. Combining that count with an unprofiled elapsed run gives about
173 million cREXX VM instructions per second.

If the VM instruction rate stayed constant, matching the observed ooRexx score
would require only about 4.24 cREXX VM instructions per nominal clause. This is
not a literal implementation target—ooRexx does not execute cREXX opcodes—but
it makes the central problem clear: the present gap is predominantly semantic
expansion and library/call work, not a small dispatch-rate deficit.

### 3.2 Procedure-level concentration

After subtracting profiler startup effects, the approximate steady-state self
shares are:

| Procedure or group | Calls per outer iteration | Approximate self share |
| --- | ---: | ---: |
| `parseExec` | 98 | 60.04% |
| `stem.get` | 125 | 10.77% |
| `stem.set` | 83 | 6.56% |
| benchmark `main` | — | 4.82% |
| `strip` | — | 4.56% |
| ADDRESS stack combined | — | about 11.1% |
| TRACE helpers | — | about 1.26% |
| `upper` | — | about 0.18% |

These are instrumented self-time shares and should be used to prioritize work,
not multiplied directly into an unprofiled CPS score. They show that the
generic Level B library implementation of Classic semantics is the dominant
cost.

Section 2.6 also shows why TRACE helper self time is not the whole TRACE-related
cost. Retained source/TRACE records lengthen runtime type/contract scans, so
that time is attributed mainly to native type instructions and the ADDRESS
path rather than to TRACE procedures.

The compiler's generic `PARSE` exit builds a frozen plan and calls
`parseExec(source, plan, template, debug)`. This is flexible and semantically
centralized, but it turns simple static templates into a large interpreted
library path. A direct lowering or specialized frozen-plan execution path is
the single most important semantic experiment.

### 3.3 Instruction mix

Important counts per outer iteration include:

| Instruction family | Approximate dynamic count | Share of all VM instructions |
| --- | ---: | ---: |
| integer comparisons | 31,917 | 16.48% |
| Boolean `BRT`/`BRF` | 36,818 | 19.01% |
| `NULL` plus `ENDLIFE` | 27,015 | 13.95% |
| numeric-context setters | 5,246 | 2.71% |
| `SWAP` | 1,706 | 0.88% |

The Boolean and comparison figures overlap conceptually as producer/consumer
pairs. They identify compare-to-branch lowering as a large, general
opportunity. An opcode is the VM's encoded operation number. RXAS already
contains some integer compare/branch opcode rules, but a 20-instruction
keyhole—sliding-window—queue and local liveness model harvest only a small part
of the dynamic opportunity. The benchmark still exposes about 31,252
eligible-looking integer compare/branch pairs per outer iteration while only
about 869 are currently fused.

`NULL` is a real clear operation, not an empty VM opcode. It initializes a
general value and may release owned payload. `ENDLIFE` releases reference
identity/lifetime state at block exit. In this benchmark no reference opcode is
executed, so many dynamic `ENDLIFE`s are effectively dispatch-only; many
initial `NULL`s may also be removable after definite-assignment proof. The
optimization must be based on value/reference effects, not the benchmark name.

#### Instruction-latency audit

Dynamic counts alone are insufficient. The original retained-metadata,
100-outer-iteration instruction profile already contained the following
latency signals:

| Instruction | Dynamic count | Average instrumented time | Instrumented instruction-time share | Interpretation |
| --- | ---: | ---: | ---: | --- |
| `SRCFPROCSEL_REG_STRING_REG` | 7 | 100,964,351 ns | 72.18% | re-enters the VM to perform a runtime factory match; the outer timer includes child execution and child profiler overhead, so this is not handler self time |
| `COPY_REG_REG` | 107,766 | 356 ns | 3.92% | credible high-volume candidate; copying can include owned payloads and attributes, so value shape and static call site must be separated |
| `ASSERTTYPE_REG_STRING` | 2,807 | 8,762 ns | 2.51% | genuine anomaly; the TRACE metadata control subsequently confirmed a linear runtime-metadata scan |
| `FTOS_REG_REG` | 5,604 | 180 ns | 0.103% | smaller conversion candidate |
| `FTOD_REG_REG` | 2,802 | 171 ns | 0.049% | smaller conversion candidate |
| `SRCMETHODSEL_REG_REG_STRING` | 2,814 | 20 ns | 0.006% | not a performance blowout in this run |
| `BPOFF` | 2,805 | 14 ns | 0.004% | not a performance blowout in this run |

The process should therefore have found the type-check problem before the
TRACE-off experiment. In the cleaner no-TRACE paired control, identical
dynamic counts gave `ASSERTTYPE_REG_STRING` averages of about 4,910 ns with
source/TRACE metadata retained and 988 ns when it was stripped: a 4.97-times
latency ratio. That paired result proves the metadata relationship much more
strongly than the original average alone.

Large average time is a triage signal, not by itself a finding. Rare native or
operating-system operations may be legitimately slow; a re-entrant handler's
timer may include all child execution; and a frequent moderately expensive
instruction can matter more than a spectacular one-off. Every instruction
profile review must therefore include:

1. rankings by total time, average time (with a minimum-count threshold) and
   dynamic count;
2. paired before/after deltas with identical instruction counts where a causal
   control is available;
3. per-static-site and value-shape breakdown for polymorphic operations such as
   `COPY`;
4. separate handler self time and child/native time for re-entrant operations;
5. median and high-percentile latency where the sample count permits; and
6. confirmation with unprofiled paired wall-clock runs before forecasting a
   benchmark gain.

This audit promotes `ASSERTTYPE`/metadata lookup to a confirmed P0 issue,
`COPY` to a measured investigation candidate, and runtime factory selection to
an attribution-and-architecture investigation. It does not identify
`SRCMETHODSEL` or `BPOFF` as current optimization targets.

### 3.4 Call path

The 100-outer-iteration profile contains approximately:

| Event | Per outer iteration |
| --- | ---: |
| calls (`CALL` plus `DCALL`) | 1,049.29 |
| returns | 1,049.30 |
| `SWAP` | 1,706.26 |
| general `COPY` | 1,077.66 |

That is about 1.63 `SWAP`s per call. It justifies a call fast-path track, but
the raw `SWAP` count is less than 1% of all VM instructions and cannot by
itself explain 45.7 times.

The current ABI uses a contiguous call window. The compiler evaluates
arguments, swaps them into the window, calls, then emits reverse swaps. The VM
records the window start and argument count so a cold signal unwind can restore
the caller's pointer permutation. The compiler already reuses incoming
registers for provably read-only by-value formals, and RXAS already removes
some cancelling swaps locally.

The first call optimization should preserve that contract:

1. place argument results directly into their final call-window registers when
   liveness permits;
2. schedule the remaining parallel moves/swaps once, including cycles and
   repeated-source arguments;
3. coalesce the return register with its final consumer where safe;
4. retain the existing signal-unwind metadata and test all alias/reference
   cases.

A precomputed arbitrary register-to-argument mapping may later avoid more
movement, but it changes the runtime contract and cold unwind logic. It is a
P1 measured prototype, not the first production step.

### 3.5 Dynamic sequence evidence

An RXSEQ profile is the compact binary dynamic-sequence record consumed by the
`rxseq` analyser. Capture over 1,000 iterations found these leading repeated
windows:

- `NULL, NULL`: 11,086.707 occurrences per outer iteration, across 200 sites
  and seven modules;
- `ENDLIFE, ENDLIFE`: 10,497.589 per outer, across 265 sites and five modules;
- 19 integer compare/branch patterns: 31,251.924 per outer in total;
- repeated five-instruction numeric-context setup at about 1,048 procedure
  entries per outer;
- recurring PARSE, stem and call-marshalling patterns.

`rxseq` identifies candidates, not safe fusions. Control flow, liveness,
aliasing, reference lifetime, signals, source-step behavior and debug/trace
coordinates must still be proved.

### 3.6 Bounded optimizer ceiling

A deliberately optimistic count-only model gives:

| Local change set | Theoretical speedup if retired instructions cost equally | Approximate CPS from current mean |
| --- | ---: | ---: |
| batch lifecycle sequences, fuse all eligible-looking integer compares/branches, fuse numeric setup | 1.417x | 1.266 million |
| additionally remove all apparently useless lifecycle instructions | 1.476x | 1.319 million |

This is an upper-bound model, not a forecast. It proves that local instruction
cleanup is worthwhile and insufficient. Even perfect removal of those counted
instructions leaves the principal gap intact.

Procedure-level Amdahl ceilings—the maximum whole-program gain possible when a
measured fraction is accelerated—reinforce the point:

| Hypothetical change | Overall ceiling | Approximate CPS |
| --- | ---: | ---: |
| make `parseExec` ten times faster | 2.176x | 1.94 million |
| make `parseExec` free | 2.503x | 2.24 million |
| make PARSE and stem self time free | 4.419x | 3.95 million |
| also make ADDRESS and TRACE self time free | 9.737x | 8.70 million |

No one of dispatch, calls, lifecycle cleanup or PARSE can close the gap. The
response must combine semantic lowering, compiler/RXAS work, value/call
efficiency and eventually adaptive execution.

### 3.7 What is and is not yet known about ooRexx

The investigation has ruled out several superficial explanations: RexxCPS
2.1 versus 2.2, the score arithmetic, the 1,000-clause convention, Release
optimization flags, and `rxvm` versus `rxbvm`. It has also positively located
large cREXX costs in generic PARSE, stem, ADDRESS and TRACE library paths and
in dynamic instruction expansion.

It has **not** yet completed a source- and counter-backed ooRexx architecture
or optimizer review. The M1 result makes that omission more important: ooRexx
reports about 4.52 times the NetRexx 2.1n rate even though the latter executes
compiled Java bytecode and may receive JVM JIT compilation.

That does not imply that ooRexx is “faster than C.” RexxCPS counts nominal Rexx
source clauses; it does not count C, C++, Java or machine instructions. CREXX
currently expands each nominal clause into about 194 VM instructions and then
executes significant PARSE, stem, ADDRESS and TRACE behavior in general Level
B library code. A mature Classic interpreter may instead execute one
specialized native handler for a high-level operation. The comparison is
between complete semantic paths, not implementation languages.

Likely explanations to test include native implementation of Classic
PARSE/compound-variable/TRACE/ADDRESS semantics, a lower-overhead internal
clause representation, fewer general language-level calls and conversions,
mature interpreter specialization, cached plans and legitimate constant
folding. Partial benchmark-work elimination must also remain an explicit
hypothesis until section 2.5 is complete. The existing trace argues against
wholesale deletion but is not enough to rule out partial optimization.

Strand 2 must therefore map the canonical ooRexx timed kernel to actual runtime
paths, internal operation counts and hardware evidence; inspect optimizer
decisions; and compare it with both NetRexx generated Java and native-C control
kernels before CREXX bases an architecture decision on the result. These are
forensic questions, not reasons to discount the ooRexx score in advance.

## 4. Existing VM result and architecture constraints

The recently implemented dispatch work added coherent active-frame caching and
a separate computed-goto runtime instruction image while keeping canonical
RXBIN immutable. On Apple clang, the final `rxvm / rxbvm` median is 0.999 with
a 0.960–1.031 workload range; each VM wins some workloads. RexxCPS likewise
shows no material difference.

Consequences:

- adopting `rxbvm` as the default is still a cross-platform portfolio decision,
  not an automatic speed fix;
- canonical `segment.binary` must remain portable, immutable and suitable for
  reflection/serialization;
- process-local handler addresses belong only in a separate runtime image;
- on-disk RXBIN is already packed and decoded at load, so smaller serialized
  opcode numbers do not automatically make execution faster;
- a compact **runtime execution stream** is a valid prototype only if it
  improves cache behavior after accounting for decode, operands, branch
  targets, metadata and source coordinates.

The public instruction table is predominantly zero-to-three operand formats.
The RXAS optimization queue carries up to ten token fields because metadata and
trace records need them; this is not evidence that five-operand executable
instructions are already cheap or supported. Before adding a general
five-operand format, compare:

1. true four/five-operand runtime instructions;
2. a constant-pool or call-site descriptor referenced by a normal instruction;
3. quickened runtime-only instructions with side metadata;
4. multiple compact instructions whose combined stream is smaller and easier
   to dispatch.

Widening common instructions can increase execution-stream footprint and
register pressure. Operand count is a design variable, not an objective.

## 5. What “competitive” should mean

### 5.1 RexxCPS policy

ooRexx parity on RexxCPS is an aspiration and useful outcome, **not a Release 1
gate**. Release governance should instead require:

- a published, equivalent-work RexxCPS score and explanation;
- no benchmark-specific shortcut in the standard score;
- a documented plan for any remaining order-of-magnitude outlier;
- no regression hidden by a portfolio average;
- sustained improvement on representative workloads without semantic loss.

A benchmark-specific native instruction may be built as a labelled research
upper bound. It must not appear in the standard product score unless it is a
general production implementation of documented language semantics and is
available to ordinary programs.

### 5.2 Portfolio scorecard

Competitiveness should be assessed in several dimensions:

| Dimension | Required measures |
| --- | --- |
| steady-state throughput | correctness-gated per-workload ratios and portfolio geometric mean—the multiplicative average that gives equal weight to benchmark ratios |
| startup/latency | compile, assemble, link, load, prepare and first-result time reported separately |
| memory | peak resident set size (RSS), allocation counts/bytes, frame/value high-water marks, runtime-image size |
| artifact efficiency | source, RXAS, RXBIN and linked-image sizes |
| build/deployment | compilation/link time, package size, portability, deterministic AOT workflow |
| semantics/flexibility | typed Level B coverage, Classic workload coverage, classes/interfaces/references, plugins and ADDRESS, late loading |
| integration | embeddability, host callbacks, native packaging, absence of a mandatory JVM or JIT runtime |
| observability | TRACE/debug/source coordinates, disassembly, profiler and `rxseq` support |
| maintainability | shared semantics, cross-platform behavior, generated tests and supportable VM variants |

CREXX's flexibility is a real product benefit: a portable compiler-to-bytecode
pipeline, explicit linking and late loading, strict typed text/binary surfaces,
classes/interfaces/references, plugin and ADDRESS integration, native packaging,
embeddability, and first-class instrumentation are not represented by
RexxCPS. These should be measured and described. They are part of the value
case, but not a waiver for avoidable throughput overhead.

Numerical portfolio thresholds should be fixed only after the baseline matrix
exists. Until then, “competitive” should mean a strong improving geometric
mean, no unexplained representative pathological outlier, and documented
trade-offs where CREXX buys flexibility or observability with measurable cost.

### 5.3 NetRexx as an achievable comparative target

Subject to a current same-host equivalence and optimizer-resistance audit,
NetRexx-equivalent throughput is a credible long-term CREXX target. At minimum,
CREXX should aim to stay in the same decimal order of magnitude across the
comparable portfolio; the preferred target is to approach the NetRexx
geometric mean rather than merely cross the mathematical ten-times boundary.
The quoted M1 RexxCPS 2.1n value of 5.43 million CPS makes the existing
approximately five-million first architecture waypoint plausible, but it is
context rather than a target baseline until reproduced with current ports on
the same hosts.

This comparison also captures a legitimate CREXX value proposition. NetRexx
normally needs a JVM and its class/runtime deployment, whereas CREXX can deploy
portable RXBIN with a native VM and does not require a JVM or machine-code JIT.
That potential advantage should be reported with measured package size,
startup latency and RSS rather than asserted qualitatively. Equivalent
throughput with materially lighter deployment would be a competitive result;
some throughput deficit may also be acceptable where the deployment,
embeddability, observability or language-flexibility gain is explicit and
measured.

ooRexx remains the stronger Classic-performance reference and the urgent
forensic case. Its RexxCPS lead over NetRexx is too large to explain by saying
only “native interpreter” or “Java overhead.” The programme must determine how
much comes from equivalent useful work, Classic-specific native operations,
representation/call efficiency, optimizer specialization, or invalidly
eliminated benchmark work.

## 6. P0/P1 no-regrets activity register

### 6.1 P0: begin immediately

| ID | Activity | Why it is no-regrets | Deliverable / exit criterion |
| --- | --- | --- | --- |
| NR-01 | Reproducible benchmark portfolio and runner | Every architecture choice needs comparable evidence | versioned workload manifest; serial raw samples; correctness checks; machine/build provenance; startup and steady-state reports |
| NR-02 | Equivalence and optimizer-resistance ledger | Prevents invalid cross-language claims, dead-code elimination and unreported specialization | per-port timed-kernel table, opaque-input/result-observation variants, dynamic counts, disclosed substitutions and independent review |
| NR-03 | Automated performance evidence bundle | Existing profiling tools are valuable only if runs are repeatable and reviewed across all cost dimensions | one command produces unprofiled timing, instruction/procedure comma-separated-value (CSV) data, allocation counters and sequence-window lengths N=2/3/4 as RXSEQ artifacts for an exact image set; the report ranks instruction count, total time, average latency and paired delta, and separates static sites plus re-entrant self/child time |
| NR-04 | Opcode effects inventory | Flow, liveness and superinstructions need machine-readable semantics | generated read/write/kill/alias/reference/throw/branch/call metadata checked against handlers and tests |
| NR-04A | Kind-index runtime metadata and scan counters | Ordinary type/ADDRESS operations must not traverse source/TRACE records; the TRACE-off control found a general retained-metadata penalty | per-kind traversal counters; indexed or split runtime contract lookup; identical stripped/unstripped instruction counts; retained-metadata portfolio gain without losing diagnostics |
| NR-05 | Call-path census | Call optimization must be portfolio-led, not RexxCPS-led | calls by kind/arity, swaps/copies per call, frame reuse, return placement, dynamic selection and signal-unwind frequency |
| NR-06 | Compiler call-window placement fast path | Removes movement while preserving the present ABI and unwind contract | arguments emitted into final slots where safe; correct parallel-copy scheduling; focused alias/reference/signal tests; measured portfolio result |
| NR-07 | Direct compare-to-branch lowering | Removes a known, large general producer/consumer pattern | `rxc` emits typed branch forms when the Boolean has no other use; RXAS fallback retained; source/debug semantics tested |
| NR-08 | Definite initialization and reference-lifetime facts | Enables safe removal of `NULL`, `ENDLIFE` and redundant copies across many designs | compiler annotations/dataflow, fail-closed elimination, reference escape tests, dynamic count reduction |
| NR-09 | RXSEQ candidate ledger | Converts one-off mining into an evidence pipeline | top patterns by workload/site/module with semantic status, owner, proposed lowering/fusion and measured outcome |
| NR-10 | ooRexx/Regina/NetRexx/Java/C forensic baselines | Establishes whether gaps come from algorithms, optimizer behavior, runtime architecture or invalid work | same-host versions, raw results, source/internal-form provenance, dead-code-elimination proof, ooRexx operation counts, generated Java/class inspection, labelled native-C ceilings and reproducible commands |
| NR-11 | Performance governance and scorecard | Prevents single-score optimization and ambiguous release claims | agreed highest-priority (Tier A) and secondary (Tier B) portfolio, geometric-mean policy, outlier rule, regression budget and publication template |

NR-06 is deliberately P0. The current RexxCPS profile contains about 1,706
swaps for about 1,049 calls per outer iteration, and the compiler already has
the contiguous-window and const-formal machinery needed for an incremental
implementation. This is a bounded compiler/code-generation optimization, not
an ABI redesign. Its priority comes from generality and low architectural
regret, not from a claim that it will close the RexxCPS gap.

### 6.2 P1: bounded production improvements

| ID | Activity | Boundary | Exit criterion |
| --- | --- | --- | --- |
| NR-12 | Extend read-only by-value and return copy coalescing | no weakening of by-value isolation or reference semantics | semantic proof plus reduced call copies on portfolio |
| NR-13 | Redundant numeric-context setup elimination | only when effective procedure context is identical | cross-procedure numeric tests and dynamic-count reduction |
| NR-14 | Static/frozen `PARSE` lowering fast path | general supported template classes only; generic `parseExec` remains fallback | equivalent result/TRACE/source behavior and material gains on RexxCPS plus other parse workloads |
| NR-15 | General stem default/reset fast path | preserve generation/default/drop/tail semantics | focused Regina/ooRexx semantic matrix and stem-heavy performance cases |
| NR-16 | TRACE-off and same-ADDRESS-environment fast paths | preserve mode changes, hooks, signals and host callbacks | semantics tests plus lower call/instruction counts in ordinary off/same-state cases |
| NR-17 | Link-time direct provider/call resolution | retain late-load and plugin fallback | resolved calls bypass redundant stubs/lookups; linked and late-load suites remain green |
| NR-18 | Safe RXAS rule harvest | only rules proved by opcode effects/liveness, not pattern frequency alone | generated rule tests and before/after portfolio plus RXAS size report |
| NR-19 | Optional C link-time optimization (LTO), profile-guided optimization (PGO) and code-layout experiment | build option, no semantic or package dependency | paired compiler/platform evidence; adopt only if repeatable and supportable |
| NR-20 | Value/frame allocation counters and targeted pooling | no background garbage-collection (GC) assumption or ownership weakening | allocation and high-water evidence, leak/sanitizer gates, workload benefit |

### 6.3 P1 architecture-neutral prototypes

These are no-regrets as **time-boxed evidence**, not automatic production
commitments.

| ID | Prototype | Question answered | Adoption gate |
| --- | --- | --- | --- |
| NR-21 | mapped-call descriptor | does a precomputed register-to-argument map beat compiler placement and preserve aliasing/cold unwind? | cross-platform portfolio win large enough to justify ABI/runtime complexity |
| NR-22 | compact runtime execution stream | can smaller opcode/operand cells improve instruction-cache (I-cache) and branch behavior after preparation cost? | canonical RXBIN remains unchanged; counters and total-time win on at least two architectures |
| NR-23 | runtime quickening/superinstructions | can hot stable operations be patched in the private runtime image without semantic drift? | dequickening, signals, TRACE/debug coordinates, late load and both VM modes are defined and tested |
| NR-24 | profile-selected fusion | do RXSEQ-selected fusions outperform static lowering without code-size explosion? | gains repeat across portfolio and are not RexxCPS-only |
| NR-25 | value hot/cold split or pool allocator | is value footprint/allocation a limiting factor after semantic lowering? | cache/allocation counters justify complexity; sanitizer and native payload ownership remain sound |

## 7. Ordered programme strands and terms of reference

### Strand 1 — Benchmark science and comparative baseline

**Question:** How fast is CREXX, at what lifecycle stage, on what work, relative
to what implementation?

**Work:** Expand the current five-program language suite with established
Classic Rexx, Are We Fast Yet?/Simple Object Machine (SOM), parser/text,
allocation/object, binary, collections, calls/recursion, startup and application
workloads. Run ooRexx and
Regina where the semantics are genuinely comparable; include NetRexx and Java
ports only with explicit optimizer-resistance checks and comparable algorithms.
Keep the canonical benchmarks unchanged and add separately labelled opaque-input
and result-observation variants where elimination is a concern.

**Deliverables:** workload manifest; provenance/equivalence tables; harness;
raw samples; host/build/version record; separate startup/steady/memory/artifact
reports; portfolio scorecard.

**Gate:** no performance claim enters release material without correctness,
provenance, raw samples and a named lifecycle measure.

### Strand 2 — Comparative runtime forensics

**Question:** Why do ooRexx, Regina, NetRexx and Java execute each workload at
their observed rates, and is ooRexx performing all intended RexxCPS work?

**Work:** Inspect their executable representation, loop/branch lowering,
variable and compound-variable lookup, PARSE implementation, call/frame path,
string/number representation, memory allocation and any interpreter or JIT
specialization. For ooRexx, inspect the source/internal instruction stream and
optimizer, add internal operation counts where feasible, run the section 2.5
challenge variants, and compare source with supported precompiled execution.
For NetRexx, retain generated Java, inspect class bytecode, record JVM/JIT and
warmup settings, and prove timed results remain observable. Add mechanical and
dynamic-value native-C ceiling controls with explicit anti-elimination sinks.

**Deliverables:** architecture comparison; workload-to-runtime operation maps;
hardware-counter comparison where feasible; optimizer-elimination ledger;
validated same-host ooRexx/NetRexx ratio; confirmed advantages separated from
hypotheses.

**Gate:** do not copy an architecture feature until a CREXX profile identifies
the same bottleneck and a prototype predicts a portfolio benefit.

### Strand 3 — Semantic lowering and general native fast paths

**Question:** Which high-level semantics are being reinterpreted through large
Level B library paths and should instead have direct compiler/VM support?

**Work:** Static/frozen PARSE, stems, TRACE-off, same-environment ADDRESS,
frequent string scans and numeric conversions. Start with compiler lowering to
existing primitives; add a native instruction only when the operation is
general, stable, measurable and cannot be expressed efficiently otherwise.

**Deliverables:** semantic equivalence tests, fallback path, instruction/call
count delta and portfolio timings.

**Gate:** benchmark-only opcodes are labelled research upper bounds. Only
general production semantics may contribute to standard benchmark scores.

### Strand 4 — `rxc` and RXAS flow analysis

**Question:** What work can be eliminated or combined with control-flow,
liveness, alias and reference facts unavailable to the current keyhole pass?

**Work:** machine-readable opcode effects; basic blocks and a control-flow graph
(CFG); definite
assignment; compare-result single use; lifetime/escape facts; copy propagation;
dead initialization; numeric-context propagation; safe branch fusion. Decide
which facts originate in the typed abstract syntax tree (AST)/`rxc` and which
remain useful for hand-written RXAS.

**Deliverables:** staged analysis intermediate representation (IR), generated
effect validation, small
fail-closed transformations and optimizer diagnostics.

**Gate:** each transform has a semantic proof obligation, negative tests and a
measured dynamic effect. Full global RXAS optimization is not a prerequisite
for profitable compiler lowering.

### Strand 5 — VM dispatch, execution stream and instruction design

**Question:** Which execution representation gives the best portable portfolio
result without damaging RXBIN portability, reflection or diagnostics?

**Work:** complete native Intel Linux and Windows `rxvm`/`rxbvm` evidence;
measure compact runtime cells; investigate encoded branch targets; mine
superinstructions; compare three operands, wider formats and descriptors;
measure preparation/startup and runtime image size.

**Deliverables:** cross-platform paired samples and counters; runtime-stream
prototype; ISA/code-size study; default-VM decision.

**Gate:** canonical RXBIN remains immutable and portable. A default or format
change requires a portfolio win on multiple architectures, complete semantics,
late-load, signal, metadata, debugger and sanitizer validation.

### Strand 6 — Calls, register allocation and frames

**Question:** How much call cost comes from argument placement, copying,
dynamic resolution, frame setup and return handling, and what is the least
complex fast path?

**Work:** call census; compiler call-window allocation; parallel-copy
scheduling; const/by-value copy elision; return coalescing; frame reuse metrics;
direct linked-call resolution; separately prototype a mapped-call descriptor.

**Deliverables:** arity/call-kind dashboard; semantic stress suite; phased
compiler fast path; optional descriptor prototype.

**Gate:** production phase A preserves the current contiguous-window ABI and
signal unwind. Any mapped ABI must beat phase A materially and prove references,
repeated arguments, mutation, exceptions/signals, recursion, dynamic calls and
native calls.

### Strand 7 — Value representation, allocation and lifetime

**Question:** Are value footprint, initialization, payload ownership, string
materialization, attributes or allocation limiting representative workloads?

**Work:** counters before redesign; remove dead lifecycle work; exploit small
values/strings; investigate hot/cold split and pools; improve string
representation caching; preserve deterministic frame/reference ownership.

**Deliverables:** size/layout report, allocation and cache counters, focused
prototypes and sanitizer evidence.

**Gate:** no background GC or ownership redesign without demonstrated need.
The current VM has deterministic frame cleanup and native payload ownership;
an extra memory-management thread is not a free optimization.

### Strand 8 — Link/load, whole-program and build optimization

**Question:** What can be pre-resolved once rather than discovered or copied at
runtime?

**Work:** direct provider references; redundant import stub removal;
kind-specific runtime metadata indexes, then metadata stripping/deduplication;
runtime preparation caches; optional LTO/PGO and code layout; linked-image
versus late-load comparison.

**Deliverables:** link/load time and image-size reports, direct-call prototype,
late-load fallback and packaging validation.

**Gate:** dynamic modules/plugins continue to work; production builds remain
reproducible and supportable without mandatory profile data.

### Strand 9 — Adaptive execution and poor man's JIT

**Question:** How much of JIT benefit can CREXX obtain without committing to a
machine-code compiler?

**Work:** counters in the private runtime image; quickened typed operations;
inline caches for dynamic selection; profile-selected superinstructions; hot
procedure cloning/specialization; evaluate machine-code compiler back ends only
after these results.

**Deliverables:** dequickening and invalidation model, source-coordinate model,
prototype results and complexity assessment.

**Gate:** machine-code JIT remains P2 research. Quickening must preserve
signals, TRACE/debug behavior, late loading, canonical RXBIN and deterministic
fallback.

### Strand 10 — Reentrancy, concurrency and background services

**Question:** Which concurrency capabilities benefit user workloads, and which
single-thread VM services can safely operate asynchronously?

**Work:** first define shared-state, stack, value, reference, plugin, ADDRESS,
signal and module ownership. Measure whether any loader, compiler or host I/O
work can overlap. Treat application-level independent VMs/processes separately
from shared-memory threads.

**Deliverables:** ownership/design note, race/sanitizer strategy and one bounded
prototype only if a measured workload supports it.

**Gate:** threads are not a RexxCPS optimization. Dispatch and instruction
execution are serial dependencies, and the VM has no background GC to offload.
Shared-memory VM threads remain design-only for Release 1 unless their safety
model and an independent product use case are approved.

## 8. Dependencies and execution order

The strands should overlap, but their decision dependencies are:

```text
portfolio + equivalence + instrumentation
                  |
       runtime forensics / RXSEQ mining
        /          |            \
semantic       flow/lifetime    call-window
lowering        analysis         fast path
        \          |            /
        measured production foundations
                  |
  dispatch/compact stream/value/link prototypes
                  |
        architecture selection gate
                  |
       quickening, JIT or concurrency research and development
```

P0 evidence work should start at once. P0 compiler call placement,
compare-to-branch and lifecycle facts can proceed against focused tests while
the full comparative matrix is assembled. Direct PARSE and stem work should
then dominate P1 because profiling shows they are far larger than dispatch or
raw swap overhead.

## 9. Target bands, not promises

Current evidence supports staged planning bands:

| Stage | Indicative result | Basis and caution |
| --- | ---: | --- |
| local compiler/RXAS/lifecycle work | 1.2–1.4 million RexxCPS | consistent with the 1.42–1.48x count-only upper bounds; not guaranteed |
| first serious architecture target | about 5 million | requires direct PARSE plus faster stems/TRACE/ADDRESS and general call/copy reductions; near the quoted 5.43 million M1 NetRexx context but not yet a same-host target |
| NetRexx comparative target | same decimal order first; approach parity preferred | must be set from an audited current same-host portfolio, not the single 2.1n M1 result |
| stretch target | about 10 million | likely needs broader semantic primitives, call/value work and superinstructions/quickening; would put RexxCPS clearly in the NetRexx performance band if the audited reference remains similar |
| current ooRexx level | about 40.8 million | would require radically lower work per clause, JIT/AOT, or a highly specialized Classic execution path; do not promise |

These are engineering waypoints for RexxCPS, not release gates and not
portfolio forecasts. The programme should revise them after the first complete
baseline and after each major semantic-lowering experiment.

## 10. Release 1 response and decision gates

### 10.1 Recommended release sequence

1. **Close beta 3 conservatively.** Land only benchmark/equivalence assets,
   evidence automation, already-bounded safe optimizer work and this programme
   charter. Do not put a new ISA, ABI, JIT or shared-memory thread design into
   beta 3.
2. **Replace the fixed 2026-08-31 Release 1 event with an architecture selection
   gate.** Complete the benchmark portfolio, ooRexx forensics, P0 compiler fast
   paths, cross-platform dispatch matrix and first PARSE/stem prototypes.
3. **Plan a beta 4 Performance Foundations release.** Ship the selected bounded
   compiler/RXAS/semantic improvements with repeatable evidence and unchanged
   correctness.
4. **Start Release Candidate only after beta 4 and two cross-platform
   validation cycles.** Package, ABI/format and performance claims should then
   describe what is actually shipped.

If an RXBIN format, public call ABI or value representation change is selected,
late Q4 2026 is the earliest credible Release 1 window. If machine-code JIT or
shared-memory threading becomes must-ship, Release 1 moves into 2027 and needs
a separate charter. Neither is recommended as a must-ship item now.

### 10.2 Architecture selection gate

The gate should answer:

- Is `rxvm` or `rxbvm` the default on each supported platform, or is one common
  default supportable?
- Which P0/P1 compiler and semantic improvements have repeatable portfolio
  gains?
- Is a compact runtime stream or mapped-call descriptor worth its complexity?
- Does any accepted work change RXBIN, the call ABI, native/plugin interfaces,
  debugging or package compatibility?
- What RexxCPS gap remains, why, and what is the next bounded response?
- Is the portfolio result competitive when throughput, latency, memory,
  portability, flexibility, integration and observability are considered?

### 10.3 Release acceptance policy

Release 1 should not be blocked because CREXX fails to equal ooRexx on one
synthetic benchmark. It should be delayed if the team has not established
credible evidence, does not understand major representative regressions, or
has accepted an unstable architecture change without cross-platform proof.

The acceptance record should contain:

- all Tier A (highest-priority release workload) correctness results;
- raw benchmark samples and portfolio scorecard;
- RexxCPS 2.2c result and equivalence disclosure;
- all known material outliers with explanation/owner/next action;
- startup, memory and image-size results;
- flexibility/integration capability statement;
- selected VM/ISA/ABI decisions and fallbacks;
- supported-platform and package validation.

## 11. Immediate first work package

The recommended first bounded work package is:

1. freeze the current RexxCPS evidence and commands in a dated raw report;
2. create the separately labelled optimizer-resistance variants from section
   2.5 and run them first on ooRexx, then NetRexx, Regina and CREXX;
3. retain/inspect ooRexx internal operation counts and NetRexx generated
   Java/class code, and add labelled native-C ceiling controls;
4. make the five existing language workloads the seed, not the final portfolio;
5. add call-kind/arity/swap/copy and allocation counters to the evidence bundle;
6. implement compiler call-window placement without changing the ABI;
7. implement direct compare-to-branch for single-use typed Boolean results;
8. introduce definite-initialization/reference-lifetime facts and remove only
   proven-dead `NULL`/`ENDLIFE` operations;
9. run `rxseq` over the full seed portfolio and rank general patterns;
10. prototype static/frozen PARSE lowering and measure it before proposing an
   opcode;
11. add focused stem, TRACE and ADDRESS benchmarks and semantic matrices;
12. report all results to the architecture selection gate, including negative
    and workload-specific results.

This package produces useful code and evidence under every later architecture
choice. It also gives the team an early answer to the practical question: how
far can CREXX move through compiler placement, flow facts and direct semantic
lowering before a new execution format or adaptive runtime is justified?

## 12. Evidence sources and caveats

Primary repository evidence for this report includes:

- [`tests/benchmarks/rexxcps_levelb.crexx`](../../../tests/benchmarks/rexxcps_levelb.crexx)
  and its equivalence audit in
  [`tests/benchmarks/README.md`](../../../tests/benchmarks/README.md);
- the current five-workload baseline in
  [`levelb-library-benchmark-baseline-2026-07-13.md`](levelb-library-benchmark-baseline-2026-07-13.md);
- VM profiling and RXSEQ usage in
  [`profiling.md`](../../books/crexx_programming_guide/profiling.md);
- the implemented dispatch evidence in
  [`vm-dispatch-performance-investigation.md`](../beta-3/notes/vm-dispatch-performance-investigation.md);
- current call/unwind behavior in
  [`RXVM_INTERPRETER.md`](../../ai-context/RXVM_INTERPRETER.md);
- compiler call marshalling and read-only-formal optimization in
  `compiler/rxcp_emit_expr.c` and `compiler/rxcp_opt.c`;
- RXAS rules and the bounded optimization queue in `assembler/rxas_opt.c` and
  `assembler/rxas.h`;
- the existing packed/link design constraints in
  `compiler/docs/rxbin_compaction_working.md`;
- the official NetRexx compiler description in the
  [NetRexx Programming Guide](https://www.netrexx.org/documents/NetRexx%204.02-GA%20Programming%20Guide.pdf),
  which documents translation through Java source to JVM class bytecode;
- the official
  [ooRexx source tree](https://sourceforge.net/p/oorexx/code-0/HEAD/tree/main/trunk/),
  which is the required basis for the proposed internal execution and optimizer
  audit rather than speculation from the headline score.

The detailed RexxCPS profiles were captured from a profiling build and local
ignored artifacts. Instrumented absolute times are perturbed and are not used
as product throughput. Procedure shares, dynamic instruction counts and RXSEQ
patterns guide prioritization; final speed claims must come from an ordinary
unprofiled Release build using identical generated images and raw serial
samples.
