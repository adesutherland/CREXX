# Report writer example plan

## Vision and intended outcomes

Provide a runnable `report_writer` example demonstrating a Level L report
definition grammar and a Level B reader. A `.build` plan will compile the
grammar and reader, then run the reader against a delimiter-separated employee
data file. The reader will parse report statements into a report specification,
then stream input records through that specification, render fixed-width
output, apply the supported numeric format mask, and write either to the
console or to the configured output file.

## Acceptance criteria

- AC-01: The report grammar compiles and the reader imports its generated
  `reportgrammar` namespace successfully.
- AC-02: The sample `.build` plan compiles the grammar and reader and runs the
  report reader against the included data file.
- AC-03: The reader extracts input, delimiter, columns, breaks, totals, and
  output settings from the parse tree without child-name or child-position
  mismatches.
- AC-04: The reader renders fixed-width records, applies the supported
  `9,999.99`-style numeric format, and emits break totals.
- AC-05: `OUTPUT` writes the rendered report to the requested text file; an
  empty output setting continues to write to the console.
- AC-06: Documentation in the example explains how to start the sample and
  identifies the generated artifacts.
- AC-07: A larger fixture contains about 100 input records, and an `.rxpp`
  sample embeds its report definition with the report-specific `##DALIAS`
  block form that preprocesses and runs successfully.
- AC-08: Report execution reads records incrementally with `lines()` and
  `linein()` instead of loading the complete input file or rendered report into
  arrays.
- AC-09: Console and file output are emitted incrementally while preserving
  heading, break, detail, subtotal, and final-total ordering.
- AC-10: The report reader exposes a callable definition-array entry point;
  the RXPP-generated `employees[]` array can execute the same report without
  writing an intermediate `.report` definition file.
- AC-11: The data fixtures may contain simple leading `#` documentation lines;
  the report reader ignores those lines while preserving normal delimited
  records. Evidence: both employee fixtures retain their data row counts and
  the existing report plans continue to run.
- AC-12: Report-definition files may contain `#` purpose/comment lines that
  are ignored by the report grammar. Evidence: `simple-sample/employees.report`
  contains a purpose header and the simple report plan still completes.

## Implementation steps

- STEP-01: **Complete.** Added the report sample plan and delimited input
  fixture. `rxbuild examples\\report_writer\\simple\\report_writer.build` compiled
  the grammar and reader and ran the sample. Serves AC-01 and AC-02.
- STEP-02: **Complete.** Hardened parse-tree extraction and validate that
  referenced `BREAK` and `TOTAL` columns exist. Serves AC-03.
- STEP-03: **Complete.** Implemented fixed-width numeric mask formatting and
  report line collection. The verified output contains `1,250.50`,
  `4,000.50`, and `4,100.15`. Serves AC-04.
- STEP-04: **Complete.** Implemented console/file output selection and write
  failure reporting. The sample verified file output at
  `cmake-build-release/report_writer/simple-sample/employees.txt`; the empty-output console
  branch is present in `run_report`. Serves AC-05.
- STEP-05: **Complete.** Added `examples/report_writer/README.md` with the
  startup command and generated artifact paths. Serves AC-06.
- STEP-06: **Complete.** Added the 100-record `employees_100.data` fixture and
  `employees_100.rxpp`; verified RXPP preprocessing and execution of the
  generated source. Serves AC-07.
- STEP-07: **Complete.** Replaced whole-file input and report-line buffering
  with `lines()`/`linein()` input and immediate `say`/`lineout` output. The
  100-record fixture verified 100 detail records and four totals; output is
  truncated with `erasefile()` before streaming. Serves AC-08 and AC-09.
- STEP-08: **Complete.** Exposed `reportreader..run_lines(report_name, body)`
  and connected the RXPP-generated `employees[]` sample to it. The dedicated
  `report_writer_100.build` plan compiled and ran the direct-array program,
  producing 100 detail records and four totals without an intermediate
  `.report` definition file. Serves AC-10.
- STEP-09: **Complete.** Added simple leading `#` documentation lines to the
  employee data fixtures and made the reader ignore blank/comment lines.
  Serves AC-11 without changing the report grammar or record format.
- STEP-10: **Complete.** Split the external-definition and RXPP-embedded
  fixtures into `simple-sample/` and `rxpp-sample/`, with separate build and
  output paths.
  Serves AC-06 and AC-07 while keeping the grammar and reader shared.
- STEP-11: **Complete.** Added the established Level L `#` line-comment rule
  to the report grammar and documented the external report definition. The
  simple report plan rebuilt and ran successfully. Serves AC-12.
