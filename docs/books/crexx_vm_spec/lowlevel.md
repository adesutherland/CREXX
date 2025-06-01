# Low-Level System Information

## Register based Virtual Machine
\crexx{} uses a register based virtual machine.

A register-based virtual machine uses software registers to store and manipulate data. In a register-based virtual machine, the instructions explicitly reference the registers by number, and the values are loaded into and out of the registers as needed.

In contrast, a stack-based virtual machine, like used for the JVM and UCSD-Pascal, uses a stack to store and manipulate data. In a stack-based virtual machine, the operands are pushed onto the stack, and the operators pop them off the stack, perform the operation, and push the result back onto the stack. The stack-based approach is simpler than the register-based approach because there are no explicit references to registers in the instructions, and the operands are automatically managed by the stack.

Some other differences between a register-based virtual machine and a stack-based virtual machine include:

- Register-based virtual machines typically use more memory for the registers than stack-based virtual machines use for the stack.
- Register-based virtual machines may require more complex instruction encoding and decoding logic than stack-based virtual machines, which can impact the size and complexity of the virtual machine implementation.
- Stack-based virtual machines can be easier to implement and optimize for certain types of operations, such as function calls and loops, where the data can be easily pushed onto and popped off the stack.
- Register-based virtual machines can provide more flexibility in terms of register allocation and optimization, which can be particularly important for high-performance applications.

The execution environment of a \crexx{} program is a
threaded[^threaded] virtual
machine that is designed for optimal performance. This virtual
machine, implemented in the ```rxvm``` executable, executes
machine instructions produced by the ```rxc``` \crexx{}
compiler, or written by hand, assembled into an ```.rxbin``` binary file by
the ```rxas``` assembler.

[^threaded]: an alternative, non-threaded executable is available under the ```rxbvm``` name.

## Machine Interface

This section describes the processor-specific information for the
hypothetical rxvm processor. \index{rxvm processor} The instruction set can be seen as the <!--index-->ISA (instruction set architecture) for this ```rxvm``` processor, of which the microcode is implemented in the C99 language.

Programs intended to execute directly on the processor use the
<!--index-->`rxvm` instruction set and the
instruction encoding and semantics of this architecture.

An application program can assume that all instructions defined by the
<!--index-->architecture and that are neither privileged nor optional, exist and work
as documented. At this moment, there are no <!--index-->privileged RXVM instructions.

To be ABI (application binary interface) conforming, the processor must implement the instructions of
the architecture, perform the specified operations, and produce the
expected results. The ABI neither places performance constraints on
systems nor specifies what instructions must be implemented in
hardware.  A software implementation of the architecture conforms to
the ABI; likewise, the architecture could be implemented in hardware,
e.g. an <!--index-->FPGA.

## The Register

\index{register}
\begin{wrapfigure}{l}{0.4\textwidth}
\includegraphics[scale=0.4]{charts/register.pdf}
\end{wrapfigure}

The number of registers is only limited by memory and, for
practical purposes, can be considered unlimited. The address size of
the fields in a register is, for the virtual machine implementations,
implied by the address size that the host OS can handle. For hardware
the size is undefined and can follow the hardware address generation
capacity.

```c <!--register.c-->
struct value {
    /* bit field to store value status - these are explicitly set */
    value_type status;

    /* Value */
    rxinteger int_value;
    double float_value;
    void *decimal_value; /* TODO */
    char *string_value;
    size_t string_length;
    size_t string_buffer_length;
    size_t string_pos;
#ifndef NUTF8
    size_t string_chars;
    size_t string_char_pos;
#endif
    void *object_value;
```

\index{conversions, data type in assembler}
\index{say instruction, rxas}
\index{int_value, register}

The status field determines for instructions that expect a data type,
which field is the field to act upon. Conversions are possible but
never implicit. So for example, when the register contains an int
value in field `int_value`, but it needs to be printed with the
`say` instruction, the
<!--index-->`itos` instruction takes care of the conversion and the population of the
memory area the \code{*string_value} points to. 

```c <!--statusfield.c-->
typedef union {
    struct {
        unsigned int type_object : 1;
        unsigned int type_string : 1;
        unsigned int type_decimal : 1;
        unsigned int type_float : 1;
        unsigned int type_int : 1;
    };
    unsigned int all_type_flags;
} value_type;
```

