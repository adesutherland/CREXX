# Level L Syntax: AI and Implementer Context

Status: Level L language design for the `unicode` research branch. The lexer
maker is the first defined subset; implementation and release status are
separate.

Use this document when generating, reviewing, parsing, or implementing Level L
source. For the human-facing introduction, see
`docs/books/crexx_language_reference/level_l_syntax.md`.

This document defines the Level L authored surface, beginning with the lexer
maker. It is deliberately precise enough for an AI agent to write consistent
examples and for an implementation to parse them. Real lexer, Unicode-rule,
and later parser-maker use cases are expected to refine and extend the surface.

The words **must**, **should**, and **may** describe the Level L contract on
this branch. They do not claim that the current CREXX compiler implements the
syntax or that Level L has been released.

## Design In One Page

A Level L lexer specification is a Rexx-family source module:

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

The central declaration shape is the existing cREXX form:

```text
symbol: kind qualifiers = definition
```

Level L extends the kinds already seen in declarations such as
`name: procedure`, `thing: class`, and `work: task` with `token`,
`diagnostic`, `lexer`, `charset`, `pattern`, and `rule`. A later parser-maker
can extend the same family with `parser`, `nonterminal`, `precedence`, and
`production` without replacing the lexer syntax.

## Rexx Source Conventions

### Options and module identity

The source must begin with `options levell`. An ordinary `namespace`
declaration supplies the module identity. Level L does not add a second
`language { ... }` wrapper.

### Comments

Comments use the existing Rexx form:

```rexx
/* This is a comment. */
```

There are no slash-delimited regular expressions. In particular, `/.../` is
not a Level L pattern delimiter because `/` is already an operator and `/*`
opens a Rexx comment. Exact input text is written as a quoted string:

```rexx
slash: rule = "/" then emit slash_token
comment_open: rule = "/*" then emit comment_open_token
```

Comment markers inside a quoted literal are literal characters, not source
comments.

### Case and exact text

Declaration names follow the existing cREXX symbol case rules. Quoted strings
and `U+` code-point literals denote exact input. This syntax does not normalize
source, change CREXX identifier equality, or introduce a Unicode identifier
policy.

### Clauses and continuation

Declarations are clauses. A definition may continue while its expression is
syntactically incomplete, for example after `|` or within parentheses. The
implementation must retain the source range of every declaration and rule for
diagnostics and generated-source mapping.

## Symbols, Kinds, and Compound Names

Every declarative entity has a name and a kind:

```rexx
number: token
ascii.digit: charset = U+0030 to U+0039
integer: pattern = ascii.digit+
number_literal: rule = integer then emit number
```

The name to the left of `:` gives the entity a stable identity. Rule names are
not cosmetic: they are used in diagnostics, deterministic ordering,
differential tests, and source maps.

Dotted, stem-shaped names organize related declarations:

```rexx
ascii.digit
ascii.letter
unicode.xid_start
comment.body
number.hex
number.decimal
```

Within Level L declaration and pattern contexts these are **static compound
symbols**. `ascii.digit` always names that declaration; it is not a runtime
stem lookup and its tails are not replaced from variables. This distinction is
required for deterministic compilation.

A reference first resolves in the containing lexer and then at module scope.
An unresolved or ambiguous reference is a specification error.

## Module-Level Declarations

### Tokens

A token declaration introduces a symbolic token kind:

```rexx
eof: token = 0
number: token
identifier: token
punctuation.plus: token
```

Token names, rather than numeric IDs, are normative within lexer and parser
specifications. An explicit non-negative integer fixes an externally relevant
ID. Tokens without explicit IDs receive the smallest unused positive IDs in
source declaration order. Generation must record the resulting mapping and
must not depend on hash-table or filesystem order.

The initial convention reserves token ID zero for `eof`. A specification may
spell the symbol differently, but exactly one emitted end token should use ID
zero.

Every emitted token has the following logical fields:

