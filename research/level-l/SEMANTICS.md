# Level L research semantics, revision 1

Approved programme: [PLAN.md](PLAN.md), STEP-08 onward. This is a provisional
research contract, not adopted product syntax. The existing lexer surface and
its longest-match/source-order semantics remain authoritative.

**11 October product decision:** PLAN.md LANG-D01 selects DFA, LL(1), LR(1),
ordinary packrat PEG, Pratt and Earley initially. This document retains the
research mechanisms and their exact limits; selection does not silently expand
their implementation domains. The next common product contract is proposed in
[SURFACE.md](SURFACE.md#common-surface-unicode-and-composition-review--11-october)
and qualified under LANG-AC01–14, including easy contextual messages,
progressive authoring and composition.

## Observable contract

A specification declares a semantic profile independently of its backend:
`ordered` means ordinary PEG recognition; `contextfree` means CFG alternatives;
`squirrel` names the pinned reference's recursive seed-growth policy; `pika`
names the pinned reference scheduling/selection semantics. Recursive variants
are not aliases for ordinary PEG. A shared admitted fragment may be evaluated
by several engines only when the intended observations agree.

Observations are: full-input acceptance, selected named-production tree or
explicit forest, ordered terminal leaves, original byte spans and trivia,
error/recovery status, and declared resource-exhaustion status. Numeric internal
node IDs and table layouts are not semantic. A grammar rewrite must carry a
projection back to authored production names. A recognition-only theorem does
not establish tree or diagnostic equivalence.

Recognition is pure. Tree construction records immutable alternatives and
selects a root; visitors execute after recognition. Consumer actions whose
state influences parsing must declare that state in the context/memo identity
and invalidate any certificate whose assumptions they violate.

## Initial executable IR

The first graph uses stable one-based clause handles and named rule wrappers.
Kinds: terminal 1; sequence 2; ordered choice 3; CFG alternatives 4; rule 5;
empty 6; repetition 7; negative predicate 8; positive predicate 9; EOF 10;
inclusive terminal range 11; any terminal 12; literal byte string 13;
one-or-more 14; optional 15; enumerated/inverted byte set 16. Extended kinds
retain the Squirrel reference's literal frontiers and combinator identities.
The earlier PEG control rejects them until lowered with a reconstruction map.
Sequence/choice operands are
handles; rule wrappers reference a body; terminals/ranges carry symbols.
All mutations increment the grammar revision and invalidate analyses.

The first alphabet is integer symbols 0..255, with EOF separate (256 only in
FIRST/FOLLOW analyses). It serves bytes and small token alphabets. It is not
the Unicode product contract; wider-symbol capability must reject until added.
An independent token stream retains source byte spans and the original binary
buffer. EOF cannot be confused with a NUL byte or token kind zero.

The source-tree arena records ordered edges, original spans and production
identities, without compiler AST dependencies. Child handles refer to older
arena entries, making a DAG. Alternative histories are allowed; only nodes
reachable from the chosen root are the selected tree. Structural checking is
revision-bound. This is a checked invariant, not a proof of consumer semantics.

`llhistory.arena` separately owns recognition attempts and failure histories.
Alternatives may overlap; a failed child's frontier may go beyond the prefix
its parent accepted. Its invariant is valid source positions and an acyclic
edge order, not the selected-tree invariant. The native Squirrel core retains
all allocated history entries until request disposal; this is not the Dart
reference's garbage-collected live-object accounting. Selected-tree projection
and dead-history compaction are separate obligations.

## Admission and certificates

Structural validity, possible nullability, FIRST/FOLLOW approximations and
left-corner reachability are distinct facts. Predicates are conservatively
possibly nullable. A cyclic left-corner graph prevents admission to ordinary
packrat and LL(1); recursive profiles require their own treatment. The shared
analyser reports nullable repetition; each backend decides whether its
semantics can handle it. The initial PEG/predictive engines reject unproved
progress rather than silently accepting zero progress. CFG alternatives cannot
be executed as ordered choice without a checked conversion.

A certificate records grammar revision, backend/profile and assumptions. It
certifies the checked grammar property conditional on the engine's stated
correctness contract, not formal machine verification of the complete cREXX
compiler/runtime. Mutation invalidates it. `unsupported`, `not proved`,
`construction limit`, `invalid input`, `recovered` and `resource exhausted`
remain distinct outcomes.

### Analysis ownership and plugin contract

The design has three responsibilities, clarified in response to Adrian's
analyser/plugin question:

1. `llcheck.analysis` computes facts over one owned grammar snapshot: structure,
   may-nullability, FIRST/FOLLOW approximations, left-corner reachability,
   possible empty loops and conservative prediction conflicts. It contains no
   backend-name dispatch. Algorithms can request additional reusable analyses
   as the research grows; a fact is not itself a backend rejection.
2. Each implementation supplies `llcap.backend`: name, version and
   `inspect(facts) -> decision`. The provider owns semantic restrictions,
   engine-specific constructions and justified cost claims. For example, an
   LR plugin will construct its tables; the central analyser should not
   simulate every engine or maintain a hardcoded list of all algorithms.
   `llpeg.engine` is the initial provider for its three evaluator modes.
3. `llcap.admit` coordinates structural validation, plugin dispatch and an
   optional requested bound. Its structured result distinguishes supported,
   unsupported, not proved, invalid structure and analysis-construction limit.
   A supported grammar with unknown cost can execute under a resource policy;
   requesting a linear-input certificate for it produces a build-time
   not-proved result. A bound never silently changes the selected semantics.

The eventual composite planner must check boundary obligations in addition
to accepting the individual plugins: semantic profile, consumed unit and end
positions, context dependencies, result multiplicity, tree projection and
diagnostic ownership. Individual engine admission is necessary, not sufficient
for admitting a composition. A caller that needs every CFG result cannot
replace that contract with a single PEG endpoint.

Certificates identify the exact serialized grammar, root and revision plus
backend version, bound and assumptions. They remain conditional on the named
backend implementation. Third-party plugin declarations are claims until a
trusted checker or reviewed proof rule validates them; this interface alone
does not make arbitrary plugins sound. The research contract is not yet a
stable external plugin ABI.

## Initial corpus policy

[CORPUS.json](CORPUS.json) freezes workload purposes and semantic expectations
before implementation comparison. Concrete fixtures and independent expected
trees will be versioned before timing. Generated short-input enumerations and
mutations use deterministic seeds; held-out mutations are not used to tune
recovery. Performance evidence must identify complete inputs and semantics.
The corpus spans general tooling; cREXX is one migration case study.

## Executed composition and transformation contracts

The authored research frontend lowers named parser rules to this shared model;
its supported spelling and historical differences are catalogued in SURFACE.md.
`llstage` has explicit finite-context, bounded-lookahead, first-matching-rule
semantics. Every output stream owns original bytes and a map into its input
stream. Contextual keyword meaning is a declared stage, not an assumption
hidden inside a parser plugin.

`llregional` admits delimiter-separated Pratt/operator regions inside an LR
outer grammar. Its checker proves the boundary alphabet conditions and builds
both the specialized tables and the unspecialized whole CFG. Matching an API
signature alone never authorizes replacing an arbitrary CFG region with one
selected PEG result. COMP-01 states the exact substitution theorem and limits.

`llproject` supplies named-node/arity selection. `llgroup` supplies declared
marker/member partitioning, pure reconstruction and exact/composite/synthetic
origins. These generic libraries import no compiler AST declarations. The B/G
legacy adapter is a consumer comparison; it does not change their contracts.
The current ordered source-tree arena is not the universal mutable semantic IR
for hoisting, code duplication and later control lowering.

`llschedule` owns scheduling for an explicitly declared Boolean-join equation
profile. It generates a DAG order when available; an actual dependency-cycle
witness explains why a requested DAG schedule fails. Its automatic mode can
select a least-fixed-point worklist because all admitted operations have a
finite-height monotonicity proof. Nonmonotone/callback operations reject. This
is a useful example of central generic analysis plus a provider's algorithmic
proof obligations, not an automatic proof of arbitrary compiler validation.


## Local LR recovery contract

`lllrrepair` searches from the first parser error without changing the already
accepted prefix. It returns all minimum-cost local edit scripts in its completed
search bucket and selects a candidate by forward parsing distance. A local
result has status 1 even if the whole edited token sequence parses; `complete()`
is a separate observation. Fuel/storage failures are status 2, invalid requests
3, and no repair within the declared cost bound 4. Unprocessed input is retained
verbatim as tokens. Edited token IDs are not source spelling for a general
lexer: the request retains original bytes and insertion/original-token mapping.
This research control has neither global repair optimality nor a CPCT+ speed
qualification. REC-08 states the actual shortest-path and cost assumptions.

## Diagnostic controls and frozen comparisons

`lllabel` adds explicit throw/recovery annotations as a request-level research
API. The ordinary profile without annotations retains its meaning. Annotation
placement can change clean acceptance and requires its own proof or admission
policy; it is not silently inserted by the common analyzer. Predicates suppress
labels/recovery, and recovery expressions may not recursively recover labels
in this control. Recovered error spans belong to the source tree; arbitrary
correction text is not synthesized. REC-09 records outcomes and limitations.

`llpikaexplain` interprets a completed positive-clause chart without changing
recognition or choosing an edit. Explanations and island coverage are different
observations. A diagnostic frontend may combine an engine's observations with
source excerpts, carets, grammar names and related delimiters. The array corpus
renderer demonstrates that presentation; its bracket tracking is a consumer
adapter, not a universal lexer or delimiter inference service.
