# Level C `CENTRE`

The standalone direct entry point is:

```rexx
rexxclassicbif_centre(context = reference .RexxBifCallContext) = .RexxValue
```

`CENTRE(string, length [,pad])` is the British-spelling alias of the Classic
`CENTER` BIF. Its exported direct entry lives beside and delegates directly to
the standalone CENTER entry in `RexxClassicBifCenter.crexx`, so both names
share the same argument validation, BYTE/UTF8 profile behavior, centring bias,
and error reporting without duplicating Level C logic.

The CheckArgs contract is `rANY rWHOLE>=0 oPAD`. Standard context errors are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole length;
- `RXC-LC-40.13` for a negative length; and
- `RXC-LC-40.23` unless `pad` is exactly one configured character unit.

The result is exactly `length` BYTE octets under the default profile or Unicode
codepoints under the opt-in UTF8 profile. Uneven padding is placed on the
right; uneven truncation retains the left-biased central slice.

`lib/rxfnsc/tests_functional/testRexxClassicBifCentre.crexx` calls this entry
directly in optimized and unoptimized overlays and covers normal results,
Unicode, zero width, omitted arguments, and every error family above. It never
calls the compatibility dispatcher.
