# Level C `DIGITS`, `FORM`, and `FUZZ`

The standalone direct entry points are:

```rexx
.rexxclassicbifnumeric..rexxclassicbif_digits(
    context = reference .RexxBifCallContext) = .RexxValue
.rexxclassicbifnumeric..rexxclassicbif_form(
    context = reference .RexxBifCallContext) = .RexxValue
.rexxclassicbifnumeric..rexxclassicbif_fuzz(
    context = reference .RexxBifCallContext) = .RexxValue
```

Each is a zero-argument BIF and returns the setting inherited from its immediate
caller:

- `DIGITS()` returns the current positive significant-digit count as a
  RexxValue integer.
- `FORM()` returns `SCIENTIFIC` or `ENGINEERING` as RexxValue text.
- `FUZZ()` returns the current non-negative ignored-digit count as a RexxValue
  integer.

Supplying any argument reports standard context error `RXC-LC-40.4`; a valid
call clears a previous context error. The functions are pure and do not modify
the caller's numeric settings.

Each direct procedure inherits only the setting it returns and reads it with a
single VM `getnum*` instruction. This keeps caller-context behavior local to the
direct call and avoids adding capture state or overhead to the shared BIF
context. The functions do not invoke the name controller or the Level B
accessors.

`lib/rxfnsc/tests_functional/testRexxClassicBifNumeric.crexx` calls all three
standalone entries in optimized and unoptimized overlays. It covers two
distinct caller contexts, both forms, multiple DIGITS/FUZZ values, and the
excess-argument error for every entry.
