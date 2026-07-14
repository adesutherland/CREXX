# Architectural Facilities

The cRexx Virtual Machine is organized as a collection of architectural
facilities. Each facility provides a coherent set of operations that implement
a particular aspect of program execution. The instruction set is the
programmer-visible interface to these facilities.

This organization separates the architectural capabilities of the virtual
machine from the individual instructions that implement them. New instructions
may be introduced in future releases without altering the architectural model,
provided they extend existing facilities or introduce new ones in a compatible
manner.

The following sections describe each architectural facility at a conceptual
level. The detailed semantics, operands, exceptions, and examples for
individual instructions are described in the Instruction Reference chapter.



## Fixed-Point Arithmetic

The Fixed-Point Arithmetic facility provides the fundamental integer
computation capabilities of the cRexx Virtual Machine. These facilities operate
on signed integer values and are used for counters, loop variables, indexing,
address calculations, and general-purpose arithmetic. Since integer operations
produce exact results whenever the mathematical result is representable, they
form the basis of most compiler-generated code.

The facility supports arithmetic operations, comparisons, sign manipulation,
increment and decrement operations, and conversions between integer values and
the other numeric representations supported by the virtual machine.
Representative instructions include `iadd`, `isub`, `imul`, `idiv`, `isex`,
`inc`, and `dec`.

Arithmetic operations perform the elementary mathematical functions on integer
operands. Comparison operations establish ordering relationships that are
examined by the Control Flow facility. Conversion operations translate integer
values into floating-point, decimal, binary, and string representations.



## Floating-Point Arithmetic

The Floating-Point Arithmetic facility provides efficient computation on
approximate real numbers. It is intended for scientific, engineering,
statistical, and graphical applications where numerical range is more important
than exact decimal representation.

The facility supports arithmetic operations, comparisons, mathematical
transformations, and conversions between floating-point values and the other
numeric representations supported by the virtual machine. Representative
instructions include `fadd`, `fsub`, `fmul`, `fdiv`, floating-point comparison
operations, and the various conversion instructions.

Unlike fixed-point arithmetic, floating-point operations may involve rounding,
overflow, underflow, and exceptional values such as infinities and NaNs. These
behaviors are defined by the floating-point model implemented by the virtual
machine.



## Decimal Arithmetic

The Decimal Arithmetic facility provides exact computation on decimal values.
Unlike binary floating-point arithmetic, decimal operations preserve decimal
precision throughout a computation, making them particularly suitable for
financial, commercial, and accounting applications.

The facility supports decimal arithmetic, comparison, scaling, rounding, and
conversions between decimal values and the other numeric representations
supported by the virtual machine. Representative instructions include decimal
addition, subtraction, multiplication, division, comparison, scaling, and
rounding operations.

By providing decimal arithmetic directly within the instruction set, the
virtual machine ensures predictable and portable behavior independent of the
decimal facilities provided by the host processor. There are two implementations
for every decimal instruction, one following the traditional Rexx unlimited precision
model, and the other a high-performance hardware variant.



## String Processing

The String Processing facility provides the text manipulation capabilities of
the cRexx Virtual Machine. Character strings are represented as immutable
UTF-8 values, allowing efficient storage while supporting the complete Unicode
character repertoire.

The facility includes string construction, concatenation, extraction,
searching, comparison, formatting, and conversion between strings and the
other data types supported by the virtual machine. Representative instructions
include concatenation, substring extraction, lexical comparison, string
search, and conversions between strings and numeric or binary values.

Because text processing occupies a central role in the Rexx language family,
the virtual machine provides dedicated string operations rather than relying
entirely on runtime library routines.



## Binary Memory

The Binary Memory facility provides operations on uninterpreted sequences of
bytes. Unlike the String Processing facility, binary memory assigns no
character encoding or semantic meaning to stored data. These facilities are
intended for implementing binary file formats, communication protocols,
serialization, cryptographic algorithms, and interfaces to external systems.

