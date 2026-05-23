# Statements {#statements}

## ADDRESS

`ADDRESS` sends commands or function requests to a named external environment.
It is implemented through the current compiler-exit and VM environment protocol.

Basic command form:

```rexx
address system "echo hello"
```

Command output and error streams can be captured:

```rexx
address command "echo #42" output out error err
say out
```

ADDRESS host-variable anchors such as `:name` and `${name}` are compiler
auto-expose syntax. Their command meaning belongs to the selected environment
handler; the VM carries binding values and write-back updates.

The current native registration API is environment based:

```c
rxvml_address_register_callback_environment(ctx, name, id,
    command_cb, function_cb, userdata);
```

The old command-only callback registration form is retired.

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

Simple `DO ... END` groups may also carry block-scoped signal handlers using
`ON SIGNAL` clauses. The handler clauses are valid only on a simple `DO` group,
not on counted, conditional, forever, or expression-form `DO`. To protect code
inside a loop, nest a simple signal-handling `DO ... END` group inside the loop
body.

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

Current implementation status:

* `PARSE VALUE ...`, `PARSE VAR ...`, and `PARSE ARG ...` are implemented through the certified `PARSE` exit.
* `PARSE ARG` uses the current procedure's `arg()` compatibility view.
* In implicit `main`, that means command-line arguments.
* In other procedures, that means the `...` tail if present, or an empty source string if there is no `...` tail.

## PROCEDURE

`PROCEDURE` starts a named local routine and optionally declares its return
type and the module-global variables that routine can see.

Common forms:

```rexx
name: procedure
name: procedure = .int
name: procedure = .void expose state count
```

Procedure-level `expose` is local to that procedure declaration. The listed
names are bound to module-global storage shared with other procedures that
also expose the same names. A procedure that does not list the name does not
see that exposed storage unless the name is also exposed by the file-level
`namespace ... expose ...` declaration.

```rexx
proc1: procedure = .void expose var
  var = "Hello World"
  return

proc2: procedure = .void expose var
  say "var is" var
  return
```

This is distinct from `ARG expose`, which exposes a call argument by reference.

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

## SIGNAL

Signals are Level B error/condition objects implementing `.signal`.
Rexx-created signals can be raised with a signal object or with the compact
named forms:

```rexx
signal .signal("error", "message")
signal error
signal error "message"
```

Procedure-scoped handlers are installed with `SIGNAL ON` and removed with
`SIGNAL OFF`:

```rexx
signal on conversion_error call handle_conversion
signal on error, syntax call handle_problem
signal off conversion_error

handle_conversion: procedure = .signalaction
  arg problem = .signal
  say problem.source()
  return .signalaction.skip()
```

The handler procedure receives one `.signal` argument and returns a
`.signalaction`: `.signalaction.skip()`, `.signalaction.retry()`, or
`.signalaction.fail()`.

Block-scoped handlers use `ON SIGNAL` clauses on a simple `DO ... END` group:

```rexx
do
  risky_work()
on signal conversion_error as problem
  say problem.source()
on signal error, syntax
  call cleanup()
on signal
  call log_unhandled_signal()
end
```

The statements before the first `ON SIGNAL` clause are the protected body.
Normal completion skips the handlers. A handler that completes normally leaves
the `DO` block. `ON SIGNAL` with no names catches all maskable signals.
`AS name` binds the current `.signal` object; if `AS` is omitted, no signal
object is available to that handler.

Only a simple `DO ... END` group can carry `ON SIGNAL` clauses. Counted,
conditional, forever, and expression-form `DO` loops do not carry handlers
directly. To protect code inside a loop, put a simple signal-handling
`DO ... END` group inside the loop body.

## TRACE

`TRACE` enables or disables VM breakpoint-backed tracing for the current call
frame and procedures called from it.

Supported forms are:

```rexx
trace off
trace normal
trace results
trace rexx
trace asm
trace as
trace llm
trace ll
trace env
trace value expr
trace results to stderr
trace llm to file "trace.jsonl"
```

