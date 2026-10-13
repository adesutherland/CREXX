# What lexing first can prove

This analysis separates grammar equivalence, operation counts, live storage and
elapsed time. A faster recognizer for a changed language is not an optimization
of the original parser.

## Equivalence at token boundaries

Let an input byte string `s` have length `n`. Let a deterministic lexer produce
`m` tokens with kinds and original-source half-open spans `(kind, start, end)`.
Let `b[j]` identify the source position corresponding to token boundary `j`.
Trivia may need its own channel: discarding it is an explicit grammar decision.
If trivia is skipped, first prove a grammar normalization that absorbs exactly
that trivia into lexical units, including leading and trailing trivia. Define
`b` at those logical unit boundaries; a token's visible spelling can occupy only
part of its unit. The invariant below applies to that normalized grammar.

A sufficient condition for replacing scannerless grammar `Gc` by token grammar
`Gt` is the following boundary factoring contract:

1. Every lexical atom of `Gc` consumes exactly one complete lexer token at a
   permitted boundary, with the same kind, spelling/payload and end boundary.
2. All structural compositions, rule calls and lookaheads occur at permitted
   boundaries. No competing alternative needs to split a token, inspect its
   interior as syntax, or choose a different tokenization in context.
3. Lexical priority, case policy, keyword policy and trivia requirements agree.
   Rejecting malformed strings/comments belongs to the lexical contract;
   unterminated input must never be silently accepted as skipped trivia.
4. Actions/AST leaves consume retained source spans and lexemes, preserving
   required structure and values. Removing lexical nodes from an AST needs an
   explicit projection, not an assertion that the trees are identical.

Under these conditions the recognition invariant is

`Gc(rule, b[j]) = b[k]  iff  Gt(rule, j) = k`.

Proof for a finite, non-left-recursive PEG: establish the invariant for lexical
atoms; sequence composes equal endpoints; ordered choice sees the same success
or failure for its alternatives in the same order; repetition advances through
the same boundaries; permitted lookahead observes the same recognition without
consuming. Induction over these constructions proves the root invariant.

For left-recursive extensions, also prove that the chosen seed/update order and
match-comparison policy commute with `b`. An algorithm that violates its PEG
contract cannot acquire that contract simply by tokenizing its input. This
extra obligation is material for the tested Pika reference.

The native five-case timing panel verifies recognition equivalence on those
fixtures, not on every string in either specification. The token grammar's
public lexer recognizes `//` comments that the paired character grammar omits;
that is deliberately outside the panel's equivalence domain. A production
comparison must close that difference and test entire supported language
families, ASTs, diagnostics and malformed input before adopting it.

## Executed counterexamples

`S <- ('a' / 'ab') 'c'` rejects `abc`: the first alternative consumes `a`,
then `c` fails at `b`; PEG does not retry `ab` after that enclosing failure.
A maximal-munch lexer producing `AB, C`, followed by `S <- AB C`, accepts it.
The Java reference and native generator both demonstrate the PEG rejection.

The native `priority.lex` produces hard keyword `IF` for `if`, but identifier
`IDENT` for the longer `iff`. A statement grammar requiring an identifier
accepts `if=1;` with the ordinary identifier lexer and rejects it with the hard
keyword lexer. Keyword spelling should remain available for contextual parser
decisions. Rexx whitespace/abuttal concatenation is another reason a generic
trivia-deleting lexer is insufficient.

## Cost and break-even

For an algorithm with bounded per-position work, define:

- `Cc`: structural/lexical clause cost per source position;
- `Ct`: structural clause cost per token position;
- `Cl`: lexer plus token-record construction cost per source byte;
- `D`: extra mapping/preparation cost for this input.

Then an approximate, explicitly conditional cost model is

`Tchar = Cc*n`, `Ttoken = Cl*n + Ct*m + D`.

Lexing wins exactly when `Cc > Cl + Ct*(m/n) + D/n` in this model. With average
token/trivia compression `rho = n/m`, the estimated speedup is

`S = Cc / (Cl + Ct/rho + D/n)`.

The lexer sets a ceiling: as `rho` grows, `S` tends to `Cc/Cl` rather than
infinity. Tiny inputs can be dominated by preparation or process startup;
comparing a re2c subprocess with an in-VM scanner must include that cost.
The recorded native token timings include DFA lexing and symbol conversion.
They exclude source compilation, image startup and AST construction in both
cells. Those are separate lifecycle/feature obligations.

For memo tables with `Mc` character clauses and `Mt` token clauses, logical
position slots fall from `Mc*(n+1)` to `Mt*(m+1)`. Add token/span storage and
the retained original input to the token side. This is a slot count, not a
measurement of VM RSS; Rexx array slots contain more than a raw machine integer.
Sparse entries and all reachable parse-tree objects must also be counted.

In the long-identifier native fixture, `n=519`, `m=8`, `Mc=95`, `Mt=34`:

`49,400 / 306 = 161.44` times fewer dense memo slots.

Observed clause evaluations fall from `295,165` to `1,088` (271.29 times),
including the removal of lexical clauses and spurious interior matches.
These are deterministic counts from this reduced engine; its elapsed ratio is
not a prediction for the Java reference or an optimized production parser.

## Pika's tested quadratic family

Pinned upstream: `8050ceaf70c79a04416fdbdc374e92f9478de7b0`.

Use `A <- A 'a' / 'a'` on `a^n`. A bottom-up parse seeds every start position.
At start `i`, the longest match grows through the remaining `n-i` characters.
Each growth constructs new match structure rather than charging that work once
to a globally shared token. Summing growth over all starts gives

`sum(k=1..n) k = n(n+1)/2 = Theta(n^2)`.

The unmodified reference produces exactly `n(n+1)` created/memoized match
objects for the measured lengths. An identity traversal from every retained
memo entry finds **all** of these objects still reachable. For `n=256`, that is
65,792 objects although there are only 767 clause/position memo entries.
Thus both construction work and reachable match-object count are quadratic for
this family. Counting memo keys alone would miss the storage problem.

