# Controlling Arithmetic Behaviour

Rexx decimal arithmetic is based on numbers whose working precision is
controlled by the program. This differs from languages in which an operation
is determined primarily by a fixed machine type such as a 32-bit integer or a
binary double-precision value. A Rexx procedure handling the `.decimal` type instead operates within a
numeric context that defines how many significant digits are retained, how
numbers are compared, how exponential results are written, and which
arithmetic conventions apply.

The `NUMERIC` instruction establishes this context for a procedure. It controls:

* the working precision through `DIGITS`;
* tolerance in numeric comparisons through `FUZZ`;
* scientific or engineering exponential notation through `FORM`;
* the spelling of special numeric values through `CASE`; and
* the selected set of arithmetic rules through `STANDARD`.

In cRexx Level B, these settings are fixed at the beginning of each procedure.
This restriction allows the compiler to determine the numeric environment
before it generates the procedure's code. Settings can either be constants,
known at compilation time, or be explicitly inherited from the calling
procedure.

Numeric behaviour must be distinguished from numeric *syntax*. The file-level
instruction:

```rexx
OPTIONS {NUMERIC_CLASSIC | NUMERIC_COMMON}
```

affects how the parser interprets operators throughout the source file. It can
determine such matters as precedence, associativity, and the accepted remainder
operator. `NUMERIC STANDARD`, by contrast, selects runtime arithmetic semantics
for a procedure. It does not change the syntax tree already constructed by the
parser.

Keeping these two responsibilities separate is essential when a source file
uses one parsing convention but an individual procedure selects another
arithmetic standard.

## Syntax

The `NUMERIC` instruction must be the first instruction in a procedure,
immediately following its label. This position permits the compiler to
establish the numeric context before compiling the executable body.

```rexx <!--numeric1.rexx-->
NUMERIC [ DIGITS [ <constant_value> | INHERITED ] ]
        [ FORM [ SCIENTIFIC | ENGINEERING | INHERITED ] ]
        [ FUZZ [ <constant_value> | INHERITED ] ]
        [ CASE [ UPPER | LOWER | INHERITED ] ]
        [ STANDARD [ CLASSIC | COMMON | INHERITED ] ]
```

The operands have the following meanings:

* `<constant_value>` is a literal whole number, such as `18` or `0`;
* `SCIENTIFIC` and `ENGINEERING` select an exponential notation;
* `UPPER` and `LOWER` select the case used for exponent markers and special
  numeric values;
* `CLASSIC` and `COMMON` select a set of arithmetic semantics; and
* `INHERITED` obtains the specified setting from the caller's numeric context.

More than one component of the numeric context may be declared by the
instruction. Each component may occur only once in a procedure. A setting
given as a constant cannot subsequently be changed dynamically.

For example, a procedure can establish an 18-digit context with engineering
notation and two ignored comparison digits:

```rexx
calculate:
numeric digits 18 form engineering fuzz 2

-- procedure body
```

Alternatively, it can inherit selected properties while fixing others:

```rexx
calculate:
numeric digits inherited fuzz 0 standard common

-- procedure body
```

In this example the precision comes from the caller, while comparison fuzz and
the arithmetic standard are local constants.

If a component is omitted, its cRexx default is used. Omission does not imply
inheritance. Inheritance must always be requested explicitly.

## Procedure Scope

The numeric context applies to all numeric operations performed within the
procedure. It remains in effect for the lifetime of that invocation and is not
altered by ordinary executable statements.

A called procedure does not automatically acquire every numeric setting from
its caller. Unless it requests `INHERITED`, it uses its own declared or default
settings. This gives a procedure a stable arithmetic environment independent
of the code that happens to call it.

That stability is valuable for library routines. A calculation written and
tested for 18 significant digits should not silently run at a caller's lower
precision unless the procedure explicitly permits that behaviour. Conversely,
`INHERITED` is useful for a general-purpose routine intended to participate in
the caller's chosen numeric environment.

## NUMERIC DIGITS

`NUMERIC DIGITS` sets the number of significant digits used for calculations.
It defines the working precision of the procedure rather than merely the
number of digits displayed in its output.

```rexx
numeric digits 24
```

The value must be a positive whole number and must be greater than the current
`NUMERIC FUZZ` value.

The defaults are:

* `18` digits for cRexx Level B; and
* `9` digits in classic Rexx.

A larger value allows calculations to retain more significant information,
but generally requires more work and storage. A smaller value can be
sufficient for ordinary calculations but may cause earlier rounding during a
sequence of operations.

The current setting can be obtained with the `DIGITS()` built-in function:

