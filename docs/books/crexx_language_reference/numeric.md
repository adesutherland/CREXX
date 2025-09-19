# NUMERIC Instruction: Controlling Arithmetic Behaviour

\rexx{} uses *arbitrary-precision decimal arithmetic*, which requires specific rules for computation. 
The `NUMERIC` instruction controls the *precision*, *rounding*, and *comparison* for all 
numeric operations within a procedure. In \crexx{} Level B, its usage is slightly restricted to enable 
performance optimizations.

This section covers the `NUMERIC` instruction, including the new `STANDARD` sub-option. 
While `NUMERIC` controls semantic aspects, the `OPTIONS {NUMERIC_CLASSIC | NUMERIC_COMMON}` 
instruction modifies the parser's behavior for the entire source file, affecting fundamental 
syntactic aspects like operator precedence. The `NUMERIC` instruction, even with its `STANDARD` 
sub-option, does **not** change the operator precedence or associativity rules set by the 
file-level `OPTIONS` instruction.

## Syntax

The `NUMERIC` instruction must be the very first instruction in a procedure, right after the procedure's label, to enable 
performance optimizations. It can specify `DIGITS`, `FUZZ`, `FORM`, or the new `STANDARD` option 
with a constant value, or it can explicitly request to inherit settings from the calling procedure.

```rexx <!--numeric1.rexx-->
NUMERIC [ DIGITS [ <constant_value> | INHERITED ] ]
        [ FORM [ SCIENTIFIC | ENGINEERING | INHERITED ] ]
        [ FUZZ [ <constant_value> | INHERITED ] ]
        [ CASE [ UPPER | LOWER | INHERITED ] ]
        [ STANDARD [ CLASSIC | COMMON | INHERITED ] ]
```

* **\<constant\_value\>**: A literal number (e.g., 18, 0).
* **SCIENTIFIC | ENGINEERING**: Keywords for exponential notation.
* **CLASSIC | COMMON**: Keywords for arithmetic semantics.
* **UPPER | LOWER**: Keywords for case sensitivity in numeric conversion (`e`, `inf`, `nan`)
* **INHERITED**: A special keyword that inherits a specific setting from the caller's context.

## Effect

The `NUMERIC` instruction controls numeric behavior for all operations within its procedure. Each setting (`DIGITS`, 
`FUZZ`, `FORM`, `CASE`, `STANDARD`) can be specified only once with a constant value and cannot be changed 
dynamically.

### NUMERIC DIGITS

Sets the number of significant digits for all calculations.

* **Default**: 18 for \crexx{} Level B. Classic \rexx{} default is 9.
* **Value**: Must be a positive whole number greater than `NUMERIC FUZZ`.
* **Retrieval**: Use the `DIGITS()` built-in function.

### NUMERIC FORM

Sets the preferred exponential notation format.

* **Options**: `SCIENTIFIC` (default) or `ENGINEERING`.
    * **SCIENTIFIC**: One non-zero digit before the decimal point.
    * **ENGINEERING**: Power of ten is a multiple of three.
    * **INHERITED**: The procedure inherits the format from the caller's context.
* **Retrieval**: Use the `FORM()` built-in function.

### NUMERIC FUZZ

Sets the number of digits to ignore during numeric comparisons.

* **Default**: 0.
* **Value**: Must be zero or a positive whole number smaller than `NUMERIC DIGITS`.
* **INHERITED**: The procedure inherits the fuzz value from the caller's context.
* **Retrieval**: Use the `FUZZ()` built-in function.

### NUMERIC CASE

Sets the case sensitivity for special numeric literals.

* **Options**: `UPPER` or `LOWER` (default).
    * **UPPER**: Generates `E`, `INF`, `NAN`.
    * **LOWER**: Generates `e`, `inf`, `nan`.
    * **INHERITED**: The procedure inherits the case setting from the caller's context.

### NUMERIC STANDARD

Selects a predefined set of arithmetic semantic rules. This is a new option.

* **CLASSIC**: Adheres to classic ANSI \rexx{} X3.274-1996 rules. This is the default for \crexx{} Level C.
    * **Remainder (`//` or '%')**: The division is calculated at full (digits) division and truncated, then the remainder 
      is computed, ('a - (TRUNC(a / b) * b)'). 
    * **Integer Magnitude-Precision Constraint**: The integer quotient for `%` and `//` must fit within 
      `NUMERIC DIGITS` **without exponential notation**, or a `SYNTAX Error 26.11` is raised.
    * **Rounding**: Uses traditional "round-half-up."
    * **Integer Division (`%`)**: The numbers are divided at current precision (digits) and then truncated
      to an integer towards zero.
  
* **COMMON**: Employs semantics closer to other C like languages. This is the default for \crexx{} 
  Level B.
    * **Remainder (`%` or '//')**: The division is calculated at full (digits) division and truncated, then the remainder
      is computed, ('a - (TRUNC(a / b) * b)').
    * **Integer Magnitude-Precision Constraint**: Ignored. Quotients can exceed `NUMERIC DIGITS` and use exponential form.
    * **Rounding**: Uses "round-half-even."
    * **Division (`/`)**: Where the operands and target are all integers, the result is
      converted to an integer, raising a signal if the result is not an integer. Otherwise, normal float/decimal 
      division is performed.
    * **Integer Division ('%')**: The numbers are converted to integers, divided and truncated to an integer 
      towards zero.

* **INHERITED**: The procedure inherits arithmetic semantics from the caller's context.

* **Retrieval**: Use the `STANDARD()` built-in function.

## Interaction with OPTIONS

The `OPTIONS {NUMERIC_CLASSIC | NUMERIC_COMMON}` instruction sets the default arithmetic rules for the entire file. 
It influences how the CREXX parser builds the syntax tree, affecting operator precedence and 
associativity globally.

**`NUMERIC STANDARD` does not alter these file-level parsing rules.** For example:

* **Associativity (CLASSIC)**
    * `say -3**2` evaluates as `(-3)**2`, resulting in `9`.
    * `say 2**2**3` evaluates as `(2**2)**3`, resulting in `64`.
* **Associativity (COMMON)**
    * `say -3**2` evaluates as `-(3**2)`, resulting in `-9`.
    * `say 2**2**3` evaluates as `2**(2**3)`, resulting in `256`.
* **Remainder token** 
    * The specific token used for a remainder (`//` vs `%`)

These behaviors, set by `OPTIONS`, are constant for the file, regardless of `NUMERIC STANDARD` settings 
within procedures.

## Runtime Optimization Considerations

\crexx{} benefits from knowing arithmetic rules at compile time. Declaring `NUMERIC` settings as constants at the start 
of a procedure allows the compiler to:

* Embed numeric context values directly into the bytecode.
* Avoid runtime overhead from dynamic lookups.
* Enable specialized, optimized code paths.

Using `INHERITED` prevents these compile-time optimizations because the numeric context isn't 
known until runtime. This is a trade-off for flexibility at the cost of performance.

