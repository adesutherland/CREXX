# Standard Library

The \crexx{} standard library is built from a mix of Level B source, RXAS, and
native support. The release model is explicit: source imports name the
namespace to use, and runtime images or linked artifacts provide the bytecode
and native pieces needed by the VM.

## Core Level B Library: `rxfnsb`

`rxfnsb` is the Level B built-in-function library. It contains many
REXX-familiar functions such as string, numeric, date/time, argument, and
conversion helpers.

Use it explicitly in reusable Level B source:

```rexx
options levelb
import rxfnsb

say date("w")
say length("hello")
```

The `crexx` driver imports `rxfnsb` automatically only for headerless
top-level scripts.

Many `rxfnsb` functions are written in \crexx{} itself under
`lib/rxfnsb/rexx/`. Low-level functionality is provided through RXAS or native
runtime support where needed.

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
the experimental debugger work.

`rxdb` remains experimental for Release 1 beta 1. Treat it as a smoke-tested
debugging aid, not as a supported full debugger contract.

## Class Library

`classlib` is loaded by the `crexx` driver by default and is part of the beta
surface, but its public contract should stay small until class-library tests
and examples are expanded. Prefer documenting concrete, tested classes rather
than broad promises.

## Level G and LLM Work

`rxfnsg` contains early Level G class-shaped library work, including the LLM
client modules used by demos. This is useful and real, but Level G itself is
not the baseline user language for Release 1 beta 1.

## Native Plugins

Native functions use the RXPA plugin architecture. Dynamic plugins can be
loaded by the VM, and selected plugins can be statically linked into packaged
executables depending on build configuration.

From Level B source, calls look the same whether the implementation is written
in \crexx{}, RXAS, or native code. The import and runtime library path decide
which module or plugin is available.
