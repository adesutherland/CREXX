# RexxScript Step 1 Architecture

## Purpose

This document describes the architecture, design goals, and planned evolution of RexxScript.

RexxScript is exposed to CREXX applications through the `REXXSCRIPT` command and provides a lightweight scripting environment for configurable business rules, calculations, decision logic, and generated code execution.

The current implementation is built upon an execution engine that evolved from the original `EVALUATE()` prototype. While that implementation remains important internally, RexxScript should be viewed as the primary architectural concept and user-facing feature.

The intent is evolutionary rather than revolutionary: build on the current working implementation, gain practical experience, and introduce additional capabilities only when there is a demonstrated requirement.

This document focuses on architectural decisions and implementation direction. For command syntax and end-user usage, see the **RexxScript User Guide**.

---

## Current Status

RexxScript is now a working structured execution environment integrated into CREXX through the `REXXSCRIPT` command.

The implementation supports structured control flow, local variable management, intrinsic functions, output capture, and host variable exchange through explicit exposure semantics.

Internally, the runtime evolved from the original `EVALUATE()` prototype, but the primary architectural focus is now RexxScript as a language feature rather than the underlying implementation entry point.

The current implementation serves as both:

```text
a lightweight runtime scripting engine

and

a foundation for future RexxScript evolution
```

---

## Design Philosophy

The current implementation is viewed primarily as a lightweight generated-code execution engine rather than a general-purpose scripting language.

Step 1 emphasizes:

* simplicity
* readability
* predictability
* maintainability
* rapid experimentation

The implementation intentionally favors practical utility over strict language completeness.

RexxScript is primarily intended for configurable business rules, decision logic, calculations, and generated code execution. As a result, language features are introduced based on practical usage rather than compatibility goals.

The implementation intentionally favors intrinsic functions over general function dispatch.

Common utility operations can therefore be introduced with minimal runtime complexity while avoiding the compatibility, security, and maintenance implications of external function dispatch.

This approach allows the language surface to evolve incrementally while preserving a lightweight and predictable execution engine.
--- 

## Step 1 Scope

The current architecture follows a straightforward execution pipeline:

```text
RexxScript Engine
    -> split source into statements
    -> parse statements
    -> execute statements
    -> capture SAY output
    -> export results
```

---

## Implemented Language Features

### Statements

```text
assignment

SAY

IF / THEN
IF / THEN / ELSE

DO / END
DO WHILE
DO UNTIL
DO variable = start TO limit [BY step]

LEAVE
ITERATE

RETURN
```

### Expressions

```text
integer expressions
string expressions

string concatenation

comparisons
arithmetic operators

left-to-right evaluation
```

### Variables

```text
local variables

string-based internal storage

result export
```

### Output

```text
captured SAY output

execution status

flat result export
```

### Intrinsic Functions

Step 1 supports a small but extensible set of intrinsic functions.

Intrinsics are implemented directly by the execution engine and are not general Rexx function calls.

The intrinsic subsystem is intentionally separated from the expression parser.

Function-call syntax is parsed once and dispatched through a centralized intrinsic-function handler. As a result, new intrinsic functions can normally be added without parser modifications and with minimal impact on the execution engine.

This architecture allows practical utility functions to be introduced incrementally as requirements emerge.

Function names are matched case-insensitively.

Current intrinsic set:

```text
LENGTH
SUBSTR

LEFT
RIGHT
STRIP
POS

UPPER
LOWER

WORDS
WORD

ABS
```

Examples:

```rexx
LEFT(name,10)
RIGHT(code,3)

STRIP(customer)
POS("-", date)

WORD(fullname,2)

ABS(balance)
```

Unknown function names generate a controlled runtime error.

Potential future candidates include:

```text
MIN
MAX
SIGN

SPACE
VERIFY
DATATYPE
```

---

## CREXX Integration

The preferred integration mechanism is the `REXXSCRIPT` command.

Applications should normally interact with RexxScript through:

```rexx
REXXSCRIPT rule
           EXPOSE customer, score
           OUTPUT out
           RESULT raw
```

Conceptually:

```text
EXPOSE
    Shared variables

OUTPUT
    Captured SAY output

RESULT
    Raw diagnostic information

script_status
    Execution completion status
```

The underlying evaluator remains an implementation detail and may evolve without affecting the external RexxScript programming model.

The communication layer between CREXX and RexxScript is still considered an active design area and may evolve as additional experience is gained.

---

## Alignment With Future RexxScript

The longer-term RexxScript vision includes:

```text
runtime integration

preprocessor integration

macro facilities

variable pool policies

host adapters
```

The current implementation remains compatible with that direction but does not attempt to implement the complete architecture immediately.

---

## Deliberate Step 1 Decisions

### Expression Evaluation

Step 1 intentionally retains strict left-to-right expression evaluation.

Example:

