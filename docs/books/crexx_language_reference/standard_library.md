# Standard Library

The cRexx standard library is built from a mix of Level B source, rxas, and
native support. The release model is explicit: source imports name the
namespace to use, and runtime images or linked artifacts provide the bytecode
and native pieces needed by the VM.

## Core Level B Library: `rxfnsb`

`rxfnsb` is the Level B built-in-function library. It contains many
REXX-familiar functions such as string, numeric, date/time, argument, and
conversion helpers. It also contains the supported Level B array helper
surface: `arrayinsert`, `arraydelete`, `arrayappend`, `arrayprepend`,
`arraypop`, `arrayshift`, `arrayget`, `arrayset`, `arraycontains`,
`arrayindexof`, `arrayreverse`, `arrayjoin`, and the older copy, move, sort,
format, dump, find, high-water, and drop helpers. New code should prefer these
standard BIFs over the deprecated native arrays plugin.

`rxfnsb` also exposes the `.stem` class for classic Rexx compound-variable
style string-to-string keyed data. Stems support dotted tails and bracket keys,
for example `s.name` and `s["customer.id"]`.

Use it explicitly in reusable Level B source:

```rexx
options levelb
import rxfnsb

say date("w")
say length("hello")
```

The `crexx` driver imports `rxfnsb` automatically only for headerless
top-level scripts.

Many `rxfnsb` functions are written in cRexx itself under
`lib/rxfnsb/rexx/`. Low-level functionality is provided through RXAS or native
runtime support where needed. The mutating array helpers use VM array
attribute instructions for insert, delete, shrink, and clear operations, so
common list-like operations can adjust the pointer array without a Rexx-level
per-element copy loop.

### Testing and Debugging Rexx BIFs

The library build is a bootstrap build, not an ordinary application build. Most
Rexx BIF source files in `lib/rxfnsb/rexx/` are compiled with compiler exits
disabled (`rxc -x`). That means certified-exit statements such as `TRACE`,
`PARSE`, and `ADDRESS` are rejected in those files during the BIF build with
`#CERTIFIED_EXIT_DISABLED`.

Do not add `TRACE RESULTS` directly to a BIF source file to debug it. Use one
of these routes instead:

- Write or extend a functional test under `lib/rxfnsb/tests_functional/` that
  calls the BIF from normal Rexx code.
- Build a small scratch program with exits enabled, import `rxfnsb`, and call
  the BIF under `TRACE R`, `TRACE I`, `TRACE ASM`, or `TRACE LLM`.
- If the trace needs to include standard-library frames, add
  `TRACE UNSUPPRESS NAMESPACE rxfnsb` before the call. The default TRACE filter
  hides `rxfnsb` and `_rxsysb` so ordinary user traces are not dominated by
  runtime-library internals.
- For native or linked-image debugging, keep source/TRACE debug metadata in the
  linked image. The `crexx -native` driver strips that metadata by default; use
  `--link-keep-source` for a debuggable linked intermediate.

The functional BIF test target is `testbifs`, and the individual tests are
named `ts_*_noopt` and `ts_*_opt`. The system plugin smoke test is named
`test_system`.

## JSON, Sockets, and HTTP

The Level B library includes small, stable building blocks for integration
work:

- `rxjson`: string-oriented JSON validation, path lookup, quoting, arrays, and
  objects
- `rxsocket`: VM-backed TCP sockets with optional TLS depending on build
  configuration
- `rxhttp`: HTTP client support layered on `rxsocket`

These modules are intentionally modest. They provide enough transport and JSON
support for current demos and integrations without claiming to be complete web
frameworks.

## ADDRESS and Trace Support

`_address.rexx` contains the Rexx-side ADDRESS protocol support used by
command dispatch, redirects, sandboxes, function calls, and host-variable
binding helpers.

`trace.rexx` contains the runtime trace/debugger support used by `TRACE` and by
the experimental debugger work. User-facing traces suppress standard library,
compiler-exit, runtime-support, and debugger namespaces by default, while
`TRACE SUPPRESS NAMESPACE`, `TRACE UNSUPPRESS NAMESPACE`, and
`TRACE RESET NAMESPACES` let a debugging session adjust that filter.

`rxdb` remains experimental for the Release 1 beta line. Treat it as a
smoke-tested debugging aid, not as a supported full debugger contract.

## Class Library

`classlib` is loaded by the `crexx` driver by default and is part of the beta
surface, but its public contract should stay small until class-library tests
and examples are expanded. Prefer documenting concrete, tested classes rather
than broad promises.

## Level G and LLM Work

`rxfnsg` contains early Level G class-shaped library work, including the LLM
client modules used by demos. This is useful and real, but Level G itself is
not the baseline user language for the Release 1 beta line.

## Native Plugins

Native functions use the RXPA plugin architecture. Dynamic plugins can be
loaded by the VM, and selected plugins can be statically linked into packaged
executables depending on build configuration.

From Level B source, calls look the same whether the implementation is written
in cRexx, rxas, or native code. The import and runtime library path decide
which module or plugin is available.
