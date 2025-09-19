# Data Types

Types in the language are named using a period (`.`) followed by the type name (e.g., `.string`). This naming convention 
helps to distinguish types from other elements in the language, such as variables and procedures.

## Built-In Types

The language includes a number of built-in types, which are provided by the language itself.

| Type       | Description                                                               |
|:-----------|:--------------------------------------------------------------------------|
| `.void`    | Represents the absence of a value.                                        |
| `.boolean` | Represents a boolean value (`1` or `0`).                                  |
| `.integer` | Represents an integer.                                                    |
| `.float`   | Represents a floating-point number.                                       |
| `.decimal` | Represents an arbitrary-precision decimal number.                         |
| `.string`  | Represents a string of characters – Unicode codepoints, encoded as UTF-8. |
| `.binary`  | Represents an opaque binary value.                                        |
| `.object`  | Represents the base class for all objects in the language.                |

## Statically Typed

The language's type system is **statically typed**, which means that the types of variables and expressions are 
known at compile time. This helps to ensure that the language is type-safe, preventing many runtime errors. 
When operations involve different types, CREXX Level B automatically promotes them according to a fixed set of rules.

## Type Promotion

When an operator is used with operands of different data types, CREXX automatically converts one or both operands to a 
common, compatible type before performing the operation. This process is called **type promotion** and occurs 
at compile time.

#### Summary of Promotion Rules

The promotion logic follows a clear hierarchy. When combining two different types, the result is promoted to the type that is higher in the following list:

1.  **`.object`**: Any operation involving an `.object` results in an `.object`. The operation is delegated to the object itself.
2.  **`.decimal`**: The highest-precision numeric type. Any operation involving a `.decimal` and any other numeric or string type results in a `.decimal`.
3.  **`.float`**: Any operation involving a `.float` and an `.integer` or `.boolean` results in a `.float`.
4.  **`.integer`**: Any operation involving an `.integer` and a `.boolean` results in an `.integer`.
5.  **`.boolean`**: The lowest-level numeric-like type.

Special cases:

* **`.string`**: When a `.string` is used in a numeric context, it is converted to a number, and the rules above apply (typically resulting in a `.float` or `.decimal`).
* **`.binary`**: This type is generally isolated and will cause a compile-time error if mixed with numeric or string types.

#### Detailed Promotion Matrix

For a complete and exhaustive reference, the full promotion logic is defined by the matrix used internally by the compiler.

*(Invalid/disallowed promotions are marked with '—')*

| **Operand 1**   | **.void**  | **.boolean** | **.integer** | **.float** | **.decimal** | **.string** | **.binary** | **.object** |
|:----------------|:-----------|:-------------|:-------------|:-----------|:-------------|:------------|:------------|:------------|
| **.void**       | `.void`    | `.boolean`   | `.integer`   | `.float`   | `.decimal`   | `.float`    | `.binary`   | `.object`   |
| **.boolean**    | `.boolean` | `.boolean`   | `.integer`   | `.float`   | `.decimal`   | `.float`    | —           | `.object`   |
| **.integer**    | `.integer` | `.integer`   | `.integer`   | `.float`   | `.decimal`   | `.float`    | —           | `.object`   |
| **.float**      | `.float`   | `.float`     | `.float`     | `.float`   | `.decimal`   | `.float`    | —           | `.object`   |
| **.decimal**    | `.decimal` | `.decimal`   | `.decimal`   | `.decimal` | `.decimal`   | `.decimal`  | —           | `.object`   |
| **.string**     | `.float`   | `.float`     | `.float`     | `.float`   | `.decimal`   | `.float`    | —           | `.object`   |
| **.binary**     | `.binary`  | —            | —            | —          | —            | —           | `.binary`   | `.object`   |
| **.object**     | `.object`  | `.object`    | `.object`    | `.object`  | `.object`    | `.object`   | `.object`   | `.object`   |

## Taken Constants

Taken [as a] Constants are symbols that are unassigned REXX variables, these are read-only and have a value equal to their name.

If a symbol is used as a constant it cannot be subsequently used as a variable:

*EXAMPLE:*

```rexx
SAY CONSTANT_SYM; /* Prints "CONSTANT_SYM" */
CONSTANT_SYM = 1; /* "CONSTANT_SYM is a constant" error */
```

## Arrays

These look like STEMS but actually are (at their simplistic default) 1 based dynamic arrays, with `.0` giving the array size.

Designed to be simple and fast and cover many basic use cases.

*EXAMPLE:*

```rexx
array.1 = "Value"
array.2 = "Last Value"
```

In this case `array.0` has value 2.

This usage implicitly declares the array as 1-dimensional, 1-base array, which dynamically grows, and has type string.

A 2-dimensional array of type float could be implicitly declared with:

```rexx
array.1.1 = 0.0
```

### Alternative Square Bracket Syntax

Square brackets can be used e.g. `array[2]` or `array.2` are equivalent.

### Explicit Declaration {#explicit-declaration}

Arrays can be explicitly declared:

