# Unicode Text Services

`rxunicode` is cREXX's explicit Unicode 17.0.0 library. It normalizes text,
performs full default case conversion and case folding, segments default
extended grapheme clusters, and converts between `.string` text and `.binary`
encoded bytes.

```rexx
options levelg
import rxfnsb
import rxunicode

text = rxunicode..decode(readbinary("input.txt"), "Windows-1252")
text = rxunicode..toNFC(text)
text = rxunicode..toUppercase(text)
bytes = rxunicode..encode(text, "UTF-8")
call writebinary "output.txt", bytes
```

Importing `rxunicode` does not change ordinary string equality, codepoint
indexing, Level B `upper` or `lower`, file I/O, or `.binary` semantics. Every
potentially expensive or information-changing operation remains visible at the
call site.

## Text, bytes, and graphemes

When choosing a representation or indexing unit, cREXX keeps bytes, Unicode
codepoints, and user-perceived characters distinct:

| Concern | cREXX surface |
| --- | --- |
| Arbitrary or encoded bytes | `.binary`, `readbinary`, and `writebinary` |
| Valid UTF-8 text and scalar positions | `.string` and ordinary Level B string operations |
| User-perceived character boundaries | explicit `rxunicode` grapheme operations |

Ordinary `.string` length, position, substring, and reverse operations count
Unicode codepoints. Use the grapheme family when the application means a
user-perceived character. Use `decode` and `encode` when crossing a byte/text
boundary; a cast alone does not express a legacy encoding.

`rxunicode..version()` returns the Unicode data version used by every
versioned algorithm in the module, currently `17.0.0`.

## API at a glance

The public API has two forms: stateless namespace procedures for one-off work,
and an indexed grapheme snapshot for repeated boundary access. Whole-file byte
I/O is supplied separately by Level B.

### Namespace procedures

Most `rxunicode` services are namespace procedures. Their static types are:

```text
version() = .string

toNFD(text = .string) = .string
toNFC(text = .string) = .string
toNFKD(text = .string) = .string
toNFKC(text = .string) = .string
isNFD(text = .string) = .boolean
isNFC(text = .string) = .boolean
isNFKD(text = .string) = .boolean
isNFKC(text = .string) = .boolean

toUppercase(text = .string) = .string
toLowercase(text = .string) = .string
toCasefold(text = .string) = .string
toSimpleCasefold(text = .string) = .string
toTurkicCasefold(text = .string) = .string
toTurkicSimpleCasefold(text = .string) = .string

graphemeCount(text = .string) = .int
graphemeSubstr(text = .string, start = .int
               [, length = .int [, pad = .string]]) = .string
graphemePos(needle = .string, haystack = .string
            [, start = .int]) = .int
graphemeReverse(text = .string) = .string

encode(text = .string [, encoding = .string
       [, replacement = .binary]]) = .binary
decode(data = .binary [, encoding = .string
       [, replacement = .string]]) = .string
isDecodable(data = .binary [, encoding = .string]) = .boolean
isEncodingSupported(encoding = .string) = .boolean
```

The codec `encoding` argument defaults to `UTF-8`. The defaults and error rules
for the other optional arguments are described with the relevant operation
families below.

### Indexed grapheme classes

Repeated grapheme indexing is the one public class-based family. Calling
`.rxunicode..graphemes(text)` captures an immutable text snapshot and prepares
its boundary index once:

```text
.rxunicode..graphemes(text = .string) = .graphemes
  text() = .string
  count() = .int
  at(position = .int) = .string
  substr(start = .int [, length = .int [, pad = .string]]) = .string
  pos(needle = .string [, start = .int]) = .int
  reverse() = .string
  codepointStart(position = .int) = .int
  codepointLength(position = .int) = .int
  iterator() = .graphemeiterator
  version() = .string
  profile() = .string

.graphemeiterator
  hasNext() = .int
  next() = .string
  index() = .int
  reset() = .void
  codepointStart() = .int
  codepointLength() = .int
```

Applications obtain `.graphemeiterator` from the snapshot's `iterator()`
method. The iterator's packed-boundary constructor arguments are internal and
are not a supported application entry point.

### Related Level B file procedures

Whole-file byte I/O belongs to Level B rather than `rxunicode`. The related
procedures are `readbinary(path = .string) = .binary` and
`writebinary(path = .string, data = .binary) = .int`. The codec section below
shows how to compose them with `decode` and `encode`.

## Normalization

Normalization makes selected canonically or compatibly equivalent scalar
sequences use a defined Unicode form:

