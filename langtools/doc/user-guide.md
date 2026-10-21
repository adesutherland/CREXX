# Level L user guide

Level L helps you build programs that read a language: configuration files,
commands, scripts, expressions or source code. You describe the language in a
`.crexx` file beginning with `options levell`, or a short-form `.rxl` file.
The `rxlang` command generates a module that your cREXX program uses to
recognize input and return its structure. Your program then reads settings,
evaluates expressions, executes statements or produces another output.

You do not need parsing experience to begin. Read the overview, run the settings
example, then add the features your language needs. The reference sections give
exact declaration forms, runtime methods and error handling rules.

## Contents

- [Overview: from a language description to a useful program](#overview-from-a-language-description-to-a-useful-program)
- [Build and run a complete example](#build-and-run-a-complete-example)
- [Grow one settings grammar](#grow-one-settings-grammar)
- [Choose a backend and understand competing paths](#choose-a-backend-and-understand-competing-paths)
- [Build an end-to-end calculator](#build-an-end-to-end-calculator)
- [Specification reference](#specification-reference)
- [Lexical rules and Unicode](#lexical-rules-and-unicode)
- [Comments and layout](#comments-and-layout)
- [Parser rules and operators](#parser-rules-and-operators)
- [Token stages and composite parsing](#token-stages-and-composite-parsing)
- [Runtime API reference](#runtime-api-reference)
- [Trees, fields and source locations](#trees-fields-and-source-locations)
- [Sparse AST properties and defaults](#add-sparse-properties-and-defaults)
- [Flow mappings and definite assignment](#describe-execution-order-with-flow-mappings)
- [Error messages and recovery](#error-messages-and-recovery)
- [Command line and build integration](#command-line-and-build-integration)
- [Resource limits](#resource-limits)
- [Intermediate representation and semantic services](#intermediate-representation-and-semantic-services)
- [RexxScript: a larger example](#rexxscript-a-larger-example)
- [Troubleshooting](#troubleshooting)
- [Glossary](#glossary)

## Overview: from a language description to a useful program

Consider a settings file containing:

```text
retries = 3
```

A program reading it needs to answer three questions:

1. What pieces of text are present? Here they are a name, an equals sign and a
   number. A **lexer** finds these pieces, called **tokens**.
2. How do the pieces fit together? Here they form an assignment. A **parser**
   checks their arrangement against your **grammar**, the rules describing
   valid structures in the language.
3. What should the assignment do? Your application might store the number 3
   under the name `retries`. This is application code you write in cREXX.

The parser returns a **tree**: a collection of connected nodes showing which
pieces belong together. For example, an assignment node contains a name and a
value. Trees are particularly useful when constructs nest, as with expressions
inside assignments or statements inside loops.

### Build the language module, then use it on documents

There are two kinds of input file. A `.crexx` specification with `options levell`,
or a short-form `.rxl` file, describes your language. A settings file or script is a document written in that language.

```mermaid
flowchart LR
    G["Your language specification"] --> B["crexx calls rxlang to check rules and generate a module"]
    B --> M["Language module"]
    M --> A["Your cREXX application"]
    S["Settings file, script or other input"] --> A
    A --> R["Result or error messages"]
```

At build time, `crexx` calls `rxlang` to check the specification and generate the
language module. It assembles the module, compiles your application and links
them with the Level L runtime. On later builds it reuses unchanged work.

At run time, your application creates a language definition from the module
and a fresh **request** for each input document. A request holds the source,
tokens and parse result. The language rules are reused across documents.

### How a specification is arranged

A small specification has four main parts:

| Part | What you write | What it supplies |
|---|---|---|
| Header | `options levell` and a namespace | The generated module's name. |
| Token declarations | `identifier: token`, `number: token` | Names for the kinds of text the parser receives. |
| Lexer | Patterns for names, numbers, punctuation and whitespace | Rules for turning text into tokens. |
| Parser | Rules for assignments and the whole document | Rules for combining tokens into a tree. |

For example, the lexical rule `digit+` means one or more digits. It produces a
`number` token. The parser rule `identifier equals number` means those three
token kinds in that order.

Whitespace has its own lexical rule. If it uses `then skip`, the parser does
not receive whitespace tokens. The original text remains available for source
locations and messages.

Most languages can start with these four parts. Comment declarations, custom
messages, named fields and extra processing stages are optional additions.
The specification describes the language; its declarations are not instructions
that execute from top to bottom for each document.

### Follow the input through the parser

For `retries = 3`, the lexer produces:

| Token kind | Text | Byte range |
|---|---|---|
| `identifier` | `retries` | `[0,7)` |
| `equals` | `=` | `[8,9)` |
| `number` | `3` | `[10,11)` |

A **byte range** begins at its first byte and ends just after its last byte.
Thus `[0,7)` covers seven bytes. Token text is still text: the application will
convert `3` to a numeric value if it needs one.

The tutorial's parser contains these rules:

```rexx
assignment: rule = name.value equals value
name.value: rule = identifier
value: rule = number | string
```

Here `|` means an alternative: a value may be a number or a string. The parser's
**start rule** describes the complete document. Parsing succeeds only when all
tokens fit that rule.

The resulting named tree is:

```text
settings
└── assignment
    ├── name.value
    │   └── identifier       "retries"
    ├── equals              "="
    └── value
        └── number          "3"
```

Rule names become node names. The `name.value` and `value` nodes remain because
they are named in the grammar. Repetition puts successive assignments directly
under `settings`; its generated helper nodes are hidden. The `equals` token
remains as a child of the assignment.

### Use the tree

A **tree walk** visits nodes and their children. The settings reader finds each
`assignment` and prints its name and value text. A configuration loader could
instead convert values and check for duplicate names.

A script interpreter uses the same idea to execute statements. For an IF, it
evaluates the condition and executes the selected branch. Both branches must
be syntactically valid, but only the selected branch has effects.

Choose the tree representation that fits the application:

| Representation | When to use it |
|---|---|
| Raw tree, `.llcore..tree` | Low-level tree operations. It includes helper nodes used to represent the grammar. |
| Named view, `.lltree..view` | Inspect the grammar structure. It hides internal helper nodes and exposes your named rules and tokens with their ordered children. |
| Application AST, `request.ast()` | Work with kinds and named fields chosen by `then tree` mappings, such as `Assignment(name, value)`. |

An **abstract syntax tree (AST)** is organized around the application's work.
Add `then tree` mappings when grammar wrappers and punctuation would otherwise
make your reader cumbersome. The basic settings reader uses the named view;
the calculator and settings lesson 9 use AST fields, common walks and rewrites.
The [tree reference](#application-trees-with-then-tree) explains both choices.

An AST field connects a parent to a child or stores text. For example, an `If`
node can have a `condition` child and two block children called `yes` and `no`.
Your `then tree` mapping defines those field names. An optional **flow mapping**
uses the same names to describe which children execute in which order. The
library can then build a graph of possible execution paths and analyse it.
The [IF walkthrough](#from-grammar-captures-to-ast-fields) follows these links
from source text to an assignment diagnostic.


### Where extra stages fit

The normal request performs these operations:

```mermaid
flowchart TD
    S["Original source"] --> P["Validate encoding and handle declared comments"]
    P --> L["Lexer: recognize tokens"]
    L --> C["Optional token stages: classify or combine tokens"]
    C --> R["Parser: recognize structure"]
    R --> T["Tree and named view"]
    T --> A["Application: validate and use the result"]
```

A **stage** is a processing step with an input and an output. For example, a
keyword stage can classify `SAY` as a command at the beginning of a statement,
while allowing it as a variable in `say = 7`.

References between grammar rules are different from stages. Rules for
`expression`, `product` and `atom` all contribute to one parse; they do not
mean that the source is parsed three times. The [RexxScript example](#rexxscript-a-larger-example)
shows how real token stages surround a single parser.

## Build and run a complete example

This example reads:

```text
retries = 3
greeting = "café"
note = "say ""hello"""
```

Names begin with an ASCII letter or underscore and may contain digits. Values
are unsigned integers or double-quoted strings. A doubled double quote denotes
a quote within a string. Newlines and semicolons separate assignments; blank
lines and a final separator are allowed.

Use these complete files:

- [01-basic.crexx](../examples/settings/01-basic.crexx): the specification.
- [settings-reader.crexx](../examples/settings-reader.crexx): the cREXX reader.
- [settings.txt](../examples/settings.txt): sample input.

### Read the specification

The complete basic specification is:

```rexx
options levell
namespace levell_settings
identifier: token
number: token
string: token
equals: token
separator: token
scan: lexer
  letter: charset = 'a' to 'z' | 'A' to 'Z' | '_'
  digit: charset = '0' to '9'
  space: charset = ' ' | U+0009
  body: charset = scalar except '"' | U+000A | U+000D
  name.lex: rule = letter (letter | digit)* then emit identifier
  number.lex: rule = digit+ then emit number
  string.lex: rule = '"' (body | '""')* '"' then emit string
  equals.lex: rule = '=' then emit equals
  separator.lex: rule = U+000D U+000A | U+000D | U+000A | ';' then emit separator
  whitespace: rule = space+ then skip
parse: parser start settings
  settings: rule = separator* (assignment (separator assignment?)*)?
  assignment: rule = name.value equals value
  name.value: rule = identifier
  value: rule = number | string
```

Read the lexer rules first. A **charset** is a set of characters: `digit` is the
set `0` through `9`. `U+0009` names the tab character by its Unicode number.
`scalar` means any Unicode scalar value, the character units used by the lexer.
The string rule accepts doubled quotes and characters other than line endings. Outside strings, CR, LF, CRLF and `;` produce a
`separator` token.

Read `settings` from left to right:

- `separator*` accepts any leading blank lines or semicolons.
- `assignment` reads the first setting.
- `(separator assignment?)*` reads further separators, each optionally
  followed by a setting. This allows blank lines and a final separator.
- The outer `(...)?` allows a file with no assignments.

`*` means zero or more, `+` means one or more, and `?` means optional. Repeated
matches appear in source order under their containing rule. Level L generates
the internal rules needed to implement repetition for the selected backend.


The short headers select UTF-8 character matching and the LR(1) parser. You can
use those defaults without understanding the algorithms. [Backend selection](#choose-a-backend-and-understand-competing-paths)
explains when to choose another one.

### Prepare the tools

An installed cREXX distribution supplies `crexx`, `rxlang` and the runtime
libraries. The commands below run from the repository root and use that
installation:

```sh
task_bin="$(dirname "$(command -v crexx)")"
```

To use a local build instead, build `stage-product` and set
`task_bin` to its `bin` directory:

```sh
cmake -S . -B cmake-build-levell-product -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-levell-product --target stage-product --parallel 4
task_bin="$PWD/cmake-build-levell-product/bin"
```

Parallel Make builds use the same bootstrap ownership as other generators:
the library and Unicode generator wait for verified L2 completion. No serial
build workaround is required for that preparation path. See the
[bootstrap guide](bootstrap.md#build-lifecycle-and-failure) for dependencies
and incremental rebuild behaviour.

### Build and run the reader

The reader imports `levell_settings`, the namespace declared in the grammar.
Include both source files in one program build:

```sh
"$task_bin/crexx" --program build/settings-reader \
  langtools/examples/settings-reader.crexx \
  langtools/examples/settings/01-basic.crexx
"$task_bin/rxvme" build/settings-reader.rxbin \
  -a langtools/examples/settings.txt
```

The grammar can appear before or after the reader. `crexx` generates it first
and supplies its module when compiling the reader. The linked program includes
the Level L runtime. **You do not need a separate library build.**

For a quick build and run, leave out `--program`:

```sh
"$task_bin/crexx" langtools/examples/settings-reader.crexx \
  langtools/examples/settings/01-basic.crexx \
  --args langtools/examples/settings.txt
```

This writes the program beside the reader. Use `--program` to choose an output
path and build without running. Add `--native` to create a native executable.

Expected output:

```text
retries -> 3
greeting -> "café"
note -> "say ""hello"""
```

The reader prints source text, so the string's quotes remain in the output.
To use a value as an integer or decoded string, add that conversion to the
reader after parsing succeeds.

### Understand the reader

The central calls are:

```rexx
definition = levell_settings..language()
request = .lllanguage..request(definition,source)
status = request.parse()
if status <> 0 then do
  say llsource..diagnostic(source,request.offset(),'SETTINGS_PARSE',request.error())
  return status
end
tree = .lltree..view(request.tree(),request.root())
if tree.error() <> '' then do
  say tree.error()
  return 3
end
```

`source` is a `.binary` containing the input file's bytes. After a successful
parse, the named view gives access to rules and tokens. The complete reader
walks the tree, finds `assignment` nodes, and reads children 1 and 3.

Keeping parsing before application actions means an error near the end of a
file is found before any setting takes effect.

## Grow one settings grammar

The files in [examples/settings](../examples/settings) extend the same language.
Each numbered file is complete. Generate the selected file in place of
`01-basic.crexx` in the program build command above, then run the reader.

| Lesson | Adds | Reader |
|---|---|---|
| [01-basic](../examples/settings/01-basic.crexx) | Assignments, strings, numbers and separators | `settings-reader.crexx` |
| [02-messages](../examples/settings/02-messages.levell) | A custom value error | `settings-reader.crexx` |
| [03-unicode](../examples/settings/03-unicode.levell) | Explicit Unicode name ranges | `settings-reader.crexx` |
| [04-recovery](../examples/settings/04-recovery.levell) | Continue after a damaged setting | `settings-recover.crexx` |
| [05-composite](../examples/settings/05-composite.levell) | Arithmetic inside brackets | `settings-reader.crexx` |
| [06-fields](../examples/settings/06-fields.levell) | Named `name` and `contents` fields | `settings-fields.crexx` |
| [07-properties](../examples/settings/07-properties.levell) | Unicode identifier properties | `settings-fields.crexx` |
| [08-comments](../examples/settings/08-comments.levell) | Line, block and nested comments | `settings-fields.crexx` |
| [09-ast](../examples/settings/09-ast.rxl) | Application nodes and lists with `then tree` | `settings-ast.crexx` |

The [recovery reader](../examples/settings-recover.crexx) and
[field reader](../examples/settings-fields.crexx) use the same generated
namespace. To build either, substitute its source filename and output basename
for `settings-reader` in the build and run commands.

### Add your own wording

Lesson 2 adds one declaration:

```rexx
value.error: diagnostic rule value =
  'Expected a number or quoted string; found {found}.'
```

For `workers = many`, the reader reports line 1, column 11, and:

```text
value.error: Expected a number or quoted string; found 'many'.
```

For `workers =`, `{found}` becomes `end of input`. The message applies when the
parser is expecting a value. An extra word after a completed value gets a
separator error instead. [Message templates](#message-templates) lists the
available fields.

### Add Unicode names

Lesson 3 extends the sets of characters allowed in names. Its
[sample input](../examples/settings/unicode.settings) includes `café`, `温度`
and `Δ`. String values already support Unicode; only the name rules change.

Lesson 7 replaces the illustrative ranges with standard identifier properties:

```rexx
letter: charset = unicode.XID_Start | '_'
continuation: charset = unicode.XID_Continue | '_'
name.lex: rule = letter continuation* then emit identifier
```

`XID_Start` is a Unicode set intended for the first character of an identifier;
`XID_Continue` supplies characters allowed after it, including digits and many
combining marks. The example also permits underscore. Level L uses Unicode
17.0.0 property data.

Matching preserves the original spelling. A precomposed `é` and an `e` followed
by a combining accent can both be accepted, but are still different sequences.
If your application treats such names as equal, make that comparison rule part
of its name handling. [Unicode reference](#unicode-properties) explains the
available sets and source positions.

### Continue after an error

Lesson 4 adds:

```rexx
read.errors: recovery rule settings sync separator
```

**Recovery**, also called **resynchronization**, means skipping damaged input
so that later errors or useful records can still be found. This declaration
chooses a separator token as the point at which reading can resume.

Use `.llrecover..request` to apply it. Ordinary `request.parse()` still stops
on an error. The recovery reader reports the bad settings in
[damaged.settings](../examples/settings/damaged.settings), then displays the
valid later assignments. It returns status 1 to say that the document contains
errors. Use the surviving tree for inspection; require status 0 before applying
configuration or executing code.

Separators inside strings or valid comments do not end a record. The
[recovery reference](#recovery-requests) covers statuses, skipped records and
which languages can use this policy.

### Include arithmetic expressions

Lesson 5 adds bracketed values such as `workers = [2 + 3 * 4]`:

```rexx
arithmetic: operators atom number group lparen rparen
sum: infix arithmetic plus precedence 10 left
product: infix arithmetic times precedence 20 left
negative: prefix arithmetic minus
math.region: region token expression using arithmetic
  open lbracket close rbracket
```

The complete file declares the tokens and their spellings. These declarations
put a Pratt expression parser inside the settings parser. **Composite parsing**
means using connected parsing components for different parts of the language.

The outer parser treats the bracketed expression as one value. The expression
parser handles the arithmetic inside it. The combined tree contains:

```text
expression                 [2 + 3 * 4]
└── sum                    2 + 3 * 4
    ├── number             2
    └── product            3 * 4
        ├── number         3
        └── number         4
```

The higher precedence number makes multiplication bind more tightly than
addition. An application can evaluate this tree, as the calculator below does.
The settings reader simply prints its original text.

### Name the useful fields

Lesson 6 changes the assignment rule to:

```rexx
assignment: rule = name.value as name equals value as contents
```

A **capture**, written `as name`, gives a part of the match a field name. The
field reader uses those names instead of child positions:

```rexx
name = tree.field(assignment,'name')
contents = tree.field(assignment,'contents')
say tree.text(name) || ' -> ' || tree.text(contents)
```

Fields refer to nodes with the same source text and range operations as other
nodes. They are especially useful when an application's code should not depend
on the position of punctuation in a rule.

### Add comments

Lesson 8 combines the previous features with:

```rexx
hash.notes: comments line '#'
slash.notes: comments line '//'
rexx.notes: comments nested '/*' '*/'
plain.notes: comments block '(*' '*)' newlines keep
```

Line comments end at a line ending. Block comments end at the first closer.
Nested comments allow another comment of the same form inside them.
`newlines keep` lets line endings inside the plain block continue to separate
settings. Markers inside quoted strings remain string content.

The [sample input](../examples/settings/comments.settings) produces this output
with the field reader:

```text
café -> [2 + 3 /* arithmetic notes */ * 4]
greeting -> "# /* quoted text */"
first -> 1
second -> 2
```

The comment inside the expression is still present in the field's source text.
Comments are ignored for recognition while their original spelling is retained.
See [Comments and layout](#comments-and-layout) for delimiters and string rules.

## Choose a backend and understand competing paths

A **backend** is the algorithm Level L uses to implement your rules. Start with
the default LR(1) parser for a structured language. Use Pratt for a language
made entirely of operator expressions, or for expressions inside a declared
region. Choose another backend when its behavior suits the rules you want.

The lexer uses a **deterministic finite automaton (DFA)**: a table of states
that recognizes character patterns. All parser choices below can use this
lexer. Comment processing is shared too.

| Parser | Choose it for | How it handles choices |
|---|---|---|
| **LR(1)**, the default | Statements, nested blocks, lists and expression grammars | Combines the structure recognized so far with the next token. Refuses a grammar when its table would require a conflicting decision. |
| **LL(1)**, `backend ll1` | Commands or formats where each choice is clear from the next token | Predicts which rule to use before reading its contents. Refuses overlapping predictions and left recursion. |
| **Packrat PEG**, `semantics ordered backend packrat` | Rules where trying alternatives in a specified order is part of the language | Keeps the first locally successful alternative. Remembers earlier results to avoid repeating the same work. Refuses left recursion. |
| **Pratt**, `backend pratt` | Calculators and other operator expressions | Uses declared operators, precedence and associativity to build the expression tree. |
| **Earley**, `backend earley` | Exploring general grammars, or retaining several possible interpretations | Follows competing grammar paths. Returns a tree when the interpretation is unique, or a forest when it is not. |

The `(1)` in LL(1) and LR(1) means one token of **lookahead**: the next token
used to make a parsing decision. It does not limit how deeply structures can
nest. **Left recursion** means a rule can call itself before consuming input,
for example `list = list comma identifier | identifier`. LR and Earley support
this useful way of describing lists; the LL and packrat implementations refuse it.

### A first project for each component

Several backends can handle a simple language. These projects give each one a
clear purpose and something useful to try.

**DFA: inspect the words and punctuation in a file.** Build a token viewer for
an editor, extract items from a log, or check the spelling of numeric literals.
Start with [TinyExpr's lexer](../examples/tinyexpr/tinyexpr.levell) and the
[lexer-only API](#lexer-only-applications). Print each token's name, text and
range. Try `12 + 34`, then an invalid character. This is enough for a tool that
needs individual pieces of text; add a parser when their arrangement matters.

**LL(1): a small command interface.** Start with
[predictive.levell](../examples/backends/predictive.levell), which accepts
`read port` and `write port 3`. This is a model for device commands, test-control
files or a simple administrative console. The command word tells the parser
which form follows. Try omitting the value in `write port`, then extend the
grammar with a command of your own. LL(1) is convenient when these early
decisions keep the grammar easy to read.

**LR(1): a configuration or scripting language.** Use the
[settings tutorial](#build-and-run-a-complete-example) for configuration, and
[RexxScript](#rexxscript-a-larger-example) for statements and nested IFs. LR is
a good default as a language grows to include blocks, recursive lists and
expressions. The [left-recursive list](../examples/backends/left-recursive-list.levell)
is a smaller experiment: parse `a,b,c` and inspect how the rule builds the list.
LR lets you express that recurring structure directly.

**Pratt: a formula field or calculator.** Start with the
[calculator](#build-an-end-to-end-calculator). Change `2+3*4` to `(2+3)*4` and
compare the results and tree shapes. This is useful for a spreadsheet-style
formula field, an expression in a rule file, or arithmetic values in a
configuration language. When expressions are only part of a larger document,
use [settings lesson 5](../examples/settings/05-composite.levell): LR handles
assignments while Pratt handles the bracketed formula.

**PEG: values with a deliberate interpretation priority.** A configuration
language might treat `true` as a Boolean, `blue` as a bare word and `"true"` as
a quoted string. [ordered-values.levell](../examples/backends/ordered-values.levell)
expresses that policy with:

```rexx
value: rule = boolean | bareword | quoted
boolean: rule = trueword | falseword
bareword: rule = identifier | trueword | falseword
quoted: rule = string
```

The `bareword` rule deliberately includes the Boolean token kinds. Ordered
choice gives `boolean` priority at this position, so `true` has a `boolean`
node. Change the first rule to `bareword | boolean | quoted` and the same input
has a `bareword` node. This makes PEG useful when “the more specific form wins
over a general form” is an explicit language rule. The application interprets
the selected node. Quoting lets a user request text even when its spelling
would otherwise be special.

**Earley: investigate a grammar before committing to one interpretation.**
Start with [ambiguous-arithmetic.levell](../examples/backends/ambiguous-arithmetic.levell)
and try `2+3*4`. Earley accepts the grammar and reports the competing parses,
making the missing precedence rule visible. This is useful while developing a
DSL, adapting an existing language description, or diagnosing a deterministic
parser's conflict. You can also use Earley in an application designed to retain
several interpretations. Try the
[palindrome grammar](../examples/backends/palindrome.levell) with `abba` to see
a unique result from rules that the LL(1) and LR(1) backends refuse.

### Run the backend inspection examples

The examples in `examples/backends` share the namespace `levell_lesson` and the
same tree inspector. After preparing the tools, this runs the PEG value example:

```sh
"$task_bin/crexx" --program build/inspect \
  langtools/examples/backends/inspect.crexx \
  langtools/examples/backends/ordered-values.levell
printf '%s' 'true' > build/input.txt
"$task_bin/rxvme" build/inspect.rxbin -a build/input.txt
```

The output includes `STATUS|0` and `NODE|boolean|0|4`. Each `NODE` line gives a
node name and its original byte range. Replace the specification path and input
text to try another backend example, then regenerate and relink. For Earley's
ambiguous arithmetic example, expect status 5 and
`DERIVATIONS_CAPPED_AT_2|2` instead of a single tree.

### Possible alternatives versus ordered alternatives

The default `semantics contextfree` selects a **context-free grammar (CFG)**.
Its alternatives describe possible structures. Their order does not choose a
winner when more than one complete interpretation is possible.

`semantics ordered` selects a **parsing expression grammar (PEG)**. Its
alternatives are instructions to try choices in order. Once an alternative
succeeds at that position, a later failure in its caller does not reopen that
choice.

Consider these rules, where tokens `a`, `b` and `stop` spell `a`, `b` and `c`:

```rexx
root: rule = fragment stop
fragment: rule = a | a b
```

Under CFG semantics, both `ac` and `abc` fit. Under ordered semantics, `ac` fits
but `abc` fails: `fragment` has already succeeded on `a`, so `root` next expects
`c` and encounters `b`. Writing `a b | a` makes this particular PEG accept both
inputs. When changing choice order, test both accepted and rejected examples;
priority is part of the language's meaning.

This choice is independent of the lexer's longest-match rule. The lexer has
already produced the tokens before these parser rules run.

### Examples that call for different backends

The [backend examples](../examples/backends) include complete grammars and an
[inspection program](../examples/backends/inspect.crexx) that prints a tree or
reports a forest.

| Example | What to learn |
|---|---|
| [Predictable commands](../examples/backends/predictive.levell) | `read port` and `write port 3` begin with distinct command tokens. LL(1), LR(1) and Earley accept the rules. They are a straightforward use of LL(1). |
| [Left-recursive list](../examples/backends/left-recursive-list.levell) | `list = list comma identifier \| identifier` fits LR and Earley. To use LL or packrat, express the list as `identifier (comma identifier)*` and adjust code that depends on its tree shape. |
| [Ordered prefix](../examples/backends/ordered-prefix.levell) | Demonstrates why a successful PEG choice can cause a later failure, using the `ac`/`abc` example above. |
| [Even palindrome](../examples/backends/palindrome.levell) | `palindrome = a palindrome a \| b palindrome b \| ''` accepts `abba` with Earley. These rules are refused by LL(1) and LR(1): the center is not marked by a token that tells them when to switch direction. |
| [Ambiguous arithmetic](../examples/backends/ambiguous-arithmetic.levell) | `expression = expression plus expression \| expression times expression \| number` leaves precedence unspecified. Earley retains competing trees for `2+3*4`; LR reports a build conflict. |

A **conflict** means the selected deterministic algorithm cannot make one of
its required decisions from the grammar. **Ambiguity** means the same complete
input has more than one derivation, or way of applying the rules. A conflict
can suggest ambiguity, but an unambiguous grammar can also fall outside a
particular algorithm's capabilities.

For an LR conflict, inspect the named rules and lookahead token. If you are
unsure whether the rules permit several interpretations, try them with Earley
and inspect a small failing example. For arithmetic, specifying precedence
usually expresses the intended structure directly.

### When there is more than one successful tree

For `2+3*4`, an ambiguous grammar might allow both `2+(3*4)` and `(2+3)*4`.
A **parse forest** stores those alternatives while sharing their common parts.
Earley returns request status **5** when it accepts the input without finding
one unique finite tree. Use `request.forest()` and `request.forestroot()`.
Do not send this result to code expecting a single tree.

Your language needs a rule for choosing among interpretations. Usually it is
clearest to encode that rule in the grammar, such as operator precedence. An
application that intentionally allows ambiguity can instead inspect the forest
and implement its own selection. Selecting the first stored tree would make
storage order determine the language's meaning.

`forest.derivations(root,cap)` counts derivations up to a supplied cap. With a
cap of 2, a result of 2 means at least two. It returns -1 for a reachable
cycle in the derivations and -2 if traversal exceeds its allowance. See the
[forest API](#parse-forests).

### When several paths fail

A parser may try several alternatives before the complete parse fails. Some
failures were merely unsuccessful trials; they do not all describe errors in
the user's input. Level L reports the backend's failure location and expected
tokens. It uses a custom rule message when it can identify one applicable
context, and otherwise keeps the general explanation.

For an unexpected result, first inspect the tokens, then the grammar choice
and tree. A keyword classified incorrectly by the lexer or token stage cannot
be repaired by selecting another parser. A resource-limit result needs a budget
or workload adjustment; it is not a syntax diagnosis.

## Build an end-to-end calculator

The [calculator grammar](../examples/calculator/calculator.rxl),
[evaluator](../examples/calculator/calculator.crexx) and
[sample input](../examples/calculator/example.calc) form a complete interpreter.
It accepts decimal numbers, `+ - * /`, prefix minus, parentheses and nested
`(* ... *)` comments.

The grammar chooses Pratt, declares precedence and attaches application nodes
to the token and operator declarations:

```rexx
number: token then tree Number(spelling = text)
/* Declare the other tokens and lexer rules here; see the complete file. */
parse: parser start arithmetic backend pratt
arithmetic: operators atom number group lparen rparen
sum: infix arithmetic plus precedence 10 left
  then tree Add(left = left, right = right)
difference: infix arithmetic minus precedence 10 left
  then tree Subtract(left = left, right = right)
product: infix arithmetic times precedence 20 left
  then tree Multiply(left = left, right = right)
quotient: infix arithmetic divide precedence 20 left
  then tree Divide(left = left, right = right)
negative: prefix arithmetic minus
  then tree Negate(value = inner)
```

**Precedence** controls which operator binds more tightly: multiplication and
division have 20, while addition and subtraction have 10. **Associativity**
controls operators at the same level: `left` makes `8-3-1` mean `(8-3)-1`.
Prefix minus binds more tightly than the infix operators.

After `request.parse()` succeeds, `request.ast()` constructs this AST for
`2+3*4`:

```text
Add
├── left: Number(spelling="2")
└── right: Multiply
    ├── left: Number(spelling="3")
    └── right: Number(spelling="4")
```

`then tree` selects the useful nodes and names their fields. Parentheses,
operator tokens and grammar wrappers guide parsing; the AST carries operands
and values. The original parse tree remains available for inspecting the grammar.

The evaluator uses `tree.walk('children first')`. It converts `Number.spelling`
to a `.decimal` and stores the value under `node.id()`. `Multiply` multiplies
its `left` and `right` child values; `Add` adds them. The walk visits `2`, `3`,
`4`, `Multiply`, then `Add`, producing 14. Numeric rules belong to this
application, so another consumer can use the same AST with different arithmetic.

After preparing the tools, build the calculator and its grammar together:

```sh
"$task_bin/crexx" --program build/calculator \
  langtools/examples/calculator/calculator.crexx \
  langtools/examples/calculator/calculator.rxl
"$task_bin/rxvme" build/calculator.rxbin \
  -a langtools/examples/calculator/example.calc
```

The two source files may share a basename: the driver keeps the generated
grammar and application artifacts in separate private build directories.

The sample `2 + 3 * (4 - 1)` prints `RESULT|11`. The decimal evaluator also gives
`0.3` for `0.1+0.2`. Numeric types and conversions are choices made by the
application using the tree; Level L supplies the structure and source text.

The calculator parses the whole input before evaluating it. `2+*3` reports
`CALC_PARSE` at the unexpected `*`. `7/(3-3)` reports `CALC_DIVIDE_BY_ZERO`
at the denominator. `7/0 +` reports the syntax error before evaluation begins.
These errors include the original source location and produce no `RESULT`.

## Specification reference

### File structure and names

Use `.crexx` for an ordinary cREXX source file containing a Level L description.
Begin with:

```rexx
options levell
namespace my_language
```

The short suffix `.rxl` selects Level L automatically. In that form the
`options levell` line is optional; `namespace` is still required. An explicit
other level is an error. Existing `.levell` files with their Level L header
remain supported. Suffix and header matching are case-insensitive.

Declarations have the form `name: kind ...`. Their names are global within the
specification. Names and keywords are ASCII and case-insensitive. Names begin
with a letter or underscore and may contain digits and internal dots. A
trailing dot or two consecutive dots is invalid.

Use one lexer and optionally one parser. Place lexical declarations after the
lexer header and parser rules after the parser header. Indentation helps
readability but does not create scope. Forward rule references are allowed.
Token stages and other named declarations can follow the parser rules.

Whitespace separates specification tokens; the next `name:` begins another
declaration. Do not add Rexx statement semicolons. The specification itself
accepts nested `/* ... */` comments. Declarations of target-language comment
styles affect the documents your application reads.

Single or double quotes delimit literals. Double the enclosing quote to include
it: `'can''t'` or `"say ""hello"""`. Backslashes are literal characters. To match
a newline, use `U+000A` in a lexical expression or charset.

### Declaration summary

Uppercase names below stand for names or numbers you supply; they are not
keywords to type literally.

| Form | Purpose |
|---|---|
| `NAME: token` or `NAME: token = ID` | Declare a token kind, with an automatic or explicit number. |
| `NAME: lexer [input PROFILE] [backend dfa]` | Begin lexical definitions. Default input is `utf8`. |
| `NAME: charset = SET` | Name a set of characters or bytes. |
| `NAME: pattern = EXPRESSION` | Name a reusable lexical expression. |
| `NAME: rule = EXPRESSION then emit TOKEN` | Recognize and emit a token. |
| `NAME: rule = EXPRESSION then skip` | Recognize text without emitting a token. |
| `NAME: comments line 'OPEN'` | Declare a line comment. |
| `NAME: comments block 'OPEN' 'CLOSE'` | Declare a nonnested block comment. |
| `NAME: comments nested 'OPEN' 'CLOSE'` | Declare a nested block comment. |
| `NAME: parser start RULE [semantics MODE] [backend ENGINE]` | Begin a parser. Default semantics are `contextfree`, with backend `lr1`. |
| `NAME: rule = EXPRESSION` | Define a parser rule after the parser header. |
| `NAME: diagnostic rule RULE = 'MESSAGE'` | Customize the message for a parser rule. |
| `NAME: diagnostic = 'MESSAGE'` | Define a message for a lexical error action. |
| `NAME: recovery rule START sync TOKEN` | Declare record recovery for the start rule. |
| `NAME: operators atom TOKEN group OPEN CLOSE` | Define an operator component. |
| `NAME: infix COMPONENT TOKEN precedence NUMBER left` | Declare a left-associative infix operator; `right` is also available. |
| `NAME: prefix COMPONENT TOKEN` | Declare a prefix operator. |
| `NAME: region token VIRTUAL using COMPONENT open OPEN close CLOSE` | Embed a Pratt expression in an LR parser. |
| `NAME: tokenstage start STATE` | Define a token-processing stage. |
| `NAME: transform stage ...` | Define one action in that stage; see the [stage reference](#token-stages). |
| `... then tree [KIND](FIELDS)` | Attach application-node construction to a token, parser rule or operator declaration. |
| `NAME: tree rule RULE kind NUMBER arity NUMBER` | Keep a named rule in the binary application-tree projection. |

Square brackets in this reference table mean an optional part of a declaration.
Do not include those brackets in the specification. The parser header also
accepts `input tokens`, its default. The generated parser consumes token kinds.

### Expression notation

| Form | Meaning |
|---|---|
| `a b` | Match `a`, then `b`. |
| `a \| b` | Alternatives; [choice semantics](#possible-alternatives-versus-ordered-alternatives) depend on the parser mode. |
| `(a b)` | Group an expression. |
| `a?` | Zero or one occurrence. |
| `a*` | Zero or more occurrences. |
| `a+` | One or more occurrences. |
| `a{n}` | Exactly `n` occurrences. |
| `a{n,}` | At least `n` occurrences. |
| `a{n,m}` | Between `n` and `m` occurrences, inclusive. |
| `''` | An empty match. |
| `a as field` | Capture a parser match as a named field. |
| `&a` | In an ordered parser, require `a` to match next without consuming it. |
| `!a` | In an ordered parser, require `a` not to match next, without consuming it. |

Repetition binds more tightly than a sequence; a sequence binds more tightly
than alternatives. Use parentheses whenever the intended grouping is unclear.
The default maximum finite repetition count is 256; `grammar.repeat` changes
that build allowance.

Lexer expressions use named charsets, patterns and literals. Token parser
expressions use named tokens and rules. A nonempty literal such as `'='` is
invalid in a token parser: declare an `equals` token, emit it from the lexer,
and use `equals` in the parser rule.

The `&` and `!` forms are called **predicates** or lookahead tests. They consume
no input. For example, `identifier !equals` matches an identifier only when
an equals token does not immediately follow. Predicates require ordered
semantics; CFG backends reject them. Repetition must make progress: do not
repeat an expression that can succeed without consuming anything.

## Lexical rules and Unicode

### Tokens and matching priority

Use `identifier: token` to allocate a token number automatically, in declaration
order, or `identifier: token = 7` to choose one. Ordinary token IDs are 1–255.
Use token names in grammar rules and `definition.tokenid(name)` in application
code so that code does not depend on assigned numbers.

A lexical rule either emits a token or skips its match:

```rexx
digit: charset = '0' to '9'
digits: pattern = digit+
integer: rule = digits then emit number
whitespace: rule = (' ' | U+0009)+ then skip
```

A `pattern` is reusable and does not emit anything by itself. Every emitting
or skipping rule must consume at least one byte. A rule that can match nothing
is called **nullable** and is rejected with `LL_NULLABLE_TOKEN`.

At each input position, the lexer chooses the **longest match**. If several
rules match the same length, the earliest declared emitting or skipping rule
wins. A keyword rule before an identifier rule wins for the exact keyword;
a longer name such as `ifname` still matches the identifier rule.

Literal matching is case-sensitive. For case-insensitive keywords, use explicit
character alternatives or a [token stage](#token-stages).

Lexical definitions must be acyclic: a pattern cannot refer back to itself,
either directly or through other patterns. Use repetition for repeated text.
Use comment declarations for nested comments and parser rules for other nested
structures.

### Character sets

Charsets name the possible values of one character or byte:

```rexx
letter: charset = 'A' to 'Z' | 'a' to 'z'
digit: charset = '0' to '9'
name.start: charset = letter | '_'
line.character: charset = scalar except (U+0000 | U+000A | U+000D)
```

`to` includes both ends of a range. `|` combines sets. `except` removes the
right-hand set from the left-hand set. Subtraction binds more tightly than
union: write `scalar except ('"' | U+000A | U+000D)` to exclude all three values.
Without those parentheses, `scalar except '"' | U+000A | U+000D` removes the
quote and then adds the two line-ending characters.

Quoted values in a charset contain one character. `U+` followed by hexadecimal
digits writes its numeric value, such as `U+0020` for a space. Charsets can refer
to other charsets. `scalar` denotes all Unicode scalar values: U+0000 through
U+10FFFF, excluding the surrogate range U+D800–U+DFFF.

### Input profiles

The **input profile** specifies what character values in the lexer mean:

| Header | Meaning |
|---|---|
| `scan: lexer` or `scan: lexer input utf8` | Match Unicode scalar values in valid UTF-8 input. `U+00E9` matches `é`. Use this for text languages. |
| `scan: lexer input bytes` | Match raw byte values 0–255. Use this for binary formats. |
| `scan: lexer input utf8bytes` | Validate UTF-8, then match byte values 0–255. Use this for byte-oriented rules over UTF-8 data. |

In `utf8bytes`, `U+00E9` means the byte E9, not the two-byte UTF-8 encoding of
`é`. A quoted `'é'` supplies its UTF-8 bytes. Prefer `utf8` when describing text
by its characters.

All token and tree positions are byte offsets in the original source, whichever
profile is selected. U+0000 (NUL) is data; end of input is determined by the
buffer length. Keep raw binary slices as `.binary` rather than converting them
to `.string`.

### Unicode properties

Unicode properties provide character sets without manually listing ranges:

```rexx
letter: charset = unicode.XID_Start | '_'
continuation: charset = unicode.XID_Continue | '_'
identifier.lex: rule = letter continuation* then emit identifier
```

Properties require `input utf8`. The generated definition reports the data
version through `unicodeversion()`; the supplied data is Unicode **17.0.0**.
Recognition uses that data consistently across host locales.

| Property family | Available names |
|---|---|
| Identifier characters | `unicode.XID_Start`, `unicode.XID_Continue` |
| Alphabetic and case properties | `unicode.Alphabetic`, `unicode.Lowercase`, `unicode.Uppercase` |
| Whitespace | `unicode.White_Space`, `unicode.Pattern_White_Space` |
| Normally invisible characters | `unicode.Default_Ignorable_Code_Point` |
| General categories | `unicode.gc.` followed by `Lu Ll Lt Lm Lo Mn Mc Me Nd Nl No Pc Pd Ps Pe Pi Pf Po Sm Sc Sk So Zs Zl Zp Cc Cf Cs Co Cn` |
| Category groups | `unicode.gc.L`, `unicode.gc.M`, `unicode.gc.N`, `unicode.gc.P`, `unicode.gc.S`, `unicode.gc.Z`, `unicode.gc.C` |

Category letters stand for letters (L), marks (M), numbers (N), punctuation (P),
symbols (S), separators (Z) and other characters (C). For example, `gc.Lu` means
uppercase letters, `gc.Mn` nonspacing marks and `gc.Nd` decimal digits. `gc.Cs`
is empty here because Unicode scalar values exclude surrogates.

Property names are case-insensitive. A charset you explicitly declare with
the same name takes precedence over a builtin property. Unknown properties,
or properties used with a byte profile, are build errors.

Character recognition and name comparison are separate operations. Level L
preserves spelling. **Normalization** converts equivalent Unicode spellings
into a chosen standard form; **case folding** creates a form suitable for
case-insensitive comparison. Implement these explicitly in your application's
name handling when required. A whitespace property likewise specifies which
characters match; your lexical rules decide which ones separate statements.

### Lexical error messages and end of input

Two special lexical expressions let you name error messages:

```rexx
bad.encoding: diagnostic = 'Input must be valid UTF-8.'
bad.character: diagnostic = 'This character is not allowed here.'
scan: lexer
  /* Ordinary charsets and lexical rules go here. */
  invalid.encoding: rule = malformed then error bad.encoding
  invalid.character: rule = otherwise then error bad.character
```

`malformed` selects the UTF-8 validation message. `otherwise` selects the message
when no lexical rule matches. These messages are literal text; parser template
fields such as `{found}` are used only by `diagnostic rule` declarations.

The parser supplies end of input internally. For a lexer adapter that needs
EOF metadata, declare `end.token: token = 0` and use
`end.rule: rule = eof then emit end.token`. The ordinary token stream contains
no EOF record; do not append one before calling a Level L parser.

## Comments and layout

Use these simple declarations to choose the forms your language accepts:

```rexx
hash.notes: comments line '#'
slash.notes: comments line '//'
plain.notes: comments block '/*' '*/'
rexx.notes: comments nested '(*' '*)'
```

| Form | How it ends |
|---|---|
| `line 'OPEN'` | Just before CR, LF, CRLF or end of input. |
| `block 'OPEN' 'CLOSE'` | At the first closing delimiter. |
| `nested 'OPEN' 'CLOSE'` | When every opening delimiter of that form has a matching closer. |

For example, `(* outer (* inner *) outer *)` is one nested comment. Within a
comment, markers belonging to other comment forms are ordinary text. Thus a
`/*` inside a `#` line comment does not begin a block.

Comment processing replaces comment bytes with spaces in a scanning copy.
Include a rule that skips spaces in your lexer. The original source is retained,
so tree text and diagnostics still show the comment's spelling.

### Delimiters and overlapping forms

Delimiters are nonempty strings you choose. Block and nested forms require
different opening and closing strings. Opening strings must be unique across
declarations.

When several openings match at the same position, the longest wins. For example,
`#` line comments can coexist with `## ... $$` nested comments: `## notes $$`
is recognized as the nested form. Inside a nested comment, the closer takes
priority when an opening and closing string both match at one position.

An unclosed block produces `LL_COMMENT`, identifying its comment declaration
and opening position. UTF-8 validation occurs before comment processing, so a
malformed byte sequence in a comment is still an encoding error.

### Keep line endings when they matter

Line comments leave the line ending for the lexer. Block and nested comments
normally hide their internal line endings along with the rest of the comment.
To preserve them, add:

```rexx
plain.notes: comments block '(*' '*)' newlines keep
```

This retains the original CR and LF bytes in the scanning copy. A language whose
lexer emits them as separators can then start a new statement after a comment
containing a newline. A language that skips them continues to treat them as
whitespace. Indentation and line continuation require their own lexical or
application rules.

### Protect strings from comment recognition

By default, comment processing recognizes single- and double-quoted strings
with doubled quote escaping. Markers inside those strings are treated as string
content. This protection is called **shielding**. If a default quoted string is
unclosed, the rest of the input stays protected and the lexer reports the
string error.

For another string convention, give the comment declaration an `outside`
pattern. This example recognizes backslash-escaped strings:

```rexx
scan: lexer input utf8
  any.character: charset = scalar
  string.body: charset = scalar except ('"' | U+005C | U+000A | U+000D)
  quoted.text: pattern = '"' (string.body | U+005C any.character)* '"'
  string.lex: rule = quoted.text then emit string
notes: comments nested '/*' '*/' outside quoted.text
```

`outside quoted.text` replaces the default string protection for that comment
form. Apply it to each form that should use this convention. Comment markers
within a complete longest match of the pattern are ignored by that form. An
incomplete custom string gets no protection from that pattern; define its
lexical error handling accordingly.

The named pattern must consume input and use the supported acyclic lexical
expressions. Its opening must be distinguishable from the comment opener.
Generation rejects cases where either can be a prefix of the other's opening
match. Comment markers occurring later inside a complete string are allowed.

You can also skip a line comment with an ordinary lexical rule, such as
`'#' line.character* then skip`. When combining several styles, use shared
`comments` declarations so they all agree which text is already inside a string
or comment.

## Parser rules and operators

### Parser headers

These headers select the available parser modes:

```rexx
parse: parser start document
parse: parser start document backend ll1
parse: parser start document backend earley
parse: parser semantics ordered start document backend packrat
parse: parser start arithmetic backend pratt
```

Use one header in a specification. The first selects context-free semantics
with LR(1). Ordered semantics default to packrat when the backend is omitted.
You may write `semantics contextfree input tokens` explicitly.

Each backend checks that it can implement the rules. An unsupported construct
or conflicting grammar fails generation; there is no automatic backend change.

### Describe nested structure with rules

A parser rule refers to tokens or other parser rules:

```rexx
sum: rule = sum plus product | product
product: rule = product times atom | atom
atom: rule = number | leftparen sum rightparen
```

These rules make multiplication bind more tightly than addition: a `product`
is a unit within a `sum`. The left-recursive forms make successive operations
associate left. LR and Earley can use them. LL(1) and packrat require a
non-left-recursive expression of the same intended language.

Rule names also shape the named tree. A one-child rule remains visible, so even
a single number can have `sum`, `product` and `atom` wrappers. Use meaningful
rule names and inspect the tree before writing code that depends on child
positions. Captures can provide named access to important parts.

There are no application actions embedded in parser rules. Perform conversions,
validation and execution by reading the successful tree.

### Lists and optional parts

Use repetition to describe a list directly:

```rexx
names: rule = identifier (comma identifier)*
block: rule = open statement* close
call: rule = name lparen arguments? rparen
```

The first rule requires a name and then accepts any number of comma/name pairs.
A trailing comma is an error. The second allows an empty block; replace `*`
with `+` to require at least one statement. Its `statement` rule must include
any required statement terminator. The third makes the argument list optional.

Repetition helpers are hidden in the named tree. Use a repeated capture when
you want to retrieve just the items and leave separators out of the traversal:

```rexx
names: rule = identifier as item (comma identifier as item)*
```

`tree.fieldcount(node,'item')` gives the number of names, and
`tree.field(node,'item',index)` retrieves each one in source order. The fields
refer to matched source nodes; your application converts their text into values.

A repeated expression must consume input whenever it succeeds. For example,
`(identifier?)*` is rejected because its item can be empty. Parser admission
still applies to the expanded grammar. If generation reports a conflict,
inspect the alternatives and repeated part of the reported rule.

### Captures

Attach a field name with `as`:

```rexx
assignment: rule = identifier as name equals value as contents
items: rule = (identifier as item)*
```

The second rule produces one `item` field per identifier. In contrast,
`identifier* as items` captures the whole repetition as one field. An absent
optional capture has no occurrence. Capturing an explicitly empty expression
produces a zero-length occurrence.

Captures wrap matched nodes with labels such as `@field.name`. They leave
recognition unchanged and provide access through `field()` and `fieldcount()`.
They apply to parser expressions; lexer tokens already carry their text and
source range.

### Operator components

A specification can declare one operator component. It describes expressions
built from one atom token kind, a grouping pair, and declared prefix and infix
operators:

```rexx
arithmetic: operators atom number group lparen rparen
sum: infix arithmetic plus precedence 10 left
product: infix arithmetic times precedence 20 left
negative: prefix arithmetic minus
```

`sum`, `product` and `negative` become node names in the expression tree.
The infix tokens are binary operators; prefix tokens take one operand.
Parentheses produce a `group` node.

Larger precedence numbers bind more tightly. Use one associativity at each
precedence level: `left` or `right`. All prefix operators bind more tightly
than every infix operator. The grouping tokens, atom token and operator roles
must satisfy the generator's checks.

For a standalone expression parser, use `parser start arithmetic backend pratt`.
An operator component can also be translated into ordinary CFG rules for a CFG
backend that accepts them. Ordered parsers require explicitly authored ordered
rules rather than this CFG translation.

For expressions embedded in a larger language, use a region declaration as
shown next.

## Token stages and composite parsing

### Token stages

A token stage handles a limited amount of context between lexing and parsing.
It can reclassify a token, combine a pair, skip tokens, or pass them through.
For example, a statement keyword can also be allowed as a variable name.

This fragment assumes the tokens have already been declared:

```rexx
keywords: tokenstage start 1
say.keyword: transform stage keywords state 1 take identifier text 'SAY' unless next equals emit kw.say next 2
after.separator: transform stage keywords state 0 take separator emit copy next 1
ordinary.word: transform stage keywords state 1 take identifier emit copy next 2
```

State 1 means “at a statement start”; state 2 means “inside a statement”. Those
meanings are your design: the runtime stores the integer state. The `text`
condition compares ASCII text without case distinction. `unless next equals`
keeps `say = 3` as an assignment, while `say 3` begins a SAY statement.

The full form is:

```text
NAME: transform stage STAGE state STATE
  take TOKEN [TOKEN] [text 'TEXT'] [unless next TOKEN]
  emit TOKEN|copy|skip next STATE
```

| Part | Rule |
|---|---|
| `state` | Match a positive state number; 0 matches any current state. |
| `take` | Consume one or two token kinds. `any` matches any kind. |
| `text` | Compare the first consumed token's original text, ignoring ASCII case. |
| `unless next` | Reject this transform when the token immediately after the first input token has the specified kind. This is the second token even for a two-token `take`. |
| `emit TOKEN` | Emit that kind over the combined source range of the consumed tokens. |
| `emit copy` | Keep the first token's kind, using the consumed source range. |
| `emit skip` | Emit nothing. |
| `next` | Set a positive state number. |

The first matching transform wins. Unmatched input passes through with the
state unchanged. Stages run once each, in declaration order. They preserve
source coordinates and do not maintain a nesting stack. For parenthesis depth
or layout rules that require more state, insert an application token pass
using the [staged request API](#request-operations).

### Expression regions

A region sends a delimited part of the token stream to a Pratt component and
presents the result as one token to an outer LR parser:

```rexx
expression: token
math.region: region token expression using arithmetic
  open lbracket close rbracket
```

`expression` is a **virtual token**: the region processor produces it, so the
lexer must not emit it. The outer parser can use `expression` in its rules just
like another value. The delimiters `lbracket` and `rbracket` belong to the region;
`lparen` and `rparen` belong to the inner arithmetic component.

The generated language supports one such region declaration with an LR outer
parser and a Pratt component. Generation checks distinct delimiter ownership,
the operator component and the virtual token. The resulting tree joins the
outer structure to the expression tree, preserving original ranges.

An invalid inner expression reports its input location and opening region.
A missing closing delimiter is an error. During record recovery, separators
inside a declared region do not end an outer record.

### Choose the right processing step

| Need | Suitable component |
|---|---|
| Names, numbers, punctuation and strings | Lexer rules. |
| Line, block or nested comments | Comment declarations. |
| Contextual keywords or combining a fixed token pair | Token stage. |
| Parenthesis-sensitive layout or implicit operators | Application token pass. |
| Nested statements and IF/ELSE attachment | Parser rules. |
| Arithmetic inside explicit delimiters | Expression region. |
| Named access to matched parts | Captures in parser rules. |
| A smaller application tree | Tree projection or application conversion. |
| Names, types, duplicate declarations and execution | Application code using the tree. |

Start with a lexer, one parser and a reader. Add a stage only when there is a
specific transformation to make. Check its input tokens, output tokens and
source ranges with accepted and rejected examples.

## Runtime API reference

Import the generated namespace together with the Level L modules you use. The
settings reader imports `rxfnsb`, `llcore`, `lllanguage`, `llsource`, `lltree` and
`levell_settings`.

The generated `NAMESPACE..language()` function returns a
`.lllanguage..definition`. Create it once for a language, then create a request
for each document:

```rexx
definition = levell_settings..language()
request = .lllanguage..request(definition,source)
status = request.parse()
```

`source` is `.binary`. The optional third constructor argument is a
`.lllimits..policy`, described under [Resource limits](#resource-limits).

<a id="proposed-borrowed-language-requests"></a>

### Borrowed language requests

A caller-owned, fully prepared DFA/LR
language may serve fresh requests and readers without copying its packed tables
or production metadata. The owner remains live, anchored and unchanged for the
whole borrowed lifetime. These additions do not change the owning APIs above.

Keep the definition in anchored storage until the request and its readers end:

```rexx
language = levell_settings..language()
request = .lllanguage..request.borrowed(reference language,source)
status = request.parse()
```

The generic additions use existing types and explicit weak references:

| API | Exact arguments / result |
|---|---|
| `lllanguage.request.borrowed` factory | `language: reference .lllanguage..definition, source: .binary, ?limits: .lllimits..policy, ?comments: .llcomments..rules` |
| `lllanguage.definition.borrowedscanner` method | `source: .binary, mode: .string, fuel: .int = 10000000` → `.llmunch..scanner` |
| `lllanguage.definition.borrowedparser` method | `input: .llcore..stream, fuel: .int = 1000000` → `.lllr..parser` |
| `llmunch.scanner.borrowed` factory | `dfa: reference .llmunch..automaton, source: .binary, mode: .string, fuel: .int = 10000000` |
| `lllr.parser.borrowed` factory | `compiled: reference .lllr..tables, input: .llcore..stream, fuel: .int = 1000000` |
| `lllr.tables.productionlength`, `productionlhs` | Each takes `production: .int` and returns `.int`; delegate existing production-index lookups. |
| `lllr.tables.symbolkind`, `symbollabel` | Each takes `symbol: .int`; returns `.int` / `.string` respectively. |
| `lllr.tables.terminalname` | `token: .int` → `.string`; retain the existing parser's first matching terminal label, `token N` fallback and EOF `end of input`. |
| `llcfg.productions.symbolkind`, `terminalname` | `symbol: .int` → `.int` / `token: .int` → `.string`; read the owned grammar directly, without returning or copying it. |

The table queries delegate the existing CFG authority; move the parser's
terminal-label lookup there rather than adding a second algorithm. Preserve
terminal order, reduction labels and augmented-root behavior exactly. Existing
production/symbol bounds and error behavior remain. The parser uses these
queries during reductions and expected-token trials instead of value-returning
`grammar()` / `source()` calls. Definition wrappers borrow their private fields;
they add no public mutable-table reference getters. Scanner/parser owning and
borrowed factories feed the same recognition loops.

Definition wrappers require an anchored, live definition receiver: child weak
references point to its retained DFA/LR fields, never method locals, temporary
definition values or self-references into a subsequently copied wrapper.
The borrowed request supports prepared DFA lexing and the LR1 parsing route.
Other parse backends return status 3 with `LL_BORROW_BACKEND: borrowed parsing
requires LR1`; there is no silent value-copy fallback. An omitted limits argument
uses the existing default policy. Omitted comments copy the definition's baseline
rules; supplied comments copy and validate that request's override through the
existing rules API. The request never calls
`usecomments` on the owner. Each consumer retains its declared lexical rules
and exact source bytes.

The caller keeps every borrowed definition/automaton/table owner live, at a
stable storage location, unchanged and in the same VM worker until its last
borrower ends. This is an explicit precondition: reference validity detects
expired storage, not mutation or rebinding of a still-live owner. Borrowed
factories and owner-dependent operations refuse an empty, expired or live but
uninitialized owner with status 3 and an
`LL_BORROW_OWNER:` diagnostic, without reading it or publishing partial output.
Ordinary direct method-receiver guards remain unchanged. There is no generic
freeze or mutation-detection mechanism and no cross-worker sharing. Reader methods take
scoped live aliases; they do not store self-references into an owning wrapper
that may subsequently be returned or copied. Copying a borrower retains weak
borrows; it does not snapshot the heavy owner. Sources, token streams, policies,
comment rules and already materialized trees/ASTs retain their own data after
readers end. Later parsing/projection/AST construction still needs a live
definition for its metadata; expiry does not retroactively invalidate results.

### Definition methods

| Method | Result |
|---|---|
| `tokenid(name)` | Token number, or -1 when the name is unknown. |
| `tokenname(id)` | Declared name for a token number. |
| `hasparser()` | 1 when the definition includes a parser; 0 for a lexer-only definition. |
| `backend()` | Selected parser implementation; a composed expression language reports `regional`. |
| `encoding()` | Lexical input profile. |
| `unicodeversion()` | Unicode property-data version recorded in the definition. |

Metadata retains the spelling of declared names. Use that spelling, including
case, for string lookups such as `tokenid()` and comparisons with tree labels.
Capture-field lookup is case-insensitive.

### Request operations

| Method | What it does |
|---|---|
| `parse()` | Prepare the source, lex it, apply declared token stages and parse the result. Returns a status. |
| `lex()` | Prepare the source, lex it and apply declared token stages. Returns a status. |
| `lex(0)` | Prepare and lex, leaving declared token stages unapplied. Returns a status. |
| `classify_tokens(tokens)` | Apply the declared stages to a supplied token stream from the same source. Returns a status. |
| `parse_tokens(tokens)` | Parse supplied tokens from the same source. The caller handles source validation, lexing and classification. Returns a status. |
| `tokens()` | Token stream held by the request. |
| `tree()`, `root()` | Raw tree and root of a successful single-tree result. |
| `forest()`, `forestroot()` | Earley forest and its root. |
| `ast()` | Construct an owned application AST from `then tree` mappings. Requires a successful unique parse. |
| `projection()` | Create the declared application-tree projection. Call its `run()` method to perform it. |
| `offset()`, `error()` | Error's original byte offset and message. |

Use a fresh request for each document. Lexing is cached: calling `lex()` after
`lex(0)` does not classify the cached stream. To insert a custom token pass,
call `lex(0)`, perform that pass, call `classify_tokens(...)` if needed, then
call `parse_tokens(...)`. Check the status after every operation.

### Strict request statuses

These codes apply to `.lllanguage..request`:

| Status | Meaning | Caller action |
|---|---|---|
| 0 | Success | Read the tokens or tree; perform application validation. |
| 1 | Invalid input | Report `offset()` and `error()`. |
| 2 | Resource allowance exhausted | Report the named limit and adjust the workload or policy. |
| 3 | Invalid configuration or API use | Check the supplied stream, definition, source and policy. |
| 5 | Earley accepted the input without one unique finite tree | Inspect the forest or revise the grammar's ambiguity policy. |

A lexer-only success gives tokens. A successful `parse()` gives a single tree.
[Recovery requests](#recovery-requests) have their own status table, including
status 1 for a recovered document.

### Lexer-only applications

Omit the parser header and parser rules to generate a lexer-only module. Call
`request.lex(0)` for raw tokens or `request.lex()` to include token stages.
Calling `parse()` on that definition returns status 3 with `LL_NO_PARSER`.

Token indexes start at zero. For each `i` from 0 through `tokens.size()-1`:

```rexx
kind = tokens.symbol(i)
first = tokens.start(i)
last = tokens.finish(i + 1)
text = binsubstr(source,first + 1,last - first)
```

`start(i)` uses a token index. `finish(i + 1)` uses the boundary after that token.
Byte ranges are zero-based and half-open; `binsubstr` uses a one-based starting
position. `text` in this example is a binary slice.

Skipped text remains in the original source but has no ordinary token entry.
Use `definition.tokenname(kind)` to display a token's declared name.

### Supply tokens from another producer

Create a source-owning stream with `.llcore..stream.tokens(source)`. Append each
token with `append(kind,start,finish)`, checking its Boolean return and `error()`.
Ordinary token ranges are nonempty, ordered and nonoverlapping; gaps are allowed.
Use `definition.tokenid(name)` to obtain the expected kinds.

Call `request.parse_tokens(tokens)` with the completed stream. The stream must
refer to exactly the same source bytes as the request. If it needs the generated
classification stage, call `classify_tokens(tokens)` first and parse the
resulting `request.tokens()`.

### Parse forests

A forest node may have several **families**. Each family is one way of deriving
that node, with up to two child nodes. Common child structures are shared.

| Forest method | Result |
|---|---|
| `derivations(root,cap)` | Count up to `cap`; -1 for a reachable cycle, -2 for traversal exhaustion. |
| `validate()` | 1 when the forest's structural checks pass. |
| `nodes()`, `families()` | Total stored node and family counts. |
| `first(node)` | First family for a node, or 0. |
| `next(family)` | Next alternative family, or 0. |
| `left(family)`, `right(family)` | Child node identifiers; 0 means no child. |
| `start(node)`, `finish(node)` | Token-boundary positions within the parser input. |
| `label(node)` | Internal grammar label number. |

Forest coordinates are token positions. Use the request's token stream to map
them back to original bytes. A forest is a lower-level representation than the
named tree view; an application using it must handle alternative families and
cycles explicitly.

## Trees, fields and source locations

### The named view

Construct `.lltree..view(request.tree(),request.root())` after a successful
parse. Check `error()` before using it. The view retains authored rule names,
named tokens and capture wrappers while hiding internal grammar helper nodes.

| Method | Result |
|---|---|
| `root()` | Root node identifier. |
| `name(node)` | Rule, token or capture label. |
| `count(node)` | Number of immediate children. |
| `child(node,index)` | Child at a one-based index; 0 if the index is out of range. |
| `fieldcount(node,name)` | Number of matching immediate capture fields. |
| `field(node,name,occurrence)` | Capture node; occurrence defaults to 1. Returns 0 if absent. |
| `start(node)`, `finish(node)` | Original half-open byte range. |
| `text(node)` | Original text in the range, converted to `.string`. |
| `source()` | Complete original `.binary`, including skipped text. |
| `error()` | Empty on successful view construction; otherwise a message. |

Field names are case-insensitive. Field lookup stays within the named node:
a nested rule's field does not satisfy a lookup on its parent. Repeated fields
are returned in source order. Check for 0 before using an optional field.

Node identifiers belong to that tree or view. Do not reuse an identifier with
a different parse. For binary input, slice `source()` with `binsubstr` rather
than calling `text()`.

### Source ownership

Tokens and trees retain the original source. Whitespace and comments may be
absent from the token stream, but remain available through source ranges and
`source()`. A root's range can exclude leading or trailing skipped text; use
`source()` when reproducing the whole file.

Definitions, requests and returned trees use cREXX value semantics. Passing a
whole view by value can copy it. The example readers use iterative walks and
node identifiers to avoid passing the entire tree through each recursive call.
Packed table lookups read the stored tables directly; request construction and
public value returns can still copy data.

### Application trees with `then tree`

An **abstract syntax tree**, or AST, records the parts your application needs.
Use `then tree` to describe those parts beside the rule that recognizes them:

```rexx
number: token then tree Number(spelling = text)
program: rule = (statement as item)*
  then tree (items = item*)
statement: rule = identifier as name equals number as value
  then tree Assignment(name = text(name), value = value)
```

The token declaration makes a `Number` with its original spelling. The
`program` rule makes a node with an ordered list called `items`. The statement
rule makes an `Assignment` with a text field and a child field. `identifier` and
`equals` do not have mappings, so they do not become application nodes.

The declaration label supplies the default kind: `then tree (...)` on `program`
creates a kind named `program`. Put a name before the parentheses to choose a
different kind, as in `then tree Assignment(...)`. Names are case-insensitive;
the spelling of the declared kind is retained when `kind()` returns it.

| Field expression | Result |
|---|---|
| `name = text` | Text covered by this rule or token. |
| `name = text(capture)` | Text covered by one named capture in this rule. |
| `name = capture` | One application child from that capture. |
| `name = capture?` | Zero or one child. |
| `name = capture*` | An ordered list with zero or more children. |
| `name = capture+` | An ordered list with at least one child. |
| `left = left`, `right = right` | The operands of an infix operator. |
| `value = inner` | The operand of a prefix operator. |

`text` is a selector, not a conversion. `Number(spelling = text)` retains `007`
as `007`; the application decides whether to convert it to a number. Text from
a quoted string includes the quotes. There are no application expressions or
callbacks inside a mapping.

The two uses of `*` have related but distinct jobs. In the rule, `(statement as
item)*` recognizes repeated statements. In the mapping, `items = item*` collects
the captured results into a list. Each matched capture must supply one AST
child. Put the capture inside the repetition to capture every item separately.

Unmapped rules pass their mapped descendants through. This removes wrappers
such as `value: rule = number | string` without an additional mapping. A mapped
rule chooses its own fields; descendants that it does not choose are omitted.
The final result must have one root. An unmapped empty document therefore needs
a mapped enclosing rule if it is to produce an AST.

Level L infers a **schema**, the contract describing each kind's field names,
text or child fields, and allowed child counts. Mappings that use the same kind
must agree on that contract and field order. The grammar checker reports
unknown captures, duplicate fields, incompatible kind declarations and impossible
capture counts with a specification line and byte location. Construction also
checks that every selected child capture produces one application node.

For an embedded expression, put mappings on its tokens and infix/prefix
operators, or on an enclosing parser rule. The embedded component does not
retain the `operators` declaration as a wrapper node. A mapping attached to
that absent wrapper is a build error.

See [settings lesson 9](../examples/settings/09-ast.rxl) and its
[reader](../examples/settings-ast.crexx) for lists, original Unicode spelling,
comments and embedded arithmetic in one complete example.

### Construct and read an AST

Import `llast` in the application, then construct the AST after a successful
parse:

```rexx
tree = request.ast()
if tree.status() <> 0 then do
  say tree.error()
  return tree.status()
end
root = tree.root()
items = root.each('items')
do while items.next()
  assignment = items.node()
  say assignment.text('name')
end
```

The AST owns its source and nodes, so the request can be released after
construction. `request.ast()` rejects failed parses and ambiguous forests. A
recovery reader handles its damaged records separately; an AST request does not
silently turn damaged input into a clean program.

| Tree method | Result |
|---|---|
| `status()` | 0 for a clean complete tree, 1 for a recovered inspection tree, 2 for a resource limit, 3 for a contract failure. |
| `error()` | Empty on success; otherwise the diagnostic. |
| `root()` | A handle to the root; invalid on failure. |
| `source()` | The complete original bytes, including skipped text. |
| `entries()` | Allocated nodes, field slots and child links. |
| `walk(order)` | An iterative walk; defaults to `children first`. |
| `rewrite(order)` | A checked reconstruction; accepts `children first`. |
| `analyse(limit)` | A children-first analysis; defaults to one million visited nodes. |

A **handle** is a small reference to one node. Copying a handle does not copy
the tree. Keep its owning `tree` alive and do not reassign that tree while using
its handles or walks. Copying the owning tree itself makes an independent value.
A handle from the first tree still belongs to that first tree.

| Node method | Result |
|---|---|
| `valid()` | Whether the owner and node still exist. |
| `kind()` | Declared kind, such as `Assignment` or `Number`. |
| `text(name)` | A text field's value. |
| `count(name)` | Number of children in a child field. |
| `child(name,index)` | Child handle; index defaults to 1. Invalid when the optional child or list index is absent. |
| `start()`, `finish()` | Original half-open byte range. |
| `offset()` | Diagnostic anchor within that range; initially its start. |
| `line()` | One-based source line containing the diagnostic anchor; CRLF counts as one line ending. |
| `children()`, `at(index)` | Child count and indexed access across all fields. |
| `each(field)` | A linear child cursor; omit `field` to visit all child fields. |
| `sourcetext()` | Original text covered by that range. |
| `origin()` | `exact`, `composite` or `derived`, as explained below. |
| `id()` | Tree-local integer, useful for storing calculated values by node. |

An unknown field, or using `text()` on a child field, raises an `ERROR` signal
containing `LL_AST_FIELD`. An expired or replaced tree raises an `ERROR` signal containing
`LL_AST_OWNER`; `valid()` lets you check a handle first. Use `count()` or the returned handle's
`valid()` when an optional child may be absent.

### Calculate facts about a tree

An **analysis** calculates information about a tree without changing it. For
example, a calculator can find constant values; a script checker can list the
variables an expression reads. Level L supplies the traversal and result table.
Your program supplies the calculation and the class used for each result.

Call `tree.analyse()` and record one object for each node. Children are visited
before their parent, so a parent can use their completed results. Here is a
complete calculation of subtree sizes:

```rexx
sizefact: class
  _size = .int
  *: factory
    arg size = .int
    _size = size
    return
  size: method = .int
    return _size

/* Inside the procedure that owns tree: */
pass = tree.analyse()
do while pass.next()
  node = pass.node()
  size = 1
  children = node.each()
  do while children.next()
    child = pass.get(children.node()) as .sizefact
    size = size + child.size()
  end
  ok = pass.put(.sizefact(size) as .object)
end
facts = pass.finish()
if facts.error() <> '' then do
  say facts.error()
  return 1
end
result = facts.get(tree.root()) as .sizefact
say result.size()
```

The cast `as .object` stores the application's class in the common result table.
The cast `as .sizefact` checks its type when reading it back. `each()` visits
every child without copying the list. Use `each('items')` for one named list,
or `child('value')` to read a single field. A cursor has `next()`, `node()` and
`error()` methods. Indexed access is convenient for a particular item; a cursor
visits a long list in linear time.

| Analysis method | Meaning |
|---|---|
| `next()` | Advance to the next node; zero when finished or failed. |
| `node()` | Current input node. |
| `get(node)` | Read a completed dependency's result. |
| `put(object)` | Record this node's result exactly once. |
| `finish()` | Publish the complete result table. |
| `error()` | Empty until a contract or resource check fails. |

The completed table provides `get(node)`, `has(node)`, `size()` and `error()`.
Keep the input tree alive while using its results. A copied or rewritten tree
has its own identity and needs its own analysis. Reading another tree's facts
reports `LL_AST_OWNER`; reading an unfinished dependency reports
`LL_AST_DEPENDENCY`. An incomplete or failed pass publishes no usable results.
These checks prevent a transformation from accidentally using stale facts.

`tree.analyse(limit)` bounds the number of visited nodes and result records.
Exhaustion reports `analysis.nodes`. The calculation and the size of each result
belong to the application. This pass has no repeated callbacks: each node is
visited once, and its result must depend on results already available.

### Add sparse properties and defaults

Use **properties** for information your application attaches to nodes: a review
message, a priority, or the result of a check. Properties are separate from the
fields that define tree structure. You can add a message to an Assignment
without changing its required `name` and `value` fields.

Declare a group after the grammar:

```rexx
notes: properties
  message: property .string default ''
  priority: property .int default 0
```

The group owns the successive `property` declarations. The next declaration of
another kind ends the group. Indentation makes the relationship easy to read;
it does not establish scope. Names within a group are case-insensitive. Use
simple names without dots for groups and properties. The group name becomes
a generated class or procedure; `language` is reserved for the parser factory.

With namespace `flowdemo`, these declarations generate a `.flowdemo..notes`
class. Create a draft for your tree, write the properties you need, then finish:

```rexx
draft = .flowdemo..notes(reference tree)
call draft.setmessage(node,'Check this assignment')
call draft.setpriority(node,2)
notes = draft.finish()
if notes.error() <> '' then do
  say notes.error()
  return 1
end
note = notes.at(node)
say note.getmessage()
say note.hasmessage()
```

Each property has `get` and `has` methods on the node view, and a `set` method
on the draft. Thus `message` generates `getmessage()`, `hasmessage()` and
`setmessage(node,value)`. The setter returns 1 on success and 0 on failure.
Unknown accessors and values of the wrong type are diagnosed when compiling
your cREXX program.

**Sparse** means that the table stores only values you explicitly set. An unset
priority reads as 0 and `haspriority()` returns 0. Setting priority to 0 makes
`haspriority()` return 1: the value is now explicit. Reading a default does not
insert an entry. `size()` counts explicit node/property pairs; setting the same
pair again replaces its value.

Defaults are literal `.string` or `.int` values. Strings use Rexx quoting and
integers may have a sign. Each group stores its defaults once. A property group
can annotate any kind of node in its owning tree.

`finish()` freezes the draft and returns an independent finished group. Check
its `error()` before reading it. A failed draft cannot publish readable partial
values. Writing after finish or finishing twice fails. Keep the tree alive
while using a group, and keep the finished group alive while using an `at(node)`
view. Another tree, including a copied or rewritten tree, needs its own group.
Replacing the owning tree with a different snapshot invalidates its groups.

| Generated group operation | Meaning |
|---|---|
| `.notes(reference tree,limit)` | Create a draft; `limit` defaults to 1,000,000 explicit entries. |
| `setmessage(node,value)` | Write or replace a string property. |
| `setpriority(node,value)` | Write or replace an integer property. |
| `finish()` | Freeze and return a finished group. |
| `at(node)` | Borrow a view of one node's properties. |
| `size()` | Number of explicit entries; reads of defaults do not change it. |
| `error()` | Empty on success; otherwise the ownership, lifecycle or resource error. |
| `view.getmessage()`, `view.getpriority()` | Return an explicit value or its declared default. |
| `view.hasmessage()`, `view.haspriority()` | Whether that property was explicitly set. |

### Describe execution order with flow mappings

A tree describes how source is nested. A **flow graph** describes the order in
which operations can happen. These are different questions. An IF has a
condition and two branches in its tree; execution evaluates the condition and
then follows one branch.

A flow declaration tells the graph builder how each AST kind connects its
children's execution paths:

```rexx
execution: flow
  Program: sequence items
  Block: sequence items
  Assignment: assign name value
  If: branch condition then yes else no
  Say: evaluate value
  Name: read spelling
  Number: value
```

The declaration is a recipe shared by every node of each named kind. For each
particular `If` in the parsed source, the builder looks up that node's fields
and constructs its part of the flow graph. It retains links back to the AST
and source locations. Your application constructs the graph explicitly:

```rexx
tree = request.ast()
graph = flowdemo..execution(reference tree)
```

Building the graph describes possible execution. It does not run the target
program or choose an IF branch from a runtime value.

#### From grammar captures to AST fields

Read this declaration as a sentence: "For an If node, branch on its condition
child; the true arm is its yes child, and the false arm is its no child."

```rexx
If: branch condition then yes else no
```

| Part | What it names |
|---|---|
| `If` | An AST kind declared by `then tree If(...)`. |
| `branch` | The library's branching operation. |
| `condition` | A child field on this If node, containing its expression AST. |
| `then`, `else` | Keywords in the flow declaration. |
| `yes`, `no` | Child fields on this If node, containing its two arms. |

`condition`, `yes` and `no` are names chosen by the grammar author. They are
neither flow-point identifiers nor Boolean values. They are defined on the
left of the equals signs in `then tree`:

```rexx
conditional: rule = kw.if expression as test kw.then block as taken (kw.else block as alternative)? kw.end semi
  then tree If(condition = test, yes = taken, no = alternative?)
```

There are two sets of names here:

- `as test`, `as taken` and `as alternative` name **captures**: the pieces of
  this rule's successful match that the AST mapping will use.
- `condition`, `yes` and `no` name the resulting **AST fields**. For example,
  `condition = test` puts the expression captured as `test` into the child
  field called `condition`.

The flow declaration refers to the AST fields. It does not refer directly to
the captures. Many examples use the same name for both, such as
`condition = condition`; this is a naming convenience. If you rename the AST
field, update the flow declaration to use that name too. The grammar checker
checks this connection before generating the language module.

For the following input:

```text
if flag then
  x = 10;
else
  x = 20;
end;
```

the relevant tree is:

```text
If
  condition -> Name
                 spelling = "flag"
  yes       -> Block
                 items[1] -> Assignment
                               name = "x"
                               value -> Number(spelling = "10")
  no        -> Block
                 items[1] -> Assignment
                               name = "x"
                               value -> Number(spelling = "20")
```

Each arrow above is a child field or list member. `spelling` and `name` hold
text. If the source has no ELSE, `no` has zero children because its mapping
uses `alternative?`.

Application code follows exactly these links. Given a handle called `ifNode`:

```rexx
testNode = ifNode.child('condition')
thenBlock = ifNode.child('yes')
if ifNode.count('no') = 1 then elseBlock = ifNode.child('no')
```

The graph builder does the same lookup using the fields named in the flow
declaration. It constructs flow for the condition expression and each block,
then connects them:

```text
                       true  -> process x = 10 --+
read flag -> decision <                         +-> join -> following statement
                       false -> process x = 20 --+
```

The decision and join are **flow points** created by the builder. They are not
extra fields declared on the AST. An AST node can contribute several flow
points: this If contributes entry, decision and join points, while its children
contribute their own points. The tree records nesting; the graph records
possible execution order.

#### Flow operation reference

| Mapping | Flow represented |
|---|---|
| `Kind: sequence items` | Visit the children of `items` in order. |
| `Kind: evaluate left right` | Visit the listed child fields in order, and each field's children in order. One or more fields may be listed. |
| `Kind: assign name value` | Evaluate the one `value` child, then assign the variable in text field `name`. |
| `Kind: branch condition then yes else no` | Evaluate `condition`, then connect a true path to `yes` and a false path to `no`. Their normal completions join. An absent optional `no` goes straight to the join. |
| `Kind: read spelling` | Read the variable in the text field `spelling`. |
| `Kind: value` | A leaf value with no variable reads or writes. |
| `Kind: loop while condition do body` | Test before each iteration. False leaves the loop; the body may run zero times. |
| `Kind: loop do body until condition` | Run the body, then test. True leaves the loop; false repeats. |
| `Kind: loop do body` | Repeat the body until an explicit exit. |
| `Kind: break` | Leave the nearest enclosing loop. |
| `Kind: continue` | Start the nearest loop's next test, or its next body for an unconditional loop. |
| `Kind: return value` | Evaluate the `value` child when present, then leave the analysed root. |
| `Kind: return` | Leave the analysed root immediately. |
| `Kind: shortcircuit left and right` | Evaluate `left`; evaluate `right` only on the true path. |
| `Kind: shortcircuit left or right` | Evaluate `left`; evaluate `right` only on the false path. |

`sequence` is convenient for a statement list; `evaluate` names one or more
child fields such as operands. Both preserve the specified order. `value`
means that a leaf has no flow work inside it. It does not calculate a value or
declare that a condition is true. An assignment's write follows the flow of its
value child, so `x = x;` reads x before assigning it. `Say: evaluate value`
describes evaluation of the argument; output remains application behavior.

Every AST kind needs a mapping. Every child field needs a declared execution
role; text metadata may be left unused. `condition` and `yes` must each contain
one child. `no` may contain one child or be optional. The grammar checker
reports unknown kinds, misspelled fields and incompatible field types or
counts before generating code. For example, using `test` when the actual field
is `condition` reports the available fields.

Loop conditions, loop bodies and short-circuit operands are single required
children. A return value may be required or optional. Use a Block with
`sequence items` to hold several loop statements. `break` and `continue` have
no child fields. A jump outside a loop reports its AST kind, line and byte
offset when the graph is built, including when the jump follows a return.

Only paths that finish normally join the following statement. `break` goes to
the loop exit, `continue` goes to the next test or iteration, and `return`
ends the current analysis root. Analyse each function body as a separate root;
these mappings do not model calls between functions. A binary arithmetic node
whose children always execute can use `evaluate left right`. Give calls and
other effects an explicit application contract before analysing them.

Short-circuit expressions preserve their true and false outcomes through nested
short-circuit expressions and into a branch or loop test. When used as an
ordinary value, both outcomes join after evaluation. `evaluate` describes
ordered evaluation; it does not declare that the parent's truth value equals
one of its children's truth values.

#### Conditions and analysis facts

The condition remains an ordinary expression AST. For `if flag`, it is a Name
node; for a language supporting `if 2 + 3 > 4`, it would contain the arithmetic
and comparison nodes. A flow mapping describes when that expression executes.
An analysis of values answers what it can mean.

The built-in `definiteassignment()` analysis tracks variable reads and writes.
By default it considers both branch edges possible, including when the condition
is a literal. Supplying `flag` as an entry variable says it is assigned; it does
not supply flag's value. To exclude an outcome, supply an explicit condition fact
before building the graph.

Your application can inspect the condition while analysing the graph. For a
decision in the If mapping above, the graph provides its source If node:
Check `graph.op(point) = 'branch'` and `graph.node(point).kind() = 'If'`:

```rexx
ifNode = graph.node(point)
testNode = ifNode.child('condition')
```

This example uses the field name declared above. The source link supplies the
whole expression, not just its text or a branch label. An application analysis
can combine that expression with facts about the program:

| Kind of fact | Example | How it helps reasoning |
|---|---|---|
| Constant expression | `2 + 3 > 4` is true under the language's numeric rules. | The false edge cannot be taken after successful evaluation. |
| Value at a program point | Every incoming path assigns flag the value 1, and the language treats 1 as true. | This particular test of flag has a known outcome. |
| Fact on a branch edge | The condition is `x > 0`. | On the true edge, a value analysis can use `x > 0` until an operation changes that knowledge. |

These are different questions from whether x or flag has been assigned. A
value-aware analysis needs the target language's arithmetic, comparison and
truth rules. It must account for writes, joins, and any effects or errors in
condition evaluation. Knowing the result of a condition does not by itself
justify removing the work needed to evaluate it. Unknown values retain both
possible outcomes.

The common AST [analysis service](#calculate-facts-about-a-tree) can store
application-defined facts against nodes. [RexxScript's constant
analysis](#analyse-and-fold-a-script) is a worked consumer. Convert a proven
Boolean result into a condition fact to use it in flow analysis; storing a
constant or a sparse property alone does not change the graph.

#### Supply a known condition

A **condition fact** says that an expression has a particular truth value every
time it completes normally. The application establishes the fact using its
language's rules. For example, if that language defines literal `1` as true:

```rexx
facts = .llflow..conditions(reference tree)
condition = tree.root().child('items',1).child('condition')
ok = facts.set(condition,'true','Literal 1 is true in this language')
graph = flowloops..execution(reference tree)
ok = graph.using(reference facts)
result = graph.definiteassignment()
```

For `if 1 then x = 10; end; say x;`, the fact removes the false outcome. The
analysis now finds that `x` is assigned before `say x`. Expression evaluation
still happens. A condition such as `set(x,1)` writes `x` even when its result
is known. Similarly, a false left operand of AND skips the right operand,
and a true left operand of OR skips it. A fact used inside a loop must remain
true on every visit to that expression, not just the first iteration.

Use only `true` or `false`; omitted facts mean unknown. Each fact requires a
nonempty reason for inspection and diagnostics. The library checks ownership,
conflicting facts and the resource allowance. The application is responsible
for the truth of its claim: a reason string is an explanation, not a proof
checked by a theorem solver. Facts about a complete expression can exclude its
outcome without inferring additional facts about its operands.

Build the complete [conditions example](../examples/flow/conditions.crexx):

```sh
crexx --program flow-conditions langtools/examples/flow/loops.rxl \
      langtools/examples/flow/conditions.crexx
rxvm flow-conditions
```

It reports two possible unassigned reads without facts, zero with facts,
`x assigned before SAY: 1`, and `Skipped operand reachable: 0`.

| Condition method | Meaning |
|---|---|
| `.llflow..conditions(reference tree,limit)` | Create a fact table for one tree; the default allowance is 1,000,000 stored facts. |
| `facts.set(node,value,reason)` | Store `true` or `false`. Repeating the same value retains the original reason; a conflicting value fails. |
| `facts.get(node)` | Return `true`, `false` or `unknown`. |
| `facts.reason(node)` | Return the stored explanation, or an empty string for an unknown fact. |
| `facts.size()`, `facts.error()` | Stored fact count and any contract/resource error. |
| `graph.using(reference facts)` | Attach the table once, before querying the graph or starting analysis. |

The first graph query seals the table. Keep both tree and facts alive while
using the graph or its results. Replacing either owner, or trying to mutate
sealed facts, invalidates dependent answers. The `flow.conditions` allowance
counts facts; it is separate from the graph and solver allowances.

### Worked example: find a read before assignment

The complete example is in [examples/flow](../examples/flow). Build it with:

```sh
crexx --program flow-check langtools/examples/flow/rules.rxl \
      langtools/examples/flow/check.crexx
rxvm flow-check
```

The grammar's assignment rule is ordinary Level L:

```rexx
assignment: rule = identifier as name equals expression as value semi
  then tree Assignment(name = text(name), value = value)
```

Its IF rule captures a condition, a required block and an optional ELSE block:

```rexx
conditional: rule = kw.if expression as condition kw.then block as yes (kw.else block as no)? kw.end semi
  then tree If(condition = condition, yes = yes, no = no?)
```

For this source, the host supplies `flag`:

```text
if flag then
  x = 10;
end;
say x;
```

The tree has an If followed by a Say. The If's `yes` child is a Block containing
an Assignment, and its `no` field is empty. The important execution paths are:

```text
                 true -> assign x ---+
read flag ------<                    +--> read x
                 false --------------+
```

**Definite assignment** asks whether a variable has been assigned on every path
that reaches a particular point. Here the false branch reaches `say x` without
assigning `x`, so the answer for `x` is false.

```rexx
graph = flowdemo..execution(reference tree)
result = graph.definiteassignment('flag')
if result.error() <> '' then do
  say result.error()
  return 1
end
say result.before(sayNode).assigned('x')  /* 0 */
```

The argument `'flag'` names variables already assigned at entry. Supply several
names separated by spaces. Variable names are case-sensitive and retain their
exact spelling. A language with case-insensitive variables should normalize
its AST names consistently before constructing flow. This profile uses a
single variable scope.

A completed result can contain diagnostics. That is distinct from `error()`,
which means the analysis could not produce a usable answer:

```rexx
do i = 1 to result.count()
  node = result.node(i)
  say llsource..diagnostic(tree.source(),node.offset(),'UNASSIGNED',result.message(i))
  say result.witness(i)
end
```

The example reports:

```text
UNASSIGNED at line 4, column 5 (byte 32): x may be read before it is assigned.
  say x;
      ^
From entry: Take the false branch of If on line 1. Reach this read without assigning x.
```

Add `else x = 20;` before `end;` and both paths assign `x`. The diagnostic goes
away and `assigned('x')` before Say returns 1. An assignment such as `x = x;`
still reports the read on its right-hand side: the write happens afterwards.
The checker considers both branches possible; it does not evaluate conditions.

The example also copies each diagnostic message into `notes`, demonstrating
how calculated facts and application annotations fit together. Flow analysis
does not change the AST or execute the target program.

| Flow or result method | Meaning |
|---|---|
| `execution(reference tree,limit)` | Construct checked flow; `limit` counts events plus edges and defaults to 1,000,000. |
| `graph.error()` | Construction or ownership error. |
| `graph.points()`, `graph.edges()`, `graph.entries()` | Successful graph dimensions. |
| `graph.op(point)` | Operation at a flow point, such as `read`, `write`, `branch` or `join`. Point IDs run from 1 to `points()`. |
| `graph.node(point)` | The AST node responsible for that flow point. Follow its fields to inspect the corresponding expression or statement. |
| `graph.from(link)`, `graph.destination(link)` | Source and destination point IDs for an edge. Edge IDs run from 1 to `edges()`. |
| `graph.label(link)` | `true` or `false` for a branch choice, `break` or `continue` for a jump; empty for an ordinary edge. |
| `graph.definiteassignment(names,cells,work)` | Analyse with entry names; defaults are 1,000,000 state cells and 10,000,000 work units. |
| `result.error()` | Empty when the analysis completed successfully. |
| `result.before(node).reachable()` | Whether entry can reach this node in the flow graph. |
| `result.before(node).assigned(name)` | Whether the name is definitely assigned on entry to that AST node. Unknown names return 0. |
| `result.count()` | Number of unsafe reads in a completed result. |
| `result.node(i)` | Source AST node for diagnostic `i`, numbered from 1. |
| `result.message(i)` | The unsafe-read message. |
| `result.witness(i,limit)` | One actual entry-to-read path without an assignment; default 1,000,000 edge visits. |
| `result.work()` | Work consumed by the analysis. |

Keep the tree, graph and result alive while using their borrowed views.
Replacing a graph or result invalidates its earlier views. Failed construction,
exhausted allowances and expired owners expose no usable answers. Resource
errors name `flow.entries`, `analysis.cells`, `analysis.work` or `witness.work`
so the caller can increase the relevant allowance.

The analysis tracks names that may still be unassigned. At a join it combines
those names from the incoming paths. An assignment removes its name from the
outgoing set. New information only adds possibilities at each point; with a
finite graph and finite set of names, that process reaches a stable result.
The guarantee is relative to the execution roles declared in the flow mapping.

### Worked example: loops, exits and skipped operands

Build [loops.rxl](../examples/flow/loops.rxl) with
[loops.crexx](../examples/flow/loops.crexx):

```sh
crexx --program flow-loops langtools/examples/flow/loops.rxl \
      langtools/examples/flow/loops.crexx
rxvm flow-loops
```

Start with an ordinary grammar rule that captures the condition and body:

```rexx
pretest: rule = kw.while expression as condition kw.do block as body kw.end semi
  then tree While(condition = condition, body = body)
```

The flow group explains how those children execute:

```rexx
execution: flow
  While: loop while condition do body
  Until: loop do body until condition
  Forever: loop do body
  Break: break
  Continue: continue
  Return: return value
```

`While`, `Until` and `Forever` are AST kinds chosen by this grammar. `condition`
and `body` name their fields. The words `loop`, `while`, `do` and `until` select
the execution rule. The complete file also contains the mappings for blocks,
assignments, reads and the other node kinds.

The example analyses this program with `flag` and `other` assigned at entry:

```text
while flag do
  if other then break; end;
  x = 1;
  continue;
end;
say x;
return;
say missing;
```

The first `say` can run without assigning x: the WHILE test might be false
immediately, or the IF might take BREAK. The diagnostic points to x and shows
one such path. CONTINUE goes back to the WHILE test. The final `say` cannot be
reached because RETURN ends the program, so it produces no unsafe-read message.

```rexx
result = graph.definiteassignment('flag other')
if result.error() <> '' then do
  say result.error()
  return 1
end
say result.before(lastSay).reachable()  /* 0 */
```

`reachable()` means that the constructed graph has a path from entry to this
node. `assigned('x')` is true when x has been assigned on every path reaching
it. An unreachable node returns 0 for both queries. Test reachability first
when presenting assignment information about dead code.

For an UNTIL loop, normal completion and CONTINUE reach the test after the
body. BREAK skips that test. An unconditional loop has no implicit exit; code
after it is reachable only through a BREAK. Nested jumps target the innermost
loop. A RETURN inside any number of loops still leaves the analysis root.

The small example language spells short-circuit expressions as `both(a,b)`
and `either(a,b)`, and assignment expressions as `set(name,value)`. These make
execution order visible without adding operator precedence to the lesson:

```rexx
  Both: shortcircuit left and right
  Either: shortcircuit left or right
  Set: assign name value
```

Compare these inputs:

```text
say both(flag, set(x, 1));
say x;
```

The second statement reports an unsafe read: false flag skips the assignment.
Now use the expression as a condition:

```text
if both(flag, set(x, 1)) then
  say x;
end;
```

Reaching the true branch requires evaluating the right operand, so x is
assigned before that SAY. With `either`, reaching the false branch requires
evaluating the right operand. This is reasoning about execution paths; the
analysis does not calculate the truth of the literal 1.

Loops may repeat indefinitely in the target program, but the analysis still
terminates. It tracks a finite set of variable names and only adds facts about
possible unassigned paths until no facts change. The `analysis.work` allowance
can stop a large calculation with a resource diagnostic. Check `result.error()`
before reading results; an interrupted analysis has no usable partial answers.


### Walk the nodes

A walk follows named child fields in their declaration order and list members
in their input order:

```rexx
walk = tree.walk('parents first')
do while walk.next()
  node = walk.node()
  say node.kind() || ': ' || node.sourcetext()
end
if walk.error() <> '' then say walk.error()
```

`parents first` visits a node before its children. It is useful for displaying
the tree or finding declarations. `children first` visits children before their
parent. It is useful for calculating a parent's value from its children's
values, as the calculator does. These are also called *preorder* and *postorder*.
`walk.depth()` gives the current nesting depth: 0 for the root, 1 for its
children, and so on.

The walk uses an explicit stack, so deeply nested input does not require a
recursive application procedure. Each reachable node is visited once. For a
tree with N nodes, F field slots and E child links, walking takes O(N + F + E)
operations; the cursor stack grows with nesting depth. Here O(...) describes how
work grows with tree size, not a time in seconds. Each child has one parent;
construction rejects shared children rather than traversing them repeatedly.

A walk is a structural operation. An interpreter still decides which IF branch
to execute and when to stop a loop. Walking every node is appropriate for
inspection and pure calculations, not for executing both branches of an IF.

### Rewrite into a new tree

A **rewrite** replaces selected nodes while preserving the input tree. This
example folds additions of numeric literals:

```rexx
pass = tree.rewrite('children first')
do while pass.next()
  node = pass.node()
  if node.kind() = 'Add' then do
    left = node.child('left')
    right = node.child('right')
    if left.kind() = 'Number' & right.kind() = 'Number' then do
      value = (left.text('spelling') as .decimal) + (right.text('spelling') as .decimal)
      replacement = pass.make('Number')
      ok = replacement.settext('spelling',value as .string)
      ok = pass.replace(replacement)
    end
  end
end
folded = pass.finish()
if folded.status() <> 0 then do
  say folded.error()
  return folded.status()
end
```

At each step, `node` already contains the rewritten children. Replacing a child
therefore affects what its parent sees later in the same pass. Unchanged nodes
are copied into the result automatically. Replacement nodes are not revisited.
Call `finish()` after `next()` returns 0; it validates and publishes a complete
new tree. The original remains usable after success or failure.

`pass.original()` gives the corresponding input node. Use it to read facts
calculated before rewriting; `pass.node()` gives the output node with its
already-rewritten children. Keeping these roles explicit prevents mixing the
identities of the two trees.

`make(kind)` creates a **draft**, a replacement under construction. The kind
must occur in the inferred schema. Set text fields with `settext(name,value)`
and add children with `append(name,child)`. Children must come from the current
pass's nodes. A draft belongs to the current step; after `replace()` accepts it,
it cannot be edited. Required fields and child counts are checked. Foreign
children, cycles, reused children and stale drafts are errors. A failed pass
publishes no root, and `finish()` can be called only once.

A reconstruction visits each original node once. With a fixed schema and a
bounded amount of application work per node, its work grows linearly with tree
size. The `tree.entries` limit also counts replacement nodes, fields and links,
including intermediates that later become unreachable. Application arithmetic
and arbitrary callback loops remain the application's responsibility.

The [complete folding calculator](../examples/calculator/fold.crexx) handles
all of the calculator's operators. Build it with the same grammar:

```sh
"$task_bin/crexx" --program build/fold \
  langtools/examples/calculator/fold.crexx \
  langtools/examples/calculator/calculator.rxl
"$task_bin/rxvme" build/fold.rxbin \
  -a langtools/examples/calculator/example.calc
```

It reports the original expression, the resulting `Number` and `RESULT|11`.

### Lower into a simpler tree

**Lowering** translates a convenient source construct into simpler constructs
that later compiler passes already understand. For example, the language in
[examples/lowering](../examples/lowering) has an `unless` statement:

```rexx
conditional: rule = kw.unless expression as condition kw.do block as body kw.end semi
  then tree Unless(condition = condition, body = body)
```

`unless flag do x = 10; end;` executes its body when `flag` is false. We can
represent it using an ordinary If:

```text
Unless                         If
  condition -> Name(flag)        condition -> Name(flag)
  body -> Block                  yes -> Block (empty)
            Assignment(x,10)     no  -> Block
                                         Assignment(x,10)
```

The new tree has its own **schema**: the list of kinds, named fields and allowed
child counts. The example's `target()` procedure declares Program, Block, If,
Assignment, Say, Name and Number with `.llast..schema()`. There is no Unless in
that schema. Later passes need only understand If.

```rexx
pass = tree.lower(target())
do while pass.next()
  source = pass.original()
  if source.kind() = 'Unless' then do
    empty = pass.make('Block')
    yes = pass.accept(empty)
    replacement = pass.make('If')
    ok = replacement.append('condition',pass.translated(source.child('condition')))
    ok = replacement.append('yes',yes)
    ok = replacement.append('no',pass.translated(source.child('body')))
    ok = pass.replace(replacement)
  end
  else ok = pass.keep()
end
lowered = pass.finish()
```

Children are visited before their parents. `source` belongs to the original
tree; `translated(source.child(...))` retrieves the completed target child.
`make()` creates a draft in the target schema. `accept()` checks and seals an
intermediate node so another draft can use it. `replace()` records the result
for the current source node. `keep()` explicitly copies the current kind and
fields using the translated children. Every visited source node needs one
`replace()` or `keep()`; forgetting a case reports its kind and source byte.

Build and run the complete example:

```sh
crexx --program lower-example langtools/examples/lowering/unless.rxl \
      langtools/examples/lowering/lowering.crexx \
      langtools/examples/lowering/show.crexx
rxvm lower-example
```

It shows `Unless` becoming `If`, with zero statements in the true arm and one
in the false arm. Flow analysis then reports that the later `say x` may read an
unassigned variable: the true arm leaves x untouched. Supplying a justified
false fact for the condition makes that assignment definite.

| Lowering method | Meaning |
|---|---|
| `tree.lower(schema,limit)` | Start a children-first translation; default output allowance is 1,000,000 entries. |
| `pass.next()`, `pass.original()` | Advance and obtain the current input node. |
| `pass.translated(inputNode)` | Obtain the completed output for an earlier input node. |
| `pass.make(kind)` | Create a target draft using the current input's source range and diagnostic anchor. |
| `pass.synthetic(kind,offset)` | Create a generated draft with an empty range at an original-byte anchor. |
| `pass.accept(draft)` | Validate and seal an intermediate target node; return its handle. |
| `pass.replace(draft)` | Validate the result for the current input node. |
| `pass.keep()` | Copy its kind, fields and source origin, substituting translated children. |
| `pass.finish()` | After traversal, publish one validated owned tree, or a failed tree with no root. |
| `pass.error()` | Return a construction or ownership error. |

Check `lowered.status()` before using the result. Unknown kinds or fields,
missing required fields, incorrect child counts and reused children are errors.
An intermediate has one parent in the final tree; use separate copies when two
places need equal-looking nodes. Keep the original tree alive until the pass
finishes. Replacing it invalidates the pass. Drafts belong to one pass and one
visit; accepted drafts are immutable.

The structural checks establish a well-formed target tree. The author of a
lowering establishes its meaning. In this example both forms evaluate the same
condition once and execute the same body exactly when it is false. Regression
tests compare their execution for both truth values and check the resulting
assignment analysis. Node properties and previous analysis tables remain tied
to the original tree; calculate new facts for the lowered tree.

### Origins survive rewriting

An **origin** explains how a node relates to the original source. Its range is
half open: `start()` is the first byte and `finish()` is the byte just after it.
`offset()` is the diagnostic anchor within that range. Offsets count bytes from
zero; `line()` counts lines from one and treats CRLF as one line ending.

| Origin | Meaning |
|---|---|
| `exact` | A grammar mapping produced this node directly from its source range. |
| `composite` | Projection combined source structure, for example retaining parentheses on their surviving expression node. |
| `derived` | A rewrite or lowering created a replacement from an original node. It retains that node's source range. |
| `synthetic` | The application generated a node with no source spelling. Its range is empty at an explicitly chosen original-byte anchor. |
| `recovered` | The node covers skipped input, or is the root of a tree containing recovery damage. Inspect the tree's damage records. |

A folded `Number` may have spelling `14` while `sourcetext()` still returns
`2+3*4`. Changing a field never changes the original bytes. Use
`draft.locate(offset)` to choose a more precise diagnostic anchor within the
node's retained range.

Both rewriting and lowering offer `synthetic(kind,offset)`:

```rexx
generated = pass.synthetic('Number',source.offset())
ok = generated.settext('spelling','42')
ok = pass.replace(generated)
```

Here `source` is the current original node. The result has equal start and
finish offsets, and empty `sourcetext()`. Its field still holds `42`. Use this
for inserted defaults or generated control nodes. Use `make(kind)` when the
replacement represents an existing source expression. Anchors must lie between
zero and the original source length, inclusive; a generated end marker may
therefore sit just after the last byte.

#### Inspect a recovered AST

`llrecover.request.ast()` creates an owned tree of the surviving records.
Release the request when convenient: the tree retains the source and diagnostic
records. Its status is **1 for recovered input**, 0 for clean input, and greater
than 1 for a construction failure. A recovered tree has a usable root for
inspection, and `error()` returns an `LL_AST_RECOVERED` explanation. Require
status 0 before executing a script or applying configuration.

```rexx
request = .llrecover..request(language,source)
tree = request.ast()
if tree.status() > 1 then do
  say tree.error()
  return 1
end
damage = tree.damage()
do i = 1 to damage.size()
  say llsource..diagnostic(tree.source(),damage.offset(i),'INPUT',damage.message(i))
end
```

Each damage record keeps two ranges. `first(i)` to `last(i)` is the complete
skipped record; `offset(i)` to `finish(i)` is the diagnostic location within it.
For a missing value, the diagnostic may have zero length at a separator.
`message(i)` preserves the parser or lexer message. Records are numbered from
one in source order. Unicode and malformed-byte diagnostics use the unchanged
original bytes. `tree.isrecovered()` reports whether any records exist;
`tree.damage()` returns an independent value copy of the small record table.

Copying, rewriting or lowering preserves these records and the recovered
status, even when a transformation replaces the root or discards all surviving
statements. Children outside damaged ranges keep their more precise origins.
The root remains recovered so that damage outside its own surviving range is
still visible. The `tree.entries` allowance includes one entry per damage
record as well as nodes, fields and links.

The common flow and condition-fact services require a clean tree and report
`LL_AST_RECOVERED` for inspection trees. Strict `lllanguage.request.ast()` also
continues to require a successful strict parse. Reparse the corrected source to
obtain a clean tree.

Run [the complete origins example](../examples/origins/show.crexx):

```sh
crexx --program origins langtools/examples/origins/records.rxl \
      langtools/examples/origins/show.crexx
rxvm origins
```

It retains two valid statements around a missing value, prints the original
line-2 diagnostic, generates a Number with a source anchor, and shows that the
rewritten tree still has status 1 and one damage record.

### Binary application-tree projection

The binary projection API returns integer kinds and at most two children per
node. Use `then tree` for named fields, lists and common walks. A **projection**
builds a smaller tree by keeping selected named rules and
removing other wrappers. For example:

```rexx
value: rule = number
keep.value: tree rule value kind 10 arity 0
```

Here `kind 10` is an application-chosen positive integer tag. **Arity** means
the number of children retained after projection. This declaration keeps a
`value` node with no retained children; its source range still covers the
number. Projection declarations accept arities 0, 1 or 2.

For each rule you keep, declare the child count expected after unselected
wrappers have been removed. The requested result must contain one selected
root. A mismatch is an error, which helps detect when a grammar change no
longer matches the application's expected shape.

Run the projection explicitly:

```rexx
projection = request.projection()
status = projection.run()
if status <> 0 then do
  say projection.error()
  return status
end
result = projection.tree()
root = projection.root()
```

The result is a `.llcore..tree` with the declared kinds and original source
ranges. `projection.origin(node)` gives its corresponding raw source-tree node.
Unselected terminals and wrappers are omitted. The original parse tree and its
named view remain available.

### Locations displayed to users

Level L stores byte offsets so that every range identifies the original text
exactly. The source formatter converts an offset to a one-based line and column:

```rexx
say llsource..diagnostic(source,offset,'SETTINGS_PARSE',message)
```

It recognizes CRLF, CR and LF line endings. Columns count Unicode scalar values.
That differs from byte count for non-ASCII text and from the number of visible
screen cells for tabs or combining characters.

The default excerpt escapes tabs, controls and non-ASCII characters and aligns
the caret with the escaped text. A fifth argument, `'unicode'`, displays Unicode
glyphs. Malformed UTF-8 is shown with a hexadecimal excerpt. Prefix the result
with the filename when your application has one.

For repeated coordinate lookups, retain a source index rather than repeatedly
scanning the source prefix. It owns the original bytes and has no global state:

```rexx
positions = .llsource..index(source)
location = positions.position(offset)
if location.error() = '' then do
  say location.line() location.column()
  linebytes = binsubstr(source,location.first() + 1,location.last() - location.first())
end
```

`first()` and `last()` are zero-based byte boundaries for the containing line,
excluding its newline. EOF is a valid position, including an empty final line;
negative and past-EOF offsets report an error. The constructor's optional limits
are line entries (1,000,000), logical bytes (64 MiB, including source bytes and
16 bytes per line) and work (10,000,000). Construction scans bytes once; lookup
searches line starts and counts scalar-leading bytes within the selected line.
The work limit applies separately to construction and each lookup. These bounds
exclude VM container overhead. Check `positions.error()` after construction and
`location.error()` after lookup. Malformed UTF-8 is never decoded by the index;
use `diagnostic` for the existing hexadecimal error display.

## Error messages and recovery

There are two places to handle errors: the specification you build with `rxlang`,
and the documents read by the generated language.

### Errors in a specification

`rxlang` checks syntax, references and whether the selected backend can implement
the rules. A failure names the relevant declaration or input location. Syntax
messages include an excerpt; semantic messages such as an undefined reference
can use a rule name and line/byte location.

The specification reader can collect several syntax errors by resuming at later
`name:` declarations. Fix the first relevant error and rebuild: later messages
may follow from the same damaged declaration. Any error makes generation fail.
Use the build wrapper to preserve the last successfully generated file.

Common diagnoses include:

| Diagnosis | What to check |
|---|---|
| Undefined or duplicate name | Declaration spelling and global name uniqueness. |
| `LL_NULLABLE_TOKEN` | A lexical emit/skip rule can match nothing; make it consume input. |
| `LL_LEX_RECURSION` | A lexical definition refers back to itself; express repetition without recursion. |
| LL prediction conflict | Two alternatives can begin with the same next token, or an empty alternative overlaps what follows. Factor the rules or choose a suitable backend. |
| `LL_LR_CONFLICT` | The LR table has competing decisions. Inspect the named rules, token, precedence and nesting. |
| `LL_RESOURCE_LIMIT` | Generation ran out of its allowance. Check the named setting. |
| Comment/string opening conflict | A comment opener overlaps the start of its `outside` pattern. Give them distinct openings. |

### Message templates

Attach a message to a parser rule:

```rexx
value.error: diagnostic rule value =
  'Expected a number or quoted string; found {found}.'
```

| Field | Substitution |
|---|---|
| `{found}` | Unexpected token text, escaped for display, or `end of input`. |
| `{expected}` | Names of tokens the parser expects. |
| `{rule}` | The rule receiving the message. |
| `{{` and `}}` | Literal braces. |

Unknown fields and unbalanced braces are build errors. Text substituted from
the input is displayed as data; braces within it are not interpreted again.

Rule messages apply to LR, LL, packrat and Earley parser rules. A standalone
Pratt parser reports operator-expression errors; it rejects parser-rule message
attachments.

A template customizes a message only when its rule is an applicable error
context. If several contexts compete, Level L uses the backend's generic
expectation message. This keeps a separator error after a complete value from
being described as a missing value. A message declaration leaves parsing
choices and tree structure unchanged.

### Errors in target input

Strict parsing first prepares and lexes the complete source, then parses its
tokens. It stops when a stage fails. This means a lexical error later in the
file can be reported before an earlier parser error.

An unmatched character normally reports `LL_LEXICAL` at its byte position.
An unclosed string may appear as an unmatched opening quote. An unclosed block
comment reports `LL_COMMENT`. Parser failures report the unexpected input and
expected token names, or an applicable custom message.

For example:

```text
SETTINGS_PARSE at line 1, column 8 (byte 7): unexpected input at byte 7; expected number, string
  first =
         ^
```

Expected tokens describe possible next steps at the failure position. They help
a user repair the input; further errors may be found after that repair.

### Recovery requests

Record recovery is useful for configuration editors and validators that should
report several errors in one document. Declare the start rule and a separator:

```rexx
read.errors: recovery rule settings sync separator
```

Then use the recovery class:

```rexx
request = .llrecover..request(definition,source)
status = request.run()
do i = 1 to request.errors()
  say llsource..diagnostic(source,request.offset(i),'SETTINGS_PARSE',request.message(i))
end
```

The request finds the record containing an error, skips through its next
separator and retries the remaining tokens. It retains the original source and
records the skipped ranges. It does not insert missing values.

The grammar must describe records with a useful synchronization token and have
no declared token stages. Recovery also respects the delimiter boundary of the
supported LR/Pratt region. A malformed UTF-8 or lexical region can be reported
and its containing record skipped. An unclosed block comment stops recovery
because a reliable later boundary is unavailable.

| Recovery status | Meaning | Caller action |
|---|---|---|
| 0 | Clean parse | Use the tree normally. |
| 1 | Recovered document with errors | Report errors; inspect surviving structure. |
| 2 | Resource or error-count allowance exhausted | Report `error()`; collection stopped. |
| 3 | Invalid recovery configuration or request | Check the declaration, token stages and policy. |
| 4 | Recovery cannot continue safely | Report `error()` and the collected diagnostics. |
| 5 | Parser accepted the remaining input without a unique finite tree | Revise the grammar or use a strict Earley request to inspect its forest. |

Only statuses 0 and 1 supply a tree for ordinary inspection. Require 0 before
executing a script or applying configuration. The recovery request does not
expose a forest when it returns 5.

| Recovery method | Result |
|---|---|
| `run()` | Process the document and return its status. |
| `errors()` | Number of collected diagnostics. |
| `message(i)` | Diagnostic text, indexed from 1, in source order. |
| `offset(i)`, `finish(i)` | Original diagnostic byte range; a syntax diagnostic may have zero length. |
| `tree()`, `root()` | Result tree and root after status 0 or 1. |
| `ast()` | Run recovery if needed and build an owned AST from `then tree` mappings. Clean and recovered input retain statuses 0 and 1 respectively. |
| `errornode(i)` | Associated `error.skipped` node when produced. These nodes are stored separately from the successful root's children. |
| `error()` | Overall reason collection stopped. |

The constructor accepts a maximum error count as its third argument and an
optional `.lllimits..policy` as its fourth. With no explicit count, the default
is 16. When both are supplied, the smaller error allowance applies.

## Command line and build integration

### Build a program or reusable library

For an application that owns its grammar, build both in one command:

```sh
crexx --program build/reader reader.crexx settings.rxl
```

This produces `build/reader.rxbin`. The driver generates each language module,
compiles the application against the generated modules and links the Level L
runtime. Input order does not matter. Add `--native` for an executable.
`--program` builds without running; an ordinary invocation builds and runs:

```sh
crexx reader.crexx settings.rxl --args settings.txt
```

Use a library build when several applications should share a generated module:

```sh
crexx --library build/languages/settings settings.rxl
crexx --program build/reader reader.crexx -i build/languages -l build/languages/settings
```

`-i` locates the library's interfaces for compilation; `-l` includes its
implementation in the linked program. The reader imports the namespace in the
grammar, independently of the source filename. Libraries contain the generated
modules; a program build adds their shared Level L runtime once. Several
grammars can share one library or be built as separate libraries.

A grammar-only invocation, `crexx settings.rxl`, also builds a library beside
the source. It does not attempt to execute a parser definition.

The driver uses a private `OUTPUT.crexx-build` directory for generated files and
dependency records. Unchanged builds reuse their outputs. Grammar, generator
and runtime changes invalidate the relevant work. Failed generation or
compilation preserves the last successfully published library or program.
Messages identify the original grammar file and source location.

### Use the generator directly

```text
rxlang specification.crexx [--rxas|--graph] [--limit name=count]
       [--lexer-in PATH] [--lexer-out NEWPATH]
rxlang --limits
```

| Mode | Output |
|---|---|
| `--rxas` | RXAS assembly exporting `NAMESPACE..language()`, with the generated parser data and packed lexer tables. Use this for ordinary builds. |
| No output switch | Level B-compatible cREXX source for the same language definition. Useful for reading the generated code. |
| `--graph` | The frontend's parser-rule graph for inspection. Backend table construction is not run. |
| `--limits` | Available resource settings, defaults and maximum representable values. |

The input may also be `.rxl` or `.levell`.

The command writes generated output and its principal diagnostics to standard
output. Check its exit code before treating output as a generated module:

| Exit code | Meaning |
|---|---|
| 0 | Success. |
| 1 | Invalid specification, unsupported rules or construction failure. |
| 2 | Invalid command usage or an input/output failure. |

`--limit name=count` can be repeated to set several build allowances. These are
settings for that invocation; they do not become rules of the generated language.

For `rxas` or source output, `--lexer-in` can supply an owned lexical capsule
from an earlier successful generation, and `--lexer-out` writes the current
capsule to a new scratch file. Each option appears at most once; neither is
valid with `--graph`. Output must not already exist. Source, input and output
paths must differ; exact `stdin`, `stdout` and `stderr` names are refused.
Conservative spelling/content guards and exclusive output creation protect
inputs; they do not promise general filesystem alias resolution. Capsule I/O
finishes before generated stdout is written. The capsule is private build data,
not a portable public format or an authenticated input; see the
[generation contract](generation-design.md).

### Write an output file safely

Use [generate.cmake](../cmake/generate.cmake):

```sh
task_work="$PWD/build/generated"
mkdir -p "$task_work"
cmake -DLEVEL_L="$task_bin/rxlang" \
  -DSPEC="$PWD/my_language.levell" \
  -DOUTPUT="$task_work/my_language.rxas" -DFORMAT=rxas \
  -P langtools/cmake/generate.cmake
```

The wrapper generates a temporary file and replaces the output only on success.
A failed build preserves the previous module. Direct redirection with
`> my_language.rxas` truncates that file before `rxlang` runs.

Optional `-DREUSE_LEXER=ON` lets this wrapper own an output-derived lexical
cache, checksum manifest and lock in the build directory. Missing/stale/damaged
cache data falls back to cold generation. Parser, source and output validation
still run freshly; a failed generation preserves prior output/cache. The wrapper
serializes that output's transaction and publishes the manifest last. An equal
capsule is not rewritten, and removing the optional cache does not force an
otherwise up-to-date build. Clean the three sidecars with the owning output.
Only the research C grammar opts in by default; bootstrap generation stays cold.

`FORMAT=crexx` selects source output. Compile it with `rxc`, assemble with `rxas`,
and link as for an RXAS-generated module. Level L delegates bytecode production
to the assembler and linker.

### CMake dependencies

A generation command should depend on the specification, the generator, its
runtime inputs and `generate.cmake`. Assembly depends on the generated RXAS;
linking depends on the assembled module and the consumer's libraries.

If an executable embeds bytecode, include those image files as dependencies so
it relinks when they change. A target-order dependency alone only controls when
build steps run. See [RexxScript's build integration](../../rexxscript/modern.cmake)
for a complete example.

For details of building Level L itself, see the [bootstrap guide](bootstrap.md).
Generator internals and development checks are described in the
[tool implementation notes](../README.md).

## Resource limits

Level L supplies default allowances for construction and parsing. A small grammar
needs no resource settings. If an operation reaches an allowance, its message
names the setting and the amount needed so far:

```text
LL_RESOURCE_LIMIT: lr.states budget 1, required at least 2. Increase the host budget or reduce the workload.
```

For example, increase the LR construction allowance with:

```sh
rxlang my_language.levell --rxas --limit lr.states=12000
```

A limit failure says the operation stopped before completing. Increase the
relevant allowance or simplify the workload, then retry. More allowance cannot
resolve a conflicting grammar or add an unsupported feature.

### Configure a request

```rexx
limits = .lllimits..policy()
if limits.set('parse.work',2000000) = 0 then do
  say limits.error()
  return 3
end
request = .lllanguage..request(definition,source,limits)
```

`set()` returns 1 on success and 0 on an unknown name, nonpositive count or value
above its representation ceiling. A failed setting leaves the policy invalid;
create a fresh policy after correcting it. `limit(name)` reports its configured
or default value. `ceiling(name)` reports its maximum representable value.

### Settings reference

A **state** is an internal step in a generated recognition table. A **clause**
is a node in Level L's internal representation of a rule. A **production** is
a parser rule alternative after grouping and repetition have been expanded.
These counts describe generated structures, so they can be larger than the
number of declarations you wrote.

| Setting | Default | Controls |
|---|---|---|
| `source.bytes` | 1,048,576 | Source length accepted by preparation, including specification files. |
| `spec.tokens` | 8,192 | Tokens in the specification, including its reader's EOF marker. |
| `grammar.clauses` | 512 | Internal parser-rule graph. |
| `lexer.clauses` | 65,536 | Internal lexical graph. |
| `grammar.repeat` | 256 | Largest finite repetition count in a specification. |
| `nfa.states` | 65,536 | Intermediate lexical states during construction. |
| `dfa.states` | 4,096 | Deterministic lexer states after construction. |
| `cfg.productions` | 4,096 | Expanded CFG alternatives used in LR construction or Earley requests. |
| `lr.states` | 8,192 | States in the canonical LR parser table. |
| `lr.items` | 16,384 | Rule positions tracked within one LR state. |
| `lr.rhs` | 2,048 | Symbols on the right-hand side of one expanded production. |
| `stage.rules` | 128 | Transforms in one token stage. |
| `scan.work` | 10,000,000 | Counted scanning work, separately for comment preparation and lexical scanning. |
| `parse.work` | 1,000,000 | Counted parser or token-stage operations, including diagnostic work. |
| `earley.cells` | 8,000,000 | Index capacities for Earley's partial parses and forest. |
| `tree.entries` | 1,000,000 | Entries in each tree construction stage. A named view counts nodes and child links; an AST or rewrite counts nodes, field slots and child links, including discarded intermediates. |
| `recovery.errors` | 16 | Collected target or specification errors. |
| `recovery.declarations` | 512 | Declaration intervals inspected during specification recovery. |

Each token stage and recovery retry receives its own work allowance. A regional
parse shares one parser allowance between its inner and outer parsers. A named
view constructed directly with `.lltree..view(raw,root,limit)` has its own entry
limit, defaulting to 1,000,000.

Work and cell counts are operation/storage safeguards, not seconds or bytes of
memory. Larger allowances permit larger costs. `source.bytes` is checked during
preparation, after the caller has read the source into memory; a host accepting
large files should also bound its file-reading step.

### Fixed representation and nesting constraints

Ordinary token IDs are limited to 1–255. Recursive specification and lexical
processing use a depth ceiling of 128; recursive LL, PEG, Pratt and forest
operations use 512. An `LL_IMPLEMENTATION_LIMIT` message identifies one of
these fixed constraints. Increasing a work allowance does not change them.

Use `rxlang --limits` to check maximum count values for the tool you are running.
The policy rejects values beyond encoded table/index capacities before using
them. Packed table loaders validate the generated dimensions independently of
the generating host's default construction allowances.

### Storage in generated modules

DFA and LR tables remain packed in the generated module. Runtime lookups read
that storage directly. LR loading adds an index of its table rows; it does not
expand every action into a separate object. Other parser backends retain the
grammar or operator data needed by their runtime algorithm.

Reuse a language definition and create fresh requests for input documents.
Avoid repeatedly copying complete tree views in visitor calls. These practices
keep application overhead separate from the work of recognition.

## Intermediate representation and semantic services

An AST describes the constructs in a source program: an assignment, an IF, a
call, or a literal. A compiler often needs a second representation that spells
out the order of evaluation and the possible transfers of control. That is an
**intermediate representation**, usually shortened to **IR**.

Level L provides this through `llir` and `llirservices`. They use compact numeric
records internally and give applications named operations, checked handles and
readable dumps. You can use them with any language. Register its types and
operation semantics in ordinary Level B code.

### From a tree to ordered operations

For `say 2+3`, a source tree could contain a Say node whose value is an Add node
with two literal children. Its IR makes each step explicit:

```text
%1 = core.constant 2 : crexx.g.int64
%2 = core.constant 3 : crexx.g.int64
%3 = crexx.g.iadd %1 %2
%4 = crexx.g.itext %3
crexx.g.say %4
core.return
```

The numbers identify immutable results. The addition's result feeds conversion
to text; that result feeds SAY. The compiler registers what addition, conversion
and output mean. Level L checks their input/result types and preserves their
order. SAY carries an output effect, which prevents an optimiser from deleting
it simply because it has no result value.

The [worked example](../../research/level-l/compilers/examples/ir-demo.crexx) constructs this graph, folds the
addition and text conversion, displays both graphs, and prints `5` from the
retained SAY argument. The compiler operation families live in a separate
Level B library. The wrapper keeps its runtime imports external and supplies
Level L when linking the program. From the repository root, using installed tools:

```sh
crexx --library build/ir/semantics research/level-l/compilers/common/llscalar.crexx research/level-l/compilers/typed/lltir.crexx
crexx --program build/ir/demo research/level-l/compilers/examples/ir-demo.crexx -i build/ir -l build/ir/semantics
rxvm build/ir/demo
```

### Blocks, slots and analysis

A block is a sequence of operations ending in a branch, jump, protect or return.
Branches join blocks into a graph; a loop is a cycle in that graph. A temporary
value is local to its block. To carry a value across a branch, store it in a typed
slot and load it in the destination block. A slot represents mutable local
storage in one function invocation.

`llirservices..analyse(reference graph)` determines which blocks can be reached
and whether each slot is assigned on every path to a load. At a join, a slot is
definitely assigned only when all incoming paths assign it. Both Boolean branch
edges are considered. The result explains a possible read before assignment
with its operation ID. This supplements the AST flow mappings: AST mappings
analyse the source-shaped tree, while IR analysis follows the explicitly lowered
operations and graph.

`block.protect(normal,exceptional,origin)` declares the normal continuation and
an exceptional entry for the remainder of one invocation. Both edges stay
visible to analysis, pruning and renumbering. The host owns runtime exception
binding and cleanup; a backend without that support must reject the declaration.
The exceptional entry can rely on setup slots assigned before `protect`, not
later stores in the normal body. See the [core contract](ir-design.md#invocation-protection).

### Contracts make transformations checkable

A schema names types, operands, results, attributes and effects. A type codec
checks exact constant bytes. An application can register a semantic verifier,
a constant evaluator and a lowering service. These are Level B objects supplied
by the host application.

A constant evaluator can return a checked value, keep the original operation,
report invalid input, or report that its work budget ran out. For example, a
checked integer addition folds `2+3`; an overflowing addition is kept for
runtime handling. Unknown effects grant no permission to fold an operation.

A transformation builds a new module and verifies it before returning it.
Failure preserves the original module. Analysis facts belong to one exact
module revision; recompute them after accepting a rewrite. Source origins survive
into new operations, including the distinction between authored, generated and
recovered input.

Use the [IR API reference](ir-design.md) for construction, semantic interfaces,
analysis, checked replacement, target lowering, saved forms and the chunk-based
text emitter. The example constructs IR directly; source-language frontends and
target instruction selection are applications built on these services.

## Compiling C/G source into shared IR

The C/G frontend examples use Level L to scan, parse, build their source trees
and construct verified IR. The compiler host modules are written in Level B.
Classic retains dynamic pool names, exact literal spelling and NUMERIC context;
G retains typed bindings, defaults and short-circuiting. Both use the same
Level L graph and checked extension framework.

For example:

```rexx
/* Classic */
options levelc
total = 2 + 3
if total > 4 then say total
else say 0
```

The G version selects `options levelg` and adds `total = .int` before the
assignment. G lowers the sum to `crexx.g.int64.add` and stores a typed slot.
Classic lowers it to `crexx.c.add` with an explicit borrowed context and stores
the pool key `TOTAL`. Each lowers the IF expression, converts its result to a
core Boolean and creates yes/no/join blocks. SAY remains an observable operation.

A Level B consumer uses the reusable frontend service:

```rexx
request = .lltfrontend..request(source, artifacts, 'demo.crexx')
if request.status() <> 0 then say request.diagnostic()
else do
  graph = request.graph()
  say graph.dump()
end
```

Here `source` is `.binary`; `artifacts` names the directory containing the
required runtime metadata images. Import `lltfrontend` and `llir` alongside
`rxfnsb` in the host. Querying the request runs it once and retains an owned
result. A failed request returns no successful graph. Error messages name the
file and explain the category, line, Unicode column and exact byte offset, with
a source excerpt and caret. Missing THEN/END, malformed expressions/calls,
unterminated strings/comments and invalid encoding have regression coverage.
These grammars stop at the first syntax error and accept no recovered input.

For a built inspection consumer, from the repository root:

```sh
cmake-build-levell-integration/bin/rxbvm \
  cmake-build-levell-integration/research/level-l/compilers/frontends/opt/frontend-probe-linked.rxbin \
  -a g research/level-l/compilers/examples/step01/pipeline-g.crexx \
  cmake-build-levell-integration/bin
```

Change `g` and the example filename to inspect Classic. See the
[frontend API and admitted language profile](../../research/level-l/compilers/doc/frontends.md) and
[paired walkthrough](../../research/level-l/compilers/doc/pipeline-walkthrough.md) for the complete
source, actual generated trees/IR, calls, loops and numeric differences.

The frontend service ends at checked IR. The separate `llccompiler` or `llgcompiler` service
continues through register assignment and RXAS selection. Its implementation is
Level B code **using Level L's `llir.writer` emitter library**.

## Compiling C/G source to RXAS

The installed commands are `llrxc-g` and `llrxc-c`. They implement the
[admitted profile](../../research/level-l/compilers/doc/poc-scope.md); ordinary `rxc` and `rxc-c`
retain their existing product roles.

Research compilers are opt-in; normal `stage-product` provides the generic SDK
and Level L tools. From the repository root:

```sh
cmake -S . -B cmake-build-debug -DCREXX_BUILD_RESEARCH_COMPILERS=ON
cmake --build cmake-build-debug --target stage-product stage-research-compilers --parallel 8
cmake --install cmake-build-debug --prefix '/tmp/crexx review'
cmake --install cmake-build-debug --prefix '/tmp/crexx review' --component ResearchCompilers
```

Select only C or G with `-DCREXX_RESEARCH_LANGUAGES=C` or `G`.
Build-tree commands and service packages live under
`cmake-build-debug/research/level-l/compilers/products/`.
Use the chosen prefix's `bin` directory for the following commands and imports.
Installation copies built products; it does not build missing ones.

```sh
llrxc-g -i /path/to/bin -o demo.rxas demo-g.crexx
rxas -n demo.rxas
rxlink -o demo-linked demo.rxbin /path/to/bin/library.rxbin
rxbvm demo-linked.rxbin
```

For Classic, use `llrxc-c` and add `/path/to/bin/llctarget.rxbin`,
`/path/to/bin/rxfnsc.rxbin` and `/path/to/bin/classlib.rxbin` to the link command.
The collection library supplies the Classic variable pool; include it explicitly
rather than depending on VM autoload from the compiler installation. The
`llcompiler_c_only_*` regressions compile with native/typed compiler products
absent and execute linked static images in a separate directory with autoload
disabled. This verifies the bounded current profile, not complete C. Use VM `-a` followed by program
arguments when needed. Keep linker debug metadata for runtime source locations;
`rxlink -s` deliberately removes it. Assembler optimisation may also be enabled
by omitting its `-n`. The C/G compilers enable shared IR optimisation by default;
use `llrxc-g -n` or `llrxc-c -n` for the cleaned unoptimised control. The native
B seed's `rxc -n`, the new compiler's `-n` and `rxas -n` control three separate
stages. Use assembler `-n` when inspecting the new compiler's own effects.

Classic statement CALL can use explicit prebuilt providers. The `-i` directory
must contain each selected `<artifact-stem>.rxbin` and the runtime images.
For a C source provider, produce one ordinary void/frame export with `--routine`:

```sh
llrxc-c -i /path/to/artifacts --routine sample.run -o /path/to/artifacts/provider.rxas provider.crexx
rxas /path/to/artifacts/provider.rxas
llrxc-c -i /path/to/artifacts --provider provider sample.run -o main.rxas main.crexx
```

Repeat `--provider <artifact-stem> <fully-qualified-callable>` for more bindings;
link the selected provider images into the application. Local labels and exact
BIF names retain priority. Qualified A.run/B.run may coexist; an ambiguous short
RUN refuses compilation. B/G providers are first built by their owning compiler.
The C request reads checked binary metadata; it never compiles a provider source,
searches at runtime or supplies external function expressions. Missing inventory
fails43.1 when reached; a selected provider omitted from the linked image retains
FUNCTION_NOT_FOUND. Routine output has no command main/status bridge and leading
PROCEDURE remains17.1. See the
[static-provider contract](../../research/level-l/compilers/doc/architecture.md#call3-static-provider-contract).

Standalone Classic `PULL` uses the same templates as `PARSE UPPER PULL`. It
acquires selected queue or fallback input once, including bare/empty-first forms;
later comma templates receive empty text. Shared UPPER and live variable-pool
writes remain in the existing executor. Configured NOTREADY, broader host
behavior, conditions/TRACE and declared-platform closure remain open under the
[PULL contract](../../research/level-l/compilers/doc/architecture.md#pull-contract).

Classic `PUSH expression` and `QUEUE expression` convert an operand once and
insert it at the front or tail of the selected queue; bare forms insert empty
text. Text retains its case, Unicode and NUL bytes. `PARSE PULL` observes insertion
without UPPER; standalone `PULL` applies UPPER. Selected queue state is shared by
nested/private program roots, and `QUEUED()` does not consume entries. Wider
host/condition/TRACE/platform closure remains open under the
[queue contract](../../research/level-l/compilers/doc/architecture.md#queue-contract).

Classic `ADDRESS` selects/swaps lasting environments or executes one transient
command with ordered `WITH` resources and lexical VAR/counted-STEM anchors.
An expression-only clause executes through the current environment and lasting
connections, captured after its expression converts to command text. Both routes
share the B command adapter. Operand EXIT skips construction and dispatch.
Configured conditions/resumption, TRACE, native host/NUL transport and declared
platforms retain their [separate gates](../../research/level-l/compilers/doc/architecture.md#address-c-command-contract).

`llrxc-c` prints `RXC-LC-IMPLICIT_ADDRESS` warnings once to stderr for implicit
commands whose classified head is not a literal string. Grouped strings and
left-string concatenations suppress the warning; assignments and explicit
instructions do not warn. Warnings do not change successful status or output
publication. C frontend/service `warnings()` returns owned immutable records
with severity/code/message, filename, exact byte bounds, scalar line/column and
formatted text. An empty array denotes no collected warnings; completed records
remain available after later lowering/backend failure and after request expiry.
The getter prepares frontend work only, without emitting RXAS.

A Level B host imports `llgcompiler` and `llir`, links `llgcompiler.rxbin`,
`rxfnsl.rxbin` and `library.rxbin`, and uses the following G service. For C,
import/link `llccompiler` and call its own request with the same arguments:

```rexx
request = .llgcompiler..request(source, artifacts, 'demo.crexx')
if request.status() <> 0 then say request.diagnostic()
else do
  output = request.output()
  say output.text()
end
```

`source` is binary; `artifacts` names the target runtime-image directory.
The request does not print, write files, exit or invoke another compiler.
`output` is an owned `.llir..output`, with `text`, `bytes`, `error` and streaming
support, and remains usable after the request. `graph()` retains the frontend
IR; after successful status, `emissiongraph()` shows the graph sent to the
backend. It is the same graph when optimisation is disabled.
The optional budgets after the filename are source/semantic work, nesting,
emitted bytes and registers per function, followed by `optimise=1`. Append zero
to select the unoptimised path without changing earlier argument positions:

```rexx
request = .llgcompiler..request(source, artifacts, 'demo.crexx', 1000000, 512, 67108864, 65536, 0)
```

C requests additionally accept optional `providers:llcbindings.binding[]` and
`routine_export:string` after these eight arguments. Import `llcbindings` to
construct each explicit `(artifact, callable)` binding. The request owns its
prepared selections; `graph()` lazily prepares the frontend too. The empty
default performs no provider acquisition. The same byte/work ceilings bound
aggregate provider acquisition and metadata selection.

The work/byte budgets also protect the shared optimisation phase. Defaults and the complete API are in
the [backend reference](../../research/level-l/compilers/doc/backend.md).

The command writes a completed result through a claimed sibling staging
directory and replaces the destination only after a successful write/close.
An existing staging directory or failed compilation/write produces a diagnostic
and preserves prior output. The [paired worked examples](../../research/level-l/compilers/examples/step04/README.md)
show actual source, IR and RXAS, including Classic `say 01` printing `01` and G
printing `1`. Classic keeps its numeric/argument/pool semantics; G keeps typed
values and short-circuiting. A declared G decimal starts absent and displays
`nan`, while integer/float/Boolean defaults are zero and text is empty.

The current output uses stable registers and runtime adapters. Shared Level L
passes propagate proved private values, evaluate admitted constants, simplify
Boolean branches and remove safe unused operations. G admits checked int64
arithmetic/comparisons and canonical conversions. C retains the effectful runtime
truth check; exact literal `0`/`1` separately supplies a derived Boolean branch
fact so dead-arm pruning preserves context checks and errors. Classic arithmetic,
mutable numeric context and variable
pools keep runtime semantics. Both languages preserve effects and errors. See
the [actual before/after example](../../research/level-l/compilers/examples/step05/README.md) and
[generic Level L optimisation API](ir-design.md#optimise-a-frozen-module).

Explicit operands do not construct spare defaults;
bare SAY and declared-variable defaults remain. Frame planning uses selected
instructions and hidden helper calls to reserve scratch and call space only
when needed. Straight jumps to the next emitted block are omitted. Source records carry expression ranges and
exact/generated provenance. Identical straight-path records are suppressed;
different expressions and control-flow entries can still need records on the
same line. These are metadata, not printed TRACE events. Clause ID zero means
that authored clause grouping is not yet available.

The CLI records the source basename. The service preserves a logical relative
filename such as `examples/demo.crexx`, while absolute physical paths become
basenames. This keeps checkout paths out of source metadata. The VM may still
show the assembler's binary-module identity separately.

For example, compile this once with `options levelc` and once with
`options levelg`:

```rexx
options levelc
say 1
say 1 / 0
```

Both print `1`, then fail on line 3. G reports `DIVISION_BY_ZERO` at the expression.
Classic reports `RXC-LC-42.3` with the authored expression's context and retains
the original helper fault location. Classic still maintains a diagnostic
location field because deeper frames unwind before its handler runs; unchanged
updates are suppressed. Entry/generated context is labelled explicitly rather
than presented as an exact authored line.

Keep linker metadata for the original fault source. A stripped image falls back
to module/address (Classic's retained context can still appear in its message).
The supported relay covers existing fatal string-message conditions, not
arbitrary condition payloads or complete backtraces. Clause grouping, full TRACE,
and the other remaining quality work are recorded in the
[current plan](../../research/level-l/PLAN.md).

### Following a complete C/G compilation

The [paired walkthrough](../../research/level-l/compilers/examples/step06/README.md) includes real
source trees, frontend IR, optimised emission IR and RXAS. It also gives build,
installed execution and view-regeneration commands. Use the same body with
`options levelc` or `options levelg`:

```rexx
say 01
if 1 then say 'ok'
else say 1 / 0
```

C prints `01` then `ok`; G prints `1` then `ok`. Both source trees preserve the
literal spelling `01`. C's semantic mapping keeps those bytes; G's maps them to
the integer value 1. Both frontend IRs retain the division arm. Shared Level L
optimisation proves the condition true and removes the unreachable arm from the
emission graph. Both default and `-n` compilation execute correctly.

Change `if 1` to `if 0`: division is reached and remains a runtime error, with
line 4 source context. It is not a compile-time error just because its operands
are constants. Remove `then` instead: compilation fails with
`LL_COMPILER_SYNTAX`, and an existing output file is preserved. In this exact
example the parser reports the unexpected ELSE on line 4, where THEN is still
required; the diagnostic location is the detection point.

The [profile map](../../research/level-l/compilers/doc/poc-coverage.md) gives the exact supported
language, ownership/error tests and remaining boundaries. Compilation is Level
B code using Level L libraries all the way through emission. Existing RXAS,
linker and VM tools still process the result. Full C/G syntax, proper B
self-hosting, arbitrary imports, full TRACE/condition handlers and INTERPRET
are outside this admitted compiler slice.

### Classic SAY and ordinal literals

`llrxc-c` supports bare SAY and SAY with an expression. The expression is
evaluated once before its line is written; a failed expression writes no line
for that SAY. For example:

```rexx
options levelc
say
say 'hello'
say '41 42'x
say '01000011'b
```

This writes an empty line, `hello`, `AB`, then `C`. An adjacent `x` or `b`
suffix is case-insensitive. Hex groups contain pairs of digits and binary groups
contain groups of four bits; only the first group may be shorter. Empty
literals are valid. Leading/trailing blanks, invalid digits or broken groups
produce a located syntax diagnostic and leave an existing output file intact.
Every decoded byte maps to the Unicode character with the same ordinal:
`'80FF'x` is U+0080 followed by U+00FF; `'00'x` preserves NUL. `'C3A9'x` contains
two characters and differs from ordinary `'é'`. No runtime decoder is emitted.

`'41' x` remains a string followed by a blank and variable X; `'41'xyz` uses
abuttal with variable XYZ. This adds no G literal syntax or general Classic BIF
support. Full expression, host and TRACE coverage remains described by the
[instruction review](../../research/level-l/compilers/doc/LEVELC-INSTRUCTION-REVIEW.md).

### Classic variable symbols and DROP

The C profile preserves raw variable spellings for reads, assignments, ARG
destinations and DROP. Names start with a Unicode letter or `! ? _ @ # $`;
later characters may also be ASCII digits or dots. Stems and arbitrary compound
tails use the existing shared pool rules. Leading-digit and ordinary leading-dot
constant symbols cannot be DROP targets; `.MN`, `.RESULT`, `.RC`, `.RS` and
`.SIGL` are the admitted leading-dot variable exceptions. Combining marks and emoji are not variable-name letters,
and names are not implicitly normalized. Micro-sign `µ` remains distinct from
Greek `μ`/`Μ`. ARG still uppercases argument contents independently of the raw
destination spelling. Routine/call names retain the ASCII-undotted profile and
wider shapes receive an explicit diagnostic.

```rexx
options levelc
µ = 'micro'
victim = 'held'
names = 'names victim'
drop (names) µ
say names victim µ
```

The output is `NAMES VICTIM µ`. A DROP list contains direct variable symbols
or parenthesized single variable references. Items execute in source order;
each indirect item captures its current value once, then the shared pool drops
that captured subsidiary list in order. Dropping its carrier does not discard
the remaining captured words. Invalid subsidiary words are skipped and later
valid words continue. Lists use the borrowed configuration's Unicode and extra
blanks. Direct source targets must be valid variables, even in unreachable
source; `(a+1)` is an expression and is not an indirect DROP reference.

DROP reuses scalar, stem, compound-tail and exposed-alias pool behavior. A
dropped binding differs from an assigned empty value and a dropped stem removes
its default and tails. The compiler emits one contextual operation and existing
pool call per item; it does not split indirect words or substitute tails itself.
Source EXPOSE, resumable conditions and full TRACE remain later compiler work.
The [independent DROP fixtures](../../research/level-l/compilers/tests/drop/)
own source, state, diagnostic, borrowed-alias and emitted-code expectations.

### Classic assignment

An assignment evaluates its expression once before the current variable pool
resolves the target. A missing expression assigns empty text. The pool owns
scalar replacement, compound-tail substitution and stem defaults; the compiler
passes the raw target name and exact value to that shared implementation.

```rexx
options levelc
key = 'b'
items.key = 'old'
items.key = items.key || '+'
empty =
say items.key || '|<' || empty || '>'
```

The output is `old+|<>`. Substituted tail contents keep their case and may
contain Unicode, dots or NUL; these contents are values, not newly parsed source
identifiers. A whole-stem assignment replaces its default and clears explicit
or dropped tails. Assigning an individual tail restores a previously dropped
binding. Assignment preserves the value's text and numeric representation.

Private source routines have fresh pools. The [independent assignment
fixtures](../../research/level-l/compilers/tests/assignment/) combine source-call
ordering with a separate B-owned, prebound-context control;
they do not establish source EXPOSE or shared-label behavior. Those forms,
resumable conditions and full TRACE remain later compiler work.

### Classic NOP

NOP has no operand and leaves surrounding execution order unchanged. It can
occupy the admitted main, IF/ELSE, group, WHILE and private-routine statement
positions. The same word can still name a variable in assignment or expression
context:

```rexx
options levelc
nop = 'variable'
NoP
say nop
```

The output is `variable`. An extra token after the statement, as in `nop x`,
is rejected even in unreachable source. An unobserved NOP adds no value,
runtime helper or frame demand; its authored AST span remains available.
SELECT/shared-label placements, exact Classic diagnostic identities and full
clause/TRACE observations remain later compiler work.

### Classic OPTIONS

Every OPTIONS clause retains its ordinary optional expression. The operand is
evaluated once where the clause is reached, including Name reads, arithmetic,
calls and errors; bare OPTIONS has no operand. The current shared Classic
processor recognizes no runtime words, so unknown/DBCS words and embedded NUL
have no processor effect. The compiler omits the empty processor call and unused
simple String/Number construction, while retaining operand effects and validation.

Only a complete first nonempty clause made of OPTIONS and bare words selects
file settings. `LEVELC` and `NUMERIC_CLASSIC` are accepted. Nested block comments
remain enabled; `COMMENTS_HASH`/`COMMENTS_NOHASH` and
`COMMENTS_DASH`/`COMMENTS_NODASH` select request-local line comments.
`COMMENTS_NOSLASH` is accepted; slash comments, incompatible levels/numeric
policy, conflicting switches and unsupported `SRCMAP` are rejected. `STRICTC`
selects reached-only invalid LEAVE/ITERATE-target errors for this request; default C
rejects source-provable invalid transfer targets even in dead source. Identical switches
may repeat; quoted/later/call-shaped STRICTC stays an ordinary operand. Header
words still have their ordinary expression evaluation.

```rexx
options levelc comments_hash
# This line is a comment in this request.
options 'unknown' || '00'x
options probe()
exit 0
probe: procedure
  say 'once'
  return 'ignored'
```

The output is `once`. Quoted, operator-shaped and call-shaped first clauses,
and all later OPTIONS clauses, do not select file settings. For example,
`options levelg()` is an ordinary private function call when that routine is
defined. Settings never leak into a later compilation request. Adding runtime
words or effects to the shared processor requires compiler contract review and
recompilation; complete header controls, source maps, conditions and TRACE
remain later work under the [OPTIONS plan](../../research/level-l/compilers/PLAN.md#current-action-options).

### Classic IF

IF evaluates its condition once and executes only the selected instruction.
ELSE belongs to the nearest open IF; a false condition without ELSE continues
with the following statement. The condition must produce exact text `0` or `1`.
For example, `'1.0'` raises Classic 34.1 before either arm executes.

```rexx
options levelc
if '1' then say 'selected' else say 1/0
if '0' then say 'not reached'
```

The output is `selected`; the unchosen division is never evaluated. Current
main/group/WHILE/private-routine placements retain each arm instruction's own
contract. Both terminating arms add no spare join/default-return path, and
literal branch pruning retains the contextual truth check. Full DO,
shared labels/EXPOSE, complete diagnostic identities, resumed conditions and
TRACE remain later work in the [IF plan](../../research/level-l/compilers/PLAN.md#current-action-if).

### Classic SELECT

SELECT evaluates WHEN conditions in source order until the first exact text
`1`, then executes only that instruction. Exact text `0` continues to the next
WHEN. Later expressions and arms remain dormant after a match. OTHERWISE
accepts an instruction list, including an empty list, and is optional and final.

```rexx
options levelc
select
  when '0' then say 1/0
  when '1' then say 'selected'
  otherwise
    say 'fallback'
    nop
end
```

The output is `selected`. An invalid logical WHEN value, such as `'1.0'`, raises
Classic 34.2 at WHEN before either its body or later code. With no true WHEN
and no OTHERWISE, Classic 7.3 identifies the SELECT line. Nested SELECT and
admitted main/group/WHILE/private-routine arms preserve ordinary loop transfers;
SELECT itself creates no loop frame. Current generated C uses the fatal,
nonresumable condition profile. Shared labels/fall-through/EXPOSE, complete
expressions/DO, full diagnostic recovery and conditions/TRACE remain open in
the [SELECT plan](../../research/level-l/compilers/PLAN.md#current-action-select).

## Optional arguments and separate G modules

STEP07A adds scalar optional arguments to `llrxc-g`, using existing G syntax:

```rexx
options levelg
say scale(7)
say scale(7,3)
exit 0
scale: procedure = .int
  arg value = .int, factor = 2
  return value * factor
```

The output is `14`, then `21`. A literal default determines the scalar type.
`?name = .int` declares an optional integer with its primitive zero default;
string defaults are empty, Boolean/float defaults are zero and a primitive
optional decimal retains the existing absent decimal value. `?name` in an
expression tests original presence, even after assignment. A supplied empty
string is present and undergoes the ordinary parameter conversion; it is not
an omitted argument. Positional holes such as `f(,3)` are allowed only for
optional formals. A required formal after an optional formal still needs its
actual value.

Classic retains its explicit equivalent:

```rexx
options levelc
say scale(7)
say scale(7,3)
exit 0
scale: procedure
  arg value, factor
  if \arg(2,'E') then factor = 2
  return value * factor
```

The C call baseline can be checked independently of G using the existing
examples and expected results:

```sh
cmake --build cmake-build-debug --target llcompiler_c_calls_measure
```

This runs the B/L C compiler image in a restricted host and its linked output
in a separate compiler-free runtime, with bytecode autoload disabled. It checks
argument holes versus empty strings, recursion, evaluation order, the current
LENGTH/ARG/DIGITS bindings and rejection of incompatible Classic metadata.
It does not establish complete ARG/CALL, shared-label activation, the remaining
BIF bindings or full conditions/TRACE. The [C plan](../../research/level-l/compilers/PLAN.md#active-c-contract-baseline-increment)
records the precise qualification boundary; INTERPRET is the final stage.

The shared B runtime now exposes a borrowed execution bundle for direct hosts:

```rexx
import rexxactivation
import rexxpool
import rexxclassicconfig
import rexxclassiccontext

activation = .rexxactivation..RexxActivationArguments()
pool = .rexxpool..RexxVariablePool(1)
config = .rexxclassicconfig..RexxClassicConfig("UTF8")
context = .rexxclassiccontext..RexxClassicContext(reference activation, reference pool, reference config)
```

Keep those three initialized owners in stable storage for the bundle's lifetime.
Its `activation()`, `pool()` and `config()` methods return checked references;
`bindPool(reference replacement)` changes only the receiving bundle's pool
binding. Copying the bundle shares the owners but not later binding changes.
Helpers that must observe rebinding take `reference context`. Invalid or expired
references raise `REFERENCE_INVALID`; a rejected rebind preserves the old pool.

Direct hosts import `llctarget` from `llctarget.rxbin` and use the
[borrowed-context adapters](../../research/level-l/compilers/doc/architecture.md#checked-bif-reuse-and-target-compatibility-decision).
They reuse existing BIFs and the host's character configuration. Numeric calls
inherit the executing caller frame; `setting` updates stored activation state
only, so a host installs validated settings in its own frame. Generated C does
this at procedure entry and after each successful NUMERIC statement, preserving
child inheritance and parent restoration. The direct fixture
`research/level-l/compilers/tests/target-context.crexx` demonstrates the same
boundary independently. Generated C uses one required context reference and one
fresh result packet; caller-owned child activations/pools share the root UTF8
configuration. The [current C-family contract](../../research/level-l/compilers/doc/architecture.md#current-c-family-contract)
owns the research schema and target surface: rebuild earlier C images/IR;
no legacy bridge is provided.
NUMERIC FORM accepts fixed SCIENTIFIC/ENGINEERING, VALUE expression and bare
scientific reset. VALUE may be omitted before a parenthesized or unary
expression. DIGITS/FUZZ retain defaults 9/0. All three evaluate an operand once,
validate through the shared setter and install only after success; configured
conditions/LOSTDIGITS, TRACE and wider platform acceptance remain separate.
Static SIGNAL accepts direct, decoded quoted and VALUE targets, with an implied
VALUE before a parenthesized or unary expression. The selected Level C profile
folds all target forms through shared Unicode casing and compares exact text;
numeric-looking names, trailing blanks and embedded NUL remain distinct.
Successful transfer updates SIGL/.SIGL and leaves active loop control while
retaining the current invocation, arguments, pool and numeric settings. It
cannot rearm PROCEDURE eligibility. The [static SIGNAL contract](../../research/level-l/compilers/doc/architecture.md#signal-static-contract)
keeps ON/OFF, typed handlers, clause/TRACE and wider host/platform work separate.
SELECT adds contextual WHEN truth and a shared no-match error bridge; the
borrowed-context procedure ABI is unchanged. The
[DO contract](../../research/level-l/compilers/doc/architecture.md#approved-do-contract)
adds counted/controlled/FOREVER/WHILE/UNTIL forms, ordered once-only header
captures and hidden frame-owned state delegated to shared `RexxDoState`.
Grouping and state-free repetition allocate no DO state. END names compare
authored symbols using shared Classic casing. LEAVE accepts an optional raw
control name, resolves the innermost matching controlled loop within this
routine and closes crossed states without end checks/steps or pool-name search.
ITERATE accepts the same optional raw names, closes only crossed inner states
and resumes the selected loop's UNTIL/advance/entry path, retaining its owner.
Shared labels/EXPOSE and complete conditions/TRACE remain later work. The admitted DO
forms have [focused local qualification](../../research/level-l/compilers/PLAN.md#current-do-proof);
[named LEAVE proof](../../research/level-l/compilers/PLAN.md#current-leave-proof)
covers static targets and request-local invalid-target policy.
[Named ITERATE proof](../../research/level-l/compilers/PLAN.md#current-iterate-proof)
covers selected-loop end work and default/STRICTC 28.2/28.4 errors.
INTERPRET remains unimplemented,
and the current fatal-condition profile remains nonresumable.

Ordinary C text follows the agreed Unicode codepoint model, preserving codes
0–255 through the fixed Latin-1 ordinal bridge. Modern SAY and BIF behavior is
the delivery target. Further historical SAY processor/callback/byte-exact edge
parity is deferred and unclaimed; existing useful controls remain.

For separate G compilation, put `namespace scalelib expose scale` after the
provider's OPTIONS line and keep only routine declarations. In the consumer,
write `import scalelib` and call `scalelib..scale(7)`. Compile and assemble the
provider first, supply its directory with `-i` when compiling the consumer,
then link both RXBIN files with `library.rxbin`. See the
[complete commands and actual IR/RXAS](../../research/level-l/compilers/examples/step07a/README.md).
The provider owns the default; changing it only requires rebuilding the
provider and relinking, not recompiling its callers.

A Level B IR consumer can use the same generic facilities without C/G modules:

```rexx
fn = builder.function('choose',integer)
call fn.exportas('demo.choose')
ordinal = fn.argument(integer,1)
entry = fn.block('entry')
present = entry.hasarg(ordinal,origin)
fallback = entry.constant(integer,'2',origin)
value = entry.readarg(ordinal,origin,fallback)
call entry.returnvalue(value,origin)
```

A required formal uses `argument(integer)` and the two-argument `readarg`.
`module.function(id)` exposes checked signature/export views. Optional fallback
values are already evaluated in generic IR; the C/G slice admits only pure
literal or primitive defaults. Saved IR writers use version 2, with checked
version-1 reader compatibility. See the [IR reference](ir-design.md).

Malformed imports, non-exported/ambiguous callables, missing required actuals,
unsupported parameter types and invalid default forms produce located errors
and preserve an existing output file. Arbitrary default expressions, references,
classes, arrays, module initialisers, source imports and general Classic external
loading remain outside this increment. Compiler code remains Level B and uses
Level L's grammar, IR, transformation and writer libraries throughout.

## RexxScript: a larger example

RexxScript demonstrates how the same machinery supports a script interpreter.
Its [grammar](../../rexxscript/grammar/rexxscript.levell) describes statements
and expressions. Its interpreter reads the resulting tree and executes it
with RexxScript values and function services.

### Its processing stages

| Order | Component | Purpose |
|---|---|---|
| 1 | Level L source preparation | Validate encoding and handle declared comments, preserving the original source. |
| 2 | Generated lexer | Recognize raw tokens, initially treating keyword spellings as identifiers. |
| 3 | RexxScript layout pass | Handle physical newlines, continuation and parenthesis context. |
| 4 | Generated keyword stage | Classify words according to statement context. |
| 5 | RexxScript concatenation pass | Insert blank/abuttal operators where the language requires implicit concatenation. |
| 6 | Generated LR parser | Build the statement and expression tree. |
| 7 | Level L application AST | Construct named fields from the grammar's `then tree` mappings. |
| 8 | RexxScript validation | Check control placement and source nesting before executing anything. |
| 9 | Analysis and checked folding | Calculate constants and replace eligible expressions with exact typed literals. |
| 10 | RexxScript execution | Execute chosen branches with fresh variable and function state. |

**Abuttal** means adjacent terms without an explicit operator between them.
RexxScript distinguishes it from blank concatenation and function-call
adjacency. Its application passes supply explicit tokens for the parser.

The layout and concatenation passes are in
[RexxScriptProgram.crexx](../../rexxscript/RexxScriptProgram.crexx).
Keyword classification comes from the grammar's token-stage declarations.
Execution is in [RexxScriptExecution.crexx](../../rexxscript/RexxScriptExecution.crexx).

RexxScript calls `lex(0)`, performs layout work, calls `classify_tokens(...)`,
performs concatenation work and calls `parse_tokens(...)`. These are actual
stages. The expression and statement rules within the LR grammar all take part
in one parse.

Statement sequences use the same list notation as the settings example:

```rexx
statements: rule = separator* (statement as item (separator (statement as item)?)*)?
  then tree Sequence(items = item*)
```

The `Sequence` node contains an ordered `items` list of statements. Execution
visits each item, stopping when control flow leaves the sequence.

### How IF statements get their structure

The grammar determines which ELSE belongs to which IF. The interpreter then
uses that structure to choose a branch. The relevant rules are:

```rexx
statement: rule = closed.statement | open.statement
closed.statement: rule = simple | block | if.closed
open.statement: rule = if.open | if.continued
if.closed: rule = ifword expression thenword closed.statement elseword closed.statement
if.open: rule = ifword expression thenword statement
if.continued: rule = ifword expression thenword closed.statement elseword open.statement
```

An **open statement** ends with an IF that can still take an ELSE. A **closed
statement** has no such exposed IF. A simple statement and a `DO ... END` block
are closed. An IF whose branches are both closed is also closed.

Consider:

```rexx
if a then if b then say "yes" else say "no"
```

Its main structure is:

```text
if.open                         outer IF has no ELSE
├── condition: a
└── then: if.closed              inner IF has both branches
    ├── condition: b
    ├── then: say "yes"
    └── else: say "no"
```

To attach the ELSE to the outer IF, its THEN branch would need to be closed.
But `if b then say "yes"` is open. Attaching ELSE to that inner IF closes it
and satisfies the grammar. This expresses the usual “nearest unmatched IF”
rule directly.

`if.continued` handles a statement such as:

```rexx
if a then say "A" else if b then say "B"
```

The outer IF has an ELSE, but its final branch is still an open IF. Appending
`else say "C"` closes the inner one. A `DO ... END` block creates a boundary:
an IF inside it cannot take an ELSE outside the block.

These are categories of statements within one grammar. Recursive references
carry the open/closed property through nesting; they are not separate passes.
This technique is useful whenever different syntactic categories need different
rules about what can follow them.

### Execution follows the chosen structure

The grammar maps `if.closed` and `if.continued` to `IfElse(condition, yes, no)`.
It maps `if.open` to `IfThen(condition, yes)`. The open/closed distinction still
resolves ELSE during parsing; the interpreter uses the useful branch fields:

```rexx
value = expression(node.child('condition'))
if rexxscriptoperators..truth(value) then call statement(node.child('yes'))
else if node.kind() = 'IfElse' then call statement(node.child('no'))
```

A general walk visits every child. Execution instead chooses branches and
short-circuits logical AND/OR; it must not execute every node it encounters.
Loop and return behavior remain part of the interpreter.

### Analyse and fold a script

The [analysis example](../../rexxscript/examples/ast-analysis.crexx) follows
this script through the full process:

```rexx
x=2+3*4; if x>10 then say x else say 1/0
```

`rexxscriptanalysis..analyse(reference tree)` returns facts for each AST node:
`known()` says whether its constant value was calculated, `value()` returns that
RexxValue, `reads()` lists variable names used directly in expressions, and
`effects()` marks possible state changes or function calls. Reads and effects
include both branches. A function may access additional state, so a call keeps
its effect flag and stays outside constant folding. The analysis never calls
a BIF or consults the variable pool.

`rexxscriptanalysis..fold(reference tree)` creates a new checked AST. It changes
`2+3*4` into a Number with spelling `14`, while retaining the original source
range. The expression `x>10` depends on a variable and remains an expression.
The failing calculation `1/0` also remains an expression; the script can run
successfully because that branch is not selected.

Analysis and execution share RexxScript's numeric operations. A folded literal
must decode to the same type and exact value. Floating-point spelling is checked
against the original bits, including signed zero. A value that cannot round-trip
stays as an expression. Failed arithmetic is left for execution so that folding
does not introduce errors in an unselected branch.

A replacement keeps the full original range for `sourcetext()`. Its `offset()`
can retain a more precise diagnostic anchor within that range. This preserves
the location of a later error even when its operand was folded. Applications
can set that anchor with `replacement.locate(offset)` before accepting a draft.

Build and run the example using installed tools:

```sh
crexx --program ast-analysis rexxscript/examples/ast-analysis.crexx
rxvm ast-analysis.rxbin
```

The output reports constant value `14`, variable read `X`, the Number replacement
and original spelling `2+3*4`, followed by the script's output `14`. Execution
folds by default. Pass `0` as the fourth argument of
`rexxscript1run..execution(program, fuel, maximum, fold)` to compare ordinary
execution with folding. Both paths apply the same validation and runtime rules.

See the [RexxScript user guide](../../rexxscript/doc/user-guide.md#rexxscript-1-modern-level-l-review-implementation)
for the script language and its value rules.

## Troubleshooting

| Symptom | What to check |
|---|---|
| A token is never emitted | A longer rule may win, or an earlier rule may win an equal-length match. Inspect raw tokens with `lex(0)`. |
| Punctuation is rejected in a parser rule | Refer to its token name, such as `equals`, rather than a quoted literal. |
| A keyword cannot be used as a variable | Check lexical priority and the token stage's state and guards. |
| A sequence of assignments fails | Check that the lexer emits a separator between them. |
| Statements join across a comment | Use `newlines keep` if newlines inside that block should remain separators. |
| A comment marker in a string starts a comment | Check the default quote convention or give each comment form the appropriate `outside` pattern. |
| Unicode names fail | Use `input utf8` and name rules containing the desired characters or Unicode properties. |
| Two visually identical names compare differently | Decide whether the application should normalize or case-fold names before comparison. Preserve their original spelling for messages. |
| A tree has more wrappers than expected | Authored rule names remain in the named view. Use fields or a projection for application access. |
| `then tree` has no visible effect | Call `request.ast()` after a successful parse; the named view remains a separate representation. |
| A binary `tree rule` declaration has no visible effect | Call `request.projection()` and its `run()`. |
| An AST field reports an unexpected child count | Put `as item` inside the repetition and match the mapping suffix to the rule: `item?`, `item*` or `item+`. Each capture supplies one mapped node. |
| A custom message is not used | Check that it names the rule failing at that position and that the context is unambiguous. |
| Only one error is shown | Strict requests stop at failure. Use a declared record recovery request for multi-error inspection. |
| Status 5 is returned | The input was accepted by Earley but has no unique finite tree. Inspect the forest. |
| Raising a limit does not help | Check the diagnostic category: conflicts and fixed implementation constraints need a different response. |
| Old behavior remains after editing the grammar | Regenerate, assemble and relink. Compile against matching imports from the same build. |

The [comma-list example](../tests/example.levell) is another small complete
language. The [backend examples](../examples/backends) illustrate choices and
ambiguity, and the [calculator](../examples/calculator) shows evaluation.

## Glossary

| Term | Meaning |
|---|---|
| Alternative | One of several allowed matches in a rule. In ordered parsing, earlier alternatives have priority. |
| Ambiguity | More than one derivation for the same complete input. |
| AST | Abstract syntax tree: an application-oriented representation of constructs such as assignments or calls. |
| Backend | The algorithm implementing the grammar. |
| Capture | A matched part of a parser rule given a field name with `as`. |
| CFG | Context-free grammar: rules describe possible structures without choosing among alternatives by their order. |
| Charset | A named set of characters or bytes. |
| Conflict | A decision the chosen deterministic parser cannot resolve from the grammar. |
| Derivation | A way of applying grammar rules to recognize an input. |
| DFA | Deterministic finite automaton: the state-table recognizer used by the lexer. |
| Grammar | Rules describing how tokens form a valid document. |
| Lexer | The component that recognizes tokens in source text or bytes. |
| Lookahead | Inspecting upcoming input to decide what can match. |
| NFA | Nondeterministic finite automaton: an intermediate lexical representation that can track several possible matches before it is converted to a DFA. |
| Nullable | Able to match without consuming input. |
| PEG | Parsing expression grammar: alternatives are tried in order, and a locally successful choice is retained. |
| Precedence | The relative binding strength of operators. |
| Associativity | The grouping of operators at the same precedence, such as `(a-b)-c` for left associativity. |
| Parse forest | A shared representation of competing derivations. |
| Parser | The component that matches tokens against grammar rules and builds their structure. |
| Projection | A pass that constructs a smaller tree from selected parts of another tree. |
| Recovery / resynchronization | Continuing after damaged input at a chosen boundary to find later errors or structure. |
| Source span | A range in the original source; Level L trees and tokens use byte offsets. |
| Start rule | The rule describing a complete parser input. |
| Token | A recognized piece of input, with a kind and source range. |
| Unicode scalar | A Unicode codepoint other than a surrogate; one matching unit in `input utf8`. |
| Visitor / tree walk | Application code that visits tree nodes to inspect, validate, transform or execute them. |


## Level L tool ownership and callable generation

`langtools/` owns Level L itself; `lib/rxfnsl/` owns the generic SDK.
The tool library `rxlanglib.rxbin` supplies `llfront` and `llgenerate`, separately
from `rxfnsl.rxbin`. Grammar-generated programs use only the SDK; hosts that
compile specifications must explicitly import and link the tool library too.
There is no dependency from either product into the experimental C/G compilers.

`llbinmeta` reads callable contracts independently of the source compilers.
A callable definition must point to its own procedure and reciprocal public
export within the same pool. Imported prototypes do not become definitions
when pools are merged. Genuine duplicate definitions remain ambiguous; private
callables and malformed procedure/export references are rejected.

`catalog.field(12, attributeName, '', 4)` returns the META_ATTR register index
as decimal text: `-1` is the unmapped sentinel and `0..2147483647` are mapped
indices (**CMETA-AC01**). This reads the existing full 64-bit wire field, not a string-pool ID.
An unsupported index or selected record size returns empty and sets the existing
`LL_RXBIN_IMPORT` diagnostic; a missing attribute returns empty under the ordinary
field lookup contract. Fields 1–3 retain name/options/type semantics (**CMETA-AC02**). Register
views derive from the complete type; no separate view field is inferred.

The independent [metadata attribute fixture](../tests/metadata-attributes.crexx)
covers these contracts in complementary owning build/VM modes (**CMETA-AC03**).

### Selected callable classification

The locally qualified `llbinmeta` classification increment lets independent
consumers distinguish an exported procedure from a callable with a hidden
class/interface owner, using existing RXBIN007 facts. It does not parse source,
execute provider code or decide a consumer language's allowed ABI.

`catalog.classify(name:.string)->.llbinmeta..callable` selects one real definition
through the same pool-local procedure and reciprocal public-export checks as
`lookup`. Imported prototypes are not definitions. New classification matches
the whole name strictly under the existing metadata identity casing; it does
not trim or pad names. Identifier bytes follow
[metadata conformance](../../compiler/docs/metadata_conformance.md#record-envelope-and-responsibilities),
including non-ASCII bytes and qualified names. Consumer language casing remains
the consumer's responsibility.

`llbinmeta..validname(name:.string)->.int` is the pure metadata identifier check,
available without constructing or preparing a catalogue. It returns `1` for a
nonempty byte spelling whose first byte is not a digit and whose bytes are ASCII
letters/digits, non-ASCII bytes, `_!?$@#` or dots; otherwise it returns `0`.
It neither folds nor trims names and does not impose per-component dot rules.
`catalog.validname` reserves the same name extent against a bounded catalogue's
work allowance before delegating to this one check. LMC-AC03 covers utility and
catalogue name checks through the independent name/formal-name controls;
consumer type and signature rules remain separate.

The owned result adds `kind()->.string` and `owner()->.string`: `procedure` has
an empty owner; `method` and `factory` name their qualified class/interface.
An ordinary class method needs no `META_MEMBER`. A real interface default method
needs the matching `method final` member. Exact `owner.§factory` and
`owner.§factory.member` identify default and named factories; a factory marker
without an owning header refuses. A similarly spelled `§factoryish` is not a
marker. `META_IMPLEMENTS` does not own a callable. Conflicting or multiple
matching owners refuse rather than choosing a prefix or record order.
Header/member flag, type and argument fields are checked as valid string
references without NUL; their language-specific grammar and ABI remain the
consumer's responsibility. The callable signature comes from its selected
`META_FUNC`, not a guessed string equivalence with a member declaration.
The existing four-argument `callable` factory, `lookup` and `field` retain their
contracts; their kind/owner values are empty. An additional
`callable.classified(name,result,arguments,flags,kind,owner)` factory constructs
owned fields, but arbitrary factory fields do not prove catalogue admission.

Classification builds one immutable request-owned locator index. Every locator
retains its pool and row identity. It associates headers and members only in the
selected definition's pool, without inferring ownership from a dot count.
A recoverable matching owner with malformed payload refuses the selected
classification. A readable owner with unreadable member identity defers an
owner-scoped error; readable owner/member with unreadable kind defers errors for
both possible method/factory identities. Wholly unkeyable unrelated ownership
records remain opaque. Selected errors return a failed callable without poisoning
unrelated selections. Completed classification outcomes are cached by selected
real row and pool: either success or a deterministic selected-data failure.
Arbitrary missing names are not retained.
The existing `read_image`/`records` helper wrappers invalidate this new index and
cache before mutating storage; a subsequent rebuild is charged again. Normal
lazy preparation uses the reference workers directly.

`catalog.bounded(image:reference .binary, bytes:.int, work:.int)` supplies required
nonnegative allowances. The synchronous input borrow is checked before the one
owned image copy, and is not retained. Zero is valid and exhausts at the first
required reservation. `consumed()->.int` and `work()->.int` query counters without
preparing the catalogue. `consumed` counts reserved logical payload bytes:
owned image/expanded sections, integer vectors at eight bytes per cell, locator
and cache payloads, retained text and returned owned fields. It never refunds
space after failure or release. It is not a resident-memory or VM temporary
accounting promise. Fixed catalogue control state and its bounded fixed failure
diagnostic are exempt, so an exhausted allowance can still report failure;
input-derived text and owned metadata results are charged. The existing
`catalog(image,budget=16777216)` remains an
unmetered legacy route; both counters return zero there.

One work unit is a byte scanned, copied, expanded or passed through text
validation/casing, plus a structural header/record/candidate/link/cache visit or
index/search comparison. Bulk operations reserve their whole byte extent before
execution; this does not add a method call per byte. Compressed input and expanded
output are both charged. Reserved work is not refunded after an early malformed
input. Checked subtraction precedes every reservation. Preparation/indexing runs
once; indexed selections and cached results still charge query and owned-copy
work. No operation rearms the allowance.

Exhaustion is sticky on that bounded catalogue and reports
`LL_RESOURCE_LIMIT: metadata.bytes` or `metadata.work`; no partial index or
selection is published. A dead input reference reports
`LL_RXBIN_IMPORT: image owner is not live`. Envelope failures retain import
diagnostics. Owned callable results survive source/catalogue expiry. A caller
can group all selections for one artifact, debit the counters and owned snapshot
copies from its aggregate allowance, release that catalogue, and pass the
remaining allowances to the next; no shared mutable quota service is needed.

**LMC-AC01–06 are locally qualified.** The independent
[metadata classification fixture](../tests/metadata-classify.crexx) and its tiny
RXAS producers own the behavioral, lifecycle and logical-budget checks;
actual reader/sort/cache RXAS is part of code review. This establishes the stated
metadata boundary, not arbitrary provider-code validity or a resident-memory cap.

1. **LMC-AC01:** plain, class, exact factory and interface-default callables
   classify from the selected real row and pool; prototype/duplicate/owner
   conflicts refuse; legacy lookup/field/factory results remain unchanged.
2. **LMC-AC02:** selected malformed/partial ownership refuses, unrelated opaque
   rows do not poison it, and foreign-pool ownership is never joined. Tiny RXAS
   producers and scalar record mutations exercise these boundaries.
3. **LMC-AC03:** strict full names preserve supported identifier bytes and
   metadata casing; no language-specific Unicode or resolution algorithm is added.
4. **LMC-AC04:** returned fields own their lifetime, while preparation/indexing
   and selected-row caching remain bounded and misses do not grow retained state.
5. **LMC-AC05:** pre-copy input, preparation, index, selection and output-copy
   reservations exhaust atomically; counters are monotonic/non-triggering;
   sequential groups passed remaining budgets cannot exceed the aggregate.
6. **LMC-AC06:** independent B consumers pass in complementary compiler/VM modes;
   readable RXAS confirms scoped image aliases, no repeated image copies and
   no new native/VM service or compiler-policy dependency.

Implementation steps: **LMC-STEP01 — complete:** contract and independent
criteria (AC01–06); **LMC-STEP02 — complete:** factored reader, bounded route,
classifier and fixtures (AC01–05); **LMC-STEP03 — complete:** complementary
consumer qualification, actual RXAS review and local dependency delivery
(AC01–06).

### Callable generation service

```rexx
import llgenerate
import llir
request = .llgenerate..request(source, 'rxas', 'lesson.levell')
if request.status() <> 0 then say request.error()
else do
  output = request.output()
  say output.text()
end
```

Here source is `.binary`. Formats are `rxas`, `crexx` and `graph`; optional
arguments are `limits=.lllimits..policy()`, `rxl=0`, `bytes=67108864`, and a
trailing optional `lexical=.binary`.
Queries evaluate once. Output owns its chunks. Failed admission, generation or
budget checks produce a diagnostic and no partial artifact. The service performs
no file I/O or printing; the `rxlang` CLI is a consumer. See
[tool documentation](../README.md) for bootstrap and helpers.

Omit `lexical` for ordinary cold behavior without capsule work. Present empty
binary opts into cold construction plus `lexical()` output; present owned bytes
tries reuse of the complete main/outside-comment-shield DFA set. `reused()` is
1 only on successful checked reuse; graph/error/disabled/cold requests return 0.
`lexical()` owns its returned bytes and is empty on errors, graph requests or
unprepared manual builds. Source/parser/policy/output checks always run on the
current specification. The existing `bytes` cap bounds capsule/key work too;
oversize/stale/malformed candidates miss without overriding ordinary limits.
Persisted-byte integrity is the host's responsibility; the wrapper checks SHA256.
