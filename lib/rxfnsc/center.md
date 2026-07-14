# Level C `CENTER`

The standalone direct entry point is:

```rexx
rexxclassicbif_center(context = reference .RexxBifCallContext) = .RexxValue
```

It implements `CENTER(string, length [,pad])`. The result contains exactly
`length` Unicode codepoints. Short strings are padded with a blank by default;
long strings are centrally truncated. Uneven padding is placed on the right,
and uneven truncation retains the left-biased central slice.

The CheckArgs contract is `rANY rWHOLE>=0 oPAD`. Standard context errors are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole length;
- `RXC-LC-40.13` for a negative length; and
- `RXC-LC-40.23` unless `pad` is exactly one codepoint.

The implementation uses direct VM string length, cursor, substring, pad, and
append operations. It allocates no padded template for the right side and no
substring unless truncation is needed.

`lib/rxfnsc/tests_functional/testRexxClassicBifCenter.crexx` covers ordinary,
empty, zero-width, Unicode, argument, width, and pad behavior in optimized and
unoptimized direct-call overlays. It never calls the compatibility dispatcher.

`CENTRE` is a separate recognized alias and is completed in its own selector
row so both public names retain explicit tests and documentation.
