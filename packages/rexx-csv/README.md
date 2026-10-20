# rexx-csv

A CSV source library with a shared format contract and fixtures for cREXX,
Regina, ooRexx and NetRexx. All four runtimes are qualified on macOS with
the tested versions recorded in the plan. This package has not been
uploaded to rexxpository.

The parser and writer operate on text and string fields. They preserve empty
fields, spaces, leading zeros, quoted newlines and Unicode. Parsing accepts
LF or CRLF records; writing uses CRLF. Headers are ordinary rows, and row
widths may differ. Invalid CSV returns an explicit result with an error code
and logical record/field coordinates. There is no native plugin dependency.

See [SPEC.md](SPEC.md) for the complete format, limits and binding APIs, and
[PLAN.md](PLAN.md) for acceptance status and the remaining runtime/platform work.
File I/O and byte decoding are caller responsibilities; the example shows an
explicit UTF-8 boundary without newline translation.

## cREXX usage

Requires the complete cREXX Beta 3 core or a compatible later toolchain.
The package uses Level B and `rxfnsb`, without a Level G or JSON dependency.

```rexx
options levelb
import rxfnsb
import rexx_csv

document = csvparse('name,city' || '0a'x || 'Ada,London')
if document.ok() = 0 then do
  say document.error_code() document.error_record() document.error_field()
  exit 1
end
say document.field(2, 1)

rows = document.rows()
values = .string[]
values[1] = "Ada"
values[2] = "Montréal"
rows[2] = .csvrow(values)
output = csvwrite(rows)
if output.ok() = 0 then exit 1
call writebinary "people.csv", output.text() as .binary
```

Rows are immutable snapshots. To change a row, construct and replace it in
the caller-owned writer array. Parsed results remain unchanged. Indices are
one-based. An invalid `field()` index raises `INVALID_ARGUMENTS`;
`field_count()` returns -1 for an invalid document row. Check `ok()` before
reading a result. See the specification for atomic errors and resource limits.

From this directory, build and run the repository example:

```sh
crexx -program csv-demo examples/crexx_demo.crexx crexx/rexx_csv.crexx
rxvm csv-demo.rxbin
rxvm csv-demo.rxbin -a "input file.csv" "output file.csv"
```

`crexx -program` builds the program; `rxvm` launches it. An input file must
contain valid UTF-8. The example reads/writes binary files and explicitly
converts between UTF-8 bytes and text so embedded newline sequences survive.
It writes canonical CSV only when an output path is supplied. On Windows
Beta 3, use ASCII command-line filenames; spaces and Unicode CSV contents
are supported, but arbitrary Unicode CLI filenames are not.

## Regina and ooRexx usage

One file, `classic/rexx_csv.rexx`, runs on both interpreters. Qualified versions
are Regina 3.9.7 and ooRexx 5.1.0 r12973 on macOS ARM64; older versions and
Linux/Windows qualification is recorded by the hosted matrix described below. There are no native or ooRexx
class dependencies. Put the file with your application and run from that
directory so the external function can be found. Quote the lowercase external
name as shown: Regina otherwise searches for uppercase REXX_CSV on Linux.

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

`ROW` builds a row from string arguments; `TABLE` combines rows/tables.
Values are owned, opaque strings. `WITH_FIELD` returns a new table, leaving
the original unchanged. `FIELD` returns a checked value; `TEXT` extracts it.
Check `OK` so a valid empty cell is distinguishable from an error.
`FIELD_COUNT` returns -1 for invalid access; `FIELD` returns `E_ARGUMENT`.
Operation names are case-insensitive; data is preserved. The specification
lists every operation, diagnostic and sentinel.

For large tables, combine batches with balanced TABLE merging rather than
repeatedly appending to a growing value. Indexed access reads fixed index
entries; host interpreters may still copy value arguments at call boundaries.
The API makes no constant-time runtime or resident-memory guarantee.

Copy `examples/classic_demo.rexx` beside `classic/rexx_csv.rexx`, then from
that directory run either interpreter (use the actual ooRexx executable):

```sh
regina classic_demo.rexx
/path/to/oorexx/bin/rexx classic_demo.rexx
regina classic_demo.rexx '"input file.csv" "output file.csv"'
/path/to/oorexx/bin/rexx classic_demo.rexx '"input file.csv" "output file.csv"'
```

Classic command-line scripts receive one command string. This example accepts
zero, one or two double-quoted paths inside that string; filenames containing
literal double quotes are outside the example's argument convention.
It reads/writes bytes with CHARIN/CHAROUT and checks UTF-8 in the library.
Paths can contain spaces. On macOS/Linux the examples also qualify Unicode
filenames. The tested Windows cREXX Beta 3, Regina and ooRexx launchers use a
legacy command-line filename boundary: use ASCII paths there. Unicode CSV
**content** remains fully tested and supported; NetRexx also tests Unicode
filenames on Windows. CSV/encoding failure leaves an
existing output untouched; file-write failure has ordinary caller I/O semantics.

## NetRexx usage

The `rexxcsv` package provides `Csv`, `CsvRow`, `CsvDocument` and `CsvOutput`.
Qualified with NetRexx 5.10-GA and Temurin JDK 26.0.1+8 on macOS ARM64.
Other JVM versions remain unqualified; Linux/Windows results are in the hosted
matrix below. Target class-file
version 52 alone does not establish an older-JVM pass.

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

CSV accessor coordinates are one-based; JVM arrays are zero-based. The example
changes CSV row 1, field 2, preserving the original document. `Csv.row(fields)`
constructs an immutable row from a String array; `Csv.write(rows)` accepts a
CsvRow array. Factories and `rows()` snapshots have defensive array boundaries.
Documents/outputs expose checked errors, and failed results contain no partial
data. Nulls and invalid field/edit coordinates throw IllegalArgumentException;
malformed UTF-16 parser/writer text returns binding E_ARGUMENT. Limits count
UTF-8 bytes, including supplementary and combining text.

