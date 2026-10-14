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
