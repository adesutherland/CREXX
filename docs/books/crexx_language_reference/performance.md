## Performance

For precise technical documentation, we refer to the \emph{\crexx{} VM Specification} publication, which contains the last word on the architecture and implementation of the bytecode compiler, the assembler and the first two virtual machines. One of these is a conventional byte code interpreter, the other is a \emph{threaded code interpreter}. These share over 99\% of their source code.

The virtual machine executing the \code{rxas} assembler instructions is written according to a strict set of rules that limits the possibility of pipeline stalls in modern processor architectures.

## Compiler inlining

The `rxc` compiler performs conservative inlining as an optimisation pass. When
the compiler can prove that replacing a call with the body of the called
procedure, method, or factory preserves the same behaviour, it may do so before
emitting `rxas` assembly. If that proof is not available, the call is left as an
ordinary call. This means inlining should be treated as an implementation
optimisation, not as a source-language guarantee.

The implementation is deliberately conservative because /crexx{} inlining is not
simple text substitution. The compiler rewrites the already-validated abstract
syntax tree and then continues through the normal compiler pipeline. That tree
surgery must preserve:

- argument binding, including defaults, by-value copies, and exposed/reference
  arguments
- return values, including object, binary, and array-shaped values
- nested scopes, local variables, and source/debug locations
- method receiver state and copyback for mutating methods
- factory setup and object initialization
- multi-level imports, where source, rxas, rxdas, binary metadata, and linked
  libraries must all describe the same callable contracts

Some cases are intentionally left as calls. Examples include procedure-level
`expose`, dynamic-index varargs, assembler code that aliases registers, receiver
expressions that need complex copyback, interface/member dispatch that cannot
be proved to target one concrete implementation, and factories or methods whose
class layout cannot yet be proved safely. These exclusions are not failures;
they are how the optimiser avoids changing program semantics.

For the initial public release the compiler uses a 300 AST-node body cutoff for
inline candidates. This is a profitability and metadata-size limit rather than
a language rule. Larger bodies, or bodies that cross one of the semantic gates
above, remain normal calls.

Use `rxc -n` to disable compiler optimisation, including inlining, when
comparing generated assembly or investigating optimiser behaviour.

## Native executables

Level B can produced native[^1] executables which can be distributed
and run on machines that do not have any \crexx{} infrastructure
installed. These contain a compiled version of the programs linked
into it, as well as their own version of the \crexx{} virtual machine;
the startup of these programs is very fast because the compilation,
assembly and linkedit steps can be skipped.

[^1]: native to combination of the instruction set architecture of the machine and
its operating system calling conventions.
