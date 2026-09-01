# MSAY and FMTMASK

## Overview

`fmtmask` is a lightweight picture-mask formatter. It combines
literal text with fixed-width text and numeric fields, making it convenient
for producing aligned messages, summaries, and simple reports. Its mask
notation is inspired by the picture clauses and report-writing facilities of
COBOL, but provides a much smaller and simpler set of features.

The formatter is available in two forms. The `fmtmask()` function returns the
formatted text to the program:

```rexx
line = fmtmask(mask, value-1, value-2, ...)
```

The `MSAY` compiler-exit statement uses the same formatting engine and writes
the resulting line immediately:

```rexx
MSAY mask, value-1, value-2, ...
```

The choice between them depends on what must happen to the result.
`fmtmask()` is appropriate when the text must first be stored, modified,
logged, returned, or passed to another routine. `MSAY` provides a more concise
form when the only purpose of the formatted text is to display it.

For example:

```rexx
line = fmtmask(
    "Name: XXXXXXXXXX  Price: $$$$9.99  Qty: 999",
    "Fred",
    64.31,
    12
)

say line
```

produces:

```text
Name: Fred        Price:   $64.31  Qty:  12
```

The same operation can be written with `MSAY`:

```rexx
MSAY "Name: XXXXXXXXXX  Price: $$$$9.99  Qty: 999", ,
     "Fred", 64.31, 12
```

This is equivalent to:

```rexx
say fmtmask(
    "Name: XXXXXXXXXX  Price: $$$$9.99  Qty: 999",
    "Fred",
    64.31,
    12
)
```

## How a Mask Is Applied

A mask is a character string containing literal text and one or more field
descriptions. Literal text is copied to the result unchanged. A sequence of
`X` characters describes a text field, while a numeric field is described by
`9` characters, optionally combined with a decimal point and a leading
currency symbol.

The formatter scans the mask from left to right. Whenever it encounters a
field, it takes the next value argument, formats that value according to the
field description, and inserts the result. The values therefore correspond
to fields by position rather than by name.

Consider the following mask:

```rexx
mask = "Name: XXXXX  Qty: 999  Price: 999.99"
```

When it is used with these values:

```rexx
"Tea", 7, 12.45
```

the fields and values are associated as follows:

```text
XXXXX   -> "Tea"
999     -> 7
999.99  -> 12.45
```

The words `Name`, `Qty`, and `Price`, together with their punctuation and
spacing, are literal text. They appear in the result exactly as written in the
mask. Only the three picture fields consume values.

This positional processing makes masks compact and predictable. It also means
that the order and number of values must agree with the fields in the mask.
Changing the order of the fields requires the corresponding values to be
reordered as well.

## Text Fields

A contiguous sequence of `X` characters defines a text field. The number of
characters in the sequence determines the width of the field:

```text
XXXXXXXXXX
```

This mask describes a text field ten characters wide. Text is aligned on the
left. A value shorter than the field is padded with spaces on the right, while
a value longer than the field is truncated to the available width. The mask
therefore fixes both the position at which the text begins and the amount of
space it may occupy.

For example, formatting `Fred` with a ten-character field produces:

```text
Fred
```

The displayed text is followed by six spaces. Those spaces may not be visible
on their own, but they ensure that any later fields begin at a fixed position.
This is particularly useful for columns:

```rexx
MSAY "Name: XXXXXXXXXX  Department: XXXXXXXX", "Fred", "Sales"
```

If a value does not fit, it is shortened rather than allowed to disturb the
remainder of the line. Thus, a ten-character field containing
`Christopher` produces the first ten characters:

```text
Christophe
```

## Numeric Fields

Numeric fields are right-aligned so that values of different lengths line up
naturally in a column. Spaces are inserted on the left when a value occupies
less than the available width. Unlike text fields, numeric fields are not
silently truncated. If a numeric result cannot fit, the complete field is
replaced by asterisks, making the error visible in the output.

### Integer Fields

A contiguous sequence of `9` characters without a fractional part defines an
integer field. For example:

```text
999
```

defines a field three characters wide. The value is converted using `TRUNC()`
and then aligned on the right. Formatting the value `12` consequently gives:

```text
 12
```

The leading space occupies the unused position. Because `TRUNC()` is used,
this form discards any fractional part rather than rounding it for display.

If the value is too large for the field, the field is filled with asterisks:

```text
Mask   : 999
Value  : 12345
Result : ***
```

### Fixed-Decimal Fields

A numeric mask containing a decimal point followed by one or more `9`
characters defines a fixed-decimal field:

```text
9999.99
```

