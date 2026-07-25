# FSAY and FSAYFMT

## Overview

`FSAY` provides formatted string interpolation. It allows variable
names and their formatting instructions to be written directly in a template,
close to the surrounding text:

```rexx
name = "Fred"
qty = 12
price = 64.31

FSAY "Name: {name:<10} Qty: {qty:>3} Price: {price:8.2}"
```

The result is:

```text
Name: Fred       Qty:  12 Price:    64.31
```

Unlike a runtime interpolation facility, `FSAY` is a compiler-exit statement.
The compiler exit transforms the template into an ordinary cRexx string
expression and places that expression after `SAY`. The generated statement is
then compiled in the normal way.

Conceptually, the preceding statement becomes:

```rexx
say 'Name: ' || left(name,10) ||,
    ' Qty: ' || right(qty,3) ||,
    ' Price: ' || format(price,8,2)
```

This distinction is important. The template is analysed while the source
program is being compiled, but the variables are evaluated when the generated
statement runs. `FSAY` therefore combines concise template notation with the
normal name lookup, expression evaluation, and formatting functions of cRexx.

The transformation is performed by the Level B helper `fsayfmt`. That helper
returns cRexx source text; it does not itself evaluate the variables or produce
the final output string.

## The FSAY Statement

The statement has the following form:

```rexx
FSAY template
```

The template is a quoted string containing literal text and optional
placeholders. Literal text appears in the output unchanged. Each placeholder
names a variable whose value is inserted at that position. A placeholder can
also specify a field width, alignment, or decimal precision.

A simple placeholder performs interpolation without imposing a field width:

```rexx
name = "Fred"
FSAY "Hello, {name}"
```

This produces:

```text
Hello, Fred
```

At compilation time, the compiler exit passes the quoted template source token
to `fsayfmt`. The helper decodes one matching pair of outer quotes, scans the
template, and constructs a cRexx expression from its literal and variable
parts. The returned expression is inserted after `SAY` and compiled as part of
the program.

Consequently, `FSAY` does not capture the value of a variable at compilation
time. If a variable changes between two executions of the statement, the
current value is used each time:

```rexx
status = "starting"
FSAY "Status: {status}"

status = "complete"
FSAY "Status: {status}"
```

## Placeholders

A placeholder is enclosed in braces. Its general form is:

```text
{variable[:alignment][width][.decimals]}
```

Only the variable name is required. The remaining parts determine how its
value is presented:

* `alignment` is `<`, `>`, or `^`;
* `width` is a non-negative decimal integer; and
* `decimals` is a non-negative decimal integer following a period.

The colon separating the variable name from its format is optional when
whitespace is used instead:

```text
{name:<10}
{name <10}
```

Both forms describe the same ten-character, left-aligned field. The colon form
is usually more compact, while the whitespace form may be more readable in
dense templates.

The variable must be an identifier or compound variable. A placeholder is not
a general cRexx expression: operators, function calls, and array subscripts
cannot be placed inside it. Values requiring prior calculation should be
assigned to a variable before the `FSAY` statement:

```rexx
total = price * quantity
FSAY "Total: {total:10.2}"
```

Keeping placeholders restricted to names makes their parsing unambiguous and
keeps templates readable.

## Field Width and Alignment

When no format is present, the variable is inserted directly:

```text
{name}
```

Conceptually, this contributes `name` to the generated string expression. No
padding or truncation is requested by the placeholder itself.

A width reserves a field of the specified number of characters. A plain width
uses left alignment:

```text
{name:10}
```

This generates:

```rexx
left(name,10)
```

The same alignment can be stated explicitly with `<`:

```text
{name:<10}
```

Right alignment uses `>`:

```text
{qty:>3}
```

which generates:

```rexx
right(qty,3)
```

Centred text uses `^`:

```text
{heading:^20}
```

which generates:

```rexx
center(heading,20)
```

The alignment character therefore maps directly to the familiar cRexx string
functions:

```text
<   left
>   right
^   center
```

An explicit alignment character requires a width. Alignment has no useful
meaning without a field in which the value can be positioned, so forms such
as `{name:<}` are invalid.

### Alignment Examples

The following placeholders illustrate the generated expressions:

| Placeholder | Generated expression |
| --- | --- |
| `{name}` | `name` |
| `{name:10}` | `left(name,10)` |
| `{name:<10}` | `left(name,10)` |
| `{name:>10}` | `right(name,10)` |
| `{name:^10}` | `center(name,10)` |

For example, a common field width can be used to align a sequence of names:

```rexx
FSAY "Name: {name:<12} State: {state}"
```

The text following the name begins at the same position regardless of the
length of the current name, subject to the behaviour of `LEFT()`.

## Decimal Formatting

A decimal precision is introduced by a period. It requests numeric formatting
through `FORMAT()`:

