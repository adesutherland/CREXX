# `rxunicode` case folding

`rxunicode` provides explicit Unicode 17.0.0 case folding for valid cREXX
`.string` values. It does not change ordinary equality, Level B `upper` or
`lower`, normalization, indexing, or `.binary` semantics.

```rexx
options levelg
import rxunicode

say rxunicode..toCasefold("Straße")       /* strasse */
say rxunicode..toSimpleCasefold("ẞ")      /* ß */
```

## Convenience procedures

| Procedure | Unicode mapping |
| --- | --- |
| `toCasefold(text)` | Default full folding (`C + F` records). This is the TUTOR-compatible name. |
| `toSimpleCasefold(text)` | Default simple, non-expanding folding (`C + S`). |
| `toTurkicCasefold(text)` | Full folding with the Unicode `T` mappings for `I` and dotted `İ`. |
| `toTurkicSimpleCasefold(text)` | Simple folding with the Unicode `T` mappings. |

Every procedure accepts and returns `.string`. Full folding can expand text;
for example, German `ß` folds to `ss`. Simple folding is useful only when a
one-codepoint result is required and is not a substitute for full caseless
matching.

The Turkic operations implement the special Unicode case-fold mappings used
for Turkish and Azerbaijani. They are explicit operations, not a locale switch.
Default folding excludes those mappings.

## Reusable folder

One immutable `.casefolder` fixes the mode for repeated calls:

```rexx
full = .rxunicode..casefolder.full()
simple = .rxunicode..casefolder.simple()
turkic = .rxunicode..casefolder.turkic()
turkicSimple = .rxunicode..casefolder.turkicSimple()

folded = full.fold(text)
say full.mode()       /* FULL */
say full.version()    /* 17.0.0 */
```

`mode()` returns `FULL`, `SIMPLE`, `TURKIC_FULL`, or `TURKIC_SIMPLE`.
`version()` returns the pinned Unicode data version.

## Important boundaries

- Case folding supports caseless matching; it is not presentation-oriented
  uppercasing or lowercasing.
- Case folding does not preserve normalization in general. Normalize separately
  when a comparison contract requires a particular normalization/folding order.
- None of these calls normalizes implicitly.
- `.binary` has no case-fold overload. Decode bytes to a validated `.string`
  through a future codec boundary before applying Unicode text operations.
- The result preserves no source-position mapping. Expansions mean result
  indexes need not correspond to source indexes.

The implementation reads one immutable prepared table and iterates with RXVM
codepoint operations. It does not copy the input through `.binary`, use a
separate UTF-8 decoder, or materialize UTF-32 input.
