# MiniForth design

## Purpose

MiniForth is a proof of concept showing that cREXX can host a small embedded
or emulated language. The guest language has its own words, stack, dictionary,
execution rules, and diagnostics, while the implementation remains ordinary
cREXX code.

It is deliberately a small Forth-like interpreter, not a complete or
standards-compliant Forth system.

## Module boundary

`forth.rxpp` is the reusable interpreter module. Applications import it and
call:

```rexx
import forth
##external forth

call forth_run program
```

The caller owns the input array. The module initializes a fixed-size integer
data stack for each `forth_run` call and returns a status code. The interpreter
keeps its execution state inside the module namespace while the call is active.

The module also exposes `forth_set_debug`, which selects the trace level:

- `0`: no execution trace
- `1`: readable word execution and stack transitions
- `2`: detailed VM-style diagnostics

## Input and dictionary construction

Forth input is currently embedded with RXPP's existing `##DATA` block syntax:

```rexx
##data program
  3 SQUARE .
  bye
##end
```

User-defined words are collected by an RXPP `##RULE` block and converted into a
dictionary containing a name and whitespace-separated body for each word:

```rexx
##rule definitions into forthRule
  : SQUARE DUP * ;
##end
```

This keeps the first implementation simple and makes the separation between
host source preparation and guest execution explicit. There is intentionally
no dedicated `FORTH` RXPP block tag at present. A tag can be introduced later
if Forth blocks become common enough to justify language-specific syntax.

## Execution model

The interpreter splits each input line into words and executes them from left
to right. A word is handled in this order:

1. integer literal — push it on the data stack;
2. built-in primitive — perform the operation;
3. dictionary lookup — execute the stored user-word body recursively;
4. otherwise — report an unknown-word error.

The current stack has 256 integer cells. Primitive operations perform explicit
underflow and overflow checks. Division by zero and unknown words are reported
as execution errors, and `forth_run` returns a non-zero status when execution
fails.

The readable trace uses an execution-depth counter so nested user words do not
produce duplicate completion snapshots. Detailed level-2 tracing remains
available for implementation diagnostics.

## Relationship to Forth and RPN

Forth uses postfix, stack-oriented notation related to Reverse Polish Notation:
operands are placed on the stack and an operator consumes the values it needs.
The underlying Polish notation was introduced by the Polish logician Jan
Łukasiewicz; the reverse form became widely known through stack-based
calculators and languages such as Forth.

MiniForth adopts this execution model without attempting to reproduce the full
Forth language, standard word set, memory model, or target-machine tradition.

## Future extension points

The current `forth_run` interface can remain stable while the guest language is
extended with:

- `IF ... ELSE ... THEN` and loop control;
- an explicit return stack and `EXIT`;
- runtime parsing of `: NAME ... ;` definitions;
- additional stack and arithmetic words;
- comments and more flexible tokenization;
- console words such as `.S`, `WORDS`, `CR`, and `EMIT`;
- variables, memory words, character values, and string output.

