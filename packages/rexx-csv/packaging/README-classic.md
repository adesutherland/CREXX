# rexx-csv 0.1.0 — Classic source package

One pure Rexx CSV parser/writer for Regina and ooRexx. MIT licensed.
Qualified on macOS ARM64 with Regina 3.9.7 and ooRexx 5.1.0 r12973.
Older runtimes, Linux and Windows remain unqualified. There is no native,
ooRexx class, JSON or Python runtime dependency. NetRexx uses a separate binding.

Extract all five files into an application directory. Keep `rexx_csv.rexx`
with the calling application and run with that directory as the working
directory. No build step is needed. This archive is unpublished; live
rexxpository selection/metadata remains to be verified before publication.

```sh
regina classic_demo.rexx
/path/to/oorexx/bin/rexx classic_demo.rexx
regina classic_demo.rexx '"input file.csv" "output file.csv"'
/path/to/oorexx/bin/rexx classic_demo.rexx '"input file.csv" "output file.csv"'
```

Each runtime receives one command string; double quotes delimit the file paths
inside it. The example supports zero, one or two quoted paths, without literal
double quotes in a filename. With two paths it replaces the output after valid
UTF-8 decoding, CSV parsing and canonical writing succeed. It uses CHARIN and
CHAROUT to preserve embedded CR/LF bytes. File open/write failures are caller
I/O errors; this small example does not promise atomic replacement on disk
failure. Parsing/writing are in memory and return atomic checked values.

```rexx
document = rexx_csv('PARSE', 'name,city' || '0a'x || 'Ada,London')
if rexx_csv('OK', document) = 0 then do
  say rexx_csv('ERROR_CODE', document)
  exit 1
end
cell = rexx_csv('FIELD', document, 2, 1)
if rexx_csv('OK', cell) = 0 then exit 1
say rexx_csv('TEXT', cell)
changed = rexx_csv('WITH_FIELD', document, 2, 1, 'Grace')
written = rexx_csv('WRITE', changed)
if rexx_csv('OK', written) = 1 then say rexx_csv('TEXT', written)
```

`ROW` builds a row from string arguments, `TABLE` combines rows or tables.
For large construction use batches/balanced TABLE merging; avoid appending
one row to an ever-growing table or relying on enormous source argument lists.
Opaque values own their data; retain them as strings and access through the API.
Their binary representation is private and must not be edited or persisted.
Check `OK` before `TEXT`, since an empty field is valid. Indices are one-based.
An invalid FIELD/WITH_FIELD or malformed UTF-8 returns binding `E_ARGUMENT`.
`ROW_COUNT`/`FIELD_COUNT` use the sentinels defined in the specification.

The profile preserves spaces, empty fields, leading zeros and Unicode;
accepts LF/CRLF; writes canonical CRLF with quoting; and keeps headers as data.
It removes at most one document BOM, rejects NUL and invalid CSV, and permits
ragged rows. Limits: 16 MiB input/output, 1 MiB decoded field, 10,000 records,
1,024 fields/record, 100,000 total fields. Errors expose code, record, field,
limit and readable message, without partial rows/output. No global reset or
close is required. A successful write can always be parsed back under these
limits. See `rexx-csv-SPEC.md` for the complete API, precedence and byte rules.
