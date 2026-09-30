# prolog.crexx — a Prolog interpreter written in CREXX

**For day-to-day use, see `MANUAL.md` instead** — it's the current,
user-facing reference (introduction, worked sample sessions, a full
command reference, and an index). This file is implementation/history
notes: what was built, how it was tested, and the CREXX toolchain
quirks found along the way. A few things described below (the
interactive top level's exact conventions, in particular) predate
later refinements — `MANUAL.md` reflects the current behavior;
consider this file's design commentary accurate for the *engine*
(parser, unification, resolution) and slightly historical for the
*top-level UX*, which evolved after this was first written.

A from-scratch Prolog implementation: tokenizer, operator-precedence
parser, unification engine with a trail for backtracking, a clause
database with cut-aware SLD resolution, a useful core of ISO-ish
built-in predicates, workspace save/load, and a set of procedures a
CREXX program can call directly to run Prolog goals and read results
back. Built the same way as the earlier `apl.crexx` port: incrementally,
testing every piece against the real CREXX toolchain (`crexx-1.0.0-beta`,
updated mid-project to the latest `master`, ~800 commits ahead of the
version used for the APL port).

## Running it standalone

```
crexx prolog.crexx                  # interactive top level
crexx prolog.crexx -args prog.pl    # consult a file, then the top level
```

**Current top-level convention (see `MANUAL.md` §2 for the full
explanation):** end a line with `.` to add it as a fact/rule, or with
`?` to ask it as a question — `female(ann).` adds a fact,
`female(ann)?` asks whether it's true. (An earlier version of this
interpreter required *every* line to be a question, `?-`-prefixed or
not, the way a traditional Prolog top level works; that's no longer
the case, per user feedback during development — seeing bare
`female(ann).` at `?-` produce an "unknown procedure" error, rather
than just adding the fact, was the original motivation.) `)listing`
shows what you've added this session; `save('file.pl').` exports it as
loadable Prolog source (the `_all` forms include the built-in library
too); `)clear` resets to a fresh session; `)lib` lists `.pl` files
ready to consult; `halt.` ends the session.

## Calling it from another CREXX program

See `caller_example.crexx` for a complete working demo covering every
API function. In short:

```rexx
options levelb
import prolog

call prolog..PrologConsultString "factorial(0,1) :- !.",
    "factorial(N,F) :- N>0, N1 is N-1, factorial(N1,F1), F is N*F1."
result = prolog..PrologCallGet("factorial(6, X)", "X")
say result   /* 720 */
```

The full API (`PrologCall`, `PrologGetVar`, `PrologCallGet`,
`PrologConsultFile`, `PrologConsultString`, `PrologFindAll` +
`PrologFindAllNth`/`Count`, `PrologReset`) is documented in comments
directly above each procedure in `prolog.crexx`, in the section headed
`CREXX-FACING API`, roughly a third of the way through the file.

**Important — you cannot just run `crexx yourprogram.crexx` for this.**
See "A real toolchain bug: cross-module calls" below. Use the provided
build script instead:

```
./build_with_prolog.sh yourprogram.crexx
/path/to/rxvm yourprogram_linked.rxbin
```

(`build_with_prolog.sh` prints the exact `rxvm` command it wants you to
run as its last line.) The standalone interpreter itself is unaffected
by this bug — `crexx prolog.crexx` works completely normally — it's
only calling *into* it from a separately-compiled program that needs
this workaround.

## Workspace save/load

`save(File)` (also callable as `prolog..PrologCall("save('File')")`,
though there's no dedicated `PrologSave` wrapper since it's just an
ordinary predicate) writes every clause currently in the database back
out as plain, valid Prolog source text — reusing the same term-writer
that powers `write/1` and the top level's answers. Loading it back is
then simply `consult(File)` (or the standalone interpreter's own
`crexx prolog.crexx -args File`), exactly like loading any other
Prolog source file. There's no separate binary/serialised format to
maintain — the save format *is* Prolog source, so it's human-readable,
diffable, and hand-editable.

