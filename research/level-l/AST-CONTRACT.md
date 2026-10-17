# Attached AST mappings and one-pass reconstruction

This note records the contracts implemented by `lib/rxfnsl/rexx/llast.crexx`
and the mapping checks in `llfront.crexx`. The delivery scope and acceptance
criteria belong to [PLAN.md](PLAN.md); the application-facing API belongs to
[the user guide](../../langtools/doc/user-guide.md#application-trees-with-then-tree).

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

## Generated and recovered origins

Every arena range satisfies `0 <= start <= offset <= finish <= source bytes`.
Origin tags are restricted to exact, composite, derived, synthetic and recovered.
Synthetic nodes require `start = finish`: their explicit source anchor supplies
a diagnostic position, while their spelling comes solely from their fields.
An empty source permits only anchor zero. Derived replacements retain the
original node's range and anchor. Neither operation edits the original bytes.

A recovery record retains a skipped interval [current plan](PLAN.md) and
[current plan](PLAN.md) record the exact source/tool
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

The graph builder makes two iterative parents-first walks. The first allocates
entry and normal-exit ports and records lexical loop owners. Short-circuit
nodes also receive separate true/false ports. The second connects the ports;
parents establish each loop's continue target before visiting its descendants.
The AST's checked one-parent invariant makes each child have exactly one use.
No mapping callback or fixed-point search runs during graph construction.

The construction preserves the following path invariants, inductively over the
AST structure (loop edges connect the completed finite fragments):

- Assignment's write follows normal completion of its value. Sequence/evaluate
  connect normal completions in field/list order.
- Branch tests follow condition evaluation. True and false normal completions
  join; an absent false arm goes directly to that join.
- WHILE entry goes to the condition, true to body, false to exit, and body
  completion/CONTINUE to condition. UNTIL entry goes to body, body completion/
  CONTINUE to condition, true to exit and false to body. Unconditional loops
  enter/repeat their body, with no implicit exit edge.
- BREAK goes directly to the nearest enclosing loop exit. CONTINUE goes to its
  recorded test/body entry. RETURN evaluates its optional child then stops.
  Their normal-exit ports have no incoming paths, so syntactic statements after
  them cannot create a false normal completion. Unreachable jump placement is
  still validated. Return's boundary is the analysed root, not a call graph.
- AND routes left-true to right, left-false to false; OR routes left-false to
  right, left-true to true. Right outcomes are preserved. Tested short-circuit
  nodes use separate outcome paths, never a merge followed by an independent
  truth choice. Ordinary value use joins those ports. An opaque expression
  has two possible outcomes after evaluation; no literal values are assumed.

Thus graph paths model the declared structured execution, including finite
prefixes of loops, under nondeterministic opaque tests. This is exact for that
abstract execution contract, not for correlations or constant values omitted
by the contract. Generic `evaluate` does not forward a child's Boolean value.
Exceptions, call effects and variable bindings need additional contracts.

For N reachable AST nodes the construction creates O(N) events and edges and
uses O(A + N) indexed records, where A is the bounded AST arena capacity. Each
node has at most four initial ports, and each test adds at most two decisions.
The two walks and connections are finite, irrespective of graph cycles; field
and mapping lookups add their schema-dependent search costs. The flow entry
budget counts every event and edge, including disconnected normal-exit ports.

Public construction mutations outside the builder are refused; failed graphs
publish no start point, dimensions or readable events. Owner identities are
captured before the first query. Graph/result serials invalidate replaced
analyses and point views even on the same AST. The wider grouping/lowering,
origin and production-migration criteria remain open.

## Optional condition facts

A conditions table belongs to one immutable AST identity. A fact `(n,b,r)`
contains a node, a Boolean outcome and a nonempty explanation. Its semantic
premise is: whenever n completes normally, its truth under the target language
is b. The premise must hold on every dynamic visit, including loop iterations.
Checking the explanation's presence does not establish this premise.

Construction seals the borrowed table and records its identity. Missing facts
are unknown. Contradictions, invalid values, foreign or expired AST nodes,
expired/replaced tables and attempted post-seal mutation invalidate dependent
graphs and analyses. The allowance counts unique stored facts, including facts
never used as a test. Indexed storage uses O(A) slots for bounded AST capacity A;
the count allowance is not a claim about total resident bytes.

For a known ordinary test the builder retains evaluation and removes only the
edge for the opposite outcome. For a short-circuit test it retains distinct
outcome ports and removes the opposite outgoing edge; ordinary value use joins
only allowed outcomes. Induction over execution fragments shows that every
concrete execution satisfying the facts still has a corresponding graph path.
Consequently definite-assignment answers remain sound under the declared flow
semantics and supplied premises. No evaluation, read or write is eliminated
merely because the completed result is known. A fact for a compound expression
does not propagate constraints backwards through its operands: some impossible
prefixes can remain and produce conservative diagnostics. This is outcome
filtering, not a general path-condition solver.

Pruning edges cannot increase graph dimensions or the finite solver bound
below. Tests separately control literal truth rules, unknown conditions,
effectful tests, while/until exits, nested short-circuiting and witness paths.

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
therefore terminates, including on cyclic graphs. Removing x in a transfer never removes incoming facts,
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
analysis. `reachable()` exposes the separately tracked entry reachability;
`assigned()` returns false at an unreachable point rather than presenting
vacuous truth as a usable fact. Both require a completed, current owner.
The tests enumerate all four paths through two branches for each of 16 write
configurations, and independently enumerate 16 loop-arm write/break/continue
configurations. The latter admits an exit only from a breaking arm and requires
all such arms to write; a backedge cannot repair an unsafe first-iteration exit.
Additional controls test nested targets, post-test continue, returning branches,
short-circuit truth correlations, diagnostic paths and exact budget boundaries.
