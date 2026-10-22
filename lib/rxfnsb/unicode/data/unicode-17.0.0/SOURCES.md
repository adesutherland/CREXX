# Unicode 17.0.0 sources

These pinned Unicode Character Database 17.0.0 files feed the production
`rxunicode` normalization, default case-mapping, case-fold, default
extended-grapheme implementations, and independent Level L lexical properties.

- Upstream: <https://www.unicode.org/Public/17.0.0/ucd/UnicodeData.txt>
- Upstream: <https://www.unicode.org/Public/17.0.0/ucd/DerivedNormalizationProps.txt>
- Upstream: <https://www.unicode.org/Public/17.0.0/ucd/NormalizationTest.txt>
- Upstream: <https://www.unicode.org/Public/17.0.0/ucd/SpecialCasing.txt>
- Upstream: <https://www.unicode.org/Public/17.0.0/ucd/CaseFolding.txt>
- Upstream: <https://www.unicode.org/Public/17.0.0/ucd/auxiliary/GraphemeBreakProperty.txt>
- Upstream: <https://www.unicode.org/Public/17.0.0/ucd/auxiliary/GraphemeBreakTest.txt>
- Upstream: <https://www.unicode.org/Public/17.0.0/ucd/emoji/emoji-data.txt>
- Upstream: <https://www.unicode.org/Public/17.0.0/ucd/DerivedCoreProperties.txt>
- Upstream: <https://www.unicode.org/Public/17.0.0/ucd/PropList.txt>
- Unicode terms of use: <https://www.unicode.org/terms_of_use.html>
- Checksums: [`SHA256SUMS`](SHA256SUMS)

The build verifies these checksums before running the deterministic Level B
table compilers. Updating the Unicode version requires updating the sources,
checksums, prepared-image audits, conformance tests, and public documentation
in one change.

Level L generates its own small recognition index directly from these pinned
inputs using the minimal B library. It does not import the optional Unicode
runtime image or the G facade.
