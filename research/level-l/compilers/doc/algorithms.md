# Processing algorithms and their contracts

The foundation algorithms below are implemented in `llir` and `llirservices`.
The source frontends and RXAS selection are implemented. Production IR
optimisation remains planned. The [backend algorithm](backend.md#structures-and-algorithm)
describes stable registers, checked call windows, literals, source mapping and atomic output.
Each implemented stage carries its contract and decisive tests under [LLCOMP-AC15](../PLAN.md#numbered-acceptance-criteria).

## Stages and evidence

| Stage | Algorithm and output | Decisive checks |
|---|---|---|
| Lexing | Generated DFA over UTF-8 input; existing nested-comment stage; exact token spans and retained lexical adjacency. | Malformed UTF-8, CRLF, escaped quotes, nested comments, source origins and keyword-like identifiers. |
| Contextual token classification | Explicit Classic clause/context state, label/assignment lookahead, documented promotions with provenance. | `if = 1`, nearest ELSE, THEN boundary, nested groups and malformed clauses. No silent B AST manufacturing. |
| Parsing | Canonical LR(1), explicit expression precedence and matched/unmatched IF productions. | Build-time conflicts with witnesses; whole-source recognition and located failure; these grammars do not request recovery. |
| AST projection | Existing selected-parse projection into named fields/lists. | Cardinality, one owned parent, independent sources/origins, all damage records. |
| Binding and typing | Iterative source traversal plus scope/routine tables. | Unknown/duplicate declarations, exact conversions, Classic dynamic-name semantics and unsupported routine shapes. |
| IR construction | Explicit block builder and slot/value IDs; bounded recursive structured-statement lowering. | Function/block ownership, joins, loop targets, short-circuit order and failure/return outcomes. |
| Verification | Indexed ownership/type checks plus block order and successor checks; separate slot-initialisation analysis. | Malformed records, cross-block temporaries, missing terminators, wrong signatures and stale references. |
| Local propagation/folding | Forward block scan, temporary/slot fact map, registered exact constant evaluators. | Unknown contexts/calls, overflow, side effects and dead-path runtime errors. |
| Branch/reachability | Replace a proved branch edge selection, then graph reachability over admitted outcomes. | Keep observable condition evaluation; recompute after graph change. |
| Unused-operation removal | Use counts and a worklist restricted to proven discardable operations. | Never remove I/O, trace, possible failure/divergence or unknown effects merely because a result is unused. |
| Target lowering | Explicit C/G selectors over verified blocks, producing checked RXAS instructions and ABI calls. | All specialised operations handled, exact literals, state/error contracts and generated origins. |
| Register assignment | Initially distinct locations for slots/temporaries, separate contiguous call windows, explicit cleanup. | Live values survive calls and branches; recursion uses the proper frame; target limits diagnosed. Reuse follows a separately checked liveness implementation. |
| Emission | Chunk builder, indentation/sections, stream/file sink and source map. | Exact bytes, Unicode/binary quoting, failed writes and no partial successful publication. |

The frontend-stage choices reuse the supported Level L portfolio. Grammar
construction may expose an admission or resource problem; retain its witness
and review a remedy instead of silently adding an experimental backend.

## Frontend processing and limits

1. Validate UTF-8 and mask nested comments through Level L, then run the generated
   DFA. The original bytes remain owned by the request. Lexical failure stops
   before syntax or semantic processing.
2. Classify names with clause, parenthesis and IF/THEN state. Assignment/label
   lookahead protects contextual names. For C, combine a string and its adjacent
   single-symbol X/B suffix into one original-source token before inserting any
   concatenation. Longer/spaced symbols remain separate. Classify before separator normalization:
   a variable named THEN must not swallow the following instruction. Normalize
   separators adjoining actual THEN/ELSE, and physical newlines inside parentheses.
   Insert zero-width blank/abuttal tokens from retained token adjacency, excluding
   function-name parentheses and the CALL target/arguments boundary.
3. Run the generated canonical LR(1) parser and `then tree` projection. The grammar
   owns precedence and nearest-IF ELSE association. Expected-token messages are
   translated to source spellings, then formatted with the exact byte location,
   Unicode column, escaped excerpt and caret. No repaired input is compiled.
4. Walk the AST iteratively to check the nesting limit before recursive semantic
   visitors. Inventory routines, exports and explicit imports; collect G bindings; repeatedly infer implicit
   types until stable. The type order only widens; the pass guard is `5*bindings+1`.
   Node-visit work accounting also covers expression type queries. Unknown names,
   unresolved types and duplicate declarations fail before a graph is published.
5. Register only observed target/presence call signatures and construct the graph. G slots receive defaults
   and parameter stores at entry. C functions receive argument and numeric packets
   and create a private pool. A loop stack resolves LEAVE/ITERATE. Each condition
   produces a core Boolean branch. Values crossing newly created blocks are
   stored in temporary slots and reloaded; block-local handles never cross joins.
   C ordinal literals validate grouped digits and use B `b2x`/`x2c` at compilation
   to construct exact Unicode text constants, without runtime conversion. Original
   spelling/ranges remain in the AST. Validation and decoding scan the bounded
   literal; malformed digits/groups fail before publishing successful IR.
6. Evaluate call arguments in source order into private slots/packets before
   building the final operation. C omission bits and result presence are explicit. G masks contain one 0/1
   byte per formal, and inputs only for supplied positions. Every required
   position is checked, including those after optional formals.
   Read selected external signatures from bounded RXBIN007 data; never execute
   import metadata. Freeze and verify the complete module, then publish it.

The request defaults to a 1,000,000 source-byte/semantic-visit budget and 512 AST
nesting levels, both caller-configurable. Grammar wrappers count toward nesting.
Layout lookahead is capped at sixteen times the request budget. Existing Level L
scan/parse policies additionally bound generator/runtime work. IR construction
uses the request entry budget and 64 times that budget for bytes. Metadata reads
admit at most 16 MiB for B and 64 MiB for the current Classic runtime, including
expanded sections. These are explicit protection limits, not performance promises.

Flat name/binding tables make lookups linear; repeated inference and duplicate
checks can be quadratic. A long separator run can also require quadratic
lookahead, capped by its work limit. The source tree uses the generic iterative
walker, while semantic lowering is recursive under the prior depth check. The
first implementation favours visible invariants over an unmeasured indexing
optimisation. An exhausted budget returns a diagnostic and no successful graph.

The metadata reader checks magic/version, reserved fields, directories, alignment,
section ranges, expansion budgets, LZSS backreferences, sorted pool-local IDs and
selected record sizes/references. It resolves strings with binary searches,
requires a callable export, and retains exact callable/type/argument/member and
autoload fields. Unsupported required signatures are rejected. Unused executable,
source and inline-template records are opaque; this is not a full replacement
for the production metadata conformance reader.

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

## STEP07A contract processing

Each observed G call resolves a local or selected imported signature before
schema freeze. Cache keys include target and presence pattern. The operand
order is ascending formal position and source order; missing positions never
create a default expression in the caller. Signature matching is linear in
formal count after the existing linear symbol lookup. The number of call
schemas is bounded by observed source calls and schema/IR budgets; there is no
power-set expansion. Duplicate selected metadata records are ambiguous, even
if they otherwise have matching types. Whole-literal validation distinguishes
legacy defaults from type names and never evaluates metadata as source code.

Per-function emission scans uses to defer only a sole-use pure constant (or
primitive decimal default) feeding an optional read. The constant is emitted
inside that read's missing branch. A multi-use or nonliteral fallback retains
its original eager evaluation. Presence reads do not modify argument registers;
private-slot assignments therefore cannot change presence, including recursion.
Shared reverse demand can remove unused hasarg/optional-read operations, while
conservative call effects prevent propagation through unknown callee state.
Export validation precedes target text publication. These scans use bounded
IR tables; repeated per-function scans are not a claim of globally linear
compile complexity or an optimised register allocator.
