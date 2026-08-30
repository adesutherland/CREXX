# `rxunicode` text services

`rxunicode` provides explicit Unicode 17.0.0 case folding and default extended
grapheme clusters for valid cREXX `.string` values. It does not change ordinary
equality, Level B `upper` or `lower`, codepoint indexing, normalization, or
`.binary` semantics.

```rexx
options levelg
import rxunicode

say rxunicode..toCasefold("Straße")       /* strasse */
say rxunicode..toSimpleCasefold("ẞ")      /* ß */
```

## Case-fold procedures

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

## Default extended grapheme clusters

The grapheme family implements UAX #29 revision 47 conformance profile
`UAX29-C1-1`: the default extended grapheme-cluster rules with no tailoring.
These boundaries approximate user-perceived characters and keep sequences such
as a base plus combining marks, emoji ZWJ families, and paired regional
indicators together.

```rexx
text = "A" || familyEmoji || "B"

say rxunicode..graphemeCount(text)
say rxunicode..graphemeSubstr(text, 2, 1)
say rxunicode..graphemePos(familyEmoji, text)
say rxunicode..graphemeReverse(text)
```

| Procedure | Contract |
| --- | --- |
| `graphemeCount(text)` | Count default extended grapheme clusters. |
| `graphemeSubstr(text, start[, length[, pad]])` | Select by one-based grapheme position; with a length, pad on the right to the requested grapheme count. `pad` must be exactly one grapheme. |
| `graphemePos(needle, haystack[, start])` | Find an exact match whose start and end are both grapheme boundaries; return its one-based grapheme position or zero. |
| `graphemeReverse(text)` | Reverse cluster order without reversing codepoints inside a cluster. |

None of these operations normalizes or case-folds. Exact codepoint content
therefore remains significant to `graphemePos`.

## Indexed grapheme snapshot

Repeated indexed work should prepare one immutable view:

```rexx
view = .rxunicode..graphemes(text)

say view.text()
say view.count()
say view.at(2)
say view.substr(2, 3)
say view.pos(needle)
say view.reverse()
say view.codepointStart(2)
say view.codepointLength(2)
say view.version()     /* 17.0.0 */
say view.profile()     /* UAX29-C1-1 */

iterator = view.iterator()
do while iterator.hasNext()
  cluster = iterator.next()
  say iterator.index() iterator.codepointStart(),
      iterator.codepointLength() cluster
end
```

`codepointStart` is one-based and `codepointLength` is measured in cREXX
`.string` codepoints, not UTF-8 bytes. The iterator starts at index zero;
`codepointStart()` and `codepointLength()` require a current item. Calling
`next()` after exhaustion signals `OUT_OF_RANGE`.

Direct count and bounded substring operations stream codepoints and do not
allocate a complete boundary vector. `.graphemes(text)` deliberately builds a
private packed codepoint-boundary index once, then reuses it for access,
substring, position, reverse, and iteration. The runtime reads one immutable prepared
property constant using RXVM `STRCHAR` and `BGETU8`; it has no re2c decoder,
UTF-32 input copy, string/binary conversion, runtime table copy, or VM grapheme
cache flag.
