# Accepted Rexx-like Level L surface

**Current status, 11 October:** the declaration style is accepted and the
bootstrap/product subset is implemented. Adrian has now selected DFA, LL(1),
LR(1), ordinary packrat PEG, Pratt and Earley for the initial toolkit. Adrian
subsequently approved implementation of this proposal, with one growing worked
example. The short headers, backend selection, three message fields, operator
declarations, one Pratt/LR region profile and bounded flat-record recovery now
have product routes under qualification. The five settings increments and
their actual limits are in the user guide. Pinned Unicode properties/comparison
keys, richer captures, general pipelines and the complete recovery contracts
below remain open; approval of the proposal is not delivery evidence.
PLAN.md, LANG-D01–10 and LANG-AC01–14,
owns decisions and acceptance. The later sections retain historical research
evidence; their old capability limits are not the current product reference.
Use [the user guide](../../langtools/doc/user-guide.md) for what runs today.

## Common surface, Unicode and composition review — 11 October

### Start small and grow the same grammar

Adrian explicitly requires a simple surface, optional advanced capabilities
and a gentle learning curve. The component inventory below is an architecture
map, not a list of concepts that every beginner must learn or declarations
that every grammar must contain. The first tutorial should show useful input,
a small grammar, the resulting named tree and one understandable error. Easy
bespoke messages with the right context are part of this first lesson, not an
advanced capability hidden behind recovery or tree customization.

A **candidate beginner example** for the single setting `port = 8080` is:

```rexx
options levell
namespace one_setting

name: token
number: token
equals: token

scan: lexer
  letter: charset = 'a' to 'z' | 'A' to 'Z' | '_'
  digit: charset = '0' to '9'
  space: charset = ' ' | U+0009 | U+000A | U+000D
  name.lex: rule = letter (letter | digit)* then emit name
  number.lex: rule = digit+ then emit number
  equals.lex: rule = '=' then emit equals
  whitespace: rule = space+ then skip

parse: parser start setting
  setting: rule = name equals number
```

This is proposed syntax, not a runnable replacement for the current tutorial:
the shortened lexer/parser headers are not yet implemented. Token IDs remain
automatic. The character ranges intentionally describe this small language;
valid UTF-8 input does not imply that every Unicode scalar is a legal name.

Recommended defaults to review: a lexer without an input qualifier reads UTF-8
scalars; a parser without a profile/backend qualifier is a full-input CFG parser
using canonical LR(1) over the sole connected token stream. Return the existing
useful named tree and located diagnostics without requiring captures, a tree
schema or authored error labels. Preserve fail-on-error behavior; opt into
analysis recovery explicitly. Pin these defaults to the surface version and
record the effective configuration in the generated artifact. Do not silently
fall back to PEG or Earley when LR admission fails.

The corresponding advanced declarations remain available:

```rexx
parse: parser semantics contextfree input tokens start setting backend ll1
parse: parser semantics ordered input tokens start setting backend packrat
```

These are alternative examples, not two definitions to place in one file. A
backend override and a semantic-profile change are different operations; the
tool must explain the distinction. Default CFG versus explicit profile selection
is a review decision, not approval inferred from the current consumer backend.

Introduce optional capabilities only when the worked language needs them:

| Author's next need | Small addition to the same grammar |
|---|---|
| More statements or alternatives | More named rules and ordinary grouping/repetition |
| Clearer application tree | Named captures; optional node/list shaping |
| Expressions | One operator table, with the backend hidden unless relevant |
| More helpful wording | One rule-bound message with simple context placeholders; part of the beginner surface |
| Later errors in the same input | Optional explicit safe recovery boundaries |
| International identifiers | Selected Unicode property classes and, separately, an optional comparison policy |
| Contextual keywords / embedded syntax | A token stage or named parser region, then an explicit pipeline if needed |
| A particular algorithm or bound | Backend and resource/admission options, with source-level explanations |

Stable convenience does not require concealing behavior. Provide an optional
explanation view of the effective settings and inferred stage connections.
More capable examples should add a few local declarations; they should not
replace the beginner notation with algorithm-specific rules or mandatory IR
schemas. Stage contracts can usually be derived and checked from declarations,
with explicit annotation required only where inference is insufficient.

Keep named tokens initially, including punctuation, until a short literal form
has an unambiguous binding contract. Automatic literal tokens are a possible
later convenience; guessing precedence, keyword reservation or longest-match
priority would be a language change rather than harmless abbreviation.

