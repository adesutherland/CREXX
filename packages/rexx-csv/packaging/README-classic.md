# rexx-csv 0.1.0 — Classic source package

One pure Rexx CSV parser/writer for Regina and ooRexx. MIT licensed.
Qualified on macOS ARM64, Ubuntu 24.04 x64 and Windows 2025 x64 with
Regina 3.9.7 and ooRexx 5.1.0 r12973. All 71 common fixtures, boundary/lifecycle
checks and fresh installed consumers passed. Older runtimes remain unqualified.
The [Linux/Windows run](https://github.com/adesutherland/CREXX/actions/runs/39443274327)
records the exact versions and source revision. There is no native,
ooRexx class, JSON or Python runtime dependency. NetRexx uses a separate binding.

Install [rexx-csv](https://www.rexxpository.org/cgi-bin/rxpip_server.rexx?action=PKG&name=rexx-csv)
with your runtime's client (Regina or ooRexx respectively):

```sh
regina -a /path/to/rxpip.rexx install rexx-csv==0.1.0
rxpip-oorexx install rexx-csv==0.1.0
```

Choose one command. Both select the identical `SOURCE/any` artifact. On Windows,
use the supplied `rxpip.bat` or `rxpip-oorexx.bat` launcher. Installation puts
all five files in the current directory; no build step is needed.
Keep `rexx_csv.rexx`
with the calling application and quote the lowercase external name as shown
below, so Regina finds it on case-sensitive filesystems. Run with that directory as the working
directory.

```sh
regina classic_demo.rexx
/path/to/oorexx/bin/rexx classic_demo.rexx
regina classic_demo.rexx '"input file.csv" "output file.csv"'
/path/to/oorexx/bin/rexx classic_demo.rexx '"input file.csv" "output file.csv"'
```

On Windows `cmd.exe`, the launchers transport that command string differently:

```bat
C:\Regina\regina.exe classic_demo.rexx "\"input file.csv\" \"output file.csv\""
C:\ooRexx\rexx.exe classic_demo.rexx "input file.csv" "output file.csv"
```

The Regina command supplies one argv value containing literal quotes; ooRexx
uses the raw command tail. These are `cmd.exe` commands; other shells may apply
different native argument escaping. Use ASCII application/input/output paths
on Windows, including installation directories. Spaces are supported.

Each runtime receives one command string; double quotes delimit the file paths
inside it. The example supports zero, one or two quoted paths, without literal
double quotes in a filename. With two paths it replaces the output after valid
UTF-8 decoding, CSV parsing and canonical writing succeed. It uses CHARIN and
CHAROUT to preserve embedded CR/LF bytes. Paths can contain spaces. Use ASCII
filenames on Windows: the tested native launchers do not preserve Ω filenames.
This is separate from Unicode CSV contents, which remain exact UTF-8. File open/write failures are caller
I/O errors; this small example does not promise atomic replacement on disk
failure. Parsing/writing are in memory and return atomic checked values.

```rexx
document = 'rexx_csv'('PARSE', 'name,city' || '0a'x || 'Ada,London')
if 'rexx_csv'('OK', document) = 0 then do
  say 'rexx_csv'('ERROR_CODE', document)
  exit 1
end
cell = 'rexx_csv'('FIELD', document, 2, 1)
if 'rexx_csv'('OK', cell) = 0 then exit 1
say 'rexx_csv'('TEXT', cell)
changed = 'rexx_csv'('WITH_FIELD', document, 2, 1, 'Grace')
written = 'rexx_csv'('WRITE', changed)
if 'rexx_csv'('OK', written) = 1 then say 'rexx_csv'('TEXT', written)
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
