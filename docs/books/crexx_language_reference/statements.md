# Statements {#statements}

## **ADDRESS** {#address}

ADDRESS … 

The ADDRESS instruction is a directive that enables the transmission of commands to an external environment.

## 

## **ARG**  {#arg}

See Procedures and Arguments Section

## 

## **CALL** {#call}

CALL routine \[ parameter \] \[, \[ parameter \] ... \] 

## 

## **DO/END** {#do/end}

DO \[ repetitor \] \[ conditional \] ; \[ clauses \]

END \[ symbol \] ;

repetitor : \= symbol \= expri \[ TO exprt \] \[ BY exprb \] \[ FOR exprf \]

conditional : \= WHILE exprw UNTIL expru

The DO/END statement is the command employed to iterate and group multiple statements into a singular block. This instruction consists of multiple clauses.

## 

## **EXIT**  {#exit}

EXIT \[ expr \] ;

Causes the Rexx program to cease execution and, optionally, returns the expression expr to the calling program.

## 

## **IF/THEN/ELSE** {#if/then/else}

IF expr \[;\] THEN \[;\] statement

\[ ELSE \[;\] statement \]

This provides the standard conditional statement structure.

## 

## **ITERATE** {#iterate}

ITERATE \[ symbol \] ;

The ITERATE instruction will execute the innermost, active loop in which the ITERATE instruction is situated repeatedly. If a symbol is specified, it will execute the innermost, active loop having the symbol as the control variable repeatedly.

## 

## **LEAVE** {#leave}

LEAVE \[ symbol \] ;

This statement terminates the innermost, active loop. If symbol is specified, it terminates the innermost, active loop having symbol as control variable. 

## 

## **NOP** {#nop}

NOP ;

The NOP instruction is the "null operation" directive; it executes without performing any operation.

## 

## **OPTIONS** {#options}

OPTIONS expr ;

The OPTIONS instruction is used to set various interpreter-specific options. See Language Level and Options Section

## 

## **PARSE** {#parse}

PARSE \[ option \] \[ CASELESS \] type \[ template \] ;

*CURRENT STATUS: not implemented*

## 

## **PROCEDURE** {#procedure}

See Procedures and Arguments Section.

## 

## **SAY** {#say}

SAY \[ expr \] ;

Evaluates the expression expr and prints the resulting string onto the standard output stream.

## 

## **SELECT/WHEN/OTHERWISE** {#select/when/otherwise}

*CURRENT STATUS: not implemented*

## 

## **TRACE** {#trace}

CURRENT STATUS: not implemented (Debugging Approach TBC)

# Procedures and Arguments {#procedures-and-arguments}

## **Function Arguments** {#function-arguments}

Arguments can be passed to a procedure by reference or by value. When an argument is passed by reference, the procedure can modify the original variable that was passed to it. When an argument is passed by value, a copy of the variable is passed to the procedure, and any changes made to the copy do not affect the original variable.

By example:

ARG a1 \= 0, a2 \= .int, expose a3 \= .aclass, ?a4 \= .aclass, a5 \= .string\[\]

* Arg a1 is an optional integer (and 0 if not specified in the call)  
* Arg a2 is a mandatory integer (pass by value)  
* Arg a3 is a mandatory class aclass pass by reference  
* Arg a4 is a optional class aclass pass by value, value from the default factory if not specified in the call  
* Arg a5 is an array of strings and is one way to allow an arbitrary number of strings to be passed to the procedure (see also Ellipsis later)

## **Ellipsis (...)** {#ellipsis-(...)}

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

## **arg() Operator** {#arg()-operator}

The compatibility arg() operator is designed to provide some compatibility with classic REXX; by example:

* arg() is equivalent to arg.0 etc. Type Integer.  
* arg(1) is equivalent to arg.1 etc. The type of this operator is the same as the '...' argument and like arg.1 can signal OUTOFRANGE  
* arg(4,E), arg(4,"E"), arg(4,Exxx), arg(4,"Exxx") etc. all return 1 (true) if there were 4 or more '...' arguments given or 0 (false) otherwise. E is Exists.  
* Likewise arg(4,'O') etc. (O is Omitted) is equivalent to \~arg(4,'E').

## **Implicit Main Procedure** {#implicit-main-procedure}

In the event that a module file contains instructions preceding a PROCEDURE instruction, an implicit procedure named main() is automatically generated within the namespace of the module file. The arguments for this procedure can be accessed through the pseudo array arg or arg() operator. The return type of the implicitly defined main() procedure is automatically set to either int or void.

