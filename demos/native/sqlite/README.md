# Native SQLite ADDRESS Environment Demo

This demo is the database-shaped native ADDRESS provider. It is intentionally
written around a small C driver table so later database demos can reuse the same
environment protocol and add another driver behind a different ADDRESS
environment name.

`ADDRESS SQLITE` is routed by the normal environment name carried in the
`addressrequest` / `addressfunctionrequest`. The native provider looks up that
name in its driver registry and dispatches to the SQLite driver. Future drivers
can follow the same pattern with their own environment names, for example
`ADDRESS POSTGRES` or `ADDRESS ODBC`.

The CMake build downloads the pinned SQLite amalgamation from sqlite.org when
`CREXX_BUILD_SQLITE_ADDRESS_DEMO=ON`:

```sh
cmake --build cmake-build-debug --target native_sqlite_address_demo
cmake-build-debug/compiler/tests/native_sqlite_address_demo
```

The demo commands are:

- `OPEN path`
- `EXEC sql`
- `QUERY sql`
- `VALUE sql INTO ${target}`
- `CLOSE`

SQL parameters use normal SQLite named parameters, so Rexx code can write:

```rexx
name = "Ada"
role = "compiler"
"EXEC INSERT INTO people(name, role) VALUES(:name, :role)"
```

The ADDRESS exit auto-exposes `name` and `role`; the SQLite driver binds those
values to the SQL parameters. `VALUE ... INTO ${target}` returns a scalar through
the same ADDRESS updated-binding path used by the other demos.
