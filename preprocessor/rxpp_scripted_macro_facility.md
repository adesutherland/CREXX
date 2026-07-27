# RXPP Scripted Macros

## Overview

RXPP supports two complementary macro mechanisms:

* **C-style macros** (`##DEFINE`) perform simple textual substitution and are ideal for inline code templates.
* **Scripted macros** (`##MACRO`) execute RexxScript during preprocessing and generate CREXX source dynamically.

Scripted macros are intended for code generation tasks that cannot be expressed conveniently using simple parameter substitution. Typical examples include loops, conditional code generation, table-driven code, and source transformations.

The generated source becomes part of the CREXX program and is compiled normally after preprocessing.

---

# Defining a Scripted Macro

A scripted macro begins with `##MACRO` and ends with `##MEND`.

```rexx
##MACRO factorial name, upto

   .gen &name=1
   do i=2 to upto
      .gen &name=&name*&i
   end

##MEND
```

The parameter list follows the macro name and uses the same positional and keyword syntax as C-style macros.

---

# Calling a Scripted Macro

A scripted macro is invoked using the `##` prefix.

```rexx
##factorial result,5
```

During preprocessing, RXPP executes the macro, which generates CREXX source.

The example above produces

```rexx
result=1
result=result*2
result=result*3
result=result*4
result=result*5
```

The generated code is then compiled together with the remainder of the program.

---

# Execution Model

When a scripted macro is invoked, RXPP executes the macro as RexxScript during preprocessing.

The macro may perform arbitrary calculations, loops and conditional processing. Generated CREXX source is emitted using `.GEN` or deferred using `.TAIL`.

After preprocessing completes, the generated source is compiled together with the original program.

---

# The `.GEN` Statement

`.GEN` emits a single line of CREXX source immediately.

Unlike `SAY`, which writes diagnostic output to the console during preprocessing, `.GEN` appends the generated line to the output program.

```rexx
.gen say "Hello World"
```

produces

```rexx
say "Hello World"
```

in the generated source.

---

# The `.TAIL` Statement

`.TAIL` appends a line of generated CREXX source to the end of the current translation unit.

Unlike `.GEN`, which emits source immediately, `.TAIL` delays output until preprocessing has completed. This is useful for generating declarations, registration tables, initialization code, cleanup code, or any source that must appear after the main generated code.

```rexx
.tail say "Generated at end of source"
```

If multiple `.TAIL` statements are executed, they are appended in the order in which they were generated.

---

# Variable Substitution

Within a `.GEN` or `.TAIL` statement, references beginning with `&` are replaced by the value of the corresponding RexxScript variable.

```rexx
name='counter'

.gen &name=&name+1
```

produces

```rexx
counter=counter+1
```

The `&` substitution mechanism is recognized only within `.GEN` and `.TAIL` statements.

---

# `QUOTE()` and `UNQUOTE()`

`QUOTE()` and `UNQUOTE()` normalize the representation of a macro parameter before it is inserted into generated source.

* `QUOTE()` ensures that a value is enclosed in quotes.
* `UNQUOTE()` ensures that no surrounding quotes are present.

Both functions are **idempotent** and leave already normalized values unchanged.

The macro processor cannot determine whether a parameter represents an identifier, an expression or a string literal. This decision belongs to the macro programmer, who must choose the appropriate normalization according to how the parameter will be used in the generated source.

## Using `UNQUOTE()`

Use `UNQUOTE()` when a parameter is inserted as an identifier or when it is embedded inside an already quoted string.

```rexx
const=UNQUOTE(const)

.gen &const=.int[]
.gen say "Generating &const"
```

If

```rexx
const="first"
```

the generated source becomes

```rexx
first=.int[]
say "Generating first"
```

## Using `QUOTE()`

Use `QUOTE()` when a parameter is inserted as a standalone string literal in the generated source.

```rexx
message=QUOTE(message)

.gen say &message
```

If

```rexx
message=Hello World
```

the generated source becomes

```rexx
say "Hello World"
```

Without `QUOTE()`, the generated source would become

```rexx
say Hello World
```

which CREXX interprets as identifiers rather than a string literal.

---

# Built-in Variables

Several RexxScript variables are maintained automatically by the macro processor.

| Variable  | Description                                                          |
| --------- | -------------------------------------------------------------------- |
| `_mcalls` | Number of invocations of the current scripted macro.                 |
| `_sysndx` | Global count of all scripted macro invocations during preprocessing. |

Example

```rexx
.gen say "Macro call=&_mcalls"
.gen say "Global macro count=&_sysndx"
```

might generate

```rexx
say "Macro call=3"
say "Global macro count=17"
```

---

# Typical Uses

Scripted macros are particularly useful for

* generating repetitive code
* generating loops
* compile-time calculations
* compile-time validation
* creating lookup tables
* generating property accessors
* building parser tables
* generating serialization code
* source transformations

---

# Example: Enumerations

```rexx
##MACRO ENUM upto,const,cmt

   const=UNQUOTE(const)
   cmt=UNQUOTE(cmt)

   .gen say "Enumerate &const up to &upto"

   .gen &const=.int[]

   do i=1 to upto

      .gen &const[&i]=&i
      .gen say "Generating &const=&i &cmt"

      .tail say "Tail generated for &const=&i"

   end

##MEND
```

Invocation

```rexx
##ENUM 3,first,"Example"
```

may generate

```rexx
say "Enumerate first up to 3"

first=.int[]

first[1]=1
say "Generating first=1 Example"

first[2]=2
say "Generating first=2 Example"

first[3]=3
say "Generating first=3 Example"

say "Tail generated for first=1"
say "Tail generated for first=2"
say "Tail generated for first=3"
```

---

# Example: Properties

```rexx
##MACRO PROPERTY name,type

   .gen private &name=&type

   .gen method get&name()
   .gen    return &name
   .gen end

   .gen method set&name(value=&type)
   .gen    &name=value
   .gen end

##MEND
```

Invocation

```rexx
##PROPERTY CustomerName,.string
```

may generate

```rexx
private CustomerName=.string

method getCustomerName()
   return CustomerName
end

method setCustomerName(value=.string)
   CustomerName=value
end
```

---

# Summary

Scripted macros combine the simplicity of RXPP with the expressive power of RexxScript.

* `##MACRO` defines a scripted macro.
* Scripted macros execute during preprocessing.
* `.GEN` emits generated CREXX source immediately.
* `.TAIL` appends generated CREXX source after normal generation has completed.
* `&variable` inserts the value of a RexxScript variable.
* `QUOTE()` ensures that generated source contains a quoted value.
* `UNQUOTE()` ensures that generated source contains an unquoted value.
* The intended use of a parameter (identifier, expression or string literal) is determined by the macro programmer.
* The generated code is compiled as ordinary CREXX source.
