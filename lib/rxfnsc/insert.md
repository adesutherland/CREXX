# Level C `INSERT`

The standalone direct entry point is:

```rexx
rexxclassicbif_insert(context = reference .RexxBifCallContext) = .RexxValue
```

It implements `INSERT(new, target [,before [,length [,pad]]])`. `before` is the
number of configured target units preceding the insertion and defaults to zero.
An omitted `length` uses the configured-unit length of `new`; a supplied zero
inserts no new content. BYTE is the default and operates on exact octets; UTF8
is opt-in and operates on Unicode codepoints. A target shorter than `before`
and an insertion shorter than `length` are extended with the one-unit pad,
which defaults to blank. BYTE results remain binary-authoritative.

The CheckArgs contract is `rANY rANY oWHOLE>=0 oWHOLE>=0 oPAD`. Standard
context errors are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole position or length;
- `RXC-LC-40.13` for a negative position or length; and
- `RXC-LC-40.23` unless a supplied pad is exactly one configured unit.

The implementation caches both configured-unit lengths and uses direct VM
string or binary slice, append, and pad operations, with no name dispatcher or
Level B formatting helper.

`lib/rxfnsc/tests_functional/testRexxClassicBifInsert.crexx` covers defaults,
omitted optional positions, padding, truncation, Unicode, substantial padding,
and every argument-error class in optimized and unoptimized direct-call
overlays.
