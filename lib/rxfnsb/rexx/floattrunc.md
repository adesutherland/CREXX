# Level B `floattrunc`

```rexx
floattrunc(number = .float, fraction = .int optional) = .string
```

`fraction` defaults to zero. The finite binary64 value is extracted under the
caller's current `NUMERIC DIGITS` and `NUMERIC CASE`, then truncated toward zero
to fixed non-exponent text with exactly `fraction` fractional digits. Extraction
is native and never creates a decimal payload.

Exact signed zero becomes unsigned zero text. A negative nonzero value that
truncates to zero retains its minus sign. Negative `fraction` and non-finite
inputs signal `INVALID_ARGUMENTS`; typed conversion failures signal
`CONVERSION_ERROR`. The result is ordinary `.string` text and receives no
hidden numeric return type.
