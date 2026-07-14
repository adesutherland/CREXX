# Level C `ABBREV`

The standalone direct entry point is:

```rexx
rexxclassicbif_abbrev(context = reference .RexxBifCallContext) = .RexxValue
```

It implements:

```rexx
ABBREV(string, candidate [, minimum])
```

The comparison is case-sensitive. The result is `1` when `candidate` is a
leading prefix of `string` and contains at least `minimum` characters;
otherwise it is `0`. The omitted minimum is the candidate length. An empty
candidate therefore matches when the minimum is omitted or zero.

The CheckArgs contract is `rANY rANY oWHOLE>=0`. Standard Level C errors are
reported through the call context:

- `RXC-LC-40.3` for too few arguments;
- `RXC-LC-40.4` for too many arguments;
- `RXC-LC-40.5` for an omitted required argument;
- `RXC-LC-40.12` for a non-whole minimum; and
- `RXC-LC-40.13` for a negative minimum.

The implementation checks both lengths before scanning and compares Unicode
codepoints without allocating a substring. The direct opt/noopt harness covers
the documented examples, empty and short inputs, Unicode, and every listed
error. It calls `.rexxclassicbifabbrev..rexxclassicbif_abbrev` and never calls
the deprecated name dispatcher.

`RexxClassicBifs.crexx` temporarily retains an explicitly deprecated internal
compatibility body for current controller/compiler artifacts. Final integration
will register this standalone module and point RexxScript at it; compiler
lowering remains outside this programme.
