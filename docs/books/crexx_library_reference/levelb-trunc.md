## Level B `trunc`

The native Level B function truncates a decimal value to fixed text:

```rexx
trunc(number = .decimal, fraction = 0) = .string
```

`fraction` is a non-negative `.int`. The result contains exactly that many
fractional digits, adding zeros when required, and is never rounded or written
in exponent form. With the default zero it has no decimal point.

```rexx
trunc(12.3)          /* "12" */
trunc(127.09782, 3)  /* "127.097" */
trunc(127.1, 3)      /* "127.100" */
trunc(1E-12, 12)     /* "0.000000000001" */
```

A negative fraction signals `INVALID_ARGUMENTS`. The input is not modified.

The implementation extracts the decimal coefficient and exponent once, removes
the coefficient point, and assembles the requested fixed result with direct
substring and zero-padding instructions. It performs no float conversion,
power, multiplication, integer conversion, or helper call. One string copy is
used to materialize the coefficient's VM text metadata before scanning it.

The Level B call boundary performs ordinary `.decimal` conversion. Invalid
dynamic conversion raises the catchable `CONVERSION_ERROR` signal.

The distinct Level C `TRUNC` BIF accepts Classic numeric text through RexxValue
and preserves mantissas longer than the active typed-decimal precision.