```rexx
array = .integer[10]            /* 1 Dimensional, 1 base, 10 elements */
array = .integer[10,10]         /* 2 Dimensional, 1 base, 10x10 elements */
array = .integer[0 to 10]       /* 1 Dimensional, 0 base, 11 elements */
array = .integer[-2 to *]       /* 1 Dimensional, -2 base, dynamic growth (-2, -1, 0, 1, ...) */
array = .string[0 to 5, 4 to 10]
```

Arrays are Objects (see next and the Classes and Interfaces section) and can be passed to and from functions.

## Objects {#objects}

CREXX Registers contain objects with certain physical attributes, defined below. In fact all variables, even simple values are stored as objects.

Classes and Interfaces are templates that define the properties and methods of an object. These templates provide the capability for REXX programs to use objects. See the Classes and Interfaces section for details.

For CREXX Level B the object design is required to ensure that the low level REXX VM capabilities are available to programmers from Level B programs.

## Object Physical Attributes {#object-physical-attributes}

Each Object has the following physical attributes. REXX and RXAS programs can use these physical attributes to create a logical view of an object’s “userland” attributes and methods.

Note that all these values can be used concurrently. However CREXX protocols need to be understood, for example the object’s string_value is used for string conversions from the object’s numeric values by the compiler.

* **Status** - 64 bit field to store value status flags. These attributes will be defined here and need to be consistent across the compiler, assembler, virtual machine
* **Int_value** - 64 bit integer
* **Float_value** - double float
* **Decimal_value** - arbitrary precision maths value (TODO)
* **Decimal_value_precision** - precision information on the above (TODO)
* **String_value** - text in UTF8. It can be null terminated, and RXAS instructions exist to ensure the text is null terminated where required
* **String_length** - text length in bytes
* **String_pos** - current text position in bytes from the text start
* **String_chars** - text length in UTF codepoints
* **String_char_pos** - current text position in UTF codepoints from the text start
* **Binary_value** - Binary data
* **Binary_length** - Binary data length in bytes
* **Attributes** - An array of child objects (which can be nested to any level). Attributes are the capability used by the compiler to store REXX class logical attributes. They are also used to store members of an array
* **Num_attributes** - The number of child objects

## CREXX VM Objects {#crexx-vm-objects}

The following are objects created and used by the Rexx VM.

### Caller Address {#caller-address}

Both passed to an interrupt handler and also returned by the *metaloadcalleraddr* assembler instruction.

* Attribute 1. int_value = return module number
* Attribute 2. int_value = return instruction address (in module binary space)

### Loaded Modules {#loaded-modules}

Returned by *metaloadedmodules* assembler instruction.

* Array of module names

### Loaded Procedures {#loaded-procedures}

Returned by the *metaloadedprocs* assembler instruction

An array of objects, each object has

* Attribute 1. string_value = name of the procedure
* Attribute 2. int_value = pointer to the private CREXX procedure block

### Decoded Opcode {#decoded-opcode}

Returned by the *metadecodeinst* assembler instruction

* Attribute 1. Int_value = opcode
* Attribute 2. String_value = instruction
* Attribute 3. String_value = description
* Attribute 4. Int_value = number of operands
* Attribute 5. Int_value - operated 1 type
* Attribute 6. Int_value - operated 2 type
* Attribute 7. Int_value - operated 3 type

### Procedure Details {#procedure-details}

Returned by the *metaloadoperand* assembler instruction (TODO review name).

* Int_value = function name

TODO needs to do more that get the function name - a function object is needed

### Metadata {#metadata}

Returned by the *metaloaddata* assembler instruction.

An array of objects for all the metadata returned by the instruction, meaning for a code address. The array has the following types of objects

*META_SRC:*

* String_value = ".meta_src"
* Attribute 1. Int_value = source line
* Attribute 2. Int_value = source column
* Attribute 3. String_value = source code

*META_FILE:*

* String_value = ".meta_file"
* Attribute 1. String_value = source file name

*META_FUNC:*

* String_value = ".meta_func"
* Attribute 1. String_value = function symbol
* Attribute 2. String_value = option (e.g. “b”)
* Attribute 3. String_value = function return type
* Attribute 4. String_value = argos
* Attribute 5. Int_value = function pointer
* Attribute 6. String_value = inline string

*META_REG:*

* String_value = ".meta_reg"
* Attribute 1. String_value = variable symbol
* Attribute 2. String_value = option (e.g. “b”)
* Attribute 3. String_value = variable type
* Attribute 4. Int_value = register number

*META_CONST:*

* String_value = ".meta_const"
* Attribute 1. String_value = variable (constant) symbol
* Attribute 2. String_value = option (e.g. “b”)
* Attribute 3. String_value = type
* Attribute 4. String_value = constant value

*META_CLEAR:*

* String_value = ".meta_clear"
* Attribute 1. String_value = variable symbol (moving out of scope)

### Redirect(s) {#redirect(s)}

Used by spawn assembler instructions for IO redirects. Each object is a pointer to an opaque REDIRECT binary structure 
(e.g. generated by assembler instruction *redrtoarr*)

The array structure holds three redirects: input, output, error

### Signal {#signal}

Raised signal object

* int_value == Signal code
* string_value = Any other signal context
