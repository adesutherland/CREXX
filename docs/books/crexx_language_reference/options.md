# The `OPTIONS` Instruction

The `OPTIONS` instruction configures fundamental parsing rules and default behaviours for an entire cREXX
source file. It must be the very first instruction in a file, preceding all other statements.
This allows the compiler to establish the correct environment before tokenising and parsing the rest of the code.

## Syntax

All settings are specified on a single `OPTIONS` line, with options separated by spaces.

`OPTIONS option1 [option2] [option3] ...`

**Example:**

```rexx
options levelb numeric_common comments_slash floats_decimal
```

This example configures the file for Level B, sets the arithmetic rules to align with common standards across 
languages, enables `//` style line comments, and specifies that floating-point variables should be treated as 
decimal types.

## Language Level

This option determines the core feature set, syntax rules, and default behaviours for the program.

* `levelb`: The foundational system language for cREXX. It is statically typed, emphasises safety and
  efficiency, and provides direct access to low-level VM capabilities. Its default arithmetic standard 
  is `numeric_common`.
* `levelc`: Embodies "Classic" REXX, aiming for strict adherence to the ANSI X3.274-1996 standard. Its 
  default arithmetic standard is `numeric_classic`.
* `leveld`: Extends Level C with new features, such as the `USE` statement.
* `levelg`: A modern, general-purpose REXX that builds upon Level B syntax.
* `levell`: A specialised version for language engineering, providing extended `PARSE` capabilities to 
  handle PEG grammars and native support for structures like Abstract Syntax Trees (ASTs). 

**Default Behaviour**: If the `OPTIONS` instruction is omitted or does not specify a language level, 
the compiler assumes `levelc` (Classic REXX). *Note: Level C is not yet implemented, and the default may 
change to Level G in the future.*

## Arithmetic Standard

This option modifies the parser's behaviour for arithmetic expressions throughout the entire file.

It has two primary effects:

1. Global Parser Configuration: This instruction directly influences how the CREXX parser processes
   arithmetic expressions. This means operators, operator precedence, and associativity rules (see below).
3. Default NUMERIC STANDARD: It implicitly sets the default NUMERIC STANDARD for any procedure within that file that
   does not explicitly specify a NUMERIC STANDARD itself.

The options are:

* `numeric_classic`: (Default for `levelc`) Adheres to the classic ANSI REXX rules.

    * **Precedence**: The prefix minus (`-`) has a *higher* priority than the power operator (`**`). 
      `SAY -3**2` evaluates as `(-3)**2`, resulting in `9`.
    * **Associativity**: The power operator is *left-associative*. `SAY 2**2**3` evaluates as `(2**2)**3`, 
      resulting in `64`.
    * **Remainder Operator**: The remainder operator is ('//').
    * **Integer Division**: The integer division operator is (`%`).
   

* `numeric_common`: (Default for `levelb`) Adheres to rules common in C-like languages.

    * **Precedence**: The prefix minus (`-`) has a *lower* priority than the power operator (`**`). 
      `SAY -3**2` evaluates as `-(3**2)`, resulting in `-9`.
    * **Associativity**: The power operator is *right-associative*. `SAY 2**2**3` evaluates as 
      `2**(2**3)`, resulting in `256`.
    * **Remainder Operator**: The remainder operator is ('%').
    * **Integer Division**: The division operator is ('/'), and integer division is performed if the operands are integers.

*Note: The arithmetic standard is an experimental feature and may be subject to change.*

## Comment Style

These options enable or disable specific single-line comment formats.

* `comments_hash` / `comments_nohash`: Controls `#` style comments. This is the **default enabled** format 
   and allows for the use of a POSIX `#!` shebang on the first line.
* `comments_slash` / `comments_noslash`: Controls `//` style comments. The **default is disabled**.
* `comments_dash` / `comments_nodash`: Controls `--` style comments. The **default is disabled**.

## Floating-Point Type (Level B)

This option defines how `.float` variables are treated internally in Level B programs.

* `floats_binary`: (Default) Treats floats as binary floating-point types (e.g., C `double`).
* `floats_decimal`: Treats floats as decimal floating-point types, using a decimal-native representation 
   for higher precision and to avoid binary conversion artefacts.
