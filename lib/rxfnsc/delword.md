# Level C `DELWORD`

The standalone direct entry point is:

```rexx
rexxclassicbif_delword(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `DELWORD(string, start [,count])`. Words are delimited by the
VM's Unicode whitespace classification. The blanks before the first deleted
word are preserved; deleted words and their following blanks are removed.
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

The direct implementation currently uses the VM's standard Unicode whitespace
set. Supporting a configured `Config_OtherBlankCharacters` set is tracked as a
shared dependency for the word-family Level C BIFs rather than being hidden in
this selector.

`lib/rxfnsc/tests_functional/testRexxClassicBifDelword.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers omitted, zero, and
oversized counts; leading and Unicode whitespace; word boundaries; empty input;
and every standard argument error without using the compatibility dispatcher.
