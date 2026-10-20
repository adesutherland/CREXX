# rexx-csv 0.1.0 — cREXX source package

This is an unpublished first cREXX implementation. It requires the complete
cREXX Beta 3 core or a compatible later toolchain. No extra plugin, Python or
JSON library is needed by the installed source.

In the extracted package directory:

```sh
crexx -program csv-demo crexx_demo.crexx rexx_csv.crexx
rxvm csv-demo.rxbin
rxvm csv-demo.rxbin -a "input file.csv" "output file.csv"
```

The first command builds the example; the second runs its built-in data.
The third reads a UTF-8 file and writes canonical CSV. Binary file I/O and an
explicit UTF-8 conversion preserve embedded line endings. A malformed UTF-8
file raises `UNICODE_ERROR` at that conversion boundary.

To use the library, add `import rexx_csv` and include `rexx_csv.crexx` in your
project build. `csvparse(text)` returns an immutable `.csvdocument`;
`csvwrite(rows)` returns an immutable `.csvoutput`. Check `ok()` first.
Diagnostics have `error_code()`, `error_record()`, `error_field()`,
`error_limit()` and `error()`. Every public row/column coordinate is one-based.

`document.rows()` returns an owned `.csvrow[]`. Construct a replacement row
with `.csvrow(values)`, where `values` is a dense one-based `.string[]`.
Replacing a caller-owned row does not change the parsed document or earlier
writer output. Rows have `field_count()` and `field(column)` methods and no
mutators. Empty field text is preserved; a row with zero fields cannot be written.

CSV values remain strings. Headers are ordinary rows; differing row widths
are allowed. Spaces, leading zeros, formula-like text and Unicode are not
transformed. Quoted commas, doubled quotes and embedded newlines are supported.
The parser accepts LF and CRLF records; output always uses CRLF. No spreadsheet
formula transformation is performed.

See [the full contract](rexx-csv-SPEC.md) for the exact format, BOM handling,
recoverable errors, resource limits and invalid-index signals. The library
handles text only; callers own files and encoding selection.
The [MIT license](rexx-csv-LICENSE.txt) covers this package's original source
and documentation. Separate Regina/ooRexx and NetRexx source bindings are also qualified on
macOS; this archive contains the cREXX binding.

On Windows Beta 3, use ASCII command-line filenames (spaces are supported).
Its native CLI does not preserve Ω filenames; this is separate from Unicode
CSV contents, which remain exact UTF-8.
