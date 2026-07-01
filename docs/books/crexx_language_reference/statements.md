# Statements {#statements}

## ADDRESS

`ADDRESS` sends commands or function requests to a named external environment.
It is implemented through the current compiler-exit and VM environment protocol.
If no environment is selected, the default command environment is `CREXX`.

Basic command form:

```rexx
address crexx "echo hello"
```

Command output and error streams can be captured:

```rexx
address crexx "echo #42" output out error err
say out
```

### Built-In Command Environments

The built-in environments are deliberately separate:

| Environment | Purpose |
| --- | --- |
| `CREXX` | The default cREXX command environment. It is cREXX-specific and OS-independent, implemented by cREXX rather than by a shell. |
| `SYSTEM` | The platform command processor. On POSIX this is standard `sh -c`; on Windows this is `%COMSPEC% /D /S /C` with a `cmd.exe` fallback. |
| `COMMAND`, `CMD` | Compatibility aliases for `SYSTEM`. |
| `PATH` | Direct executable dispatch through the platform process API. It resolves executables through process `PATH` where needed and calls them without shell syntax. |
| `SHELL` | Explicit configured shell dispatch. Set `CREXX_ADDRESS_SHELL` to the shell executable and optionally `CREXX_ADDRESS_SHELL_ARGS` to the argument list used before the command text. If unset, it falls back to the platform command processor defaults. |

```rexx
address system "echo one && echo two" output out error err
address cmd "cd ."
address path "rxas -h" output out error err
```

### CREXX Command Environment

`CREXX` is not a shell. It is a cRexx -specific command environment with stable
command names and cREXX-defined return-code behavior across supported operating
systems. It does not interpret shell punctuation such as `;`, `&&`, `||`, or
pipes. Use multiple `ADDRESS` statements, or send newline-separated commands to
`ADDRESS CREXX "batch"`. Blank batch lines and lines whose first non-blank
characters are `--` are skipped; batch stops at the first non-zero return code.

`cd`, `pushd`, and `popd` change the cRexx process working directory and
therefore persist for later `ADDRESS CREXX`, `ADDRESS PATH`, `ADDRESS SYSTEM`,
file IO, and relative-path operations in the same process. In contrast,
`ADDRESS SYSTEM "cd path"` runs inside the child command processor and does not
change cREXX's working directory after that child exits.

The command set is intentionally useful but bounded. cRexx command names are
literal. Host-variable anchors are supported only in command operands. In the
table below, `anchorable` means an operand may be a scalar anchor such as
`:name` or `${name}`. An operand ending in `...` may also be supplied by a stem
anchor such as `:name[]`, `:name.`, `${name[]}`, or `${name.}`, which expands
to zero or more operands. For fixed-arity commands, a stem anchor is valid only
when it expands to exactly the number of operands required at that point.