## The other embedding direction: Prolog calling CREXX (`crexx_call/2,3`)

The CREXX-facing API above (`PrologCall` etc.) only covers CREXX
calling *into* Prolog. Prolog calling back *out* to arbitrary CREXX
code turned out to need a different, more constrained design, for one
concrete reason: **CREXX has no function values or pointers.** There is
no way for a general-purpose library module to accept an arbitrary
caller-supplied procedure at runtime and invoke it later — every call
target has to be a compile-time-resolved import.

Given that constraint, the only architecturally honest option was a
**fixed, single, well-known dispatch point**: `prolog.crexx` has an
unconditional `import crexxcallback` and, for every `crexx_call/2,3`
goal, calls exactly one procedure — `crexxcallback..CrexxCallDispatch(name,
argtext)` — in whatever module happens to be compiled under that exact
namespace. The embedder's own routing logic (name → real CREXX
function) lives entirely inside that one file, which they edit and
recompile; `prolog.crexx` itself never changes.

**This makes `crexxcallback.crexx` a hard, unconditional dependency of
`prolog.crexx`** — it will not compile without one present (even a
completely empty implementation would do, syntactically; the shipped
one is a working stub with two example callbacks, `hello` and
`double`, so `crexx_call/2,3` has something to actually demonstrate out
of the box, and so a name nobody has implemented yet fails with a
clear message — `'$NOCALLBACK$'`, matched via the `CREXXCALL_NOTFOUND`
sentinel — rather than a compile error or a wrong answer).

**Argument/result conversion at the boundary** deliberately mirrors
`PrologCall`/`PrologGetVar`'s own text-based boundary rather than
inventing a new convention: the Prolog argument (for `crexx_call/3`) is
converted with `WriteTerm(..., 0)` (the same unquoted rendering
`write/1` uses) before being handed to `CrexxCallDispatch`; the text
`CrexxCallDispatch` returns is unified back as a number if it looks
like one (`IsNumber`), otherwise wrapped as a quoted atom via
`MkQAtom` so arbitrary text — spaces, uppercase, anything — round-trips
safely. One real gotcha found while testing this: a **double-quoted**
Prolog string argument (`crexx_call(double, "21", Result)`) silently
becomes a list of character codes before it ever reaches
`CrexxCallDispatch` — standard Prolog string semantics, already
documented elsewhere in this file, but easy to forget applies at this
new boundary too. `WriteTerm` on a code list produces a printed list
like `[50,49]`, not `"21"`, which is very likely not what a user
expects to receive as `argtext`. Documented directly in
`crexxcallback.crexx`'s own comments as a heads-up; using a number or a
quoted atom for the argument avoids it entirely.

**A real bug found and fixed while building this**: initially,
`build_with_prolog.sh` only compiled and linked the caller and
`prolog.crexx`, on the (at-the-time-still-correct) assumption that
those were the only two modules involved. Adding the unconditional
`crexxcallback` import broke that assumption silently — `rxc`
successfully compiles `prolog.crexx` against *any* syntactically valid
`crexxcallback.crexx` it can find via source-root scanning (enough to
satisfy type-checking), but the compiled `crexxcallback.rxbin` was
never actually included in the final `rxlink` step. The result:
`build_with_prolog.sh` reported success, produced a `_linked.rxbin`
that ran fine right up until a `crexx_call/2,3` goal was actually
reached, and then PANICKED with `FUNCTION_NOT_FOUND` — a "looks done,
fails at the worst possible moment" failure mode, exactly the kind
worth calling out for anyone else building something similar on this
toolchain. Fixed by having the script explicitly compile and link a
third module (defaulting to the shipped stub, overridable via a second
command-line argument), and reverified end-to-end with a genuinely
custom (non-stub) `crexxcallback.crexx` — a real Prolog rule calling
out to custom CREXX logic (including a case using CREXX's own
`translate()` for uppercasing) and receiving the freshly-computed
result back correctly.

