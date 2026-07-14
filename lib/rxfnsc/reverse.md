# Level C `REVERSE`

The standalone direct entry point is:

```rexx
rexxclassicbif_reverse(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `REVERSE(string)` with the CheckArgs contract `rANY`. The result
contains the input Unicode codepoints in reverse order; UTF-8 bytes are never
reversed independently. Grapheme clusters are not a separate unit at this API
level.

Standard context errors are `RXC-LC-40.3` and `RXC-LC-40.4` for too few or too
many arguments and `RXC-LC-40.5` for an omitted required argument.

The implementation extracts RexxValue text once, caches its codepoint length,
and makes one reverse pass using direct VM `strchar`/`appendchar` operations. It
calls neither the name dispatcher nor the Level B REVERSE helper.

`lib/rxfnsc/tests_functional/testRexxClassicBifReverse.crexx` calls the direct
entry in optimized and unoptimized overlays and covers ASCII, Unicode, empty,
one-character, substantial, non-mutation, and argument-error behavior.
