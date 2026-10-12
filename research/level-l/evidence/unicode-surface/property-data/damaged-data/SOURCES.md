# Pinned Level L Unicode 17.0.0 inputs

The Level B property compiler uses these exact raw Unicode Character Database
files, checked against `SHA256SUMS` before generation. DerivedCoreProperties and
UnicodeData are byte-identical to the project's existing Level G raw inputs;
the Level L copy keeps the standalone bootstrap self-contained. No generated
Level G module or runtime service is required.

- [DerivedCoreProperties.txt](https://www.unicode.org/Public/17.0.0/ucd/DerivedCoreProperties.txt)
- [UnicodeData.txt](https://www.unicode.org/Public/17.0.0/ucd/UnicodeData.txt)
- [PropList.txt](https://www.unicode.org/Public/17.0.0/ucd/PropList.txt)
- [Unicode terms of use](https://www.unicode.org/terms_of_use.html)

Property names are case-insensitive in Level L. Identifier recognition remains
a language-owned choice; property selection does not normalize source or change
identifier comparison. Updating the pinned version requires changing raw files,
checksums, generator audits, regression expectations and documentation together.