```rexx
2 + 3 * 4
```

evaluates as:

```text
20
```

rather than:

```text
14
```

This approach keeps the parser simple and predictable.

Operator precedence and parenthesized expression support may be considered later if justified by practical requirements.

### Variable Representation

Internally, all RexxScript variables are stored as strings.

Numeric conversion occurs only when required by arithmetic or comparison operations.

Example:

```rexx
x = "10"
y = 20

z = x + y
```

produces:

```text
z = "30"
```

This closely follows traditional Rexx semantics while keeping variable management straightforward.

### Control Flow

Structured control flow is preferred over lower-level execution mechanisms.

Supported constructs include:

```text
IF / THEN
IF / THEN / ELSE

DO / END
DO WHILE
DO UNTIL
counted DO loops

LEAVE
ITERATE

RETURN
```

The implementation remains free to lower these constructs internally if beneficial.

### Variable Model

The current implementation uses isolated local variables during execution.

Variables are explicitly shared with the host through the RexxScript integration layer.

This design intentionally avoids assumptions that would complicate future support for variable pools.

Possible future directions include:

```text
in
out
inout

local
meta
```

The current implementation maintains an isolated local variable environment and exchanges values with the host through the `EXPOSE` mechanism.

This provides a natural migration path toward future variable-pool models. Output-oriented pools appear straightforward because shared variables already pass through a well-defined exchange stage. Input-oriented pool semantics are less clearly defined and remain an area for future design discussion.

Variable pools are therefore viewed as a natural extension of the current architecture rather than a prerequisite for the Step 1 implementation.


### Result Representation

The evaluator exports a flat result structure.

This representation is intentionally simple and integrates naturally with CREXX arrays.

The raw result structure is primarily intended as an integration and diagnostic mechanism rather than a user-facing programming interface.

The preferred application interface consists of:

```text
EXPOSE
OUTPUT
script_status
```

---
## Compiler Exit Clarification

Compiler exits and runtime RexxScript execution are separate concerns.

Compiler exits operate on source code during compilation. RexxScript executes a supplied script during program execution.

The existence of compiler exits therefore does not eliminate the need for a runtime execution engine.

However, the same RexxScript engine may prove useful in future preprocessing or compiler-exit scenarios. This would represent an additional use of the engine rather than a replacement for runtime RexxScript execution.

The current implementation focuses exclusively on runtime execution through the `REXXSCRIPT` command.

---
## RexxScript and Preprocessor Integration

One promising future direction is the use of RexxScript as a preprocessor macro language.

Unlike traditional C-style macros, which primarily perform textual substitution, RexxScript can use full Rexx-style logic to generate source code.

For example:

```rexx
do i = 1 to 3
  say "customer" || i || " = .string"
end
```

could generate:

```rexx
customer1 = .string
customer2 = .string
customer3 = .string
```

This would allow the same RexxScript engine to be reused both:

```text
at runtime

and

during source generation
```

without requiring separate scripting technologies.

---

## Later Steps

### Step 2

Potential enhancements:

```text
execution limits

output limits

line and column diagnostics

improved error reporting

variable pool policies

SELECT / WHEN / OTHERWISE

additional intrinsics
```

### Step 3

Potential enhancements:

```text
runtime host adapters

preprocessor host adapters

macro facilities

shared RexxScript core
```

### Compliance Phase

Only if justified by practical requirements:

```text
operator precedence

parenthesized expressions

expanded expression grammar

additional Rexx compatibility
```

---

## Out Of Scope For Step 1

```text
full Rexx parsing

PROCEDURE
CALL
ADDRESS

host commands
host command execution

file access
network access

real INTERPRET

external function dispatch
dynamic function resolution
user-defined functions

object syntax

compound variables
stems

decimal arithmetic
```

---

## Architectural Direction

The implementation should remain small while maintaining clear extension points.

The preferred internal structure is:

```text
statement splitter

tokenizer

parser

optional validation

executor

result builder
```

This provides a natural evolution path without requiring a future rewrite.

The implementation remains free to use simpler internal representations than the language surface suggests.

---

## Summary

RexxScript has evolved from a simple assignment evaluator into a structured scripting environment integrated into CREXX through the `REXXSCRIPT` command.

Implemented:

```text
IF / THEN / ELSE

DO WHILE
DO UNTIL
counted DO loops

LEAVE
ITERATE

RETURN

intrinsic functions

captured SAY output

REXXSCRIPT integration

script_status

string-based variables
```

Deferred:

```text
runtime adapters

preprocessor adapters

full language compatibility

operator precedence
```

Retained intentionally:

```text
left-to-right evaluation

small generated snippets

simple execution model
```

The original `EVALUATE()` implementation remains the foundation of the runtime, but it is no longer the primary architectural abstraction presented to application developers.

The objective remains to evolve the current implementation incrementally rather than replace it with a new design.
