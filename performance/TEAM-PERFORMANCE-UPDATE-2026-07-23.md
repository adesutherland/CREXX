# cREXX performance programme — team update

Date: 2026-07-23
Audience: cREXX contributors and technical stakeholders

## Executive summary

The initial performance sweep has moved cREXX from a position where the
RexxCPS result exposed an approximately **45.7x gap** to ooRexx into one where
ooRexx is a credible, bounded target.

At the start of the programme, cREXX produced about **0.89 million RexxCPS
clauses per second**, roughly **2.2%** of the quoted 40.8 million ooRexx result.
The latest accepted cREXX checkpoint is **28.120M** on `rxvm` and **26.119M** on
`rxbvm`. Against the most recent retained ooRexx result of 39.921M, that is
**70.4%** and **65.4%** respectively—roughly a thirtyfold improvement from the
original cREXX result.

Across the five workloads that currently have qualified, equal-work cREXX and
ooRexx forms, cREXX's overall geometric-mean throughput is now approximately
**86.8% (`rxvm`) and 82.0% (`rxbvm`) of ooRexx**. Two workloads are already
decisive cREXX wins: Sieve is about 7.1x/5.4x ooRexx and Permute about
2.1x/2.0x. Base64 is at about 71–79% of ooRexx. Bounce and Richards remain the
large individual deficits and will be central to the next phase.

The accurate team headline is therefore:

> cREXX has progressed from about 2% of ooRexx on RexxCPS to 65–70%, while the
> qualified common benchmark portfolio now averages 82–87% of ooRexx. cREXX
> already wins two common workloads by large margins, but the aggregate should
> not be described as every individual benchmark being within 80%.

The July 23 cREXX results and July 20 competitor results are from different
sessions, so these are planning comparisons rather than a new governed
same-session baseline. Refreshing that baseline is the first activity in the
new roadmap.

## What changed

The gains did not come from one magic instruction. They came from repeatedly
removing work that a typed, register-based Rexx implementation should not need
to redo inside a hot loop.

### 1. We made the evidence trustworthy

The programme first established a correctness-gated benchmark portfolio,
same-work cross-runtime comparisons, exact artifact hashes, repeatable
profiling, allocation/call censuses and formal noise/regression rules. This was
essential: it stopped us optimizing invalid comparisons or mistaking profiler
time for product performance.

The initial register closed with **21 completed activities**, one deliberately
rejected experiment, one deferred item and six architecture questions
transferred into the successor roadmap.

### 2. We moved stable semantic work out of repeated execution

Several of the largest improvements follow one rule: if a decision is stable,
make it once and retain the answer.

- **Frozen PARSE plans (NR-14):** a stable Rexx `PARSE` template is now compiled
  into an execution plan instead of being interpreted repeatedly. Focused parse
  time fell by more than 90%, and RexxCPS improved about 45% in both VMs.
- **TRACE and ADDRESS stable-state paths (NR-16):** inactive TRACE now avoids
  the general tracing machinery, and an unchanged ADDRESS environment avoids
  repeated environment selection. The corrected general fast paths moved
  RexxCPS from roughly 6M to 26–28M clauses/s—a 4.5–4.7x increase—while
  preserving the full dynamic behavior when state changes.
- **Direct call binding (NR-17):** stable procedure operands are bound to
  process-local runtime targets once, rather than rediscovered on every call.
  Late loading and invalidation remain supported.

This is the same broad pattern seen in faster Rexx implementations, but cREXX
can apply it cleanly at compiler, assembler, linker, loader or VM level.

### 3. We improved core data and call paths

- **Typed numeric Level B functions:** the benchmark and library now retain
  honest integer, float and decimal behavior instead of crossing through
  general decimal/string operations unnecessarily.
- **Stem default/reset and access (NR-15):** the accepted hybrid representation
  made focused get-hit work about 75–77% faster, the independent histogram
  about 32% faster and RexxCPS about 10.9% faster. A *stem* is Rexx's keyed
  compound-variable structure; the new design makes its stable/default cases
  much cheaper without dropping generation, copy or reset semantics.
- **Fixed small-call forms (NR-21):** `CALL1` through `CALL4` reduce argument
  movement for the common arities, improving List by about 6% and Permute by
  3–4% while shrinking images.

### 4. We taught the compiler and assembler to remove more work

- **Typed flow analysis (NR-26):** the compiler now reasons about which values
  reach a point, which assignments are live and which initializations or
  copies are unnecessary.