**Why this doesn't need the `rxlink` workaround for the standalone
REPL specifically**: running `crexx prolog.crexx` directly makes
`prolog.crexx` the *root* module — its own internal call into
`crexxcallback..CrexxCallDispatch` is an ordinary same-build call, not
one reached by *entering* a cross-module boundary first (which is what
actually triggers the cross-module bug documented below). The bug only
bites when some *other* program imports `prolog` as a dependency and
calls into it from outside — exactly the scenario `build_with_prolog.sh`
exists for, and now correctly covers for both embedding directions.

## What's implemented

- **Syntax**: facts, rules, directives (`:- Goal.`), a fixed (not
  user-definable via `op/3`) but fairly complete ISO-shaped operator
  table (`:-` `-->` `;` `->` `,` `\+` `=` `\=` `==` `\==` the `@`
  comparisons `is` the arithmetic comparisons `=..` `+` `-` `*` `/`
  `//` `mod` `rem` `**` `^`), lists (`[H|T]`, `[a,b,c]`), strings as
  code lists, quoted atoms with escapes, line and block comments.
- **Control**: `,` `;` `->` (including full if-then-else) `\+` `!`
  (real, barrier-based cut, not an approximation) `call/1..N`.
- **Arithmetic**: `+ - * / // mod rem ** sqrt abs sign` and the
  rounding family (`truncate round floor ceiling integer float`), plus
  `pi`/`e`. No bitwise operators, no transcendentals beyond `sqrt` — a
  documented scope limitation, not a bug.
- **Built-ins**: unification and term comparison (`=` `\=` `==` `\==`
  `@<` family, `compare/3`), type tests (`var` `nonvar` `atom`
  `atomic` `number` `integer` `float` `compound` `callable` `is_list`
  `ground`), term construction/inspection (`functor/3` `arg/3` `=..`
  `copy_term/2`), atom/string conversion (`atom_codes` `atom_chars`
  `number_codes` `number_chars` `atom_length` `char_code`
  `atom_concat` `upcase_atom` `downcase_atom`), database mutation
  (`assert(z)` `asserta` `retract` `retractall`), `findall/3`, I/O
  (`write` `writeln` `nl` `print` `tab`), and `consult/1` `load/1`
  `listing/0,1` `save/1` `halt/0,1`.
- **Library** (see `BootstrapLibText` — defined in Prolog itself,
  consulted at startup, not hand-written as native code, since ordinary
  backtracking through user predicates is all they need): `append`
  `member` `length` (works in both directions — including generating
  a list of fresh variables for a given length, via ordinary
  backtracking) `reverse` `last` `nth0` `nth1` `sum_list` `max_list`
  `min_list` `between` `forall` `sort` `msort` `select` `selectchk`
  `permutation` `delete` `include` `exclude` `maplist/2,3,4`
  `concat_atom`.

## What's not implemented (documented limitations)

- **No user-definable operators** (`op/3` is not supported) — the
  operator table above is fixed.
