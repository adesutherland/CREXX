# `XRANGE` (Level C Classic BIF)

```text
XRANGE([start [, end]])
CheckArgs: oPAD oPAD
```

The standalone `rexxclassicbif_xrange` entry returns the inclusive byte ordinal
range, wraps after `FF`, and defaults
to `00` through `FF`. Supplying only `start` selects through `FF`; an omitted
`start` with an explicit `end` starts at `00`. Every result contains at most 256
ordinals. BYTE results remain binary-authoritative.

Compiled Level C returns one Unicode `U+00XX` scalar per ordinal. A higher
endpoint reports `RXC-LC-23.1`; Level B `sequence` is the separate non-wrapping
Unicode codepoint operation.

Endpoints must each be one byte or one `U+00XX` scalar; invalid widths report `40.23`.
Argument count reports `40.4`. The optimized/unoptimized direct harness covers
defaults, wrapping, optional holes, raw bytes, text ordinals, and errors.

The distinct native Level B helper is documented in
[`lib/rxfnsb/rexx/xrange.md`](../rxfnsb/rexx/xrange.md).
