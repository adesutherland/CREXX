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
  cmake-build-debug/demos/native/sqlite/sqlite_address_demo \
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