- **No `bagof/3` / `setof/3`** (free-variable grouping semantics adds
  real complexity beyond `findall/3`'s scope) — use `findall` +
  `sort`/`msort` for the common cases.
- **No SWI-style lambda syntax** (`[X]>>Goal` from library `yall`) —
  pass a named predicate to `maplist`/`include`/`exclude` instead (see
  `caller_example.crexx`'s tests, or the built-in test suite, for the
  pattern).
- **`atom_concat/3`'s "split" mode is not supported** — both the first
  two arguments must be bound; it can only concatenate, not
  nondeterministically split an atom into two parts.
- **No occurs-check** in unification (standard for most compact/toy
  implementations; `X = f(X)` will happily "succeed" and build a
  structure that would loop if printed/traversed infinitely — this
  matches most real Prolog systems' *default* behavior, which also
  omits the occurs-check unless explicitly asked for).
- **Self-recursive control flow inside `findall`'s template/goal is
  fully supported** (this was a real concern carried over from the
  `apl.crexx` port's architecture, where the tokenizer/parser's shared
  global state broke under reentrancy — Prolog's resolution engine
  does NOT share that problem, since `Solve` never re-tokenizes or
  re-parses mid-search; only genuinely-recursive *parsing* would be at
  risk, and nothing in this engine reparses while already parsing).

## Testing

Extensively tested by hand-writing dozens of queries and checking
output for correctness (there's no separate reference Prolog
implementation as convenient to diff against as Regina was for the
APL port, so this relied on manual verification against known Prolog
semantics rather than an automated side-by-side diff). Verified
correct: backward chaining and backtracking across multiple clauses
(including generating multiple grandparent solutions and correctly
exhausting them), cut (both in iterative "commit to first match"
predicates and inside recursive definitions), `findall/3` (including
building the goal and template in one parse so shared variables are
genuinely shared — see the bug note below about a mistake I initially
made testing this), the entire arithmetic evaluator (operator
precedence, `mod`/`rem` sign semantics, `//` truncation, `**` including
fractional exponents, `sqrt`), the standard order of terms, `functor/3`
in all four mode combinations, `=..` in both directions, `copy_term/2`
(confirmed shared variables in the source stay shared, and get FRESH
shared names, in the copy), the entire bootstrap library, the
interactive top level's `;`-driven backtracking and session commands,
and a full save-to-file-then-reload-and-re-query round trip (which
directly exercises the term writer's precedence-aware parenthesisation
and the parser's functor-vs-grouping-paren disambiguation working
together correctly).

## Bugs found in the CREXX toolchain along the way

None of these are bugs in the Prolog engine's logic — they're
toolchain quirks/bugs that had to be identified and worked around.
Recorded here in case they help anyone else. (See also `apl.crexx`'s
own `PORT_NOTES.md` for a first batch found during that port; two of
those — `datatype(".", "N")` returning true, and `datatype("-2","N")`
returning false — are confirmed FIXED as of the toolchain update mid-way
through this project, along with `charout()`'s inability to write text,
which was also broken at the time of the APL port.)

1. **`datatype(".", "N")` returns `1`** in this build (even after the
   update above fixed the *previously*-known `.`-related quirk from
   the APL port — this is a *different* one: a bare, standalone `.`
   character is still misclassified as numeric). This silently broke
   identifier/atom scanning in the tokenizer (a `.` immediately after
   an atom, e.g. the clause-ending period, was being swallowed into
   the atom as if it were a trailing digit). Worked around with a
   dedicated `IsDigitChar` helper using `pos(ch, '0123456789')` instead
   of `datatype(ch, 'N')` for every single-character digit check in
   the tokenizer, and hardened the whole-token `IsNumber` check to also
   require at least one real digit.

2. **A serious, genuinely hard-to-find silent compiler bug**: a
   variable that is pre-declared, then assigned and *read* inside an
   early-`return`ing `if` block, and *also* reassigned again later in
   the same procedure — in code that particular branch can never
   actually reach, since it already returned — can end up reading a
   **stale value inside the first block** instead of what was just
   computed there, with **no warning or error**. This is a different
   (and worse) failure mode than the `apl.crexx` port's "block-scoping
   trap": that one was reliably caught by re-declaring the variable
   before the conditional; this one is NOT fixed by pre-declaration,
   because the two assignments are in genuinely different, mutually
   unreachable branches, and the compiler's codegen still gets
   confused about which one a read inside the first branch should see.
   Found via `functor/3` silently returning arity `0` for every
   compound term. Confirmed via careful bisection (repeatedly
   simplifying and restructuring the same test case) that the fix is
   to split such a procedure into two separate procedures — one per
   logical branch — so that no variable is ever assigned in two
   different unreachable-from-each-other branches of the same
   procedure. Applied to `SolveFunctor` (now `SolveFunctorDecompose` /
   `SolveFunctorCompose`); given how subtle and silent this is, treat
   any two-branch procedure with a shared "result" variable name with
   suspicion if its output ever looks wrong in a way that seems to
   defy the visible logic.

3. **Functor-paren detection is more subtle than "no preceding
   whitespace"**: real Prolog syntax distinguishes `foo(X)` (compound
   term) from `foo (X)` (atom `foo` applied to a parenthesised group,
   via whatever operator context) purely by whether there's whitespace
   between the name and the `(`. A naive "is the immediately preceding
   *character* non-blank" check is not enough — it also wrongly fires
   for punctuation immediately followed by `(`, most importantly a
   comma directly followed by a parenthesised group (`,(` — which is
   very natural to end up with in compact, auto-generated source, e.g.
   this engine's own `save/1` output of a body like
   `foo(X), (Y > 0 -> ... ; ...)`written without a space). Fixed by
   also requiring that the *previously emitted token* be atom-shaped
   (a plain or quoted atom) — not a variable, number, string, or
   punctuation — before treating an adjacent `(` as a functor-paren.
   This was caught by a full save-then-reload round-trip test, which
   is worth doing for any similar project: self-generated source is a
   good adversarial test of a parser/writer pair.