```rexx
say digits()
```

When inheritance is required, write:

```rexx
numeric digits inherited
```

The effective value is then taken from the caller when the procedure is
invoked.

## NUMERIC FORM

`NUMERIC FORM` selects how a number is written when exponential notation is
required. It does not change the underlying value or working precision.

The available forms are:

* `SCIENTIFIC`, the default, which places one non-zero digit before the
  decimal point; and
* `ENGINEERING`, which uses an exponent that is a multiple of three.

For example, a value may be represented conceptually in scientific form as:

```text
1.2345E+7
```

Engineering notation adjusts the exponent to a multiple of three and permits
one, two, or three digits before the decimal point:

```text
12.345E+6
```

Engineering notation is often convenient when values are read together with
SI prefixes, because powers of one thousand correspond to prefixes such as
kilo, mega, and giga.

The form is selected with:

```rexx
numeric form engineering
```

or:

```rexx
numeric form scientific
```

A procedure that should follow its caller's convention can use:

```rexx
numeric form inherited
```

The effective setting is returned by the `FORM()` built-in function.

## NUMERIC FUZZ

`NUMERIC FUZZ` reduces the number of significant digits considered when
numeric values are compared. It provides a controlled tolerance for results
that may differ in their least significant positions.

```rexx
numeric digits 18 fuzz 2
```

With this context, calculations still use 18 significant digits, but numeric
comparisons disregard the least significant two digits for comparison
purposes. `FUZZ` does not reduce the precision at which the calculations
themselves are performed.

The value must be zero or a positive whole number smaller than `NUMERIC
DIGITS`. A fuzz value equal to or greater than the working precision would
leave no significant digits on which to base a comparison and is therefore
invalid.

The default is:

```text
0
```

With a zero fuzz value, all digits available under the current numeric
precision participate in comparisons.

A procedure can inherit the caller's tolerance:

```rexx
numeric fuzz inherited
```

The current value is returned by the `FUZZ()` built-in function.

`FUZZ` should be selected deliberately. It can be useful for comparisons of
calculated values, but an unnecessarily large setting may cause values with a
meaningful difference to compare as equal.

## NUMERIC CASE

`NUMERIC CASE` controls the case used when numeric conversion produces an
exponent marker or a special numeric value. It affects spellings such as:

```text
e
inf
nan
```

The available settings are:

* `LOWER`, the default, which produces `e`, `inf`, and `nan`; and
* `UPPER`, which produces `E`, `INF`, and `NAN`.

For example:

```rexx
numeric case upper
```

selects uppercase numeric text for the procedure. This can be useful when
generated output must conform to an external convention or when a body of
report text uses a consistent uppercase notation.

The setting affects the textual representation, not the mathematical meaning
of the value. A procedure can follow the caller by specifying:

```rexx
numeric case inherited
```

The current setting is returned by the `NUMCASE()` built-in function.

## NUMERIC STANDARD

`NUMERIC STANDARD` selects a predefined collection of arithmetic semantic
rules. It allows a procedure to request classic Rexx behaviour or the common
behaviour adopted by cRexx Level B.

```rexx
numeric standard classic
```

or:

```rexx
numeric standard common
```

This setting affects the meaning and constraints of arithmetic operations. It
does not alter operator precedence or associativity; those are parser choices
made by the file-level `OPTIONS` instruction.

### CLASSIC

`CLASSIC` selects the arithmetic conventions defined by ANSI Rexx
X3.274-1996 where classic-compatible mode is selected.

Its principal rules include:

* **Remainder (`//` or `%`)** -- division is calculated at the current
  precision and truncated, after which the remainder is computed as:

```text
  a - (TRUNC(a / b) * b)
```

* **Integer magnitude and precision** -- the integer quotient used for `%`
  and `//` must fit within `NUMERIC DIGITS` without exponential notation.
  Otherwise, `SYNTAX Error 26.11` is raised.

* **Rounding** -- values use the traditional round-half-up rule.

* **Integer division (`%`)** -- the operands are divided at the current
  precision and the result is truncated towards zero.

The magnitude constraint is a characteristic part of classic Rexx numeric
semantics. `NUMERIC DIGITS` limits not only the precision of a calculation but
also the acceptable representation of the integer quotient involved in these
operations.

### COMMON

`COMMON` selects semantics closer to those of C-like languages. It is the
default standard for cRexx Level B.

Its principal rules include:

* **Remainder (`%` or `//`)** -- division is calculated at the current
  precision and truncated, after which the remainder is computed as:

```text
  a - (TRUNC(a / b) * b)
```

