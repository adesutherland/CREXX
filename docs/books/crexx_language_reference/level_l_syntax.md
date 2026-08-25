# Level L Syntax

Level L is the cREXX language-engineering surface for describing lexers,
parsers, grammars, tokens, and related tools. The `unicode` research branch
defines the first part of that language: a lexer maker.

The syntax described here is the Level L design on that branch. It is expected
to grow and to be tuned as it is exercised against real language and Unicode
specifications. The declaration syntax is not yet implemented by the compiler.

## A first lexer

This example describes a lexer for a small expression language:

```rexx
/* TinyExpr lexer */
options levell
namespace tinyexpr

eof: token = 0
number: token
identifier: token
plus: token
minus: token

invalid_utf8: diagnostic = "invalid UTF-8 input"
invalid_character: diagnostic = "invalid input character"

scan: lexer input utf8
  ascii.digit: charset = U+0030 to U+0039
  ascii.letter: charset = U+0041 to U+005A |
                          U+0061 to U+007A |
                          "_"
  ascii.space: charset = U+0009 | U+000A | U+000D | U+0020

  integer: pattern = ascii.digit+
  name: pattern = ascii.letter (ascii.letter | ascii.digit)*

  whitespace: rule = ascii.space+ then skip
  number_literal: rule = integer then emit number
  identifier_name: rule = name then emit identifier
  plus_sign: rule = "+" then emit plus
  minus_sign: rule = "-" then emit minus

  end_of_input: rule = eof then emit eof
  bad_encoding: rule = malformed then error invalid_utf8
  unmatched_input: rule = otherwise then error invalid_character
```

Level L retains the character of cREXX rather than wrapping another
lexer-generator language inside it. The file uses `options levell`, ordinary
Rexx comments, a namespace, and cREXX's `symbol: kind` declaration form.

## Declarations

The common Level L declaration is:

```text
name: kind qualifiers = definition
```

The lexer subset adds these declaration kinds:

| Kind | Purpose |
| --- | --- |
| `token` | A symbol emitted for a parser or another consumer |
| `diagnostic` | A named error and its default message |
| `lexer` | A scanner and its input interpretation |
| `charset` | A set containing individual input characters |
| `pattern` | A sequence expression built from characters and other patterns |
| `rule` | A named recognition expression and its action |

Giving rules names makes generated diagnostics and source maps useful. It also
makes it possible to compare a generated Level L lexer with a reference lexer
rule by rule.

## Rexx comments and exact literals

Level L comments use the existing Rexx form:

```rexx
/* A comment may span lines. */
```

Regular expressions are not enclosed in slashes. `/` is already an operator,
and `/*` begins a comment. Exact input is written as a string:

```rexx
slash: rule = "/" then emit slash_token
comment_open: rule = "/*" then emit comment_open_token
```

The text inside the quotes is literal; the second example does not begin a
source comment.

## Names and stem-shaped families

Names may use a dotted, stem-shaped form to organize related definitions:

```rexx
ascii.digit
ascii.letter
unicode.xid_start
comment.body
number.hex
```

These are static Level L names. `ascii.digit` always refers to the declaration
with that name. It is not a runtime stem lookup, and the value of a variable
called `digit` cannot change what it means.

Names follow the existing cREXX symbol case rules. String and code-point
literals remain exact. Level L does not silently normalize its source or
change CREXX identifier equality.

## Tokens and diagnostics

Tokens are declared before the lexer that emits them:

```rexx
eof: token = 0
number: token
identifier: token
```

Token names are the important part of the specification. An explicit number
fixes a token ID. Unnumbered tokens are assigned deterministic positive IDs in
declaration order. Token ID zero is reserved for the end token.

Every token initially records its kind, starting byte position, and byte
length. The syntax deliberately says nothing about whether those fields use a
portable binary layout, host-native packed integers, or a mixture. That is a
generated-runtime decision.

A diagnostic has a stable name and a default message:

```rexx
invalid_character: diagnostic = "invalid input character"
```

When a rule reports that diagnostic, the generated scanner also supplies its
input position and source-rule location.

## Lexer input

A lexer declaration names the scanner and says how to interpret its input:

```rexx
scan: lexer input utf8
```

A `utf8` lexer validates UTF-8 and matches Unicode scalar values. Token
positions and lengths are nevertheless byte offsets, so a parser and source
diagnostic can refer directly to the original input.

The syntax also reserves `input binary` for byte-oriented scanners. Its
non-ASCII literal notation will be settled using real binary lexer examples.

The declarations after `scan: lexer` belong to that lexer. Like a cREXX class
or procedure body, the lexer continues until the next applicable top-level
declaration or the end of the file. Level L does not introduce braces or an
extra language wrapper.

## Character sets

A `charset` contains individual characters:

```rexx
ascii.digit: charset = U+0030 to U+0039
ascii.hex: charset = ascii.digit | U+0041 to U+0046
line_break: charset = U+000A | U+000D
comment.character: charset = scalar except (U+0000 | line_break)
```

The initial forms are:

- `U+0041` for one Unicode scalar value;
- `U+0041 to U+005A` for an inclusive range;
- `a | b` for a union;
- `a except b` for set subtraction; and
- parentheses for grouping.

