# STEP05 proposal: clean construction, then shared IR optimisation

15 October 2026. **Approved design; cleanup verdict accepted; shared passes
implemented and locally qualified.** See the [receipt](../evidence/step05-20261015/README.md). The original proposal below records
the design rationale; the [worked example](../examples/step05/README.md) now
contains actual cleaned and optimised IR/RXAS. Adrian clarified that formal
performance testing is not required for this research project; root `AGENTS.md`
records the scoped exception. No separate formal Release verdict is pending.
Current source baseline: `afcde7f6c2b2dcce6a8a6e89705457aafcac8344`.
[PLAN.md](../PLAN.md) owns acceptance and approval. Compiler, semantic callbacks,
passes and backend remain Level B. RXAS continues through the Level L emitter.
There is no dependency on a native compiler optimisation service or G host.

## Recommended order

First repair the direct construction/emission defects left by the
[quality review](EMISSION-QUALITY-REVIEW.md), with IR optimisation disabled:

1. Construct a default operand only when needed. `SAY expression` and Classic
   assignment currently create an unused empty value first. Preserve bare SAY's
   empty line and every required declared-variable default.
2. Size scratch, diagnostic state and call windows from actual selected needs,
   including hidden adapter calls, argument-count registers and cold handlers.
   Remove the blanket eight-argument reservation from call-free G bodies and
   size the main wrapper correctly. Keep stable separate registers for values;
   this is not liveness-based reuse. Preserve the public register-limit contract.
3. Omit a jump whose destination is the next emitted block. Preserve labels,
   source anchors and joins; do not reorder evaluation or erase needed cleanup.

Keep Classic activation/pool/numeric packets, argument ownership, adapter calls
and the origin-preserving fatal handler. Removing those needs a separate
semantic/use analysis. Real clause grouping and full TRACE/INTERPRET provenance
remain open; they are not bundled into this cleanup.

Then insert one shared optimisation pipeline between verified source IR and
RXAS selection. Generic algorithms belong in Level L; C/G rules belong in the
compiler's semantic extension classes. The initial ordered passes are:

| Pass | Initial proof boundary |
|---|---|
| Temporary propagation | Substitute immutable values and block-local compiler-private temporary slots with a unique known definition and no escape/alias. Preserve left-to-right evaluation and invalidate facts at unknown state/call boundaries. No Classic user-pool constant propagation. |
| Constant evaluation | Ask a registered evaluator for exact typed bytes. Fold only a certified successful result; `keep` preserves runtime behaviour. Never execute a source program or its arbitrary calls in the compiler. |
| Branch simplification | A proven Boolean selects one successor. Delete the other edge, then blocks unreachable from all admitted entry points. Keep condition evaluation if it has observable effects, may fail or diverge. |
| Safe unused-operation removal | Remove unused constants and specifically proven discardable operations. Keep reachable output, stores, calls, possible conditions, divergence, cleanup and unknown effects. An unreachable SAY can disappear; a reachable SAY cannot. |

Use indexed numeric IDs and one checked graph reconstruction per pass/batch,
not a complete graph clone for every individual fold. Keep source graphs
immutable, remap uses and origins, create fresh revision-bound facts, and verify
each candidate before accepting it. Iterate only while a pass makes a monotone
reduction (eligible computations, temporary accesses, edges or unused records),
with explicit work/storage limits. No unbounded fixed point or silent partial
successful output on `invalid`/`limit`.

## Level L improvements driven by these examples

The compilers are demanding consumers of Level L. This increment must leave
reusable library capabilities, not private C/G copies of generic algorithms.
Most of the graph work below completes an intentionally small foundation;
it is not evidence that the existing one-operation API is defective.

| Layer | Proposed work and boundary |
|---|---|
| Level L graph transformation | Extend the existing rebuild coordinator to apply a checked batch of substitutions, edge changes and removals. Keep the single-operation `rewrite` API working. Maintain old-to-new operation/value/block maps, exact attributes and derived origins; verify before publishing. No graph-table copying in either frontend. |
| Level L analysis and pass driver | Build revision-owned numeric indexes for definitions, uses and successors needed by these passes. Share eligibility checks using the existing state/failure/termination contracts. Invalidate analyses on graph replacement and bound pass work/storage. Keep the sparse application-facts API; do not redesign it or adopt SSA for this increment. |
| Level L source positions | Extract reusable byte-offset-to-line/Unicode-column lookup from the currently duplicated diagnostics/backend scans. A request-owned line index supports repeated lookups without rescanning the whole prefix. Reuse it in Level L diagnostics and C/G source emission, preserving their separate presentation policies. |
| Level L writer | Retain the existing chunked writer, sections, origin tags, byte limits and sink contract. Check provenance after graph remapping, section/error propagation and output limits through the new pipeline. No new assembly framework or writer replacement is justified by this review. |
| Compiler consumers | Own C/G evaluators, frontend default construction, RXAS instruction selection, call windows, fall-through and `.srcstep` policy. The generic writer cannot safely remove repeated instructions or source records: their control-flow meaning belongs to the backend. |

