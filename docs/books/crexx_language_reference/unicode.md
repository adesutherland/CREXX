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

The VM whitespace table and the Level C UTF8 profile are pinned to the same
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

Classic compatibility selects a character profile through the
`RexxBifCallContext` configuration object. The profile is call/runtime state;
it is not selected by flags on an individual `RexxValue`.

Two profiles are defined:

| Profile | Meaning |
| --- | --- |
| `BYTE` | Default Classic compatibility profile. Character units are exact bytes. Arbitrary binary values remain valid inputs, PAD means one byte, positions are byte positions, and the configured range is `00` through `FF`. |
| `UTF8` | Opt-in text profile. Inputs used as text must be valid UTF-8; character positions are Unicode codepoints, PAD means one codepoint, and default blanks use Unicode 17.0.0 `White_Space`. |

Both profiles can add configured blank characters. BYTE additions are bytes;
UTF8 additions are codepoints. With no additions, the UTF8 word scanner uses
the VM fast path.

`RexxValue` text/binary flags only describe which representations are current
and whether held bytes are valid UTF-8. They never change the active profile.
There is no implicit fallback from UTF8 to BYTE and no implicit normalization.

The direct Level C conversion BIFs preserve exact encoded bytes. C2X and C2D
read those bytes; X2C and D2C produce them. In the UTF8 profile, a produced
value is marked as text only when its exact bytes are canonical valid UTF-8;
otherwise it remains binary-authoritative. BYTE XRANGE provides the inclusive,
wrapping 256-byte configured range. UTF8 XRANGE is intentionally unavailable
because it is not a Unicode range API; Level B `sequence` is the non-wrapping
Unicode-codepoint operation.