- token kind;
- zero-based source byte start;
- source byte length; and
- no semantic payload in the first subset.

This is a logical contract, not a physical layout declaration. Portable
`<at..type>` fields, host-native `<packed..int>` fields, and hybrid layouts are
later binary-surface implementation choices.

Typed token payloads and conversions such as decimal-number decoding are
deferred until real examples establish their required semantics. Until then,
the token's source span is the value available to its consumer.

### Diagnostics

A diagnostic declaration gives an error a stable symbolic name and default
message:

```rexx
invalid_character: diagnostic = "invalid input character"
invalid_utf8: diagnostic = "invalid UTF-8 input"
```

An emitted diagnostic includes its symbolic identity, message, source byte
position, and rule/source location. The first whole-buffer scanner stops after
an `error` action. Recovery policy is deferred.

## Lexer Declarations and Scope

A lexer declaration names a scanner and its input interpretation:

```rexx
scan: lexer input utf8
```

The first input profiles are:

- `utf8`: validate UTF-8 and match Unicode scalar values while retaining byte
  offsets; and
- `binary`: match uninterpreted octets. The exact spelling of non-ASCII binary
  literals remains to be proved before the binary profile is accepted.

`charset`, `pattern`, and `rule` declarations following a lexer declaration
belong to that lexer. As with existing class/callable boundaries, a following
top-level `lexer`, `parser`, procedure, class, interface, or end-of-file closes
the lexer body. An explicit `end` is not introduced unless implementation
experience shows that the existing boundary model is inadequate.

## Character Sets

A `charset` describes a set of exactly one input unit: one Unicode scalar for
an `utf8` lexer or one octet for a `binary` lexer.

```rexx
ascii.digit: charset = U+0030 to U+0039
ascii.hex: charset = ascii.digit | U+0041 to U+0046
line_break: charset = U+000A | U+000D
comment.character: charset = scalar except (U+0000 | line_break)
```

The initial character-set operations are:

- `U+HHHH`: one Unicode scalar value;
- `a to b`: the inclusive range from scalar `a` through scalar `b`;
- `a | b`: union;
- `a except b`: set subtraction; and
- `(expression)`: grouping.

Within a UTF-8 lexer, `scalar` is the set of all Unicode scalar values and
therefore excludes surrogate code points. A quoted literal used in a
`charset` must contain exactly one scalar value.

Intersection, Unicode-property imports, and the binary-profile universe are
not yet part of the accepted subset.

## Patterns

A `pattern` describes a sequence of input units:

```rexx
integer: pattern = ascii.digit+
identifier: pattern = ascii.letter (ascii.letter | ascii.digit)*
line_end: pattern = U+000D? U+000A
short_hex: pattern = ascii.hex{2,8}
```

The initial pattern algebra is:

| Form | Meaning |
| --- | --- |
| `"text"` | Exact input sequence |
| `U+0041` | One Unicode scalar |
| `name` | A named `charset` or `pattern` |
| `a b` | Concatenation |
| `a \| b` | Alternation |
| `a?` | Zero or one occurrence |
| `a*` | Zero or more occurrences |
| `a+` | One or more occurrences |
| `a{n}` | Exactly `n` occurrences |
| `a{n,}` | At least `n` occurrences |
| `a{n,m}` | From `n` through `m` occurrences |
| `(a)` | Grouping |

Postfix repetition binds most tightly, concatenation binds next, and
alternation binds least tightly. Parentheses should be used whenever the
intended grouping is not visually obvious.

A normal rule must not match an empty sequence. This prevents a generated
scanner from accepting without advancing. End-of-input is represented by the
special `eof` rule subject instead.

## Rules and Actions

A rule has a stable name, a recognition expression, and one restricted action:

```text
name: rule = recognition then action
```

Examples:

```rexx
whitespace: rule = ascii.space+ then skip
number_literal: rule = integer then emit number
end_of_input: rule = eof then emit eof
bad_encoding: rule = malformed then error invalid_utf8
unmatched_input: rule = otherwise then error invalid_character
```

