# Attached AST mappings and one-pass reconstruction

This note records the contracts implemented by `lib/rxfnsl/rexx/llast.crexx`
and the mapping checks in `llfront.crexx`. The delivery scope and acceptance
criteria belong to [PLAN.md](PLAN.md); the application-facing API belongs to
[the user guide](../../lib/rxfnsl/doc/user-guide.md#application-trees-with-then-tree).

## Capture counts at build time

For each field selector, the frontend computes lower and upper bounds on the
number of matching captures within its enclosing rule. Counts use the finite
abstract domain `{0, 1, 2+}`, where `2+` means two or more. Addition saturates at
`2+`. Called named rules are boundaries; a caller cannot borrow a callee's field.

- A matching capture contributes exactly one occurrence; other leaves contribute
  zero. A capture occurrence and its projected child count are separate facts.
- Sequence adds bounds. Alternatives take the minimum lower bound and maximum
  upper bound. This also safely overapproximates ordered choice: an unreachable
  alternative can make the bound less precise but cannot hide an extra match.
- Repetition has lower bound zero and upper bound `2+` when its body can capture.
  A nonempty repetition retains its body's lower bound. Optionality has lower
  bound zero and retains its body's upper bound.
- `one` requires lower = upper = 1; `optional` requires upper <= 1; `some`
  requires lower >= 1. `many` permits every count. Upper = 0 diagnoses an unknown
  selector rather than silently supplying an empty field.

An explicit postorder stack and memoized completed nodes evaluate each local
expression graph once per selector and direction. Shared repetition bodies do
not cause exponential re-evaluation. Recursion through named language rules
cannot create an analysis cycle because this analysis stops at those boundaries.
It is a structural bound, not a decision procedure for grammar equivalence or
for the reachability of every alternative.

The inferred output schema records kind, ordered field names, text/child mode
and cardinality. Mappings of the same kind must agree on those contracts. A
successful selected capture must project to exactly one application child;
construction checks this additional obligation against the actual parse.

## Projection and ownership

Raw derivation children precede their parents. Projection validates the raw
input, marks the selected root's reachable nodes and processes them in increasing
ID order. Rejected parser alternatives cannot construct application nodes.
Unmapped subtrees retain a saturated output count and, for a single output, its
ID. They do not copy lists of all descendant outputs at every wrapper.

Operator fields use the projected frontier in source order, independent of the
physical wrappers used by Pratt, LR or Earley. Mapped parents choose the fields
that they retain. Discarded expression groups extend the retained child's
original range and mark it composite. The source bytes themselves are immutable.

An owning tree contains its own arena and original source. A node handle has
an arena reference, a snapshot serial and an integer ID; a walk has a borrowed arena reference
and cursor arrays. They do not contain an owning arena copy. Owners must outlive
their handles. A request can be replaced after constructing its owning AST.
The VM's reference validity check rejects expired owners; the arena's immutable
serial detects replacement of a live owner variable. Serials are module-local
and refuse exhaustion before signed-integer wrap. Arena identity rejects foreign
replacement children. Copying the owning tree remains a value copy.

## Termination and resource bounds

Let N be reachable AST nodes, F their field slots, E their child links, R raw
parse nodes and G the finite schema size.

1. Every stored child ID is smaller than its parent ID. Therefore the stored
   graph is acyclic, with depth at most the number of allocated nodes.
2. Final validation rejects a second visit to a reachable node. Thus the result
   is a rooted tree rather than a DAG, and E = N - 1 for a nonempty result.
3. A walk's cursor advances past each field and edge once. It takes
   `O(N + F + E)` work and `O(depth)` cursor storage. The tree stays borrowed.
4. A rewrite walks the original tree once, cloning each original node with its
   already-rewritten children. New nodes are not scheduled. For fixed G and
   bounded application work per visit, reconstruction takes `O(N + E)` work.
   There is no automatic retry, fixpoint or replacement revisit.
5. `tree.entries = L` bounds allocated AST nodes + field slots + child links,
   including discarded intermediate replacements. The input parse and named
   view have their own admission/work/entry checks. Projection's bookkeeping
   also uses `O(R)` entries; L is not a byte count for total process memory.

The storage limit bounds library allocation, not an arbitrary application's
own loop. The named view and projection also inspect grammar/source structure;
the walk's linear bound must not be presented as a bound on all parsing or
AST-construction work for every grammar.

## Rewrite transaction and meaning

A rewrite owns a separate output arena. The input is read-only. A draft is
restricted to its current step, and accepting it seals its fields. Only a
complete traversal followed by successful schema/tree validation publishes an
owning result. An invalid field, foreign child, stale draft, shared child,
incomplete pass or exhausted budget publishes no root. Finishing twice is an
error. Published results retain the original source and replacement origins.

These properties prove termination of the library's finite traversal and the
structure/ownership contract. They do not prove the meaning of an arbitrary
application transformation. The calculator's constant fold is a concrete
application: after child values have been computed, replace each pure operator
with a Number containing that result. Equivalence additionally depends on using
the same decimal operations and a value-preserving decimal spelling conversion.
The retained regression corpus checks precedence, associativity, prefix minus,
decimal values and original-source failures; it is not a proof of arbitrary
numeric codecs or effectful rewrites.

## Acyclic analysis and invalidation

`tree.analyse(limit)` schedules the same finite children-first walk. It accepts
one initialized application object per visited node. Reading an unrecorded
result fails with `LL_AST_DEPENDENCY`; a missing result, duplicate result or
incomplete finish publishes no usable table. Thus every admitted dependency
points to an earlier completed result. This proves the schedule acyclic; it
does not discover arbitrary application dependencies or solve cyclic equations.

A table borrows its owning arena and records the arena's immutable snapshot
serial. Both must match at lookup. Distinct tree copies have distinct owner
references; rewritten contents have a new serial even when assigned into the
same variable. Facts are therefore invalidated by replacement without trying
to infer which application properties may still be true. Node handles, walks
and child cursors use the same check. Rewrite completion checks its input's
identity too. Result objects are returned by value.

For N visited nodes, the scheduler fills N result records. Its arrays use arena
IDs, so their capacity is bounded by A, the number of allocated arena nodes,
plus the walk's O(depth) cursors. A can exceed N when an earlier rewrite left
unreachable intermediate nodes; the tree's entry allowance already bounds A.
Each reachable node is scheduled once. The analysis node allowance bounds filled
records/visits, not array capacity, application result size or callback work.
The `each(field)` cursor follows each child link once; repeated indexed lookup
is not claimed linear. The RexxScript client uses cursors for list operations.
Its conservative read-name sets may require more than linear total storage;
they are application facts, not a parser complexity certificate.

## RexxScript folding profile

Recognition retains the original language rules. Identical capture wrappers
with the same target and name are interned, avoiding duplicate labelled LR
reductions in shared prefixes. This does not merge captures with different
names or alter PEG choice ordering.

Named AST fields replace positional expression, assignment, SAY and IF access.
The enclosing loop algorithms are preserved; only their tree access changes.
Validation precedes folding. Runtime branch choice, short-circuiting, variable
lookups and BIFs remain in execution, not a whole-tree evaluation walk.

Analysis reports constants, conservative reads and potential effects. Literal
conversion and scalar operations are shared with execution. It never invokes a
BIF or inspects the variable pool. A failing calculation stays unknown, allowing
its runtime error to remain conditional on reaching that expression. Folding
uses the common checked single traversal and replaces only supported pure
expressions with a literal whose decoding preserves the scalar type and value.
Float round trips compare the native bits, including signed zero. Integer and
decimal comparisons retain their respective numeric domains. Unsupported or
non-round-trippable values remain expressions.

Each replacement retains the full original range and a checked diagnostic
anchor within that range. The analysis carries the last operand actually
visited under short-circuit rules; the replacement uses that anchor. This
preserves downstream error locations when an operand was folded. A pass cannot
set an anchor outside its replacement's original range.

The structural schedule and identity contracts are independent of numeric
semantics. The shared-operation and exact-round-trip conditions describe the
admitted folding argument; differential tests cover the current RexxScript
implementation, not arbitrary new effects, numeric codecs or callbacks.
Equivalence assumes sufficient resource allowances. Folding can reduce runtime
visits, so execution fuel bounds work rather than prescribing an identical
number of ticks for the two trees.

## Qualification

[First AST evidence](evidence/ast-20261013/README.md) and
[analysis/RexxScript evidence](evidence/ast-step04-20261013/README.md) record the exact source/tool
identities, backend comparisons, runtime ownership/budget tests, bootstrap
agreement and installed examples. Wider cyclic-analysis scheduling, synthetic/recovered
origins and compiler migration criteria remain in the authoritative plan.

## Sparse application properties

`llannotation.contract` contains each field name, scalar type and literal default
once. A store contains K explicit `(node ID, property ID, value)` entries. There
is no node-count by property-count array. Default reads do not allocate entries;
an explicit value equal to the default still occupies one entry. The initial
lookup uses a linear scan of K entries: O(K) lookup/update, O(K + P) records for
P declarations, plus stored text. This is a sparse representation, not a claim
of constant-time associative lookup. Reads and replacement writes do not grow K.

A store borrows an AST root handle, whose arena reference and snapshot serial
check every read/write. A copied tree has a different owner reference. A rewrite
has a new snapshot serial. Replacing the same owning variable with different
contents therefore invalidates its properties. Restoring the identical snapshot
value into the same owning variable preserves the identity/content pair; this
contract is about snapshot contents and owner identity, not assignment counting.

Finishing seals a draft and returns a value-owned table. Its failure state is
sticky; no getter can expose partial values after a failed write/finish. A node
view borrows the table, captures its group serial and checks both that serial
and the AST binding. Rejecting a stale view does not poison the replacement
group it happened to reference. Default values are immutable scalar strings/integers;
there are no shared mutable collection defaults. Generated source and RXAS
facades call the same checked runtime, with identical public type signatures.
RXAS receiver argument windows use the compiler's swap/copyback pattern so an
ERROR signal can restore the caller's mapping before a handler runs.

## Checked structured flow

`llflow.mapping` validates every AST kind and every named execution field
against the inferred schema. Assignments have a text target and exactly one
value child. Branches have a required condition and true child, with a required
or optional false child. Sequence/evaluate consume their selected children in
field/list order. Every child has one execution role; unused text metadata is
inert. Read/value leaves cannot conceal child computations.

The graph builder walks children first. Each AST node receives an entry event;
its children's completed flow fragments are connected as specified. Assignment
adds its write after the value fragment. A branch adds a decision after its
condition and a join after both arms; an absent false arm supplies a direct
false edge to the join. A sequence connects each fragment once in order.
The mapping contains no callback and the builder performs no fixed-point search.
An N-node tree creates O(N) events and edges and uses O(A + N) indexed records,
where A is arena capacity (already bounded by the AST entry budget). Source
handles remain borrowed, including each branch edge's originating AST node.

The structured source profile generates acyclic flow. It does not yet expose
loops, returns, exceptions, call effects, bindings or short-circuit semantics.
A public construction mutation outside the builder is refused; failed graphs
publish no start point, dimensions or readable events. Owner identities are
captured on construction, including before the first query. Graph/result serials prevent an old
analysis or point view from being reused after replacement, even for another
graph on the same AST. This increment does not close the wider cyclic-language
mapping and grouping/lowering acceptance criteria.

## Definite assignment domain and proof

Let B be flow events, E edges, and V the finite set of variable spellings found
in read/write events or supplied at entry. Names are exact and case-sensitive.
Let `U[b]` be the set of variables that may be unassigned just before event b.
Reachability is tracked separately: an empty set at an unvisited point must not
be confused with proof that all names are assigned.

For entry names I:

- Entry starts reachable with `U[entry] = V \ I`.
- For a write to x, `T_b(U) = U \ {x}`. Other events use `T_b(U) = U`.
- For every reachable edge `b -> c`, add `T_b(U[b])` to `U[c]` and mark c reachable.
- A read of x is unsafe exactly when x is in U at that reachable read event.

The implementation starts other sets empty and schedules a point only when
reachability or its incoming set grows. Union and removal of a *fixed* name are
monotone under subset inclusion. Thus every bit changes at most once, from 0 to
1, and each point changes from unreachable to reachable at most once. There are
at most B*V bit additions and B reachability additions. A deduplicated worklist
therefore terminates. Removing x in a transfer never removes incoming facts,
so it does not invalidate this argument.

For a finite graph with these operations the equations characterize existence
of a path reaching a point without an intervening write. Equivalently, a name
is definitely assigned when no such path exists. This follows by induction on
path length: entry supplies exactly the initially unassigned names; identity
preserves a witness, writing x eliminates its witness, and union retains any
incoming witness. Each transfer distributes over union, so the least fixed
point equals the union of path results. This proves the analysis relative to
its supplied execution graph. It does not establish that an incorrect mapping
matches the target language, or that a syntactically possible branch is feasible
under the language's value semantics.

The state matrix contains `B * max(1,V)` bytes, checked by division before
multiplication. The extra B-byte case covers reachability bookkeeping when V=0.
The counted cell allowance is not the total resident memory: names, adjacency,
source handles and worklist records are separate. Adjacency permits outgoing
edge iteration without rescanning the entire graph. Each event is scheduled at
most V+1 times; the main propagation costs O((B+E)*(V+1)*max(1,V)) in the
worst case, plus bounded name lookup/collection work. Linear name lookup can
cost O((B+I)*V) while collecting names. The work allowance counts these name
comparisons, propagated edge visits and variable tests; it interrupts the
calculation independently of the mathematical convergence bound. A failed
calculation publishes no point answers.

For a reported read, witness construction searches from entry while refusing
to cross any write to that name. A found predecessor chain is consequently an
actual graph path reaching the read unassigned. Its branch edges provide the
explanation. The search visits each reachable event/edge at most once and has
its own edge allowance; message construction and source-line scanning depend
on path/source length. Witness exhaustion does not rewrite the completed
analysis. The tests also enumerate all four paths through two branches for each
of 16 combinations of writes, providing an independent result control.