| Transform | Predicate | Meaning |
| --- | --- | --- |
| `toNFD(text)` | `isNFD(text)` | Canonical decomposition |
| `toNFC(text)` | `isNFC(text)` | Canonical decomposition followed by composition |
| `toNFKD(text)` | `isNFKD(text)` | Compatibility decomposition |
| `toNFKC(text)` | `isNFKC(text)` | Compatibility decomposition followed by composition |

Each transform accepts and returns `.string`. Each predicate accepts `.string`
and returns `.boolean`.

```rexx
decomposed = "65cc81"x        /* e plus combining acute */
composed = rxunicode..toNFC(decomposed)

if rxunicode..isNFC(composed) then say "NFC"
```

NFD and NFC preserve canonical meaning but can change codepoint count and byte
representation. NFKD and NFKC may additionally remove compatibility
distinctions such as presentation forms, width variants, and some formatting
differences. Select compatibility normalization only when that loss is part of
the application contract.

No assignment, comparison, concatenation, codec, case operation, grapheme
operation, or file operation normalizes implicitly. There is no public
normalizer object: direct procedures keep ownership and lifecycle simple while
the runtime may use trustworthy string certificates internally.

## Full default case conversion

`toUppercase(text)` and `toLowercase(text)` apply Unicode's locale-neutral full
default mappings and return `.string`. They can expand their input; for example,
default uppercase maps German `ß` to `SS`. Default lowercase includes Unicode's
context-sensitive Greek final-sigma rule.

```rexx
say rxunicode..toUppercase("Straße")
say rxunicode..toLowercase("ΟΣ")
```

The procedures do not normalize and do not select Turkish, Azerbaijani,
Lithuanian, or another locale. cREXX deliberately does not expose titlecase or
a locale object in this baseline. Level B `upper` and `lower` retain their
existing simple runtime contracts; use the `rxunicode` names when the complete
Unicode default mapping is required.

## Case folding

Case folding produces a stable form for caseless matching. It is not
presentation-oriented uppercasing or lowercasing.

| Procedure | Result |
| --- | --- |
| `toCasefold(text)` | Default full folding using Unicode `C + F` mappings |
| `toSimpleCasefold(text)` | Default simple, non-expanding folding using `C + S` mappings |
| `toTurkicCasefold(text)` | Full folding with the Unicode Turkic `I` mappings |
| `toTurkicSimpleCasefold(text)` | Simple folding with the Unicode Turkic `I` mappings |

Full default folding is the ordinary choice for locale-independent caseless
keys and comparisons. It may expand: `ß` folds to `ss`. Simple folding maps no
more than one output scalar for each input scalar but is not a substitute for
full folding when expansion is semantically required.

```rexx
key1 = rxunicode..toCasefold(name1)
key2 = rxunicode..toCasefold(name2)
if key1 == key2 then say "same caseless key"
```

The Turkic procedures explicitly select the special dotted and dotless I
mappings used for Turkish and Azerbaijani. They do not change a process, task,
or object locale.

Folding performs no normalization. Canonical caseless matching and
`NFKC_Casefold` are separate contracts; an application that needs either must
define its normalization/folding sequence. Expansion also means result indexes
cannot be mapped back to source indexes without an application-owned map.

The four direct procedures are the complete case-folding API. Case folding
retains no useful state between independent strings, so the library does not
add a reusable case-folder object around the same operation.

## Default extended grapheme clusters

The grapheme family implements Unicode Standard Annex #29 revision 47 profile
`UAX29-C1-1`: default extended grapheme clusters without tailoring. These
boundaries keep important sequences together, including a base plus combining
marks, emoji joined with zero-width joiners, and paired regional indicators.
They do not claim to count glyphs, terminal columns, or language-specific
words.

```rexx
familyEmoji = "👩‍👩‍👧‍👦"
text = "A" || familyEmoji || "B"

say rxunicode..graphemeCount(text)
say rxunicode..graphemeSubstr(text, 2, 1)
say rxunicode..graphemePos(familyEmoji, text)
say rxunicode..graphemeReverse(text)
```

| Procedure | Contract |
| --- | --- |
| `graphemeCount(text)` | Count default extended grapheme clusters. |
| `graphemeSubstr(text, start[, length[, pad]])` | Select from a one-based grapheme position. With `length`, right-pad to that many graphemes. |
| `graphemePos(needle, haystack[, start])` | Find an exact occurrence whose beginning and end are grapheme boundaries; return its one-based grapheme position or zero. |
| `graphemeReverse(text)` | Reverse cluster order while preserving codepoint order inside each cluster. |

`graphemeSubstr` follows the Rexx `SUBSTR` shape. `start` must be positive and
a supplied `length` must be non-negative. The default pad is one blank. A
supplied pad may contain several codepoints but must form exactly one grapheme
cluster. Without `length`, the result extends to the end and is not padded.

