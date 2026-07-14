# Level B `min`

The native Level B `min` function returns the least of one or more decimal
values:

```rexx
min(first = .decimal, ... = .decimal) = .decimal
```

```rexx
min(100, 99, 1)                              /* 1 */
min(-4, -1, -9)                              /* -9 */
min("9007199254740993", "9007199254740992") /* 9007199254740992 */
```

At least one argument is required by the typed signature. The call boundary
performs ordinary `.decimal` conversion for every argument. The implementation
uses one linear comparison pass, retains the first value on a tie, allocates no
string, and calls no helper.

Invalid dynamic decimal conversion raises the catchable `CONVERSION_ERROR`
signal.
The separate Level C `MIN` BIF validates Classic `rNUM...` text and preserves
the first selected argument's normalized text representation.