| Command | Anchorable operands | Behavior |
| --- | --- | --- |
| `help` | None. | Print the command list. |
| `echo [text...]` | `text...` may use scalar or stem anchors. | Write text followed by a newline. |
| `pwd` | None. | Print the current cRexx process working directory. |
| `cd [path]` | `path` may use a scalar anchor, or a one-item stem anchor. | Change the cREXX process working directory; no path means the user's home directory where known. |
| `pushd path` | `path` may use a scalar anchor, or a one-item stem anchor. | Push the current directory and change to `path`. |
| `popd` | None. | Return to the most recent pushed directory. |
| `ls [path...]`, `dir [path...]` | `path...` may use scalar or stem anchors. | List directory entries, excluding `.` and `..`. |
| `exists path...` | `path...` may use scalar or stem anchors. | Print `1 path` or `0 path` for each path; returns non-zero if any are missing. |
| `stat path...` | `path...` may use scalar or stem anchors. | Print type, size, and modification time for each path. |
| `mkdir [-p] path...` | `-p` may be literal or scalar-anchored; `path...` may use scalar or stem anchors. | Create directories; `-p` creates missing parents. |
| `rmdir path...` | `path...` may use scalar or stem anchors. | Remove empty directories. |
| `rm [-r] path...`, `del [-r] path...` | `-r` may be literal or scalar-anchored; `path...` may use scalar or stem anchors. | Remove files, or recursively remove paths with `-r`. |
| `copy source target`, `cp source target` | `source` and `target` may use scalar anchors, or one stem anchor that expands to both operands. | Copy a file. |
| `move source target`, `mv source target`, `rename source target` | `source` and `target` may use scalar anchors, or one stem anchor that expands to both operands. | Rename or move a path. |
| `touch path...` | `path...` may use scalar or stem anchors. | Create files if missing and update modification times. |
| `cat path...`, `type path...` | `path...` may use scalar or stem anchors. | Write file contents to the command output stream. |
| `head [-n count] path` | `count` and `path` may use scalar anchors; a stem anchor must expand to the exact option/value/path shape. | Write the first lines of a file; default count is 10. |
| `tail [-n count] path` | `count` and `path` may use scalar anchors; a stem anchor must expand to the exact option/value/path shape. | Write the last lines of a file; default count is 10. |
| `lines [path]` | `path` may use a scalar anchor, or a one-item stem anchor. | Count lines in a file, or in redirected command input when no path is supplied. |
| `write path text...` | `path` may use a scalar anchor; `text...` may use scalar or stem anchors. | Replace a file with the supplied text. |
| `append path text...` | `path` may use a scalar anchor; `text...` may use scalar or stem anchors. | Append the supplied text to a file. |
| `which command` | `command` may use a scalar anchor, or a one-item stem anchor. | Resolve an executable through process `PATH`. |
| `now [local\|utc]`, `date [local\|utc]` | `local`/`utc` may be literal or scalar-anchored. | Print an ISO-like timestamp. |
| `sleep seconds` | `seconds` may use a scalar anchor. | Sleep for the requested duration. |
| `platform`, `os` | None. | Print operating-system and architecture details. |
| `env [name...]` | `name...` may use scalar or stem anchors. | Print all environment variables, one variable's value, or `name=value` lines for multiple names. |
| `setenv name value...` | `name` may use a scalar anchor, or a stem anchor whose first item is the name. `value...` may use scalar or stem anchors and is joined with spaces. | Set a process environment variable. |
| `unsetenv name...` | `name...` may use scalar or stem anchors. | Clear one or more process environment variables. |
| `pid` | None. | Print the current cREXX process id. |
| `ps [pid]` | `pid` may use a scalar anchor. | Print current process details, or check whether a process id is alive. |
| `kill pid [signal]` | `pid` and `signal` may use scalar anchors. | Terminate or signal a process. |
| `resolve host` | `host` may use a scalar anchor. | Resolve host names to numeric addresses. |
| `tcp host port` | `host` and `port` may use scalar anchors, or one stem anchor that expands to both operands. | Check that a TCP connection can be opened. |
| `batch` | None in the `batch` command itself; input lines are runtime text and are not compiler auto-exposed. | Read commands from input and execute them in order. |
| `run executable [arg...]` | `executable` may be literal or scalar-anchored. `arg...` may use scalar or stem anchors. `run :argv[]` is also accepted: the first stem item is the executable and the remaining items are arguments. The word `run` itself must be literal. | Execute a direct `PATH` command and forward its output and error streams. |

Anchors must occupy a whole parsed operand. For example, `cat :file` expands
`:file`, but `--file=:file` is a literal operand. Build combined operands in
Rexx first, then pass the result with a scalar anchor. Stem anchors preserve
each item as one operand, even when an item contains spaces or shell punctuation
such as `&&`.

## ARG

See the [Procedures and Arguments](procedures-and-arguments) section on page \pageref{procedures-and-arguments}.

## CALL

CALL routine \[ parameter \] \[, \[ parameter \] ... \] 

## CONSTANT

Level B supports named compile-time constants:

```rexx
constant FLAG_STRING = 0x00010000
constant PAYLOAD = "41424344"x as .binary
```

The initializer must be a compile-time constant expression. The declared name
is immutable after declaration and can be used in ordinary expressions or as an
inline assembler literal operand. Integer constants used as assembler immediates
are substituted directly; string, decimal, float, and binary payload constants
are stored through the normal RXBIN constant-pool path.

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

The NOP instruction is the *No Operation* directive; it executes without performing any operation. It is syntactically valid but intentionally does nothing. It exists primarily as a placeholder, in the following cases:

- as a placeholder while developing or debugging;
- in generated code where an empty statement is needed;
- to make an intentionally empty branch explicit rather than accidental

This example shows how a branch can be temporarily disabled while developing by inserting a `nop` statement:

```rexx <!--nopexample.crexx-->
if retries > 0 then
    nop               /* retry logic temporarily disabled */
else
    call abortTransfer
```

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

Evaluates the expression expr and prints the resulting string onto the standard output stream.[^newline]

[^newline]: with an added newline. For cases where no newline is wanted, the conventional way of using `call lineout` can be used, or the `sayx` assembler instruction.

## MSAY

MSAY mask, value-1, value-2, ...

Example:

```rexx <!--msayexample.crexx-->
options levelb comments_dash numeric_classic
import rxfnsb
numeric digits 15
b=.decimal;c=.decimal;p=.decimal
n=.decimal;mn=.decimal
b = 963807195502/100
c = 180945931154/100
p = 0.081090
n = 30
-- COMPUTE MN = (1 + P) ** N
mn = (1+p) ** n
-- COMPUTE B = (B + C) * MN + C * ( (MN - 1) / P - 1) .
b = (b+c) * mn + c * (( mn - 1) / p - 1)
msay "$$$,$$$,$$$,$$$,$$9.99.",b  
```

<!--splice--crexx msayexamplenep-->

`MSAY` is a convenience syntax that formats values using `fmtmask` and immediately outputs the resulting line. The facility is intended for report-style output where fixed-width text and numeric alignment are desirable. See [MSAY and fmtmask](msay-and-fmtmask) on page \pageref{msay-and-fmtmask} for the complete template syntax.

## FSAY

`FSAY` is a convenience syntax that formats the template using `fsayfmt()` and
outputs the resulting line.

Unlike `fmtmask` and `MSAY`, which use COBOL-inspired picture masks,
`fsayfmt` uses embedded placeholders similar to modern string interpolation
systems.


```rexx
FSAY template
```

The template may contain one or more placeholders enclosed in braces.

Example:

```rexx <!--fsayexample.crexx-->
name  = "Fred"
qty   = 12
price = 64.31

FSAY "Name: {name:<10} Qty: {qty:>3} Price: {price:8.2}"
```

<!--splice--crexx fsayexample-->

See [fsay and fsayfmt](fsay-and-fsayfmt) on page \pageref{fsay-and-fsayfmt} for the complete template syntax.

## SELECT/WHEN/OTHERWISE

SELECT [expression] [;]
  WHEN expression [, expression ...] [;] THEN [;] instruction [;]
  [WHEN expression [, expression ...] [;] THEN [;] instruction [;]]
  ...
  [OTHERWISE [;] [instruction] [;] ...]
END [;]

The SELECT statement allows you to conditionally evaluate multiple expressions and execute corresponding instructions based on the first expression that evaluates to true (1).

There are two styles of the SELECT statement in cRexx:

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
trace suppress namespace name
trace unsuppress namespace name
trace add suppressed namespace name
trace remove suppressed namespace name
trace reset namespaces
trace results to stderr
trace llm to file "trace.jsonl"
```

The standard Rexx option letters `A`, `C`, `E`, `F`, `I`, `L`, `N`, `O`, and
`R` are accepted, including a leading `?` prefix and signed integer settings.
Options use a minimum-abbreviation rule: the spelling must be a left-prefix of
the full option word. For example, `TRACE R`, `TRACE RE`, `TRACE RES`,
`TRACE RESULT`, and `TRACE RESULTS` all select Results, while `TRACE RAS` is
invalid. The cRexx extensions use `AS` as the minimum abbreviation for `ASM`
and `LL` as the minimum abbreviation for `LLM`; `ENV` is an exact cRexx
extension spelling. `TRACE REXX` remains supported as an exact legacy cRexx
source-trace spelling; it is not abbreviated because `R` and `RE...` belong to
Results.
This is not yet full semantic compatibility, but the noninteractive output
shape follows the standard prefix vocabulary for implemented events:

```
       >  >   escaped-source-file
     5 *-* escaped-source
       >=>   "escaped-assignment-result"
       +++   RC=-3 ENVIRONMENT escaped-command
