# Report writer example

This example demonstrates a Level L report-definition grammar and a cREXX
reader. The report definition selects a delimiter-separated employee data
file, declares fixed-width output columns, groups records by department, and
calculates a salary total for each group.

From the CREXX project root, run:

```powershell
.\cmake-build-release\bin\rxbuild.exe examples\report_writer\simple-sample\report_writer.build
```

The build creates these generated artifacts:

- `cmake-build-release/report_writer/simple-sample/reportgrammar.rxbin`
- `cmake-build-release/report_writer/simple-sample/report_reader.rxbin`
- `cmake-build-release/report_writer/simple-sample/employees.txt`

The simple sample keeps its external report definition and data file together
in `simple-sample/`. The shared grammar is in `grammar/report.levell` and the shared
reader is `report_reader.crexx`.

The reader supports the first report format used here: `INPUT_FORMAT
DELIMITED`, `DELIMITER`, fixed-width `COLUMN` definitions, `BREAK`, `TOTAL`,
and text `OUTPUT`. The numeric mask `9,999.99` is supported for numeric
columns and totals. The definition is parsed once; input records are then
read with `lines()`/`linein()` and emitted incrementally with `say` or
`lineout`. Blank lines and lines beginning with `#` in the data file are
ignored, so the fixtures can document their field layout without adding a
record. The external report definition also accepts `#` purpose/comment lines.

## Larger RXPP fixture

The `rxpp-sample/` sample contains 100 employee records across four departments.
`rxpp-sample/employees_100.rxpp` embeds the corresponding report definition in an RXPP
`##dalias report` / `REPORT ... END REPORT` block. RXPP generates that block as a
string array, and the generated cREXX program passes that array directly to
the report reader.

To preprocess, compile, and run that embedded-definition sample in one step:

```powershell
.\cmake-build-release\bin\crexx.exe `
  examples\report_writer\rxpp-sample\report_writer_100.build
```

This writes the 100-record report to
`cmake-build-release/report_writer/rxpp-sample/employees_100.txt` without creating an
intermediate `.report` definition file. `COMPILE RXPP` writes the generated
`.crexx` source into the same build directory that `COMPILE PROGRAM` uses for
module imports. The generated source and bytecode are build artifacts and may
be removed after the run.

For a direct RXPP-only run, use `rxpp.exe` with an explicit output and macro
library path. The subsequent program compilation still needs the output
directory on its import path:

To preprocess, compile, assemble, and run the RXPP source directly with
`crexx.exe`, first build the `reportreader` and `reportgrammar` binaries:

```powershell
.\cmake-build-release\bin\crexx.exe `
  -i .\cmake-build-release\report_writer\rxpp-sample `
  .\examples\report_writer\rxpp-sample\employees_100.rxpp `
  -keep -verbose3
```

The `-i` option supplies the module directory needed while compiling the
imported `reportreader` namespace. `##EXTERNAL` supplies the modules for the
runtime or link stage.