### Bespoke messages are an easy core feature

Build on the existing named `diagnostic` declaration by allowing it to attach
directly to a parser rule. This **proposed spelling** needs no custom schema,
capture or recovery configuration:

```rexx
value: rule = number | string
value.error: diagnostic rule value =
  'Expected a number or quoted string; found {found}.'
```

For a settings input `port = ;`, the intended display is:

```text
settings.txt:1:8: Expected a number or quoted string; found ';'.
  port = ;
         ^
```

This is a design example, not output from the current product. `{found}` is
supplied by Level L and rendered safely as the actual unexpected text or
`end of input`. The declaration name supplies a stable diagnostic identity.
Filename, ranges, excerpt and caret are automatic. The message is optional;
without it the normal generated explanation remains useful.

The same mechanism should work for a named expectation inside a sequence when
the surrounding rule is too broad, without requiring the user to learn the
backend's states. Built-in context can include the rule/construct and expected
items. An author may later add a named capture or related opener to say
"Expected a value for option 'port'" or point to the opening parenthesis.
Validate field names at generation. If a capture has not been established or
competing parses disagree, use a defined generic fallback rather than inventing
the value or attributing another branch's context. Exact capture/fallback
spelling is a review detail; it must not complicate the basic one-line feature.

Messages describe the selected failure; they are not procedural throws. A
failed alternative that is legitimately bypassed must not report an error.
PEG lookahead/predicates suppress speculative diagnostic noise; Earley must
not arbitrarily borrow a context from one surviving alternative. Selection
should favor the relevant specific expectation once the failure is established,
while preserving structured evidence for related locations and debugging.
Authored message text is data, with no arbitrary Rexx evaluation or host effects.

Custom wording and resynchronization are independent: the author can add a
message without changing parser control flow. In particular, a diagnostic
attachment must not silently introduce a PEG cut or change accepted clean
inputs. The same message identity/context should remain usable when recovery
is enabled later. LANG-AC14 qualifies this across the initial backend set.

### One grammar description, explicit kinds of meaning

Retain `name: kind qualifiers = definition`, Rexx comments, doubled-quote
literals, named tokens/rules and dotted names. Keep Level L's own declaration
names ASCII and case-insensitive for this increment; a target language's
identifier policy is independent. Backends consume a checked common model,
including original declaration ranges, rather than each parsing its own dialect.

The proposed organization is:

| Declaration group | What the author describes | Result / responsibility |
|---|---|---|
| Source and text profile | Bytes versus Unicode scalars, pinned property version, malformed-input policy and line handling | Owned original source with exact byte ranges |
| Tokens, charsets and lexical patterns | Token names, regular character rules, trivia and explicit lexer modes | Tokens referring to source; DFA admission |
| Token stages | Continuations, contextual words and other declared stream rewrites | A new token view plus origin mapping |
| Parser rules and operators | Grammatical structure, choice semantics, precedence and associativity | Derivation tree, or an explicitly requested CFG forest |
| Captures and tree construction | Named children, lists, transparent helpers and application-oriented nodes | A stable named tree independent of table internals |
| Diagnostics and recovery | Construct names, expected phrases, related delimiters and safe restart boundaries | Structured diagnostics and an explicitly damaged result |
| Pipelines, regions and tree passes | How declared components connect and where a child parser owns input | A checked composition with source/result contracts |

Simple languages need only tokens, one lexer and one parser. The pipeline can
be implicit when there is exactly one unambiguous connection; composition is
an additional visible declaration when several stages or parsers are involved.
The exact default/selection spelling remains part of LANG-D05 review.

Keep semantic profile separate from implementation. The following is a
**proposed spelling sketch**, extending the existing parser declaration with
`backend` (not accepted by the current product frontend):

```rexx
parse: parser semantics contextfree input tokens start settings backend ll1
  settings: rule = (setting separator)*
  setting: rule = identifier equals value
  value: rule = number | string
```

Within the CFG profile, LL(1), LR(1) and Earley must preserve the grammar's
meaning. An explicit tree/forest requirement can still make a backend
inapplicable. Under `semantics ordered`, `|` retains PEG first-success choice;
ordinary packrat admits its supported progressing, non-left-recursive domain.
Lexer alternatives remain longest match followed by declared priority. Do not
silently reinterpret one choice policy as another or turn resource exhaustion
into selection of different semantics. Predicates are explicit PEG features;
operator specialization is admitted only for a matching expression contract.

