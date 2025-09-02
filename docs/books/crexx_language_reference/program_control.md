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

reply = "YES"      /* assigns the string "YES"
                      to the variable REPLY */

say reply          /* displays "YES"        */
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
