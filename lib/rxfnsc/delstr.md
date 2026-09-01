# Level C `DELSTR`

The standalone direct entry point is:

```rexx
rexxclassicbif_delstr(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `DELSTR(string, start [,length])` over 1-based configured
character positions. BYTE is the default and positions exact octets; UTF8 is
opt-in and positions Unicode codepoints. Omitted length deletes through the
end; explicit zero changes nothing; start beyond the value returns the original
payload. BYTE results remain binary-authoritative.

The CheckArgs contract is `rANY rWHOLE>0 oWHOLE>=0`. Standard context errors
are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole start or length; and
- `RXC-LC-40.14` for nonpositive start; and
- `RXC-LC-40.13` for negative length.

The implementation checks optional-argument presence explicitly, computes the
source length once, and extracts at most one prefix and one suffix using direct
VM string or binary operations.

`lib/rxfnsc/tests_functional/testRexxClassicBifDelstr.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers omitted/zero/oversized
lengths, boundaries, Unicode, and every standard argument error without using
the compatibility dispatcher.
