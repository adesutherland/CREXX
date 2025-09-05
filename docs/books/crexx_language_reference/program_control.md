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
- Instructionl only if `expression` is true
- Instructi on2 only if `expression` is false.

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

`OTHERWISE` is essentially the `SELECT`-equivalent of `ELSE`. If there is any
possibility that all the `WHEN` expressions could be *false*, there must be an
`OTHERWISE` clause.
