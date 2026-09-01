# RexxCPS cross-runtime source review

**Date:** 2026-07-22
**Scope:** Read-only investigation for the cREXX performance programme
**Repository snapshot:** `240b29f456e995928206f04285a7c319612ff022` (`develop`) at the start of the review
**Output status:** Dated investigation record copied into the repository on 2026-07-23; no production code or worktrees were created or changed by the review itself.

> **2026-07-23 live-status addendum:** The analysis below deliberately preserves
> the 2026-07-22 snapshot at `240b29f456e995928206f04285a7c319612ff022`.
> Subsequent accepted work materially changed the current comparison:
> `performance/ROADMAP.md` now records NR-15, NR-16, and NR-17 complete, and the
> final NR-16/NR-17 absolute checkpoint reports 28.120M/26.119M clauses/s for
> `rxvm`/`rxbvm`. That invalidates the snapshot's statement that the remaining
> current gap is comfortably above tenfold. The competitor-source mechanisms
> remain useful architectural evidence, but all gap sizes and ranked follow-up
> actions below must be read as dated findings and reconciled with the live
> roadmap before new work is selected.

## Executive conclusion

Regina, ooRexx, and NetRexx do not owe their RexxCPS lead to one exceptional opcode or allocator. Their shared advantage is that much more semantic work is performed once, before or at the first execution of a site, and the repeated hot path is then expressed in forms close to the runtime's native data structures:

1. variables, calls, loop limits, and parse templates are resolved or compiled once;
2. common built-ins execute as direct native/runtime operations rather than as general interpreted library procedures;
3. strings and numerics retain reusable representations to avoid repeated conversion;
4. frames, argument boxes, expression stacks, and small objects are reused or pooled; and
5. the runtime keeps a cheap, direct path for the common case while preserving a general semantic fallback.

The strongest cREXX-local evidence for this explanation is already in the programme. NR-14 replaced repeated generic work for frozen `PARSE` templates with precompiled plans and raised RexxCPS by about 45% in both VM variants. That is the same broad technique seen throughout the three faster implementations.

The formal NR-10 same-host baseline recorded cREXX at about 1.22 MCPS, versus 33.21 MCPS for Regina, 39.92 MCPS for ooRexx, and 48.07 MCPS for NetRexx. This is a 27x, 33x, and 39x lead respectively. Later accepted NR-14 results put cREXX around 1.78–1.79 MCPS. Comparing those later cREXX figures to the older competitor measurements only as an orientation still leaves an approximately 19x–27x gap. A new controlled cross-runtime run is required before treating those latter ratios as current measurements.

For future cREXX work, the best next question is therefore not “how can the dispatch loop be made ten times faster?” cREXX already has computed-goto dispatch, while ooRexx remains much faster despite virtual instruction dispatch and object semantics. The better question is “which semantic operations and helper boundaries still recur inside the canonical RexxCPS hot path, and which can safely become pre-bound, precompiled, intrinsic, or representation-preserving common cases?”

## 1. Performance programme status

`performance/ROADMAP.md` is the live control plane and was changing during this review. The following is a point-in-time read, not a claim that the active tree is stable:

- The original P0 measurement and governance programme, NR-01 through NR-11, is complete or explicitly rejected where appropriate.
- Numeric work and the small NR-08/NR-09 runtime improvements are complete.
- NR-14, frozen/hybrid parse plans, is complete and accepted. It produced the large RexxCPS result discussed below.
- NR-15, stem/string access, is in progress in the active tree and is being handled by another agent. Its worklist and evidence were deliberately not treated as final evidence here.
- NR-16, NR-17, NR-19, NR-20, and NR-22 through NR-25 remain queued or otherwise not completed in the live roadmap.
- NR-18, NR-21, NR-26, and NR-27 are recorded complete.
- The architecture-selection gate is not yet complete.

The working tree changed while the review was running. By the final status read, the active work included modifications under `binutils/`, `interpreter/`, `lib/rxfnsb/rexx/`, `performance/`, and `tests/performance/`, plus new NR-15 worklist, fixture, and evidence files. This report did not edit, stage, reset, stash, or otherwise manipulate any of it.

## 2. Measurement anchor and remaining gap

### 2.1 Formal NR-10 same-host baseline

The formal source is `performance/evidence/2026-07-20-nr-10-formal-baseline/scorecard.md`. It used the same host and retained exact runtime and build provenance. Medians are over ten recorded runs.

