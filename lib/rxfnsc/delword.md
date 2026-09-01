# Level C `DELWORD`

The standalone direct entry point is:

```rexx
rexxclassicbif_delword(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `DELWORD(string, start [,count])`. BYTE words use exact byte
positions and space plus configured blank bytes. UTF8 words use codepoint
positions and Unicode 17.0.0 `White_Space` plus configured blank codepoints;
with no additions the scanner retains the VM fast path. The blanks before the
first deleted word are preserved; deleted words and their following blanks are removed.
Omitted count deletes through the end, explicit zero changes nothing, and a
start beyond the available words returns the original string.

The CheckArgs contract is `rANY rWHOLE>0 oWHOLE>=0`. Standard context errors
are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole start or count;
- `RXC-LC-40.14` for nonpositive start; and
- `RXC-LC-40.13` for negative count.

The implementation scans the source directly and extracts at most one prefix
and one suffix. It does not dispatch through the compatibility controller or
call the `WORDS`, `WORDINDEX`, `WORD`, or `SUBSTR` selectors.

`lib/rxfnsc/tests_functional/testRexxClassicBifDelword.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers omitted, zero, and
oversized counts; BYTE/UTF8/default/configured blanks; word boundaries; empty
input; and every standard argument error without using the dispatcher.
