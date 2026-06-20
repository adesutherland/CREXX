# MSAY and FMTMASK

## Overview

`fmtmask` is a lightweight picture-mask formatter for CREXX.

It formats text and numeric values according to a simple mask syntax inspired by COBOL report writers and picture clauses.

The same formatting engine is also available through the compiler-exit statement:

```rexx
MSAY mask, value-1, value-2, ...
```

`MSAY` is a convenience syntax that formats values using `fmtmask` and immediately outputs the resulting line.

The facility is intended for report-style output where fixed-width text and numeric alignment are desirable.


## Function Syntax

```rexx
line = fmtmask(mask, value-1, value-2, ...)
```

### Example

```rexx
line = fmtmask(
    "Name: XXXXXXXXXX  Price: $$$$9.99  Qty: 999",
    "Fred",
    64.31,
    12
)

say line
```

Output:

```text
Name: Fred        Price:   $64.31  Qty:  12
```

## MSAY Syntax

```rexx
MSAY mask, value-1, value-2, ...
```

### Example

```rexx
MSAY "Name: XXXXXXXXXX  Price: $$$$9.99  Qty: 999", ,
     "Fred", 64.31, 12
```

Output:

```text
Name: Fred        Price:   $64.31  Qty:  12
```

### Equivalent Code

```rexx
say fmtmask(
    "Name: XXXXXXXXXX  Price: $$$$9.99  Qty: 999",
    "Fred",
    64.31,
    12
)
```

## Mask Structure

A mask consists of:

* Literal text
* Text fields
* Numeric fields

Literal text is copied unchanged.

Each field consumes the next value argument from left to right.

Example:

```rexx
mask = "Name: XXXXX  Qty: 999  Price: 999.99"
```

Values:

```rexx
"Tea", 7, 12.45
```

Mapping:

```text
XXXXX   -> "Tea"
999     -> 7
999.99  -> 12.45
```

## Text Fields

A text field is a contiguous sequence of `X` characters.

Example:

```text
XXXXXXXXXX
```

Field width equals the number of `X` characters.

Rules:

* Left aligned
* Truncated if too long
* Space padded if too short

Example:

Mask:

```text
XXXXXXXXXX
```

Value:

```text
Fred
```

Result:

```text
Fred
```

(with six trailing spaces)

## Integer Numeric Fields

A contiguous sequence of `9` characters defines an integer field.

Example:

```text
999
```

Rules:

* Value is converted using `TRUNC()`
* Right aligned
* Space padded on the left
* Overflow produces asterisks

Example:

Mask:

```text
999
```

Value:

```text
12
```

Result:

```text
 12
```

Overflow:

```text
Mask   : 999
Value  : 12345
Result : ***
```

## Fixed Decimal Fields

A numeric mask containing a decimal point and digits after the decimal point defines a fixed-decimal field.

Example:

```text
9999.99
```

Rules:

* Decimal places determined by digits after the decimal point
* Formatting performed using `FORMAT()`
* Right aligned
* Overflow produces asterisks

Example:

Mask:

```text
9999.99
```

Value:

```text
64.31
```

Result:

```text
  64.31
```

## Currency Fields

Currency fields are numeric masks with leading currency characters.

Supported currency symbols:

```text
$
€
£
¥
```

Example:

```text
$$$$9.99
```

Rules:

* Currency characters contribute to field width
* Numeric value is formatted normally
* Currency symbol is inserted into the formatted field
* Overflow still produces asterisks

Example:

Mask:

```text
$$$$9.99
```

Value:

```text
64.31
```

Result:

```text
  $64.31
```

Example:

```rexx
MSAY "Total: $$$$$9.99", 123.45
```

Output:

```text
Total:   $123.45
```

## Overflow Handling

If a formatted value does not fit within the available field width, the entire field is replaced by asterisks.

Examples:

```text
Mask   : 999
Value  : 12345
Result : ***
```

```text
Mask   : $$$9.99
Value  : 123333.3
Result : *******
```

## Parsing Rules

Text fields:

```text
XXXXX
```

Numeric fields:

```text
999
999.99
$$$$9.99
```

A decimal point belongs to a numeric field only when followed by at least one `9`.

Examples:

```text
999.99
```

is parsed as a decimal field.

```text
999.
```

is parsed as:

```text
999
.
```

(integer field followed by literal period)

## Value Consumption

Values are consumed strictly from left to right.

Example:

```rexx
MSAY "Item: XXXXX  Qty: 999  Price: 999.99",
     "Tea", 7, 12.45
```

Produces:

```text
Item: Tea    Qty:   7  Price:  12.45
```

## Design Goals

`fmtmask` is intentionally small and practical.

It provides:

* Fixed-width text formatting
* Simple numeric formatting
* Report-style output
* COBOL-inspired picture masks

It is **not**:

* A full COBOL PIC implementation
* A locale-aware currency formatter
* A complete report writer
* A replacement for sophisticated formatting libraries

## When to Use MSAY

Use `MSAY` when formatting output directly:

```rexx
MSAY "Customer: XXXXXXXXXX  Balance: $$$$$9.99",
     customer,
     balance
```

Use `fmtmask()` when the formatted string must be stored, modified, logged, or returned before being displayed.

```rexx
line = fmtmask(mask, values...)
call logfile line
say line
```
