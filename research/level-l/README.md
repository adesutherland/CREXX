# Level L lexer and parser research

For the current product implementation, start with the
[Level L user guide](../../lib/rxfnsl/doc/user-guide.md) and its runnable
settings-language example. The
[diagnostic and recovery assessment](../../lib/rxfnsl/doc/diagnostics.md)
distinguishes product behavior from the research engines below.

The proposed [cREXX C/G compiler plan](../../lib/rxfnsl/compilers/PLAN.md)
defines separate compilers under `lib/rxfnsl/compilers/`, written in cREXX with a
shared extensible Level L IR, C/G semantic extensions, optimisations and RXAS
backend. It records the two-frontend PoC and
the later production-replacement/bootstrap gates. STEP02 foundation implementation
is authorised; the existing compilers remain unchanged. The
[STEP01 review](../../lib/rxfnsl/compilers/doc/STEP01-REVIEW.md) now supplies the
paired C/G scope, source trees, generic IR/extension API, algorithms and checked
source examples. It also records the native B seed -> Level L -> proper B ->
verified B/L rebuild -> G/C compiler-host sequence, with proper B self-hosting
tracked explicitly under STEP07B.

**12 October continuation:** resource settings now cover construction and
recovery, pinned Unicode 17 properties grow the same settings grammar, and
located diagnostics plus lexical/UTF8 recovery have new native regressions.
CONT-AC01–04 pass for their bounded scope; full LANG and original research
criteria remain open. See the [continuation receipt](evidence/continuation-20261012/README.md).

