## Level B `intmin`

```rexx
intmin(first = .int, ... = .int) = .int
```

Returns the least of one or more native signed integers. All variadic arguments
are homogeneous `.int` values, comparison is native, and the first occurrence
of an equal minimum is retained. There is no arithmetic overflow path.

Invalid dynamic argument conversion signals `CONVERSION_ERROR`.
