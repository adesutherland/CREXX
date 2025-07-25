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

Variables can also be declared and initialized by class name.

* Integer Variable \= .INT where INT is the integer built in type (value 0).
* Class Instance \= .ACLASS Class with via the default factory
* Class Instance \= .ACLASS() Class with via the default factory
* Class Instance \= .ACLASS(ARG1,ARG2) via a factory function with 2 arguments

Once a variable has been assigned a type, then that type cannot be changed.
