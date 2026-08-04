## _itrunc

`_itrunc` is a private `_rxsysb` helper used by Level B `format`:

```rexx
_itrunc(number = .float) = .string
```

It converts `number` with the VM's active numeric context and returns the text
before the decimal point, including a negative sign. If the formatted
representation has no decimal point, it returns that representation directly.

```rexx
_itrunc(12.75)  /* "12" */
_itrunc(-0.125) /* "-0" */
_itrunc(42.0)   /* "42" */
```

Typed-call conversion failures are reported through the VM's
`CONVERSION_ERROR` signal. The caller's value is not modified.

After numeric conversion, the implementation performs one native decimal-point
search and at most one direct substring. It has no interpreted per-character
loop, helper call, conversion-per-character, or append loop.

This is not a public Classic BIF and has no Level C contract. Its output is an
implementation component of `format`, not an independent decimal-arithmetic
API.
