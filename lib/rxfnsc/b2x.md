# `B2X` (Level C Classic BIF)

```text
B2X(binary)
CheckArgs: rBIN
```

The direct entry point is
`rexxclassicbifb2x.rexxclassicbif_b2x(context_ref)`. It accepts one RexxValue
binary string and returns a new RexxValue containing uppercase hexadecimal
text. It does not use the deprecated name-based controller.

The empty binary string is valid. Interior blanks are valid only on standard
nibble boundaries: each blank must have a multiple of four binary digits to
its right. Blanks are omitted from the conversion.

```rexx
B2X("11000011")  /* C3 */
B2X("10111")     /* 17 */
B2X("10 1010")   /* 2A */
```

Standard context errors are `RXC-LC-40.3` for a missing argument count,
`RXC-LC-40.4` for extra arguments, `RXC-LC-40.5` for an omitted required
argument, and `RXC-LC-40.24` for invalid binary text or blank placement.

The focused direct harness is
`lib/rxfnsc/tests_functional/testRexxClassicBifB2x.crexx`. The native Level B
API is documented separately in
[`lib/rxfnsb/rexx/b2x.md`](../rxfnsb/rexx/b2x.md).