Search is exact: neither argument is normalized or folded. A sequence occurring
only inside a cluster is not a match, and canonically equivalent but differently
encoded text remains different unless normalized separately.

### Indexed grapheme snapshot

Direct calls suit one count, bounded slice, search, or reverse. Repeated indexed
work should prepare one immutable snapshot:

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
say view.version()
say view.profile()
```

| Method | Contract |
| --- | --- |
| `text()` | Return the captured text. |
| `count()` | Return the grapheme count. |
| `at(position)` | Return one grapheme at a one-based position. |
| `substr(start[, length[, pad]])` | Apply the direct substring contract using the retained index. |
| `pos(needle[, start])` | Apply the direct exact-search contract using the retained index. |
| `reverse()` | Reverse the retained text by cluster. |
| `codepointStart(position)` | Return the cluster's one-based `.string` codepoint position. |
| `codepointLength(position)` | Return the cluster length in `.string` codepoints. |
| `iterator()` | Return a fresh forward iterator. |
| `version()` | Return `17.0.0`. |
| `profile()` | Return `UAX29-C1-1`. |

The snapshot owns a packed boundary index proportional to the number of
boundaries. Its iterator implements `.StringIterator`:

```rexx
iterator = view.iterator()
do while iterator.hasNext()
  cluster = iterator.next()
  say iterator.index(), iterator.codepointStart(),
      iterator.codepointLength(), cluster
end
call iterator.reset()
```

`index()` returns zero before the first item and then the current one-based
grapheme position. Position methods require a current item. Invalid indexed
access and `next()` after exhaustion signal `OUT_OF_RANGE`.

## Encoding and decoding

The whole-value codec boundary is typed and strict by default:

```text
encode(text = .string[, encoding = "UTF-8"[, replacement = .binary]]) = .binary
decode(data = .binary[, encoding = "UTF-8"[, replacement = .string]]) = .string
isDecodable(data = .binary[, encoding = "UTF-8"]) = .boolean
isEncodingSupported(encoding = .string) = .boolean
```

The supported canonical encodings are:

| Canonical name | Accepted aliases |
| --- | --- |
| `UTF-8` | case variations; hyphens and underscores are ignored, for example `utf_8` |
| `UTF-16LE`, `UTF-16BE` | case variations with optional hyphens or underscores |
| `UTF-32LE`, `UTF-32BE` | case variations with optional hyphens or underscores |
| `US-ASCII` | `ASCII`, `ISO-646-US` |
| `ISO-8859-1` | `latin1`, `ISOLatin1`, `IBM819`, `CP819`, `819` |
| `Windows-1252` | `Windows1252`, `CP1252`, `1252` |
| `IBM437` | `CP437`, `437` |
| `IBM850` | `CP850`, `850` |
| `IBM1047` | `CP1047`, `1047` |

Hyphens and underscores are ignored, surrounding whitespace is trimmed, and
matching is case-insensitive. Internal blanks are not accepted. Ambiguous
endian names such as `UTF-16` and `UTF-32` are not accepted.

`encode` never adds a byte-order mark. `decode` never consumes one as metadata:
a BOM encoded in the selected format becomes U+FEFF in the resulting text.
Callers that own a protocol-level BOM convention must add, remove, or interpret
it explicitly.

The strict form signals `UNICODE_ERROR` for malformed input or text that the
target cannot represent:

```rexx
utf16 = rxunicode..encode(text, "UTF-16LE")
text = rxunicode..decode(utf16, "UTF-16LE")

if rxunicode..isDecodable(bytes, "UTF-8") then
  text = rxunicode..decode(bytes)
```

An explicitly supplied third argument opts into replacement. Decode
replacement is non-empty `.string` text and is inserted once per maximal
malformed input subpart. Encode replacement is the exact non-empty `.binary`
byte sequence to emit for each unmappable scalar, and those bytes must
themselves be valid in the target encoding.

```rexx
text = rxunicode..decode("C08041"x as .binary, "UTF-8", "?")  /* ??A */

question = "3F"x as .binary
bytes = rxunicode..encode("Tea ☕", "Windows-1252", question)

ebcdicQuestion = "6F"x as .binary
bytes = rxunicode..encode(text, "IBM1047", ebcdicQuestion)
```

Replacement bytes are target bytes, not UTF-8 spelling. Thus ASCII and
Windows-1252 use byte `3F` for `?`, while IBM1047 uses byte `6F`. Supplying an
empty or invalid replacement signals `INVALID_ARGUMENTS`. Omitting the third
argument is not the same as supplying an empty replacement.

Windows-1252 follows the retained TUTOR mapping policy for its five undefined
byte positions: they decode to the matching C1 control codepoints and can be
encoded back. Every byte value round-trips for all four table-backed legacy
encodings.

### Whole-file conversion

`readbinary(path)` and `writebinary(path, data)` are Level B whole-file byte
operations. They make the simplest file conversion explicit:

```rexx
options levelg
import rxfnsb
import rxunicode

