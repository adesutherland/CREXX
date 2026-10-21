# GLOBALV provider

This component incorporates the portable GLOBALV implementation from the
VM/370plus-style C application. It produces three optional artifacts:

- `globalv` — the standalone Windows/Linux/macOS command-line utility;
- `rxglobalv.rxplugin` — the RXPA provider containing the shared command core;
- `rxglobalv_address.rxbin` — the separately installed cREXX ADDRESS facade.

The provider is not added to `library.rxbin`, and normal cREXX programs do not
load it automatically. An application that wants GLOBALV explicitly loads the
provider and the ADDRESS facade, for example through its runtime import list:

```rexx
address globalv "SELECT BUILD SETL MODE release"
mode = ""
address globalv "SELECT BUILD GET :mode"
say mode
```

`GET :name` reads the GLOBALV name and writes the result to the cREXX host
variable named `name`. `PUT :name` takes the current host-variable value and
stores it. Without an anchor, `GET`, `LIST`, `STACK`, and `GRPLIST` use normal
ADDRESS output redirection. Direct functions include `ID`, `NAME`, `INIT`,
`GET`, `SET`, `SETS`, `SETL`, and `SETP`.

The PC stores remain the original fixed F 300 record layout:

- lasting values: `%APPDATA%\\globalv` on Windows, or `$HOME/.globalv` on Unix;
- session/table values: `%TEMP%\\globalv` on Windows, or the runtime temporary
  directory on Unix;
- `GLOBALV_HOME` and `GLOBALV_SESSION` override those locations.

Names are uppercased and limited to 32 bytes; values are limited to 255 bytes.
The PC implementation treats those limits as bytes, which is important for
UTF-8 text. CMS source files and `GVBUILD.EXEC` are retained for CMS builds but
are not compiled by the ordinary host CMake build.

The provider serializes calls within one process because the portable PC
adapter uses one active record stream at a time. The PC storage layer also
uses a shared `lock.globalv` sidecar. `globalv.exe` and the cREXX provider
hold the OS lock across each complete read-modify-write operation, so they can
be used by separate processes without changing the fixed F 300 data format.
The lock handle is kept open and is released automatically if a process
terminates. Other GLOBALV implementations must participate in the same
sidecar-lock protocol; direct edits to the data files bypass this protection.

For lock timing diagnostics only, `GLOBALV_TEST_HOLD_LOCK_MS` can be set for
one process. The storage layer then holds the acquired lock for that many
milliseconds before continuing. Values from 1 through 60000 are accepted;
normal applications should leave this variable unset.

## Two-program persistence test

The test sources `tests/globalv_writer.rxpp` and
`tests/globalv_reader.rxpp` demonstrate access from two separate cREXX
programs. Run the writer first, then the reader. The writer uses `SETL`, so
the value is stored in the lasting GLOBALV file and remains available to a
later process; the reader does not purge the group.

Both sources declare `##EXTERNAL &syslib/rxglobalv_address.rxbin`, so they can
be run with the ordinary CLion external tool:

```powershell
crexx.exe tests/globalv_writer.rxpp -keep -verbose3
crexx.exe tests/globalv_reader.rxpp -keep -verbose3
```
