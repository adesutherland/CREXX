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
