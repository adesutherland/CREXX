# Level C `INSERT`

The standalone direct entry point is:

```rexx
rexxclassicbif_insert(context = reference .RexxBifCallContext) = .RexxValue
```

It implements `INSERT(new, target [,before [,length [,pad]]])`. `before` is the
number of target characters preceding the insertion and defaults to zero. An
omitted `length` uses the character length of `new`; a supplied zero inserts no
new text. A target shorter than `before` and an insertion shorter than `length`
are extended with the one-character pad, which defaults to blank.

The CheckArgs contract is `rANY rANY oWHOLE>=0 oWHOLE>=0 oPAD`. Standard
context errors are:

- `RXC-LC-40.3`, `40.4`, and `40.5` for argument presence/count;
- `RXC-LC-40.12` for a non-whole position or length;
- `RXC-LC-40.13` for a negative position or length; and
- `RXC-LC-40.23` unless a supplied pad is exactly one codepoint.

The implementation works on extracted RexxValue text and returns a new
RexxValue. It caches both character lengths and uses direct VM substring,
append, and pad operations, with no name dispatcher or Level B formatting
helper.

`lib/rxfnsc/tests_functional/testRexxClassicBifInsert.crexx` covers defaults,
omitted optional positions, padding, truncation, Unicode, substantial padding,
and every argument-error class in optimized and unoptimized direct-call
overlays.
