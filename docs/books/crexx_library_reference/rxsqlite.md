# SQLite with `rxsqlite`

CREXX ships SQLite as the supported `rxsqlite` component. It has two layers:

- the typed `rxsqlite` RXPA provider for applications which need precise value
  types, prepared statements and explicit resource control; and
- the installed `rxsqlite_address` Level G module, which implements
  `ADDRESS SQLITE` and `ADDRESS SQLITE3` over that provider.

There is one SQLite implementation. The ADDRESS module contains no
`sqlite3_*` calls and introduces no second database path.

## Typed provider

Import the public module directly:

```rexx
options levelb
import rxsqlite

database = .binary
statement = .binary
call sqliteopen ":memory:", database
call sqliteprepare database, "select ?1", statement
call sqlitebindint statement, 1, 9223372036854775807
if sqlitestep(statement) = 100 then do
  value = 0
  call sqlitecolumnint statement, 0, value
  say value
end
call sqlitefinalize statement
call sqliteclose database
```

Every procedure which reports status returns one of these provider codes:

| Code | Meaning |
| ---: | --- |
| `0` | success |
| `100` | `sqlitestep` produced a row |
| `101` | `sqlitestep` completed |
| `-1` | invalid argument or bound value |
| `-2` | invalid, stale or cross-session handle |
| `-3` | closed handle |
| `-4` | database/statement handle kind mismatch |
| `-5` | allocation failure |
| `-6` | SQLite operation failed; inspect the structured diagnostics |
| `-7` | requested cREXX type does not match the SQLite value |

The public namespace and exact signatures are:

| Area | Procedures |
| --- | --- |
| connections | `sqliteopen(path, expose handle)`, `sqliteopenmode(path, mode, expose handle)`, `sqliteclose(handle)`, `sqliteexec(handle, sql)` |
| capabilities and control | `sqliteversion()`, `sqlitecapability(handle, name, expose available)`, `sqlitebusytimeout(handle, milliseconds)`, `sqlitecheckpoint(handle, mode, expose log_frames, expose checkpointed_frames)` |
| maintenance | `sqlitebackup(source, destination, pages_per_step, busy_retries, sleep_milliseconds, expose remaining, expose page_count)`, `sqliteintegrity(handle, mode, max_errors, expose ok, expose details)` |
| statements | `sqliteprepare(handle, sql, expose statement)`, `sqlitestep(statement)`, `sqlitereset(statement)`, `sqliteclearbindings(statement)`, `sqlitefinalize(statement)` |
| bind metadata | `sqlitebindindex(statement, name, expose index)`, `sqlitebindcount(statement, expose count)`, `sqlitebindname(statement, index, expose name)` |
| typed binding | `sqlitebindnull`, `sqlitebindint`, `sqlitebindreal`, `sqlitebindtext`, and `sqlitebindblob`, each with `(statement, index, value)` except null, which has no value argument |
| column metadata | `sqlitecolumncount(statement, expose count)`, `sqlitecolumnname(statement, column, expose name)`, `sqlitecolumntype(statement, column, expose type)`, `sqlitecolumnisnull(statement, column, expose value)` |
| typed columns | `sqlitecolumnint`, `sqlitecolumnreal`, `sqlitecolumntext`, and `sqlitecolumnblob`, each with `(statement, column, expose value)` |
| connection state | `sqlitechanges(handle, expose changes)`, `sqlitelastinsertrowid(handle, expose rowid)` |
| diagnostics and cleanup | `sqliteerror(expose code, expose sqlite_code, expose operation, expose message)`, `sqliteerrorextended(expose code, expose sqlite_code, expose sqlite_extended_code, expose operation, expose message)`, `sqliteerrorjson()`, `sqlitecleanup()` |

`sqliteopen` is the create/read-write shorthand. `sqliteopenmode` accepts only
`readonly`, `readwrite`, or `create`. The named capabilities are `fts5`,
`threadsafe`, `session_affinity`, and `json1`. Checkpoint modes are `passive`,
`full`, `restart`, and `truncate`; integrity modes are `quick` and `full`, with
`max_errors` bounded from 1 through 1000.

SQLite integers use signed 64-bit `.int`, reals use `.float`, UTF-8 text uses
`.string`, blobs use exact `.binary`, and SQL NULL is observed through
`sqlitecolumntype` or `sqlitecolumnisnull`. Blob binding and retrieval preserve
embedded zero and arbitrary non-text bytes. Column indexes are zero-based;
bind indexes are one-based, matching SQLite.

### Handles, transactions and sessions

Database and statement values are opaque native `.binary` handles. Copying a
handle is supported: all copies retain the same resource until their cREXX
values are finalized. Explicit close/finalize closes the shared resource, so a
later operation through any copy reports a closed handle. Closing a database
also finalizes its outstanding statements. `sqlitecleanup()` deterministically
closes every live statement and database owned by the current provider session
and returns the number closed.