The initial actions are:

- `emit token-name`: append a token covering the accepted source span;
- `skip`: consume the accepted span without appending a token; and
- `error diagnostic-name`: report the diagnostic at the current input
  position and stop the first whole-buffer scanner.

Arbitrary embedded cREXX or host-language actions are not part of the first
subset. This keeps specification parsing, automaton construction, layout, and
source emission independent.

The special rule subjects are:

- `eof`: the cursor is at the end of valid input;
- `malformed`: the UTF-8 decoder cannot produce a valid scalar at the cursor;
  and
- `otherwise`: a valid input unit is present but no ordinary rule accepts it.

A complete UTF-8 lexer should define all three explicitly. Omitting one is an
error in the first subset rather than an implicit recovery policy.

## Matching Semantics

At each input position the generated lexer applies these rules:

1. If decoding is malformed, run the `malformed` rule.
2. If the cursor is at end of input, run the `eof` rule once and finish.
3. Consider every ordinary rule that can start at the current position.
4. Choose the rule that accepts the greatest number of input units.
5. If two or more rules accept the same greatest length, choose the rule
   declared first in source order.
6. Execute its restricted action and continue after the accepted span.
7. If no ordinary rule accepts, run `otherwise`.

This is maximal-munch matching with source-order tie breaking. It is not Rexx
`select`/`when` first-true behaviour, so the authored surface deliberately uses
`rule ... then ...` rather than `when`.

The emitted implementation may use an optimized `select` over DFA states. That
is an output strategy and does not change these authored semantics.

For a UTF-8 lexer, pattern lengths are measured in accepted scalar values but
token starts and lengths are recorded in source bytes. No normalization, case
conversion, or case folding occurs unless a later explicitly approved
operation requests it.

## Separation From Parsing and Unicode Algorithms

The lexer recognizes input and emits tokens. It does not perform the NFD
algorithm or hide a line grammar inside unrestricted actions.

For example, a lexer for ICU `gennorm2` input can emit tokens for hexadecimal
values, `..`, `:`, `=`, `>`, comments, and line endings. A later parser builds
typed range and mapping records. The Unicode data compiler consumes those
records, and the normalizer consumes the generated data. Keeping these layers
separate makes each independently testable against the retained re2c oracle.

The intended later parser shape uses the same declaration convention:

```rexx
expression_parser: parser
  expression: nonterminal
  term: nonterminal

  additive: precedence left plus minus

  sum: production = expression plus term
  difference: production = expression minus term
  simple: production = term
```

This parser fragment illustrates structural continuity only. Production
actions, semantic values, recovery, ambiguity reporting, and the exact parser
grammar remain separate design decisions.

## Compact Grammar for Implementers and Language Models

This grammar describes the intended first surface. It is explanatory EBNF, not
yet a checked generated grammar.

```text
module              ::= "options" "levell" namespace? declaration*
namespace           ::= "namespace" qualified-name

declaration         ::= token-declaration
                      | diagnostic-decl
                      | lexer-declaration
                      | existing-level-l-declaration

token-declaration   ::= qualified-name ":" "token" ("=" integer)?
diagnostic-decl     ::= qualified-name ":" "diagnostic" "=" string
lexer-declaration   ::= qualified-name ":" "lexer" "input" input-profile
                        lexer-member*
input-profile       ::= "utf8" | "binary"

lexer-member        ::= charset-declaration
                      | pattern-declaration
                      | rule-declaration

charset-declaration ::= qualified-name ":" "charset" "=" charset-expression
pattern-declaration ::= qualified-name ":" "pattern" "=" pattern-expression
rule-declaration    ::= qualified-name ":" "rule" "=" rule-subject
                        "then" action

rule-subject        ::= pattern-expression
                      | "eof"
                      | "malformed"
                      | "otherwise"

action              ::= "emit" qualified-name
                      | "skip"
                      | "error" qualified-name

charset-expression  ::= charset-union ("except" charset-union)*
charset-union       ::= charset-primary ("|" charset-primary)*
charset-primary     ::= codepoint ("to" codepoint)?
                      | one-scalar-string
                      | qualified-name
                      | "scalar"
                      | "(" charset-expression ")"

pattern-expression  ::= alternation
alternation         ::= concatenation ("|" concatenation)*
concatenation       ::= repetition+
repetition          ::= pattern-primary quantifier?
pattern-primary     ::= string
                      | codepoint
                      | qualified-name
                      | "(" pattern-expression ")"
quantifier          ::= "?" | "*" | "+"
                      | "{" integer "}"
                      | "{" integer "," "}"
                      | "{" integer "," integer "}"

qualified-name      ::= symbol ("." symbol)*
codepoint           ::= "U+" hexadecimal-scalar-value
```

