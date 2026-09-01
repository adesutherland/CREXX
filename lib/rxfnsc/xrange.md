# `XRANGE` (Level C Classic BIF)

```text
XRANGE([start [, end]])
CheckArgs: oPAD oPAD
```

The standalone `rexxclassicbif_xrange` entry is available in the BYTE profile.
It returns the inclusive configured byte range, wraps after `FF`, and defaults
to `00` through `FF`. Supplying only `start` selects through `FF`; an omitted
`start` with an explicit `end` starts at `00`. Every result contains at most 256
bytes and is binary-authoritative.

UTF8 reports `RXC-LC-40.1`: Classic XRANGE is not a Unicode scalar or grapheme
range. Level B `sequence` is the non-wrapping Unicode codepoint operation.

BYTE endpoints must each be one exact byte; invalid endpoints report `40.23`.
Argument count reports `40.4`. The optimized/unoptimized direct harness covers
defaults, wrapping, optional holes, raw bytes, profile rejection, and errors.

The distinct native Level B helper is documented in
[`lib/rxfnsb/rexx/xrange.md`](../rxfnsb/rexx/xrange.md).