The standard Rexx option letters `A`, `C`, `E`, `F`, `I`, `L`, `N`, `O`, and
`R` are accepted, including a leading `?` prefix and signed integer settings.
Options use a minimum-abbreviation rule: the spelling must be a left-prefix of
the full option word. For example, `TRACE R`, `TRACE RE`, `TRACE RES`,
`TRACE RESULT`, and `TRACE RESULTS` all select Results, while `TRACE RAS` is
invalid. The cREXX extensions use `AS` as the minimum abbreviation for `ASM`
and `LL` as the minimum abbreviation for `LLM`; `ENV` is an exact cREXX
extension spelling. `TRACE REXX` remains supported as an exact legacy cREXX
source-trace spelling; it is not abbreviated because `R` and `RE...` belong to
Results.
This is not yet full semantic compatibility, but the noninteractive output
shape follows the standard prefix vocabulary for implemented events:

```text
     5 *-* escaped-source
       >>>   "escaped-result"
       +++   RC=-3 ENVIRONMENT escaped-command
```

`TRACE N` is the quiet/default mode: it does not trace ordinary statements and
emits `+++` only for failing ADDRESS commands. `TRACE C`, `TRACE E`, and
`TRACE F` are ADDRESS-command driven. `TRACE A` traces source clauses.
`TRACE R` traces source clauses and currently emits `>>>` for simple assignment
results where compiler metadata identifies the target register. `TRACE I` is
accepted but currently has the same result coverage as `R`; intermediate
`>V>`, `>O>`, and related records are future work. `TRACE L` is accepted, but
label-pass events are not emitted yet. `O`/`OFF` disables breakpoint tracing.
`TRACE ASM` traces VM/RXAS instruction information and includes source text
where metadata is available.

`TRACE LLM` is a cREXX extension that emits one JSON-lines-style trace record
per event. It is intended for debugger automation and for validating emitted
`.src` metadata; source text is escaped so control characters and backslashes
remain visible. `TRACE VALUE expr` evaluates `expr` at runtime and normalizes it
using the same trace option rules.

Trace output defaults to stdout. Add `TO STDERR`, `TO STDOUT`, `TO FILE expr`,
or `TO expr` to choose a sink. `TO FILE` opens the selected file in append mode
for each trace record.

`TRACE ENV` explicitly checks two environment variables at that point in the
program. `CREXX_TRACE` supplies the mode using the same option rules as
`TRACE VALUE`, and `CREXX_TRACE_TO` supplies the sink using the same rules as
`TO`. For example, `CREXX_TRACE=results CREXX_TRACE_TO=stderr` can switch
tracing on at a compiled `TRACE ENV` marker without editing the source. If
`CREXX_TRACE` is unset or empty, `TRACE ENV` turns tracing off. If `CREXX_TRACE`
has an invalid value, `TRACE ENV` turns tracing off and emits a `+++` trace
message naming the invalid value.

TRACE is implemented as a certified compiler exit. It requires normal compiler
exit loading; compiling with exits disabled rejects the statement rather than
treating it as an implicit command.

Implementation status and compatibility requirements are tracked in
`docs/ai-context/CREXX_TRACE_REQUIREMENTS.md`.

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
* In a procedure without a `...` tail, the count forms return `0`

The type of this Pseudo is the type of the '...' argument

## arg() Operator

The compatibility arg() operator is designed to provide some compatibility with classic REXX; by example:

* arg() is equivalent to arg.0 etc. Type Integer.  
* arg(1) is equivalent to arg.1 etc. The type of this operator is the same as the '...' argument and like arg.1 can signal OUTOFRANGE  
* arg(4,E), arg(4,"E"), arg(4,Exxx), arg(4,"Exxx") etc. all return 1 (true) if there were 4 or more '...' arguments given or 0 (false) otherwise. E is Exists.  
* Likewise arg(4,'O') etc. (O is Omitted) is equivalent to \~arg(4,'E').
* In a procedure without a `...` tail, `arg()` returns `0` and the `E`/`O` probe forms operate on that empty tail

## Implicit Main Procedure

In the event that a module file contains instructions preceding a PROCEDURE instruction, an implicit procedure named main() is automatically generated within the namespace of the module file. The arguments for this procedure can be accessed through the pseudo array arg or arg() operator. This implicit main() case is the compatibility bridge that maps classic `arg(n)` access onto command-line arguments when no explicit signature is present. The return type of the implicitly defined main() procedure is automatically set to either int or void.
