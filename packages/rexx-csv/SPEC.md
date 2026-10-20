# rexx-csv — shared contract, version 0.1

**Shared contract approved for the cREXX increment on 20 October 2026.**
The cREXX and Classic interfaces are approved. The concrete NetRexx API
below was approved under D-05 on 20 October 2026. The
owning scope, decision gates and acceptance criteria are in `PLAN.md`. The
fixtures express this contract and change with any approved contract revision.

## Model and scope

The semantic operations are `parse(text)` and `write(rows)`. A table is an
ordered sequence of rows; a row is an ordered, nonempty sequence of strings.
Rows may have different widths. Every field is text: no inferred numbers,
dates, booleans or nulls. Headers are ordinary rows. Duplicate headings,
leading zeros, spaces and formula-looking strings retain their values.

Both operations return a success or failure result. Parsing succeeds with
rows; writing succeeds with text. A failure exposes a diagnostic and no partial
table or text. No operation opens a file, changes a queue, invokes a shell or
uses process-global state. A result remains valid after another call or after
the caller changes its original input. Any public row-building helpers must
have documented ownership and must not mutate an existing parse result.

## Format profile

[RFC 4180](https://www.rfc-editor.org/rfc/rfc4180.txt) is an Informational
description of CSV. This profile uses its comma separator, double-quoted
fields, doubled-quote escaping, significant spaces and CRLF output convention.
It defines the edge cases below explicitly. It also accepts LF record endings
and Unicode text, extending the RFC's original grammar. Do not advertise this
as a validator of the RFC's ASCII grammar.

Review the [RFC errata](https://www.rfc-editor.org/errata/rfc4180) alongside
the document. Verified editorial erratum 5714 removes misleading spaces after
a closing quote. Held erratum 6230 proposes a different final-newline rule;
it is not treated as an adopted standard here.

### Parsing

1. A comma separates fields. A quote can open a quoted field only at its
   beginning. Inside a quoted field, two consecutive quotes represent one
   quote. Commas and CR/LF there are literal data and retain their exact values.
2. After a closing quote, only comma, record separator or end of input is
   valid. Spaces and tabs outside the closing quote are errors. A quote in an
   unquoted field is also an error. Do not silently trim or repair input.
3. Between records, accept CRLF or LF, including a mixture. Reject a lone CR
   outside quotes. A final record separator is optional and does not create
   an additional row by itself.
4. Empty input is zero rows. A blank line is one row containing one empty
   string. A trailing comma adds an empty field. Thus `a,` is `[a, ""]`,
   one LF is `[[""]]`, and two LFs are `[[""], [""]]`.
5. Remove at most one leading U+FEFF at the absolute beginning of the input
   as a document BOM. A BOM inside a field, including inside an initial quoted
   field, is data. BOM-only input therefore has zero rows.
6. Preserve valid Unicode scalar values without normalization, case changes,
   trimming or newline conversion. Tabs and other control characters are
   permitted as data except U+0000, which is rejected wherever encountered.
7. Preserve ragged rows. Field-count validation and header interpretation are
   caller policies, not parse failures in this profile.

### Canonical writing

1. Emit comma-separated fields and a CRLF after every row, including the last.
   Zero rows produce empty output. A row with no fields is an error.
2. Quote an empty field, or a field containing comma, quote, CR or LF. Double
   every embedded quote. Other spaces and tabs remain unchanged and do not
   require quoting.
3. Also quote the first field of the first row if it starts with U+FEFF. This
   protects its value from the parser's document-BOM rule and preserves the
   round-trip property. Emit no document BOM.
4. Retain field text exactly. Do not escape spreadsheet formulas, coerce
   numbers, pad rows, normalize Unicode or choose the host OS newline.
5. Reject U+0000, zero-field rows and the resource-limit violations below.
   Return no partial text, even if earlier rows could have been written.

For every accepted table, `parse(write(table))` must recover identical cells.
Writing parsed input canonicalizes framing, not field values. In particular,
an empty document and a document with one empty field stay distinguishable.

## Encoding boundary

The common transport and test data use UTF-8. The logical contract is valid
Unicode scalar text. cREXX uses valid UTF-8 `.string` values; Classic bindings
use UTF-8 byte strings; NetRexx converts its host text explicitly for the common
UTF-8 boundary. No locale-dependent conversion is permitted.

Decoding arbitrary file bytes and choosing a legacy encoding are caller/adapter
responsibilities. Tests must read fixtures without automatic newline conversion
and must reject malformed UTF-8 at that boundary. The CSV core does not claim
to decode arbitrary binary input. Similarly, a NetRexx string with an unpaired
surrogate does not satisfy the valid-text precondition. The binding must
document and test its boundary behaviour before claiming support.

No byte, codepoint or UTF-16 offset appears in the common diagnostic interface;
those units differ across runtimes. Logical record/field coordinates are common.

## Failures, coordinates and limits

Diagnostics contain `code`, `record`, `field`, an optional `limit` name and a
human-readable message. Record and field numbers are one-based logical CSV
coordinates; a quoted newline does not begin a new record. Whole-input
preflight errors use record 0, field 0. English messages are not an exact test
oracle. Success has no diagnostic. Accessor misuse is separate from a CSV
syntax error and must have a documented binding-specific failure contract.

| Code | Meaning |
| --- | --- |
| `E_QUOTE` | Quote encountered inside an unquoted field. |
| `E_AFTER_QUOTE` | Unexpected character after a closing quote. |
| `E_UNCLOSED_QUOTE` | End of input while a quoted field is open. |
| `E_RECORD_SEPARATOR` | Lone CR outside a quoted field. |
| `E_NUL` | U+0000 in a field or input. |
| `E_EMPTY_ROW` | Writer supplied a row with no fields; field is 0. |
| `E_LIMIT` | A named resource limit would be exceeded. |

The approved fixed first-version limits keep the simple
in-memory interface bounded. They are not streaming or measured memory-usage
guarantees. Limits may become configurable in a later version; no mutable
process-wide settings are proposed.

D-08 was resolved on 20 October 2026: both input and output allow 16 MiB,
so every successful canonical write fits the parser input budget.

| Limit name | Inclusive maximum | Unit |
| --- | --- | --- |
| `input_bytes` | 16,777,216 | UTF-8 input bytes, including any document BOM |
| `output_bytes` | 16,777,216 | Canonical UTF-8 bytes including quotes/separators |
| `field_bytes` | 1,048,576 | Decoded field's UTF-8 bytes, excluding CSV framing |
| `records` | 10,000 | Logical rows |
| `fields_per_record` | 1,024 | Fields in one row |
| `total_fields` | 100,000 | Fields in the table |

Input-byte length is checked before parsing; exceeding it returns `E_LIMIT`
with `(0, 0)` even if later syntax is invalid. Afterwards, scan left to right.
Check record/field counts when entering a row/field and decoded field size as
data is accumulated. A separator after the last row does not enter a new row
unless input remains. Count a new field after a comma even at end of input.
Report the first error reached by this order; for a count limit, report the
row/field that would exceed it. At each character, diagnose NUL before applying
CSV-state rules or counting it as field data. At EOF an open quote is
`E_UNCLOSED_QUOTE`.

The writer visits rows and fields in order. It validates counts, nonempty row
shape and field text, then charges each field's complete encoded contribution
to `output_bytes`. Commas are charged with the following field; the final CRLF
is charged with the last field of that row. An output-limit diagnostic names
the field whose contribution would exceed it. There is no partial successful
result. Resource allocation failure outside these limits remains a host-runtime
failure; do not promise recovery from an exhausted process.

## Language bindings

The cREXX and Classic bindings below implement their approved interfaces.
The NetRexx D-05 interface was approved on 20 October 2026.

### cREXX binding

Source: `crexx/rexx_csv.crexx`; namespace: `rexx_csv`.
Export these factories/types and convenience procedures:

```text
.csvrow(expose values = .string[])           # immutable snapshot; empty allowed
.csvdocument(text = .string)                 # parse result
.csvoutput(expose rows = .csvrow[])           # write result
csvparse(text = .string) -> .csvdocument
csvwrite(expose rows = .csvrow[]) -> .csvoutput
```

The exposed array arguments are borrowed read-only during each call. Factories
retain independent values; caller mutation after return cannot change a result.
Writer input is a dense, one-based growable row array; each row's input is a
dense, one-based growable string array. An uninitialized row object is invalid
API usage and raises `INVALID_ARGUMENTS`; an initialized zero-field row returns
the shared writer diagnostic `E_EMPTY_ROW`.

| Type | Methods and return types |
| --- | --- |
| `csvrow` | `field_count() -> .int`, `field(column = .int) -> .string` |
| `csvdocument` | `ok() -> .int`, `row_count() -> .int`, `field_count(row = .int) -> .int`, `field(row = .int, column = .int) -> .string`, `rows() -> .csvrow[]` |
| `csvoutput` | `ok() -> .int`, `text() -> .string` |
| Both result types | `error_code() -> .string`, `error_record() -> .int`, `error_field() -> .int`, `error_limit() -> .string`, `error() -> .string` |

`ok()` returns 1 or 0. Success diagnostics are empty strings and zero
coordinates. Failed documents have zero rows and `rows()` returns an empty
array; failed writers return empty text. `field_count()` returns -1 for an
invalid row or failed document. `field()` raises `INVALID_ARGUMENTS` for an
invalid row/column or failed document, rather than returning an ambiguous empty
cell. Indices are one-based. `rows()` returns independently owned row values for
editing/replacement in a caller-owned writer array; rows themselves have no
mutators. There is no public binary serialization format.

Internally the parse result may keep flat cells and row boundaries, avoiding
repeated whole-table copying during indexed access. The parser scans exact
UTF-8 bytes, recognizes only ASCII framing and materializes field spans; it
does not reinterpret non-ASCII bytes as character positions. The writer uses
bounded binary accumulation and returns validated UTF-8 text. Neither has
mutable global state, assembler source or a native provider.

Read `docs/ai-context/CREXX_LEVELB_AUTHORING.md`, the Unicode/string contract
and current class examples before writing source. Select public core services
that the released baseline actually provides.

### Regina and ooRexx — approved D-04 interface

Use one external Classic Rexx file, `classic/rexx_csv.rexx`, called as
`'rexx_csv'(operation, arguments...)`. Quote the lowercase external name so
Regina finds `rexx_csv.rexx` on case-sensitive filesystems. This preserves the
function signature and operation semantics. Operation names are case-insensitive;
field values are never case-folded or trimmed. The implementation uses ordinary
Rexx strings, stems local to that external activation and shared built-ins.
It requires no ooRexx classes, native extension, JSON library or caller-stem
mutation. Adrian approved this interface on 20 October 2026; the implementation
and its shared-oracle checks run on both selected interpreters.

All row/table/document/field/write values are opaque owned strings. Their
indexed, length-framed representation is private to the package version and is
not a file format or public serialization ABI. A copied value remains valid
after nested/later calls and caller input mutation. No close/reset is needed.

| Call | Result |
| --- | --- |
| `'rexx_csv'('PARSE', text)` | Checked document value, with the shared CSV diagnostics. |
| `'rexx_csv'('ROW', field1, field2, ...)` | Owned row value; zero fields are representable, and fail only when written. |
| `'rexx_csv'('TABLE', rowOrTable1, rowOrTable2, ...)` | Owned concatenated table value; zero operands produce zero rows. Existing successful documents/tables may be operands. |
| `'rexx_csv'('WITH_FIELD', table, row, column, text)` | New table value with one existing field replaced; the original stays unchanged. |
| `'rexx_csv'('WRITE', table)` | Checked write value whose text follows the shared canonical rules. A successful parse document may be used directly. |
| `'rexx_csv'('OK', value)` | 1 for a valid successful value, otherwise 0. |
| `'rexx_csv'('ROW_COUNT', documentOrTable)` | Row count; zero for a valid failed parse, -1 for invalid API usage. A row value counts as one row. |
| `'rexx_csv'('FIELD_COUNT', documentOrTable, row)` | Field count, or -1 for a failed document/invalid row or value. |
| `'rexx_csv'('FIELD', documentOrTable, row, column)` | Checked field value; invalid coordinates return `E_ARGUMENT`, never an ambiguous empty field. |
| `'rexx_csv'('TEXT', fieldOrWriteValue)` | Exact field/output string; empty on failure or a value of the wrong kind. Check `OK` first. |
| `'rexx_csv'('ERROR_CODE', value)` | Shared CSV code, binding-only `E_ARGUMENT` for API misuse, or empty on success. |
| `'rexx_csv'('ERROR_RECORD', value)` / `ERROR_FIELD` | Shared logical coordinates, or zero for success/API misuse. |
| `'rexx_csv'('ERROR_LIMIT', value)` / `ERROR` | Limit name and readable diagnostic, respectively; empty on success. |

Constructors wrap field values without applying writer limits prematurely, so
an oversized field or zero-field row still receives its specified diagnostic
from `WRITE`. Invalid opaque values, unknown operations, malformed argument
lists or invalid accessor/replacement coordinates produce a checked
`E_ARGUMENT` result where a value is returned. Scalar queries use the explicit
sentinels above; diagnostic queries on an invalid value describe `E_ARGUMENT`.
These are binding-usage failures, not new CSV syntax errors. Ordinary runtime
allocation/stack exhaustion remains a host failure.

`FIELD` returns a checked value; `TEXT` extracts its string. This distinguishes
an invalid cell and a valid empty field without requiring an ooRexx exception
facility. All row/column coordinates remain one-based. For example:

```rexx
document = 'rexx_csv'('PARSE', text)
if 'rexx_csv'('OK', document) = 0 then do
  say 'rexx_csv'('ERROR_CODE', document)
  exit 1
end
cell = 'rexx_csv'('FIELD', document, 2, 1)
if 'rexx_csv'('OK', cell) = 1 then say 'rexx_csv'('TEXT', cell)
changed = 'rexx_csv'('WITH_FIELD', document, 2, 1, 'Ada')
written = 'rexx_csv'('WRITE', changed)
if 'rexx_csv'('OK', written) = 1 then say 'rexx_csv'('TEXT', written)
```

Parsing/writing retain all shared limits, BOM/newline rules and failure
coordinates. UTF-8 remains an explicit caller/adapter text boundary: Classic
byte-string BIFs do not themselves prove valid Unicode. The PARSE entry and
WRITE field boundary explicitly validate UTF-8 and return binding-only
`E_ARGUMENT` for malformed text. Input-byte preflight still precedes decoding.
ROW/TABLE may wrap raw field bytes; WRITE applies the format and size rules. Indexed value lookup
must use fixed index entries without explicit whole-table scans/copies.
Host interpreters may still copy value arguments at procedure/call boundaries;
this is not a constant-time runtime or resident-memory guarantee.
Constructors and parser/writer assembly
use bounded/balanced aggregation rather than growing one whole-table string
per appended cell. `TABLE` accepts existing tables to support balanced batched
construction; do not require a single 10,000-argument call to the interpreter.
`VERIFY`, `CHANGESTR` and `COUNTSTR` have been checked on both selected
runtimes. Internal field/index helpers expose only the external library
activation's private value, avoiding repeated whole-table argument copies in
the writer. There is no exposure of a caller's variables or global storage.
Parser spans, row/cell indexes and writer pieces are assembled with balanced
joins, releasing consumed chunks. The actual API's maintained contracts cover
100,000 indexed fields, a 16 MiB constructor payload, external scope, numeric
context, snapshots, nested calls and all shared limit recipes. These replace
the pre-implementation representation prototype.

The installed ooRexx `csvStream.cls` remains an existing stream API to document.
It is not a dependency or source input for this shared Classic implementation.

### NetRexx — approved D-05 interface

**Approved and implemented 20 October 2026; macOS qualification is in PLAN.md.** Use the
JVM package `rexxcsv`, with four public final classes: `Csv`, `CsvRow`,
`CsvDocument` and `CsvOutput`. Source belongs under `netrexx/rexxcsv/`.
`Csv` is a utility class with static factories. Row/result construction is
package-private; applications use the factories below. This keeps the public
surface close to the cREXX binding while fitting NetRexx's typed JVM methods.

Public text parameters/returns are `java.lang.String`; rows are typed
`CsvRow[]` arrays. There is no implicit number, header, locale or null-value
conversion. NetRexx callers can supply string literals or explicitly convert
their Rexx values to String. No public mutable properties are exposed.

The signatures below use `name(type) -> type` notation, not Java source.

| Class | Public factory/operation |
| --- | --- |
| `Csv` | `parse(text: String) -> CsvDocument` (static) |
| `Csv` | `row(fields: String[]) -> CsvRow` (static) |
| `Csv` | `write(rows: CsvRow[]) -> CsvOutput` (static) |
| `CsvRow` | `fieldCount() -> int`, `field(column: int) -> String` |
| `CsvRow` | `withField(column: int, text: String) -> CsvRow` |
| `CsvDocument` | `ok() -> boolean`, `rowCount() -> int` |
| `CsvDocument` | `fieldCount(row: int) -> int`, `field(row: int, column: int) -> String` |
| `CsvDocument` | `rows() -> CsvRow[]` |
| `CsvOutput` | `ok() -> boolean`, `text() -> String` |
| Both result types | `errorCode() -> String`, `errorRecord() -> int`, `errorField() -> int`, `errorLimit() -> String`, `error() -> String` |

No dispatcher strings, checked FIELD wrapper, varargs, generics or separate
public table/error class are required. Standard JVM exceptions can distinguish
an invalid accessor from a valid empty string. Private implementation helpers
remain free to share parsing, row storage and diagnostic construction.

#### Ownership, editing and indices

CSV accessor coordinates are **one-based**, as in the other bindings.
`String[]` and `CsvRow[]` are ordinary **zero-based JVM arrays**. Array length
is the count; there is no count stored in element zero.

`Csv.row` defensively copies its input array. Rows are immutable; `withField`
returns an edited copy and preserves the original. `CsvDocument.rows()`
returns a new array whose immutable row objects may be shared. Replacing an
array element cannot change the document. The writer does not retain a
caller's mutable array. Later calls and caller mutations cannot change a
completed parse/write result. Callers must not concurrently mutate an input
array while a factory/writer is reading it.

A zero-length row array writes an empty document. `Csv.row(String[0])` is
representable and receives `E_EMPTY_ROW` only from the writer. Constructors
and edits do not prematurely enforce the writer's field/count/output limits.
They may hold oversized or malformed-Unicode field strings so `write` can
return the binding's specified diagnostic. No close/reset or serialization
API is proposed.

For example, this NetRexx consumer changes one field:

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

Here `rows[0]` is CSV row 1, and `withField(2, ...)` changes CSV field 2.
The original document still contains `name,city`. To build a new table, fill
a `String[n]` array, call `Csv.row(fields)`, place rows in `CsvRow[n]` and
call `Csv.write(rows)`. This avoids interpreter argument-count constraints.

#### Checked results and API misuse

All shared CSV syntax/limit failures are checked document/output results with
the existing codes and logical coordinates. Success has `ok() = true`, empty
diagnostic strings and zero error coordinates. Failed documents expose zero
rows and an empty `rows()` array. Failed output has empty `text()`.

`fieldCount(row)` returns -1 for an invalid row or failed document. An invalid
`field(...)` or `withField(...)` coordinate throws
`java.lang.IllegalArgumentException`; it never returns an ambiguous empty cell.
Null input strings/arrays, null row/field elements and null replacement text
are API misuse and throw that same exception. Empty strings/arrays remain
valid according to the shared rules. No checked Java exception is added to
the library signatures for normal CSV failures.

An unpaired UTF-16 surrogate supplied to `parse`, or encountered in a field
visited by `write`, produces binding-only `E_ARGUMENT` at (0,0), empty limit
and no partial result. This is an invalid-text precondition failure, distinct
from shared CSV syntax errors and from null/coordinate misuse. Parser text
validation precedes CSV processing: malformed UTF-16 yields `E_ARGUMENT`
even if that invalid string is also large; it has no defined valid UTF-8 byte
length. For valid scalar text, the 16 MiB input preflight always precedes CSV
syntax/count/field checks. The writer checks row/field counts in the shared
order, validates each visited field's scalar text, then applies its shared
NUL/field-size/output rules. Messages remain non-normative.

#### Unicode, I/O and implementation boundary

The shared budgets count **UTF-8 bytes**, never Java `String.length()` UTF-16
units. Supplementary scalars, combining sequences and the document BOM retain
the shared behaviour. Count safely without integer overflow or a replacement
encoding; preserve every accepted scalar and embedded CR/LF exactly.

File I/O remains with the caller/example. Read bytes, explicitly decode UTF-8
using a decoder configured with `CodingErrorAction.REPORT`, then call `parse`.
Write validated output with explicit UTF-8 and no newline conversion. A file
decoder's `CharacterCodingException` is a caller encoding failure, not a CSV
syntax result. Do not use platform-default decoding or an API that silently
replaces malformed input. This matches the shared B005 boundary requirement.

Implement the parser and writer in NetRexx using owned arrays/row values and
bounded builders. Keep field lookup indexed; do not rescan or copy a document
on each accessor, or repeatedly concatenate a growing whole output string.
Standard JVM arrays, String/StringBuilder and charset utilities are allowed;
no external CSV parser, Java CSV implementation, native plugin or JSON runtime
dependency is introduced. The existing hand-authored fixtures remain the oracle.

The qualified development baseline is NetRexx 5.10-GA and Temurin JDK
26.0.1+8 on macOS ARM64. Fresh compiled consumers use class-file version 52;
that alone does not qualify an older JVM. The implementation uses Java 8-era
library APIs; actual tested compiler/JVM versions are declared in the plan.
Older JVMs and Linux/Windows remain unverified. Source compilation uses
`NetRexxF.jar`; the installed application check runs with only the compiled
application/package and `NetRexxR.jar`. No compiler JAR is a runtime requirement
for a precompiled consumer.

Binding references: the [NetRexx class/method tutorial](https://www.netrexx.org/Tutorial/nr_11.html),
the installed typed-array examples, and the [JDK charset encoder contract](https://docs.oracle.com/en/java/javase/17/docs/api/java.base/java/nio/charset/CharsetEncoder.html).
These support the JVM/language boundary; CSV semantics come from this shared
specification. The actual shared-oracle, boundary and fresh-install results
and D-05 approval are recorded in `PLAN.md`.

## Conformance and distribution

`tests/fixtures/cases.json` is the common oracle for literal parse/write cases.
`tests/fixtures/README.md` defines storage, transport and required boundary and
lifecycle cases. Runtime adapters are test tooling, not shipped dependencies.
Expected results come from the approved contract, not majority voting among
implementations or whichever existing parser accepts an input.

Plan one logical package, `rexx-csv`, with a common version and separate
source artifacts for cREXX, Classic and NetRexx. Proposed rexxpository labels
are `SOURCE/crexx`, `SOURCE/any` and `SOURCE/netrexx`, subject to actual selection
tests. Include Markdown documentation, license and runnable examples in each
artifact. Every upload must list its qualified bindings and operating systems
explicitly; unverified bindings/platforms remain planned. All package uploads
need separate authorization.
