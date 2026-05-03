# Program Control

If we go further than scrips that are straightforward lists of clauses and external commands, we need keywords to manipulate the flow of control in a program. These are shown in this chapter.

## Changing the flow of a program

A program can be a single list of instructions, or a number of lists of instructions that determine which list to process and how many times to process it.

Instructions that direct the processing of the program are called *control instructions*. They do the following things:

Branching
: Selecting one of several lists of instructions to process. The branching instructions are `IF` and `SELECT`.

Looping
: Repeating a list of instructions, either for a speficied number of times or as long as some condition is satisfied. The `LOOP` or `DO` instruction (when used with keywords like `UNTIL` and `WHILE` do the looping in \rexx{}.

Exiting
: A program that is a single list ends when it reaches the last instruction. To explicity end a program, use the instructions `EXIT` and `RETURN`. 

## Grouping instructions

What most control instructions have in common is that they often use groups of clauses that act as a single clause. The simplest way to group clauses is with the keyword `DO`. For example:

```rexx <!--doclause.rexx-->
DO
  clause1
  clause2
  clause3
  ...
END
```

If the keyword `DO` is a clause by itself, the list of clauses that follows (up to the `END` keyword) is processed once (no loop is implied). This form of the `DO` instruction and the `END` keyword associated with it tell \rexx{} to treat the enclosed instructions as a single instruction.

## Testing Conditions

In a comparison, two terms are joined by operators as `=` (equal to), `>` (greater than), or `<` (less than) in order to pose a test of the terms. The expression itself evaluates as 1 if the comparison is *true* or 0 if the comparison is *false*. For example:

```rexx <!--comparisons.rexx-->
options levelb
/* some comparisons */
say 5 = 5           /* displays '1' - true  */
say 5 < 4           /* displays '0' - false */
say 5 = 4           /* displays '0' - false */
say 5 > 4           /* displays '1' - true  */

reply = "YES"       /* assigns the string "YES"
                      to the variable REPLY */

say reply           /* displays "YES"       */
say reply = "MAYBE" /* displays '0' - false */
say reply = "YES"   /* displays '1' - true  */
```

<!--splice--comparisons.rexx-->

## Simple Branching

To tell \rexx{} how to make a decision about a single instruction, use:

```rexx <!--ifthen.rexx-->
IF expression
THEN instruction
```

\rexx{} processes `instruction` only if `expression` is true.

```rexx <!--confirmation.rexx-->
options levelb
say "Type YES to continue"
reply = "YES"
if reply = "YES" then say "OK!"
/* program continues from here ...*/
```

<!--splice--confirmation.rexx-->

The instruction `say "OK!"` is only executed if the variable `reply` has the value `YES`. 

The IF instruction introduces a new branch of instructions to process when the `IF`
expression is true. Programmers often visualize the action of a decision-making
instruction by using a diagram like this, called a flowchart.

<!-- \begin{wrapfigure}{r}{0.4\linewidth} -->
\includegraphics[width=400pt]{flowchart_svg.pdf}
<!-- \end{wrapfigure} -->

The decision-making expression is represented by a diamond. If the expression (here
REPLY="YES") evaluates as true, then the program branches, or takes a detour,
through one additional instruction before resuming on the next line.

## Using DO..END for multiple clauses

To put a list of instructions after the `THEN`, use the `DO` instruction and the `END`
keyword. That turns the whole group into a single instruction. For example:

```rexx <!--doclause2.rexx-->
IF expression THEN
DO
instructionl
instruction2
instruction3
< ... and so on>
END
```

With the `DO` and `END` keywords bracketing the list, \rexx{} knows to treat the
listed instructions as a unit to:

- Process all of them if `expression` is true
- Ignore them all if `expression` is false.

```rexx <!--wakeup.rexx-->
options levelb
/* Wake-up call */
sun = 'shining'
if sun = "shining"
then
  do
    say "Get up ! "
    say "Get out ! "
    say "Meet the sun half way! "
  end
```

<!--splice--wakeup.rexx-->

The flowchart diagram would look like this:

\includegraphics[width=400pt]{sun_flowchart_svg.pdf}

In the previous example, if `sun = "shining"` evaluates as 1 (*true*), then all three `SAY`
instructions are processed. But if `sun = "shining"` evaluates as 0 (*false*), then none
of the `SAY` instructions are processed.

The `THEN` and `DO` keywords are each on a separate line.
This is optional. You could also write the program as:

```rexx <!--wakeup2.rexx-->
options levelb
/* Wake-up call */
sun = 'shining'
if sun = "shining" then
do
  say "Get up ! "
  say "Get out ! "
  say "Meet the sun half way! "
end
```

or

```rexx <!--wakeup3.rexx-->
options levelb
/* Wake-up call */
sun = 'shining'
if sun = "shining" then do
  say "Get up ! "
  say "Get out ! "
  say "Meet the sun half way! "
end
```

## Two paths: ELSE

Used alone, `IF` ... `THEN` adds a branch of instructions to process when the
controlling expression is true. You can also add a second branch of instructions to
process when the expression is false. The keyword `ELSE` introduces this alternate
list. For example:

```rexx <!--ifthenelse.rexx-->
IF expression
THEN instructionl
ELSE instruction2
```

When `IF` is used this way, `REXX` processes only one of these instructions, not the
other. It will process:

- Instruction l only if `expression` is true
- Instruction 2 only if `expression` is false.

The flowchart diagram would look like this:

\includegraphics[width=400pt]{who_flowchart_svg.pdf}

## The SELECT instruction

You are not limited to two choices. You can use the `SELECT` instruction to have a
\rexx{} program select one of any number of branches. For example:

```rexx <!--select.rexx-->
SELECT
WHEN expression1 THEN instruction1
WHEN expression2 THEN instruction2
WHEN expression3 THEN instruction3
OTHERWISE
	instruction
	instruction
	instruction
END
```

- If `expression1` is true, `instruction1` is processed. After this, processing
continues with the instruction following the `END`.

- If `expression1` is false, then `expression2` is tested. If it is true, then
instruction2 is processed and processing continues with the instruction
following the `END`.

- If `expression1`, `expression2`, and so on, are all *false*, then processing continues
with the instruction following the `OTHERWISE`.

`OTHERWISE` is essentially the `SELECT`-equivalent of `ELSE`. If no `WHEN` expressions evaluate to *true* and there is no `OTHERWISE` clause, the `SELECT` instruction simply acts as a `NOP` (null operation) and does nothing.

## Looping with DO

The `DO` instruction also controls iteration. There are three common forms:

- Simple `DO … END` (grouping only; runs once)
- Counted `DO` (with a control variable and optional `TO`/`BY`/`FOR`)
- Conditional `DO WHILE` / `DO UNTIL`

### Simple DO (grouping only)
```rexx <!--simpledo.rexx-->
options levelb
if cond then do
  say "A"
  say "B"
end
```
This does not loop; it groups multiple instructions into one block.

### Counted DO
```rexx <!--countedo.rexx-->
options levelb
sum = .int
/* i counts from 1 to 5 by 1 */
do i = 1 to 5
  sum = sum + i
end
say sum  /* 15 */
```

Syntax:
```rexx <!--dosyntax.rexx-->
DO i = expr_start [ TO expr_stop ] [ BY expr_step ] [ FOR expr_count ]
  ...
END
```
- `TO` sets the inclusive upper bound (when present).
- `BY` sets the step (defaults to 1 if omitted; negative steps count down).
- `FOR` limits the number of iterations (optional), stopping earlier even if `TO` would allow more.

### Conditional DO

```rexx <!--conditionaldo.rexx-->
options levelb
/* while-loop */
do while i < 10
  i = i + 1
end

/* until-loop */
do until ready
  call work
end
```
- `DO WHILE e` executes the body as long as `e` is true.
- `DO UNTIL e` executes until `e` becomes true (i.e., repeats while `\e`).

### LEAVE and ITERATE
- `ITERATE` restarts the current loop at the next iteration (optionally naming a control variable to target an outer loop).
- `LEAVE` exits the current loop immediately (optionally naming a control variable to target an outer loop).

### Block Expressions

`DO ... END` can also appear inside an expression. In that form it behaves like a local statement block that yields a value back to the parent expression.

```rexx
options levelb
total = 10 + do
  a = 5
  if a > 0 then leave with a * 2
  leave with 0
end
say total   /* 20 */
```

Rules:

- A block expression introduces a local block scope, just like a grouped `DO`.
- The block result is produced with `LEAVE WITH expression`.
- Every reachable value-producing path should end in `LEAVE WITH ...`.
- If more than one `LEAVE WITH` is present, all yielded values must have the same type.
- Plain `LEAVE` keeps its loop meaning. Inside an expression block, use `LEAVE WITH` when you intend to return a value.

Because block expressions are ordinary expressions, they can be nested and used anywhere a primary expression is valid, such as arithmetic, comparisons, function arguments, and short-circuit boolean expressions.

### Variable shadowing and Scoping (Level B)

\crexx{} Level B introduces block-level scoping for certain control structures. It is important to distinguish between single-instruction branches and grouped instructions (DO blocks).

- **Single-instruction branches**: These execute in the current (procedure) scope.

```rexx
  if condition then x = 10 /* x is in procedure scope */
```

- **Grouped instructions (DO blocks)**: These create a `SCOPE_LOCAL` (block scope).

- **Variable Hoisting (Untyped)**: Untyped assignments in subscopes (e.g. `if condition then do; x = 1; end`) bind to the procedure-level variable. If the variable has not been used earlier in the source, it is automatically "hoisted" and created in the procedure scope, making it visible to the rest of the routine.

- **Variable Shadowing (Typed)**: A typed declaration (`x = .int`) inside a `DO` block always creates a block-local that shadows any outer variable (or constant) of the same name for the duration of the block.

- **Temporal Resolution**: An untyped use in a subscope only binds to a parent variable if that variable has been assigned or used **earlier in the source code**. If the parent variable only appears later in the source, the subscope will create its own local definition and the compiler may issue a `#SHADOWING` warning if the names conflict.

This difference in behavior between single-instruction branches and `DO` blocks is an intentional architectural design choice in Level B to balance REXX compatibility with modern structured scoping.

- A typed declaration inside a `DO` block always creates a block-local that shadows any outer variable (or constant) of the same name for the duration of the block:

```rexx <!--shadowingdo.rexx-->
  options levelb
  main: procedure
    x = .int; x = 1
    do
      x = .int  /* block-local, shadows outer x */
      x = 9
    end
    say x  /* 1 */
```

- An untyped assignment inside a `DO` block uses an existing variable if one was used earlier in the source; otherwise it creates a new procedure-scoped variable (hoisting):

```rexx <!--newscopedo.rexx-->
  options levelb
  main: procedure
    do
      y = 3  /* no prior y => hoisted to procedure scope */
    end
    say y    /* 3 - y is still in scope */
```

- Counted `DO` header: the control variable behaves the same way — if an outer variable with that name exists, it is reused; otherwise a loop-local control variable is created. However, you can explicitly force the creation of a block-local control variable that shadows the parent scope by using a typed constructor initializer:

```rexx <!--counteddoheader.rexx-->
  options levelb
  main: procedure
    i = .int; i = 100
    do i = 1 to 3   /* reuses outer i */
      /* ... */
    end
    /* i is 100 here (outer one modified) */

    do j = 1 to 3   /* creates loop-local j */
      /* ... */
    end
    /* j is not visible here */

    do i = .int(1) to 3 /* explicit shadowing loop-local */
      /* ... */
    end
    /* i is still 100 here (outer one wasn't modified) */
```

Note: This syntax candy (`do i = .int(1) to 3`) desugars into a wrapped block (`do; i = .int; do i = 1 to 3; ...; end; end`). It is only supported for fundamental types (`.int`, `.string`, `.float`, `.boolean`, `.decimal`). Attempting to instantiate a non-fundamental class in a loop initialization (e.g. `do i = a_class(5) to 10`) will result in a `#LOOP_CLASSES_NOT_SUPPORTED` compilation error.

Rationale: typed declarations always define intent and should introduce a new local; untyped uses favor existing bindings to reduce surprises, while remaining safe by creating a loop-local when none exists.
