## floatsign

```rexx
floatsign(number = .float) = .int
```

Returns `-1`, `0`, or `1` after binary64 comparisons. Both signed zeros return
zero, infinities return their respective sign, and NaN signals
`INVALID_ARGUMENTS`. No decimal payload is created.

Invalid dynamic conversion to `.float` signals `CONVERSION_ERROR`.