```text
{price:8.2}
```

This generates:

```rexx
format(price,8,2)
```

Here, `8` is the field width and `2` is the number of digits after the decimal
point. Decimal formats default to right alignment, which is the customary
presentation for numeric columns.

The width may be omitted while retaining the decimal precision:

```text
{price:.2}
```

This generates:

```rexx
format(price,,2)
```

The omission leaves the overall width to `FORMAT()` while still fixing the
number of decimal places.

Explicit alignment can be combined with decimal formatting. Right alignment
uses the width directly in `FORMAT()`:

```text
{price:>8.2}
```

generates:

```rexx
format(price,8,2)
```

For left or centred presentation, the number is first formatted to the
requested precision without an overall numeric width. The resulting text is
then aligned:

```text
{price:<8.2}
```

generates:

```rexx
left(format(price,,2),8)
```

and:

```text
{price:^8.2}
```

generates:

```rexx
center(format(price,,2),8)
```

The complete set of decimal examples is therefore:

| Placeholder | Generated expression |
| --- | --- |
| `{price:8.2}` | `format(price,8,2)` |
| `{price:.2}` | `format(price,,2)` |
| `{price:<8.2}` | `left(format(price,,2),8)` |
| `{price:>8.2}` | `format(price,8,2)` |
| `{price:^8.2}` | `center(format(price,,2),8)` |

Widths and decimal precisions must be non-negative decimal integers. They are
part of the template notation and are fixed when the template is expanded;
they are not variable expressions evaluated at runtime.

## Literal Text and Braces

All characters outside placeholders are literal template text. During
expansion, `fsayfmt` turns each literal part into canonical single-quoted
cRexx source. Apostrophes in the template are doubled in the generated source
so that they remain literal apostrophes when the expression is compiled.
Unicode text is preserved.

Because single braces delimit placeholders, doubled braces are used when an
actual brace must appear in the output:

```rexx
FSAY "Set {{name}} to {value}"
```

If `value` contains `Fred`, the result is:

```text
Set {name} to Fred
```

Here, `{{` produces a literal opening brace and `}}` produces a literal closing
brace. The text between those doubled braces is not treated as a variable
reference.

This escaping rule allows templates to include notation that already uses
braces, while keeping an isolated `{name}` available for interpolation.

## Errors in Templates

The structure of an `FSAY` template is checked while the compiler exit expands
the statement. Malformed templates therefore fail during compilation rather
than producing partially interpolated output at runtime.

The following conditions signal `INVALID_ARGUMENTS`:

* an unmatched opening or closing brace;
* a nested opening brace;
* an empty placeholder;
* an invalid variable name;
* an empty format specification;
* an invalid field width;
* an invalid decimal precision; or
* an alignment character without a width.

Examples of invalid forms include:

```text
{}
{name:}
{name:<}
{name:ten}
{price:8.two}
```

These checks protect the generated source expression from ambiguous or
incomplete template notation. They also make errors local to the `FSAY`
statement that contains them.

## Direct Use of FSAYFMT

The typed Level B interface is:

```rexx
fsayfmt(template = .string) = .string
```

A direct call supplies decoded template text and receives a cRexx source
expression. For example:

```rexx
fsayfmt("Hello {name}")
```

returns source equivalent to:

```rexx
'Hello '||name
```

Similarly:

```rexx
fsayfmt("{name:10}")
```

returns:

```rexx
left(name,10)
```

This return value is source code, not the formatted result of evaluating
`name`. Direct use is consequently most relevant to compiler exits, source
generation, testing, and other tooling that needs to construct a cRexx
expression.

For compiler-exit use, one matching outer quoted source token is decoded before
the template is scanned. `fsayfmt` then makes a single scan over the decoded
template. It does not invoke the general quote-aware library.

## Choosing Between FSAY and MSAY

`FSAY` and `MSAY` both produce formatted output, but they express the layout in
different ways.

`FSAY` uses named placeholders:

```rexx
FSAY "Customer: {customer:<12} Balance: {balance:10.2}"
```

The name of each value appears at the point where the value is inserted. This
makes the template easy to read when it contains descriptive text, when values
appear in a different order from their declarations, or when a value appears
more than once.

`MSAY` and `fmtmask` use picture fields and consume separately supplied values
from left to right:

```rexx
MSAY "Customer: XXXXXXXXXXXX Balance: 9999999.99",
     customer, balance
```

This style is particularly useful for traditional fixed-width reports in which
the shape of each column is more important than the name of the value.

As a practical rule, use `FSAY` for readable named interpolation and local
formatting within prose-like messages. Use `MSAY` or `fmtmask` for
picture-driven layouts and repeated report lines whose fields are naturally
positional.

See the selector-level
[Level B contract](../../../lib/rxfnsb/rexx/fsayfmt.md) for direct-call and
test details.
