# `FORMAT` (Level C Classic BIF)

```text
FORMAT(number [,before [,after [,expp [,expt]]]])
CheckArgs: rNUM oWHOLE>=0 oWHOLE>=0 oWHOLE>=0 oWHOLE>=0
```

The direct Level C entry point is
`rexxclassicbifformat.rexxclassicbif_format(context_ref)`. It accepts Classic
RexxValue arguments, preserves omitted argument positions, and returns a new
RexxValue containing formatted numeric text. It does not use the deprecated
name-based controller.

`number` is normalized under the caller's current numeric settings. `before`
sets the integer width including a sign, `after` sets the exact fractional
width, `expp` sets the exponent width, and `expt` controls when exponent form is
selected. Rounding is decimal half-up. A supplied `expp` of zero suppresses
exponential form. Scientific or engineering layout comes from the caller's
current `NUMERIC FORM`; there is no sixth `exform` argument.

```rexx
FORMAT(' - 12.73')          /* -12.73 */
FORMAT('1.73', 4, 0)        /*    2 */
FORMAT('12345.73',,,2,2)    /* 1.234573E+04 */
```

Standard errors are stored on the call context:

| Code | Condition |
| --- | --- |
| `RXC-LC-40.3` / `40.4` | Too few or too many arguments |
| `RXC-LC-40.5` | Required `number` omitted |
| `RXC-LC-40.11` | `number` is not numeric |
| `RXC-LC-40.12` / `40.13` | A control is not a whole number or is negative |
| `RXC-LC-40.38` | The integer part or exponent does not fit its requested width |

The focused direct harness is
`lib/rxfnsc/tests_functional/testRexxClassicBifFormat.crexx`. The native typed
Level B API is documented separately in
[`lib/rxfnsb/rexx/format.md`](../rxfnsb/rexx/format.md).