This disproves an unconditional linear time/space claim for the pinned
implementation. It does not prove every Pika grammar is slow, nor disprove a
different implementation with different sharing/scheduling. The
[paper](https://arxiv.org/html/2005.06444) discusses lex preprocessing in section
6.3 and already warns about recognition changes. Its grammar/distribution
assumptions cannot replace a worst-case bound for the implemented algorithm.

The dense native recognizer makes an even narrower claim: a cell's match length
only increases through a finite integer range. Total updates are bounded by
`O(M*n^2)`; rescanning `M` clauses after each possible change gives a conservative
`O(M^2*n^2)` time bound and `O(M*n)` span-table storage. It retains no AST versions.
This is a correctness/explanation control, not the proposed production scheduler.
Across repeated calls, this prototype's module arrays retain their past
high-water allocation. The storage bound then uses the largest input parsed,
not just the latest input; the reported cells are logical current-parse slots.
An owned/resettable production parse context must make that lifecycle explicit.

## DFA scanning is not automatically linear either

For rules `a*b` followed by `a`, and input `a^n` with no `b`, maximal munch
must inspect the remaining suffix before falling back to the single `a` rule.
Repeating that fallback inspects

`n + (n-1) + ... + 1 = n(n+1)/2` bytes.

The generated native DFA gives 36, 136, 300 and 528 transitions for lengths
8, 16, 24 and 32. It has no regex backtracking engine; the quadratic mechanism
is token-boundary rollback. A linear lexer cost `Cl*n` therefore requires
bounded rollback, a proof of amortized rescanning, or a scanner construction
that avoids repeated unsuccessful suffix work.

The ordinary benchmark specification has accepting identifier/number prefixes,
single-character punctuation, and string/comment starts whose failed bodies
terminate scanning instead of falling back to many short tokens. The fixtures
exercise bounded work (for example 526 transitions for 519 source bytes).
That observation is a starting point for a rule-by-rule proof, not a proof for
arbitrary user regex specifications.

Full DFA subset construction can be exponential in regex/NFA size. Alphabet
equivalence classes reduce the table constant; they do not change that worst
case. The research generator stops at a 2,048-state budget rather than promising
unbounded determinization.

## Optimization directions justified by these results

1. Pre-tokenize unique lexical atoms and preserve source/trivia maps. This
   targets both dense table positions and match creation inside opaque regions.
2. Shield strings/comments first when a full boundary equivalence proof is
   unavailable. Do not shield interpolation or nested language syntax blindly.
3. Dispatch terminal seeds by token kind; schedule only dependent parent clauses,
   with a proved ordering and nullable policy. Benchmark grammar preparation too.
4. Keep persistent/shareable AST nodes or build the selected AST after recognition.
   Repetition suffix sharing alone does not fix left-recursive prefix histories.
5. Handle PEG priority and recursive growth explicitly; grouping an operator
   choice fixes the mixed-operator fixture, not every cyclic grammar.
6. Keep diagnostic explanation and edit recovery separate from recognition.
   An island after a gap does not prove a minimum-edit repair or the author's
   intended syntax.

Each direction needs its own semantic and complexity proof before production
selection. The newer Squirrel paper's version-tagged approach is worth an
independent adversarial test; a per-position `<= n` growth bound alone does not
justify summing it to `O(n)` over all positions.

## Expanded proof ledger: backend admission and cost

These are arguments about the declared algorithms and data structures, not
machine-checked proofs of the compiler/VM. `g` below includes normalized RHS
slots; `q` is DFA states, `c` alphabet classes, `n` input bytes and `t` input
symbols. Constant/amortized indexed-array operations are an explicit RAM-model
assumption. Generation, input materialization, error recovery and tree
traversal must be charged separately. Executed checks challenge the arguments;
finite test success does not establish a universal theorem.

### LEX-02 — Reverse DFA longest-match theorem

**Theorem, under stated assumptions.** Let a deterministic finite automaton
have a partial transition function `delta`, an initial state `q0`, and a
preselected winning token kind `A(q)` at each accepting state. That kind
already embodies rule priority. There are no context changes, trailing-context
captures, semantic callbacks or empty tokens in this admitted model.

For input `x[0:n]`, define `F(q,i)` as failure or the pair `(j,kind)` for the
longest accepting prefix starting at position `i` in state `q`. Failure is
smaller than every success. Set

```
base(q,i) = (i,A(q)) when q accepts; failure otherwise
F(q,n)   = base(q,n)
F(q,i)   = max_by_endpoint(base(q,i), F(delta(q,x[i]),i+1))
```

An absent transition contributes failure. A transition consumes exactly one
byte, so a successful successor has endpoint at least `i+1`; there is no tie
between it and `base(q,i)`. Rule-priority ties were settled in `A(q)` during
DFA construction. Backward induction on `n-i` proves the recurrence: every
accepting path either consumes nothing or takes the unique next transition.
Its longest endpoint is therefore exactly the maximum above.

A tokenization starting at `i` uses `F(q0,i)`, emits or skips that token, then
continues at its endpoint. Induction on token count proves equality with
ordinary maximal munch, including the first unmatched offset and partial
token prefix. Rejecting an accepting initial state ensures strict progress.

`llmunch.scanner` computes two columns of all state results and retains only
`F(q0,i)` for each start. It performs at most `q*n` transition queries, uses
`O(q+n)` additional DP entries and emits at most `n` nonempty tokens. Including
table validation gives `O(q*c + q*n + n + output)` under the RAM assumptions.
It requires the whole buffer; this is not a streaming theorem. It is a simple
reverse-tabulation variant, not a claim to reproduce Reps's exact construction.
[Reps's primary account](https://research.cs.wisc.edu/wpis/abstracts/toplas98b.abs.html)
establishes the broader possibility of linear maximal munch.

**Executed counterexample/control.** On `a*b` followed by fallback `a`, the
unchanged generated four-state DFA uses `n(n+1)/2` transitions in the ordinary
scanner and `4*n` in reverse mode. At `n=128` these are 8,256 and 512. The cases
also compare tokens and spans with generated controls, all 128 low-byte probes,
malformed strings, priority, skipped trivia, explicit high-byte rejection in
the legacy profile and a generic DFA consuming NUL. These are operation counts,
not speed or representative product-performance measurements. Full reverse
preparation can cost more on easy inputs; end-to-end timing remains open.

### PEG-02 — Non-left-recursive normalized packrat bound

**Conditional theorem.** If left-corner analysis excludes cycles, every
recursive call either follows an acyclic zero-consumption dependency or moves
to a greater input position. With pure constant-time terminals and one memo
cell per normalized clause/position, there are at most `g*(t+1)` cells. Every
cell's body makes a bounded number of memo calls. The repetition implementation
must memoize its *tail*: an iterative star that scans the whole remaining
suffix separately at every position does not meet this premise.

The `llpeg` packrat mode uses a memoized repetition tail, making recognition
and its binary history construction `O(g*(t+1)+a)` under these conditions.
Long recursion can reach the explicitly reported stack/fuel limit; a resource
failure is not a rejection of the input language. This theorem does not certify
quality of expected-token messages or unrestricted semantic actions.

### CFG-02 — Indexed Earley and packed derivations

**Conservative implementation argument.** An Earley item is identified by a
production, dot, origin and end position. There are `O(g*(t+1)^2)` distinct
items. Direct indexed membership prevents reprocessing the same item. A
prediction scans at most `g` productions. A completed symbol scans at most
`O(g*(t+1))` waiting items in its origin chart. Thus the simple control has
`O(g^2*(t+1)^3)` work, including grammar factors deliberately retained here.

Intermediate forest nodes identify a dotted production and interval. A
packed family beneath such a node is uniquely identified by its pivot: the
left intermediate and right symbol intervals then determine its children.
Symbol-node alternatives are identified by production. The direct family
index preserves these alternatives without enumerating trees. The implemented
dense membership structures have a conservative space bound
`O(g*(t+1)^2*max(g,t+1))`; claiming ordinary quadratic Earley space for this
particular representation would be wrong. Construction/storage budgets remain
explicit. Input-size admission uses division before multiplication to avoid
integer overflow while checking the dense-index bound.

Nullable completions must work in both orders: completion advances existing
waiters, and a newly processed waiter consults an already completed zero-width
symbol. Later alternatives mutate shared forest nodes and therefore do not
require replaying all parents. Reachable unit/nullable cycles remain explicit
in the forest. Counting reports a cycle separately from a finite saturated
count or traversal-resource failure.

**Executed controls.** `S -> S S | a` yields 2 derivations for `aaa` and 14
for `aaaaa`, the corresponding Catalan counts. `S -> A A; A -> a | empty`
retains both placements on `a` and one on empty input. `A -> A | empty`
recognizes empty input with a finite cyclic forest representing infinitely
many derivations. These results do not yet satisfy the independent general-CFG
oracle and recovery obligations of AC-15/20.

### LR-02 — Table-dependent admission

**Conditional theorem.** Canonical LR(1) and merged LALR table construction
are finite decision procedures for the particular normalized grammar. A
conflict-free table admits deterministic parsing; the table construction may
itself be large and is separately budgeted. A LALR conflict does not prove the
language ambiguous or incapable of deterministic parsing.

The native control constructs canonical tables first, then optionally merges
identical LR(0) cores. A four-context fixture succeeds canonically and has an
A/B reduction conflict after merging. A separate `S -> S S | a` fixture has
an LR(1) conflict. Earley's two complete derivations on `aaa` establish the
latter ambiguity; a table conflict by itself would not establish it.

### REC-02 — Squirrel expansion is a separate cost variable

**Counterexample to an unconditional linear claim.** For the pinned core,
`S <- &E x S / x; E <- E x / x` grows E at each suffix start. The successful
expansion total is `sum(k, k=1..n) = n(n+1)/2`. Native and Dart counters agree
at n=4,8,16,32 (10,36,136,528 expansions). Linear many memo keys therefore do
not prove linear total work. The current upstream manuscript acknowledges
this family; the older general publicity must not override the newer analysis.

The native core matches the reference's success/failure trees on the bounded
corpus. It stores every history allocation in an arena until request disposal;
Dart can collect attempts not reachable from retained results. Their allocation
and live-memory claims must consequently be assessed separately. Disjoint
expansion conditions, exact recursive semantics outside the corpus and c14's
pruning/optimality arguments remain open.

**Independent CFG control.** Bison 3.8.2 canonical LR and IELR confirm the
four-context acceptance and Bison LALR confirms the reduction conflicts.
A bounded GLR **set-of-tree-identities** oracle produces the same 1,1,2,5,14
counts for concatenation lengths 1..5. An earlier scalar additive `%merge`
returned 16 at length 5; its source/output remain retained. Therefore arbitrary
semantic merge callbacks cannot be assumed to count distinct derivations.
The short-input oracle enumerates tree identities and is not a scalability
control. The native packed forest is the scalable representation being tested.

### LEX-03 — A sufficient bounded-backup certificate

**Theorem, sufficient but not necessary.** Form the subgraph of nonaccepting
DFA states reachable immediately after some accepting state, following only
nonaccepting states thereafter. If this graph is acyclic, let B be its number
of vertices. After its most recent accepting prefix, a scan traverses at most
B such states before either accepting again, failing on the next byte, or
reaching EOF. Its backup therefore has a grammar-dependent constant bound.
Charge consumed prefixes to the emitted/skipped token and at most B+1 extra
inspections to each boundary. A final unmatched scan occurs only once. Total
scanning is linear in input for the fixed DFA, with the B-dependent constant.

`llmunch.automaton.backupbound` computes a conservative reachable graph and
cycle test for at most 128 states. A detected cycle returns **not proved**, not
“quadratic”: some such automata can still have a linear amortized tokenization.
A larger graph returns the analysis construction limit. Reverse mode needs
neither this condition nor this cycle analysis. The certificate binds the full
DFA table/map/acceptance identity and backend mode, preserving the distinction
between grammar support and a proved bound for one implementation.

### PIKA-02 — Faithful operational port and two implementation boundaries

**Executed correspondence, not a universal semantic proof.** `llpika` consumes
the reference's explicit ranked, interned normal-form graph. The Java exporter
retains n-ary clauses and reads raw match children/selected-choice indices,
rather than flattening away differences. The native evaluator independently
computes the specialised zero flags and seed dependencies, preserves duplicate
priority events, uses strictly longer memo replacement, performs top-down
negative lookup and creates the reference's empty placeholders. All 508 cases
agree in optimized/no-opt builds on both VMs, including raw selected trees,
match/update/entry counts and every reference clause family in the byte domain.
Reference normalization of precedence is tested as input to the evaluator;
this does not yet establish a native precedence/AST-label lowering.

**Counterexample (assertion placeholder).** With `S <- 'x' ^ 'a'`, input `xa`
has a stored two-character match in both implementations. `Start.match(1)`
fails, but `Start.canMatchZeroChars` is true, so a subsequent lookup returns a
zero-length placeholder. The start assertion is thereby bypassed. Its actual
predicate must be checked, rather than treating its zero-width capability as
unconditional success. Preserve this observation in the faithful port; repair
it only in a separately named variant.

**Boundary (EOF wave).** The reference evaluates positions n-1 down to 0,
never n. Consequently no stored root match is produced on empty input, even
for Nothing or an empty literal. A caller explicitly using `lookUpBestMatch`
can get a placeholder for nullable clauses; that is a different result contract
from the stored matches inspected by this conformance test. These details
preclude advertising the current port as an unrestricted ordinary PEG engine.

### PIKA-03 — Conditional quadratic event bound

**Theorem under explicit RAM/model assumptions.** Let Z be the scheduling
edges c -> p for which p has the reference's zero flag. Such a parent is
queued even after a failure or an unchanged result. If Z is acyclic, let P
be its total number of directed paths, including a length-zero path per node.
Also require the graph followed by top-down negative lookups to be acyclic.
`llpika.graph.eventpaths` checks these sufficient conditions, returning unknown
for cycles or its path-count cap. This is not ordinary grammar nullability.

At position i each clause can improve its stored length at most n-i+1 times
(the initial zero-length match is included). Therefore the total strict updates
are at most U = g*n*(n+3)/2. There are at most n*g initial terminal events and
e*U notifications caused by strict improvements, where e is the number of seed
edges. Every other event descends along an unconditional Z edge. Unrolling
those acyclic paths gives the deliberately loose bound

`queue events <= P * (n*g + e*U)`.

For fixed graph, each event has bounded clause/terminal and negative-lookup
work. The native multiset scan and parent scan each cost O(g), unlike Java's
heap; account for these grammar factors rather than claiming identical speed.
The memo index uses O(g*(n+1)) cells. All attempted matches and placeholders
remain in the native arena, so its allocation/retention is bounded by the event
count times grammar-dependent lookup/arity work: conditionally O(n^2), not
linear. Expanding the DAG into an enumerated tree is excluded. The certificate
binds the ranked graph, root and implementation version. P may grow
exponentially with grammar size; this is a fixed-grammar input bound, not a
combined polynomial-in-grammar claim. The certificate is a mathematical
argument plus an executable sufficient-condition checker, not a mechanized
proof of cREXX or of the port's implementation.

### PEG-03 — Settled right-to-left tabulation

**Theorem for the admitted profile.** Define a conservative same-position
dependency graph: sequences depend on their first operand and also their
second when the first may be nullable; choice depends on both; wrappers,
predicates, optional and repetition depend on their body. Require this graph
acyclic and repetition bodies provably consuming on success. Compute a child
before parent order and evaluate positions n down to 0.

Induct on descending position, then this clause order. A sequence's second
operand is either at a larger (already settled) position or at the same
position with a required earlier clause. A repeated tail is strictly later;
predicate and choice operands are settled at the same position. Thus every
cell is the definitive ordinary-PEG result, including failure, empty success,
ordered selection and EOF. No provisional empty success or length comparison
is needed. This also covers consuming right recursion.

`lltabpeg` implements the construction and asserts every consulted cell is
settled. Its normalized binary clause work is O(G*(n+1)+a), where G includes
literal/set lengths; the output arena has at most one node per successful
cell. The native packrat control agrees on 154 acceptance/selected-tree cases;
additional literal, plus, optional, binary, EOF/trivia and resource cases run.
This is a separately named experiment, not a claim to fix recursive Pika.

### PRUNE-01 — Refinement question and semantic gate

**Research hypothesis, still open for recursive profiles.** Retain dependency
versions and sufficient candidate information to discard a result only under
a proved continuation-preserving dominance relation. For observations O,
discarding B in favour of A requires that every permitted continuation C
preserves the required O when it is given A instead of B. O includes selected
tree and source provenance where promised, not only final acceptance.

Matching a longer prefix can make a caller fail when a shorter prefix would
allow it to match. Keeping all endpoints and choosing whichever completes the
document can change PEG commitment into CFG-like search. Therefore an endpoint
set is not automatically a correct PEG refinement. Likewise ordered choice
and negative predicates are not monotone in matched length. The earlier
`A <- A 'b' / 'a'` example has body F(1)=2 and F(2)=1 on `ab`; retaining the
larger seed defines an operational policy, not an ordinary fixed point of F.

Test dependency invalidation first, then candidate frontiers against a stated
recursive semantics; certify bounded frontiers only for proved grammar
conditions. Keep semantic changes, tree changes and time/memory changes as
separate verdicts. The stratified result above establishes the nonrecursive
baseline; it does not discharge these recursive obligations.

### REC-03 — c14 ranking is not right-congruent in general

**Executed algebraic counterexample.** The pinned c14 comparator includes a
one-point absorption penalty when a repaired reading consumes more unproved
characters than it supplies evidence. Compare readings starting at 0:

| Reading | End | Deleted | Missing | Evidence | Absorbed | Charge |
|---|---:|---:|---:|---:|---:|---:|
| A | 5 | 4 | 0 | 1 | 0 | 4 |
| B | 5 | 0 | 4 | 2 | 3 | 5 |

Both have spent 4 and end at 5; A wins. Append the **same** clean picky
one-character step. End becomes 6, evidence becomes 2 for A and 3 for B,
and B's absorption penalty disappears. Both now charge 4, so B wins on more
evidence. `c14-rank-probe.dart` executes the upstream private algebra in an
untracked source copy: comparator values are -1 before and +1 after. The native
`llc14cost` port reproduces it in both build modes/VMs. EOF absence charging is
also explicitly nonadditive (many missing-at-end marks incur one cost charge).

**Executed grammar-level counterexample.** Reachability is now established by
`cases/c14-pruning-probe.dart`, run against the actual pinned engine:

```
S <- X 'f'
X <- 'e' / 'z' 'z' 'z' 'z' 'a' 'b' . . .
input: abcdef
```

A deletes `abcd` and reads `e`; B records four missing `z` characters, reads
`ab`, then absorbs `cde`. Both are produced by the engine's own candidate
machinery at budget 4. The outer sequence retains A at position 5 and prunes
B. After the common `f`, B ranks better by the engine's own seven-key rule.
The probe also checks that the normal recovery result is the completed A and
prints the reachable completed B. `evidence/c14-pruning-reference.out` retains
the trees and comparator values. This refutes global minimization of that
ranking over these generated candidates; it says nothing about which repair
a human would prefer. It is separate from Pika's recognition semantics.

### REC-04 — A conservative algebraic repair frontier

**Sufficient theorem for identical future steps, not a whole-search proof.**
For readings A and B with the same start and end, let A dominate B only if:

- deleted, missing, EOF-missing, penalties, absorbed and incomplete of A are
  no greater than B;
- evidence and first-doubt position of A are no smaller than B;
- last-doubt position of A is no greater than B.

Appending an identical step adds equal amounts to the additive fields,
combines first doubt with `min` and last doubt with `max`, and applies the same
monotone incomplete update. The absorbed difference is also preserved because
the prefix endpoints agree. Thus every displayed inequality survives any
finite common continuation. The repaired-and-more-absorbed-than-evidence flag
cannot be larger for A; neither can the EOF-presence flag. Charge(A) is at
most charge(B). On a tie every successive key of the seven-key comparator is
no worse for A, including `min(deleted, missing + EOF-missing)`. Consequently
A cannot rank worse after a common continuation.

`llc14cost.dominates` implements this sufficient test. Its separate experimental
frontier retains incomparable summaries. A and B in REC-03 are incomparable,
so both survive and the completed B can win. Both native build modes and VMs
pass 2,016 bounded continuation checks and the counterexample. These checks
challenge the implementation of the theorem; finite tests are not its proof.

This does **not** yet justify pruning in the entire recovery evaluator:
continuation availability, remaining budget, preferred-prefix gates,
seed/version dependence and selected-tree tie semantics must also be preserved.
The frontier keeps the first exactly equal summary; faithful c14 replaces an
exact rank tie with the newest tree. Nor is a small frontier guaranteed for
arbitrary grammars. The allocation cap reports resource exhaustion, not syntax
failure or a successful complexity certificate. The full native c14 evaluator and exact weighted byte-edit oracle now execute
(REC-05/06); neither closes those whole-search obligations.

### OP-01 — Checked operator specialization and tree projection

**Theorem for an explicit restricted declaration.** Atom and delimiter tokens
are distinct from operator tokens. Infix precedence levels are totally ordered;
each level has uniform left or right associativity. Prefix operators bind more
tightly than every infix. For each left-associative level generate
`E_i -> E_i op E_(i+1) | E_(i+1)`; for a right-associative level generate
`E_i -> E_(i+1) op E_i | E_(i+1)`. Primary is atom, parenthesized E_1 or
prefix-primary. Distinct operator tokens at a level select their named node.

Pratt's right operand threshold is p+1 for a left operator and p for a right
operator. Thus the former stops before the next same-level operator, producing
the generated CFG's left fold; the latter consumes it recursively, producing
its right fold. Induct over levels, then the number of operators at a level;
atoms/groups/prefixes give the base. The two constructions select the same
named expression tree with the same source spans. The profile intentionally
rejects mixed associativity within one level, although more elaborate operator
grammars can define it. Prefix binding is this example's explicit declaration,
not a claim about Rexx or other languages.

`llpratt` generates this shared CFG and supplies the specialized parser.
`lllr` independently constructs/executes its canonical LR tables. `llproject`
executes a declared keep/drop/arity mapping from concrete derivations to the
named tree; no backend-specific arithmetic actions run during recognition.
Seven value/tree examples and 781 bounded generated inputs agree in both build
modes and VMs. Negative tests exercise grammar mismatch, unsupported operator
declarations, malformed input and work/map budgets. The specialization provider
accepts only the exact generated grammar identity; altering it invalidates the
admission proof instead of assuming that the operator table remains applicable.

Pratt consumes every atom/operator/delimiter once, with bounded control work
per token and O(t) selected nodes; call depth is separately capped. Projection
terminates because source children precede parents and each source node is
visited once. Its **mapping storage** need not be linear for arbitrary trees:
transparent nodes can repeatedly propagate long child lists. The implementation
therefore budgets map entries and does not issue an unrestricted linear bound
for projection. Kept nodes retain original source bytes and an origin handle;
declared arity and structural/source invariants are checked. Deliberate span
mutation invalidates the certificate and fails revalidation. These are runtime
invariant checks and a local preservation argument, not a proof of the entire
compiler AST or semantics.

### REC-05 — Faithful c14 search: correspondence and bounds still to establish

**Executed correspondence, not a universal proof.** `llc14` ports the pinned
c14 evaluator, including its REC-03 counterexample, over explicit n-ary grammar
nodes and byte input. All 237 retained cases match status, reported cost and
complete raw tree in both compilation modes and VMs. Multi-character literals
have their own generated character slots, so their internal repairs are not
mistaken for a single missing construct. References memoize named rules;
composite repair cells follow the budget watermark and seed-version protocol.

The reference's rank, candidate gates, tie replacement and root selection are
preserved. Implementation differences include indexed arrays instead of maps,
rebuilding served views instead of identity-caching them, and retaining all
allocated readings/lists/trees for the request lifetime. These preserve the
observed result on the tested domain but can increase work and memory. A fuel
limit and an explicit memo-index budget give a bounded run with a resource
status; they are not a proof that every admitted recursive grammar completes
within a useful polynomial bound. Byte tests do not qualify Dart UTF-16 or
Unicode scalar semantics. The native port rejects an existing-token stream
whose symbols/spans are not the original bytes.

### REC-06 — Exact bounded weighted repair oracle

**Theorem for positive integral edit costs and complete terminal alphabet.**
A state is an entire byte string. Directed edges insert/delete/substitute one
byte with costs wi, wd, ws >= 1. Insertion/substitution choices include every
terminal emitted by the admitted pure-CFG lowering; deletion can remove any
input byte. Every accepted target uses that alphabet. A minimum-cost script
between the input and such a target can be taken in alignment form (no edit
of text that is later deleted, and no repeated substitutions); therefore
restricting new bytes to this alphabet loses no minimum-cost accepted target.

An additional safe lower bound counts bytes outside the complete CFG terminal
alphabet: each must separately be deleted or substituted, costing at least
min(wd,ws). A state whose current cost plus that bound exceeds d can be omitted.
The alphabet is conservatively collected from all lowered productions, so
unreachable terminals can only weaken this bound, not make it unsound.

Dijkstra's settled-distance invariant implies that the first state accepted by
Earley has minimum weighted byte-edit cost. Discarding a higher-cost route to
an identical whole string is safe: recognition and available future edits
depend only on that string. This is the crucial difference from pruning by
endpoint while forgetting candidate properties in REC-03. If all states with
distance <= d are exhausted, no repair within d exists. A construction or work
limit must instead return resource exhaustion and cannot support that claim.

Finiteness follows from positive weights: intermediate length is at most
n + floor(d/wi); the alphabet of intermediates is contained in the union of
original input bytes and grammar terminals. A crude state bound is the sum of
A^k for 0 <= k <= n + floor(d/wi), A <= 256. This is an intentionally expensive
small-input oracle, not a claimed polynomial-in-d recovery engine. The current
native implementation uses a collision-checked hash index for deduplication
and scans the state list for minimum selection, charging comparisons to fuel; Earley work is charged
as well. The returned packed forest describes the corrected bytes. Edit
positions refer to successive intermediate strings; these are not fabricated
original-source spans or a human-intent guarantee.

The metric is **not** c14's seven-key ranking. In particular, c14 groups many
EOF absences into one search charge and never invents characters, whereas this
oracle finds concrete valid replacement text. Numerical costs can be compared
only after an explicit common observation/cost adapter is established.


### REC-07 — Integrated candidate frontier: bounded evidence

**Experiment.** `llc14` preserves faithful c14 by default. Its separately named
`frontier-experiment` uses REC-04 dominance in repair cells, sequences and
repetition. Incomparable same-end candidates survive; mutually dominating ties
follow c14's newest-tree convention except where repetition requires strict
improvement. The representation can retain additional dominated slots after a
replacement, so this is a conservative frontier, not a minimal Pareto set.

Across 237 reference inputs: one improved seven-key rank (REC-03), zero worse,
236 ties, zero exhausted. All 42 clean trees match. Maximum work ratio is
923/730 and allocation ratio 671/512, on `+-a`. These operation counts do not
measure elapsed time, memory in bytes or human repair quality. Finite success
cannot prove preservation of search availability under c14's gates/versioning.

REC-06's native oracle separately agrees with Wagner-Fischer distance to the
explicit finite language {ab, ba} on 480 source/cost combinations (source
alphabet abx, length 0..3, wi/wd 1..2, ws 1..3). Ambiguous target forests, NUL,
resource failure and certified absence within a bound are additional tests.

### PRUNE-02 — Longest-choice is not a PEG-preserving Pika refinement

**Executed counterexamples.** The experimental policy evaluates all ordered
alternatives, selecting the longest and retaining earlier choices on length
ties. All other Pika scheduling rules remain unchanged.

| Grammar and input | Faithful Pika | Longest-choice experiment |
|---|---|---|
| `E <- E '+' T / E '-' T / T; T <- [ab]`, `a+b-a` | reject | accept |
| `S <- ('a' / 'ab') 'b'`, `ab` | accept | reject |
| `S <- ('a' / 'ab') 'c'`, `abc` | reject | accept |

The second and third grammars are nonrecursive ordinary PEG controls. Their
meaning is changed in both directions. The second also has a CFG derivation,
so retaining only the longest prefix is not a general CFG fallback. These
three controls and the unchanged 508 faithful conformance cases pass in both
build modes/VMs. Improving a local score alone cannot establish the desired
semantics; an improved recursive algorithm needs an explicit target definition,
dependency/invalidation proof, and enough candidate information for that target.
No faithful Pika certificate is implicitly transferred to this policy.

### SURF-01 — Authored basic-clause lowering

**Local preservation argument.** A byte literal becomes a sequence of its byte
terminals (empty becomes epsilon). A byte set becomes a union of byte ranges;
under ordered semantics, overlapping ranges still consume exactly one identical
byte, so their relative priority affects only transparent construction wrappers.
`e+` becomes `e e*`; `e?` becomes `e / epsilon` for PEG and `e | epsilon` for CFG.
Named rule wrappers preserve declaration identity through each substitution.
The applicable backend still rejects nonprogressing repetitions and unsupported
recursion/predicates. Raw normalized trees need not equal an unnormalized
reference's recovery histories: c14's n-ary recovery is explicitly excluded
from this equivalence claim.

The two compared engines receive the identical normalized graph. Packrat and
settled-dependency tabulation agree on the complete TinyConfig consumer. A
separately handwritten parser supplies 84 valid/mutated input and named-tree
expectations, including escaped strings, nested arrays, malformed punctuation,
NUL and unsupported non-ASCII string contents. LR and Earley also execute one
authored token-list CFG over all length-0..4 streams in a two-token alphabet.
These are executed comparisons, not a theorem that the frontend accepts every
intended future Level L declaration. Construction and source nesting budgets
are explicit.

### STAGE-01 — Finite-context generated token passes

**Theorem for the declared profile.** At step i the transducer state consists
of input token position and one finite context. Its rules inspect at most two
token kinds and the current token's ASCII spelling, then consume one or two
tokens, emit at most one token and select the next context. The first matching
rule supplies the declared deterministic semantics; no match copies one token.
There are at most t steps, so termination follows from strict increase of the
input position. With r declarations and B inspected spelling bytes, a conservative
bound is O(r*(t+B)); for a fixed declaration set this is linear in input plus
spelling size. Runtime fuel is a charged-work backstop, not a CPU-time promise.
The current uppercase/guard comparisons can depend on spelling length; counting
only transitions would omit that cost.

An emitted token has the span from its first consumed token's start to its last
consumed token's end. Contiguity in token order and valid input spans imply
monotone, nonoverlapping output spans. Skipped bytes remain in the owned original
source. The first/last input-token map composes between passes; it is tested
against an independent handwritten normalization/classification pass. Because
context is owned by a request and output streams are new objects, a later parser
cannot accidentally reuse a memo entry computed under a different lexical mode.

These facts prove execution and provenance for the declarations, not that any
arbitrary hand-written compiler fixup can be inferred from a grammar. A
finite-state pass cannot in general replace unbounded nesting or type-dependent
classification. Such stages require their own stack/attribute/service contracts.

### COMP-01 — Delimiter-bounded operator regions

**Sufficient-condition theorem, not a universal hybrid conversion.** An outer
pure CFG contains a reserved virtual terminal v. Its terminal alphabet excludes
fresh opener o and closer c. A region's language is OP-01's explicit operator
CFG; its alphabet excludes o, c and v. Raw input excludes v. Generate the whole
CFG by substituting `o E c` for each v site, retaining its named wrapper.

Every successful whole-CFG derivation then has uniquely identifiable disjoint
regions: an o can only be the start of a substituted site, and the first c
following it can only be that site's end. The inner grammar cannot consume a
boundary token. Parse each complete interior with the admitted Pratt engine,
replace its span with v, and parse the resulting stream with outer LR(1).
Substitution of component derivations proves acceptance in both directions;
OP-01 supplies the selected inner-tree correspondence. Reinsert the owned inner
tree under the retained site wrapper, preserving the delimiter-inclusive wrapper
span and the interior's original byte spans. The same declared semantic-node
projection agrees with parsing the generated whole CFG. This does not say that
raw construction wrappers from different algorithms are identical.

The planner in `llregional` checks conservative complete alphabets, distinct
boundaries, normalized single-terminal region sites and the actual LR table
construction. Its inner grammar is generated from the same frozen operator
object used by the Pratt engine. Overlapping alphabets are rejected, including
an attempted reuse of the region delimiters as expression parentheses. It does
not convert general CFG callees to first-success PEG callees or silently discard
alternative endpoints. Missing closers name their opener; failed embedded parses
retain their own source location. Syntax, invalid request and resource outcomes
are distinct.

Scanning and the disjoint Pratt invocations consume O(t) input work; outer LR
is linear for fixed admitted tables. Grafting a selected tree of a nodes uses
binary search for each of R region positions, giving O(a + R log R) work and
O(t+a+R) retained token/tree storage, excluding grammar construction. The current
schema projection is separately subject to OP-01's map-size limitation.
Request-owned copies preserve source bytes; a depth/fuel failure yields resource
exhaustion. There is no streaming, incremental or escaped-delimiter guarantee.
Already-tokenized strings/comments can be opaque single terminals, but their
correct lexing belongs to the ingress contract.

Executed controls compare 789 inputs (including 781 bounded expression strings)
against the unspecialized LR parser, checking selected semantic trees and spans.
A repeated outer grammar additionally exercises two independent regions.
These tests challenge both the implementation and proof assumptions; they do
not establish the ability to synthesize arbitrary compiler fixups.

### TREE-02 — Declared region grouping and certificate lifetime

**Local preservation theorem.** The admitted input is an owned valid source
DAG with an ordered binary-list root. Its flattened top-level items are unique;
each is either a declared marker kind or an admitted member kind. A marker
starts a region and the next marker ends it. Members before the first marker
require an explicitly declared implicit-group label; otherwise construction
fails. Repeated visits in the flat list are rejected rather than duplicating
instructions through accidental sharing.

The native `llgroup` rewrite copies each selected member subtree with the same
kind, payload label, source bounds and child order. It creates a body list for
each maximal marker/member region, preserving member order, and retains marker
header children before the body. Partitioning the finite flat list proves that
every member belongs to exactly one result body, in the original order, and
that every explicit marker starts exactly one group. The result group's range
is the union of its header and body extents. Input ordering plus child-span
containment establishes the output's source-tree invariants. Synthetic implicit
groups, body wrappers and lists are marked synthetic; expanded marker groups
have composite provenance; copied subtrees retain exact origin handles.

Flattening visits each list/item once, memoized copying visits each source node
at most once, and member admission scans r declared member kinds. For N source
nodes and m flat members the algorithm is O(N + r*m), plus owned-source storage;
output storage is O(N+m). Depth/work limits yield resource exhaustion rather
than malformed syntax. These bounds assume constant-time indexed arena access
and do not claim a specific VM allocation or copy cost. The output keeps the
source bytes after the builder/rewrite request goes away.

The legacy adapter supplies three frozen B/G raw trees (two explicit procedures
and implicit main). The independently executed compiler's STAGE_FIXUP gives
expected node/value/order observations. The Level L builder, traversal and
declared grouping agree on that projection. OPTIONS, synthetic empty ARGS,
synthetic return-type CLASS and later symbol/type facts are outside this
comparison; declared CLASS nodes remain. The raw debug dump has no source
positions, so a closed-corpus lexical adapter supplies anchors. This is not a
full source-span correspondence claim or an implementation of type inference.

`tree.validate()` is a revision-bound runtime certificate for structural/source
properties. Every relocation invalidates it. A deliberately broken parent span
fails revalidation and is rejected before rewriting; restoring the span requires
and passes a fresh check. An independently retained original tree stays valid.
These tests establish the executed certificate protocol. Mathematical grouping
preservation, runtime invariant checks and compiler correspondence are distinct
evidence, not interchangeable claims of proving all compiler AST invariants.

**Migration limitation.** This arena's ordered source children are appropriate
for authored structure. Arbitrary later declaration hoisting, code duplication
or control-flow lowering may need a different semantic-IR ordering and multiple
source origins. Do not weaken the source-tree validator to make a reordered work
AST pass; provide the appropriate generic semantic layer and its own contracts.

### ATTR-01 — Generated DAG schedules and finite-height join worklists

**Theorem for an explicitly restricted equation language.** Each Boolean
attribute has equation `x_i = seed_i OR OR(x_j : j -> i)` with a fixed finite
dependency graph. There are no host callbacks, negations, destructive updates
or dynamically added variables during evaluation. The product lattice has
bottom all-false; each coordinate can rise once. The function is monotone.

For an acyclic graph, Kahn's algorithm generates a topological order. Evaluating
nodes in that order gives the unique solution, since every dependency has its
final value before its consumer. For a cyclic graph, seed true coordinates and
propagate only first changes along outgoing edges. Every true coordinate has a
path from a seed (soundness, by induction on queue insertion). Conversely every
coordinate reachable from a seed eventually becomes true (induct on path length).
That reachability vector is a solution and is contained in every solution,
therefore it is the least fixed point. Each variable is queued at most once and
each outgoing edge is processed at most once: O(V+E) time/storage after graph
construction. This is stronger than merely imposing an iteration cap.

`llschedule` implements both modes and an automatic checked choice. A required
DAG profile rejects a cycle and produces a real `A needs B needs A` witness by
following residual incoming edges until a variable repeats. Automatic mode can
select the worklist because every admitted equation has the monotonicity law
above. An attempted `not` equation is rejected; it is not made safe by a fuel
limit. Resource exhaustion never produces a certificate for partial facts.
Dependency mutation invalidates the prior result and requires fresh evaluation.

The native test enumerates all 512 directed graphs on three variables and all
eight seed subsets, comparing both modes against independent repeated relational
closure. A tree consumer declares a synthesized `has.return` fact, seeds nodes
of its chosen RETURN kind, and generates child-to-parent dependencies. This is
a generic attribute example, not a proof of cREXX return reachability or its
interacting type/exit/import pipeline. Richer finite lattices, transfer functions
and effectful services need separate admitted laws and cost/termination proofs.


## POWER-01 — Executable non-context-free example (local proof)

`specs/equal-three.levell` declares ordinary ordered PEG rules equivalent to
`S <- &(A 'c') 'a'+ B`, `A <- 'a' A? 'b'`, `B <- 'b' B? 'c'`, with
whole-input acceptance. The language is exactly `{a^n b^n c^n | n >= 1}`.
Induction on recursive A depth establishes that A consumes an equally sized,
nonempty a/b pair. Requiring the next character to be c makes the leading
lookahead fix the complete a and b blocks. The consuming a+ then stops at b;
B's analogous induction equates b/c counts. Whole-input acceptance excludes
trailing characters. Conversely each equal triple follows those derivations.

For non-context-freeness, use the CFG pumping lemma on a^p b^p c^p. A pumped
window of length at most p cannot contain both an a and a c, so pumping a
nonempty portion changes at most two block counts, or breaks their order.
The unchanged third count prevents an equal triple. Thus no CFG recognizes
this language. This is a power example for ordinary PEG lookahead; it does
not establish a new language class or a Pika-specific advantage.

The independent test oracle counts and orders blocks. Native packrat,
settled tabulation and the explicitly operational Pika lowering are compared
on all block counts 0..6 plus all words over abc of length 0..5. Passing this
finite panel is implementation evidence for the proof's example, not the
proof itself. The faithful Pika evaluator remains distinct from ordinary PEG.

## POWER-02 — Current separation literature (external result; not locally proved)

Ford's [2004 PEG paper](https://bford.info/pub/lang/peg.pdf) establishes
non-context-free examples and deterministic-CFG inclusion; its incomparability
claim was conjectural. A new [Kim/Park preprint, v1, 30 August 2026](https://arxiv.org/html/2608.29592v1)
claims a linear context-free language outside total PEGs and nonclosure under
reversal and concatenation. Its Lean artifact explicitly assumes a
cell-probe lower-bound corollary for the negative results. We have not run
that artifact or independently verified the entire proof; do not describe
it as a local machine-checked theorem or silently inherit its axioms.

The invoked [Ko theorem, current v2, 25 September 2026](https://arxiv.org/html/2603.25914v2)
uses single-bit updates and m=n^(1+Omega(1)) preprocessed vectors. Its stated
query lower bound is Omega((log n / log(t_u w))^2). With constant update cost
and logarithmic word size this is superlogarithmic, consistent with the
separation paper's required contradiction. This checks the stated interface,
not the internal proof. Total ordinary PEG language results do not
automatically classify Squirrel's operational left-recursion extension or
Pika's retained-seed behavior. The support decision must therefore use
explicit semantics and tested admission rather than claim that one engine
handles every CFG or every language.


## DIAG-01 — Grammar witnesses and limits (local arguments)

`lllexplain` performs BFS on the generated DFA from its actual start state.
Every visited accepting state is reachable by its predecessor word. Ending
input there makes that state the maximal-munch winner, so its rule is usable.
Conversely any token selecting a rule ends in a reachable accepting state
bearing that rule ID. With distinct rule IDs, absence from all reachable
accepting states therefore proves that rule never wins. BFS supplies a shortest
witness for reachable rules. The ASCII-table bound is O(128q) work, O(q)
search storage, apart from table validation/storage. Exhausting fuel is not a
proof of unreachability. Multiple rules sharing an ID cannot be distinguished
by this table and require richer provenance before this analysis applies.

`llwitness` explores a finite declared alphabet by increasing word length.
For CFG ambiguity it requires two derivations of the *same complete word* in
one successful packed forest. For priority it compares the declared ordinary
PEG to an explicitly different unordered interpretation, on a restricted
predicate/repetition-free shared grammar. CFG success plus PEG rejection is
a concrete counterexample, not a conversion theorem. The priority search
rejects predicates rather than erase them. Reachable forest cycles are reported
as cyclic evidence separately from two finite derivations.

For alphabet size k and bound m, at most sum(k^i,i=0..m) candidate inputs are
visited, each charged its actual Earley/PEG action work. Case and work budgets
are distinct from 'none found within the complete requested bound'. Neither
bounded exhaustion result proves unambiguity or equivalence. The dependency
explainer follows actual conservative same-position edges and reports the
cycle path, but nullability overapproximation means it establishes a missing
termination proof, not inevitable nontermination on a particular input.


## REC-08 — LR local repair control (local shortest-path argument)

`lllrrepair` is a bounded CPCT+-style clarity-first control inspired by
[Diekmann/Tratt, ECOOP 2020](https://soft-dev.org/pubs/html/diekmann_tratt__dont_panic/).
It uses unit insertion/deletion cost, zero-cost individual reductions/shifts,
retains all distinct minimum-cost local edit scripts, then ranks them by
unmodified-input progress (default 250 tokens). Success is acceptance or three
consecutive original-token shifts. A reduction-only successor is retained;
requiring every successor to consume input would miss repairs near EOF.
It is not a port/qualification of the paper's optimized Rust implementation.

The search freezes the valid parser prefix and searches increasing cost
buckets. Every edge is nonnegative. Completing the first successful bucket
therefore minimizes cost **over this local successor relation and frozen
prefix**. Canonical scripts put adjacent insertions before deletions; without
an intervening original shift these commute to the same edited token sequence.
Deduplication requires equal concrete stack content, original position, shift
streak, previous action and edit script; it does not prune a differently ranked
repair at an equal end position. Reporting every minimum script requires the
whole successful bucket to finish: fuel/storage exhaustion returns resource
failure even if partial candidates exist. No global whole-file optimality is
claimed, and a successful local repair is not necessarily a valid whole file.

The implementation stores persistent stack nodes but uses a linear exact
configuration-key lookup. It is intentionally unsuitable for a linear-time
or CPCT+ performance claim. With C visited configurations, stack/string extent
L, and E action/reduction steps, a conservative bookkeeping term is O(C^2 L +
E L); grammar-terminal enumeration and preview work are additional charged
operations. Positive edit costs, fixed input, bounded edit cost and explicit
resource limits make this a bounded experiment; a fuel cap is not proof that
all admissible grammars successfully recover. Nullable/reduction cycles may
exhaust the budget honestly.

The selected result is a token-ID sequence plus an origin map. Positive entries
refer to original token numbers; negative entries encode insertion boundaries.
Original bytes remain separately owned. For independently lexed input, inserted
IDs are suggestions requiring spelling/value policy, not invented source text.
Later untouched tokens retain their original order and identity. Ties can
still destroy intended structure: `[aa]` admits deleting an atom or inserting
a comma at the same cost. The UI should display alternatives, not pretend
that local cost determines intent.

The native control reproduces the published `2 3 +` six-repair example using
`a` for each integer token. A separate finite-language Wagner-Fischer oracle
checks 121 inputs with insertion/deletion cost one and substitution cost two.
This finite panel checks the selected repair cost independently, while array
fixtures check subsequent structure, local-versus-complete status, original
source, insertion provenance and resource failures. The comparative held-out
recovery corpus and timing verdict remain separate obligations.

## PIKA-04 — Reachable-clause pruning (local simulation proof)

`reachable-reference` computes the transitive child-dependency closure R of the
requested root, preserving every original clause rank. It seeds only terminals
in R, schedules only parents in R and packs their memo slots. For any clause in
R, every child whose memo value it can inspect is also in R. A removed clause
cannot seed a retained parent: being that parent's child would put it in R.
Deleting outside events from the reference event trace therefore preserves the
relative order and multiplicity of retained events and every retained memo
update. Induction over that projected trace establishes identical selected root
histories, up to arena IDs, for completed runs with sufficient resources.
Top-down negative lookahead uses only the same dependency closure. This argument
does not require monotonicity of PEG choice or of matched lengths.

The claim is root recognition/tree preservation, not identical whole charts,
arena IDs, allocation counts or resource outcomes. Outside rules useful for
island diagnostics are deliberately unavailable; such a consumer must request
all relevant roots or retain the reference policy. The memo index budget becomes
O(|R|*(n+1)) cells. Queue/parent scans still iterate the original grammar ranks,
so this implementation is not an asymptotically optimal sparse queue. Lowering,
closure construction, retained history and diagnostic work remain separate.
The 508-case reference panel compares every selected raw tree and checks that
counted evaluation work does not increase; an explicit unreachable-terminal
case verifies fewer events and a smaller admitted memo budget.

## LEX-04 — Checked immediate-stop boundaries (local sufficient theorem)

Let G be a pure CFG over positive token kinds. Let D be a non-nullable byte DFA,
whose accepting state carries the winning token kind after rule priority. For
kind k define L_k as the nonempty words taking D's initial state to an accepting
state of kind k. Define the comparison character language by regular substitution
of L_k for each token terminal in G; this is an explicit semantic construction,
not an inference about an arbitrary separately authored character grammar.

Compute conservative FIRST/LAST sets for grammar clauses and all possible
adjacent token-kind pairs, including pairs across nullable clauses and repetition
boundaries. Compute DFA state reachability and reverse reachability to each
used accepting kind. These determine possible first bytes of each L_k. Require
that, for every possible adjacent pair (i,j), every reachable accepting state
of kind i has a dead transition on every possible first byte of L_j. Reject
implicit skipped tokens; their source-language semantics have not been declared
in this theorem. Unsupported predicates and construction exhaustion issue no
certificate. Missing progress or another failed sufficient condition is not a
proof that no other lex-first construction could work.

For a derivation in G expanded using words w_1...w_m with w_h in L_kh, scanning
w_h reaches an accepting state of the intended kind. At its boundary the next
byte immediately stops scanning, or EOF does. Maximal munch therefore emits
exactly those kinds and byte spans. Induction over the words proves forward
recognition preservation. Conversely, a complete scan with no skipped tokens
partitions its source into winning words; substituting those words into any
accepted token derivation proves membership in the expanded CFG. The same
mapping preserves CFG derivations and token-leaf extents; it does not invent
lexeme-internal captures, contextual reclassification or a canonical ambiguous
AST. Empty source is covered when G is nullable.

`llboundary` implements this finite check with source-located failed adjacency,
exact grammar/lexer identity binding and a separate construction budget. The
research limits are 512 grammar clauses, 128 DFA states and 255 positive kinds.
Its dense fixed-point analysis is intentionally conservative; with alphabet B,
K used kinds, g clauses and q states, a simple upper bound for construction is
O((gK+qK+q+1)*(gK + q*B*K) + g*K^2 + q*K*B). Storage is
O(gK + K^2 + qK + KB), using fixed 256-wide arrays. Work counters count selected
loop units, not CPU instructions. The certificate makes no generation-speed
claim. Mutation of either input invalidates its identity check.

The compact assignment proof uses an explicit no-skip DFA copy. The separately
authored Pika character/token assignment comparison also tests whitespace and
comments; that finite parity result is evidence, not an extension of this
certificate to arbitrary skipped trivia. UTF-8 outside the ASCII DFA is rejected
at lexical ingress. NUL is real data and is accepted inside these comment rules.
EOF is a separate symbol/position.

## REC-09 — Explicit PEG labels and recovery (restricted executable semantics)

`lllabel` is a small independent control following the labeled-failure model
in [Maidl et al.](https://arxiv.org/abs/1405.6646) and the documented
[LPegLabel contract](https://github.com/sqmedeiros/lpeglabel). An annotation on
p means `p / throw(label)`. Ordinary choice, optional and repetition catch only
ordinary failure; a thrown label propagates. Predicates convert a thrown label
into operand failure and suppress recovery. A declared recovery expression
starts at the throw position; on success it resumes recognition and records an
error node with its original source interval. Speculative alternatives roll
back their collected recovery events when ordinary failure selects a different
branch. The full raw arena can still contain unselected attempts.

The research interpreter deliberately admits only statically non-left-recursive
ordinary PEGs with progressing repetition. Nested label recovery while running
a recovery expression is disabled; this is a documented restricted control,
not a complete LPegLabel port. A recovered full parse is status 1, clean status
0, work/depth exhaustion 2, bad request 3 and unrecovered rejection 4. Recovery
that makes a repetition succeed without progress returns a resource failure.
No edit-optimality claim attaches to the number of labeled events or skipped
bytes. All labels are installed before execution and the request owns its tree
and source. The interpreter is not memoized, so no packrat linear-time bound
is claimed; fuel/depth limits are operational caps, not proofs of successful
completion for arbitrary damage.

Adding labels arbitrarily does not preserve a grammar's clean language. If
`('a' / 'b')` is changed to `(('a' / throw(L)) / 'b')`, input b now throws L
instead of selecting b. Placement must be justified by a commitment condition,
proved for a restricted grammar, or checked against an independent corpus.
The focused native control executes this counterexample, predicate suppression,
clean/recovered/rejected outcomes and owned error spans. The frozen comparison
checks clean-input preservation on its complete finite panel without claiming
that finite testing proves all inputs safe.

## REC-10 — Chart explanation and comparison boundaries

`llpikaexplain` is a separate, read-only, anchored pass over completed Pika memo
results, corresponding to the positive-clause explanation experiment in the
pinned Java probe. It follows successful sequence prefixes, visits ordered
alternatives as diagnostic hypotheses and reports terminal expectations at the
farthest reached position. Plus exposes its first item. Clause/position visits
are bounded; full literal mismatch analysis, predicates, start anchors and
flagged terminals are outside this policy. Unsupported cases return explicit
failure rather than pretend an expectation set is complete. This pass does not
repair text and does not prove which edit the author intended.

The shared 40-case array panel is frozen in `evidence/recovery-corpus.json` with
27 development and 13 held-out cases. An independent recursive-descent recognizer
and exhaustive insert/delete BFS through cost two provide clean outcomes and
bounded minimum costs; three native Earley/edit-oracle cases independently agree.
Substitution costs two. The same grouping is preserved for c14, whose structural
recovery ranking depends on grouping. Every panel engine agrees on seven clean
and 33 damaged inputs, with no clean result silently counted as repaired.

The comparison has an explicit negative result for LR locality: on `[a],a];[a]`
the global repair can insert `[` at the beginning for cost one. The local LR
search has already committed its accepted prefix and selects cost three. This
does not contradict its local shortest-path property. Two corpus rows encode
this text from distinct original mutations; do not deduplicate them after
seeing results or interpret the duplicate as independent language evidence.

Pika's 29 retained later-array islands are coverage evidence. c14's full-span
error trees are recovered analyses, not a promise of a syntactically valid
edited string. LR's local success must still be checked with complete(); two
panel cases stop locally before the remaining error. These observations must
not be merged into one misleading 'recovery success' percentage. Raw outputs,
all costs, failures and exact four-cell parity are retained in tranche 11.

## STAGE-02 — Nested-comment masking and final clause closure

**Local invariant and bounded implementation argument.** With nonempty distinct
opening/closing delimiters, the counter stage advances at least one input byte
per iteration. At positive depth, a closing delimiter decrements depth, an
opening delimiter increments it, and other bytes remain inside the comment.
At depth zero, single/double quoted strings protect delimiter text, with doubled
quotes handled explicitly. An unfinished counter or quote is a located lexical
failure, not an accepted masked source. Counter overflow is excluded by the
input/fuel bounds of these finite research fixtures; arbitrary-size production
arithmetic needs a checked representation.

Inductively, output length equals the consumed input length: outside comments
bytes copy exactly; inside comments each consumed byte becomes one space.
Consequently a generated scanner's half-open offsets map by identity back to
owned original bytes. Newlines inside comments do not create clause tokens;
newlines outside comments do. A copy of the masked source is not the source
owner. Quote, nested-comment, alternate-delimiter, NUL, rejected UTF-8, fuel and
idempotent-call controls execute in tranche 15.

For delimiter lengths a,b and input bytes n, the reported scanner-stage work is
at most n+(a+b+1)n, counting the ASCII-profile prepass. This is not a total time
certificate: the present cREXX implementation grows an output string, so a
conservative copying model permits O(n^2) construction time. Output storage is
O(n); counter depth is O(n). Replacing construction with a measured bounded-copy
buffer is a distinct implementation improvement. No timing win is claimed here.

The native representative grammar admits a final statement at EOF. This gives
the tested final-clause behavior without appending an empty source token, which
the common stream deliberately rejects. A production synthetic-token API would
need explicit origins and a distinct invariant; it is not silently introduced
by this adapter. The repeated-prefix LR factoring in the RexxScript consumer
also preserves only the stated named-tree projection, not binary wrapper IDs.

## REWRITE-01 — Transformation audit and rejected generalizations

**Local sufficient arguments; implementation status is explicit.** Pure CFG
EBNF expansion uses fresh nonterminals for alternatives and lists, with a
production-origin map and a tree projection erasing generated wrappers. Its
language argument is induction on the finite repetition length. Nullable-body
cycles can create infinitely many derivations; the forest must retain that
fact, or a stricter profile must reject. `llcfg` and the packed-forest tests
execute the selected construction; a flattened list is an observation adapter,
not a claim that all raw derivations have become identical.

For pure deterministic PEG expressions, `(A B)/(A C)` and `A (B/C)` have the
same endpoint: A fails in both, or succeeds at the same endpoint on each retry,
then B and C are attempted in the same order. A reconstruction map must restore
the chosen authored alternative and its A wrapper. Stateful actions invalidate
the argument. It does not justify factoring a seed-growing recursive rule,
where the seed/version context can differ; Pika's retained mixed-recursion
example demonstrates why a textual CFG-style rewrite cannot be presumed to
preserve that operational language. The RexxScript common-prefix CFG factoring
and declared Pratt lowering execute narrower, explicit cases.

A deterministic lexer may be minimized by partition refinement **only when its
observable accepting outputs are equal**: initialize partitions by the complete
winning output (token/skip/rule identity as required), then refine until every
input-class successor enters the same partition. Induction on remaining input
length proves identical outputs at every prefix, hence identical longest-match
endpoints and priority results. State merging does not preserve tagged captures
unless tag effects also agree. The current generator does alphabet equivalence
compression and subset construction, not state minimization. No minimization
speed or size benefit was measured; this is a proved applicable later backend
optimization, not a missing mandatory native port.

Regular-region extraction requires more than finding a regular-looking
subexpression. Prove its pure regular meaning, preserve the accepting outputs,
and check the caller's continuation and boundary obligations. LEX-04 executes
one sufficient token-boundary checker; COMP-01 executes one delimiter/operator
region checker. General automatic extraction across predicates, prioritized
recursive choices, stateful callbacks or a CFG callee with several endpoints
is **not proved and not performed**. Replacing all endpoints by the longest
one is rejected as a generic transformation. NFA/DFA lexing and the admitted
specializations supply useful restricted extraction without claiming such a
universal optimizer.

## REC-11 — Audit of the pinned Squirrel manuscript's hardness claims

**External claims audited, with local qualification.** Source:
[`squirrel_parser.tex`, bb1ae847](https://github.com/lukehutch/squirrelparser/blob/bb1ae84701060462636c07c814286ac2875e0568/paper/squirrel_parser.tex),
Theorems `hardness`, `frontier`, `general` and Corollary `linear`.

The 3-SAT reduction to minimum-distance PEG repair is persuasive on its stated
construction. With m variables, the grammar accepts `@` followed by a satisfying
m-bit assignment, or the fallback of m+1 `#` characters. From input `@`, distance
is m iff the formula is satisfiable, otherwise m+1. Length alone proves the lower
bound; allowing unit substitution/transposition does not evade it. The expanded
grammar is O(r*m), so this is **combined grammar-and-budget NP-hardness**, even
with a one-character input. It is not hardness for one fixed grammar.

The accompanying NP-membership argument uses a witness of length at most n+d.
That is polynomial in the encoded instance if d is unary, or otherwise bounded
polynomially in instance size, and membership uses a well-formed normalized
pure PEG. With unrestricted binary-encoded d, n+d can be exponential in the
input encoding. Thus the printed argument does **not** establish the unrestricted
NP-complete statement. We retain NP-hardness and the qualified NP-completeness
claim; we do not infer a different exact complexity class without proof.

The remark that every fixed grammar has only finitely many pending-lookahead
states is not established by this reduction: recursive, unbounded lookahead can
carry input-dependent information. The reduction therefore supplies no general
fixed-grammar polynomial repair algorithm or certificate. Likewise, changing
invention into missing/error marks changes the problem; avoiding this particular
hardness construction does not prove that the resulting search is polynomial.
The manuscript's incidental universal linear Pika claim is independently
contradicted by the retained quadratic family (PIKA-03).

The frontier proof distinguishes 2^m subsets through continuation queries and
therefore supports an Omega(m)-**bit** information lower bound under its explicit
one-pass/no-recomputation/no-grammar-revisit summary model. Its selected examples
use different positions for the one changed `a`, so PEG priority does not hide
the selected branch. This does not require one heap record per endpoint: an
encoding or bitmap may suffice. Nor does the lower bound prove that retaining
one best score per endpoint is sufficient. REC-03 supplies a reachable
counterexample to that sufficiency for the actual c14 ranking. Continuation
specialization, suspended computations and recomputation are different models,
not violations of this restricted information bound.

The disjoint-expansion corollary is conditional on total expansion E being
O(n), and on all clause/repetition/terminal work being covered. Rule-match
counts alone omit scans inside a rule's repetition; a rule-only cache must not
inherit normalized packrat's time certificate. Native Squirrel therefore still
reports unknown cost. Its reference conformance, quadratic expansion witness
and retained-history behavior are evidence; no general static disjointness
checker or globally optimal recovery theorem is asserted.

**Research disposition:** refine the repair state/continuation contract before
claiming safe pruning. REC-04 establishes one sufficient common-continuation
dominance relation. Search availability, budget gates, versions and tree ties
remain explicit obligations. A better scalar score alone does not discharge
them. Q020 in RECOVERY.md also separates rank from intended reconstruction.


## Property construction representation — 12 October

**Implementation argument, functional qualification in progress.** The Unicode
property compiler emits sorted disjoint scalar intervals from pinned raw data.
Balanced union lowering changes grouping, not set union: associativity preserves
membership, while a balanced union over `r` boxes has logarithmic union depth.
The existing byte-box lowering still supplies its own scalar/UTF-8 argument.

For one NFA epsilon closure, maintain a binary membership vector and a queue.
Initially all seeds are marked and queued. Mark a destination before enqueueing
it, and process only epsilon edges of popped states. The invariant is that all
marked states are reachable from a seed by epsilon edges. Completeness follows
by induction on path length: when the predecessor is popped, an unmarked
successor is marked and queued. Each state enters the queue once, so cycles do
not prevent termination. Queue traversal is `O(R + E_reached)` for `R` reached states, plus the
`O(N)` membership-vector allocation. Canonical sorted sparse state keys cost
`O(R log R)` and preserve set equality; transitions inspect those active state
keys rather than scanning the whole NFA for every byte class. This is an algorithmic
argument, not machine verification of cREXX value ownership or a wall-clock
bound.

The alphabet partition remains identical: the former byte signature contained
one bit for each character NFA state; the binary signature places those bits at
state positions and inserts constant zeroes for other states. Two bytes have
equal signatures under one representation iff they do under the other. Subset
membership/equality is unchanged. Acceptance still visits NFA state numbers in
ascending order, preserving lexical rule priority and nullable-token admission.
DFA subset construction can still be exponential; its ordinary scans and
interning have not become a linear construction algorithm. Native/raw-data and
legacy lexer controls must qualify the code before the argument supports a
product behavior claim. The abandoned first full-XID attempt overlapped build
activity and supplies no before/after timing verdict.
# Comment coordination contract — 12 October 2026

This is a preservation/progress argument for the implemented Level B coordinator,
not a machine-checked theorem or a global parsing-speed certificate.

Let the source have `n` bytes and let `m` named comment forms have maximum
delimiter length `d`. Each form specifies a line boundary or a block closer,
same-form nesting or no nesting, a newline flag and an opacity recognizer.
Openers are nonempty and distinct; the selected opener is the longest eligible
one. Inside a selected block, only its own closer/nested opener has meaning,
with closer priority when both match. Quoted/regular matches remove that form's
eligibility within their byte interval; they do not change source spelling.

**Source preservation.** Every output write replaces one existing scanning byte
with ASCII space or keeps that byte. No insertion/deletion occurs. Consequently
the scanning extent remains `n`, every token boundary remains an original byte
boundary, and the owned source used by trees/messages is unchanged. `newlines
keep` additionally preserves each original CR/LF byte; a line comment stops
before them. The lexer still decides whether the preserved line ending is a
token. This is a lexical policy, not equivalence of keep/hide languages.

**Progress and termination.** An outer noncomment step advances one byte.
A selected opener advances its positive length. Every block step consumes a
positive delimiter length or one byte; every line step consumes one byte until
CR/LF/EOF. Nesting changes a counter, never calls a recursive parser. Depth is
at most the number of consumed opening delimiters, hence at most `n`. A block
that reaches EOF with nonzero depth returns damage, never clean acceptance.
Failed custom-opacity probes do not advance the main cursor, but each probe
transition and each outer dispatch/comparison/mask write consumes finite host
work. Work exhaustion returns resource status before a further charged action.
Thus termination follows from advancing source position or finite work; this
does not prove that a finite budget suffices for successful parsing.

**Cost boundary.** Delimiter dispatch/comparison is conservatively
`O(m*n*d)` plus masking. Default doubled-quote probes cost `O(m*n)` over
complete quote intervals. General regular opacity may have arbitrarily long
unsuccessful prefixes at successive positions (`a* b` on `a^n`), producing
quadratic probe work in this straightforward coordinator. It is not certified
linear merely because its individual recognizer is a DFA. `scan.work` charges
those transitions and bounds an attempted pass; the independent adversarial
regression distinguishes exhaustion from syntax/unterminated-comment errors.
The downstream lexer/parser has its own pass allowance and existing cost
obligations. No wall-time or resident-memory theorem is inferred from fuel.

**Opacity admission.** The existing regular builder refuses nullable,
recursive and unsupported pattern roots. The coordinator additionally refuses
same-position delimiter/shielding prefix competition. This sufficient check is
conservative, not a complete test for every lexical-mode design. Complete
longest matches of an admitted custom pattern are opaque to its form; incomplete
custom matches are not silently repaired. Default broken Rexx quotes retain the
existing EOF-opacity policy so the string lexer owns their damage.

**Recovery.** Strict and record-recovery requests use the same prepared bytes
and original source owner. Separators inside valid comments/strings are absent
from the token stream; only explicitly retained comment newlines remain visible.
An unclosed target block maps to unrecoverable status and cannot be executed.
The specification reader's candidate-inspection restoration remains failed and
cannot publish a grammar. These are bounded profile contracts, not globally
optimal repair or general nested/lexical-mode recovery proofs.