The facility includes storage management, copying, updating, typed field
access, memory movement, text-field extraction, and binary comparison.
Representative instructions include `blen`, `bresize`, `bcopy`, `bappend`,
`bupdate`, `bgetu32`, `bseti64`, `bgets`, `bsets`, `bmove`, `bmemmove`,
`bcmpb`, and `bcmps`.

Storage management operations create, resize, and initialize binary objects.
Copying and movement operations efficiently relocate byte sequences without
interpreting their contents. Typed access operations interpret selected byte
ranges as fixed-width integers or floating-point values, while text-field
operations provide controlled interchange between binary objects and UTF-8
strings.



## Logical Operations

The Logical Operations facility manipulates Boolean values and individual bits
without interpreting them as numeric quantities. These operations provide the
building blocks for condition evaluation, bit masking, flag manipulation, and
compact binary encoding.

The facility supports Boolean operations, bit testing, shifting, rotation, and
other bit-oriented manipulations. Representative instructions include `and`,
`or`, `xor`, `not`, logical and arithmetic shift operations, rotation
operations, and bit-test instructions.

Logical operations frequently cooperate with the Fixed-Point Arithmetic and
Control Flow facilities to implement efficient decision-making and low-level
algorithms.



## Control Flow

The Control Flow facility governs the order in which instructions are executed.
Every programming construct, including conditional statements, loops,
procedure invocation, recursion, exception handling, and program termination,
is ultimately implemented by this facility.

The facility includes unconditional transfers, conditional branches,
procedure invocation, procedure return, and exception transfer.
Representative instructions include `branch`, the conditional branch
instructions, `call`, and `ret`.

Branch operations examine execution conditions established by previous
comparison operations. This separation between comparison and branching allows
a single comparison result to be examined by multiple subsequent branches and
simplifies compiler optimization.

Procedure invocation operations preserve sufficient execution context to
support nested procedure calls and recursive execution while maintaining
machine independence.



## Object and Runtime Facilities

The Object and Runtime Facilities provide services that manage the execution
environment rather than application data. These facilities support the
object-oriented execution model of the cRexx Virtual Machine and expose
selected aspects of the runtime environment to executing programs.

The facility includes object creation, runtime type inspection, reflection,
exception handling, metadata access, dynamic loading, stack inspection, and
runtime service invocation. Representative instructions include the object
construction, type inquiry, exception, and runtime metadata operations.

These facilities isolate application programs from the internal organization
of the virtual machine while providing the flexibility required by dynamic
language features.



## Input/Output

The Input/Output facility provides controlled access to files, streams,
devices, and operating-system services while insulating application programs
from host-specific interfaces.

The facility includes operations for opening and closing files, reading and
writing streams, positioning within files, querying stream status, and
performing related operating-system services. Representative instructions
include the file, stream, console, and operating-system interface operations.
The Input/Output Facility also includes the low-level network instructions
which form the base for TCP/IP Socket handling in the runtime library.

By presenting a uniform interface to external resources, the Input/Output
facility enables programs to execute consistently across different host
platforms.



## Time Services

The Time Services facility provides access to temporal information maintained
by the execution environment. Programs may obtain the current date and time,
measure elapsed execution intervals, and access high-resolution timing
facilities.

Representative instructions obtain calendar dates, clock values, elapsed
times, processor timing information, and related temporal measurements.

These facilities support scheduling, profiling, benchmarking, timeout
processing, logging, and other time-dependent algorithms while remaining
independent of the host operating system.



## Debugging Facilities

The Debugging Facilities provide architectural support for debugging,
instrumentation, tracing, and program analysis. They allow execution to be
suspended at well-defined locations so that execution state can be examined or
modified by external development tools.

The facility includes breakpoint operations, execution tracing, state
reporting, and controlled resumption of execution. Representative instructions
include the various breakpoint and debugging operations.

Although these facilities have no direct effect on program semantics, they
provide a standardized interface for debuggers, profilers, test frameworks,
and other software development tools.