Handles belong to the VM RXPA V2 session which created them. They cannot be
used by another VM, attached task, process worker or provider session; attempts
fail as invalid handles. Each attached local-task VM instead resolves its own
declared `rxsqlite` provider and receives an isolated registry and diagnostic
state. Handle values are native capabilities, not task-transfer values.

SQLite transaction ownership follows the database connection. Use
`sqliteexec(database, "BEGIN ...")`, `COMMIT`, `ROLLBACK`, or savepoints on
that same handle. The provider does not add repository policy or move a
transaction between connections or sessions.

Each connection opens with `SQLITE_OPEN_FULLMUTEX`, and all provider procedures
are session-affine. Separate VM sessions may operate concurrently; the
provider does not make one connection or statement concurrently shareable
between VMs. Normal SQLite locking, WAL and `sqlitebusytimeout` rules still
apply when connections use the same database file.

Diagnostics are isolated per session. A normal provider call clears the
previous diagnostic first. After `-6`, `sqliteerrorextended` reports the
provider code, SQLite primary code, SQLite extended code, operation and
message. `sqliteerrorjson` reports the same fields as a JSON object.

## `ADDRESS SQLITE`

The Level G façade is a separately installed runtime module. Load it explicitly
when running or packaging an ADDRESS program:

```sh
crexx -l rxsqlite_address program.crexx
crexx -native -l rxsqlite_address program.crexx
```

Example:

```rexx
options levelg

dbpath = ":memory:"
address sqlite "OPEN ${dbpath}"
address sqlite "EXEC CREATE TABLE people(name TEXT, role TEXT)"

name = "Ada"
role = "compiler"
address sqlite "EXEC INSERT INTO people VALUES(:name, :role)"

address sqlite "QUERY SELECT name, role FROM people" output rows
address sqlite "VALUE SELECT count(*) FROM people INTO ${total}"
say total
say addresscall("sqlite", "version")
address sqlite "CLOSE"
```

Commands are:

- `OPEN path [readonly|readwrite|create]` (with `CONNECT` as an alias);
- `EXEC sql` (`EXECUTE` is an alias);
- `QUERY sql`;
- `VALUE sql INTO ${target}` (`SCALAR` is a command alias);
- `CLOSE`; and
- `HELP`.

If no connection is open, SQL commands create a private in-memory database.
An OPEN path containing spaces should be supplied through `${name}` or `:name`
rather than placed directly in the command. Named SQLite parameters `:name`,
`$name`, and `@name` bind the matching exposed ADDRESS host value as text.

QUERY writes one pipe-delimited row per line through normal ADDRESS output
redirection. NULL is an empty field and blobs use SQLite-literal form
`X'0123AB'`. VALUE requires exactly one row, updates its `${target}` through
the normal ADDRESS binding-copyback mechanism, renders NULL as `NULL`, and
rejects an empty or multi-row result.

Functions available through `addresscall("sqlite", name, ...)` are `ID`,
`NAME`, `DRIVER`, `VERSION`, `STATUS`, `ERROR`, `CHANGES`,
`LAST_INSERT_ROWID` (`LAST_INSERT_ID` alias), and `SCALAR(sql)`. `DRIVER`
returns `sqlite`; STATUS and ERROR retain the most recent façade result and
diagnostic. `SQLITE` and `SQLITE3` are separate cached environment instances,
so each owns its own connection and façade state.

## Build and installed package

SQLite 3.53.2 is a source-controlled official amalgamation. Ordinary builds
remain offline and always compile it with mutex support and FTS5. The build
also disables loadable extensions and legacy double-quoted string literals,
and enables API armor and secure delete. A build cannot silently substitute a
system SQLite with a smaller feature set.

The installed surface is:

- `bin/rxsqlite.rxplugin`, plus canonical
  `bin/providers/rxsqlite.rxplugin` for dynamic provider discovery;
- `bin/providers/rxsqlite.a` (or `rxsqlite.lib`) as the canonical archive used
  by `crexx --native`;
- `bin/providers/rxsqlite_static.a` (or `.lib`) as the standard static-target
  compatibility spelling;
- `bin/rxsqlite_static.a` (or `.lib`) as the conventional installed CMake
  target archive; and
- `bin/rxsqlite_address.rxbin` for the Level G ADDRESS façade.

The SQLite objects are already inside the provider archives. Installed package
metadata identifies `rxsqlite` as the native dependency, so an external
consumer needs only the installed CREXX package: no source checkout, SQLite
SDK copy, or manual SQLite linker option is required.

The former incubation namespace `sqlite_boundary` is deliberately not
installed as an alias. Source should import `rxsqlite`, and provider metadata,
dynamic and static artifact stems all use that same name.
