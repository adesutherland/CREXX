# Level L language-engineering strategy

**Research and implementation report for the `rxlang` review branch.**
The 11 October remote safety checkpoint preserves this work for review;
integration, full programme completion and release remain open in PLAN.md.
On 11 October Adrian selected **DFA, LL(1), canonical LR(1), ordinary packrat
PEG, Pratt and Earley** for the initial supported toolkit. LL(1) is explicitly
selectable and Earley is an initial requirement. Product integration and
qualification remain open under PLAN.md, LANG-AC01–14. The common surface,
Unicode policies and composition/recovery declarations are being reviewed in
[SURFACE.md](SURFACE.md#common-surface-unicode-and-composition-review--11-october).
Adrian also requires a gentle learning curve and easy bespoke, context-aware
messages: these are core surface acceptance requirements, with optional advanced
Unicode, backend and composition controls.

Adrian has selected **modernized RexxScript for Level G applications** as the
first product consumer, initially without script-defined classes. Its included
expressions will follow a selected Level G profile while using RexxValue.
The old evaluator's behavior is retained as historical evidence, not as the new
language contract. The decision ledger is in PLAN.md, RS-D01–12.

The existing RexxScript consumer uses **DFA + deterministic LR**, with declared
token/tree stages; Pratt remains an optional specialization for that consumer.
This does not reduce the six-component initial toolkit requirement. Pika and
Squirrel remain experimental; research implementations of the traditional
engines still need product integration and qualification.

The accepted language baseline uses NUMERIC_COMMON Level G expressions,
RexxValue values and implicitly established script-local names. Variable/type
declarations, SIGNAL, GOTO and INTERPRET are unavailable. Classes and collections
are outside the initial subset. Adrian clarified that the BIF target is
**Level C**, already connected to RexxScript through rxfnsc. Reuse that linkage
and its shared RexxValue call context, argument-presence and error contracts;
the modernization changes expression parsing/evaluation without replacing the
existing function library. Level G expressions and Level C BIF contracts are
separate deliberate choices.
Host registration, exposure, value transport and write-back remain explicitly
for later confirmation. These pending interfaces do not block the grammar and
isolated execution proof.

The selected toolkit uses one Rexx-like declaration family, explicit grammar
semantics and checked backend selection. DFA, LL(1), LR(1), ordinary PEG,
Pratt and Earley each have a distinct initial role. Keep Pika, Squirrel and
c14 as named experimental options; ALL(*) is reserved and IELR remains later
table-compaction work. Those choices do not close the original research audit.

[PLAN.md](PLAN.md) is the scope/status authority. [AUDIT.md](AUDIT.md) maps every
required native algorithm and mathematical obligation to evidence;
[CONSUMERS.md](CONSUMERS.md) states the useful-language subsets;
[RECOVERY.md](RECOVERY.md) reports the frozen/held-out comparisons;
[MATH.md](MATH.md) contains proofs, conditions and counterexamples. Cost/lifecycle
results are in [COST.md](COST.md). Local branch `rxlang` preserves the production
checkout and existing syntax. Research completion is distinct from shipping.

## Answers to the strategic questions

**The analyser should be shared; admission belongs to the plugin.** A central
service computes grammar identity, structure, nullability, FIRST/FOLLOW and
dependencies. Each backend owns its additional checks and constructions, such
as LR states, PEG progress or Pika event dependencies. A coordinator checks the
requested semantics/bound and reports a located unsupported/not-proved/budget
result. The composite planner additionally checks component interfaces. A
plugin's unsupported result means this lowering cannot serve the request; it
does not mean the language is impossible to parse. Third-party certificates
need a trusted checker; merely implementing the interface is insufficient.

**PEG is natural when priority is part of the intended rule.** It gives direct
rules for ordered exceptions, lookahead and lexical-style descriptions. It also
makes a successful short alternative irrevocable: `('a' | 'ab') 'c'` rejects
`abc` under ordinary PEG and accepts it under CFG union. Rule reordering and
extensions can change acceptance. CFG is often clearer when alternatives mean
all possible syntax, especially while reverse-engineering a language or seeking
ambiguity witnesses. The consumer tests justify offering both. They do not
establish a universal preference among programmers; that would need a user study.

**Pika's implementation is feasible, but its advertised power needs a precise
semantic target.** A faithful native core reproduces508 ranked reference cases;
native lowering has977 Java comparisons, and the non-context-free language
control has707 independent cases. Lookahead enables an ordinary PEG to recognize
`a^n b^n c^n`; Pika does not create that expressive power. Its chosen recursive
policy has observed mixed-choice, start-assertion and empty-input boundaries.
Those cannot be repaired by scheduling alone without specifying the desired
semantics. Claims about PEG versus CFG language classes do not automatically
transfer to Pika's operational recursive extension. POWER-01/02 distinguishes
our local theorem from current external separation claims and their assumptions.

**Two different refinements have different verdicts.** Pika root-dependency
pruning is supported by a local event-trace proof and508 retained-history
comparisons. Its longest-choice experiment changes ordinary PEG acceptance in
both directions and is rejected as a semantics-preserving optimization. The
c14 repair frontier fixes an actual ranking reversal, but its local dominance
theorem is not a whole-search optimality proof. On newer damage panels it adds
work, supplies no later-structure gain, and changes one held-out tree at equal
cost away from the known original mutation. More candidate information can
help, but a better scalar score alone is insufficient.

**Automatic composites are useful when the intended stages are declared.**
Finite-context token passes, nested-comment masking, structural grouping,
operator regions and attribute schedules execute here. Dependency analysis can
choose DAG evaluation or a proved finite-height monotone worklist. It cannot
infer what an incomplete grammar's arbitrary handwritten fixup was meant to
accomplish. Production rules must state context, source mapping, purity,
termination measure and the result expected by the next stage.

## Recommended algorithm portfolio

Bounds below concern completed runs and fixed admitted grammars unless stated
otherwise; `a` is produced tree/forest size. Generation, source copying and
consumer work are separate. Every native row has a working restricted research
implementation, not a shipping support commitment.

| Candidate | Meaning / admission | Bound supported here | Diagnostics / maintenance | Recommendation |
|---|---|---|---|---|
| Native DFA | Regular byte rules, longest match, winning priority | Reverse scanner q*n transitions and O(q+n) DP entries; ordinary scanner linear if bounded backup certified; subset construction can be exponential | Shortest winning-rule examples, unmatched spans; modest runtime, generator work | Core lexer; default to proven bounded backup or explicit reverse buffer mode |
| Thompson NFA | Same regular contract | State-set simulation; rollback may repeat suffixes | Simple independent control, larger scan cost | Retain control/reference backend |
| re2c / direct cREXX emitter | Existing authoring contract and supported re2c features | Qualified shared subset only; do not transfer native proofs to all re2c features | Mature external generator, dependency and output-adapter maintenance | Retain optional backend; native generator does not yet replace all re2c functionality |
| LL(1) | Disjoint prediction and consuming progress | Linear input for fixed grammar; current control uses memo/tree storage | Clear FIRST/FOLLOW conflicts; low algorithmic complexity | Selected initial explicit restricted backend; product integration open |
| Pratt | Declared operators, precedence/associativity and delimiters | Linear token work plus tree | Contextual atom/operator errors; small runtime | Selected initial expression specialization; product integration open |
| Canonical LR(1) | Conflict-free pure CFG tables | Linear token actions plus tree; potentially large generation | Expected sets and substantive local repair; moderate/high generator complexity | Core CFG baseline |
| LALR | Same CFG when merged tables remain conflict-free | Linear runtime; smaller tables possible | Reject merge-only conflicts with LR witness, never silently resolve | Optional compact lowering |
| IELR | Full LR(1) recognition with reduced inadequacy | External Bison control only | More generator maintenance than canonical LR | Later core table optimization; no native port claimed |
| Ordinary packrat PEG | Pure normalized clauses, no same-position recursion/nullable loops | O(g*(t+1)+a), constant-cost normalized terminals | Farthest/context labels; priority surprises need witnesses | Core PEG baseline |
| Settled reverse PEG | Same ordinary PEG on admitted dependency order | O(G*(t+1)+a), G includes terminal lengths | Complete reverse table useful for inspection; buffer cost | Specialist alternative with proven semantics |
| Pika reference / reachable-root | Pinned ranked recursive operational profile | Conditional quadratic input bound; linear memo keys do not bound histories linearly | Islands and chart explanations, limited semantic guarantees; high maintenance | Experimental specialist; no universal linearity claim |
| Squirrel core | Pinned greedy seed growth, distinct from ordinary PEG on recursive cases | Expansion/work accounting; general certificate unknown | Reference failure histories; recursive semantics and retained attempts need care | Experimental recursive PEG option |
| c14 / frontier | Structural damage model; byte input; distinct ranking policies | Fuel-bounded run; no general useful cost/optimality theorem here | Rich error structures, more state and ranking complexity | Experimental recovery; retain faithful control |
| Earley + packed forest | Pure general CFG, ambiguity and nullable cycles | O(g²*(t+1)³+a) work; current dense storage O(g*(t+1)²*max(g,t+1)) | Ambiguity witnesses and forest inspection; substantial memory | Selected initial general CFG service; product integration and storage qualification open |
| LR local repair | Bounded positive token insertion/deletion around actual LR stack | Minimum over completed local search, not whole file | Explicit edit scripts/origins; later errors remain visible | Candidate core recovery service |
| PEG labels | Explicit committed expectations and recovery expressions | Bounded execution; current nonmemoized control has no linear certificate | Most direct grammar-owned wording; misplaced labels can change language | Candidate diagnostic API with annotation checks |
| Exact weighted repair | Finite byte edits + whole-target CFG recognition | Exact within completed positive-cost bound; potentially exponential search | Small independent oracle, explicit corrected text | Testing/research only |
| Composite planner | Certified token/tree stages and delimited specialization | Sum of admitted costs plus mappings; generic rewriting unknown | Attributes errors to stages; proof/interface maintenance | Core infrastructure, restricted proven transforms first |

The present research LR generator caps512 canonical states; common analysis
caps512 clauses. Recursive routines generally cap512 call depth, authored
expressions128, and several dense indices default to8,000,000 cells. These are
research resource policies, not language definitions or safe production capacity
claims. Resource failure is explicit. Wider token alphabets and iterative/sparse
representations are required for broad product scale.

## Other candidates required by the plan

These are literature/reference comparators, not additional native ports.

| Candidate | Useful role and limitation | Decision |
|---|---|---|
| GLL | CFG generality with parser control closely following grammar structure; cubic designs exist, but practical indexing/forest construction matters | Strong next comparator if Earley's footprint or grammar-debugging needs warrant another general engine. [Scott/Johnstone](https://pure.royalholloway.ac.uk/en/publications/modelling-gll-parser-implementations/) |
| Correctly bounded GLR | Natural extension of an LR service, retaining ambiguity through graph stacks/packed forests | Consider after LR/Earley contracts stabilize. Do not call every GLR implementation cubic: Bison documents exponential worst cases for its current simpler representation. [Bison GLR](https://www.gnu.org/software/bison/manual/html_node/Generalized-LR-Parsing.html) |
| Adaptive LL(*) | Practical top-down prediction with runtime analysis; ANTLR's paper gives O(n⁴) worst-case and handles non-left-recursive CFGs, with selected direct recursion rewritten | Valuable interoperability/reference route, but duplicates core coverage and adds prediction/cache machinery. Published practical speed is not a local result. [ALL(*)](https://www.antlr.org/papers/allstar-techreport.pdf) |
| Regex derivatives | Alternative regular-automata construction; attractive compositional rule representation | No unique required capability missing from native NFA/DFA yet; investigate when Boolean regular operations justify it. [Owens/Reppy/Turon](https://people.cs.uchicago.edu/~jhr/papers/2009/jfp-re-derivatives.pdf) |
| CFG derivatives | General parsing with memoization, fixed points and compaction; subsequent work establishes a cubic treatment | Research comparator, not a shortcut to a simple fast implementation. [Adams/Hollenbeck/Might](https://arxiv.org/abs/1604.04695) |
| Scannerless generalized parsing | Joint lexical/syntactic ambiguity and modular embedding; explicit disambiguation filters matter | Useful when a unique tokenization cannot be certified. Earley over bytes supplies a basic local control; no full SDF-style workbench port. [SDF3](https://spoofax.dev/references/sdf3/), [disambiguation](https://spoofax.dev/references/sdf3/disambiguation/) |

## Common surface and build behavior

Retain `symbol: kind qualifiers = definition`, named rules/tokens and Rexx
comments. The established lexer surface is unchanged. This executable parser
example is a **provisional extension for approval**, not adopted product syntax:

```rexx
options levell
namespace tokenlist
read: parser semantics contextfree input tokens start document
item: token = 1
comma: token = 2
document: rule = item (comma item)*
```

The consuming library selects LR or Earley; both preserve the declared CFG.
Changing `semantics contextfree` to `ordered` is a language decision, not an
optimization. A product-facing `backend auto` spelling is not adopted here.
The automatic service may choose only among admitted equivalent implementations.

A declaration that yields an LL conflict should report the source rule,
competing alternatives and a witness token, then offer LR/PEG only with the
necessary meaning explanation. A LALR-only conflict should recommend canonical
LR/IELR without claiming the grammar is ambiguous. An unsupported Unicode or
streaming request must fail at construction rather than silently narrow input.
[SURFACE.md](SURFACE.md) contains runnable pass examples and a rule-repair
walkthrough; runtime internals need not appear in ordinary language-tool UI.

## B/G/C frontend recommendation and migration

Use a composite initial-AST frontend with explicit dialect profiles. Keep a CFG
compatibility baseline while a PEG alternative is compared on intended choices.
Classic Rexx needs neutral words and contextual classification; B/G share some
lexical stages but need their own structural/type-facing rules. The historical
RexxScript control uses left-to-right arithmetic. Adrian's newly selected
modernized RexxScript instead follows the chosen Level G expression profile;
the old oracle must not define that new target. Keep dialect/version contracts
explicit rather than changing them to make a backend easier to use.

```mermaid
flowchart LR
  S[Owned source bytes] --> L[Declared lexical profile and nesting stage]
  L --> T[Positioned tokens and trivia]
  T --> N[Declared normalization and contextual words]
  N --> P[Admitted CFG or PEG parser]
  P --> A[Shared source tree and diagnostics]
  A --> R[Declared structural passes and provenance]
  R --> I[Dialect initial AST contract]
  I --> C[Compiler semantic services]
```

The comparison covers soft assignment/label words, abuttal/blank concatenation,
nested comments, continuations, clause endings, IF/ELSE, PARSE fragments and
procedure grouping. Raw compiler dumps do not supply independent spans;
Classic snapshots also report later missing-runtime imports. CONSUMERS states
these limits. Full compiler replacement is neither attempted nor implied.

Migrate in stages: first source/token/tree ownership and diagnostics; then
initial AST builders and pure structural passes; then typed sidecar attributes,
symbol/type dependency schedules and certificate consumers; finally semantic
IR/control/ownership transforms with their distinct invariants. Keep the legacy
adapter only while comparing old/new behavior. Generic facts belong to Level L;
dialect/runtime meanings belong to the compiler consumer. SOURCE-AUDIT maps
owning files and remaining lifecycle, import, exit and semantic obligations.

## General language-engineering dimensions

| Dimension | Executed capability | Remaining product work |
|---|---|---|
| Generation and libraries | Native regex/grammar generators, authored AST adapter; reusable owned parser/scanner classes | Stable API/ABI, installation, generated module naming/versioning and cross-platform qualification |
| Lexer only | Standalone native/re2c controls, priority/spans, NUL in generic byte DFA | Unicode/property compiler, tagged captures, modes/refill and broader re2c feature coverage |
| Existing tokens | Independent positioned token streams feed LR/PEG/Earley/Pratt | Token kinds beyond0..255, external protocol and validation contracts |
| Grammar reuse | Named rules/namespaces, common IR, explicit regional composition | Grammar imports/parameterization/versioned reusable grammar modules; no module-resolution service claimed |
| Trees/captures/trivia | Named productions, generic source trees, selected projection, original byte retention and origin maps | Arbitrary captures/tags, richer typed nodes and full-fidelity trivia attachment conventions |
| Binary/Unicode | Binary/NUL tests, byte locations, raw malformed input; existing UTF-8 surface rejects unsupported native patterns | Broad scalar/property/normalization semantics and decoder mapping; byte tests are not Unicode qualification |
| Streaming | Complete-buffer APIs only; reverse scanners/charts require retained input | Refill/checkpoint/end-of-stream semantics and proofs; no streaming parser advertised |
| Incremental/editor reuse | Request-owned contexts and retained islands; no edit-reuse API | Dependency invalidation, versioned source edits, stable node identity and measured reuse |
| Embedding | Certified delimiter/operator region, original source mapping, unsafe boundary rejection | Multiple arbitrary language modules and CFG endpoint lattices; arbitrary PEG substitution is not safe |
| Inspection/transformation | Complete config flattening, consumer visitors, declared grouping/provenance, attribute schedule | General bidirectional formatting and effectful rewrite systems with their own contracts |

Missing modes are not silently emulated. The current frontends accept only their
listed units/qualifiers/backends; unavailable streaming/incremental product
services have no advertised entry point. A future common service should give
first-class capability diagnostics for those requests.

## Decisions requested after research review

1. Initial minimum selected: DFA + canonical LR with declared stages, optional
   Pratt specialization; retain the explicit semantics/profile model. Qualify
   the actual product grammar and implementations against that selection.
2. Select the exact parser/pass surface using SURFACE.md's concrete examples;
   preserve the established lexer spelling.
3. First consumer **selected: RexxScript**, modernized toward a reduced Level G
   language. Resolve its expression/value/statement/host/version contracts using
   PLAN.md RS-D01–12 before production implementation. The complete small format
   remains a useful independent toolkit test.
4. Select whether Pika/Squirrel recovery deserves the next proof/engineering
   investment. The decisive next work is continuation-aware dominance and sparse
   owned storage, with held-out reconstruction tests; a new headline score is
   insufficient evidence.
5. Authorize production integration, broader platform QA or publication separately.

These are concrete later adoption decisions. None is required to keep the
currently approved local research and final acceptance audit moving.
