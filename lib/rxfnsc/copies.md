# Level C `COPIES`

The standalone direct entry point is:

```rexx
rexxclassicbif_copies(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `COPIES(string, count)`. Count is a non-negative Classic whole
number. Zero returns the null string and one returns a fresh RexxValue with the
source text.

The CheckArgs contract is `rANY rWHOLE>=0`. Standard context errors are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` when count is not a whole number; and
- `RXC-LC-40.13` when count is negative.

The implementation uses binary decomposition and direct VM string copies and
appends. It performs O(log `count`) append operations rather than repeatedly
concatenating every growing prefix. Resource exhaustion remains governed by
normal VM string limits.

`lib/rxfnsc/tests_functional/testRexxClassicBifCopies.crexx` calls this entry
directly in optimized and unoptimized overlays. It covers normal, zero, one,
empty, Unicode, substantial, and every standard count error without invoking
the compatibility dispatcher.