`scalar` means every Unicode scalar value; it excludes the surrogate range. A
string used as part of a `charset` must contain exactly one scalar value.

## Patterns

A `pattern` describes a sequence:

```rexx
integer: pattern = ascii.digit+
identifier: pattern = ascii.letter (ascii.letter | ascii.digit)*
line_end: pattern = U+000D? U+000A
short_hex: pattern = ascii.hex{2,8}
```

| Form | Meaning |
| --- | --- |
| `"text"` | Exact input sequence |
| `name` | A named character set or pattern |
| `a b` | Concatenation |
| `a \| b` | Alternation |
| `a?` | Zero or one occurrence |
| `a*` | Zero or more occurrences |
| `a+` | One or more occurrences |
| `a{n}` | Exactly `n` occurrences |
| `a{n,}` | At least `n` occurrences |
| `a{n,m}` | Between `n` and `m` occurrences |
| `(a)` | Grouping |

Repetition binds most tightly, concatenation next, and alternation least
tightly. A pattern may continue onto another source line after `|` or while a
parenthesized expression remains incomplete.

## Rules and actions

A rule joins recognition to one restricted action:

```text
name: rule = recognition then action
```

The first actions are:

- `emit token-name` to append a token for the accepted source span;
- `skip` to consume input without appending a token; and
- `error diagnostic-name` to report an error and stop the first whole-buffer
  scanner.

There are three special rule subjects:

```rexx
end_of_input: rule = eof then emit eof
bad_encoding: rule = malformed then error invalid_utf8
unmatched_input: rule = otherwise then error invalid_character
```

`eof` handles the end of the source. `malformed` handles input that cannot be
decoded as UTF-8. `otherwise` handles a valid scalar that no ordinary rule
accepts. A complete UTF-8 lexer defines all three explicitly.

Ordinary rules may not accept an empty sequence. Without that restriction a
scanner could repeatedly match without advancing.

## Which rule wins?

At each source position Level L chooses a rule in two stages:

1. The rule accepting the longest input wins.
2. If several rules accept the same length, the one written first wins.

This is usually called maximal munch with source-order priority. It is not the
first-true behaviour of a Rexx `select`, which is why the source declarations
are called `rule` rather than `when`.

The generated implementation may use a loop and an optimized `select` over
automaton states. Generated control flow does not alter the language rules.

No normalization, case conversion, or case folding happens implicitly while
matching. Those operations must always be requested by an explicit service.

## Relationship to parsing

Tokens and diagnostics live at module scope so a later parser can share them.
The parser syntax will follow the same declaration style:

```rexx
expression_parser: parser
  expression: nonterminal
  term: nonterminal

  additive: precedence left plus minus

  sum: production = expression plus term
  difference: production = expression minus term
  simple: production = term
```

This is only the intended grammar shape. One parser behaviour is already fixed:
a successful parse creates a syntax tree automatically. A named production
creates a node, its recognized terminals and nonterminals become ordered
children, and token leaves retain their source spans. Grammar authors do not
need embedded actions merely to obtain that tree.

Typed semantic trees, visitors, tree rewrites, error recovery, and ambiguity
reporting remain later exercised designs. They can be layered over the
automatically constructed syntax tree, in the manner of parser systems that
first provide a complete parse tree and then offer more specialised AST tools.

A Unicode rule-file lexer likewise emits tokens; it does not hide the Unicode
algorithm in a lexer action. A parser constructs typed rule records, a data
compiler builds tables, and the Level G operation consumes those tables.

## Token and AST foundation

Level L will provide `Token` and AST class families implemented entirely in
Level B. They are part of the bootstrap foundation, not Level G services. A
Level L compiler may generate a Level G Unicode library, but compiling Level L
must not first require that generated library.

The first logical `Token` contract contains a token kind, zero-based source
byte start, and source byte length. An AST node contains a stable node or
production kind, its source byte span, an ordered child sequence, and a token
association for a terminal leaf. The exact class members, ownership model, and
ordinary-object or packed representation will be settled by implementation;
the logical result must remain the same.

The initial bootstrap frontend is deliberately simple Level B code: a
hand-written scanner and recursive-descent parser, ordinary objects and arrays,
linear lookup where convenient, and no performance requirement beyond being
usable on the retained specifications. It only needs to ingest the approved
Level L lexer surface and construct the agreed syntax tree.

That frontend has two first proving uses:

1. parse the Level L specification that describes Level L's own lexer; and
2. parse a Level L specification for the Unicode rule-file lexer used by the
   retained normalization work.

The generated lexer and, later, generated parser must produce the same tokens,
tree shape, source spans, and diagnostics as the bootstrap frontend. Once the
generated frontend can regenerate itself deterministically, the hand-written
implementation remains as a small retained bootstrap and differential oracle.

## Current boundaries

The first Level L lexer syntax intentionally leaves these features for later
evidence:

- arbitrary embedded cREXX actions;
- captures and named subspans;
- typed token payloads and conversion actions;
- lexer modes and start conditions;
- trailing context and lookahead;
- streaming and incremental input;
- numeric priority overrides;
- Unicode-property import syntax;
- typed parser actions, semantic-tree helpers, and recovery; and
- physical token, AST, or table layout.

These are extension points rather than permanent exclusions. They should be
added when a retained use case establishes what they must mean.
