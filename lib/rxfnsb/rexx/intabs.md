# Level B `intabs`

```rexx
intabs(number = .int) = .int
```

Returns the native signed-integer absolute value without a decimal or float
conversion. Zero and positive values are returned unchanged. The absolute value
of `INT64_MIN` is not representable and signals `OVERFLOW_UNDERFLOW`.

Invalid dynamic conversion to `.int` follows the normal Level B
`CONVERSION_ERROR` contract.
