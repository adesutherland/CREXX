# Variables {#variables}

## Keywords

Unlike in classic \rexx{}, keywords (instructions and operators) cannot be used for variables or constant names.

## Typed Variables

The approach to typing variables is designed to be simple and as classic \rexx{}-like as possible but introduce type safety which allows more bugs to be highlighted before runtime. <!-- This is achieved by allowing -->

## Declaration

Variables will be implicitly declared and initialised to be of a certain type on their first assignment. E.g

* Integer: Variable \= 0;
* Float: Variable \= 0.0;
* String: Variable \= ""

Variables can also be declared and initialized using a constructor-style syntax for both built-in fundamental types and user-defined classes:

* `Variable = .int(value)` : Declares an integer and initializes it to `value`.
* `Variable = .string("text")` : Declares a string and initializes it to `"text"`.
* `Variable = .float(1.23)` : Declares a float and initializes it to `1.23`.
* `Variable = .boolean(1)` : Declares a boolean and initializes it to `1`.
* `Variable = .decimal(1.23d)` : Declares a decimal and initializes it to `1.23d`.

Other initialization forms include:

* Integer Variable \= .INT where INT is the integer built in type (value 0).
* Class Instance \= .ACLASS Class with via the default factory
* Class Instance \= .ACLASS() Class with via the default factory
* Class Instance \= .ACLASS(ARG1,ARG2) via a factory function with 2 arguments

Once a variable has been assigned a type, then that type cannot be changed.
