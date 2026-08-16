# Standard Library

The cRexx standard library is built from a mix of Level B source, rxas, and
native support. The release model is explicit: source imports name the
namespace to use, and runtime images or linked artifacts provide the bytecode
and native pieces needed by the VM. The complete content of the library is documented
in the *Library Reference*; here is a short impression of how to use it and what it
offers in addition to the classic set of built-in functions.

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

The private Level B `_rxhttpcore` module provides binary HTTP framing, parsing
and codecs for higher layers; it is not a user client. User-facing HTTP belongs
to `rxfnsg`: its experimental Level G client provides pooled task requests,
policy, buffered content decoding and bounded response streaming, while its
bounded server dispatches complete request values to task classes. The Level G
LLM providers use the same client and private backend. See
[Concurrent HTTP client and server](../crexx_library_reference/concurrent_http.md)
for the complete surface and examples.

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
surface. Its experimental concurrency classes include `.taskpool`,
`.taskscope`, `.task`, `.completion`, `.tasktarget`, `.taskwork`, `.channel`,
`.channelvalue`, `.byteendpoint` and `.transferbuffer`. They provide explicit
control beneath Level G syntax while preserving one provider-neutral contract.
See [Concurrency classes](../crexx_library_reference/concurrency.md) for
lifecycle rules and examples. Unsupported telemetry and service declarations
fail explicitly; they do not return invented values.

## RexxScript

RexxScript is delivered as a first-class runtime product with
`bin/rexxscript.rxbin` and the standalone `bin/rexxscript` runner. It is an
interpreted, sandboxed Rexx-family scripting surface for rules and generated
code execution, not the Level C compiler path.

The product documentation lives with the runtime source:

- [RexxScript user guide](../../../rexxscript/doc/user-guide.md)
- [RexxScript developer guide](../../../rexxscript/doc/developer-guide.md)

## Level G Libraries

`rxfnsg` contains the LLM client modules used by demos and the experimental
concurrent HTTP client/server implementation. Level G task/parallel syntax and
the classlib concurrency surface are implemented and tested on the current development
baseline, but they are not the stable Level B contract. Start with the
[concurrent programming guide](../crexx_programming_guide/concurrency.md),
which includes complete checked examples and explains when ordinary work stays
on the controlling execution.

## Level L Generated-Output Work

`rxfnsl` contains early Level L language-engineering examples. The first module
is `tinyexpr`, a tiny arithmetic lexer/parser written in the shape that a future
generator might emit. It uses packed binary constants, fixed-size binary token
records, an exposed declaration procedure for token/layout constants, direct
`<at..type>` reads/writes, and zero-copy source-slice compare to test whether
the binary-memory surface is pleasant enough for generated language tooling.

This is intentionally not a public parser-generator API yet. Its job is to
teach the project what emitted Rexx/RXAS should look like before deciding
whether to port re2c, alter a generator backend, or design a narrower Level L
generator.

## Native Plugins

Native functions use the RXPA plugin architecture. Dynamic plugins can be
loaded by the VM, and selected plugins can be statically linked into packaged
executables depending on build configuration.

From Level B source, calls look the same whether the implementation is written
in cRexx, rxas, or native code. The import and runtime library path decide
which module or plugin is available.
