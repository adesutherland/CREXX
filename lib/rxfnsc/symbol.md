# Level C `SYMBOL` BIF

The standalone Level C entry point is:

```rexx
result = rexxclassicbif_symbol(reference context)
```

The context supplies exactly one `RexxValue` argument and the caller's active
`RexxVariablePool`. The result is a `RexxValue` containing:

- `BAD` when the complete argument would not be recognized as one Classic REXX
  symbol;
- `LIT` when it is a valid constant symbol or a valid variable symbol whose
  final pool lookup is dropped;
- `VAR` when the final pool lookup has a value.

Classic REXX has contextual keywords, not a general reserved-word list, so names
such as `IF`, `DO`, and `PARSE` are valid variable symbols here. The configured
base general-letter set is ASCII letters plus `_`, `!`, and `?`; no extra-letter
characters are configured. A variable symbol may then contain general letters,
digits, and periods. Constant symbols start with a digit or period; nonnumeric
period-start symbols are limited to `.MN`, `.RESULT`, `.RC`, `.RS`, and `.SIGL`.

Compound names are derived through the caller pool. Nondigit tail components
use their pool value when present and their own uppercase spelling when dropped.
The final derived tail is checked without creating a binding or raising
`NOVALUE`.

## Errors and encoding

Missing, omitted, and extra arguments use the shared Level C context error
codes `RXC-LC-40.3`, `RXC-LC-40.5`, and `RXC-LC-40.4`. An invalid symbol is not
an invocation error and returns `BAD`. `RexxValue.asString()` supplies the
runtime text view; invalid binary-to-text conversion is therefore reported by
the standard RexxValue/VM signal path. This configuration imposes no additional
symbol-length maximum.

## Coverage and performance

`lib/rxfnsc/tests_functional/testRexxClassicBifSymbol.crexx` directly covers
simple, dropped, contextual-keyword, extra-general-letter, constant, reserved,
stem, substituted compound, empty-tail, invalid, and argument-error cases in
optimized and unoptimized overlays.

Validation is one linear pass over the candidate. Simple pool lookup is a hash
query. Compound lookup is linear in the symbol text plus hash queries for its
tail components; it does not scan the entire pool.
