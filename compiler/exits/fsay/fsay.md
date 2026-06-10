#  FSAY and FSAYFMT

## Overview

`fsayfmt` is a template-based formatting facility for CREXX.

It allows values to be embedded directly into text using named placeholders. The
facility supports field widths, alignment, and numeric formatting while keeping
the formatting specification close to the text being generated.

The same formatting engine is also available through the compiler-exit
statement:

```rexx
FSAY template
```

`FSAY` is a convenience syntax that formats the template using `fsayfmt()` and
outputs the resulting line.

Unlike `fmtmask` and `MSAY`, which use COBOL-inspired picture masks,
`fsayfmt` uses embedded placeholders similar to modern string interpolation
systems.

---

# Compiler Exit Statement: FSAY

## Syntax

```rexx
FSAY template
```

The template may contain one or more placeholders enclosed in braces.

Example:

```rexx
FSAY "Name: {name:<10} Qty: {qty:>3} Price: {price:8.2}"
```

If:

```rexx
name  = "Fred"
qty   = 12
price = 64.31
```

Output:

```text
Name: Fred       Qty:  12 Price:    64.31
```

### Equivalent Code

Conceptually, the previous statement expands to:

```rexx
say 'Name: '
    || left(name,10)
    || ' Qty: '
    || right(qty,3)
    || ' Price: '
    || format(price,8,2)
```

### Notes

* `FSAY` is a compiler-exit convenience syntax.
* `fsayfmt()` performs the template expansion.
* `FSAY` automatically outputs the resulting line using `SAY`.
* Placeholder processing rules are identical for `FSAY` and `fsayfmt()`.

---

# Function Syntax

```rexx
expression = fsayfmt(template)
```

`fsayfmt()` converts a template into an equivalent CREXX string expression.

Example:

```rexx
expr = fsayfmt("Name: {name:<10}")
```

Conceptually produces:

```rexx
'Name: ' || left(name,10)
```

---

# Template Structure

A template consists of:

* Literal text
* Placeholders

Literal text is copied unchanged.

Placeholders are enclosed in braces:

```text
{name}
```

or

```text
{name:format}
```

Example:

```rexx
FSAY "Customer: {customer}"
```

Result:

```text
Customer: Smith
```

---

# Placeholder Format Specification

General syntax:

```text
{variable[:alignment][width][.decimals]}
```

where:

| Element   | Meaning                     |
| --------- | --------------------------- |
| variable  | Variable name to insert     |
| alignment | Optional alignment modifier |
| width     | Optional field width        |
| decimals  | Optional decimal precision  |

Examples:

```text
{name}
{name:10}
{name:<10}
{name:>10}
{name:^10}
{price:8.2}
{price:<8.2}
```

---

# Variable Substitution

The simplest form inserts a variable value unchanged.

Template:

```text
{name}
```

Example:

```rexx
FSAY "Hello {name}"
```

If:

```rexx
name = "Fred"
```

Result:

```text
Hello Fred
```

---

# Width Formatting

A width may be specified after the variable name.

Template:

```text
{name:10}
```

Example:

```rexx
FSAY "|{name:10}|"
```

Result:

```text
|Fred      |
```

The field width specifies the minimum width of the generated text.

---

# Alignment

Three alignment modes are supported.

| Specifier | Meaning      |
| --------- | ------------ |
| `<`       | Left align   |
| `>`       | Right align  |
| `^`       | Centre align |

---

## Left Alignment

Template:

```text
{name:<10}
```

Equivalent to:

```rexx
left(name,10)
```

Result:

```text
Fred
```

(with trailing spaces)

---

## Right Alignment

Template:

```text
{name:>10}
```

Equivalent to:

```rexx
right(name,10)
```

Result:

```text
      Fred
```

---

## Centre Alignment

Template:

```text
{name:^10}
```

Equivalent to:

```rexx
center(name,10)
```

Result:

```text
   Fred
```

---

# Numeric Formatting

Numeric formatting uses CREXX `FORMAT()` semantics.

Syntax:

```text
{value:width.decimals}
```

Example:

```text
{price:8.2}
```

Equivalent to:

```rexx
format(price,8,2)
```

If:

```rexx
price = 123.45
```

Result:

```text
  123.45
```

---

# Numeric Formatting with Alignment

Alignment may be combined with decimal formatting.

## Left-Aligned Numeric

```text
{price:<8.2}
```

Equivalent to:

```rexx
left(format(price,8,2),8)
```

---

## Right-Aligned Numeric

```text
{price:>8.2}
```

Equivalent to:

```rexx
format(price,8,2)
```

---

## Centre-Aligned Numeric

```text
{price:^8.2}
```

Equivalent to:

```rexx
center(format(price,8,2),8)
```

---

# Placeholder Summary

| Placeholder    | Description                 |
| -------------- | --------------------------- |
| `{name}`       | Insert value                |
| `{name:10}`    | Width 10                    |
| `{name:<10}`   | Left align width 10         |
| `{name:>10}`   | Right align width 10        |
| `{name:^10}`   | Centre width 10             |
| `{value:8.2}`  | Numeric width 8, 2 decimals |
| `{value:<8.2}` | Left-aligned numeric        |
| `{value:>8.2}` | Right-aligned numeric       |
| `{value:^8.2}` | Centre-aligned numeric      |

---

# Colon and Space Separators

The formatter accepts either a colon or whitespace as the separator between the
variable name and the format specification.

The following forms are equivalent:

```text
{name:<10}
{name <10}
```

Likewise:

```text
{price:8.2}
{price 8.2}
```

Using the colon form is recommended because it is more readable and mirrors the
notation used by other formatting systems.

---

# Examples

## Simple Text

```rexx
FSAY "Customer: {customer}"
```

Output:

```text
Customer: Smith
```

---

## Report Line

```rexx
FSAY "Item: {item:<15} Qty: {qty:>5}"
```

Output:

```text
Item: Tea               Qty:     7
```

---

## Price List

```rexx
FSAY "{item:<20} {price:8.2}"
```

Output:

```text
Tea                     12.45
Coffee                   8.95
Sugar                    3.10
```

---

## Mixed Formatting

```rexx
FSAY "Customer: {cust:<20} Balance: {bal:10.2}"
```

Output:

```text
Customer: Acme Ltd            Balance:     1234.56
```

---

# Relationship to FMTMASK and MSAY

`fsayfmt` and `fmtmask` both provide formatted output facilities but use
different approaches.

## FMTMASK / MSAY

Uses COBOL-inspired picture masks.

Example:

```rexx
MSAY "Name: XXXXXXXXXX  Price: $$$$9.99",
     name,
     price
```

Advantages:

* Fixed report layouts
* Traditional picture-mask notation
* Familiar to COBOL and report-writer users

---

## FSAYFMT / FSAY

Uses embedded named placeholders.

Example:

```rexx
FSAY "Name: {name:<10} Price: {price:8.2}"
```

Advantages:

* Self-documenting templates
* Named variables
* Easier maintenance
* Readable formatting specifications
* Familiar interpolation-style syntax

---

# Design Goals

`fsayfmt` is intended to provide:

* Readable template-based formatting
* Named variable substitution
* Alignment support
* Numeric formatting support
* Efficient compiler-exit expansion

It is not intended to be:

* A full template language
* A report writer
* A replacement for sophisticated formatting frameworks

Instead, it provides a lightweight and practical formatting mechanism for CREXX
source code and compiler-generated output.
