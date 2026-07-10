# RXAS Program Syntax

RXAS is the textual assembly form for cREXX VM bytecode. It is normally emitted
by `rxc`, but it is also intended to remain readable enough for tests, focused
runtime examples, and low-level diagnostic work.

An RXAS source file is line-oriented. Blank lines are allowed, comments are
ignored, and each non-comment line is one directive, one procedure header, one
instruction, or metadata attached to the current scope. A source file has three
logical regions:

1. Optional module headers.
2. Zero or more procedure declarations or definitions.
3. Instructions and instruction-scoped metadata inside each procedure body.

Header directives must appear before the first procedure. Once a procedure
header has been seen, later lines are parsed as procedure declarations,
procedure definitions, instructions, labels, or procedure metadata.

## Comments And Whitespace

Whitespace separates tokens and is otherwise not significant. Newlines are
significant because they terminate directives and instructions.

A line comment begins with `*` when `*` is the first non-whitespace character on
the line:

```rxas
* This is a comment
    * This is also a comment
```

Block comments use `/* ... */` and may be nested:

```rxas
/*
 * Outer comment.
 * /* Nested comment. */
 */
```

## Module Headers

Module headers describe global registers, exposed symbols, module metadata, and
named constants. The common forms are:

```rxas
.globals=0
g0 .expose=sharedValue
.const bytes binary 0x01020304
.const name string "lookup-key"
```

`.globals=<integer>` declares the module global-register count. Global registers
are written as `g0`, `g1`, and so on. A global register may be exposed with
`gN .expose=<name>` so other modules can link to it. Local registers `rN` and
argument registers `aN` cannot be exposed with this header form.

`.const` creates a module-scoped read-only alias for a pooled constant. Release
1 supports binary and string constant aliases:

```rxas
.const table binary 0x0001020304050607
.const marker string "index"
```

The alias itself is an identifier operand. Each instruction form decides whether
that identifier is legal as a string constant, binary constant, label, procedure
name, or other symbolic operand.

## Procedures

A procedure definition names the callable and declares how many local registers
the procedure uses:

```rxas
main() .locals=4
```

The local register declaration must cover the `rN` registers referenced by the
procedure body. A definition can also expose the procedure under a link-visible
name:

```rxas
worker() .locals=8 .expose=package.worker
```

A procedure declaration names an external callable without defining a normal
instruction body:

```rxas
externalCall() .expose=package.externalCall
```

Declarations accept only the restricted metadata forms used by the compiler and
linker. They do not contain normal RXAS instructions.

## Instructions

Instructions use a mnemonic followed by zero, one, two, or three operands. The
first operand follows the mnemonic directly; later operands are separated by
commas:

```rxas
ret
load r1,"hello"
bgetu8 r2,table,r0
add r3,r1,r2
```

The assembler checks the mnemonic and operand types against the instruction
table. Unknown mnemonics, too many operands, missing operands, and illegal
operand forms are syntax or assembly errors.

Labels are written as `name:`. A label may stand on its own line or immediately
precede another instruction. Branch and jump instructions refer to labels by
identifier:

```rxas
loop:
    add r1,r1,r2
    br loop
```

Jump-table case labels use a label followed by `.jcase` on the same line. The
`.jcase` directive decorates the label and does not emit an instruction:

```rxas
keyword_if: .jcase keywords "if"
```

## Operands

RXAS recognizes these operand classes:

- Local registers: `r0`, `r1`, `r2`, ...
- Global registers: `g0`, `g1`, `g2`, ...
- Argument registers: `a0`, `a1`, `a2`, ...
- Identifiers: labels, constant aliases, exposed names, and symbolic operands.
- Procedure references: `name()`.
- Integer literals.
- Floating-point literals.
- Decimal literals.
- Character literals.
- String literals.
- Binary literals.

Registers are untyped storage locations at the RXAS syntax level. The selected
instruction determines how the VM interprets each register operand at runtime.

## Literal Values

Integer literals are signed or unsigned decimal digits:

```rxas
load r1,42
load r2,-1
```

Floating-point literals use a decimal point or exponent:

```rxas
load r1,1.5
load r2,1e6
```

Decimal literals use the same numeric spelling with a lowercase `d` suffix:

```rxas
load r1,10d
load r2,1.25d
```

String literals are double quoted. A literal newline is not allowed inside a
string. An escaped double quote is written as `\"`:

```rxas
load r1,"hello"
load r2,"quoted \"text\""
```

Character literals are single quoted:

```rxas
load r1,'A'
```

Binary literals use byte-paired hexadecimal after `0x` or `0X`. The empty binary
literal `0x` is valid:

```rxas
load r1,0x
load r2,0x000102ff
```

Binary literals and `.const ... binary ...` aliases are stored as binary
constant-pool values, not as integer or string values.

## Jump Tables

`.jtable` declares a procedure-local static dispatch table. `.jcase` decorates
target labels with literal keys, and `jumps`, `jumpb`, `jumpbs`, or `jumpi`
branch through the assembled table. A miss falls through to the next
instruction, so write an explicit default branch after the jump instruction.

```rxas
main() .locals=4
    .jtable keywords linear
    br after_cases
keyword_if: .jcase keywords "if"
    load r1,1
    br done
keyword_else: .jcase keywords "else"
    load r1,2
    br done
after_cases:
    load r0,"else"
    jumps r0,keywords
    br not_found
done:
    ret
not_found:
    load r1,0
    ret
```

Release 1 supports `auto`, `linear`, `openhash`, and `acph`. Explicit algorithm
names are useful for measurement and repeatable tuning. `auto` selects `linear`
for a one-case table. Larger tables use `openhash` for average key lengths up to
two bytes; tables of at least 256 cases also use it for average lengths up to
four bytes. The remaining longer-key tables use `acph`. This policy is based on
Release VM profiling and may be retuned without changing the RXAS surface.

## Metadata

`.meta`, `.srcstep`, and `.traceevent` records carry compiler, linker, debug,
interface, and trace information. Hand-written RXAS usually does not need them,
but generated RXAS relies on them for source mapping, exposed callable metadata,
classes, interfaces, attributes, inline bodies, and TRACE output.

The source-step and trace-event forms are:

```rxas
.srcstep <step-id> <clause-id> <flags> "<file>" <line> <start-col> <end-col> "<source line>"
.traceevent "<kind>" <mode-mask> "<value-source>" "<value-type>" "<register-type>" <value-ref> <source-step-id> <clause-id> <flags> "<symbol>" "<resolved-name>"
```

`.meta` has several scoped forms. The compiler should be treated as the owner of
most `.meta` payload formats unless an instruction or linking feature documents
the record explicitly.

## Minimal Example

```rxas
.globals=0
.const table binary 0x01020304

main() .locals=4
    load r0,0
    load r1,"hello, rxas"
    say r1
    bgetu8 r2,table,r0
    itos r2
    say r2
    ret
```

This module declares no global registers, defines one binary constant alias,
loads and prints a string, reads the first byte of the binary constant, converts
that integer to a string, prints it, and returns.
