# Introduction

\crexx{} is a general-purpose programming language. It is designed for
people, not computers. In this respect it follows \rexx{} closely, with
many of the concepts and some of the syntax inspired by \rexx{}
or its object-oriented variants, Object R\textsc{exx} and \nr{}, but
everything is carefully designed to be future-proof and looking
forward.

\crexx{} will be delivered in different levels. Level B is the first and
contained in this MVP release. Level C will be mostly compatible with
ISO standard Classic \textsc{Rexx}. 
In addition to the mentioned \textsc{Rexx} variants, \crexx{} level B contains 
static typing and binary arithmetic.

Later levels will contain an object model, exception
handling and a library of classes. The resulting language not only provides the scripting
capabilities and decimal arithmetic of R\textsc{exx}, but also seamlessly
extends to large application development with fast binary arithmetic.

The \crexx{} team is trying to provide a new, up-to-date
implementation of \rexx{}, and to incorporate many of the improvements
oo\rexx{} and \nr{} added to the language. In some cases, choices
need to be made. These choices are made in dialogue with the \rexx ARB
(Architecture Review Board). While, in this day and age of open source
development every language implementation team is autonomous, and in
fact everyone can fork a repository and strike out on their own, the
\crexx{} team seeks cooperation and efficient deployment of work.

The terms 'new' and 'up-to-date' refer to several different
developments in our technical and social reality. The developments of
CPU and GPU technology mean that, while we might have hit a wall with
Moore's law, parallelism and pipelining have become more important,
as are cache sizes, RISC and vectorising of instructions, like done in
different SIMD implementations. As address ranges have become large
and segmentation is something of the past, we are de-emphasising
memory management and emphasising instruction pipelining in the
\crexx{} VM Kernel. Also, we are testing our hypothesis that \rexx{}
can be a high-performance language, so apart from that VM Kernel,
everything is written in that language.

We are convinced that the working of interpreters and compilers needs
to be understandable to its users; with a more complete mental imagery
of those workings, it is possible to write much better programs. We
strive to have a completely transparent architecture for the language
tools. For this reason we have tried to open up the bytecode compiler
and interpreter layers to the user of the language, instead of hiding
these. This also enables the team (or others) to start working on
native, true compilers with backends that generate executables
for different hardware instruction set architectures or future virtual machines.

Also from a software point of view the world has changed. Apart from
fads and silver bullets, some developments have stuck and are
sometimes missed in \rexx{}, which at this point in time is more than
40 years old. The introduction of Object Orientation in the 1990's was
a long and arduous process, that might have taken too much time than
was time-to-market and user expectations allowed. What did not exactly
helped was putting this Orexx version into the hands of a different
language architect, the product intentions published early and the
product delivery taking a very long time - as a 'user switchable'
implementation on a moribund operating
system\footnote{IBM's OS/2.} Further announcements and retractions
of its owner regarding its open sourcing status did not help it in any way.
Another variant, for the Java Virtual Machine, has shown the
divergence in ideas between \rexx{}'s original architect, Mike
Cowlishaw, and the Object Rexx implementation of the same ideas.

We would like to try to avoid this by opening up architecture, source
and discussion in the user community. While \rexx{} variants may have
diverged on small points, an effort could be set up to standardize as
much as possible on new developments. What are these new developments
in programming language technology?\footnote{some might be quite old}

A discussion on various mailing fora brought about the following list
of what different variants of \rexx{} need:

- Short circuit evaluation
- Functions as first class objects
- Multi-threading
- Dynamically typed, preferably with type hints
- Co-routintes
- Support for object oriented programming
- Regular expressions
- Libraries for web programming, jpegs, JSON/YAML parsing etc
- Language tools
- Logic programming


Note that this list is just a statement of intent and some things
do exist in existing implementations while others might not be a
good idea or would be done better by tools external to the language.
  
## Level B Minimally Viable Product Snapshot

It is important to know that the Level B implementation is not complete
yet. What still is missing from this snapshot with
regard to the first \crexx{} Level B release, is the following

- Address statement
- stream I/O
- structured variables


The team deems it important to release the working parts at this time,
so people can play with it, be amazed by the speed of code implemented
with \rexx{}, use Unicode and see how it works, and possibly,
hopefully participate in its development. This means that \crexx{} has
entered the phase in which things are stable enough for working on
specific tasks by a larger team. Everybody is welcome! Lots of things
can be done in \rexx{}!

## Features of R\textsc{exx}}
The R\textsc{exx} programming language\footnote{Cowlishaw, M. F., \textbf{The REXX Language} (second edition), ISBN 0-13-780651-5, Prentice-Hall, 1990.} was designed with just one objective: to make programming easier than it was before. The design achieved this by emphasizing readability and usability, with a minimum of special notations and restrictions. It was consciously designed to make life easier for its users, rather than for its implementers.

The great strengths of R\textsc{exx} are its human-oriented features, including

- simplicity
- coherent and uncluttered syntax
- comprehensive stringhandling
- case-insensitivity
- arbitrary precision decimal arithmetic.

Care has been taken to preserve these. Conversely, the interpretive
nature of Classic Rexx has always entailed a lack of efficiency: excellent R\textsc{exx}
compilers do exist, from IBM and other companies, but cannot offer the
full speed of statically-scoped languages such as
C\footnote{Kernighan, B. W., and Ritchie, D. M., \textbf{The C
    Programming Language} (second edition), ISBN 0-13-110362-8,
  Prentice- Hall, 1988.} or Java\footnote{Gosling, J. A., \emph{et
    al.} \textbf{The Java Language Specification}, ISBN 0-201-63451-1,
  Addison-Wesley, 1996.}. Where \nr{} already offers high performance
on the Java Virtual Machine, \crexx{} aims to offer this with speed of
native code.
