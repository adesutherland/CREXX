# MiniForth example

This directory contains a small Forth-like interpreter written in cREXX. It
is intended as an example of separating a reusable cREXX module from programs
that use it.

It is also a proof of concept that cREXX can serve as the implementation base
for a small embedded or emulated language: the guest language has its own
syntax, stack, dictionary, execution rules, and diagnostics, while the host
module is still ordinary cREXX code.

## Background

Forth programs are written in postfix, stack-oriented notation. This is closely
related to Reverse Polish Notation (RPN): operands are placed on the stack and
an operator consumes the values it needs. The underlying Polish notation was
introduced by the Polish logician Jan Łukasiewicz; the reverse form became
widely known through stack-based calculators and languages such as Forth.

MiniForth is only a compact demonstration. It is not intended to be a
standards-compliant Forth implementation.

## Files

- `forth.rxpp` is the reusable interpreter kernel and provides `forth_run` and
  `forth_set_debug`.
- `forth_smoke_test.rxpp` exercises arithmetic, stack operations, comparisons,
  and user-defined words.
- `forth_trace.rxpp` demonstrates the optional execution trace.

The generated `.crexx`, `.inc`, `.rxas`, `.rxbin`, and driver-lock files are
build products and are not part of the source example.

## Installation and preparation

Before running an example, the `forth` module must be built from
`forth.rxpp`. Compile, assemble, and link `forth.rxpp` first so that the
resulting `forth` module is available on the cREXX module search path. The
`crexx.exe` driver can be used to run this complete toolchain for the module.

The example programs then import the prepared module with:

```rexx
import forth
##external forth
```

## Current RXPP syntax

There is deliberately no dedicated `FORTH` RXPP block tag at present. Forth
input is embedded with the existing `##DATA` facility:

```rexx
##data trace_input
  3 4 + .
  5 DUP * .
  bye
##end

call forth_run trace_input
```

This keeps the RXPP implementation simple while still allowing a readable
Forth program to be embedded in the example. A dedicated tag could be added
later if Forth blocks become common enough to justify language-specific RXPP
syntax.

## Tracing

Tracing is disabled by default. The example enables it with:

```rexx
call forth_set_debug 1
```

The levels are:

- `0`: no execution trace
- `1`: readable word execution and stack transitions
- `2`: detailed VM-style diagnostics, including stack-pointer information

Forth output produced by `.` remains separate from the trace output.

## Supported demonstration features

The kernel currently includes:

- integer literals and a data stack
- `+`, `-`, `*`, and `/`
- `DUP`, `DROP`, `SWAP`, and `OVER`
- comparisons and `0=`
- output with `.`
- simple colon-style user-defined words
- stack-underflow, stack-overflow, division-by-zero, and unknown-word checks

## Possible future extensions

The current implementation is intentionally small. Possible extensions, in
rough priority order, include:

- control flow with `IF ... ELSE ... THEN`
- loops such as `BEGIN ... UNTIL` and `BEGIN ... WHILE ... REPEAT`
- an explicit return stack and `EXIT`
- runtime parsing of colon definitions (`: NAME ... ;`) instead of relying on
  the RXPP `##RULE` block
- additional core words such as `MOD`, `NEGATE`, `ABS`, `ROT`, `2DUP`, and
  `2DROP`
- comments and more flexible whitespace/token handling
- diagnostic and console words such as `.S`, `WORDS`, `CR`, and `EMIT`
- variables, memory words, character values, and string output

These are deliberately not part of the current contract. They can be added
incrementally without changing the basic `forth_run` interface.
