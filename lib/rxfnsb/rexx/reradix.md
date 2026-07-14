# Level B `reradix`

`reradix` converts an unsigned integer between radices 2 through 16:

```rexx
reradix(subject=.string, FromRadix=.int, ToRadix=.int) = .string
```

The source uses ASCII digits `0` through `9` and letters `A` through `F`
case-insensitively. The result uses uppercase digits and has no numeric-size
limit imposed by the native `.int` type.

```rexx
reradix("F81", 16, 10) == "3969"
reradix("340282366920938463463374607431768211455", 10, 16) == ,
  "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
```

Leading zeros are normally suppressed. Binary-to-hex conversion preserves one
hex digit for each supplied group of up to four bits, and hex-to-binary
preserves four bits per supplied hex digit:

```rexx
reradix("00001", 2, 16) == "01"
reradix("0F", 16, 2) == "00001111"
```

An empty subject, a radix outside 2 through 16, a non-ASCII digit, or a digit
outside the source radix signals `INVALID_ARGUMENTS`. The source is not
modified.

The specialized binary/hex paths are linear. Other pairs use little-endian
target digits and bounded multiply/add intermediates, avoiding native-integer
overflow, decimal/scientific conversion, front-prepending, and helper calls.
`.Rexx.reradix(fromRadix, toRadix)` forwards to this Level B function. There is
no Level C BIF named RERADIX.
