# Low-Level System Information

## Register-Based Virtual Machine

\crexx{} uses a register-based virtual machine. Instructions name their source
and target registers directly, and logical data flow is written from right to
left in RXAS instruction forms.

The threaded `rxvm` interpreter is the base VM executable. `rxbvm` provides the
bytecode-dispatch variant. `rxvme` and `rxbvme` are extended interpreters that
include the standard library image used by common Level B programs.

## Machine Interface

RXAS is the assembly language for the RXVM instruction set. The assembler emits
RXBIN records containing constant-pool data, module records, instruction
streams, and metadata records.

There are no privileged RXVM instructions in the current release. Native and
platform integration is provided through VM instructions, the embedding API,
and the plugin architecture.

## Registers and Values

The number of VM registers is limited by memory and by instruction encoding,
not by a small hardware-style register file. Source-level variables, temporary
values, arguments, object attributes, and VM integration records are mapped to
registers by the compiler or by hand-written RXAS.

At runtime, a register holds a `value` structure. The exact C structure is an
implementation detail, but the current model contains:

- a `value_type` status
- integer, floating-point, decimal, string, binary, and object storage
- string and binary length/capacity fields
- native payload hooks used by the plugin/runtime integration layer
- object type-name and attribute storage used by the class/interface runtime

The current implementation lives in `interpreter/rxvalue.h`. A simplified view
is:

```c
typedef struct value {
    value_type status;
    rxinteger int_value;
    double float_value;
    void *decimal_value;
    size_t decimal_value_length;
    char *string_value;
    size_t string_length;
    char *binary_value;
    size_t binary_length;
    void *native_payload;
    char *object_type_name;
    struct value **attributes;
} value;
```

Do not rely on older documentation that showed `value_type` as a set of object,
string, decimal, float, and int bit flags. That layout is not the current
runtime contract.

## Conversions

RXVM instructions are typed. Conversions are explicit in generated or hand
written RXAS. For example, an integer value that must be printed as text is
converted by an integer-to-string instruction before a string-oriented
instruction consumes it.

This explicit conversion model is one of the reasons Level B can be checked by
the compiler and still expose useful assembler-level control.
