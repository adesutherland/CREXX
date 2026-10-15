# Processing algorithms and their contracts

The foundation algorithms below are implemented in `llir` and `llirservices`.
Frontend parsing, production optimisation and RXAS selection remain planned.
Each implemented stage carries its contract and decisive tests under [LLCOMP-AC15](../PLAN.md#numbered-acceptance-criteria).

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

## Foundation algorithms and bounds

Let N be operations, U operand references, B blocks, E edges, V results, S slots,
F functions, D schema fields and X source/constant bytes. These describe admitted
IR sizes, not source-language complexity. `ir-design.md` specifies record widths
and the accounting boundary of `bytes()` and `capacity()`.

- Construction appends fixed-width rows and exact byte-pool entries. Table
  capacity doubles; row-major growth copies a buffer, while the comparison
  column layout relocates each field column. IDs are checked unsigned 32-bit
  fields. Host entry and byte budgets are checked before each append.
- Verification walks each block's operation chain with a visited index. It
  checks operand definition order and block ownership, result definitions,
  terminators, signatures, ranges and successor ownership. Names are checked for
  duplicates by comparisons with earlier records. Descriptor fields are looked
  up by scanning the registered field table. Thus this first implementation has
  quadratic name-check terms and descriptor-lookup costs; it does not claim a
  universally linear validator. Constant codecs and semantic callbacks add their
  own work. Origin parents must precede children, preventing provenance cycles.
- Reachability uses a queue and follows both successors of a Boolean branch.
  Its graph traversal is O(B+E). It does not inspect literal truth to prune an
  edge. Complete analysis requires registered, normally returning extensions
  with known state effects.
- Definite assignment keeps two B*S byte matrices, rather than packed bit sets.
  Non-entry reachable blocks start with their function's slots set. Each sweep
  intersects predecessor outputs, then scans block operations to add stores.
  The implementation finds predecessors by scanning the graph, giving a loose
  bound O((B*S+1)*(B*S*(B+E)+N)) before descriptor and fact-publication costs.
  The sets can lose at most B*S members, so the sweep terminates. A work budget
  limits attempted scans; a preliminary lattice-size check bounds matrix
  allocation. Loads are diagnosed at their position in the converged block.
- Facts are sparse ordered arrays keyed by entity, ID and name. Lookup is a
  linear scan; storing M distinct facts can take O(M²) comparisons. Fact payload
  and key bytes have independent budgets. These foundations do not yet claim
  an indexed large-program fact store.
- Evaluation receives literal operands, exact operation facts and a work budget.
  The coordinator checks status, result count, every result codec and declared
  consumed facts. A service must respect its budget; arbitrary trusted callback
  execution is not preempted by this interface.
- Rebuilding traverses records, copies origins/constants, remaps temporary uses
  and verifies a fresh candidate. Single-operation folding is a foundation API,
  so repeated calls rebuild repeatedly. The later production pass should batch
  work. Both source and candidate are live during a rebuild. Target lowering
  currently admits only pure straight-line fragments with checked results.
- The emitter appends bounded chunks. `finish()` publishes a complete output;
  flattening allocates once and copies chunks linearly. Streaming walks chunks
  and stops on sink failure. Section append copies chunk descriptors/payloads
  according to Level B value ownership; no balanced rope or zero-copy claim is
  made. File publication needs a temporary file and atomic rename above this API.

The table-layout screen is explicitly non-representative. It compares the same
construction, field scan and rewrite on row and column layouts and checks exact
checksums. It does not measure complete compiler latency, VM allocation overhead
or peak process memory. Representative measurements remain LLCOMP-AC13.

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

## Implemented emitter surface

The basic Level B API is a chunk writer with `write(text)`,
`line(text)`, `indent()`, `dedent()`, `section()` and `append(section)`.
`finish()` returns a checked output object that can stream to a sink. Callers can attach source-origin IDs and inspect output byte/chunk counts. Underflowing
indentation, unknown sections, budget exhaustion and sink errors are explicit
failures. RXAS-specific quoting and register names belong to the backend.

The implementation uses an append-only chunk list for append and section
concatenation. Benchmark the
actual output workload before adding tree balancing. Emit N bytes with bounded
chunk overhead and one final materialisation or streaming traversal; do not
repeatedly concatenate the entire accumulated string.
