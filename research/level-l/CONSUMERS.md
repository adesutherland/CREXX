# Consumer evidence and boundaries

This supplements the single authoritative PLAN and SOURCE-AUDIT. These are
source-derived subsets, not replacements or qualification of Mike's languages.
Tranche 13 executes 347 cases, frozen into development and held-out partitions,
with an independent Python regex lexer and recursive syntax readers. Native
Thompson/DFA generation, reverse maximal munch, declared token stages, packrat,
settled reverse tabulation and canonical LR agree on acceptance. All four
optimized/no-opt and concrete-VM outputs are identical. Scanner generation is
byte-for-byte deterministic across the two compiler modes.

| Fragment | Fixtures | Executed distinctions | Explicitly outside this fragment |
|---|---:|---|---|
| Pascal | 60 | Case-insensitive BEGIN/END classification after neutral identifiers; assignments, nested blocks, arithmetic tiers, decimal/exponent/hex atoms, doubled quotes and adjacent `#13` string pieces; three comment forms | Full declarations/types, control flow, standard conformance, string/value decoding and range expressions. Nonclosed comments/strings are rejected strictly; this is not an assertion about every current handwritten error path. |
| PL/M | 75 | DO/END, BYTE declarations, assignment, nested blocks, `$` inside identifiers/numbers retained in raw spelling, radix-spelled numeric atoms, doubled quotes and block comments | Macro expansion/source stacks, compiler-control lines, complete identifier alphabet, numeric range/radix validation, machine semantics. Keyword classification here requires their plain spelling; embedded `$` in a keyword needs an additional spelling adapter. |
| BASIC | 92 | Numbered lines, nested FOR/NEXT, LET/assignment, PRINT lists, suffix-bearing identifiers, D exponents, doubled quotes, apostrophe comments and EOL tokens | Contextual procedure/type headers, REM, other literal radices, arbitrary dialect typing, name matching of FOR/NEXT and execution. The lexer fragment requires explicit final newlines; it does not model the complete physical-line loader. |
| Lisp | 34 | Recursive lists, quote, dotted tails, numeric/symbol atoms, strings, case-preserving bar symbols and line comments | General reader macros, quasiquote/unquote, vectors, nested block comments, the complete symbol alphabet, conditions or evaluation. |
| Prolog | 86 | Facts/rules, recursive terms/lists, list tails, quoted atoms versus variables, anonymous-variable spelling, clause termination, two comment forms | Operator tables, queries, escapes/value decoding, variable identity allocation, term semantic restrictions, unification or resolution. |

The pinned owning files and hashes are already inventoried in
`evidence/mike-sources.json`. The selected bodies were re-read at those revisions:
Pascal `tokenize`, PL/M `lexinto`/`numval`, BASIC `tokenize_line`, Lisp
`readobj`/`readlist`, and the checked-in Prolog `PTokenize`/`ParseClauseTerm`.
References supply the design constraints; the whole external interpreters were
not built or executed in tranche 13. The independently executed oracle is the
explicit subset reader in `cases/consumers-reference.py`.

Two failed controls are retained in `evidence/consumers-first-failure`:

1. Optional line numbers before both an ordinary BASIC line and NEXT caused a
   genuine LR(1) construction conflict. The declared finite-state stage now
   removes the leading number from the parser stream while raw positioned tokens
   and original source remain owned. Both parsers see the same normalized input.
   This is a concrete use for composition; no claim that BASIC inherently needs
   a more general parsing algorithm follows from the first grammar's conflict.
2. Nullable wrappers can extend a raw parent to the next token start, thereby
   including trailing comment trivia. The selected syntax observation explicitly
   uses the hull of consuming terminal leaves. Those named extents agree with
   the independent readers. The adapter does not alter the stored raw trees or
   prove all their spans are minimal; source/trivia ownership and structural
   invariants are checked separately. Production tree contracts must name this
   distinction. Full arbitrary tree equivalence is not claimed from named spans.

Prolog full stops add another small boundary obligation: they terminate before
layout, `%` or EOF. A checked post-lexing condition rejects `p.q.` rather than
silently splitting it into two clauses. A future reusable lookahead/boundary
declaration could express this; the current consumer adapter is explicit.

These fixtures exercise lexical and syntactic damage, including unterminated
quotes/comments, deletion/substitution, high-byte ingress refusal and NUL.
They do not yet compare all recovery families on these consumer languages or
constitute a user study of diagnostics. The generated legacy DFA profile is
7-bit; the common byte/source model and its separate binary tests are wider.

Run `research/level-l/run-consumers.sh opt` or `noopt` from this checkout.
The default tool path is the frozen research toolchain, not the concurrent
develop build. Full commands, inputs and results are bound in tranche 13.

## RexxScript source-to-tree consumer

**Target update:** Adrian now selects a modernized, initially class-free
RexxScript as the first product, with Level G expression rules and RexxValue
storage. The following tranche14 observations describe the old evaluator;
they are not acceptance requirements for its successor. Preserve them as
historical controls and classify intentional changes explicitly. PLAN.md
RS-D01–12 owns the pending product decisions.

Tranche 14 executes the owning `RexxScriptEvaluator.crexx` on 50 frozen inputs.
Three read-only getters expose its existing error code, statement and column;
its parsing/evaluation bodies are unchanged. Existing selected-worktree
`rxfnsc`/`classlib` binaries are snapshotted and hashed separately. This is an
actual evaluator control with stated dependencies, not qualification of an
exact product revision. Optimized/no-opt results agree on both concrete VMs.

