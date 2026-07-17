# Level B `floatmax`

```rexx
floatmax(first = .float, ... = .float) = .float
```

Returns the greatest of one or more homogeneous binary64 values without decimal
conversion. Infinities are ordered normally. The first equal value is retained,
including the sign of equal zero values. Any NaN signals `INVALID_ARGUMENTS`.

Invalid dynamic argument conversion signals `CONVERSION_ERROR`.
