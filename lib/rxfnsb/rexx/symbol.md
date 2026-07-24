## Level B `symbol`

`symbol(name)` classifies a candidate against the compiled Level B source scope
at the call site.

```rexx
count = 3
say symbol("count")  /* VAR */
say symbol("unused") /* LIT */
say symbol("a*b")    /* BAD */
```

The `.string` result is:

- `BAD` when the text is empty, contains a character outside ASCII letters,
  digits, underscore, and period, or is a Level B reserved word;
- `VAR` when current caller metadata identifies the name as a visible variable
  or constant binding;
- `LIT` when the name is valid but has no visible binding.

Level B has reserved words and compiled typed locals, so this contract is
deliberately different from the Classic Level C `SYMBOL` BIF. Invalid candidate
text is normal classification and returns `BAD`; it is not a runtime error and
does not raise a signal.

## Coverage and performance

`lib/rxfnsb/tests_functional/ts_symbol.crexx` covers visible integer/string
variables, literal values, unused names, exact reserved-word matching, dotted
constants, underscore names, empty text, and invalid characters in optimized
and unoptimized selector overlays.

The implementation validates the input once, normalizes it once, precomputes
the exact keyword and metadata probes, then scans caller metadata backwards only
as far as the current procedure boundary.