Build/extract the NetRexx source artifact below, then follow its
[compile/run instructions](packaging/README-netrexx.md). The standalone
`NetRexxDemo.nrx` uses strict UTF-8 decoding and binary file I/O, with spaced
and Unicode paths. A compiled application uses its classes and NetRexxR.jar;
the compiler JAR is only needed for source compilation. The specification has
the [complete approved API](SPEC.md#netrexx--approved-d-05-interface).

## Common tests and runtime checks

The hand-authored oracle is [tests/fixtures/cases.json](tests/fixtures/cases.json).
It is independent of every implementation. The fixture README also defines
large boundary recipes and result-lifecycle requirements. No expected result
is generated by parsing CSV with another implementation.

Use an explicit complete SDK `bin` directory:

```sh
python3 tests/check_crexx.py --bin /path/to/crexx/bin
python3 tests/check_package.py --bin /path/to/crexx/bin
```

The adapter generates exact UTF-8 fixture input in a disposable directory,
compiles, assembles, links and executes real consumers. It tests optimized and
unoptimized code on `rxtvm` and `rxbvm`, plus resource boundaries and ownership.
Python and JSON are development tooling, not installed runtime requirements.
Use `--modes opt` or `--vms rxbvm` for a focused replay. The plan records
observed qualification separately from this description of the test command.
The package check extracts an unpublished archive into a fresh directory, then
exercises the documented driver command, exact file I/O and failure recovery.

Run the same 71 fixtures and shared boundary/lifecycle recipes on each actual
Classic runtime separately:

```sh
python3 tests/check_classic.py --runtime /path/to/regina
python3 tests/check_classic.py --runtime /path/to/oorexx/bin/rexx
python3 tests/check_classic_package.py --runtime /path/to/regina
python3 tests/check_classic_package.py --runtime /path/to/oorexx/bin/rexx
```

The runner identifies the interpreter through PARSE VERSION, transports fixture
strings as exact UTF-8 hex literals and uses the shared JSON oracle without
runtime-specific expectations. `--common-only` runs just the literal fixtures.
Full checks include every row endpoint and all 100,000 cells through indexed
writing/canonical-byte comparison, accepted/first-excess limits,
16 MiB round trips, immutable edits, nested calls, numeric context, malformed
UTF-8 and invalid API arguments. Python/JSON are absent from the installed
Classic consumer; no compiler is needed.

For NetRexx, select the SDK and optionally an explicit JDK home:

```sh
python3 tests/check_netrexx.py --sdk /path/to/netrexx --java-home /path/to/jdk
python3 tests/check_netrexx_package.py --sdk /path/to/netrexx --java-home /path/to/jdk
```

The adapter compiles actual NetRexx consumers from the same 71 fixtures, then
runs them with only NetRexxR.jar as a runtime dependency. Full checks add shared
limits/lifecycle contracts, all 100,000 indexed cells, UTF-8 byte boundaries for
Latin/supplementary/decomposed text, strict caller decoding and JVM surrogate
rejection. `--common-only` focuses on the literal oracle. The package check
builds a deterministic source archive, compiles it after fresh extraction,
runs the documented usage/example and verifies exact file bytes/failure recovery.
Python and JSON are development tools and are absent from the installed consumer.

## Linux and Windows qualification

[CSV qualification](https://github.com/adesutherland/CREXX/actions/workflows/csv-qualification.yml)
runs eight independent x64 jobs: Ubuntu 24.04 and Windows 2025 for cREXX,
Regina, ooRexx and NetRexx. Each job runs the full checks above, including a
fresh installed consumer with spaced paths and exact Unicode CSV bytes.
Separate Windows filename probes report the native launcher limitations above. A green job qualifies its
exact commit and runtime; the workflow definition alone establishes no pass.

`tests/hosted-runtimes.json` pins official downloads and SHA-256 checksums:
cREXX Beta 3, Regina 3.9.7, ooRexx 5.1.0 r12973 and NetRexx 5.10-GA.
NetRexx jobs use Temurin JDK 26.0.1+8. Windows cREXX uses the MSVC SDK's
`rxbvm`; Linux exercises both `rxtvm` and `rxbvm`, optimized and unoptimized.
Python 3.12 is test tooling. Runtime assets and consumers are disposable;
nothing is uploaded to rexxpository by the workflow.

Pushes affecting this package on hotfix2/develop/master and relevant pull
requests start qualification; manual dispatch is also declared. For a
focused local replay on a disposable Linux/Windows x64 CI machine:

```sh
python3 tests/qualify_hosted.py --runtime regina
```

Choose `crexx`, `regina`, `oorexx` or `netrexx`. The bootstrap installs official
Debian packages with sudo on Linux Classic jobs; it is intended for disposable
CI machines. For NetRexx set `JAVA_HOME` to the selected JDK. Ordinary developer
checks should use explicit existing runtime paths as shown above.

## Local source artifacts

```sh
python3 packaging/build_crexx.py /tmp/rexx-csv-0.1.0-crexx.rxpkg
python3 packaging/build_classic.py /tmp/rexx-csv-0.1.0-classic.rxpkg
python3 packaging/build_netrexx.py /tmp/rexx-csv-0.1.0-netrexx.rxpkg
```

Each deterministic gzip/tar artifact contains source, an example, installation
instructions, specification and MIT license. None bundles a runtime or
implies an uploaded/registered rexxpository package. Public metadata and the
remaining platform/JVM qualification are tracked in the plan.