The implementation grammar must distinguish a new top-level declaration from
a lexer member by declaration kind, using the same kind-directed boundary idea
as existing class and callable declarations. It must not rely on indentation.

## Required Specification Diagnostics

The first implementation should reject at least:

- missing or non-initial `options levell`;
- duplicate declarations in one scope;
- unresolved or wrong-kind references;
- duplicate explicit token IDs or an invalid EOF token contract;
- invalid Unicode scalar literals and reversed ranges;
- a multi-scalar string used as a `charset` element;
- invalid or contradictory repetition bounds;
- an ordinary rule that can accept an empty sequence;
- a missing `eof`, `malformed`, or `otherwise` rule in a UTF-8 lexer;
- multiple definitions of a special rule;
- `emit` naming something other than a token;
- `error` naming something other than a diagnostic; and
- syntax from a deferred feature presented as though it were supported.

Overlapping ordinary rules are not inherently an error. Maximal munch and
source order resolve them deterministically. Tooling should nevertheless be
able to report a rule that can never win.

## Authoring Checklist

For an AI agent producing or reviewing a first-subset lexer:

1. Start with `options levell` and, normally, a `namespace`.
2. Use only `/* ... */` source comments.
3. Declare tokens and diagnostics before the lexer that uses them.
4. Write declarations as `symbol: kind`, not `kind symbol`.
5. Use dotted names only as static, literal qualification.
6. Put one-scalar sets in `charset` declarations and sequences in `pattern`
   declarations.
7. Give every rule a unique, descriptive name.
8. Order equal-length rules from most specific/highest priority to least.
9. Define explicit `eof`, `malformed`, and `otherwise` rules for UTF-8 input.
10. Use only `emit`, `skip`, and `error` actions.
11. Do not add storage layout, host-language code, implicit normalization, or
    parser behaviour to a lexer specification.

## Deliberately Deferred Surface

The first syntax does not yet define:

- arbitrary embedded code actions;
- captures/tags or named subspans;
- typed token payloads and conversion actions;
- start conditions or lexer modes;
- trailing context/lookahead;
- streaming refill and incremental scanning;
- rule-priority numbers that override source order;
- Unicode property imports or property-expression syntax;
- binary literal and binary-universe notation beyond ASCII examples;
- parser semantic construction, recovery, or AST allocation; or
- physical token/table layout.

These are extension points, not permanent prohibitions. Each should enter the
language only when a retained use case demonstrates its semantics and cost.

## Implementation Boundary

The authored syntax should lower to a neutral logical model containing:

- a token symbol table and deterministic numeric mapping;
- a diagnostic table;
- typed charset and pattern expression trees;
- ordered rules with stable names and source locations;
- explicit EOF, malformed-input, and unmatched-input actions; and
- an input profile.

Automaton construction, transition representation, generated control flow,
binary table layout, and target source emission consume that model. A re2c
syntax-file emitter may be used as a bounded executable experiment, but re2c
syntax, `YY*` names, embedded C actions, and backend mechanics are not part of
this Level L contract.
