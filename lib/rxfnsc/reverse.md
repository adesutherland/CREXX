# Level C `REVERSE`

The standalone direct entry point is:

```rexx
rexxclassicbif_reverse(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `REVERSE(string)` with the CheckArgs contract `rANY`. BYTE is the
default and reverses exact octets into a binary-authoritative result; UTF8 is
opt-in and reverses Unicode codepoints. Grapheme clusters are not a separate
unit at this API level.

Standard context errors are `RXC-LC-40.3` and `RXC-LC-40.4` for too few or too
many arguments and `RXC-LC-40.5` for an omitted required argument.

The implementation caches the configured-unit length and makes one reverse
pass using direct binary byte operations or VM `strchar`/`appendchar`
operations. It calls neither the name dispatcher nor the Level B REVERSE helper.

`lib/rxfnsc/tests_functional/testRexxClassicBifReverse.crexx` calls the direct
entry in optimized and unoptimized overlays and covers ASCII, Unicode, empty,
one-character, substantial, non-mutation, and argument-error behavior.