The number of `9` characters after the decimal point determines the number of
decimal places in the result. Formatting is performed using `FORMAT()`, after
which the result is aligned on the right within the complete field.

For example:

```text
Mask   : 9999.99
Value  : 64.31
Result :   64.31
```

The decimal point is part of the field width. In the preceding example the
complete field is seven characters wide: four positions before the point, the
point itself, and two positions after it. The mask fixes the displayed
precision as well as the alignment.

A decimal point is recognized as part of a numeric field only when it is
followed by at least one `9`. Consequently:

```text
999.99
```

is one fixed-decimal field, whereas:

```text
999.
```

is an integer field followed by a literal period. This distinction permits
ordinary punctuation to be placed directly after an integer field.

### Currency Fields

A numeric field may be preceded by repeated currency characters. The
following symbols are supported:

```text
$
€
£
¥
```

For example:

```text
$$$$9.99
```

describes a currency field whose total width includes all the currency
positions, the digit position, the decimal point, and the fractional
positions. The numeric value is formatted normally, after which one currency
symbol is placed immediately before it. Unused leading positions remain
spaces:

```text
Mask   : $$$$9.99
Value  : 64.31
Result :   $64.31
```

Repeating the currency character does not cause the symbol itself to be
repeated in the output. It reserves sufficient room for the symbol and for
the value to move within the field while remaining right-aligned.

For example:

```rexx
MSAY "Total: $$$$$9.99", 123.45
```

produces:

```text
Total:   $123.45
```

Currency formatting is intentionally independent of locale. The symbol is
taken directly from the mask; the formatter does not select a currency,
change the decimal separator, or apply national grouping conventions.

## Overflow Handling

Numeric information should not be made misleading merely to preserve the
layout of a line. For this reason, a numeric value that does not fit is not
truncated. Instead, every position in its field is replaced by an asterisk:

```text
Mask   : 999
Value  : 12345
Result : ***
```

The same rule applies to fixed-decimal and currency fields:

```text
Mask   : $$$9.99
Value  : 123333.3
Result : *******
```

Asterisks preserve the width of the report while drawing attention to the
fact that the selected mask was too narrow. The program can correct such an
overflow by increasing the field width or by otherwise constraining the value
before it is formatted.

Text fields behave differently: an overlong text value is truncated to the
field width. This is appropriate for labels and descriptions, where retaining
the layout is generally preferable to treating the value as a numeric error.

## Combining Fields in a Line

The principal benefit of picture masks becomes apparent when several fields
are combined. Literal text supplies headings and punctuation, while each
field reserves a predictable amount of space:

```rexx
MSAY "Item: XXXXX  Qty: 999  Price: 999.99",
     "Tea", 7, 12.45
```

This produces:

```text
Item: Tea    Qty:   7  Price:  12.45
```

Additional calls using the same mask retain the same column positions even
when the values have different lengths. A mask can therefore be kept in a
variable and reused for every detail line in a report:

```rexx
detailMask = "Item: XXXXXXXXXX  Qty: 999  Price: $$$$9.99"

MSAY detailMask, "Tea",    7, 12.45
MSAY detailMask, "Coffee", 2, 64.31
```

Using one mask for a group of related lines also keeps their presentation
consistent and makes later changes to the layout easier.

## Choosing Between MSAY and FMTMASK

`MSAY` is intended for direct output. It expresses formatting and display as
one operation and is consequently well suited to diagnostic messages,
headings, and report lines:

```rexx
MSAY "Customer: XXXXXXXXXX  Balance: $$$$$9.99",
     customer,
     balance
```

Use `fmtmask()` when the resulting string has a life beyond that immediate
output operation. It can be assigned to a variable, passed to another
routine, written through a logging interface, combined with other text, or
returned to a caller:

```rexx
line = fmtmask(mask, values...)
call logfile line
say line
```

Both forms use the same mask language and produce the same formatted text.
The distinction is therefore one of program structure rather than formatting
capability.

## Scope of the Facility

`fmtmask` is designed as a small, practical facility for common fixed-width
formatting tasks. It provides:

* fixed-width, left-aligned text fields;
* right-aligned integer and fixed-decimal fields;
* simple currency presentation;
* visible numeric-overflow handling; and
* a compact notation suitable for report-style output.

It is not intended to implement the complete COBOL `PIC` language or to act
as a general report writer. In particular, it does not provide locale-aware
currency conversion, national decimal and grouping conventions, or the range
of editing characters and sign-placement rules found in more extensive
formatting systems.

In this way, a mask remains easy to read beside the
values it formats, and the result can usually be understood without consulting
a large set of picture-string rules. For output that needs more sophisticated
localization or presentation logic, a specialized formatting library probably remains
the more appropriate choice.
