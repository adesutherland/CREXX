## linesize

`linesize` returns the implementation's maximum supported line size:

```rexx
linesize() = .int
```

The portable implementation returns `999999999`, the traditional effectively
unlimited value used for compatibility with Rexx on z/VM. A platform-specific
Level B library may replace the module if the host has a smaller native limit.

```rexx
if length(record) > linesize() then say "record is too long"
```

The function has no arguments, allocation, helper call, error path, or Level C
BIF counterpart. `.Rexx.linesize()` forwards directly to the same Level B
function. `lib/rxfnsb/tests_functional/tlinesz.crexx` checks the exact stable
value and repeated calls.
