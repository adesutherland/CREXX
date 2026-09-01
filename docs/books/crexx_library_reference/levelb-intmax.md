## intmax

```rexx
intmax(first = .int, ... = .int) = .int
```

Returns the greatest of one or more native signed integers. All variadic
arguments are homogeneous `.int` values, comparison is native, and the first
occurrence of an equal maximum is retained. There is no arithmetic overflow
path.

Invalid dynamic argument conversion signals `CONVERSION_ERROR`.