Recommend retaining `|` within these explicit component profiles rather than
adding a second choice punctuation now (LANG-D07). Rule imports/reuse must keep
their profile; moving a rule across profiles requires an explicit checked
conversion. EBNF lowering must preserve named-tree shape as well as recognition.
PEG repetition is greedy/possessive; CFG repetition describes possible
derivations. Neither may acquire a hidden rule-priority policy from lowering.

In token grammars, recommend named tokens first. A quoted nonempty spelling
must either resolve through an explicit, unique token binding or receive an
error explaining which token name to use. It must never become a byte-valued
token ID. Keyword bindings must respect contextual classification; a spelling
does not make a keyword globally reserved. Both quote styles retain Rexx
escaping, not different implicit case policies (LANG-D08).

### Tree shape and expressions

Rule structure describes recognition; capture/tree declarations describe the
author-facing result. A proposed capture/tree sketch is:

```rexx
setting: rule = identifier as name equals value as contents
setting.shape: tree rule setting node setting fields name contents
```

`as`, `node` and `fields` here are review vocabulary, not current syntax.
`name` would retain the identifier token and its source span; `contents`
would retain the value subtree. Repeated captures form ordered lists, optional
captures retain explicit absence, and grouping helpers need not create visible
nodes. AST construction is declarative and pure; Rexx evaluation, type checking
and host effects remain subsequent consumer work. Preserve the existing raw
and named views through a versioned migration, not a silent child-position change.

One operator declaration set should define prefix/infix/postfix forms,
precedence and associativity. It can lower into CFG/PEG rule structure where
equivalence is established, or drive Pratt directly. Pratt must not become a
second, subtly different expression language. Mixfix forms such as conditionals
need explicit operand/delimiter contracts before admission. RexxScript keeps
the accepted Level G expression profile; adding tool capability does not change
that consumer's language or BIF policy.

An Earley forest is an explicitly different result from a selected tree. A
consumer may inspect alternative derivations or apply declared disambiguation;
a normal tree visitor cannot silently receive the first alternative. Nullable
cycles need cycle-aware forest traversal and bounded extraction, not an
assumption that every forest is a finite list of ordinary trees.

### Unicode: keep encoding, character classes and name comparison separate

The existing profiles remain authoritative:

| Profile | Matching unit | Required interpretation |
|---|---|---|
| `input utf8` | Unicode scalar | Match characters, lower to canonical UTF-8 byte DFA sequences where appropriate; exclude surrogate codepoints. |
| `input bytes` | Byte | Arbitrary binary input, with NUL distinct from EOF. |
| `input utf8bytes` | Byte after UTF-8 validation | Explicit compatibility profile for existing byte-oriented specifications. |
| `input tokens` | Named token kind | All parser backends share the token/source contract; token IDs are not character values. |

These are distinct from the encoding of the `.levell` file itself, which remains
UTF-8. Another target-file encoding belongs to an explicit decoder stage, with
byte-origin mapping, rather than reinterpretation of `.binary` as text. No
UTF-16/legacy decoder is made an implicit new prerequisite of this checkpoint.

Recommended policy choices for LANG-D03:

1. **Pinned properties.** Offer Unicode property sets, including `XID_Start`,
   `XID_Continue`, general categories and explicitly named whitespace sets.
   Use the project's pinned Unicode 17.0.0 data initially; record that identity
   in generated artifacts. Do not read whatever newer host tables happen to be
   installed or silently update the recognized language. A property's name is
   not a blanket promise of an entire Unicode-regex standard.
2. **Language-owned identifiers.** Supply an XID-based building block, with
   explicit additions such as underscore; do not mandate it for all languages.
   Level L's own ASCII declaration names and RexxScript's currently accepted
   identifier profile remain unchanged until separately selected for migration.
3. **Original spelling plus optional comparison key.** Keep source bytes and
   token spelling intact. The default is exact comparison. A language may
   explicitly request an NFC-normalized or case-folded identifier key, with the
   chosen algorithm/order/version stated. `é` (U+00E9) and `e` plus U+0301 can
   have equal normalized keys while retaining different source spans. NFKC
   compatibility normalization must never be a hidden substitute for NFC.