source = readbinary("legacy.txt")
text = rxunicode..decode(source, "IBM850")
text = rxunicode..toNFC(text)
target = rxunicode..encode(text, "UTF-8")
written = writebinary("unicode.txt", target)
```

`readbinary` preserves every byte, including embedded NUL and malformed UTF-8.
`writebinary` replaces the file with exactly the supplied bytes and returns the
number written. Both signal `NOTREADY` for file failures. They load or write a
complete value; incremental codecs and encoded stream adapters are not part of
this baseline.

## Errors, mutation, and ownership

- Unsupported encoding names and invalid replacement values signal
  `INVALID_ARGUMENTS`.
- Strict codec conversion failures signal `UNICODE_ERROR`.
- Invalid grapheme arguments signal `INVALID_ARGUMENTS` or `OUT_OF_RANGE` as
  described above.
- Corrupt generated data or an internal invariant failure signals `FAILURE`.
- Public operations do not mutate their arguments or expose generated mapping
  tables.
- `.string` is valid UTF-8 text; `.binary` remains arbitrary bytes. Codec
  conversion does not create a mutable cross-type alias.

## For TUTOR users

[TUTOR](https://rexx.epbcn.com/TUTOR/) is an important catalogue of Unicode
problems and Rexx-friendly vocabulary. cREXX shares names where the complete
semantics align while retaining its statically typed `.string`/`.binary`
foundation.

| Topic | TUTOR vocabulary | cREXX baseline |
| --- | --- | --- |
| Bytes and valid text | BYTES and several text string kinds | `.binary` for bytes; valid UTF-8 `.string` for text |
| Codepoint operations | CODEPOINTS values/conversions | ordinary `.string` operations count codepoints |
| Grapheme operations | GRAPHEMES values/conversions | explicit procedures and immutable `.graphemes` snapshot |
| Normalization | `toNFD`, `toNFC`, `toNFKD`, `toNFKC`, and predicates | the same direct names; never implicit |
| Default full case fold | `toCasefold` | the same direct name, plus explicitly named simple and Turkic modes |
| Default case mapping | Unicode upper/lower services | `toUppercase` and `toLowercase`; no titlecase/locale surface in this baseline |
| Encoding boundary | `ENCODE`, `DECODE`, registry aliases | typed `encode(.string)->.binary` and `decode(.binary)->.string` with selected aliases |
| Mapping inputs | TUTOR Format A mapping files | Format A accepted at build time; cREXX emits its own checked runtime image |
| Malformed input | replacement-oriented defaults are available | strict by default; typed replacement is explicit |
| Encoded streams | encoding can be associated with classic streams | whole-value codecs plus binary file I/O; incremental adapters are roadmap work |
| Generic dispatch | `UNICODE(text, operation)` | no heterogeneous dispatch BIF in the baseline; direct typed procedures are authoritative |
| Implicit normalized TEXT | TEXT can carry normalized semantics | no implicit normalization; ordinary equality remains exact |
| Literal/string-kind suffixes | Y/P/G/T/U forms | not added; existing typed values and explicit services are retained |

For a port:

1. Type byte-bearing values as `.binary` and text values as `.string`.
2. Keep the shared normalization, fold, encode, and decode names when their
   error and endian contracts match.
3. Replace a repeated-index GRAPHEMES conversion with
   `.rxunicode..graphemes(text)`; use direct functions for one-off work.
4. Make any TUTOR implicit-normalization or replacement dependency explicit.
5. Supply exact target bytes for encode replacement, especially for EBCDIC.

This alignment shares source vocabulary without importing dynamic string-kind
coercion, mutable global Unicode policy, or hidden transformations into the
cREXX language core. A later thin TUTOR-compatibility facade remains possible
only for operations whose types, Unicode version, endian behavior, indexing
unit, and failure policy are exactly the same; it would delegate to this typed
surface rather than become a second semantic authority.

## Deliberate baseline boundary

The module does not yet promise incremental codecs, encoded stream adapters,
Unicode properties/names, normalized-caseless key profiles, security profiles,
collation, word or sentence boundaries, locale tailoring, or display width.
These remain separate designs because each needs its own error, version,
indexing, and ownership contract.

The [Unicode algorithm appendix](../crexx_vm_spec/unicode_algorithms.md)
describes the prepared data, algorithms, complexity, constant ownership, and
performance evidence behind this surface.
