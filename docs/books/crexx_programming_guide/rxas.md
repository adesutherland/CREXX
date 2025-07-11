# rxas - the \crexx{} Assembler

The purpose of the \crexx{} assembler \code{rxas} is to translate a text file with
\emph{rxvm} assembler instructions to a file with binary contents containing these
instructions in their binary, executable form. Its main use is to
translate an \emph{.rxas} file produced by the \crexx{} compiler
\emph{rxc} to a binary \emph{.rxbin} object module.

## Program Structure

\includegraphics[width=\textwidth]{charts/asmstructure-crop.pdf}

The assembler processing goes through a number of steps in a single
pass: first, the Lexer / Scanner tokenises the RXAS code. After that,
the Parser parses the structure into a series of instructions. The
binary writer generates the binary code and constant pool for the
program at hand. The backpatcher runs last and handles forward references.

## Input/Output

The \code{rxas} assembler has a \code{.rxas} file as input and
produces an \code{rxbin} file as output, which can be considered an
\emph{object module}, as it has unresolved addresses, which can be
resolved by the linkage editor component of the \code{rxvm} virtual
machine interpreter. It also produces a report to stdout (in case of
errors only) and can produce a trace file in Debug/verbose mode
(option \code{-d}).

## Character sets

The input file is assumed to be valid UTF8.
The assembler, like the compiler, operates using two character
sets. The first is for symbols in the assembler language
statements. These are all composed of the ASCII subset of Unicode. The
second character set is used for data; the contents of
variables. Here the whole of Unicode can be used.

\section{Command Line Arguments}
When the command line argument -h is specified the options are shown:

\fontspec{IBM Plex Mono}
\begin{shaded}
  \small
  \obeylines \splice{rxas -h | sed "s/&/\and/g"}
 \end{shaded}
 \fontspec{TeX Gyre Pagella}

## Optimizer

The assembler contains an optimizer; this is different from the
optimizer which is part of the compiler. This phase of the assembler
is running always, except when switched off by the \code{-n}
options. When there is any doubt whether any encountered problem is
caused by the optimizer, switching it off can help diagnosing the problem.

## Assembler Directives

For machine instructions, see the \emph{\crexx{} VM
  Specification}. This section discusses instructions to the
assembler, which are called \emph{directives} to clearly distinguish
them from virtual machine instructions. These are necessary to pass information into the compiled
\emph{.rxbin} binary file, to enable execution by the \emph{rxvm}
virtual machine. In the following itemized list, \emph{italic}
descriptors are categories, while items in roman type are literal directives.
\begin{description}
\item[\emph{comments}] A block comment can be made by
  surrounding the text block with \code{/*} and \code{*/}
  indicators. The \code{*} (asterisk) can be used as a line
  comment. The remainder of the line after a line comment is ignored.
\item[\emph{labels}] A label (a string ending with a colon, indicated in the machine
  instructions documentation as an \code{ID}, is a target for
  branching-type instructions.
  Example:
  \begin{lstlisting}[language=rxas]
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
  \end{lstlisting}
\item[\emph{registers}] \code{r0 ...\emph{n}} are names of registers, indicated
  in the machine instructions documentation as \code{REG}.
\item[.globals=\{INT\}]Defines \emph{int} global variable \code{g0 ... g\emph{n}}. These can be used within any procedure in the file.
\item[.locals=\{INT\}] The number of local registers (local to the
  source program). This number needs to be 1 greater than the highest
  used register number.
\item[.expose=\{ID\}]Any global register marked as exposed is available to any file which also has the corresponding exposed index/name.
\item[.src] Used to document source lines. This is an optional
  directive that is added for every source line processed by the
  \code{rxc} compiler. It is used for TRACE and SOURCELINE.
\item[.proc] A procedure is a scope delimiting mechanism for the access of
  registers. The registers of a procedure are independent of the
  caller's registers. The VM maps its registers to the registers in
  the caller. Each time a procedure/function is called a new \emph{stack frame} is provided. This means that the called function has its own set of registers.
The function header defines how many registers (called locals) the
function can access - for practical purposes one can consider that any
number of registers can be assigned to a function. In addition, each file defines a number of global registers that can be shared between procedures.

\end{description}

## Examples

Here are several examples of how to use \code{rxas} to
assemble a program into an object module.

\section{In-line assembly}\label{inlineAssembly}
The \crexx{} compiler \emph{rxc} enables\footnote{When used with
  \code{options level b}} inline assembly through the
\textbf{assembler} statement. When used in this way, a lot of
the complications of an assembly language program can be handled by the
\crexx{} compiler, like assigning registers to variables, and the
conversion of datatypes like \emph{integer} for display as \emph{string}.

\lstinputlisting[language=rexx,label=iexample,caption=ipowexample]{examples/pow.rexx}
\fontspec{IBM Plex Mono}
\begin{shaded}
  \small
\splice{rxc examples/pow} \obeylines
\splice{rxas examples/pow} \obeylines \splice{rxvm examples/pow}
 \end{shaded}
\fontspec{TeX Gyre Pagella}

This is a simple and straightforward way to complement the low level
assembler instructions with the power of the \textsc{Rexx}
language. The following example intends to explain how this is
implemented; it can be skipped without consequences.

In this example, the compiler generates the following assembler source:
\lstinputlisting[language=rxas,label=ipow_rxas_example,caption=ipow
rxas example.]{examples/pow.rxas}

The \code{.src} directives (intended for trace and sourceline) indicate where the work is done. The
variables are assigned, as integers, to the registers \code{r1} and
\code{r2}. The line \code{ipow, number, power} becomes \code{ipow
  r3,r1,r2}, and the display on the terminal is handled by the
\code{itos},\code{sconcat} and \code{say} instructions.

This is an example, with the remark that in this case, the microcode
for \code{ipow} is always executed, the example in \textsc{crexx} on
page \pageref{fpowexample} shows that the \crexx{} optimizer of the
compiler can eliminate this code entirely.

The use of Assembler Directives is not allowed in inline assembly, so
(as an example) is it not possible to define procedures in an inline
assembler block.

\section{Troubleshooting}
The assembler will give messages when there are problems in a source
file. These are hopefully of enough clarity to resolve the immediate
problem with syntactic issues or typos. When a program assembles
correctly but its behaviour is unexpected, or its output is incorrect,
a number of different strategies can be followed.

\subsection{Adding say statements}
It is easy to add \code{say} statements to your program. Unlike
\textsc{Rexx}, there is no trace statement for assembler programs. It
is possible to disassemble (see page \pageref{disassembler} an \code{.rxbin} module, and reassemble it
with added statements.

\subsection{Using the debugger}
The \code{rxdb} debugger has a mode for assembler. This can be used to
set breakpoints and/or step through the code; here the registers can
be traced so variables in your program can be followed and the
comparisons and branches can be checked. For more information about the
debugger, see page \pageref{debugger}.

