# Level C `LENGTH`

The standalone direct entry point is:

```rexx
.rexxclassicbiflength..rexxclassicbif_length(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `LENGTH(string)` with the CheckArgs contract `rANY`. The returned
RexxValue contains the decimal character/codepoint count, not a UTF-8 byte
count.

Standard context errors are `RXC-LC-40.3`, `40.4`, and `40.5` for argument
count or omission. The Classic specification also reserves `23.1` when a
configuration cannot interpret its input as character data. Normal cREXX
RexxValue text has already entered the configured valid `.string` model, so
that invalid-data path is not reachable in this implementation.

The implementation extracts the RexxValue text once and executes one direct VM
`strlen`; it allocates only the returned RexxValue and does not call the name
dispatcher. The older `rexxclassicbifs.rexxclassicbif_length(value)` helper is
retained temporarily for current compiler-generated artifacts and is not this
context entry point.

`lib/rxfnsc/tests_functional/testRexxClassicBifLength.crexx` calls the
standalone entry directly in optimized and unoptimized overlays and covers
empty, ASCII, Unicode, combining-codepoint, non-mutation, and argument errors.
