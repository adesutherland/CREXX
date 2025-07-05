## Performance

For precise technical documentation, we refer to the \emph{\crexx{} VM Specification} publication, which contains the last word on the architecture and implementation of the bytecode compiler, the assembler and the first two virtual machines. One of these is a conventional byte code interpreter, the other is a \emph{threaded code interpreter}. These share over 99\% of their source code.

The virtual machine executing the \code{rxas} assembler instructions is written according to a strict set of rules that limits the possibility of pipeline stalls in modern processor architectures.

## Native executables

Level B can produced native[^1] executables which can be distributed
and run on machines that do not have any \crexx{} infrastructure
installed. These contain a compiled version of the programs linked
into it, as well as their own version of the \crexx{} virtual machine;
the startup of these programs is very fast because the compilation,
assembly and linkedit steps can be skipped.

[^1]: native to combination of the instruction set architecture of the machine and
its operating system calling conventions.
