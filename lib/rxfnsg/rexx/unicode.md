# `rxunicode` text services

`rxunicode` is the explicit Unicode 17.0.0 Level G module. The canonical user
guide is [Unicode text
services](../../../docs/books/crexx_library_reference/unicode.md); the
[algorithm appendix](../../../docs/books/crexx_vm_spec/unicode_algorithms.md)
describes the prepared tables and execution model. This source-adjacent summary
is retained by the source/docs/tests contract check.

```rexx
options levelg
import rxunicode

normal = rxunicode..toNFC(text)
upper = rxunicode..toUppercase(normal)
folded = rxunicode..toCasefold(upper)
bytes = rxunicode..encode(folded, "UTF-8")
```

## Version and normalization

`version()` returns the shared Unicode data version.

| Transform | Predicate |
| --- | --- |
| `toNFD(text)` | `isNFD(text)` |
| `toNFC(text)` | `isNFC(text)` |
| `toNFKD(text)` | `isNFKD(text)` |
| `toNFKC(text)` | `isNFKC(text)` |

Every call accepts `.string`; transforms return `.string` and predicates return
`.boolean`. Compatibility forms may remove distinctions and must be selected
explicitly. There is no public normalizer object and no implicit normalization.

## Default case mapping and folding

`toUppercase(text)` and `toLowercase(text)` apply full, locale-neutral Unicode
default mappings. They may expand; lowercase includes the default Greek final
sigma context. They do not normalize or select locale tailoring.

| Procedure | Unicode folding mode |
| --- | --- |
| `toCasefold(text)` | Default full (`C + F`) |
| `toSimpleCasefold(text)` | Default simple (`C + S`) |
| `toTurkicCasefold(text)` | Full with `T` override |
| `toTurkicSimpleCasefold(text)` | Simple with `T` override |

## Typed codecs

```text
encode(text[, encoding[, replacement-bytes]]) = .binary
decode(data[, encoding[, replacement-text]]) = .string
isDecodable(data[, encoding]) = .boolean
isEncodingSupported(encoding) = .boolean
```

UTF-8 is the default. Supported families are explicit-endian UTF-16 and UTF-32,
US-ASCII, ISO-8859-1, Windows-1252, IBM437, IBM850, and IBM1047. Conversion is
strict when the third argument is omitted. Supplying a non-empty typed third
argument opts into replacement. A BOM is never added or consumed as metadata.

## Default extended grapheme clusters

The direct UAX #29 `UAX29-C1-1` procedures are:

| Procedure | Contract |
| --- | --- |
| `graphemeCount(text)` | Count clusters. |
| `graphemeSubstr(text, start[, length[, pad]])` | Select by one-based cluster position. |
| `graphemePos(needle, haystack[, start])` | Find an exact boundary-aligned occurrence. |
| `graphemeReverse(text)` | Reverse cluster order. |

`.rxunicode..graphemes(text)` creates an immutable indexed snapshot for
repeated `text`, `count`, `at`, `substr`, `pos`, `reverse`, `codepointStart`,
`codepointLength`, `iterator`, `version`, and `profile` calls. Its iterator
supports `hasNext`, `next`, `index`, `reset`, `codepointStart`, and
`codepointLength`.

All operations keep `.string` text, `.binary` bytes, codepoint indexes, and
grapheme indexes explicit. None changes ordinary equality or Level B string
semantics.