The source-position surface should stay small: one source-index object, a
`position(byte_offset)` lookup, and a result exposing line, column and the
containing line's byte range. Byte offsets remain authoritative; columns count
Unicode scalars as existing diagnostics do. Retain CRLF/CR/LF, end-of-input,
multiline and malformed-input diagnostics. Invalid UTF-8 retains the existing
byte-oriented fallback; it must not be decoded just to report an error.
Index ownership is per source/request, with explicit resource bounds and no
global cache. This is coordinate reuse, not a new TRACE/condition architecture.

The generic acceptance fixture must exercise a small non-C/G operation family
with no compiler imports: folding, branch removal, retained effects, malformed
rewrites and limits. Pair it with the actual C/G examples. Source lookup also
needs independent Level L diagnostics tests. Thus a successful compiler demo
proves a useful library service, rather than being its only consumer or test.

## Small semantic scope first

The foundation's `crexx.g.iadd` and `itext` evaluators are currently demo
contracts. Source compilation emits names such as `crexx.g.int64.add` and
`crexx.g.convert.int64.text`; these need explicit evaluator registration.
Running the existing one-operation rewrite alone will not optimise the source
compiler's actual operation families.

Initial G admission: checked int64 addition/subtraction/multiplication and
comparisons, exact int64-to-text, and canonical 0/1 Boolean conversion/truth.
Keep overflow, division and uncertain conversions at runtime. No float/decimal
reassociation, NaN/signed-zero shortcuts or changed overflow policy.

Initial C admission: `crexx.c.truth` for the exact literal byte values `0` and
`1`, producing a core Boolean. Other spellings and values remain runtime work
until their own contracts are tested. This is independent of NUMERIC state.
Contextual C arithmetic, pool reads, mutable numeric settings, general BIFs and
calls are deliberately not folded by this first whitelist. In particular,
Classic `2+3` does not inherit G's int64 rule. Both languages still benefit from
the same branch, reachability, propagation and unused-operation algorithms.

## Proposed small control surface

After qualification, normal C/G commands enable the shared passes; `-n` keeps
the cleaned unoptimised IR path. The cleanup applies in both modes.

```sh
llrxc-g    -i /path/to/bin -o demo.rxas demo-g.crexx
llrxc-g -n -i /path/to/bin -o baseline.rxas demo-g.crexx
# The same switch applies to llrxc-c.
```

This `-n` controls the new compiler's IR passes. The native B seed's `rxc -n`
controls how the compiler implementation is built; `rxas -n` controls assembler
optimisation. Keep all three independent in tests and documentation.

For the B service, append an optional `optimise=1` argument after the existing
request arguments, preserving their positions. Keep `request.graph()` as the
frontend graph and add `request.emissiongraph()` for the graph selected for
emission (the same graph when disabled). No new configuration object or pass
selection flags are needed initially.

Proposed generic entry: `llirservices..optimise(reference graph,budget)`, using
the existing change-result shape (`changed`, `keep`, `invalid`, `limit`, verified
result and old-to-new ID mappings). Extend the change result with `block(oldid)`
for removed/remapped blocks; zero denotes removal. Existing operation/value
mapping behaviour is retained. It selects no semantics by a language name;
registered contracts supply them. The entry and request option are now implemented;
qualification is tracked in the plan.

## One small source example for both languages

Use `options levelc` or `options levelg`, followed by:

```rexx
if 1 then say 'yes'
else say 'no'
```

The [complete current C/G inputs and outputs](../examples/step05/README.md)
now contain generated before/after output; all four modules execute `yes` with
`rxas -n`. The snippets below retain the original design sketches for review
history. Their registers are illustrative; repeated metadata and unchanged
setup/return/handler regions are omitted. Use the complete files for actual
current output and counts.

### G IR

Current shape, with original result numbers and abbreviated origins/types:

