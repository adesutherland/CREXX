# Literals

A literal is a fixed value that appears directly in a program's source code. It can be a string of characters, a numeric value, or a Boolean value. Literals are used to represent constant data that does not change during the execution of a program.

## String Literals

A string literal is a sequence of characters enclosed in single or double quotes. It represents a constant value in a program.

## Numeric Literals

Numeric literals can be integers, floating-point numbers, or hexadecimal numbers. Integers are whole numbers that do not have a decimal point. Floating-point numbers are numbers that have a decimal point.

## Hex/Binary Literals

A string literal that concludes with `x`, such as `"4142"x`, is decoded as
hexadecimal bytes. A string literal that concludes with `b`, such as
`"0100000101000010"b`, is decoded as binary bits. Hex literals use two hex
digits per byte. Binary literals use eight bits per byte.

If the decoded bytes are valid UTF-8, the literal is text by default:

```rexx
text = "4142"x        /* "AB" */
also = "01000001"b    /* "A" */
```

If the decoded bytes are not valid UTF-8, the literal must be used in an
explicit `.binary` context:

```rexx
payload = .binary
payload = "00ff41"x

other = "ffff"x as .binary
```

A first untyped assignment such as `payload = "ffff"x` is rejected because the
compiler treats the first binding as a text context unless `.binary` is made
explicit. The same is true for `payload = "ffff"x as .string`.

Converting ordinary text to `.binary` stores the string's UTF-8 bytes exactly,
without normalization:

```rexx
bytes = "α" as .binary    /* ce b1 */
```

Converting binary bytes back to text requires an explicit `.string` cast and
valid UTF-8:

```rexx
text = ("4142"x as .binary) as .string    /* "AB" */
```

If the bytes are not valid UTF-8, a constant cast is rejected by the compiler and
a runtime binary-to-string cast raises `UNICODE_ERROR`.
