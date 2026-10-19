# CMS ADDRESS Environment Demo

This demo promotes the CMS-style ADDRESS provider out of the compiler test
fixtures and into a place where it can be run by hand.

The provider is implemented in Rexx:

- `cms_address_environment.crexx` publishes a `CMS` ADDRESS environment.
- It implements `.addressenvironment` for instructions and normal
  `environment_name()` / `environment_id()` identity.
- It implements `.addressfunctionenvironment` so Rexx code can call CMS-style
  functions on the same provider object.
- Callers can use `addressenv("cms")` for the environment object and
  `addresscall("cms", name, ...)` for the simple string-returning function
  path, or `_address_call_response(...)` /
  `_address_function(...)` when they need rc and diagnostics.

The environment is deliberately small, but it behaves like a useful virtual CMS:
`CP QUERY USERID`, `QUERY MSG`, `SET MSG ON|OFF`, `LISTFILE`, `STATE`, `TYPE`,
`MAKEBUF`, `DROPBUF`, `QUERY BUFFER`, `NOTE`, and a tiny `PIPE UPPER` command all
do real work inside the provider. Functions include `ID`, `NAME`, `MSG_MODE`,
`BUFFER_DEPTH`, `LAST_NOTE`, `STATE`, `FILEINFO`, `DISK_FREE`, `FILES`, `UPPER`,
`WORDS`, and `PIPE`.

The CMS `NOTE` command also demonstrates ADDRESS host-variable anchors:

```rexx
note = "Rexx CMS host variable anchor"
"NOTE :note"
```

The compiler auto-exposes the visible `note` scalar, while the CMS handler
decides that `:note` means "use the binding value as the note text".

Build and run it from the repository root:

```sh
cmake --build cmake-build-debug --target address_cms_demo_bin
cmake-build-debug/bin/rxvm \
  cmake-build-debug/bin/library.rxbin \
  cmake-build-debug/compiler/tests/address_cms_provider.rxbin \
  cmake-build-debug/compiler/tests/address_cms_demo.rxbin
```

The compiled `.rxbin` files land under `compiler/tests` for now because the
existing regression harness already knows how to build multi-module ADDRESS
providers there.
