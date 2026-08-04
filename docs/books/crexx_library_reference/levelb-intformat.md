## intformat

```rexx
intformat(number = .int,
          before = .int optional,
          after = .int optional,
          expp = .int optional,
          expt = .int optional) = .string
```

Formats the full native signed-64-bit input range using the same omission,
rounding, width, exponent-trigger, `NUMERIC DIGITS`, `NUMERIC FORM`, and
`NUMERIC CASE` rules as decimal `format`. The integer is extracted directly to
numeric-context text and is never converted to decimal or float.

Negative controls and fields that cannot fit signal `INVALID_ARGUMENTS`.
Typed conversion failures signal `CONVERSION_ERROR`. The result is ordinary
`.string` text.