A native generated lexer, contextual SAY classifier, shared authored grammar
and three parsers feed the same delayed tree visitor. On 48 in-domain cases,
packrat, reverse tabulation and LR agree with the owning evaluator on syntax,
values, output and selected semantic-error codes. All four complete outputs
agree. IF-block and SIGNAL examples also run through the reference, but remain
explicitly outside this assignment/SAY tree consumer. Broader initial-AST
control flow belongs to C08. No sandbox/host write-back compatibility is claimed.

The observed expression profile matters:

- `1+2*3` produces `9`; operations share one left-to-right tier.
- Both `'a' 'b'` and `'a''b'` produce `ab`, without Classic blank-concatenation
  insertion. `12x` with exposed `x=7` produces `127`.
- Unary minus, integer division, nested LENGTH/SUBSTR calls and omitted
  arguments are exercised. Ordinary grouping parentheses are rejected by this
  owning evaluator; call parentheses are accepted.
- Missing terms, unknown variables, type errors, division by zero and selected
  BIF argument failures stay distinct. The demo visitor implements only this
  tested integer/string/LENGTH/SUBSTR subset, not the complete BIF/type/sandbox
  service. It must not become a second unmaintained Rexx runtime in a product.

The initial `(statement separator+)* statement?` shape caused an LR conflict;
factoring the common statement prefix into a right-recursive remainder admitted
the same frozen corpus. This is a grammar-encoding finding, not evidence that
RexxScript intrinsically requires PEG. A product grammar manipulator should
present the named-tree reconstruction and its preconditions with this rewrite.

An unterminated quote raises a signal from the frozen library's `qstripcomment`
before the evaluator returns its usual structured error. The original uncaught
outcome is retained in `evidence/script-reference-first-failure`; the reference
harness then catches and records it so subsequent fixtures execute. The native
scanner returns a located lexical failure. Neither the production evaluator
nor its library was changed. Other reference columns are expression-relative
and one-based, whereas native spans are absolute zero-based source bytes; the
comparison prints both instead of pretending the coordinates are interchangeable.
Raw numeric expected-token lists remain less friendly than grammar labels.

Reproduction: run `cases/prepare-script-reference.py`, `run-script-reference.sh`
and `run-script-consumer.sh` under `research/level-l`, choosing `opt` or `noopt`.
The preparation script refuses changed binary dependencies/source rather than
silently treating a different build as the retained control. The frozen corpus,
expected observations, generated scanner and source/tool hashes are in tranche 14.

## B/G/C initial-tree correspondence

Tranche 15 captures twenty actual compiler `STAGE_RAW` observations and executes
their native source-to-tree controls in all four cells. Sixteen clean raw trees
and four syntax-error cases agree through the closed-corpus projection. It
preserves node kind, value, order, expression nesting and branch/template
structure; it omits the enclosing program/options wrappers. The debug dump has
`(-1,-1)` raw locations, so this does not claim independent compiler-span parity.
Native source ownership and tree-span invariants are checked separately.

Classic cases include assignment and label priority for `if`, expression uses
of `if/then/else`, blank/abuttal concatenation, nested comments, decimal abuttal,
comma continuation, nearest-ELSE nesting, PARSE UPPER VAR, dot placeholders,
dynamic template patterns and incomplete IF/PARSE/comment/string input. B/G
cases cover continuation, nested comments, arithmetic precedence and a final
clause without a newline. Earlier tranche 6 supplies explicit/implicit procedure
grouping and declaration reconstruction. This remains a representative corpus:
the new Classic grammar requires clause boundaries before ELSE, covers a small
PARSE VAR template subset and does not implement every contextual-word mode.
B/G specimens use the common assignment/return/arithmetic subset; the Classic
statement grammar is not advertised as a complete B/G grammar.

The common components are a parameterized nested-comment counter stage,
generated DFA, declared comma/newline normalization, assignment/label guards,
contextual word classification and ordinary PEG parsing in two implementations.
Positive-next-token guards are expressed through the existing stage IR; their
proposed authored spelling still needs design. Original bytes remain the owner
of every token and tree, although the scanner sees an equal-length masked copy.
The parser's explicit EOF closure represents the final clause boundary. An
attempt to invent an empty source token was correctly rejected by the common
stream contract and is retained in `evidence/rexxfront-first-failure`.

Classic raw trees distinguish `'a''b'` (one quoted string) from blank
concatenation and preserve abuttal across nested comments. The B/G raw grammar
initially constructs OP_SCONCAT and has a later gap-refinement pass; observations
must name the stage rather than treating all representations as the same AST.
PARSE templates also remain structured syntax; executing their variable updates
belongs to a later consumer service.

The compiler snapshot run has no Classic runtime imports. Its Classic clean
cases reach raw parsing/lowering and subsequently report missing class/function
imports during later checks. Those errors and exit codes are retained; they
are not counted as syntax rejection or concealed as a compiler pass. This
qualification observes initial syntax only. The four intentionally malformed
Classic cases have actual raw ERROR nodes and corresponding native rejections.

**Frontend recommendation from these observations:** keep explicit dialect
profiles and a composite source/token/tree pipeline. A conflict-free CFG backend
is the low-risk compatibility baseline for existing B/G and Classic grammars;
ordinary PEG is also useful for authored frontends with deliberate priority,
provided tree and keyword contracts are checked. The executable PEG prototypes
do not establish that PEG is universally more natural, that all compiler fixups
can be inferred, or that the production frontends should switch algorithms.
RexxScript's selected grammar now runs with both PEG and LR, so its service API
need not expose one engine's internals. Keep effects and BIF/host policy after
tree selection. Pika/Squirrel still require their separately stated recursive
semantics and cost/recovery evidence before a consumer chooses them.