- **Whole-procedure RXAS flow (NR-27):** the assembler performs similar
  machine-level reasoning over complete control flow, including authored RXAS.
- **RXAS rule and instruction work (NR-09/18):** selected multi-operand
  operations and safe rewrite rules reduce dispatches and temporary registers.
  Crucially, neutral or harmful forms were withdrawn instead of being kept for
  opcode-count vanity.
- **Definite initialization/lifetime work (NR-08):** unnecessary reference
  lifetime operations were removed, improving RexxCPS about 4–6%.

These changes also built the prerequisites for the next step: cREXX now has
typed compiler flow, machine flow, arbitrary-operand RXAS, fixed calls, direct
link/load binding and a private per-process execution image.

## Terms used in the next phase

- **VM:** the virtual machine that executes linked cREXX bytecode. `rxvm` uses
  computed-goto dispatch; `rxbvm` uses portable switch dispatch.
- **RXAS / RXBIN:** RXAS is cREXX's assembly language; RXBIN is the portable
  linked bytecode image executed by the VMs.
- **BIF:** built-in function, such as `LENGTH`, `SUBSTR` or `WORD`. Core Level B
  BIFs are written in cREXX and can often be inlined.
- **Inlining:** replacing a small function call with its body. The new work is
  not simply “inline more”; it removes the argument, result, branch and
  temporary scaffolding left after inlining.
- **Flow analysis:** compiler/assembler proof about where values come from,
  where they are used and which operations are redundant.
- **Quickening:** a private VM optimization in which a frequently executed
  site remembers a stable type, target or representation and takes a guarded
  direct path next time. Canonical RXBIN remains portable and unchanged, and a
  complete fallback handles changed or exceptional state.

## Next phase

The aim is no longer marginal parity. The roadmap defines unquestionable
superiority as the default VM reaching at least **1.5x ooRexx on every qualified
workload**, at least **2.0x on the common-workload geometric mean**, and at least
**1.5x on the disclosed RexxCPS comparison**. The alternate VM must also beat
ooRexx on every qualified cell. This must survive the supported platform and
compiler matrix without benchmark-specific shortcuts.

The work will proceed in this order:

1. **Refresh the evidence (PERF2-01).** Profile all 11 workloads under both VMs,
   rerun the qualified ooRexx/NetRexx comparisons in the same session, and
   produce a cost and gap dossier for every workload. This prevents the next
   implementation from being selected from pre-NR-15/16/17 profiles.
2. **Semantic quickening (PERF2-02).** Compare eager, first-hit and guarded
   runtime specialization against the best compiler/RXAS static form. Preserve
   TRACE, signals, late load, profiling and both VM modes.
3. **Inlining 2.0 and core BIFs (PERF2-03/04).** Use the new flow analysis to
   make inlined code approach hand-written lowering. Start with profile-ranked
   BIFs; add only a few general RXAS/VM assists when cleaned source cannot reach
   the machine ceiling.
4. **VM and representation work (PERF2-05/06/07).** Measure execution-image
   density, dispatch/fetch behavior, calls/frames, copies, conversions,
   retained string/numeric forms and allocation lifetime before selecting a
   redesign.
5. **Close capability and comparison gaps (PERF2-08/09).** Bring currently
   non-comparable JSON, Storage, Towers, List and Mandelbrot cells to an honest
   disposition, with Level B/G language decisions kept separate and explicit.

## Recommended model ability

- **PERF2-01 evidence refresh:** Very High is appropriate. It is extensive and
  precision-sensitive, but the architecture is already specified.
- **PERF2-02 quickening architecture and first PoC:** use Ultra. This work spans
  persistent execution-image state, compiler-versus-VM ownership,
  invalidation/dequickening, computed-goto label ownership, TRACE/debugging,
  signal ordering, late loading, profiling identity and both VMs. Missing one
  invariant could produce a fast but subtly incorrect architecture.
- **Later bounded quickening slices:** Very High should be sufficient once the
  architecture, site-state machine, invariants and first accepted reference
  implementation are frozen.

The strategic opportunity is now credible: cREXX can combine decisive speed
with a flexible, typed, multi-stage architecture. The next phase must turn that
architectural advantage into consistent per-workload superiority, so cREXX is
the obvious platform for future Rexx evolution rather than merely a faster
implementation on selected tests.

Primary evidence: [successor roadmap](ROADMAP.md),
[July 23 accepted checkpoint](evidence/2026-07-23-nr-16-17-closeout/README.md)
and [cross-runtime RexxCPS source review](rexxcps-runtime-source-review-2026-07-22.md).
