# CREXX

CREXX is a modern implementation of the REXX language built as a
compiler-to-bytecode toolchain. Source programs are compiled to cREXX assembler,
assembled into `rxbin` bytecode, optionally linked into a deployable image, and
run by the CREXX virtual machine.

Current documentation baseline: `crexx-1.0.0-beta.3` on `develop`.
This is beta 3 work in progress until the `v1.0.0-beta.3` tag and release
assets exist. The latest completed beta baseline is `v1.0.0-beta.2`.

## What Is Included

The core toolchain is:

- `rxc`: compiles cREXX source to `.rxas` assembler
- `rxas`: assembles `.rxas` to `.rxbin` bytecode
- `rxlink`: combines one or more `.rxbin` modules into a linked image
- `rxvm` / `rxvme`: executes `rxbin` bytecode using the product dispatch
  engine selected for the C compiler
- `rxbvm`: explicit portable switch-dispatch VM
- `rxtvm`: explicit direct-threaded VM on GNU/Clang-family compilers
- `crexx`: driver for common compile, assemble, link, and run workflows
- `rxcpack`: packages bytecode images as C source for native executable builds

The Release 1 beta line focuses on the implemented Level B language surface,
the bytecode toolchain, core standard libraries, host integration, and packaging
on the supported desktop platforms. `rxdb` exists as an experimental debugger
prototype and is not yet part of the stable release surface.

`develop` also contains an initial Level G structured-concurrency surface:
task declarations and `DO PARALLEL`, explicit pool/scope classes, local and
isolated-process providers, bounded byte endpoints, structured child-process
redirection, and a concurrent HTTP client. This work has local macOS
qualification but is not a released, stable or fully cross-platform contract.

## Build

```bash
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug --target stage-product --parallel 10
cmake --build cmake-build-debug --target qa-smoke --parallel 10
cmake --build cmake-build-debug --target qa-comprehensive --parallel 10
```

The build creates the toolchain under `cmake-build-debug/bin`.
`rxvm` is the stable product entry point: Clang and AppleClang builds link it
to `rxbvm`, GCC builds link it to `rxtvm`, and MSVC builds copy the sole
`rxbvm.exe` engine to `rxvm.exe`. The ordinary test suite follows `rxvm`; a
small dispatch contract test still exercises each concrete engine that was
built.
`qa-smoke` is the fast local gate and prepares only essential and smoke
artifacts. `qa-comprehensive` adds the remaining normal correctness closure;
qualification and stress use their own targets. Each named target completes
its matching build preparation before CTest starts, and tests never initiate a
nested build. Timing-oriented tests are assigned to the serial
`qa-measurement` target and must run separately on a quiescent host.

Parser-mode and syntax-highlighting support use DSL Syntax Highlighter (DSLSH).
If a sibling checkout exists at `../DSL-Syntax-Highlighter`, CMake uses it.
Otherwise CMake fetches the configured DSLSH repository and tag.

Useful DSLSH options:

```bash
cmake -S . -B cmake-build-debug -DDSLSH_GIT_TAG=<tag-or-commit-sha>
cmake -S . -B cmake-build-debug -DDSLSH_PREFER_LOCAL=OFF
```

## Run A Program

For the usual workflow, use the `crexx` driver:

```bash
cmake-build-debug/bin/crexx path/to/program.crexx
```

The individual stages are also available:

```bash
cmake-build-debug/bin/rxc path/to/program.crexx
cmake-build-debug/bin/rxas path/to/program.rxas
cmake-build-debug/bin/rxvm path/to/program.rxbin
```

Current source convention: `.crexx` is the canonical cREXX source extension.
When `rxc` is given an extensionless initial source name, it falls back to
`.crexx`. `.crx` is also accepted, `.rexx` remains supported for explicit
Classic/compatibility sources, and an arbitrary extension on the initial source
is searched for same-extension imports.

## Documentation

- [Beta 3 release notes](docs/releases/v1.0.0-beta.3.md) track the current
  beta 3 WIP scope, timetable, and known limitations.
- [Beta 2 release notes](docs/releases/v1.0.0-beta.2.md) summarize the latest
  completed beta scope, signing status, and known limitations.
- [Release 1 plan](docs/release-1-plan.md) tracks the fixed-date path to
  Release 1, including scope tiers, gates, and provisional issue owners.
- [Release documentation](docs/index.md) is the main entry point for current
  as-built user and technical documentation.
- [Roadmap](docs/ROADMAP.md) collects future direction and non-release
  commitments separately from current technical documentation.
- [Documentation map](docs/DOCS_MAP.md) explains which documentation area to
  use and how much authority each area has.
- [Level B tutorial](docs/books/crexx_programming_guide/levelb_tutorial.md)
  teaches practical Level B for Rexx programmers with tested examples.
- [Concurrent programming](docs/books/crexx_programming_guide/concurrency.md)
  teaches the initial Level G task and scope model with complete checked
  examples.
- [Tasks and parallel execution](docs/books/crexx_language_reference/concurrency.md)
  defines the Level G syntax and evaluation rules.
- [Concurrency classes](docs/books/crexx_library_reference/concurrency.md) and
  [concurrent HTTP](docs/books/crexx_library_reference/concurrent_http.md)
  document the explicit library surface.
- [Concurrency architecture](docs/ai-context/CREXX_CONCURRENCY.md) records the
  ownership, transfer, provider and implementation boundaries for maintainers.
- [RexxScript user guide](rexxscript/doc/user-guide.md) covers the standalone
  `rexxscript` runner, the `REXXSCRIPT` command, and the direct evaluator API.
- [Language reference](docs/books/crexx_language_reference/about.md) covers the
  implemented language surface.
- [Programming guide](docs/books/crexx_programming_guide/about.md) covers tools,
  running programs, host integration, plugins, and practical workflows.
- [Agent and maintainer notes](docs/ai-context/CREXX_ARCHITECTURE.md) capture
  implementation facts used by contributors and coding agents.
- [Compiler internals](compiler/docs/compiler_architecture_and_debt.md) contain
  technical notes for compiler maintainers.
- [Project wiki](https://github.com/adesutherland/CREXX/wiki) is for vision,
  direction, and history. It is not the current technical reference.

## Contributing

For repository-specific working rules, start with [AGENTS.md](AGENTS.md). It
lists the current implementation references to trust before changing compiler
logic, syntax, runtime behaviour, or Level B source.