* **Integer magnitude and precision** -- the classic quotient constraint is
  not applied. Quotients may exceed `NUMERIC DIGITS` and may use exponential
  notation.

* **Rounding** -- values use round-half-even. When a value lies exactly
  halfway between two possible rounded results, the result whose final retained
  digit is even is selected.

* **Division (`/`)** -- when both operands are integers, integer division is
  performed and any fractional part is truncated towards zero. No signal is
  raised merely because the mathematical quotient is not integral. If either
  operand is a float or decimal value, normal float or decimal division is
  performed.

The last rule is particularly important:

```rexx
numeric standard common

say 5 / 2
```

produces:

```text
2
```

not `2.5`, because both operands are integers. Similarly:

```rexx
say 180945931154 / 100
```

produces:

```text
1809459311
```

When a fractional result is intended, at least one operand must explicitly be
a decimal or floating value:

```rexx
say 5d / 2
say 180945931154d / 100
say .decimal("5") / 2
```

This makes the desired kind of division visible in the source and avoids an
implicit change from integer to decimal arithmetic.

### Inheriting the Standard

A procedure can use the caller's arithmetic semantics:

```rexx
numeric standard inherited
```

This is suitable for utility routines that are deliberately intended to work
within the numeric conventions of their caller. It should not be used merely
as a substitute for choosing the standard under which a procedure is meant to
operate.

The effective setting is returned by the `STANDARD()` built-in function.

## Interaction with OPTIONS

The distinction between `OPTIONS NUMERIC_CLASSIC` or
`OPTIONS NUMERIC_COMMON` and `NUMERIC STANDARD` arises because parsing occurs
before procedure execution.

The `OPTIONS` instruction determines how the parser constructs expressions
for the entire source file. Once an expression has been parsed, changing a
procedure's numeric standard cannot give that expression a different tree.

For exponentiation, the two parser modes produce different results:

### Classic parsing

```rexx
say -3**2
```

is interpreted as:

```text
(-3)**2
```

and produces `9`.

Similarly:

```rexx
say 2**2**3
```

is interpreted as:

```text
(2**2)**3
```

and produces `64`.

### Common parsing

Under common parsing:

```rexx
say -3**2
```

is interpreted as:

```text
-(3**2)
```

and produces `-9`.

Exponentiation is associated from the right:

```rexx
say 2**2**3
```

is interpreted as:

```text
2**(2**3)
```

and produces `256`.

The file-level option can also determine which token is used for remainder,
such as `//` or `%`. These are lexical and syntactic decisions and remain
constant throughout the file.

`NUMERIC STANDARD CLASSIC` inside a procedure does not make a file parsed with
common precedence rules behave as though it had been reparsed in classic
mode. It changes only the arithmetic semantics applied after the expression
has been constructed.

## Defaults and Inheritance

Each omitted numeric setting receives its defined cRexx default. It does not
automatically take the corresponding value from the caller.

This procedure:

```rexx
worker:
numeric digits 24
```

uses 24 digits and the defaults for `FORM`, `FUZZ`, `CASE`, and `STANDARD`.
Those other components do not depend on the caller.

By contrast:

```rexx
worker:
numeric digits 24 form inherited standard inherited
```

uses a fixed 24-digit precision but obtains its exponential form and arithmetic
standard when it is called.

Selective inheritance permits a routine to protect the settings essential to
its calculation while remaining adaptable in areas that affect presentation
or calling conventions.

## Runtime Optimization

cRexx can generate more efficient arithmetic code when the numeric context is
known during compilation. Requiring `NUMERIC` at the beginning of the
procedure and restricting its operands to constants allows the compiler to:

* embed numeric context values directly in generated bytecode;
* avoid repeated runtime lookup of numeric settings;
* select specialized arithmetic operations; and
* reason about the procedure under one stable set of numeric rules.

For example:

```rexx
calculate:
numeric digits 18 fuzz 0 standard common
```

gives the compiler a complete, fixed description of several important
arithmetic properties.

`INHERITED` necessarily postpones part of that decision until the procedure is
called:

```rexx
calculate:
numeric digits inherited standard inherited
```

The effective settings cannot then be known solely from the procedure's
source. This provides flexibility, but it can prevent the corresponding
compile-time optimizations.

The choice is therefore intentional:

* use constants when a procedure has defined arithmetic requirements and
  predictable performance is important; and
* use `INHERITED` when adapting to the caller's numeric environment is part of
  the procedure's contract.

If a setting is omitted, the default is used and remains known to the
compiler. Programmers must explicitly request `INHERITED` when caller-dependent
behaviour is required.