4. **Explicit case policy.** ASCII-insensitive, Unicode simple folding and
   Unicode full folding are different operations. Full folding can expand
   characters (`ß` to `ss`); it cannot be implemented as one-character equality.
   No dependence on the process locale and no automatic folding of strings,
   comments or the whole source. Unknown policies fail admission.
5. **Coordinates separate from display.** Original byte ranges are the
   authority. Structured line/scalar-column positions remain available;
   grapheme clusters, UTF-16 editor positions and terminal cells are separately
   named views. A combining sequence or emoji can span several scalars; a tab
   depends on display settings. Provide a deterministic escaped excerpt mode
   and explicit tab/display policy, without claiming exact layout in every font.
6. **No silent damage replacement.** Strict parsing rejects malformed UTF-8.
   Recovery may retain the offending bytes as an error region and resume at a
   checked boundary under the selected policy, without replacing them silently
   with U+FFFD or marking the result clean. BOM acceptance, line separators and
   whitespace are explicit policies; a Unicode property must not silently add
   new statement terminators.

Property sets can be compiled from pinned data into the existing sorted scalar
interval/UTF-8 lowering machinery. The Level B bootstrap can generate the
required data directly; it must not import the completed Level G facade.
Normalization/folding services, when selected, need reviewed bootstrap-safe
reuse of the existing private algorithms/data rather than duplicate executors
or a new G-to-L cycle. Diagnostic display must have a foundation-only path.
There is no proposal to move the existing valid-string Unicode executors onto
byte DFAs; this is lexical matching over source, a separate responsibility.

