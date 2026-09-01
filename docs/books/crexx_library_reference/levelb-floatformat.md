## floatformat

```rexx
floatformat(number = .float,
            before = .int optional,
            after = .int optional,
            expp = .int optional,
            expt = .int optional) = .string
```

Formats a finite binary64 value using the same omission, rounding, width,
exponent-trigger, `NUMERIC DIGITS`, `NUMERIC FORM`, and `NUMERIC CASE` rules as
decimal `format`. The value is extracted directly to numeric-context text and
is never converted to decimal.

Signed zero is canonicalized by native extraction. Non-finite values, negative
controls, and fields that cannot fit signal `INVALID_ARGUMENTS`. Typed
conversion failures signal `CONVERSION_ERROR`. The result is ordinary
`.string` text.
