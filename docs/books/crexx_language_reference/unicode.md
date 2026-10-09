# Unicode

cREXX separates text semantics by language level. This avoids making Classic
byte-oriented programs and modern Unicode programs silently share incompatible
rules.

Unicode property tables that claim a version are pinned to Unicode 17.0.0.
Updating that version requires updating the tables, tests, and this document
together. Level B's deliberately limited case table is not a claim to implement
the complete Unicode 17 case algorithm.

## Level B

Level B `.string` values are valid UTF-8. Their public character operations use
Unicode scalar/codepoint positions and lengths, never UTF-8 byte offsets.
`.binary` is the distinct type for arbitrary bytes.

Level B intentionally supplies a limited Unicode foundation rather than the
full Unicode algorithm suite:

- string length, indexing, slicing, searching, and reversal are codepoint based;
- default word blanks use the Unicode `White_Space` property;
- case conversion uses the runtime's locale-independent simple mapping;
- operations do not normalize text implicitly;
- canonical equivalence and locale-sensitive comparison are not inferred.

Text I/O into a Level B `.string` validates UTF-8. Invalid byte sequences signal
`UNICODE_ERROR`; they are not repaired or reinterpreted. `LINEIN` returns text
lines and `CHARIN` counts codepoints. Text output writes the string's UTF-8
encoding. Arbitrary file or protocol bytes belong in `.binary` and byte-oriented
I/O APIs rather than being smuggled through `.string`.

The VM whitespace table and the Level C Unicode text route are pinned to the same
Unicode version. U+180E is not whitespace in that version.

## Level G

Level G owns general-purpose Unicode behavior above the Level B codepoint
foundation. The `rxunicode` namespace provides explicit Unicode 17.0.0
normalization, full default case mapping, case folding, default extended
grapheme-cluster operations, and typed byte/text codecs. It is imported
explicitly:

```rexx
options levelg
import rxunicode
```

The normalization family is `toNFD`, `toNFC`, `toNFKD`, and `toNFKC` plus the
matching `isNFD`, `isNFC`, `isNFKD`, and `isNFKC` predicates. Normalization is
never implicit. Compatibility forms may remove distinctions and must be
selected deliberately. There is no public normalizer object.

`toUppercase` and `toLowercase` apply locale-neutral Unicode full default case
mappings. They may expand and do not normalize. They are distinct from the
existing Level B simple `upper`/`lower` contract. Titlecase and locale-tailored
case mapping are not part of the baseline.

The case-fold procedures are:

- `toCasefold(text)` for default full folding;
- `toSimpleCasefold(text)` for default simple folding;
- `toTurkicCasefold(text)` for full folding with Turkic I mappings; and
- `toTurkicSimpleCasefold(text)` for simple folding with Turkic I mappings.

The shortest name is Unicode Default Case Folding and is compatible with the
useful TUTOR vocabulary. Full folding may expand a string, while simple folding
does not. These direct procedures are the complete public case-fold surface;
there is no reusable case-folder object because folding retains no useful
state between calls.

Case folding is for caseless matching. It is not locale-sensitive case
conversion, does not preserve or apply normalization, and does not retain a
source-to-result index map. The Turkic forms are explicit operations rather
than process or task locale state. `.binary` is not accepted.

The direct grapheme procedures are `graphemeCount`, `graphemeSubstr`,
`graphemePos`, and `graphemeReverse`. They implement the default extended
grapheme-cluster profile `UAX29-C1-1` without tailoring. The immutable
`.graphemes` class prepares one boundary index for repeated access, slicing,
searching, reversal, and forward iteration. Grapheme operations neither
normalize nor case-fold their input.

`encode(.string[, encoding[, replacement]])` returns `.binary` and
`decode(.binary[, encoding[, replacement]])` returns `.string`. UTF-8 is the
default. The baseline also supports explicit-endian UTF-16/UTF-32, US-ASCII,
ISO-8859-1, Windows-1252, IBM437, IBM850, and IBM1047. Conversion is strict by
default; an explicitly supplied typed third argument opts into replacement.
Codecs do not add or consume a BOM as metadata. `isDecodable` checks a complete
byte value and `isEncodingSupported` checks a name. Whole-file `readbinary`
and `writebinary` operations let callers compose binary I/O with these codecs.

These algorithms are explicit services. Level B does not silently acquire
grapheme, normalization, case-folding, or encoded-stream semantics when
`rxunicode` is imported. Ordinary string comparison and indexing remain exact
and codepoint based.

See the [Unicode text services](../crexx_library_reference/unicode.md) chapter
for the complete API, codec/file examples, error boundaries, and TUTOR migration
guide.

## Level C

The approved 2026-10-09 contract uses Unicode scalar strings. LENGTH,
substring/search, PARSE positions and table TRANSLATE count codepoints.
Unicode White_Space plus configured additions remains the word-blank default.
Values and comparisons preserve exact text without implicit normalization.
Symbols use Unicode letters and simple non-expanding case mappings; full case
mapping, folding, normalization and grapheme operations are explicit BIFs.

Byte `XX` maps exactly to U+00XX for all 256 values, including NUL. Each is one
character irrespective of internal UTF-8 storage size. C2X('é') is E9;
C2X('漢') raises Classic SYNTAX 23.1. X2C('C3A9') is two ordinal characters.
SAY writes them as text; raw CHAROUT writes the two original bytes.

Named streams default to raw ordinals. Default console streams retain UTF-8.
STREAM OPEN ENCODING selects external codepages explicitly; CP1252 byte 80
maps to Euro even though it is above U+00FF. ENCODE returns byte ordinals and
DECODE consumes them. Source remains UTF-8. No codec adds a BOM or NFC.

See the [complete Classic contract](../../../compiler/docs/levelc_unicode_and_streams.md)
for the 23 Unicode BIFs, eight stream BIFs, commands, positions, line endings,
strict/replacement policy and whole-file conversion cost. The authoritative
LC-UNICODE worklist retains qualification; this does not claim complete Classic
or Release 1 conformance. RexxValue's binary caches remain available to direct
B/G clients and RexxScript without changing their existing policy.
