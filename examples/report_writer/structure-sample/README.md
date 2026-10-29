# Structure generator sample

This sample extends the report-writer examples with a small data-class
generator. The input `employee.structure` is parsed by the separate Level L
grammar and converted into `employee.crexx`.

The generated `Employee` class implements:

- `.ObjectEquatable` through field-based `equivalent(.object)` comparison
- `.ObjectPrintable` through deterministic `toString()` output
- a factory and scalar field accessors

Run it from the repository root:

```powershell
.\cmake-build-release\bin\crexx.exe `
  examples\report_writer\structure-sample\structure_generator.graph.build
```

Generated files are written under
`.crexx-build/report_writer/structure-sample/`. The source example is
deliberately limited to `string`, `int`, and `boolean` scalar fields so the
existing marshal/unmarshal behavior remains untouched.
