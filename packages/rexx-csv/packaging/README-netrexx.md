# rexx-csv 0.1.0 — NetRexx source package

A pure NetRexx CSV parser/writer. MIT licensed. Qualified on macOS ARM64 with
NetRexx 5.10-GA (build 18-20260320-1410) and Temurin JDK 26.0.1+8.
Other JVM versions and Linux/Windows remain unqualified. Class-file target 52
alone does not prove Java 8 compatibility. No native plugin, third-party CSV
parser, JSON or Python runtime dependency is needed.

Extract the archive, preserving the `rexxcsv/` directory. From the extracted
root, compile the four library sources and the example using your NetRexx SDK:

```sh
CSV_NETREXX_HOME="/path/to/netrexx"
java -cp "$CSV_NETREXX_HOME/lib/NetRexxF.jar" org.netrexx.process.NetRexxC rexxcsv/Csv.nrx rexxcsv/CsvRow.nrx rexxcsv/CsvDocument.nrx rexxcsv/CsvOutput.nrx NetRexxDemo.nrx -ecj -utf8 -sourcedir -nologo -verbose0
java -cp ".:$CSV_NETREXX_HOME/runlib/NetRexxR.jar" NetRexxDemo
java -cp ".:$CSV_NETREXX_HOME/runlib/NetRexxR.jar" NetRexxDemo "input file.csv" "output file.csv"
```

These commands are for the qualified macOS environment. The bundled compiler
is selected by `-ecj`; `-utf8` preserves source literals and `-sourcedir` puts
classes beside their sources in the package directory. Keep the compiled
`rexxcsv/` directory on the application classpath. After compilation only your
classes and `NetRexxR.jar` are on the example's dependency classpath; the compiler
JAR, SDK example classes and checkout are absent. A source install needs a JDK
and the NetRexx compiler; a precompiled consumer needs its declared JVM/runtime.

The file example accepts zero, one or two arguments. With two it replaces the
output after successful strict UTF-8 decoding, CSV parsing and canonical writing.
It reads/writes bytes explicitly, preserving embedded CR/LF and Unicode. Invalid
encoding/CSV leaves existing output untouched. File-write failures have ordinary
caller I/O semantics; the example does not promise atomic replacement on disk
failure. No paths or SDK/runtime binaries are bundled in the archive.

For application code:

```netrexx
options binary
import rexxcsv.Csv

document = Csv.parse('name,city')
if \document.ok() then do
  say document.errorCode() document.errorRecord() document.errorField()
  System.exit(1)
end
rows = document.rows()
rows[0] = rows[0].withField(2, 'town')
output = Csv.write(rows)
if output.ok() then say output.text()
```

The result is `name,town` followed by CRLF. The original document stays unchanged.
CSV accessor coordinates are **one-based**; JVM arrays are **zero-based**.
`rows[0]` is the first row, while `field(1,2)` means row 1, field 2.

Use `Csv.row(fields)` to construct an immutable row from a `String[]` array,
and `Csv.write(rows)` with a `CsvRow[]` array. Inputs and `document.rows()`
snapshots have defensive array boundaries. Row `withField` returns an edited
copy. Strings and row objects are immutable; replacing caller array entries
cannot change completed results. Do not mutate an input array during a call.
A zero-field row can be constructed, but writing it returns `E_EMPTY_ROW`.
A zero-row table writes an empty document.

Documents expose `ok`, `rowCount`, `fieldCount`, `field` and `rows`; output
exposes `ok` and `text`. Both expose `errorCode`, `errorRecord`, `errorField`,
`errorLimit` and `error`. Check `ok()` first. A failed parse has zero rows;
failed output has empty text. `fieldCount` returns -1 for an invalid row;
invalid `field`/`withField` coordinates and null arguments/elements throw
`IllegalArgumentException`. Unpaired UTF-16 in parser/writer text returns
binding `E_ARGUMENT` at (0,0). Strict file decoder failures are caller encoding
errors, handled by the example before parsing.

The shared profile preserves spaces, empty fields, leading zeros and Unicode;
accepts LF/CRLF; writes CRLF with deterministic quoting; treats headers as data;
and permits ragged rows. It removes at most one leading document BOM and
rejects NUL. Limits count UTF-8 bytes: 16 MiB input/output, 1 MiB decoded field,
10,000 records, 1,024 fields/record, 100,000 total fields. UTF-16 length does
not determine these limits. Failures expose no partial rows/text. Every valid
canonical output can be parsed back under the same limits.

See `rexx-csv-SPEC.md` for the approved complete API, Unicode and error precedence.
This local source archive is unpublished. Live rexxpository selection/metadata
and upload require their separate checks/authorization.

On the tested Windows runner, use ASCII command-line filenames (spaces work).
The Java launcher loses Ω in argv on its native code page; this is separate
from exact Unicode CSV contents. Use the relative `.` classpath from the
installed directory, with `;` as the Windows classpath separator.