4. **A real, reproducible bug in cross-module (`import`) function
   calls**: a program that does `import somemodule` and calls
   `somemodule..SomeFunction(...)` compiles cleanly, but PANICS at
   runtime with `SIGNAL FUNCTION_NOT_FOUND` as soon as `SomeFunction`
   internally calls **any other function that takes an argument** (a
   fixed literal argument is enough to trigger it — it isn't specific
   to variables, to `expose`, or to any particular function name;
   isolated all the way down to a ~10-line minimal reproduction: two
   trivial procedures `A` and `B` in a library module, where `A`
   simply does `return B("literal")`, panics when called cross-module
   as `lib..A()`, but works completely normally when the same library
   is run as the main program, or when `A`'s internal call passes NO
   argument). **Workaround**: pre-link the caller and the library into
   one combined `.rxbin` with `rxlink`, and run *that* with `rxvm`,
   rather than relying on `crexx`'s automatic dynamic import
   resolution. This reliably works — see `build_with_prolog.sh`, which
   automates the four-command `rxc` / `rxas` / `rxc` / `rxas` /
   `rxlink` sequence. This bug does not affect running `prolog.crexx`
   standalone (as the interactive interpreter, or via
   `-args file.pl`); it only affects a *different* program importing
   and calling into it.

5. **A namespace declaration must match the file's own identity.** A
   verbatim copy of `prolog.crexx` under a different filename fails to
   compile with a long cascade of `TYPE_MISMATCH`/`BAD_CONVERSION`
   errors on essentially every exposed global in the file, even though
   the original compiles cleanly — until the copy's own
   `namespace prolog expose ...` declaration is also renamed to match
   the new filename, at which point it compiles fine again. Worth
   knowing if you ever copy a namespaced `.crexx` file for experiments
   (rename the `namespace` line too), and it's why testing this file
   during development required care (several confusing detours in this
   project's history came from forgetting this while making throwaway
   test copies).

## A note on `PVARMAP`/variable capture and the "shared X" mistake

When testing `findall/3` (and, by extension, anything that needs to
relate a variable in one part of a query to the same variable
somewhere else), an early mistake was parsing the goal and the
template as **two separate calls** to the clause parser, expecting
their same-named `X` to refer to the same variable. It doesn't —
each parse gets its own fresh set of variables (correctly; this is
what makes clauses reusable). The fix is simply to parse the goal
*and* its template together, as one term, the same way a real
`findall(Template, Goal, List)` call naturally would when typed or
read as a single piece of source — which is exactly what
`PrologFindAll(goalText, templateVarName)` in the CREXX-facing API
does internally. Recorded here because it's an easy trap to fall into
again if extending this engine, not because it was ever a bug in the
engine itself.