| Runtime | Median nominal CPS | Relative to `rxvm` | Relative to `rxbvm` |
|---|---:|---:|---:|
| cREXX `rxvm` | 1,227,312.5 | 1.00x | — |
| cREXX `rxbvm` | 1,219,940.0 | — | 1.00x |
| Regina 3.9.7 | 33,214,478.5 | 27.06x | 27.23x |
| ooRexx 5.1.0 | 39,920,638.0 | 32.53x | 32.72x |
| NetRexx 5.10-GA on mixed-mode HotSpot | 48,067,752.0 | 39.17x | 39.40x |

These numbers measure the benchmark's nominal clause rate, not raw VM opcode throughput and not simply reciprocal process wall time. The benchmark performs a one-second calibration and reports its own rate. That distinction matters when reasoning about mechanisms.

### 2.2 Accepted cREXX movement after NR-10

The accepted NR-14 first Release verdict records medians of 1,792,683 CPS for `rxvm` and 1,778,033.5 CPS for `rxbvm`, improvements of 45.249% and 45.499% over its retained baseline.

If those newer cREXX medians are divided into the older NR-10 competitor medians, the remaining leads are approximately:

| Runtime | Versus later `rxvm` | Versus later `rxbvm` |
|---|---:|---:|
| Regina | 18.53x | 18.68x |
| ooRexx | 22.27x | 22.45x |
| NetRexx | 26.81x | 27.03x |

This table is deliberately labelled **orientation, not a governed comparison**. It combines different sessions and cREXX artifacts. It establishes only that the gap remains comfortably above tenfold after NR-14; it does not replace a post-NR-14 cross-runtime baseline.

## 3. What RexxCPS actually exercises

The cREXX benchmark is `tests/benchmarks/rexxcps_levelb.crexx`, a disclosed Level B adaptation of RexxCPS 2.2d. The cross-runtime baseline retains:

- `tests/benchmarks/cross-runtime/rexxcps/rexxcps_2_2.rex`, the Classic Rexx 2.2 source used with Regina and ooRexx; and
- `tests/benchmarks/cross-runtime/rexxcps/rexxcps_2_2n.nrx`, the disclosed NetRexx adaptation.

The canonical workload is 100 outer repetitions by 100 inner iterations after a one-second calibration. The CTest smoke mode using count 10 is useful for quick validation but is not the canonical performance workload.

### 3.1 Timed kernel

The benchmark assigns a nominal 1,000 clauses to each calibrated iteration. The timed work mixes:

- nested counted and controlled loops;
- integer and decimal arithmetic and comparisons;
- string assignments and conversions;
- compound-variable/stem access and stem default assignment;
- `LENGTH`, `WORD`, and `SUBSTR` calls;
- internal routine calls and argument parsing;
- `TRACE` query/change behavior;
- `ADDRESS` query/change behavior; and
- several `PARSE VAR`/value operations.

Formatting operations such as `REVERSE`, `TRANSLATE`, `STRIP`, and `DATATYPE` occur in result formatting outside the timed kernel. They are relevant examples of runtime implementation style, but they must not be attributed to the measured gap.

### 3.2 Source-equivalence limits

The benchmark is useful for architectural comparison, but the three language variants are not byte-for-byte equivalent programs:

- The cREXX Level B version uses explicit types and conversions, including integer repetition counts, a decimal loop value, a floating timer value, and typed routine arguments.
- Regina and ooRexx run the Classic source.
- The NetRexx source changes loop spelling and compound-variable representation, removes `TRACE` to compensate for other source differences, changes the `ADDRESS` exercise, turns routines into methods, and removes Classic parse keywords. It also specifies `NUMERIC DIGITS 20`, whereas the Classic source uses 9.

Consequently, NetRexx is best treated as an architectural ceiling and a source of hypotheses, not as proof that one isolated implementation technique will reproduce its exact ratio in cREXX. Regina and ooRexx are closer comparisons because they run the same Classic source, though their runtime semantics and implementations still differ.

## 4. Regina 3.9.7 source review

