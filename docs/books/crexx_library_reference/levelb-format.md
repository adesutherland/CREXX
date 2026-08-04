## format

```rexx
format(number = .decimal,
       before = .int optional,
       after = .int optional,
       expp = .int optional,
       expt = .int optional) = .string
```

`format` is the native Level B decimal formatter. `number` is converted to a
decimal at the typed call boundary; all four formatting controls are integers.
The function inherits the caller's `NUMERIC DIGITS` and `NUMERIC FORM` settings.

- `before` is the width of the integer part, including a possible minus sign.
- `after` is the exact number of fractional digits; excess digits are rounded
  half up and missing digits are zero-filled.
- `expp` is the exponent digit width. A supplied zero suppresses exponential
  notation; a supplied width leaves a blank exponent field when the exponent
  is zero.
- `expt` is the trigger for exponential notation. When it is omitted but
  `expp` is present, the current `NUMERIC DIGITS` value is used.

Omission is significant. For example, `format(value,,4)` leaves `before`
unspecified while requesting four fractional digits.

```rexx
format(1.75, 4, 1)               /* "   1.8" */
format(12345.73,,,2,2)           /* "1.234573E+04" */

numeric form engineering
format(12345,,3,3,0)             /* "12.345E+003" */
```

Negative controls and integer/exponent fields that cannot fit signal
`INVALID_ARGUMENTS`. Invalid dynamic conversion to the typed `.decimal` or
`.int` parameters follows the Level B conversion-signal contract.

The implementation performs decimal-preserving string decomposition and exact
digit rounding. It does not pass through binary floating point and does not
call the general Level B string BIFs on the optimized path.

Classic Level C callers use the distinct RexxValue contract documented in
[`lib/rxfnsc/format.md`](../../rxfnsc/format.md).
