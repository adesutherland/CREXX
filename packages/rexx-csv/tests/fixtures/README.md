# Shared rexx-csv fixture oracle

Owner: the rexx-csv package. `cases.json` contains hand-authored input and
expected output for the shared `../../SPEC.md` contract. These are maintained
test inputs, not captured runtime output. The profile is approved for the first
cREXX increment, with 16 MiB input/output alignment approved in the plan.
Adapters are `../check_crexx.py` and `../check_classic.py` (the latter runs
separately on Regina and ooRexx), and `../check_netrexx.py`.

## Storage and transport

- The manifest is UTF-8 JSON with `schema_version`, `profile` and `cases`.
- Each case has a stable ID, a description, an `operation` (`parse` or `write`),
  its `input` text or `rows`, and an `expected` result.
- Success has `ok: true` and exactly the expected rows or output text. Failure
  has `ok: false` and `error` with code, record, field and optional limit.
  Failure also requires no exposed partial data; messages are not compared.
- JSON escapes represent the actual input characters. In particular `\r`,
  `\n`, `\u0000` and `\ufeff` are not literal backslash sequences in CSV.
  Preserve decomposed accents and non-BMP characters without normalization.
- Read the manifest as bytes and decode UTF-8 explicitly. Test adapters must
  preserve CRLF, LF and lone CR exactly. Do not pass multiline fixture input
  through shell command substitution or line-oriented text conversion.
- JSON is test storage, not a production runtime dependency. An orchestrator
  may translate fixtures to language-specific test input in a disposable
  build directory. It must not generate its expected rows using a CSV parser.
- Every delivered runtime executes every case. Separate adapters and version
  checks must distinguish Regina from ooRexx even when both executables are
  named `rexx`. Fixtures never acquire runtime-specific expected results.

Parser fixtures also require canonical write/read round trips for their
successful tables. Writer fixtures require parsing their expected output back
to their input rows. The original textual framing need not be preserved.
In particular, a leading U+FEFF field value must be quoted by the writer.

## Required boundary recipes

Build large values in the test adapter from these recipes rather than storing
megabytes of repeated text in Git. Use the shared constants in
`SPEC.md`, and give the resulting tests these stable IDs. Run both the accepted
boundary and the first rejected value. Check Unicode byte sizes independently
of host character indexing. These recipes are implemented by
`../../crexx/tests/contracts.crexx` and `../../classic/tests/contracts.rexx`.
NetRexx recipes are in `../../netrexx/tests/CsvContracts.nrx`. The Classic
adapter also generates the bounded variadic-constructor cases.
D-08 aligns input and output at 16 MiB; both largest-output round trips are tested.

| ID | Input or writer table | Required result |
| --- | --- | --- |
| L001 | Parse `a` repeated 1,048,576 times; then 1,048,577 times. | First succeeds with one unchanged field; second is `E_LIMIT/field_bytes` at (1,1). |
| L002 | Parse 524,288 U+00E9 characters; then append one ASCII `a`. | Same byte boundary and diagnostic as L001; a character count must not substitute for UTF-8 bytes. |
| L003 | Parse `a\n` repeated 10,000 times; then 10,001. | First has 10,000 rows; second is `E_LIMIT/records` at (10001,1). No extra EOF row. |
| L004 | Parse 1,023 commas; then 1,024 commas. | First has 1,024 empty fields; second is `E_LIMIT/fields_per_record` at (1,1025). |
| L005 | Parse 1,000 repetitions of a record consisting of 99 commas and LF; then append LF. | First has 100,000 empty cells; second is `E_LIMIT/total_fields` at (1001,1). |
| L006 | Parse sixteen quoted fields of 1,048,573 ASCII `a` characters, comma-separated with final LF. This is exactly 16,777,216 UTF-8 bytes. Then change final LF to CRLF. | First succeeds (one row, sixteen fields); second fails preflight `E_LIMIT/input_bytes` at (0,0). |
| L007 | Repeat L001–L005 using writer table values with the same decoded sizes/counts. | Same inclusive boundaries and count coordinates. Counts are checked before output generation exceeds its own bound. |
| L008 | Write one row with 15 fields of 1,048,576 ASCII `a` characters and a last field of 1,048,559 characters. Then append one character to that last field. | First canonical output is exactly 16,777,216 bytes; second is `E_LIMIT/output_bytes` at (1,16). |
| L009 | Repeat representative field/output limits with quotes and embedded CRLF. | Count decoded field bytes separately from doubled quotes and framing bytes. Check exact boundary and first excess. |
| L010 | Oversized input whose first character would otherwise cause a syntax error. | `input_bytes` preflight wins, at (0,0); no partial result. |

The accepted L008 and L009 canonical outputs must also parse back successfully
and recover every original field, including the complete first and last fields.
This checks the common round-trip guarantee at the aligned input/output limit.

Recipe L006 derives from `16 * (1,048,573 + 2) + 15 + 1` bytes.
Recipe L008 derives from `15 * 1,048,576 + 1,048,559 + 15 + 2` bytes.
These equalities are part of fixture-data validation, not performance claims.

## Required binding and lifecycle tests

- B001: Parse success, parse failure, parse success on the same loaded library;
  retain and re-read the first result after both later calls.
- B002: Change the caller's original parse text and writer input after a call;
  retained result values/text do not change. No partial rows/text are exposed
  by a failure. Invalid accessors fail as the approved binding specifies.
- B003: Repeated independent parse/write results need no global reset, file,
  queue or mutable process configuration. Nested calls retain both results.
- B004: Read/write fixture transport from a path containing spaces; do not
  translate embedded newlines or use the host default encoding.
- B005: Malformed UTF-8 and an unpaired JVM surrogate are rejected at the
  documented boundary. Do not count an invalid-byte decoder as a CSV test.
- B006: Run a real application consumer using only the installed package and
  declared runtime/core libraries. Test orchestration and JSON are absent.

The cREXX, Classic and NetRexx adapters/contracts and clean-package consumers
cover the applicable requirements on their qualified macOS versions, including
the JVM surrogate checks. Other operating systems and untested JVM versions
remain open.
Manifest linting alone does not satisfy these requirements. Independent library or
standard-library comparisons may help review overlapping valid cases; they
cannot replace this oracle or settle differing blank-line/error policies.
