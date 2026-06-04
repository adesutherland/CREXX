# rxas - the cRexx Assembler

The purpose of the cRexx assembler `rxas` is to translate a text file with
`rxvm` assembler instructions to a file with binary contents containing these
instructions in their binary, executable form. Its main use is to
translate an `rxas` file produced by the cRexx compiler
`rxc` to a binary `.rxbin` object module.

## Program Structure

![Program Structure](charts/asmstructure-crop.pdf "Title"){height=100 width=500}

The assembler processing goes through a number of steps in a single
pass: first, the Lexer / Scanner tokenises the RXAS code. After that,
the Parser parses the structure into a series of instructions. The
binary writer generates the binary code and constant pool for the
program at hand. The backpatcher runs last and handles forward references.

## Input/Output

The `rxas` assembler has a `.rxas` file as input and
produces an `rxbin` file as output, which can be considered an
*object module*, as it has unresolved addresses, which can be
resolved by the linkage editor component of the `rxvm` virtual
machine interpreter. It also produces a report to stdout (in case of
errors only) and can produce a trace file in Debug/verbose mode
(option `-d`).

## Character sets

The input file is assumed to be valid UTF8.
The assembler, like the compiler, operates using two character
sets. The first is for symbols in the assembler language
statements. These are all composed of the ASCII subset of Unicode. The
second character set is used for data; the contents of
variables. Here the whole of Unicode can be used.

## Command Line Arguments
When the command line argument -h is specified the options are shown:

\fontspec{IBM Plex Mono}
\begin{shaded}
  \small
  \obeylines \splice{rxas -h | sed 's/\&/\\\&/g'}
 \end{shaded}
 \fontspec{TeX Gyre Pagella}

## Optimizer

The assembler contains an optimizer; this is different from the
optimizer which is part of the compiler. This phase of the assembler
is running always, except when switched off by the `-n`
option. When there is any doubt whether any encountered problem is
caused by the optimizer, switching it off can help diagnosing the problem.

## Assembler Directives

For machine instructions, see the *cRexx VM
  Specification* publication. This section discusses instructions to the
assembler, which are called *directives* to clearly distinguish
them from virtual machine instructions. These are necessary to pass information into the compiled
`.rxbin` binary file, to enable execution by the `rxvm`
virtual machine. In the following itemized list, *italic*
descriptors are categories, while items in roman type are literal directives.


comments
: A block comment can be made by surrounding the text block with `/*` and `*/`
  indicators. The `*` (asterisk) can be used as a line
  comment. The remainder of the line after a line comment is ignored.

labels
: A label (a string ending with a colon, indicated in the machine instructions documentation as an `ID`, is a target for branching-type instructions.

Example:

```rxas
loop:
    fndnblnk r3,r1,r3   /* find first/next non blank offset    */
    ilt r5,r3,0         /* if <0, nothing found, end search    */
    brt break,r5
    inc r6              /* else increase word count            */
                        /* offset of word is in R3             */
    copy r8,r3          /* save offset of word                 */
    fndblnk r3,r1,r3    /* from offset find next blank offset  */
    ieq r7,r6,r2        /* is this the word we are looking for?*/
    brt wordf,r7        /* go and fetch it                     */
    ilt r5,r3,0         /* if <0, nothing found, end search    */
    brt break,r5        /* word not found                      */
    bct loop,r4,r3      /* continue to look for next non blank char */
```
  
registers
: `r0 ...*n` are names of registers, indicated in the machine instructions documentation as `REG`.

.globals=*INT*

: Defines \emph{int} global variable \emph{g0 ... g\emph{n}}. These can be used within any procedure in the file.

.locals=*INT*
: The number of local registers (local to the source program). This number needs to be 1 greater than the highest
  used register number.
  
.expose=*ID*
: Any global register marked as exposed is available to any file which also has the corresponding exposed index/name.

.srcstep
: Used to document source-step metadata. This optional
  directive records a self-contained source anchor: module-local step id,
  clause id, provenance flags, source file, source line number, active column
  range, and the whole source line. It is used by TRACE, SOURCELINE,
  panic reporting, and debuggers.

.traceevent
: Used to document semantic TRACE events. This optional directive records compact event/value codes, mode visibility, value source, value type, source-step id, clause id, and optional symbol or resolved compound names. TRACE handlers use these records instead of guessing values from source text.

.proc
: A procedure is a scope delimiting mechanism for the access of registers. The registers of a procedure are independent of the caller's registers. The VM maps its registers to the registers in the caller. Each time a procedure/function is called a new *stack frame* is provided. This means that the called function has its own set of registers.

The function header defines how many registers (called locals) the
function can access - for practical purposes one can consider that any
number of registers can be assigned to a function. In addition, each file defines a number of global registers that can be shared between procedures.

## Examples

Here are several examples of how to use `rxas` to
assemble a program into an object module.

### In-line assembly

The cRexx compiler `rxc`  enables[^1]  inline assembly through the
`assembler` statement. When used in this way, a lot of
the complications of an assembly language program can be handled by the
cRexx compiler, like assigning registers to variables, and the
conversion of datatypes like `integer` for display as `string`.

[^1]: When used with `options level b`

```rexx
/* crexx ipow test - inline assembler */
options levelb

number = 10
power = 2
result = 0

say 'test IPOW'
assembler do
   ipow result,number,power
end
say "result =" result /* The compiler converts integer to string */
```

<!--splice--crexx examples/pow.crexx-->

This is a simple and straightforward way to complement the low level
assembler instructions with the power of the Rexx language. The following example intends to explain how this is
implemented; it can be skipped without consequences.

In this example, the compiler generates the following assembler source:

\input{examples/pow.rxas}

The `.srcstep` directives (intended for trace, sourceline, panic reports,
and debuggers) indicate where the work is done. Related `.traceevent`
directives identify values that are actually available for `trace` display. The
variables are assigned, as integers, to the registers `r1` and
`r2`. The line `ipow, number, power` becomes `ipow
  r3,r1,r2`, and the display on the terminal is handled by the
`itos`,`sconcat` and `say` instructions.

This is an example, with the remark that in this case, the microcode
for `ipow` is always executed, the example in `crexx` on
page \pageref{fpowexample} shows that the cRexx optimizer of the
compiler can eliminate this code entirely.

The use of Assembler directives is not allowed in inline assembly, so
(as an example) is it not possible to define procedures in an inline
assembler block.

## Troubleshooting

The assembler will give messages when there are problems in a source
file. These are hopefully of enough clarity to resolve the immediate
problem with syntactic issues or typos. When a program assembles
correctly but its behaviour is unexpected, or its output is incorrect,
a number of different strategies can be followed.

### Adding say statements

It is easy to add `say` statements to your program. Unlike
Rexx, there is no trace statement for assembler programs. It
is possible to disassemble (see page \pageref{disassembler} an `.rxbin` module, and reassemble it
with added statements.

### Using the debugger

The `rxdb` debugger has a mode for assembler. This can be used to
set breakpoints and/or step through the code; here the registers can
be traced so variables in your program can be followed and the
comparisons and branches can be checked. For more information about the
debugger, see page \pageref{debugger}.