```text
entry:
  %1 = core.constant 1 : crexx.g.int64
  %2 = crexx.g.convert.int64.boolean %1
  %3 = crexx.g.truth %2
  core.branch %3 -> yes -> no
yes:
  %4 = core.constant "" : crexx.g.text      # construction defect
  %5 = core.constant "yes" : crexx.g.text
  crexx.g.say %5
  core.jump -> join
no:
  %6 = core.constant "" : crexx.g.text      # construction defect
  %7 = core.constant "no" : crexx.g.text
  crexx.g.say %7
  core.jump -> join
join:
  %8 = core.constant 0 : crexx.g.int64
  core.return %8
```

Cleanup removes `%4` and `%6` before optimisation. Evaluators then prove the
condition true; the shared passes select the `yes` edge, remove unreachable
`no` and unused condition values. A conservative result can keep the three
reachable blocks; fall-through selection emits no jumps between them:

```text
entry: core.jump -> yes
yes:
  %value = core.constant "yes" : crexx.g.text
  crexx.g.say %value
  core.jump -> join
join:
  %status = core.constant 0 : crexx.g.int64
  core.return %status
```

### G RXAS

Current body excerpt (`.srcstep` records omitted for readability):

```text
load r1,1
copy r2,r1
itob r2
icopy r3,r2
brt llb2,r3
br llb3
llb2:
load r4,""
load r5,"yes"
say r5
br llb4
llb3:
load r6,""
load r7,"no"
say r7
br llb4
llb4:
load r8,0
ret r8
```

Proposed body, after cleanup and the shared passes:

```text
llb1:
llb2:
load r1,"yes"
say r1
llb4:
load r2,0
ret r2
```

Wrapper, metadata, source records and required numeric setup remain outside
this excerpt. It is not a proposed complete four-instruction module.

### C IR and RXAS

C's current condition has a different semantic operation:

```text
%4 = core.constant 31 : crexx.c.value       # exact bytes for "1"
%5 = crexx.c.truth %4
core.branch %5 -> yes -> no
```

The arms contain C-value constants (`796573` = `yes`, `6E6F` = `no`),
`crexx.c.say` and the same jumps. The same unused empty defaults appear.
Entry pool/argument/numeric setup and `crexx.c.return.absent` are retained.
Proposed C body IR has the same surviving control shape as G, but keeps
`crexx.c.value`, `crexx.c.say` and the C return contract. A certified C truth
evaluator supplies the core Boolean; the shared optimiser does not reinterpret
all C values as G integers.

The current RXAS evaluates the condition through the adapter:

```text
load r17,"1"
load r16,1
settp r17,768
call r7,llctarget.literal(),r16
null r17
copy r17,r7
load r16,1
settp r17,768
call r8,llctarget.truth(),r16
null r17
brt llb2,r8
br llb3
```

After the proposed fold, this entire condition sequence and the unreachable
`no` arm disappear. The reachable output still uses the Classic adapter:

```text
load r17,"yes"
load r16,1
settp r17,768
call r10,llctarget.literal(),r16
null r17
copy r17,r10
load r16,1
settp r17,768
call r14,llctarget.sayvalue(),r16
null r17
```

This excerpt deliberately retains the original register numbering to show
which calls survive; right-sizing will renumber them. Source/context updates
still surround these calls. Classic activation, return packets, argument
clearing and origin-preserving error handling are not removed. The baseline
cleanup separately removes the pointless `literal("")` call before SAY.

For a second folding example, G `say 2 + 3` should become a text constant `"5"`
and SAY, with no add/conversion instruction. C's contextual addition stays
runtime work in the initial profile. This is an intentional semantic boundary,
not a separate optimisation algorithm for each frontend.

## Verification and implementation gate

PLAN.md F5-AC01–08 and F5-STEP01–04 define the checkable proposal. Required
counterexamples include reached and skipped failures, effects before a known
branch, loops and joins, recursion/call windows, wrong/stale facts, limits,
Classic literal `01`, NUMERIC changes, unset names, G overflow and short-circuit
evaluation. Test original/optimised IR with an independent bounded reference
evaluator and execute both RXAS forms with assembler optimisation disabled first.
Do not reuse the folding callback as the expected-result oracle.

Retain input graphs and expression origins; folded values use derived origins.
Reachable effect/error sites keep their original anchors and the current fatal
relay. The admitted profile has no full clause/TRACE implementation. Unknown
trace/condition semantics must block transformations, not be silently ignored.

Complete the focused correctness, resource-limit, delivery and code-quality
checks. Adrian's 15 October clarification removes formal performance testing
and the separate Release verdict gate for this research project. Static
instruction/call/frame counts are not runtime speed evidence.
Broader liveness allocation, inlining, Classic numeric specialisation, context
elision, full tracing and INTERPRET remain later work.
