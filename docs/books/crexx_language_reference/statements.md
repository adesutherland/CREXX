# Statements {#statements}

## ADDRESS

ADDRESS … 

The ADDRESS instruction is a directive that enables the transmission of commands to an external environment.

## ARG

See Procedures and Arguments Section

## CALL

CALL routine \[ parameter \] \[, \[ parameter \] ... \] 

## DO/END

DO \[ repetitor \] \[ conditional \] ; \[ clauses \]

expr ::= DO ; \[ clauses \] END

END \[ symbol \] ;

repetitor : \= symbol \= expri \[ TO exprt \] \[ BY exprb \] \[ FOR exprf \]

conditional : \= WHILE exprw UNTIL expru

The DO/END statement is the command employed to iterate and group multiple statements into a singular block. This instruction consists of multiple clauses.

When `DO ... END` appears where an expression is expected, it is parsed as a block expression. In that form the body must yield a value using `LEAVE WITH expr`.

## EXIT

EXIT \[ expr \] ;

Causes the Rexx program to cease execution and, optionally, returns the expression expr to the calling program.

## IF/THEN/ELSE

IF expr \[;\] THEN \[;\] statement

\[ ELSE \[;\] statement \]

This provides the standard conditional statement structure.

## ITERATE

ITERATE \[ symbol \] ;

The ITERATE instruction will execute the innermost, active loop in which the ITERATE instruction is situated repeatedly. If a symbol is specified, it will execute the innermost, active loop having the symbol as the control variable repeatedly.

## LEAVE

LEAVE \[ symbol \] ;

LEAVE WITH expr ;

This statement terminates the innermost, active loop. If symbol is specified, it terminates the innermost, active loop having symbol as control variable. 

`LEAVE WITH expr` is distinct from loop-control `LEAVE`. It exits the innermost enclosing expression-form `DO ... END` block and returns the value of `expr` to the parent expression.

## NOP

NOP ;

The NOP instruction is the "null operation" directive; it executes without performing any operation.

## OPTIONS

OPTIONS expr ;

The OPTIONS instruction is used to set various interpreter-specific options. See Language Level and Options Section

## PARSE

PARSE \[ option \] \[ CASELESS \] type \[ template \] ;

*CURRENT STATUS: not implemented*

## PROCEDURE

See Procedures and Arguments Section.

## SAY

SAY \[ expr \] ;

Evaluates the expression expr and prints the resulting string onto the standard output stream.

## SELECT/WHEN/OTHERWISE

SELECT [expression] [;]
  WHEN expression [, expression ...] [;] THEN [;] instruction [;]
  [WHEN expression [, expression ...] [;] THEN [;] instruction [;]]
  ...
  [OTHERWISE [;] [instruction] [;] ...]
END [;]

The SELECT statement allows you to conditionally evaluate multiple expressions and execute corresponding instructions based on the first expression that evaluates to true (1).

There are two styles of the SELECT statement in cREXX:
1. **Classic SELECT:** Does not include an initial `expression` after the `SELECT` keyword. Each `WHEN` expression is evaluated as a standalone boolean condition.
2. **C-Style SELECT (SWITCH):** Includes an initial `expression` after the `SELECT` keyword. The `expression` is evaluated once, and its result is implicitly compared for equality (`=`) against each `WHEN` expression.

If a `WHEN` condition is met, its associated `THEN` instruction is executed, and control exits the `SELECT` block. If no `WHEN` condition is met, the `OTHERWISE` block (if present) is executed. If no `WHEN` condition is met and an `OTHERWISE` block is absent, the `SELECT` statement acts as a `NOP` (null operation) and does nothing.

## TRACE

CURRENT STATUS: not implemented (Debugging Approach TBC)

# Procedures and Arguments

## Function Arguments

Arguments can be passed to a procedure by reference or by value. When an argument is passed by reference, the procedure can modify the original variable that was passed to it. When an argument is passed by value, a copy of the variable is passed to the procedure, and any changes made to the copy do not affect the original variable.

The user-visible rules are:

* Plain `ARG name = type` is pass by value.
* `ARG expose name = type` is pass by reference.
* Pass-by-value semantics are defined by caller visibility, not by the VM calling convention. If the callee writes to a by-value formal, the caller must still observe its original value after the call.
* This applies equally to simple values, arrays, and class/object references. Rebinding or mutating a by-value formal must not leak back to the caller's variable.
* The compiler is allowed to optimise away an internal defensive copy only when that cannot change caller-visible behaviour, for example when the formal is provably read-only or when the actual value is a temporary expression that has no caller-side symbol to preserve.
* If the caller wants the callee to update the original variable, the parameter must be declared with `expose`.

By example:

ARG a1 \= 0, a2 \= .int, expose a3 \= .aclass, ?a4 \= .aclass, a5 \= .string\[\]

* Arg a1 is an optional integer (and 0 if not specified in the call)  
* Arg a2 is a mandatory integer (pass by value)  
* Arg a3 is a mandatory class aclass pass by reference  
* Arg a4 is a optional class aclass pass by value, value from the default factory if not specified in the call  
* Arg a5 is an array of strings and is one way to allow an arbitrary number of strings to be passed to the procedure (see also Ellipsis later)

Examples:

```rexx
bump: procedure = .int
  arg value = .int
  value = value + 1
  return value

x = 10
say bump(x)
say x           /* still 10 */
```

```rexx
bumpref: procedure = .void
  arg expose value = .int
  value = value + 1
  return

x = 10
call bumpref(x)
say x           /* now 11 */
```

## Ellipsis (...)

The last arguments declaration can be an ellipsis ('...'), this is used to show that 0 or more arguments can be provided. For example:

ARG a1 \= 0, a2 \= .int, ... \= .string

* The '...' shows that an arbitrary number of .string arguments can be added to the end of the call.  
* The ? operator exist to access & query arguments:  
* ?a1 returns true if the optional arg a1 was specified.  
* ?a2 will always be true as a2 is not optional.

Pseudo Array arg allows access to the '...' arguments. Also see the Arrays section.

* arg\[1\] or arg.1 gives the first '...' argument. These can signal OUTOFRANGE  
* arg\[0\], arg\[\], arg.0 or arg. return the number of '...' arguments

The type of this Pseudo is the type of the '...' argument

## arg() Operator

The compatibility arg() operator is designed to provide some compatibility with classic REXX; by example:

* arg() is equivalent to arg.0 etc. Type Integer.  
* arg(1) is equivalent to arg.1 etc. The type of this operator is the same as the '...' argument and like arg.1 can signal OUTOFRANGE  
* arg(4,E), arg(4,"E"), arg(4,Exxx), arg(4,"Exxx") etc. all return 1 (true) if there were 4 or more '...' arguments given or 0 (false) otherwise. E is Exists.  
* Likewise arg(4,'O') etc. (O is Omitted) is equivalent to \~arg(4,'E').

## Implicit Main Procedure

In the event that a module file contains instructions preceding a PROCEDURE instruction, an implicit procedure named main() is automatically generated within the namespace of the module file. The arguments for this procedure can be accessed through the pseudo array arg or arg() operator. The return type of the implicitly defined main() procedure is automatically set to either int or void.
