# Unicode 17.0.0 case-fold source

`CaseFolding.txt` is the pinned Unicode Character Database 17.0.0 source for
the production `rxunicode` case-fold implementation.

- Upstream: <https://www.unicode.org/Public/17.0.0/ucd/CaseFolding.txt>
- Unicode terms of use: <https://www.unicode.org/terms_of_use.html>
- SHA-256: `ff8d8fefbf123574205085d6714c36149eb946d717a0c585c27f0f4ef58c4183`

The build verifies this checksum before running the deterministic Level B
table compiler. Updating the Unicode version requires updating the source,
checksum, prepared-image audits, conformance tests, and public documentation in
one change.
