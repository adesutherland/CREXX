# Procedures and Arguments

## Procedure-Level Expose

`procedure expose` is the local procedure form for sharing module-global state:

```rexx
main: procedure
  call proc1
  call proc2
  say "but var in main is" var
  return

proc1: procedure = .void expose var
  var = "Hello World"
  return

proc2: procedure = .void expose var
  say "var is" var
  return
```

Here `proc1` and `proc2` share the exposed global `var`. `main` does not list
`var`, so its bare `var` reference is not the same exposed variable. To let
`main` read or write the shared value, declare `main: procedure expose var` as
well.

The `expose` list follows the return type if a return type is present. The
items in the procedure-level list are bare variable names:

```rexx
worker: procedure = .int expose state errors
```

For globally published module variables, prefer file-level
`namespace name expose var`; those namespace-exposed globals auto-bind into
procedures in the same source file.

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

```rexx 
ARG a1 = 0, a2 = .int, expose a3 = .aclass, ?a4 = .aclass, a5 = .string\[\]
```

* Arg a1 is an optional integer (and 0 if not specified in the call)  
* Arg a2 is a mandatory integer (pass by value)  
* Arg a3 is a mandatory class aclass pass by reference  
* Arg a4 is an optional class aclass pass by value; the default expression is the bare typed class value `.aclass`, not a factory call
* Arg a5 is an array of strings and is one way to allow an arbitrary number of strings to be passed to the procedure (see also Ellipsis later)

Optional defaults evaluate exactly as written. Use `?x = .SomeClass` for the
bare typed class value and `?x = .SomeClass()` to call the default factory.

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

```rexx
ARG a1 = 0, a2 = .int, ... = .string
```

* The '...' shows that an arbitrary number of .string arguments can be added to the end of the call.  
* The ? operator exist to access & query arguments:  
* ?a1 returns true if the optional arg a1 was specified.  
* ?a2 will always be true as a2 is not optional.

Pseudo Array arg allows access to the '...' arguments. Also see the Arrays section.

* arg\[1\] or arg.1 gives the first '...' argument. These can signal `OUTOFRANGE`
* arg\[0\], arg\[\], arg.0 or arg. return the number of '...' arguments
* In a procedure without a `...` tail, the count forms return `0`

The type of this Pseudo is the type of the '...' argument

## arg() Operator

The compatibility arg() operator is designed to provide some compatibility with Classic Rexx; by example:

* arg() is equivalent to arg.0 etc. Type Integer.  
* arg(1) is equivalent to arg.1 etc. The type of this operator is the same as the '...' argument and like arg.1 can signal `OUTOFRANGE`  
* arg(4,E), arg(4,"E"), arg(4,Exxx), arg(4,"Exxx") etc. all return 1 (true) if there were 4 or more '...' arguments given or 0 (false) otherwise. E is Exists.  
* Likewise arg(4,'O') etc. (O is Omitted) is equivalent to \~arg(4,'E').
* In a procedure without a `...` tail, `arg()` returns `0` and the `E`/`O` probe forms operate on that empty tail

## Implicit Main Procedure

In the event that a module file contains instructions preceding a `PROCEDURE` instruction, an implicit procedure named main() is automatically generated within the namespace of the module file. The arguments for this procedure can be accessed through the pseudo array arg or arg() operator. This implicit main() case is the compatibility bridge that maps classic `arg(n)` access onto command-line arguments when no explicit signature is present. The return type of the implicitly defined main() procedure is automatically set to either int or void.