Source inspected from the official [Regina 3.9.7 release](https://sourceforge.net/projects/regina-rexx/files/regina-rexx/3.9.7/), archive SHA-256 `f13701ebd542e74d0fc83b2a7876a812b07d21e43400275ed65b1ac860204bd4`.

### 4.1 Execution representation: compact parsed tree with site caches

Regina executes a parsed instruction/expression tree directly. Its tree-node union stores not only syntax but execution-ready information: cached variable references, cached numeric values, resolved internal routine targets, and resolved built-in function pointers. The interpreter uses a large direct switch with `goto` continuations for several hot constructs, reducing repeated recursive interpreter entry.

This is a recurring pattern in Regina: a generic site becomes more specific after the runtime has learned enough about it. Examples include:

- simple-variable expression nodes retaining a variable-box shortcut, validated against the current generation;
- constants and variables retaining a parsed numeric representation alongside their string value;
- internal-call nodes retaining the resolved target; and
- built-in-call nodes retaining a native C function pointer.

The important mechanism is not merely tree walking. It is that name resolution, conversion, and target selection are not repeated in full on every visit to a stable site.

Relevant upstream files include `regina_t.h`, `interprt.c`, `expr.c`, `variable.c`, and `yaccsrc.c`.

### 4.2 Loops and arithmetic: retain state and mutate the common representation

Controlled-loop state retains the current value, increment, limit, comparison state, and numeric descriptors. Regina has a decimal-string increment path that updates the value in place when the representation permits it rather than constructing a general arithmetic expression from scratch for every iteration. Parser-side classification also marks numeric assignments and specializes several simple comparisons.

Regina therefore still performs Rexx decimal semantics, but it avoids rediscovering the same facts on every iteration. This is relevant to RexxCPS because its timed kernel repeatedly traverses simple numeric loops and comparisons.

### 4.3 Built-ins and parsing: direct C over the runtime representation

The built-in table binds names such as `ADDRESS`, `DATATYPE`, `LENGTH`, `SUBSTR`, `TRACE`, `TRANSLATE`, and `WORD` to native C routines. Common operations use direct length fields, pointer arithmetic, scans, and `memcpy`-style copies over Regina's string representation. Parsing is implemented as hand-written C over the source and template representation.

This is a material contrast with cREXX's library-facing form. Functions such as `length.crexx`, `substr.crexx`, and `word.crexx` are authored as Level B bytecode procedures. They do call low-level primitives, and the compiler may optimize some paths, but the general form can retain procedure-frame, argument, branch, and helper-call work around a very small semantic operation. A current profile is needed to determine what remains after all accepted compiler passes; the source form alone is not a dynamic count.

### 4.4 Allocation and call scaffolding: reuse at high frequency

Regina recycles parameter boxes and uses custom size-class free lists/chunks for many runtime allocations. This keeps common call and temporary-storage churn out of the general system allocator. It does not eliminate allocation, and its compound-variable path still constructs and hashes tails. The benefit is lower constant cost across many small operations, not a universally allocation-free engine.

### 4.5 Regina lesson for cREXX

Regina's approximately 27x formal lead is best explained as the accumulation of:

- self-specialising execution sites;
- cached variable, numeric, and call resolution;
- direct native built-ins and parser operations;
- specialised loop/common arithmetic paths; and
- recycled high-frequency storage.

It is not evidence that cREXX should replace computed-goto bytecode dispatch with an AST interpreter. The transferable idea is to cache or compile the semantic decision at the site and leave a cheap repeated path.

## 5. ooRexx 5.1.0 source review

Source inspected from the official [ooRexx 5.1.0 release](https://sourceforge.net/projects/oorexx/files/oorexx/5.1.0/), archive SHA-256 `6343c667c9839839519b7bbd0e9573c3899acdc9509514fc512d450da5c6a67a`.

### 5.1 Execution representation: translated, linked instruction objects

ooRexx translates source into linked instruction and expression objects. `RexxActivation::run()` follows the pre-linked next-instruction pointer, prefetches the following instruction, invokes the current instruction's execute method, and clears a preallocated expression stack between instructions.

This design still pays virtual method dispatch, activity checks, tracing checks, and managed-object semantics. Its strong RexxCPS result is therefore direct evidence that instruction-dispatch style by itself does not explain cREXX's gap.

Relevant upstream files include `interpreter/execution/RexxActivation.cpp` and the `interpreter/instructions/` and `interpreter/expression/` trees.

### 5.2 Variables and calls: translation-time binding

Simple local variables are assigned array indexes during translation. Their common access path reads or writes the indexed local slot; a dictionary is used as a fallback for dynamic/cache-miss cases. The non-traced simple assignment path is especially direct.

Function and routine expressions likewise retain translated binding information:

- internal routine targets can be stored directly;
- built-in calls carry an index into a native function table; and
- expression objects retain the resolved variable/call form.

As with Regina, repeated name lookup is removed from a stable hot site.

### 5.3 Loops and numeric/string representations

Controlled loops retain the `TO` and `BY` expressions and convert invariant bounds where possible. Integer-compatible values use native whole-number fields on their fast path, with a small integer cache for common values. Strings can retain a `NumberString` object, and number strings can retain their string form, so repeated conversions do not always reconstruct the alternate representation.

ooRexx still invokes object operations for addition and comparison in a controlled loop and therefore is not a primitive-only execution engine. The performance lesson is the availability of a short common representation path with lazy conversion and retained results.

### 5.4 Built-ins and parse plans

Built-ins dispatch into native C++ implementations and then directly use string/object methods. `LENGTH` is an immediate length access; substring and other string functions operate in native runtime code. A parse instruction retains its translated trigger array and executes those triggers directly on each visit rather than reparsing the textual template.

This parse architecture closely resembles the direction validated by cREXX NR-14. The accepted 45% RexxCPS movement gives high confidence that “compile stable parse semantics once” is not merely a competitor-source correlation; it is already a causal cREXX result.

### 5.5 Stems, frames, and managed memory

Stem default assignment is an O(1) value change. Compound entries use a tree-backed table rather than being free, constant-time array accesses. The single-tail builder avoids some copies when the tail is already a string or a cached integer string.

Internal subroutines create a new `RexxActivation`, so calls are not free. However, activity-stack storage comes from reusable buffers, the expression stack is reused, and the managed heap maintains size-class dead-object subpools. As in Regina, the aggregate reduces allocator and setup cost without erasing the object model.

### 5.6 ooRexx lesson for cREXX

ooRexx's approximately 33x formal lead is consistent with:

- translation-time binding of locals, calls, built-ins, loops, and parse triggers;
- native built-ins close to the object/string representation;
- lazy and cached numeric/string conversions;
- reusable expression, activity, and object storage; and
- specialised common cases with dynamic fallbacks.

The most important negative finding is that cREXX's raw dispatch loop is unlikely to be the first-order issue. ooRexx tolerates heavier dispatch and object machinery while doing much less repeated semantic setup around each high-level operation.

## 6. NetRexx 5.10-GA source review

Compiler/runtime source inspected from the official [NetRexx `v5.10-GA` source tree](https://sourceforge.net/p/netrexx/code/ci/v5.10-GA/tree/) at commit `f81cb4b1baeed55b51893d5cbda1c9fa7c50f4f9`. The generated Java retained in the cREXX evidence bundle was also inspected.

### 6.1 Execution representation: Java source plus HotSpot compilation

NetRexx translates the benchmark into ordinary Java control flow and runtime calls. The retained `rexxcps_2_2n.java` contains:

- Java local variables rather than interpreter variable-table accesses;
- Java `for` loops, with count expressions converted once where appropriate;
- direct static Java method calls for benchmark routines;
- direct calls to `Rexx` arithmetic and string methods;
- static-final `Rexx` constants; and
- static-final character arrays encoding parse programs.

The NR-10 measurement ran the normal mixed-mode HotSpot VM, not `-Xint`. HotSpot can therefore compile, inline, specialize, and eliminate temporary objects in the hot graph when its analysis permits it. Exact attribution would require JIT compilation and allocation logs, but the architecture removes the bytecode-language interpreter dispatch and exposes the benchmark directly to a mature optimizing VM.

### 6.2 Compiler specialization and hoisting

The NetRexx compiler's loop generator emits Java loops directly. It hoists stable `TO`, `BY`, and `FOR` values, uses a primitive counter for suitable `FOR` loops, and can emit primitive increment operations for suitable loop variables. Its constant converter lifts reusable values into static-final fields.

Its parse generator compiles each template to a compact static character-array program. Runtime parsing is then a direct call to `RexxParse.parse()` with that plan and a small destination array. This is another independently developed instance of the same precompiled-semantic-plan pattern.

Relevant upstream compiler files include `src/netrexx/lang/NrLoop.nrx`, `NrParse.nrx`, and `RxConverter.nrx`.

### 6.3 Runtime value and built-in implementation

The NetRexx `Rexx` value maintains a character-array representation plus decimal mantissa/exponent/indicator state and lookaside information. Immutable value arrays can be shared across clones. Arithmetic and built-ins are hand-coded Java over these representations; common arithmetic has early exits and specialised equal-exponent paths, while string operations use arrays and `System.arraycopy` where suitable.

The parse runtime is a compact instruction interpreter over the precompiled character array. It still creates/copies result values, so it is not allocation-free. The important point is that template interpretation and high-level control flow are already reduced before the call.

The adapted compound-variable form uses nested node/hash-table access in the runtime. This is not proof that stems are inherently cheap in NetRexx; the source and representation differ from the Classic benchmark.

### 6.4 NetRexx lesson for cREXX

NetRexx's approximately 39x formal lead combines:

- ahead-of-time translation of high-level control flow into Java;
- stable constants and parse plans hoisted out of the repeated path;
- direct, statically visible method calls;
- a compact dual string/decimal representation with hand-coded common paths; and
- dynamic optimization by HotSpot after the benchmark becomes hot.

This is the least directly transferable design. A cREXX JIT or native-code backend would be an architectural programme, not a small optimization. NetRexx is most useful as evidence of the ceiling available when the hot semantic graph is exposed to an optimizing compiler. Its modified 2.2n source also prevents assigning the exact 39x ratio to JIT compilation alone.

## 7. Cross-runtime synthesis

### 7.1 Mechanisms shared by all three faster runtimes

| Mechanism | Regina | ooRexx | NetRexx | cREXX implication |
|---|---|---|---|---|
| Resolve/compile a stable site once | Self-specialising AST nodes | Translated linked objects | AOT Java and static plans | Extend NR-14-style planning only where current profiles show repeated semantic setup |
| Direct variable/call binding | Cached variable boxes and function pointers | Indexed locals and bound call targets | Java locals and direct methods | Measure remaining name/target resolution and frame cost at current HEAD |
| Native/runtime built-ins | Direct C functions | Direct C++ functions | Direct Java runtime methods | Test intrinsic/native common cases for the hottest Level B built-ins, preserving full fallbacks |
| Retained string/numeric forms | Cached numeric alongside string | Integer/number-string/string caches | Character and decimal lookaside state | Count conversions/copies and add representation-preserving paths where proven hot |
| Precompiled parse | Hand-written parsed structures | Trigger arrays | Static char-array programs | Already validated strongly by NR-14 |
| Reused transient storage | Parameter pools and free lists | Reused stacks/buffers and object subpools | JVM allocation/GC, with possible JIT elimination | Reduce frames/temporaries based on measured current allocations, not source intuition |

### 7.2 What the review does not support

The source does **not** support these shortcuts:

- **“Switch to a different opcode dispatcher.”** cREXX already uses computed goto, and ooRexx is faster with virtual object dispatch.
- **“Eliminate all allocation.”** Every reviewed runtime allocates. They make frequent cases cheap, reuse storage, and avoid redundant objects.
- **“Stems alone explain the gap.”** Compound lookup remains nontrivial in all implementations and the NetRexx source is adapted. NR-15 should finish on its own evidence.
- **“Make everything native.”** Native built-ins help only if procedure/call and representation work are a material current cost and exact semantics remain intact.
- **“NetRexx proves a JIT will give 39x.”** Its source is different and the measurement does not isolate JIT effects.
- **“The formal 27x–39x ratios are current after NR-14.”** They are the valid NR-10 baseline, not a refreshed post-NR-14 comparison.

### 7.3 Confidence levels

- **High confidence:** the source-level mechanisms described above exist; NR-14 demonstrates that precompiling stable parse semantics materially improves cREXX RexxCPS.
- **Medium confidence:** repeated semantic setup, library-call boundaries, and representation conversion are larger contributors than raw VM dispatch. This is supported by all three source designs but needs a current cREXX attribution profile.
- **Low confidence without new experiments:** the percentage contribution of any individual mechanism, particularly stem access, built-in calls, allocation, or HotSpot JIT optimization.

## 8. Ranked future cREXX investigations

These are investigation candidates, not implementation instructions. They should be reconciled with the live roadmap and the active NR-15 result before scheduling.

1. **Refresh the post-NR-14 call/procedure/opcode and conversion census.** Establish which timed RexxCPS operations still create general frames, resolve targets, convert representations, or copy strings. Older call censuses are useful history but cannot be assumed current after NR-14 and NR-21.

2. **Extend frozen/site-specific plans to the next proven hot semantic family.** Candidate sites include stable built-in calls, internal calls/argument parsing, and `TRACE`/`ADDRESS` query/update behavior. Each needs a fully semantic fallback and an exact equivalence gate.

3. **Prototype direct common cases for the hottest built-ins.** `LENGTH`, the benchmark's `SUBSTR` form, and `WORD` are the timed built-ins. `LENGTH` already reaches a simple length primitive, so the first measurement should distinguish the helper's semantics from frame/call overhead. Do not optimize result-formatting functions as though they were timed.

4. **Reduce small-helper and internal-call scaffolding.** If current evidence shows frequent frames/copies around very small operations, extend direct-call descriptors, bounded-arity paths, or safe inlining. This should build on NR-21 rather than duplicate it.

5. **Preserve representations through decimal/string loops.** Look for repeated string-to-number/number-to-string conversion, normalization, or result allocation at stable sites. Regina and ooRexx show that a cached dual representation can coexist with Rexx semantics.

6. **Treat quickening or superinstructions as secondary.** Site-specialised bytecode may help once semantic work is reduced, but dispatch-only optimization has a weak causal case relative to the evidence above.

7. **Keep JIT/AOT as an architecture-gate option.** NetRexx shows the possible ceiling, but a cREXX JIT/native backend has a much larger cost and risk envelope than the preceding evidence-driven changes.

## 9. Evidence needed before development decisions

A clean follow-up evidence bundle should contain:

1. a new exact same-host, same-session, canonical 100x100 cross-runtime run at the accepted post-NR-14/post-NR-15 cREXX commit, with at least ten runs per runtime;
2. current cREXX opcode, procedure, call-site, allocation, string-copy, and numeric/string-conversion counts for the canonical workload;
3. controlled source variants that remove one timed family at a time—built-ins, internal call, `TRACE`/`ADDRESS`, compound/stem work, decimal loop work, and parse—to estimate attributable cost without changing the principal benchmark;
4. small native/intrinsic controls for only the proven hot built-ins, checked against exact Classic/Level B output and edge cases;
5. if NetRexx attribution becomes important, a diagnostic `-Xint` contrast and HotSpot compilation/allocation logs, clearly separated from the governed headline comparison; and
6. preservation of the original nominal-clause accounting and provenance markers so later numbers remain interpretable.

## 10. Source manifest

### cREXX evidence reviewed

- `performance/AGENTS.md`
- `performance/ROADMAP.md`
- `performance/README.md`
- `performance/evidence/2026-07-20-nr-10-formal-baseline/scorecard.md`
- `performance/evidence/2026-07-21-nr-14-hybrid-first-release-verdict/`
- `tests/benchmarks/rexxcps_levelb.crexx`
- `tests/benchmarks/cross-runtime/rexxcps/rexxcps_2_2.rex`
- `tests/benchmarks/cross-runtime/rexxcps/rexxcps_2_2n.nrx`
- retained NetRexx generated Java under `performance/evidence/2026-07-15-nr-02-cross-runtime/rexxcps/`
- relevant cREXX Level B library sources under `lib/rxfnsb/rexx/`

### Regina 3.9.7

- Official archive page: <https://sourceforge.net/projects/regina-rexx/files/regina-rexx/3.9.7/>
- SHA-256: `f13701ebd542e74d0fc83b2a7876a812b07d21e43400275ed65b1ac860204bd4`
- Principal files: `regina_t.h`, `interprt.c`, `expr.c`, `variable.c`, `yaccsrc.c`, `funcs.c`, `builtin.c`, `memory.c`

### ooRexx 5.1.0

- Official archive page: <https://sourceforge.net/projects/oorexx/files/oorexx/5.1.0/>
- SHA-256: `6343c667c9839839519b7bbd0e9573c3899acdc9509514fc512d450da5c6a67a`
- Principal areas: `interpreter/execution/`, `interpreter/instructions/`, `interpreter/expression/`, `interpreter/classes/`, and memory-management sources

### NetRexx 5.10-GA

- Official source tree: <https://sourceforge.net/p/netrexx/code/ci/v5.10-GA/tree/>
- Inspected commit: `f81cb4b1baeed55b51893d5cbda1c9fa7c50f4f9`
- Principal files: `src/netrexx/lang/NrLoop.nrx`, `NrParse.nrx`, `RxConverter.nrx`, `Rexx.nrx`, `RexxParse.nrx`, plus the retained generated `rexxcps_2_2n.java`

## Bottom line

The competitor review points toward **semantic-work elimination and stable-site specialization**, not a wholesale change of dispatch machinery. cREXX's own NR-14 result validates that direction. The prudent next step after the active NR-15 work settles is to refresh the canonical cross-runtime baseline and obtain a current per-family cost census, then select the smallest remaining repeated semantic operation for another proof-of-concept with exact equivalence and a first Release verdict.
