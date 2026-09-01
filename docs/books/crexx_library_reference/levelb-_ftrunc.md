## _ftrunc

`_ftrunc` is a private `_rxsysb` helper used by Level B `format`:

```rexx
_ftrunc(number = .float) = .string
```

It converts `number` with the VM's active numeric context and returns the text
after the decimal point. If the formatted representation has no decimal point,
it returns the empty string.

```rexx
_ftrunc(12.75)  /* "75" */
_ftrunc(-0.125) /* "125" */
_ftrunc(42.0)   /* "" */
```

Typed-call conversion failures are reported through the VM's
`CONVERSION_ERROR` signal. The caller's value is not modified.

After the numeric conversion, the implementation performs one native
decimal-point search and at most one direct substring. It has no interpreted
per-character loop, helper call, or append loop.

This is not a public Classic BIF and has no Level C contract. Its output is an
implementation component of `format`, not an independent decimal-arithmetic
API.
