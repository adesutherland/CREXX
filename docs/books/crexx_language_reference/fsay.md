# FSAY and FSAYFMT

## Overview

`FSAY` is a compiler-exit statement that turns a template into a cREXX string
expression and emits it through `SAY`. Its Level B helper, `fsayfmt`, is the
source-expression generator used by that exit; it is not a runtime
interpolation engine.

```rexx
name = "Fred"
qty = 12
price = 64.31
FSAY "Name: {name:<10} Qty: {qty:>3} Price: {price:8.2}"
```

Output:

```text
Name: Fred       Qty:  12 Price:    64.31
```

Conceptually, the statement becomes:

```rexx
say 'Name: ' || left(name,10) ||,
    ' Qty: ' || right(qty,3) ||,
    ' Price: ' || format(price,8,2)
```

## FSAY statement

```rexx
FSAY template
```

The compiler exit passes the quoted template source token to `fsayfmt`, inserts
the returned expression after `SAY`, and compiles the result normally. Variable
lookup and formatting therefore occur when the generated statement runs.

## Placeholder grammar

The general form is:

```text
{variable[:alignment][width][.decimals]}
```

Whitespace may replace the colon:

```text
{name:<10}
{name <10}
```

`variable` must be an identifier or compound variable. General expressions and
array subscripts are not accepted inside a placeholder.

| Placeholder | Generated expression |
| --- | --- |
| `{name}` | `name` |
| `{name:10}` | `left(name,10)` |
| `{name:<10}` | `left(name,10)` |
| `{name:>10}` | `right(name,10)` |
| `{name:^10}` | `center(name,10)` |
| `{price:8.2}` | `format(price,8,2)` |
| `{price:.2}` | `format(price,,2)` |
| `{price:<8.2}` | `left(format(price,,2),8)` |
| `{price:>8.2}` | `format(price,8,2)` |
| `{price:^8.2}` | `center(format(price,,2),8)` |

Plain widths default to left alignment. Decimal formats default to right
alignment. Width and decimal precision are non-negative decimal integers; an
explicit alignment requires a width.

## Literal text and braces

Literal template text is emitted as canonical single-quoted cREXX source.
Apostrophes are doubled in that generated source, and Unicode text is preserved.
Use `{{` and `}}` for literal braces:

```rexx
FSAY "Set {{name}} to {value}"
```

An unmatched brace, nested opening brace, empty placeholder, invalid variable,
empty format, invalid width, or invalid decimal precision signals
`INVALID_ARGUMENTS` while the compiler exit is expanding the statement.

## Direct `fsayfmt` use

The typed Level B surface is:

```rexx
fsayfmt(template = .string) = .string
```

A direct call normally supplies decoded template text and receives cREXX source:

```rexx
fsayfmt("Hello {name}")  /* returns: 'Hello '||name */
fsayfmt("{name:10}")    /* returns: left(name,10) */
```

For compiler-exit use, one matching outer quoted source token is decoded before
the template is scanned. `fsayfmt` scans the decoded template once and does not
invoke the general quote-aware library.

## Relationship to FMTMASK and MSAY

`FSAY` uses named placeholders close to the text they affect. `MSAY` and
`fmtmask` instead use COBOL-style picture masks and positional values. Use FSAY
for readable named interpolation and MSAY for fixed picture-driven reports.

See the selector-level [Level B contract](../../../lib/rxfnsb/rexx/fsayfmt.md)
for direct-call and test details.
