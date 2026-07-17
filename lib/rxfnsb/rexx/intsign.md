# Level B `intsign`

```rexx
intsign(number = .int) = .int
```

Returns `-1`, `0`, or `1` after native signed-integer comparisons. The function
does not convert the input to decimal or float.

Invalid dynamic conversion to `.int` signals `CONVERSION_ERROR`.
