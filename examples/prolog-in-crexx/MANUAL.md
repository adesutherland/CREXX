# Prolog-in-CREXX — User Manual

*A Prolog interpreter, written entirely in CREXX.*

---

## Table of Contents

1. [Introduction to Prolog](#1-introduction-to-prolog)
2. [Getting Started](#2-getting-started)
3. [Sample Sessions](#3-sample-sessions)
   - [3.1 A family tree — facts, rules, backtracking](#31-a-family-tree--facts-rules-backtracking)
   - [3.2 Building, saving, clearing, and reloading a knowledge base](#32-building-saving-clearing-and-reloading-a-knowledge-base)
   - [3.3 Arithmetic and list processing](#33-arithmetic-and-list-processing)
   - [3.4 Recursion, cut, and control](#34-recursion-cut-and-control)
   - [3.5 Calling Prolog from a CREXX program](#35-calling-prolog-from-a-crexx-program)
   - [3.6 The other direction: Prolog calling back into CREXX](#36-the-other-direction-prolog-calling-back-into-crexx)
4. [Language Reference](#4-language-reference)
5. [Command Reference](#5-command-reference)
6. [Known Limitations](#6-known-limitations)
7. [Troubleshooting and Tips](#7-troubleshooting-and-tips)
8. [Package Contents](#8-package-contents)
9. [Index](#9-index)

---

## 1. Introduction to Prolog

Prolog is a **declarative logic programming language**. Instead of
writing a step-by-step recipe for the computer to follow (as in
procedural languages like C, Python, or Rexx), you write a set of
**facts** and **rules** describing what is true, and then **ask
questions**. Prolog's *inference engine* searches for answers on its
own.

### Facts

A fact states something that's simply true:
```prolog
parent(tom, bob).
```
This means: "tom is a parent of bob." No further explanation — it's
just a statement of fact, forever true within this program (unless you
later remove it).

### Rules

A rule states something that's true *whenever some condition holds*:
```prolog
grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
```
Read the `:-` as "if," and the comma as "and": *X is a grandparent of Z
if X is a parent of Y and Y is a parent of Z.* `X`, `Y`, and `Z` are
**variables** (always capitalized, or starting with `_`) — placeholders
that Prolog fills in while trying to prove something.

### Queries

Once you have facts and rules, you **ask questions**:
```prolog
?- grandparent(tom, X).
```
*"Is there an X such that tom is a grandparent of X?"* Prolog searches
its facts and rules, trying to make the rule's conditions true, and
reports back whatever it finds for `X`.

### Unification

The mechanism underneath all of this is called **unification** — the
process of matching two terms together, filling in variables as
needed, so they become identical. `parent(tom, X)` unifies with
`parent(tom, bob)` by binding `X = bob`. It's a bit like pattern
matching, but it works in every argument position at once, and can
match a variable on *either* side.

### Backtracking

If a query has more than one possible answer, Prolog doesn't just give
you the first one and stop — it remembers where it made a choice, and
if you ask for another answer, it goes back (**backtracks**) to that
choice point and tries the next alternative. This is how a single
query like `parent(tom, X)` can produce *every* child of tom, one at a
time, on request.

### Why this is different

In a procedural language, you'd write a loop to search a list of
parent-child pairs. In Prolog, you write down the *relationships*, and
the search is Prolog's job. This makes Prolog especially good at
problems that are naturally about relationships and search: family
trees, parsers, puzzles, rule-based expert systems, and symbolic
reasoning generally.

The rest of this manual assumes no prior Prolog experience, but if
you've used any Prolog before, skip ahead to [§2](#2-getting-started) —
the syntax here is close to standard Prolog, with a few small,
clearly-marked differences noted throughout.

---

## 2. Getting Started

### Running the interpreter

```
crexx prolog.crexx
```
starts the interactive session. You'll see:
```
Prolog-in-CREXX -- type )help for session commands, halt. to quit
(end a QUERY with ? -- e.g. female(ann)?  or parent(tom,X)? -- end a FACT or RULE with . to add it -- e.g. female(ann). or foo(X):-bar(X).)
?-
```
That `?-` is the prompt — type a line, end it with `.` or `?` (see
below), and press Enter.

You can also load a file of facts and rules right away:
```
crexx prolog.crexx -args myfile.pl
```
This consults `myfile.pl` before dropping you at `?-`, exactly as if
you'd typed `consult('myfile.pl').` as your first command.

### The most important thing to know: `.` vs `?`

**This is the one convention that shapes everything else in this
manual**, and it's a deliberate design choice specific to this
interpreter (most other Prolog systems require an `assertz(...)` call
or a separate file to add facts). The *terminator* you end a line with
tells the interpreter what you mean:

| You type | It ends in | What happens |
|---|---|---|
| `female(ann).` | `.` | **Added** to the database (a fact) |
| `foo(X) :- bar(X).` | `.` | **Added** to the database (a rule) |
| `female(ann)?` | `?` | **Asked** — is it true? |
| `parent(tom, X)?` | `?` | **Asked** — what are the possible values of X? |

A handful of names that only ever make sense as an action — `halt`,
`listing`, `consult`, `save`, `write`, `is`, and everything else
already built in to the interpreter — are always executed immediately,
regardless of which terminator you use, since nobody wants a database
fact literally named `halt`. It's specifically queries against *your
own* predicates where the terminator is what decides "remember this"
versus "ask about this" — otherwise there'd be no way to tell them
apart, since they're the same syntax.

If you're pasting in Prolog code from somewhere else, `?- goal.` and
`:- goal.` (the conventional SWI-Prolog/file-directive style, always
ending in `.`) are also recognized as questions, so code written for a
more traditional Prolog top level still does the right thing here.

### Answers and backtracking

After a successful question, you'll be asked `more (;)?`:
```
?-
parent(tom, X)?
X = bob
     more (;)? 
```
Reply `;` to backtrack and see another answer, or just press Enter to
accept this one and return to `?-`. This happens after *every*
successful answer, even one with only a single possible value — that's
completely normal, and matches how a real interactive Prolog session
behaves.

If a question has no answer at all, you'll see `false.` instead.

---

## 3. Sample Sessions

Every transcript below was captured from a real run of the
interpreter — nothing here is hypothetical. `?-` marks the prompt;
everything after it on the same or following lines is what was typed;
plain lines are the interpreter's response. Where a session asks for
more than one answer, the `;` (backtrack) and blank-line (stop) replies
are shown exactly as typed.

### 3.1 A family tree — facts, rules, backtracking

`family.pl` (included in this package):
```prolog
parent(tom, bob).
parent(tom, liz).
parent(bob, ann).
parent(bob, pat).
parent(pat, jim).
male(tom).
male(bob).
male(jim).
female(liz).
female(ann).
female(pat).
grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
sibling(X, Y) :- parent(Z, X), parent(Z, Y), X \= Y.
ancestor(X, Y) :- parent(X, Y).
ancestor(X, Y) :- parent(X, Z), ancestor(Z, Y).
```

```
?- consult('family.pl').
true.
     more (;)? 
?- parent(tom, X)?
X = bob
     more (;)? ;
X = liz
     more (;)? 
?- grandparent(tom, X)?
X = ann
     more (;)? ;
X = pat
     more (;)? 
?- sibling(bob, liz)?
true.
     more (;)? 
?- ancestor(tom, jim)?
true.
     more (;)? 
?- findall(X, parent(tom, X), Kids)?
Kids = [bob,liz]
     more (;)? 
```

Notice `ancestor/2` proved `tom` is an ancestor of `jim` *transitively*
(tom → bob → pat → jim) — nothing in the query said how many
generations to search; the recursive rule handles any depth.

### 3.2 Building, saving, clearing, and reloading a knowledge base

```
?- likes(mary, wine).
%  added: likes(mary,wine)
?- likes(mary, food).
%  added: likes(mary,food)
?- likes(john, wine).
%  added: likes(john,wine)
?- likes(mary, X)?
X = wine
     more (;)? ;
X = food
     more (;)? 
?- )listing
likes(mary,wine).
likes(mary,food).
likes(john,wine).

?- save('mynotes.pl').
prolog: saved 3 clause(s) to mynotes.pl
true.
     more (;)? 
?- )clear
prolog: workspace cleared.
?- likes(mary, wine)?
prolog: unknown procedure likes/2
false.
?- consult('mynotes.pl').
true.
     more (;)? 
?- likes(mary, wine)?
true.
     more (;)? 
```

`)listing` and `save/1` only ever show/export **your own** facts and
rules — never the built-in library (`append`, `member`, and so on) —
so `mynotes.pl` above is a clean, three-line file, not a hundred-line
dump. See [§5](#5-command-reference) for the `_all` variants that
include everything, and `)lib` for listing what `.pl` files are
sitting in a directory, ready to `consult`.

### 3.3 Arithmetic and list processing

```
?- X is 2 + 3 * 4?
X = 14
     more (;)? 
?- X is (2 + 3) * 4?
X = 20
     more (;)? 
?- append([1,2,3], [4,5], L)?
L = [1,2,3,4,5]
     more (;)? 
?- member(X, [a,b,c])?
X = a
     more (;)? ;
X = b
     more (;)? ;
X = c
     more (;)? 
?- sort([3,1,4,1,5,9,2,6], S)?
S = [1,2,3,4,5,6,9]
     more (;)? 
?- between(1, 5, X)?
X = 1
     more (;)? ;
X = 2
     more (;)? ;
X = 3
     more (;)? ;
X = 4
     more (;)? ;
X = 5
     more (;)? 
?- sum_list([1,2,3,4,5], S)?
S = 15
     more (;)? 
```

### 3.4 Recursion, cut, and control

`recur.pl` (included in this package):
```prolog
factorial(0, 1) :- !.
factorial(N, F) :- N > 0, N1 is N - 1, factorial(N1, F1), F is N * F1.

fib(0, 0) :- !.
fib(1, 1) :- !.
fib(N, F) :- N > 1, N1 is N-1, N2 is N-2, fib(N1,F1), fib(N2,F2), F is F1+F2.
```

```
?- consult('recur.pl').
true.
     more (;)? 
?- factorial(10, X)?
X = 3628800
     more (;)? 
?- fib(15, X)?
X = 610
     more (;)? 
?- first_match([X|_], X) :- !.
%  added: first_match([_X|_],_X) :- !
?- first_match([_|T], X) :- first_match(T, X).
%  added: first_match([_|_T],_X) :- first_match(_T,_X)
?- first_match([a,b,c], X)?
X = a
     more (;)? 
?- (5 > 3 -> write(bigger) ; write(smaller)), nl?
bigger
true.
     more (;)? 
```

The `!` (cut) in `factorial(0, 1) :- !.` and `first_match`'s first
clause tells Prolog "don't bother trying any later matching clauses for
this call" once this one has succeeded — it's what stops
`first_match([a,b,c], X)` from also finding `b` and `c` on
backtracking, and what makes `factorial(0, F)` unambiguous.

### 3.5 Calling Prolog from a CREXX program

Beyond the interactive interpreter, this system can be embedded and
called from another CREXX program — see `caller_example.crexx`
(included in this package) for a complete, runnable demonstration.
The short version:

```rexx
options levelb
import prolog

call prolog..PrologConsultString "factorial(0,1) :- !.",
    "factorial(N,F) :- N>0, N1 is N-1, factorial(N1,F1), F is N*F1."
result = prolog..PrologCallGet("factorial(6, X)", "X")
say result   /* 720 */
```

**Important:** because of a toolchain limitation in this beta of
CREXX, you can't just run `crexx yourprogram.crexx` for this — you
need to pre-link your program together with `prolog.crexx` first. The
included `build_with_prolog.sh` script automates this:
```
./build_with_prolog.sh yourprogram.crexx
/path/to/rxvm yourprogram_linked.rxbin
```
(the script prints the exact `rxvm` command to run as its last line).
This only applies to *embedding* Prolog in another program — running
`crexx prolog.crexx` directly, as in every other section of this
manual, is unaffected and needs no special build step.

The full embedding API — `PrologCall`, `PrologGetVar`,
`PrologCallGet`, `PrologConsultFile`, `PrologConsultString`,
`PrologFindAll` + `PrologFindAllNth`/`Count`, `PrologReset` — is
documented in [§5.6](#56-embedding-api-for-crexx-programs) and directly
in comments above each procedure in `prolog.crexx`.

### 3.6 The other direction: Prolog calling back into CREXX

`crexx_call/2` and `crexx_call/3` let a Prolog **goal** call out to
CREXX code, by name, and get a value back — the reverse of
[§3.5](#35-calling-prolog-from-a-crexx-program):

```
?- crexx_call(hello, R)?
R = 'hello from CREXX'
     more (;)? 
?- crexx_call(double, 21, R)?
R = 42
     more (;)? 
?- crexx_call(nosuchname, R)?
prolog: crexx_call: no CREXX callback registered for nosuchname -- see crexxcallback.crexx in this package
false.
```

This works **out of the box** in the standalone interpreter (as
above — no special build step) because a small dispatcher file,
`crexxcallback.crexx` (included in this package, and required —
`prolog.crexx` won't compile without *some* `crexxcallback.crexx`
alongside it), ships with two working examples, `hello` and `double`,
wired in already. Everything else returns the "no callback registered"
message you see above.

**To make your own CREXX functions reachable from Prolog:** open
`crexxcallback.crexx`, keep its `namespace crexxcallback expose
CrexxCallDispatch` line and the `CrexxCallDispatch(name, argtext)`
signature exactly as they are (that's the fixed contract
`prolog.crexx` calls), and add your own cases to its `select` block —
routing whatever `name` you choose to whatever CREXX logic you want,
returning whatever text you want the Prolog goal's `Result` unified
with. A returned value that looks like a number becomes a Prolog
number; anything else becomes a quoted atom (safe for arbitrary text).
Then — since this changes what `prolog.crexx` itself depends on —
**rebuild** with `build_with_prolog.sh`, pointing it at your version:
```
./build_with_prolog.sh yourprogram.crexx yourcrexxcallback.crexx
/path/to/rxvm yourprogram_linked.rxbin
```
(omit the second argument to keep using the default stub). This
applies to embedding scenarios; if you've only edited
`crexxcallback.crexx` and still just want the plain `crexx
prolog.crexx` REPL, no linking is needed — that command already
recompiles everything it needs from source each time.

**Why this needs a file you edit, rather than "just register a
function"**: CREXX has no function values/pointers to hand around at
runtime, so there's no way for `prolog.crexx` to accept an arbitrary
caller-supplied procedure and remember it for later. `crexxcallback.crexx`
is a fixed, well-known place both sides agree on instead — the same
trick, really, as `PrologCall` taking goal *text* rather than a
pre-parsed value, applied to the harder direction.

---

## 4. Language Reference

### Terms

Everything in Prolog is a **term**, of one of these kinds:

- **Atoms** — plain names, starting lowercase: `tom`, `parent`,
  `[]` (the empty list), `!` (cut). An atom that needs characters a
  plain name can't have (spaces, a capital first letter, etc.) is
  written quoted: `'Hello World'`, `'+'`.
- **Numbers** — integers or decimals: `42`, `-7`, `3.14`.
- **Variables** — start with an uppercase letter or `_`: `X`, `Name`,
  `_Temp`. A bare `_` (or any name starting with `_` that you never
  reuse) is *anonymous* — a fresh, "don't care" variable every time it
  appears.
- **Compound terms** — a functor applied to arguments:
  `parent(tom, bob)`, `f(X, g(Y))`. No space is allowed between the
  functor name and the `(` — `foo(X)` is a compound term; `foo (X)`
  (with a space) is read differently, as `foo` applied to a
  parenthesized group.
- **Lists** — written `[a, b, c]`, or with an explicit
  head/tail split, `[Head|Tail]`. `[]` is the empty list.
- **Strings** — double-quoted text, `"like this"`, becomes a list of
  character codes (the traditional Prolog meaning), e.g. `"ab"` is the
  same as `[97,98]`.

### Comments

`% to end of line`, and `/* block comments */`.

### Operators and precedence

Terms can be written with operators instead of explicit functor
syntax — `1 + 2` instead of `+(1,2)`. This interpreter supports a
**fixed** set of standard operators (not user-definable via `op/3`,
which is a documented limitation — see [§6](#6-known-limitations)):

| Priority | Type | Operators |
|---|---|---|
| 1200 | xfx | `:-`  `-->` |
| 1200 | fx | `:-`  `?-` |
| 1100 | xfy | `;`  `\|` |
| 1050 | xfy | `->` |
| 1000 | xfy | `,` |
| 900 | fy | `\+` |
| 700 | xfx | `=` `\=` `==` `\==` `@<` `@>` `@=<` `@>=` `is` `<` `>` `=<` `>=` `=:=` `=\=` `=..` |
| 500 | yfx | `+` `-` `/\` `\/` `xor` |
| 400 | yfx | `*` `/` `//` `mod` `rem` `<<` `>>` |
| 200 | xfy | `^` |
| 200 | xfx | `**` |
| 200 | fy | `-` `\` |

(For the curious: "xfx" means the operator takes lower-priority
operands on both sides; "xfy"/"yfx" allow chaining on the right/left
respectively; "fx"/"fy" are prefix forms. This table controls how
`1 + 2 * 3` correctly reads as `1 + (2 * 3)` without any parentheses.)

A `-` glued directly to a following number (`-5`, no space) is always
read as a negative number literal, not a subtraction; write `X - 5`
(with spaces, or at least not glued) if you mean subtraction of the
value 5.

---

## 5. Command Reference

### 5.1 Session commands

Typed directly at `?-`, starting with `)` — these are interpreter
commands, not Prolog goals, and never end in `.` or `?`.

| Command | What it does |
|---|---|
| `)listing` | List every fact/rule **you've** added this session (not the built-in library) |
| `)lib` | List `.pl` files in the current directory, ready to `consult` |
| `)lib dir` | ...or in a given directory instead |
| `)clear` | Wipe the database back to a fresh session (built-in library reloaded, your own data gone) |
| `)help` or `)?` | Show a summary of session commands and the `.`/`?` convention |
| `)quit` or `)exit` | End the session |

### 5.2 Control

| Goal | Meaning |
|---|---|
| `Goal1, Goal2` | **And** — both must succeed |
| `Goal1 ; Goal2` | **Or** — try Goal1, then Goal2 on backtracking |
| `Cond -> Then ; Else` | **If-then-else** — commits to Cond's first solution |
| `Cond -> Then` | If-then with no else (fails if Cond fails) |
| `\+ Goal` (or `not(Goal)`) | **Negation as failure** — succeeds iff Goal has no solution |
| `!` | **Cut** — commits to all choices made since entering the current clause |
| `call(Goal)`, `call(Goal, Extra, ...)` | Meta-call — invoke a goal built at runtime, optionally with extra arguments appended |
| `true` | Always succeeds |
| `fail` / `false` | Always fails |

### 5.3 Arithmetic

`is/2` evaluates the arithmetic expression on its right and unifies
the result with its left argument: `X is 2 + 3`. Comparisons evaluate
*both* sides arithmetically first.

| Predicate | Meaning |
|---|---|
| `X is Expr` | Evaluate Expr, unify with X |
| `A =:= B` | Arithmetically equal |
| `A =\= B` | Arithmetically not equal |
| `A < B`, `A > B`, `A =< B`, `A >= B` | Arithmetic comparison |

Operators/functions usable inside an arithmetic expression: `+ - * /
// mod rem **`, and the functions `sqrt(X)` `abs(X)` `sign(X)`
`truncate(X)` `round(X)` `floor(X)` `ceiling(X)` `integer(X)`
`float(X)` `min(A,B)` `max(A,B)`, plus the constants `pi` and `e`. `//`
truncates toward zero; `mod`'s result takes the sign of the divisor;
`rem`'s result takes the sign of the dividend. No bitwise operators are
supported.

### 5.4 Built-in predicates

*Term unification and comparison*

| Predicate | Meaning |
|---|---|
| `A = B` | Unify A and B |
| `A \= B` | A and B do **not** unify |
| `A == B` | A and B are **structurally identical** (stronger than `=` — no binding takes place) |
| `A \== B` | Not structurally identical |
| `A @< B`, `A @> B`, `A @=< B`, `A @>= B` | Standard order of terms (`Var < Number < Atom < Compound`) |
| `compare(Order, A, B)` | Order = `<`, `=`, or `>` |

*Type checking*

| Predicate | True when the argument is... |
|---|---|
| `var(X)` | an unbound variable |
| `nonvar(X)` | not an unbound variable |
| `atom(X)` | a plain or quoted atom |
| `atomic(X)` | an atom or a number (not compound, not a variable) |
| `number(X)` | a number |
| `integer(X)` | a whole number |
| `float(X)` | a number with a fractional part |
| `compound(X)` | a compound term |
| `callable(X)` | an atom or compound term (something `call/1` could invoke) |
| `is_list(X)` | a proper list (nil-terminated) |
| `ground(X)` | contains no unbound variables anywhere |

*Term construction and inspection*

| Predicate | Meaning |
|---|---|
| `functor(Term, Name, Arity)` | Decompose Term into its functor/arity, or build a fresh Term from Name/Arity |
| `arg(N, Term, Value)` | Value is Term's Nth argument (1-based) |
| `Term =.. List` | Term as `[Functor, Arg1, Arg2, ...]`, either direction |
| `copy_term(Term, Copy)` | Copy with fresh (but still correctly shared) variables |

*Atoms and strings*

| Predicate | Meaning |
|---|---|
| `atom_length(Atom, N)` | N is the length of Atom |
| `atom_concat(A, B, C)` | C is A and B joined (both A and B must already be bound — see [§6](#6-known-limitations)) |
| `upcase_atom(Atom, Upper)`, `downcase_atom(Atom, Lower)` | Case conversion |
| `atom_codes(Atom, Codes)` | Atom as a list of character codes, either direction |
| `atom_chars(Atom, Chars)` | Atom as a list of one-character atoms, either direction |
| `number_codes(Number, Codes)`, `number_chars(Number, Chars)` | Same, for numbers |
| `char_code(Char, Code)` | A single character's code, either direction |

*Database*

| Predicate | Meaning |
|---|---|
| `assertz(Clause)` (or `assert(Clause)`) | Add a fact/rule at the **end** |
| `asserta(Clause)` | Add a fact/rule at the **front** |
| `retract(Clause)` | Remove the first matching fact/rule |
| `retractall(Head)` | Remove **every** fact/rule whose head matches |

(Typing a fact/rule directly, ending in `.`, does the same thing as
`assertz/1` — see [§2](#2-getting-started).)

*Finding all solutions*

| Predicate | Meaning |
|---|---|
| `findall(Template, Goal, List)` | List is every Template value for which Goal succeeds |

*Output*

| Predicate | Meaning |
|---|---|
| `write(Term)` | Print Term (atoms unquoted) |
| `print(Term)` | Print Term (atoms quoted if needed, like the answers you see at `?-`) |
| `writeln(Term)` | `write/1` plus a newline |
| `nl` | Just a newline |
| `tab(N)` | Print N spaces |

*Files and the database as a whole*

| Predicate | Meaning |
|---|---|
| `consult(File)` (or `load(File)`) | Load facts/rules/directives from File |
| `listing` | List what **you** added this session |
| `listing_all` | List the **whole** database, built-in library included |
| `listing(Name)` | List every clause of predicate Name (any arity), library or not |
| `save(File)` | Save what **you** added this session, as loadable Prolog source |
| `save_all(File)` | Save the **whole** database, built-in library included — a fully self-contained file |
| `halt`, `halt(N)` | End the session (optionally with exit code N) |

### 5.5 Bootstrap library predicates

These aren't native built-ins — they're ordinary Prolog, defined in
the interpreter's own standard library and loaded automatically at
startup (see `listing_all` above to view their actual source). They
behave exactly like predicates you'd define yourself, including full
backtracking support.

| Predicate | Meaning |
|---|---|
| `append(A, B, C)` | C is A and B concatenated |
| `member(X, List)` | X is an element of List (generates every element on backtracking) |
| `length(List, N)` | N is List's length — **also works backward**: `length(L, 3)` generates a list of 3 fresh variables |
| `reverse(List, Reversed)` | Reversed is List backward |
| `last(List, X)` | X is List's last element |
| `nth0(N, List, X)`, `nth1(N, List, X)` | X is the Nth element, 0- or 1-based |
| `sum_list(List, Sum)` | Sum of a list of numbers |
| `max_list(List, Max)`, `min_list(List, Min)` | Largest/smallest element |
| `sort(List, Sorted)` | Sorted, ascending, duplicates removed |
| `msort(List, Sorted)` | Sorted, ascending, duplicates **kept** |
| `between(Low, High, X)` | X ranges over every integer from Low to High (generates each on backtracking) |
| `forall(Cond, Action)` | True if Action holds for every solution of Cond |
| `select(X, List, Rest)` | Rest is List with one occurrence of X removed |
| `selectchk(X, List, Rest)` | Like `select/3`, but commits to the first match (no backtracking into alternatives) |
| `permutation(List, Perm)` | Perm is some reordering of List (generates every one on backtracking) |
| `delete(List, X, Result)` | Result is List with every occurrence of X removed |
| `include(Pred, List, Included)` | Included is every element of List for which `call(Pred, Elem)` succeeds |
| `exclude(Pred, List, Excluded)` | The opposite of `include/3` |
| `maplist(Pred, List)`, `maplist(Pred, L1, L2)`, `maplist(Pred, L1, L2, L3)` | Apply Pred across one, two, or three lists in parallel |
| `concat_atom(List, Atom)` | Join a list of atoms/text into one atom |

`include`/`exclude`/`maplist` need a **named** predicate — this
interpreter doesn't support lambda syntax like `[X]>>Goal`:
```prolog
gt2(X) :- X > 2.
```
```
?- include(gt2, [1,2,3,4], R)?
R = [3,4]
     more (;)? 
```

### 5.6 Embedding API (for CREXX programs)

Called as `prolog..FunctionName(...)` from a CREXX program that has
`import prolog` — see [§3.5](#35-calling-prolog-from-a-crexx-program)
for the build steps this requires.

| Function | Meaning |
|---|---|
| `PrologCall(GoalText)` | Parses and solves GoalText for its first solution; returns `'1'`/`'0'` |
| `PrologGetVar(VarName)` | The current value of a variable from the last `PrologCall`, as readable text |
| `PrologCallGet(GoalText, VarName)` | `PrologCall` then `PrologGetVar` in one step — the one-line way to call a predicate and get a value back |
| `PrologConsultFile(Path)` | Load a file |
| `PrologConsultString(Text)` | Load facts/rules/directives from a CREXX string |
| `PrologFindAll(GoalText, TemplateVarName)` | Collects every solution's value of TemplateVarName; returns the count |
| `PrologFindAllNth(N)` | The Nth (1-based) result from the last `PrologFindAll` |
| `PrologFindAllCount()` | The count from the last `PrologFindAll` |
| `PrologReset()` | Wipe the whole engine back to a fresh state |

### 5.7 Calling CREXX from Prolog

The reverse direction — see [§3.6](#36-the-other-direction-prolog-calling-back-into-crexx)
for the full explanation and how to add your own callbacks.

| Predicate | Meaning |
|---|---|
| `crexx_call(Name, Result)` | Call the CREXX function registered under Name (in `crexxcallback.crexx`) with no argument |
| `crexx_call(Name, ArgText, Result)` | ...or with one argument |

---

## 6. Known Limitations

Documented on purpose, not discovered by accident:

- **No user-definable operators** — `op/3` isn't supported; the
  operator table in [§4](#4-language-reference) is fixed.
- **No `bagof/3` / `setof/3`** — their free-variable-grouping semantics
  add real complexity beyond `findall/3`'s scope. Use `findall` plus
  `sort`/`msort` for the common cases.
- **No lambda syntax** (`[X]>>Goal`, from SWI's `yall` library) — pass
  a named predicate to `maplist`/`include`/`exclude` instead.
- **`atom_concat/3` only joins** — both inputs must already be bound;
  it can't nondeterministically split an atom into two parts the way
  some Prolog systems allow.
- **No occurs-check** in unification — `X = f(X)` will "succeed" and
  build a structure that would loop forever if fully traversed. This
  matches most real Prolog systems' *default* behavior (occurs-check
  is normally opt-in even where it's supported at all).
- **A `-` glued to a number is always a literal**, never subtraction —
  see the note at the end of [§4](#4-language-reference).

---

## 7. Troubleshooting and Tips

**"unknown procedure foo/2"** — you asked a question (`?`) about a
predicate that was never defined. Either you have a typo, or you meant
to add it as a fact first (ending in `.`).

**A query I expected to work instead got silently added as a new
fact** — you used `.` instead of `?`. This is the single easiest
mistake to make when you're used to a Prolog where *everything* typed
at the top level is a question. Re-read [§2](#2-getting-started) if
this keeps happening.

**The prompt keeps asking `more (;)?` and I just want it to stop** —
press Enter (or type anything that isn't `;`) rather than `;`. This
happens after every successful answer by design, matching real
interactive Prolog, even for a question with only one possible answer.

**My `)listing` looks empty even though I know I added something** —
`)listing` only shows what you've added *this session*; if you ran
`)clear` (or just restarted the interpreter) since then, that data is
gone unless you'd `save`d it first.

**I want my whole database in one file, not just what I typed** — use
`save_all/1` instead of `save/1` (see [§5.4](#54-built-in-predicates)).

**A rule I typed didn't do what I expected** — remember `,` means
*and*, evaluated left to right, and cut (`!`) only affects the clause
it's written in — it doesn't reach into a predicate you called from
there. See [§3.4](#34-recursion-cut-and-control) for a working
cut example.

---

## 8. Package Contents

| File | What it is |
|---|---|
| `prolog.crexx` | The interpreter itself. Run with `crexx prolog.crexx`. |
| `crexxcallback.crexx` | The Prolog-calls-CREXX bridge (see [§3.6](#36-the-other-direction-prolog-calling-back-into-crexx)). **Required** — `prolog.crexx` won't compile without it. The shipped version is a working default with two example callbacks. |
| `family.pl` | Sample knowledge base used in [§3.1](#31-a-family-tree--facts-rules-backtracking). |
| `recur.pl` | Sample recursive predicates used in [§3.4](#34-recursion-cut-and-control). |
| `caller_example.crexx` | A complete, runnable demonstration of every embedding-API function, both directions (see [§3.5](#35-calling-prolog-from-a-crexx-program)/[§3.6](#36-the-other-direction-prolog-calling-back-into-crexx)). |
| `build_with_prolog.sh` | Build/link helper required to run a CREXX program that embeds Prolog (see [§3.5](#35-calling-prolog-from-a-crexx-program)). |
| `MANUAL.md` | This document (Markdown source). |
| `MANUAL.docx` | This document, as a Word file — identical content, with a real navigable Table of Contents and page numbers. |
| `PORT_NOTES.md` | Technical/implementation notes: what's implemented, testing performed, and CREXX toolchain quirks discovered while building this. For maintainers/the curious, not required reading to use the interpreter. |

---

## 9. Index

- **`!`** (cut) — [§3.4](#34-recursion-cut-and-control), [§5.2](#52-control)
- **`)clear`** — [§3.2](#32-building-saving-clearing-and-reloading-a-knowledge-base), [§5.1](#51-session-commands), [§7](#7-troubleshooting-and-tips)
- **`)help` / `)?`** — [§5.1](#51-session-commands)
- **`)lib`** — [§3.2](#32-building-saving-clearing-and-reloading-a-knowledge-base), [§5.1](#51-session-commands)
- **`)listing`** — [§3.2](#32-building-saving-clearing-and-reloading-a-knowledge-base), [§5.1](#51-session-commands), [§7](#7-troubleshooting-and-tips)
- **`)quit` / `)exit`** — [§5.1](#51-session-commands)
- **anonymous variable (`_`)** — [§4](#4-language-reference)
- **append/3** — [§3.3](#33-arithmetic-and-list-processing), [§5.5](#55-bootstrap-library-predicates)
- **arg/3** — [§5.4](#54-built-in-predicates)
- **arithmetic** — [§5.3](#53-arithmetic)
- **assert/1, asserta/1, assertz/1** — [§5.4](#54-built-in-predicates)
- **atom_chars/2, atom_codes/2** — [§5.4](#54-built-in-predicates)
- **atom_concat/3** — [§5.4](#54-built-in-predicates), [§6](#6-known-limitations)
- **atom_length/2** — [§5.4](#54-built-in-predicates)
- **atoms** — [§4](#4-language-reference)
- **backtracking** — [§1](#1-introduction-to-prolog), [§2](#2-getting-started), [§3.1](#31-a-family-tree--facts-rules-backtracking)
- **bagof/3** (not supported) — [§6](#6-known-limitations)
- **between/3** — [§3.3](#33-arithmetic-and-list-processing), [§5.5](#55-bootstrap-library-predicates)
- **call/1 (meta-call)** — [§5.2](#52-control)
- **caller_example.crexx** — [§3.5](#35-calling-prolog-from-a-crexx-program), [§8](#8-package-contents)
- **char_code/2** — [§5.4](#54-built-in-predicates)
- **comments (`%`, `/* */`)** — [§4](#4-language-reference)
- **compare/3** — [§5.4](#54-built-in-predicates)
- **comparison operators (`=:=`, `<`, etc.)** — [§5.3](#53-arithmetic)
- **compound terms** — [§4](#4-language-reference)
- **concat_atom/2** — [§5.5](#55-bootstrap-library-predicates)
- **consult/1** — [§2](#2-getting-started), [§3.1](#31-a-family-tree--facts-rules-backtracking), [§5.4](#54-built-in-predicates)
- **copy_term/2** — [§5.4](#54-built-in-predicates)
- **crexx_call/2, crexx_call/3** — [§3.6](#36-the-other-direction-prolog-calling-back-into-crexx), [§5.7](#57-calling-crexx-from-prolog)
- **crexxcallback.crexx** — [§3.6](#36-the-other-direction-prolog-calling-back-into-crexx), [§8](#8-package-contents)
- **cut** — see **`!`**
- **database (assert/retract)** — [§5.4](#54-built-in-predicates)
- **delete/3** — [§5.5](#55-bootstrap-library-predicates)
- **embedding API** — [§3.5](#35-calling-prolog-from-a-crexx-program), [§5.6](#56-embedding-api-for-crexx-programs)
- **exclude/3** — [§5.5](#55-bootstrap-library-predicates)
- **facts** — [§1](#1-introduction-to-prolog), [§2](#2-getting-started)
- **findall/3** — [§3.1](#31-a-family-tree--facts-rules-backtracking), [§5.4](#54-built-in-predicates)
- **forall/2** — [§5.5](#55-bootstrap-library-predicates)
- **functor/3** — [§5.4](#54-built-in-predicates)
- **ground/1** — [§5.4](#54-built-in-predicates)
- **halt** — [§2](#2-getting-started), [§5.4](#54-built-in-predicates)
- **if-then-else (`->` `;`)** — [§3.4](#34-recursion-cut-and-control), [§5.2](#52-control)
- **include/3** — [§5.5](#55-bootstrap-library-predicates)
- **is/2** — [§3.3](#33-arithmetic-and-list-processing), [§5.3](#53-arithmetic)
- **is_list/1** — [§5.4](#54-built-in-predicates)
- **lambda syntax** (not supported) — [§5.5](#55-bootstrap-library-predicates), [§6](#6-known-limitations)
- **last/2** — [§5.5](#55-bootstrap-library-predicates)
- **length/2** — [§5.5](#55-bootstrap-library-predicates)
- **lists** — [§4](#4-language-reference)
- **listing/0, listing_all/0, listing/1** — [§3.2](#32-building-saving-clearing-and-reloading-a-knowledge-base), [§5.4](#54-built-in-predicates)
- **load/1** — [§5.4](#54-built-in-predicates)
- **maplist/2,3,4** — [§5.5](#55-bootstrap-library-predicates)
- **max_list/2, min_list/2** — [§5.5](#55-bootstrap-library-predicates)
- **member/2** — [§3.3](#33-arithmetic-and-list-processing), [§5.5](#55-bootstrap-library-predicates)
- **msort/2** — [§5.5](#55-bootstrap-library-predicates)
- **negation (`\+`, `not`)** — [§5.2](#52-control)
- **nl/0** — [§5.4](#54-built-in-predicates)
- **nth0/3, nth1/3** — [§5.5](#55-bootstrap-library-predicates)
- **number_chars/2, number_codes/2** — [§5.4](#54-built-in-predicates)
- **numbers** — [§4](#4-language-reference)
- **occurs-check** (not supported) — [§6](#6-known-limitations)
- **operators / precedence table** — [§4](#4-language-reference)
- **op/3** (not supported) — [§4](#4-language-reference), [§6](#6-known-limitations)
- **permutation/2** — [§5.5](#55-bootstrap-library-predicates)
- **print/1** — [§5.4](#54-built-in-predicates)
- **queries** — [§1](#1-introduction-to-prolog), [§2](#2-getting-started)
- **retract/1, retractall/1** — [§5.4](#54-built-in-predicates)
- **rules** — [§1](#1-introduction-to-prolog), [§2](#2-getting-started)
- **save/1, save_all/1** — [§3.2](#32-building-saving-clearing-and-reloading-a-knowledge-base), [§5.4](#54-built-in-predicates), [§7](#7-troubleshooting-and-tips)
- **select/3** — [§5.5](#55-bootstrap-library-predicates)
- **selectchk/3** — [§5.5](#55-bootstrap-library-predicates)
- **setof/3** (not supported) — [§6](#6-known-limitations)
- **sort/2** — [§3.3](#33-arithmetic-and-list-processing), [§5.5](#55-bootstrap-library-predicates)
- **standard order of terms (`@<` etc.)** — [§5.4](#54-built-in-predicates)
- **strings (double-quoted)** — [§4](#4-language-reference)
- **sum_list/2** — [§3.3](#33-arithmetic-and-list-processing), [§5.5](#55-bootstrap-library-predicates)
- **terminator convention (`.` vs `?`)** — [§2](#2-getting-started), [§7](#7-troubleshooting-and-tips)
- **type-checking predicates (`var/1`, `atom/1`, etc.)** — [§5.4](#54-built-in-predicates)
- **unification** — [§1](#1-introduction-to-prolog), [§5.4](#54-built-in-predicates)
- **upcase_atom/2, downcase_atom/2** — [§5.4](#54-built-in-predicates)
- **variables** — [§1](#1-introduction-to-prolog), [§4](#4-language-reference)
- **write/1, writeln/1** — [§5.4](#54-built-in-predicates)
- **`=..` (univ)** — [§5.4](#54-built-in-predicates)
- **`\=`, `==`, `\==`** — [§5.4](#54-built-in-predicates)
