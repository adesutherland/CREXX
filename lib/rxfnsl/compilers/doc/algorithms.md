# Processing algorithms and their contracts

**Proposed for STEP01 review.** This documents the intended algorithms, not
completed proofs or measured performance. Code and decisive tests must accompany
each implemented stage under [LLCOMP-AC15](../PLAN.md#numbered-acceptance-criteria).

## Stages and evidence

| Stage | Algorithm and output | Decisive checks |
|---|---|---|
| Lexing | Generated DFA over UTF-8 input; existing nested-comment stage; exact token spans and retained lexical adjacency. | Malformed UTF-8, CRLF, escaped quotes, nested comments, source origins and keyword-like identifiers. |
| Contextual token classification | Explicit Classic clause/context state, label/assignment lookahead, documented promotions with provenance. | `if = 1`, nearest ELSE, THEN boundary, nested groups and malformed clauses. No silent B AST manufacturing. |
| Parsing | Canonical LR(1), explicit expression precedence and matched/unmatched IF productions. | Build-time conflicts with witnesses; whole-source recognition and located recovery. Measure resulting tables before adoption. |
| AST projection | Existing selected-parse projection into named fields/lists. | Cardinality, one owned parent, independent sources/origins, all damage records. |
| Binding and typing | Iterative source traversal plus scope/routine tables. | Unknown/duplicate declarations, exact conversions, Classic dynamic-name semantics and unsupported routine shapes. |
| IR construction | Explicit block builder and slot/value IDs; iterative structured-statement lowering. | Function/block ownership, joins, loop targets, short-circuit order and failure/return outcomes. |
| Verification | Indexed ownership/type checks plus block order and successor checks; separate slot-initialisation analysis. | Malformed records, cross-block temporaries, missing terminators, wrong signatures and stale references. |
| Local propagation/folding | Forward block scan, temporary/slot fact map, registered exact constant evaluators. | Unknown contexts/calls, overflow, side effects and dead-path runtime errors. |
| Branch/reachability | Replace a proved branch edge selection, then graph reachability over admitted outcomes. | Keep observable condition evaluation; recompute after graph change. |
| Unused-operation removal | Use counts and a worklist restricted to proven discardable operations. | Never remove I/O, trace, possible failure/divergence or unknown effects merely because a result is unused. |
| Target lowering | Registered C/G lowerings into admitted RXAS operations and ABI calls. | All specialised operations handled, exact literals, state/error contracts and generated origins. |
| Register assignment | Initially distinct locations for slots/temporaries, separate contiguous call windows, explicit cleanup. | Live values survive calls and branches; recursion uses the proper frame; target limits diagnosed. Reuse follows a separately checked liveness implementation. |
| Emission | Chunk builder, indentation/sections, stream/file sink and source map. | Exact bytes, Unicode/binary quoting, failed writes and no partial successful publication. |

The frontend-stage choices reuse the supported Level L portfolio. Grammar
construction may expose an admission or resource problem; retain its witness
and review a remedy instead of silently adding an experimental backend.

## First analysis equations

A definitely assigned slot has a value on every path reaching its read. First
compute reachable blocks. For reachable blocks, with slot universe S:

```text
IN(entry) = explicitly initialised incoming slots
IN(block) = intersection of OUT(predecessor) for reachable predecessors
OUT(block) = IN(block) union slots written by the block
```

Initialise non-entry candidates to S and iterate until no set changes. Check
each load using the set at its exact position, adding preceding writes as the
block is scanned. G typed defaults appear as stores and therefore count as
initialisation. Classic source names are pool operations and are not subject
to this slot rule. A normal successor is reached only when preceding operations
return normally. Unsupported nonlocal/resumable transfers prevent a complete
analysis claim until their edge contracts exist.

With B reachable blocks and S slots, the candidate sets can lose at most B*S
members. This supplies a finite termination argument for this specific monotone
analysis. It is not a proof that arbitrary extension callbacks converge.

## Work and storage bounds

Let N be operations, U operand references, B blocks, E edges, V results, S slots
and X total source/constant bytes. These are sizes of an admitted IR, not source
language complexity bounds.

- With indexed IDs and bounded descriptor lookup, structural/order validation
  takes O(N+U+B+E), plus type/constant semantic-validator work. It stores visited
  and ownership indexes proportional to the records. No universal bound is
  claimed for arbitrary registered validators.
- Reachability takes O(B+E) work and O(B) marking/queue space.
- The slot fixed point is finite. With straightforward bit sets, a deliberately
  loose scheduling bound is O((B+E)*(B*S+1)*ceil(S/w)), plus the O(N+U) statement
  checks, where w is the bit-set word size. Record the actual schedule and
  tighten this bound against the implementation; do not quote it as measured
  compile time.
- The first local pass scans each operation once. Descriptor, map and evaluator
  costs are additional. Every evaluator receives a work budget and must report
  exhaustion. A budget bounds the attempt, not semantic equivalence.
- Candidate packed operation headers use 48*N bytes, value records 16*V and
  operand/edge ID arrays 4*(U+E). Add block/function/slot tables, indexes, schema,
  sparse facts, buffers and X. Retain the complete measurement, including live
  old/new snapshots during a rewrite.

Parser/table construction and regex longest-match costs require their own
grammar-specific admission/resource accounts. No blanket linear-time promise
for the entire compiler follows from a linear IR traversal.

## Transformation obligations

The first pass schedule is bounded: one local propagation/folding sweep, branch
simplification, reachability recomputation and discard-safe removal. Repeated
whole-program fixed-point optimisation is not required for the PoC.

Each rewrite records the input profile, relied-on facts, replacement mapping,
preserved source events and its validity argument. For an integer fold, the
argument includes exact result and absence of overflow. For Classic arithmetic
it includes the numeric context and language-specific conversion/error rules.
For branch simplification it includes the known Boolean outcome and retained
condition effects. Equivalence is stated for admitted executions with adequate
resources; resource exhaustion is separately tested and diagnosed.

`pure` is not enough to remove an operation: it may still fail or diverge.
Missing effect fields are unknown. Callback declarations are semantic premises,
supported by independent tests/proofs, not facts proved by their syntax.

Candidate outputs are verified before atomic publication. Compare new execution
against independent expected outcomes, each language's existing compiler, and
a small cREXX IR evaluator for the admitted operation set. Keep compiler and
assembler optimisations separately controllable. The evaluator is a test tool;
it is not an implementation shortcut for source compilation or INTERPRET.

## Emitter surface for the first review

The proposed basic Level B API is a chunk writer with `write(text)`,
`line(text)`, `indent()`, `dedent()`, `section()` and `append(section)`.
`finish()` returns a checked output object that can stream to a sink. Advanced
operations attach source origins and expose copying/work counters. Underflowing
indentation, unknown sections, budget exhaustion and sink errors are explicit
failures. RXAS-specific quoting and register names belong to the backend.

The first implementation may use an append-only chunk list rather than a
balanced rope if it only needs append and section concatenation. Benchmark the
actual output workload before adding tree balancing. Emit N bytes with bounded
chunk overhead and one final materialisation or streaming traversal; do not
repeatedly concatenate the entire accumulated string.