```

`TRACE N` is the quiet/default mode: it does not trace ordinary statements and
emits `+++` only for failing ADDRESS commands. `TRACE C`, `TRACE E`, and
`TRACE F` are ADDRESS-command driven. `TRACE A` traces source clauses.
Classic `TRACE R` traces source clauses, variable substitutions, and assignment
or expression results. cRexx emits source clauses from `.srcstep` metadata and
semantic value records from `.traceevent` metadata, including initial `>=>`
assignment, `>V>` variable, `>L>` literal, and `>O>`/`>P>` operation coverage
where the compiler can point at an available register or constant. `TRACE I`
uses the same metadata path and adds intermediate-event visibility as coverage
grows. `TRACE L` is accepted, but label-pass events are not emitted yet.
`O`/`OFF` disables breakpoint tracing. `TRACE ASM` traces VM/RXAS instruction
information and includes source text where metadata is available.

When traced execution moves between source files, cRexx emits a source-file
transition line:

```bash
       >  >   helper.crexx
```

The first visible source file is not printed, so single-file traces keep their
classic shape. Later file changes are printed before the next `*-*` source
line, including when execution returns to the original file. This is a cRexx
extension; classic Regina-style output does not provide an equivalent filename
record.

This TRACE implementation is still beta. Source reporting now uses
self-contained source-step metadata, and text TRACE no longer guesses
assignment results from source text. Result coverage is still deliberately
partial: optimized-away or folded values may have no trace event, and some
compound-variable details such as final resolved-name reporting remain a
compiler/ runtime coverage task.

`TRACE LLM` is a cRexx extension that emits one JSON-lines-style trace record
per event. It is intended for debugger automation and for validating emitted
`.srcstep` metadata; source text is escaped so control characters and
backslashes remain visible. `TRACE VALUE expr` evaluates `expr` at runtime and
normalizes it using the same trace option rules.

Trace output defaults to stdout. Add `TO STDERR`, `TO STDOUT`, `TO FILE expr`,
or `TO expr` to choose a sink. `TO FILE` opens the selected file in append mode
for each trace record.

TRACE normally hides events from system library and debugger namespaces so a
user trace follows the program being debugged instead of the machinery that
implements tracing. The default suppressed namespaces are `rxfnsb`, `_rxsysb`,
`rxfnsg`, `_rxsysg`, `rxfnsl`, `_rxsysl`, `rxfnsc`, `_rxsysc`, `rxcp`,
`rxcpexits`, `rxcptest`, `rxdb`, `rxdbgui`, `runtime_signal`, `signalaction`,
and `library`. The TRACE runtime internals themselves are always hidden.

Use namespace controls when you need to include or exclude a library while
debugging:

```rexx
trace results
trace unsuppress namespace rxfnsg
trace suppress namespace myframework
trace reset namespaces
```

`TRACE SUPPRESS NAMESPACE name` and `TRACE ADD SUPPRESSED NAMESPACE name`
suppress a namespace. `TRACE UNSUPPRESS NAMESPACE name` and
`TRACE REMOVE SUPPRESSED NAMESPACE name` make that namespace visible again.
`TRACE RESET NAMESPACES` restores the default suppression list and clears
per-session changes. Namespace names may be bare identifiers or string
literals; matching is by namespace or path component, not by arbitrary
substring, so suppressing `rxfnsg` does not suppress `myrxfnsghelper`.

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

The cRexx standard-library/BIF build deliberately compiles most built-in function source files with compiler exits disabled (`rxc -x`) to avoid
bootstrap and circular-dependency problems while building the library that the
exits themselves use. Adding `TRACE RESULTS`, `TRACE R`, or another explicit
TRACE instruction directly to a BIF source file such as `abs.crexx` therefore
produces `#CERTIFIED_EXIT_DISABLED`. Debug BIF or library behavior from a
normal test program instead: call the BIF from a fixture that compiles with
exits enabled, use `TRACE UNSUPPRESS NAMESPACE rxfnsb` if you need to see
library frames, and keep linked/native images unstripped with
`--link-keep-source` when source-level TRACE metadata is needed.

Implementation status and compatibility requirements are tracked in  
`docs/ai-context/CREXX_TRACE_REQUIREMENTS.md`.
