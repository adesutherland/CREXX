# Data Types

Types in the language are named using a period (.) followed by the type name. This naming convention helps to distinguish types from other elements in the language, such as variables and procedures.

## Built-In Types

The language includes a number of built-in types, which are provided by the language itself. These types include:

| Type       | Description                                                                 |
|------------|-----------------------------------------------------------------------------|
| `.void`    | Represents the absence of a value                                           |
| `.int`     | Represents an integer                                                       |
| `.float`   | Represents a floating-point number                                          |
| `.string`  | Represents a string of characters – Unicode codepoints, encoded as UTF-8    |
| `.binary`  | Represents an opaque binary value                                           |
| `.boolean` | Represents a boolean value                                                  |
| `.object`  | Represents the base class for all objects in the language                   |

Table: Built-in types {#tbl:id}

## Statically Typed

The language's type system is statically typed, which means that the types of variables and expressions are known at compile time. This helps to ensure that the language is type-safe, meaning that it is unlikely for programs to crash at runtime due to type errors.

REXX will automatically convert / promote variable types between Integer, Float and String \- this may cause a compile time or run time error (signal).

## Taken Constants

Taken \[as a\] Constants are symbols that are unassigned REXX variables, these are read-only and have a value equal to their name.

If a symbol is used as a constant it cannot be subsequently used as a variable:

*EXAMPLE:*

SAY CONSTANT\_SYM; /\* Prints "CONSTANT\_SYM" \*/

CONSTANT\_SYM \= 1; /\* "CONSTANT\_SYM is a constant" error \*/ 

## Arrays

These look like STEMS but actually are (at their simplistic default) 1 based dynamic arrays, with .0 giving the array size.

Designed to be simple and fast and cover many basic use cases.

*EXAMPLE:*

array.1 \= "Value"

array.2 \= "Last Value"

In this case

array.0 has value 2

This usage implicitly declares the array as 1-dimensional, 1-base array, which dynamically grows, and has type string.

A 2-dimensional array of type float could be implicitly declared with

array.1.1 \= 0.0

## Alternative Square Bracket Syntax

Square brackets can be used e.g.

array\[2\] 

or

array.2 

are equivalent.

## Explicit Declaration {#explicit-declaration}

Arrays can be explicitly declared

array \= .int\[10\]            /\* 1 Dimensional, 1 base, 10 elements \*/

array \= .int\[10,10\]         /\* 2 Dimensional, 1 base, 10x10 elements \*/

array \= .int\[0 to 10\]       /\* 1 Dimensional, 0 base, 11 elements \*/

array \= .int\[-2 to \*\]       /\* 1 Dimensional, \-2 base, dynamic growth (-2, \-1, 0, 1, ...) \*/

array \= .string\[0 to 5, 4 to 10\]  

Arrays are Objects (see next and the Classes and Interfaces section) and can be passed to and from functions.

## Objects {#objects}

CREXX Registers contain objects with certain physical attributes, defined below . In fact all variables, even simple values are stored as objects. 

Classes and Interfaces are templates that define the properties and methods of an object. These templates provide the capability for REXX programs to use objects. See the Classes and Interfaces section for details.

For CREXX Level B the object design is required to ensure that the low level REXX VM capabilities are available to programmers from Level B programs.

## Object Physical Attributes {#object-physical-attributes}

Each Object has the following physical attributes. REXX and RXAS programs can use these physical attributes to create a logical view of an object’s “userland” attributes and methods.

Note that all these values can be used concurrently. However CREXX protocols need to be understood, for example the object’s string\_value is used for string conversions from the object’s numeric values by the compiler. 

* Status \- 64 bit field to store value status flags. These attributes will be defined here and need to be consistent across the compiler, assembler, virtual machine  
* **Int\_value** \- 64 bit integer  
* **Float\_value** \- double float  
* **Decimal\_value** \- arbitrary precision maths value (TODO)  
* **Decimal\_value\_precision** \- precision information on the above (TODO)  
* **String\_value** \- text in UTF8. It can be null terminated, and RXAS instructions exist to ensure the text is null terminated where required  
* **String\_length** \- text length in bytes  
* **String\_pos** \- current text position in bytes from the text start  
* **String\_chars** \- text length in UTF codepoints  
* **String\_char\_pos** \- current text position in UTF codepoints from the text start  
* **Binary\_value** \- Binary data  
* **Binary\_length** \- Binary data length in bytes  
* **Attributes** \- An array of child objects (which can be nested to any level). Attributes are the capability used by the compiler to store REXX class logical attributes. They are also used to store members of an array  
* **Num\_attributes** \- The number of child objects

## CREXX VM Objects {#crexx-vm-objects}

The following are objects created and used by the Rexx VM.

### Caller Address  {#caller-address}

Both passed to an interrupt handler and also returned by the *metaloadcalleraddr* assembler instruction.

* Attribute 1\. int\_value \= return module number  
* Attribute 2\. int\_value \= return instruction address (in module binary space)

### Loaded Modules  {#loaded-modules}

Returned by *metaloadedmodules* assembler instruction.

* Array of module names

### Loaded Procedures {#loaded-procedures}

Returned by the *metaloadedprocs* assembler instruction

An array of objects, each object has

* Attribute 1\. string\_value \= name of the procedure  
* Attribute 2\. int\_value \= pointer to the private CREXX procedure block

### Decoded Opcode {#decoded-opcode}

Returned by the *metadecodeinst* assembler instruction

* Attribute 1\. Int\_value \= opcode  
* Attribute 2\. String\_value \= instruction  
* Attribute 3\. String\_value \= description  
* Attribute 4\. Int\_value \= number of operands  
* Attribute 5\. Int\_value \- operated 1 type  
* Attribute 6\. Int\_value \- operated 2 type  
* Attribute 7\. Int\_value \- operated 3 type

### Procedure Details {#procedure-details}

Returned by the *metaloadoperand* assembler instruction (TODO review name).

* Int\_value \= function name

 TODO needs to do more that get the function name \- a function object is needed

### Metadata {#metadata}

Returned by the *metaloaddata* assembler instruction.

An array of objects for all the metadata returned by the instruction, meaning for a code address. The array has the following types of objects

*META\_SRC:*

* String\_value \=  ".meta\_src"  
* Attribute 1\. Int\_value \= source line  
* Attribute 2\. Int\_value \= source column  
* Attribute 3\. String\_value \= source code

*META\_FILE:*

* String\_value \= ".meta\_file"  
* Attribute 1\. String\_value \= source file name

*META\_FUNC:*

* String\_value \=  ".meta\_func"  
* Attribute 1\. String\_value \= function symbol  
* Attribute 2\. String\_value \= option (e.g. “b”)  
* Attribute 3\. String\_value \= function return type  
* Attribute 4\. String\_value \= argos  
* Attribute 5\. Int\_value \= function pointer  
* Attribute 6\. String\_value \= inline string

*META\_REG:*

* String\_value \= ".meta\_reg"  
* Attribute 1\. String\_value \= variable symbol  
* Attribute 2\. String\_value \= option (e.g. “b”)  
* Attribute 3\. String\_value \= variable type  
* Attribute 4\. Int\_value \= register number

*META\_CONST:*

* String\_value \= ".meta\_const"  
* Attribute 1\. String\_value \= variable (constant) symbol  
* Attribute 2\. String\_value \= option (e.g. “b”)  
* Attribute 3\. String\_value \= type  
* Attribute 4\. String\_value \= constant value

*META\_CLEAR:*

* String\_value \= ".meta\_clear"  
* Attribute 1\. String\_value \= variable symbol (moving out of scope)

### Redirect(s) {#redirect(s)}

Used by spawn assembler instructions for IO redirects. Each object is a a pointer to an opaque REDIRECT binary structures (e.g. generated by assembler instruction  *redrtoarr*)

The array structure holds three redirects: input, output, error

### Signal {#signal}

Raised signal object 

* int\_value \== Signal code  
* string\_value \= Any other signal context

