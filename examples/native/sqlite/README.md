# SQLite ADDRESS Consumer Demo

This is an ordinary Level G consumer of CREXX's installed `rxsqlite_address`
module. The façade implements `ADDRESS SQLITE` in cREXX over the generic typed
`rxsqlite` provider. The demo contains no SQLite driver and no `sqlite3_*`
calls.

Build it with the normal demo group; no SQLite option or network access is
needed:

```sh
cmake --build cmake-build-debug --target sqlite_address_demo_bin
cmake-build-debug/bin/rxvm \
  cmake-build-debug/examples/native/sqlite/sqlite_address_demo \
  cmake-build-debug/bin/library \
  cmake-build-debug/bin/classlib \
  cmake-build-debug/bin/rxsqlite_address
```

The demo commands are:

- `OPEN path`
- `EXEC sql`
- `QUERY sql`
- `VALUE sql INTO ${target}`
- `CLOSE`

SQL parameters use normal SQLite named parameters. The ADDRESS exit exposes
the host variables and the Level G façade binds their values:

```rexx
name = "Ada"
role = "compiler"
address sqlite "EXEC INSERT INTO people(name, role) VALUES(:name, :role)"
```

`VALUE ... INTO ${target}` returns a scalar through normal ADDRESS
updated-binding copyback. See the supported [rxsqlite library
reference](../../../docs/books/crexx_library_reference/rxsqlite.md) for typed
provider APIs, all commands/functions, ownership and packaging.

## Data dictionary sample

The procedural facade user guide is in docs/data_dictionary_users_guide.md. It
covers definitions, reports, exact-name selections, KEEP/DROP and error
handling.

The moderate `data_dictionary_sample.crexx` consumer adds products,
assemblies and components, including shared references and aliases. Build it
after the dictionary module has been prepared:

```sh
cmake --build cmake-build-debug --target sqlite_data_dictionary_sample_bin
cmake-build-debug/bin/rxvm \
  cmake-build-debug/examples/native/sqlite/data_dictionary_sample \
  cmake-build-debug/examples/native/sqlite/data_dictionary \
  cmake-build-debug/bin/library \
  -a data_dictionary_sample.db \
  --provider-path "cmake-build-debug/bin;cmake-build-debug/bin/providers"
```

It creates 12 members and 15 directed references, then lists all members and
the component subset with direct `uses` and `used-by` counts. It also performs
an alias lookup, a direct BOM query for `TRUCK`, and a where-used query for
`WHEEL`. The database path may be supplied as the argument to use a different
dictionary file.

The larger `data_dictionary_trading.crexx` consumer demonstrates ordered
occurrence and nested BOM/where-used reports. Its presentation is supplied by
the state-free `data_dictionary_report.crexx` module, which accepts raw command
rows and returns formatted `.string[]` lines. The independent
`data_dictionary_report_test.crexx` consumer checks the formatter without
opening SQLite or importing the dictionary module.

The same sources can be built through the graph plan:

```powershell
.\cmake-build-release\bin\rxbuild.exe `
  examples\native\sqlite\data_dictionary.graph.build
```

The graph has separate dictionary, reporting-module, formatter-test and sample
targets, plus the default `run_dictionary_sample` target. Its default run uses
`:memory:` so repeated graph runs do not accumulate duplicate sample members.

The focused Graphviz export sample writes `order.dot`, `shared.dot` and
`complete.dot` in the graph-build output directory. The graph build then invokes
Graphviz `COMMAND` targets to write `order.svg`, `shared.svg` and
`complete.svg`. Keeping rendering in the graph build prevents binary PNG data
from entering the cREXX UTF-8 output channel. Rerunning replaces these fixed
sample filenames. It is built with:

```powershell
.\cmake-build-release\bin\crexx.exe `
  examples\native\sqlite\data_dictionary_graph_sample.graph.build
```

It demonstrates `ddGraph` and `ddGraphMembers`. The sample prints DOT source;
Graphviz is optional for the facade, but required by the rendering targets.
Set `DOT_EXECUTABLE` near the top of
`data_dictionary_graph_sample.graph.build` to the full path to `dot.exe`, or
set Graphviz on `PATH` and use `"dot"` there:

```text
DOT_EXECUTABLE = "E:/Graphviz-16.1.2-win64/bin/dot.exe";
```

The graph build supplies `-a :memory:` by default. If Graphviz is unavailable,
the sample still keeps the DOT files, while the render target fails; it does
not install or execute a shell command.