**12 October teaching increment:** the user guide now explains backend choice,
complete versus speculative paths and explicit Earley forests with runnable
language examples. A complete Pratt calculator includes its Level B evaluator,
build commands and syntax/value diagnostics. Modern RexxScript is documented
as the larger tree-interpreter example, with a source-based migration inventory.
The [teaching receipt](evidence/teaching-20261012/README.md) records 808 runtime
checks and 16 expected build refusals. The
[generalized-comment contract](SURFACE.md#generalized-comments-proposal--12-october-2026)
is approved and implemented; current qualification is tracked in PLAN.md and
the comment receipt below.

**12 October generalized comments:** shared `line`, `block` and `nested`
forms, custom delimiters, optional regular shielding and kept newlines are now
implemented in the common source layer and both emitters. The eighth settings
lesson grows the existing Unicode/field/Pratt example. Backend, ownership,
recovery, budget and bootstrap evidence is retained in
[evidence/comments-20261012/](evidence/comments-20261012/README.md). This is a
bounded increment; the complete LANG/research programme remains open.

**11 October safety checkpoint:** the maintainer has requested a commit and
remote backup on `origin/levell`. This preserves the implementation and retained
evidence for review. Bootstrap checkpoint 1 is locally verified; the common
surface programme and original research obligations remain open as recorded in
[PLAN.md](PLAN.md). This branch backup does not constitute integration or release.

The [expanded general language-engineering programme](PLAN.md#expanded-programme-proposed-for-approval)
was approved on 10 October 2026 and is in progress: shared authoring semantics, several native
algorithms, composite stages, mathematical certificates and usable diagnostics.
cREXX B/G/C is one case study. The pilot is complete; the expanded implementations and open obligations
are tracked separately in PLAN.md.
The long-term direction includes migrating cREXX's AST and its invariants onto
the shared Level L infrastructure; this programme will test that foundation
and produce the staged migration proposal.

Local branch `levell`, 10 October 2026. The requested research now has executable
native generators, a working Level L re2c wrapper, actual Pika reference probes,
positioned diagnostics and a lex-first cost/equivalence analysis.

The [accepted authored surface](SURFACE.md) is the existing Rexx-like `.levell`
syntax. Its unchanged bootstrap AST now feeds both the retained direct
re2c/cREXX emitter and the new native DFA backend, with logical token/span/EOF
and diagnostic agreement. Numbered regex and PEG files below are internal
research inputs; they do not replace that authored surface.

The main conclusion is conditional: **lexing first is very effective on the
tested long lexical atoms, and useful Pika diagnostics can be built. The pinned
Pika implementation is not a universally linear or semantically unrestricted
replacement for the current front end.**

The [plan and acceptance record](PLAN.md), [consumer audit](SOURCE-AUDIT.md) and
[mathematical analysis](MATH.md) own scope, source observations and proofs.
The work remains experimental and unmerged. The remote safety checkpoint above
supersedes earlier local-only delivery statements; compiler/frontend migration
and release are separate decisions.

## Expanded implementation checkpoint

**Latest status:** STEP-01–07 are complete. The expanded native portfolio,
composite consumers and recovery comparisons execute; STEP-14/15 remain open
for corrected lifecycle measurements and final reconciliation. The first timing
block had a request-release attribution error and concurrent host compilation;
it is inconclusive. The corrected harness passes four correctness cells. See
[COST.md](COST.md), the current step table in [PLAN.md](PLAN.md), and the draft
[support recommendation](REPORT.md). Earlier checkpoints below retain their
historical state; the plan's current audit is authoritative.

Native shared libraries now include revisioned grammar/source trees, common
analysis and backend admission; packrat and LL controls; canonical LR(1)/LALR;
Earley with packed forests; Pratt/operator lowering; reverse maximal-munch
scanning; faithful Pika and Squirrel cores; complete byte-domain c14 recovery;
and an exact bounded weighted byte-edit oracle. `llproject` supplies a generic
named-node/arity tree pass. Scripts and sources are under this directory.

The new pruning results are deliberately separate:

- c14 frontier: 237 cases, one improved rank, no worse ranks, 42 unchanged clean
  trees. A local dominance theorem supports the experiment; global optimality
  is unproved.
- Pika longest-choice: repairs mixed growth but changes PEG acceptance in both
  directions. It is a retained rejected control, not the default backend.
- Byte-edit oracle: 480 independent finite-language distance comparisons pass.
  Its metric differs from c14's structural/evidence ranking.

All counts cover optimized/no-opt builds on both concrete VMs. The provisional `llfront` now runs a complete configuration format and a token
list from Rexx-style declarations. `llstage` generates finite-context token
passes; `llregional` executes checked embedded Pratt regions inside LR and
compares them with the unspecialized whole CFG. A representative B/G grouping adapter now checks AST migration, and generated
Boolean-attribute schedules pass 4,096 graph/seed combinations. AC-12, AC-13,
AC-16 and AC-26 are verified for their stated research slices; the wider
corpus/diagnostics and lifecycle/cost work remain in progress. See PLAN.md
for the current acceptance audit and evidence/tranche-4 through tranche-6.

## Initial-pilot implementations

| Implementation | What it actually does | Current boundary |
|---|---|---|
| [Native lexer generator](tools/lexgen.crexx) | Compiles regex specifications into Thompson NFA tables; optional native subset construction creates DFA tables with byte-alphabet equivalence classes. Generated `.crexx` modules scan into kind/start/length records. | ASCII regular-expression subset; no Unicode property compiler, tags/captures, start-condition API, DFA minimization or streaming refill. DFA construction has a 2,048-state research budget. |
| [Native parser generator](tools/parsegen.crexx) | Compiles named grammar rules, literals/classes, token terminals, sequence, choice, grouping and repetition into executable Level L recognizer modules. Runs expression, statement, nested-array, direct/indirect-recursion and failure fixtures. | Pika-inspired positive subset with dense grow-only scheduling. Predicates, AST/actions, precedence annotations and general left-recursive PEG conformance are unimplemented. Mixed recursive alternatives have a demonstrated limitation. |
| [Level L re2c front end](tools/re2cwrap.crexx) | Translates the same lexical spec into re2c input, generates/compiles a C scanner, and calls it through `ADDRESS CREXX` with argv and redirected text. Token/span output agrees with the native scanner on the shared fixture. | External build dependency and process transport; no RXPA in-process provider. This C control reserves NUL as its sentinel. The comparison domain is ASCII without embedded NUL. |
| [Actual Pika probe](cases/PikaProbe.java) | Compiles and runs the unmodified reference, adding an independent anchored diagnostic explanation and identity traversal of reachable match objects. | Pins `8050ceaf70c79a04416fdbdc374e92f9478de7b0`; observations apply to that source. The explanation prototype handles positive clauses, not general predicates or minimum-edit repair. |
| [Shared authored-surface adapter](tools/surfacegen.crexx) | Reuses the existing `.levell` parser/AST unchanged and lowers its TinyExpr grammar to the native DFA backend; also reruns the established direct re2c/cREXX backend as a control. | Native backend supports the documented ASCII subset, preserves EOF and named unmatched diagnostics, and rejects unsupported profile/pattern/action features explicitly. See SURFACE.md. |

The normal lexical spec produces a **105-state NFA (15,120 table bytes)** and a
**26-state, 16-class DFA (1,840 table bytes)**. The latter reads one table
transition for each inspected byte. Those sizes are deterministic generated
table sizes, not total executable/image sizes or measured RSS.

The lexer specification is `token-id regex`, one rule per line. Negative IDs
skip trivia; positive IDs are emitted; zero and nullable rules are rejected.
Supported regex operators are concatenation, `|`, grouping, `*`, `+`, `?`,
ASCII classes/ranges and complements, `.`, escaped metacharacters and `\n`,
`\r`, `\t`. Earlier rules win equal-length ties; longer matches win first.

The parser specification is `Name <- expression`, one rule per line. The first
rule is the root. `@N` is a terminal input symbol (a token kind or ASCII value).
Single-quoted strings, character classes/ranges/complements, references, `/`,
sequence, `()`, `*`, `+`, `?` and empty literals are supported. Undefined rules,
nullable repetition and unsupported predicates are rejected. Nested arrays use
the experiment's single-quoted literal syntax; this is not a full JSON parser.

## Pika findings that affect a selection decision

The reference accepts normal arithmetic and productive indirect recursion. It
also reproduces the mixed-operator defect documented in
[upstream issue 32](https://github.com/lukehutch/pikaparser/issues/32):

```text
Sum <- Sum '+' Term / Sum '-' Term / Term
x=a+b-c;  -> rejected
x=a-b+c;  -> accepted
```

Factoring the operator choice to `Sum <- Sum ('+' / '-') Term / Term` repairs
those two fixtures. Another cyclic grammar from upstream's own tests only
matches three of four characters. The native reduced engine also demonstrates
the mixed-operator limitation and the factored repair. These are reasons to
keep a grammar conformance corpus before claiming unrestricted PEG support.

For `A <- A 'a' / 'a'` on `a^n`, the actual reference creates **n(n+1)** match
objects; all remain reachable from retained memo entries. At `n=256`, it retains
65,792 match objects behind only 767 memo keys. Construction work and the live
match graph are therefore quadratic for this family. The paper's linear claim
cannot be used as an unconditional property of this implementation.

PEG ordered choice is not CFG ambiguity handling. In particular, Pika is not a
drop-in substitute for a GLR/Earley parse forest. Its Java character positions
and this experiment's byte positions also need an explicit Unicode source-map
adapter before they can be interchanged outside ASCII.

The newer [Squirrel parser](https://github.com/lukehutch/squirrelparser) and its
[2026 paper](https://arxiv.org/html/2601.05012) are relevant alternatives. Source
at `bb1ae84701060462636c07c814286ac2875e0568` was inspected, including its detailed
development lessons. The expanded programme now executes that pinned Dart core
and its separate c14 recovery engine using a local Dart 3.13.5 SDK. The native
`llsquirrel` core agrees on 486 byte-domain cases, including complete canonical
success/failure histories and memo/expansion counters, in optimized/no-opt mode
on both VMs. This is bounded conformance evidence, not universal equivalence.
Its overlapping-expansion adversary has n(n+1)/2 expansions at the tested sizes.
The native c14 recovery port now agrees on 237 reference cases. Its separate
frontier experiment improves the known pruning counterexample; full cost and
semantic qualification remain open.

## Diagnostics and recovery that ran

The reference returns later statement matches and uncovered ranges. A separate
anchored explanation pass on that same memo table identifies the actual missing
operand and right-parenthesis positions. Including permissive recovery rules
such as `Number` in the coverage set can hide complete root-parse failure; an
empty gap list is not proof of valid input.

The native prototype produces contextual messages and repair hints, including:

```text
line=1 column=5 unexpected=;
expected=identifier integer string '('
hint=supply the missing operand before the statement separator

line=2 column=7 unexpected=; expected=')'
hint=insert ')' before the statement separator

column=5 unexpected=<end of input> expected=')'
hint=close the parenthesized expression
```

The damaged two-line fixture retains two subsequent valid statement islands and
reports two errors. Recovery here uses statement separators and designated
statement matches. It has not proved minimum edit distance, intended semantics,
scope-safe recovery for every nested construct, or freedom from cascading errors
in an arbitrary grammar. Native and reference explanation passes can include
speculative continuation alternatives; naming/ranking those alternatives is
part of turning a prototype into polished diagnostics.

## Lex-first observations

The [proof obligations and counterexamples](MATH.md) matter as much as the
timings. Hard keyword classification changes accepted identifier contexts;
maximal munch can disagree with PEG priority; even a DFA can repeatedly rescan
suffixes and perform quadratic token-boundary rollback.

The five-case panel compares the **same dense native recognizer algorithm**
with a scannerless character grammar versus a generated DFA and token grammar.
Every pair checks identical acceptance, including the malformed input. One
warmup and six balanced serial pairs ran per case on each concrete VM. Token
totals include lexing and symbol conversion. Compile/startup and AST work are
excluded. This is a small research screen with a deliberately simple baseline,
not a Java Pika speedup, product benchmark or formal Release verdict.

| Case | Bytes / tokens | rxtvm character / token-total median µs | rxbvm character / token-total median µs |
|---|---:|---:|---:|
| Short assignment | 8 / 8 | 395 / 112 | 365 / 105 |
| Long identifier | 519 / 8 | 33,307 / 141.5 | 33,827 / 143 |
| Long string | 645 / 4 | 28,913.5 / 94.5 | 31,384 / 92 |
| Long comment with statement-like contents | 687 / 8 | 33,392.5 / 147 | 33,715 / 139 |
| Malformed statement | 12 / 11 | 546 / 143.5 | 512.5 / 134 |

For the long identifier, dense memo slots fall 49,400 → 306 and clause
evaluations 295,165 → 1,088. Removing lexical grammar clauses and spurious
interior positions explains the very large ratio against this baseline.
An optimized sparse parser could shrink that ratio substantially. Do not
extrapolate these numbers to a full language, an AST-producing implementation,
Java, an in-process re2c provider, or general Pika performance.

Raw samples and Level B-generated summaries are in
[evidence/](evidence/). The timing image was rebuilt when additional functional
fixtures were added; its SHA-256 remained identical, so the retained samples
were reused.

## Reproduce

Use the existing toolchain by setting `CREXX_BIN_DIR` to a directory containing
`rxc`, `rxas`, `rxlink`, `rxtvm`, `rxbvm`, `library.rxbin` and `classlib.rxbin`.
The scripts default to the neighboring main checkout's Release build. They
log builds to bounded temporary files and keep generated artifacts under
ignored `build/`.

```sh
research/level-l/run-native.sh opt
research/level-l/run-native.sh noopt
research/level-l/run-invalid.sh
research/level-l/run-re2c.sh
research/level-l/run-surface.sh opt
research/level-l/run-surface.sh noopt
research/level-l/run-reference.sh
# Only repeat the screen when its inputs change or a new comparison is needed:
research/level-l/run-measure.sh
```

`CREXX_RE2C` overrides re2c's path; `CC` overrides the C compiler.
`PIKA_SOURCE_DIR` can point to an existing checkout at the pinned commit;
otherwise the reference script clones it into ignored `build/`.
The Java source compile needs a JDK. No Maven/test dependency installation is
needed for these bounded probes.

Functional native output, controlled invalid-spec rejections and re2c output
are retained for both VMs. Native generation/recognition ran in optimized and
no-opt forms, exercising `rxc`, `rxas`, `rxlink` and the VMs. Host, versions,
binary/source hashes and sampling policy are in
[provenance.json](evidence/provenance.json). Existing development binaries
identify older dirty build revisions; their hashes pin the actual tools used.
This does not qualify the current source-base commit or any release. The
platform evidence is local macOS ARM64 only.

The native lexer/recognizer counters and memo table use module-level last-call
state. Repeated-call reset and switching generated grammars are exercised;
queries must be consumed before another parse overwrites them. There is no
concurrent/reentrant instance API yet. A production library needs explicit
owned source, token, parse and diagnostic contexts, like the current RexxScript
evaluator. No public Level L package was installed by these scripts.

## Ranked next experiments and maintainer decisions

1. Use a real RexxScript expression corpus and Pascal/PL/M lexical adapters.
   Preserve exact source/trivia/soft-keyword contracts and compare existing
   outputs, AST/value behavior and diagnostics before choosing a backend.
2. Extend the now-executed comparison with the **existing direct re2c/cREXX
   syntax backend** behind the shared authored `.levell` frontend. Official
   [re2c syntax-file documentation](https://re2c.org/manual/manual_c.html#syntax-files)
   supports custom backends. The retained overlay uses `--lang python` and
   cREXX templates; a complete frozen backend can use `--lang none` rather than
   inherit generic templates. Compare real consumer grammars before selection.
3. Test the Squirrel reference against the same mixed/indirect/nullable/priority
   corpus and adversarial growth families. Compare a simple Pratt/LL parser for
   RexxScript with Pika rather than presuming the more general engine wins.
4. Prototype boundary/kind-directed Pika seeding and sparse parent scheduling,
   then count all live tree versions, not just memo keys. Preserve ordinary PEG
   priorities and make unsupported recursive shapes explicit.
5. Add AST/CST projection and grammar labels to diagnostics; rank anchored
   expectations and prove any insertion/deletion repair policy separately.
6. Decide whether the first product slice is generated native DFA code, a re2c
   syntax backend, or a checked RXPA scanner provider. Unicode, lexical modes,
   resource budgets, instance ownership and cross-platform packaging remain
   production acceptance work requiring a separately approved plan.

The experiments support proceeding with token/span tooling and diagnostic
infrastructure. They do not justify replacing cREXX's current re2c/Lemon pipeline
with this reduced parser or the pinned Pika implementation as it stands.

## Expanded recovery comparison: retained successes and failures

Tranche 11 compares the same 40 nested-array inputs, frozen as 27 development
and 13 held-out cases. Seven are clean and 33 damaged. Every recognizer agrees
with an independently written source recognizer. Complete optimized/no-opt
outputs are identical on both concrete VMs, including diagnostic presentation.
The panel is small; it is not a user study or an engine-wide quality ranking.

| Control | Whole-input result on 33 damaged cases | Later untouched arrays, development / held-out | What the result means |
|---|---|---|---|
| Ordinary PEG farthest failure; explicit PEG labels; LR expected tokens | 33 rejections each | Not measured as a recovery result | Located explanations; no edits or resumed tree are promised. |
| Explicit PEG label recovery | 22 recovered, 11 rejected | 13/19 and 7/10 | Declared synchronization and error nodes; cheap in this control, but the authored policy misses useful recoveries. |
| LR local repair | 31 complete edited token sequences, two local successes with later errors | 19/19 and 10/10 | Unit insertion/deletion; all minimum local candidates retained, then progress-ranked. |
| Pika chart and anchored explanation | 33 rejected roots with available islands | 19/19 and 10/10 | Useful undamaged structure remains discoverable; it is not an edited full parse. |
| c14; separate frontier experiment | 33 full-span recovered error analyses each | 19/19 and 10/10 | Structural ranking, missing/skipped nodes and explicit recovered status. This is not the LR edit objective. |

The independent insert/delete BFS finds a cost-one repair for `[a],a];[a]`
by inserting `[` at the beginning. The local LR control costs three because it
has already committed an accepted prefix. This is a concrete reason to offer
bounded rollback or a wider repair mode, rather than claim local optimality is
global optimality. On `[x];[a]`, the simple PEG label policy complains about a
closing bracket; Pika's chart explanation also offers item starts. Label
placement and synchronization rules require design, not just a nicer renderer.

The frontier experiment has the same selected trees/cost outcomes as c14 on
this panel, while its counters rise from 42,916 to 46,722 work units and 30,307
to 33,747 allocated records. These are implementation counters, not timings or
comparable units across engines. Its previously retained pruning counterexample
still improves; that one improvement does not establish a universal quality win.
The broader lexical-damage and consumer diagnostics assessment remains open.

The research frontend renders stable error categories, the specific label,
source excerpts/carets and related opening delimiters. Full findings and the
restricted semantics are in MATH REC-09/10 and the exact outputs in tranche 11.
Reproduce with `research/level-l/run-recovery.sh MODE recovery`, where MODE is
`opt` or `noopt`. The frozen corpus and independent oracle are generated by
`cases/recovery-corpus.py`; its content check prevents silent replacement.

A second frozen-tool discrepancy was exposed by exact output comparison:
optimized diagnostic rendering changes a dynamically constructed code string
in the retained reproducer. Simple accessor-only reductions do not reproduce
it, so the underlying cause remains unproved. The research renderer uses a
stable category and the parser's independently correct full label message;
all four resulting outputs agree. `run-diagnostic-compiler-repro.py` retains
both outcomes and hashes. This is an open finding in the frozen research tools,
not a claim about current develop and not a production compiler repair.