Relevant standards are the pinned
[Unicode 17 identifier guidance](https://www.unicode.org/reports/tr31/tr31-43.html),
[normalization specification](https://www.unicode.org/reports/tr15/tr15-57.html)
and [text segmentation specification](https://www.unicode.org/reports/tr29/tr29-47.html).
These inform the proposed profiles; they do not make arbitrary tokenization,
normalization or display choices equivalent.

### Composition belongs in the common surface

Recommend three explicit forms, using the existing research mechanisms as
implementation starting points:

1. **Sequential stages:** source/lexing, finite token normalization or context
   classification, parsing, then pure tree projection/transformation.
2. **Parser regions:** a parent delegates a declared input region to another
   parser, for example Pratt expressions within LR statements or an embedded
   language inside a template. The boundary grammar owns nesting, quoting and
   whether the closing delimiter is consumed by parent or child.
3. **Tree passes:** declarative field selection, grouping, flattening and
   structural rewrites, retaining original or explicitly synthetic origins.
   These operate on recognized structure; they are not arbitrary executable
   semantic actions hidden inside recognition.

An illustrative pipeline spelling, also **proposed**, is:

```rexx
read: pipeline = scan then layout then classify then parse then shape
```

Every name resolves to a declared component with checked input/output types.
Lexer-only tools, existing-token parsers and single-pass readers need not
declare irrelevant stages. The public graph is acyclic; existing bounded
nesting counters and parser recursion have their own contracts. Arbitrary
stage feedback/fixed points are not implied by this notation.

Region substitution requires more than a common calling convention. Check the
input unit, boundary/stop condition, lexical mode/context, full-region or
prefix consumption, result multiplicity, output schema, origin map and which
component owns recovery. A child cannot consume a parent delimiter while
searching for a restart point. An ordered parser's single endpoint cannot
replace all CFG alternatives unless the enclosing contract explicitly admits
that selection. Memoization keys must include any admitted context that can
affect recognition. Unproved optimization is rejected or left unspecialized;
the author does not receive a silently altered grammar.

Use the current finite token passes and tested Pratt-in-LR boundary theorem as
the first implementations. Add a declared embedded-language case, including
failed child parsing, and a tree-grouping example. Do not promise automatic
inference of existing arbitrary C/Rexx fixups. The original B/G/C migration
scope and host/type-service boundaries remain unchanged.

### Errors and resynchronization are part of the same contracts

Separate diagnostic descriptions from recovery control. A label such as
"expression after '='" improves wording; it must not secretly commit a PEG
choice. A recovery declaration attaches to a rule or region and names its
safe synchronization tokens, nesting context, insertion/deletion permissions,
and work/diagnostic limits. Exact spelling remains for review (LANG-D06).

The common result should distinguish clean success, recovered analysis,
invalid input, ambiguous CFG result, unsupported construction and resource
exhaustion. Completion and diagnostic presence remain separate observations:
a whole-file tree can still contain errors. Grammar build failures are not
target-language syntax errors. Recovery retains missing/skipped/error nodes,
diagnostic codes, original spans and related locations. No execution, host
actions or artifact replacement follows an error-bearing result.

The mechanisms can differ behind those declarations:

| Component | Initial recovery mechanism to qualify |
|---|---|
| DFA / lexical stages | Located error spans/tokens, declared string/comment boundaries and mode-aware restart; no lost source bytes. |
| LL(1) | Prediction/stack context, expected tokens and declared synchronization, with bounded local insertion/deletion where supported. |
| LR(1) | Stack-verified expectations, bounded local repair and declared synchronization fallback. |
| Packrat PEG | Contextual failures and explicit labelled recovery; suppress speculative/predicate noise and preserve clean ordered recognition. |
| Pratt | Missing operand/operator and delimiter context; return control at the region's owned boundary. |
| Earley | Chart expectations and explicit recovery boundaries/error productions with bounded work; preserve alternative repaired structures where required. No unqualified minimum-edit claim. |

Successful clean recognition and tree construction must be unchanged by enabling
diagnostic collection. PEG recovery annotations need particularly careful
controls because misplaced commitment can change acceptance. Test those cases
against strict parsing, not merely examples on which recovery looks plausible.

Resynchronize the generated Level L frontend itself at checked declaration
boundaries. An arbitrary colon inside a string/comment is not a declaration
start. An unclosed quote/group needs an explicit policy. Invalid declarations
must not trigger dozens of secondary undefined-name messages; no partial
grammar is emitted as a successful artifact. Keep the L0 seed small and
first-error-only while L1/L2 provide the product diagnostics.

Qualification is owned by PLAN.md, LANG-AC01–14. The existing diagnostic
findings LL-D01–06 stay open until their actual code and retained tests close
them. This discussion does not claim recovery implementation or universal
composition/Unicode equivalence.

## Historical surface provenance and research evidence

Adrian's 10 October direction is to retain the Rexx-like style used by the
existing re2c wrapper. The actual established frontend is on the `unicode`
checkout, pinned here to `9a86fb37c0d0d7b9cd395d676e8a0a3ef45e39b8`:

- `docs/ai-context/CREXX_LEVEL_L_SYNTAX.md` defines the authored contract;
- `docs/books/crexx_language_reference/level_l_syntax.md` is the human guide;
- `experiments/unicode/level-l-bootstrap/rxlang_bootstrap.crexx` parses and
  validates that contract into Level B token and AST objects; and
- `experiments/unicode/level-l-emitter/crexx.syntax` is the direct re2c/cREXX
  output adapter.

The declaration family is `symbol: kind qualifiers = definition`, with normal
`options levell`, `namespace`, Rexx comments, static dotted names and named
actions. [tinyexpr.levell](specs/tinyexpr.levell) is copied unchanged from the
pinned source, including these declarations:

```rexx
options levell
namespace level_l_tinyexpr

eof: token = 0
number: token = 1
ident: token = 2

scan: lexer input utf8
  ascii.digit: charset = U+0030 to U+0039
  integer: pattern = ascii.digit+
  number_literal: rule = integer then emit number
  end_of_input: rule = eof then emit eof
```

The full fixture also declares whitespace, identifiers, punctuation and named
malformed/otherwise diagnostics. Tokens are named in authored rules; assigned
numeric IDs belong to the validated lowering. Authors do not write C actions,
numbered regex rows or slash-delimited regex literals.

## Shared-frontend design

The original shared-frontend experiment preserved the bootstrap token objects,
structural AST, ID assignment, source spans and validation. The maintained
product now lives in `langtools/`; its generation-service tests own current
source/RXAS parity. The completed one-off surface comparison runner is retired.

Two backends consume the same parsed source:

```text
tinyexpr.levell
  -> existing Level L parser / validated AST
       -> existing re2c adapter / cREXX syntax backend -> generated scanner
       -> new surfacegen backend adapter -> native Thompson/DFA generator
```

The new adapter expands resolved ASCII charset/pattern references and bounded
repetitions into the native generator's internal regex format. The generated
surface wrapper preserves logical kind/start/length, emits the declared EOF
token once on success, and maps unmatched-input status to the source's named
diagnostic. Physical native records use 12 bytes; the retained re2c scanner's
physical layout remains its own implementation choice.
The differential test uses private `gsurface`/`gsurfaceraw` aliases so both
implementations can coexist in one linked image. A shipping backend must emit
the authored module identity and its selected public API; those aliases are
test infrastructure, not a change to `namespace` semantics.

Eight fixtures compare both generated scanners with the established TinyExpr
control: empty/trivia input, arithmetic punctuation, identifiers, leading-zero
numbers, an invalid prefix and valid non-ASCII unmatched input. Kinds, byte
spans, EOF and native/re2c status/offset/message agree on both VMs, in optimized
and no-opt builds. Token and structural-AST dumps are retained in evidence.

This proves a second backend for the established surface on the stated subset.
It does not redefine the existing UTF-8/Unicode contract. The new adapter
accepts the UTF-8 profile and ASCII non-NUL literals/ranges, unions, references,
concatenation/alternation, repetition and emit/skip actions. It rejects wider
Unicode pattern declarations, `scalar`, charset subtraction, the binary input
profile and ordinary error actions. Typed `.string` ingress supplies valid
UTF-8; raw malformed binary ingress is not implemented by this native adapter.
Those limitations remain backend obligations, rather than changed language
semantics. The retained re2c emitter also has its documented sentinel/whole
buffer limits; this proof does not upgrade its old qualification.
An otherwise valid specification with non-ASCII Unicode digit ranges is
explicitly rejected by the native backend in both build modes, with status 1.

## Parser-maker direction

The existing guide identifies the same declaration family as the extension
point for parser, nonterminal, precedence and production declarations. Adrian
has accepted the style. Exact future parser spelling, action semantics and
supported recursive grammar semantics still need a concrete language-design
decision. The `.peg` files are backend research inputs used to test algorithms;
they are not proposed as the authored Level L product syntax.

Preserve named tokens/rules and their source ranges all the way into grammar
diagnostics. Neutral IR, algorithm choice, automaton construction and generated
layout should remain independently replaceable behind this shared surface.

## Executable provisional parser and pass declarations

The approved expanded programme now exercises an additional native research
frontend, `llfront`. The established lexer bootstrap above is unchanged. This
extension is a concrete spelling for review, not adopted Level L product syntax:

```rexx
options levell
namespace tokenlist
read: parser semantics contextfree input tokens start document
item: token = 1
comma: token = 2
document: rule = item (comma item)*
document.shape: tree rule document kind 10 arity 0
```

`semantics ordered` gives `|` PEG priority; `semantics contextfree` gives it CFG
union. `semantics squirrel` explicitly selects the recursive operational
profile. Algorithms are chosen by the consuming tool, which must perform its
own admission check. A request for LR on ordered rules is rejected. There is
no silent semantics conversion. The byte-domain configuration example is
[config.levell](specs/config.levell); it includes forward rules, byte charsets,
quoted literals, nested arrays, numbers, strings and booleans. `!`/`&` prefix
predicates are provisional extensions; CFG backends reject them.

The frontend preserves case-insensitive ASCII names, case-sensitive quoted
literals with Rexx doubled quotes, static dotted names, Rexx comments, source
lines and original byte offsets. It lowers multi-byte literals, `+`, `?` and
byte sets into the common basic graph while retaining named rule wrappers.
Parser declarations currently have one root, no host actions, no imports and
no precedence annotation. `input bytes` means byte symbols including NUL;
`input tokens` means externally supplied IDs in 0..255 with original spans.
Unicode/scalar/streaming requests are rejected; none is silently treated as
ASCII. The lexer surface's wider Unicode contract is unaffected.

A `tree` declaration selects a named rule and checks the number of retained
children; unselected wrappers are transparent. It cannot truncate extra
children to satisfy a declared arity. Original bytes and source-node origins
remain owned by the result. Current binary semantic trees admit arity 0..2;
variable-length lists need a declared binary grouping or a later general-list
surface.

The staged fragment in [clause-stages.levell](specs/clause-stages.levell) also
uses the same declaration family:

```rexx
normalize: tokenstage start 1
continuation: transform stage normalize state 0 take comma newline emit skip next 1
terminator: transform stage normalize state 0 take newline emit semi next 1
classify: tokenstage start 1
clause.start: transform stage classify state 0 take semi emit copy next 1
keyword.say: transform stage classify state 1 take word text 'say' unless next assign emit sayword next 2
clause.body: transform stage classify state 1 take any emit copy next 2
```

State 0 matches any context; other states are 1..32. Declarations are tested in
source order, and the first match wins. One or two input tokens are consumed;
`skip` emits nothing, `copy` preserves the first kind, and a named token remaps
it. A two-token emission covers both original spans and the intervening source.
Unmatched tokens copy through without changing context. Text guards compare
ASCII case-insensitively; other byte sequences cannot satisfy such a guard.
`unless next` is a one-token negative guard. Every rule makes progress.

This is a finite-context example of clause normalization and soft-keyword
classification, not the complete Rexx rules for nesting, labels, abuttal,
comments, IF/THEN/ELSE or PARSE. Those remain separate corpus obligations.
EOF and a missing final clause terminator are not synthesized implicitly.

## Historical logical grammar: requirements still to reconcile

`docs/books/crexx_vm_spec/Logical-Grammar-Specification.tex` is an explicitly
human-interpreted proposal. It cannot be treated as executable PEG semantics.
Its initial Phase 3 suggestion of Pika is a candidate, now subject to the
reference counterexamples and the approved algorithm comparison.

| Historical convention | Current research treatment / unresolved product choice |
|---|---|
| Capitalized rule names imply lexical mode; global whitespace/comment rules hide trivia | Names are case-insensitive and input units are explicit. Source gaps survive. Automatic trivia insertion and mode boundaries still need a specified lowering. |
| `X*` is described as nongreedy and `X**` as greedy | Ordinary PEG `*` is possessive greedy repetition. The historical two forms require an explicit definition; the prototype does not pretend they are equivalent. |
| Single quotes imply case-insensitivity, double quotes sensitivity | The established lexer surface uses Rexx quoted strings. The provisional parser currently treats both as case-sensitive byte literals. A future explicit case qualifier is a review option, not adopted syntax. |
| `&\|` tests no whitespace or comment before a token | Original byte gaps are available, but a complete abuttal/trivia predicate and its lex-first certificate remain required. |
| Unordered sets and logical `//`, `&&`, `^` combinations | Each needs a precise recognition/tree meaning and a cost model. They are rejected by the current frontend, not approximated by CFG alternation. |
| Labelled sibling lists and arbitrary AST rearrangement | Named rule identity and checked binary tree projections execute. General list/capture/rewrite declarations and their invariants need a broader surface. |
| Recursion assumed for readability | Backends must admit the actual recursive profile. Ordinary PEG, a selected LR-PEG extension and CFG do not acquire identical semantics from matching spelling. |

These differences are an inventory for the final language-design decision.
They neither silently replace the historical proposal nor weaken the established
lexer contract. Declared semantics must be settled before automatic grammar
manipulation can be called meaning-preserving.


## Executed rule-repair walkthrough

With `identifier = [a-z]+` before `keyword.if = if`, both match the word `if`
at the same length. Earlier-rule priority selects the identifier. The native
DFA analyzer proves the keyword can never win, and the isolated keyword DFA
supplies the shortest concrete example. Moving the keyword first changes
priority; a contextual-word stage is the appropriate option when IF must also
remain usable as an identifier. Do not silently apply that language change.

For `document: rule = ('a' | 'ab') 'c'`, the bounded native witness finder
returns `abc`: the first alternative succeeds on `a`, leaving `b` where `c`
is required. PEG does not return to a successful choice when its caller later
fails. Reordering to `'ab' | 'a'` changes acceptance deliberately. A proposed
CFG union accepts `abc` too, but has a different declared meaning.

For `sentence: rule = sentence sentence | 'a'` under CFG semantics, the
shortest witness is `aaa`, with two complete bracketings `(aa)a` and `a(aa)`.
The claim is based on one shared forest for that input. Searching only through
length two returns 'none within bound', not 'unambiguous'. Choose an explicit
association/precedence or retain ambiguity according to the language design.

The canonical-LR/LALR example in `cases/lr.crexx` reports that merging caused
the A/B reduction conflict and includes the original rule locations. All four
full contexts `acd`, `bcd`, `ace`, `bce` work under canonical LR. The appropriate
first action is choosing the LR(1)-capable backend; arbitrarily deleting a
production would change the language. Automaton dumps remain optional detail.

These are executed diagnostic examples, not a measured user study. Source
line formatting, names for arbitrary token IDs and richer editor links still
need product presentation work.

## Generalized comments proposal — 12 October 2026

**Approved by Adrian on 12 October 2026; implementation/qualification is
tracked by COM-AC01–05 and EX-STEP05 in PLAN.md.** The contract below is the
approved direction; the user guide records the delivered syntax and admission
profile. Keep the existing small declaration family and add forms:

```rexx
shell.notes: comments line '#'
cpp.notes: comments line '//'
c.notes: comments block '/*' '*/'
rexx.notes: comments nested '/*' '*/'
```

`block` ends at its first closing delimiter; an opener inside it has no
special meaning. `nested` counts its own opener/closer pair. It is structured
nesting, not arbitrary grammar recursion. Several declarations may coexist,
but the two `/*` forms above are alternatives, not a valid combined example.
Line and nonnested block recognition is regular; nested blocks need a counter
or stack. Parser backend selection does not change that lexical meaning.

Approved contracts:

1. **Original source remains owned.** Mask only scanning bytes; never delete
   bytes or alter token origins. All comment text remains available through
   source gaps. Separate classified trivia records can be added under the
   already open source/trivia contract; this proposal does not promise a
   complete new trivia API.
2. **Line boundaries.** A line comment stops before CR, LF or the CRLF pair;
   the lexer sees that original line ending. EOF closes a line comment.
   Existing `comments nested` keeps its current default of masking internal
   newlines for compatibility. New `block` uses that same default. Optional
   `newlines keep` retains CR/LF in the scanning copy, allowing a newline-token
   language to count them deliberately. It changes tokenization, so it must
   be explicit rather than a formatter preference.
3. **Opaque strings.** The default remains current single/double quotes with
   doubled-quote escapes. An advanced optional `outside PATTERN` selects a
   named existing regular lexical pattern instead; reject unknown, nullable,
   recursive or nonregular patterns. The same authoritative pattern should
   be used by the string's lexical rule. This removes an implicit duplicate
   quoting implementation. No inference from token names. Before implementation,
   verify deterministic shielding and comment/string overlap admission with
   this model; unsupported overlaps fail construction, not silently skip text.
4. **Overlaps are explicit.** At an unshielded position, longest opening
   delimiter wins; equal spellings are a duplicate-declaration error with both
   source ranges. Inside a comment, only its own closer/nested opener applies;
   markers for other styles and quotes are text. Closers take precedence over
   nested openers at an identical position, retaining current behavior for
   prefix overlaps; equal opener/closer is invalid for block/nested forms.
   Cross-style nesting is outside this initial profile and must be rejected
   if requested. Never use declaration order to resolve duplicate openers.
5. **Failures and progress.** Reject empty delimiters, invalid scalar/byte
   spellings under the input profile and invalid shielding at build time.
   Unterminated blocks report their named form and original opening range.
   Malformed UTF-8/resource statuses remain distinct. Every scanning branch
   advances; nesting work is bounded by source bytes and the host scan budget.
   Recovery must respect quote/comment opacity, report skipped damage, and
   never turn an unterminated comment into a clean document.

Advanced form:

```rexx
quoted.text: pattern = '"' (string.body | '""')* '"'
notes: comments nested '/*' '*/' outside quoted.text newlines keep
```

`string.body` is an ordinary explicitly declared charset. Basic users need
none of these qualifiers. The optional pattern and newline policies let a
language author grow the same grammar when the default Rexx quoting/layout
is inappropriate. Languages with context-dependent string modes need a
declared lexical mode/stage contract; a regular shielding pattern is not a
claim to cover every language's interpolation or raw-string syntax.

Implementation contract: store a list of checked comment descriptors in
the common model, emit the same descriptors in source/RXAS, and implement one
source-preserving lexical coordinator shared by strict/recovery requests.
Run all forms on LL/LR/PEG/Earley/Pratt inputs, quote/escape adversaries, opener
prefixes, nesting, CR/CRLF/EOF, Unicode/NUL and lowered budgets. Verify unchanged
existing grammars, L0/L1/L2 agreement and last-good-artifact preservation. Do
not replace the old one-pair contract first and leave one emitter/seed behind.

The legacy alternative of line-comment DFA rules plus a pre-DFA nested
stage is useful but does not provide this composition: a block opener inside
a line comment is still seen by the earlier preparation stage. The user guide
states that ordering limit. Shared declarations coordinate these responsibilities
in one pass rather than merely accepting additional spellings.
