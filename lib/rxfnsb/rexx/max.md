# Level B `max`

The native Level B `max` function returns the greatest of one or more decimal
values:

```rexx
max(first = .decimal, ... = .decimal) = .decimal
```

```rexx
max(100, 99, 1)                              /* 100 */
max(-4, -1, -9)                              /* -1 */
max("9007199254740992", "9007199254740993") /* 9007199254740993 */
```

At least one argument is required by the typed signature. The call boundary
performs ordinary `.decimal` conversion for every argument. The implementation
uses one linear comparison pass, retains the first value on a tie, allocates no
string, and calls no helper.

Invalid dynamic decimal conversion is subject to the VM conversion-signal
dependency recorded for numeric Level B selectors in the programme worklist.
The separate Level C `MAX` BIF validates Classic `rNUM...` text and preserves
the first selected argument's normalized text representation.
