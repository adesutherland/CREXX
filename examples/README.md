# CREXX Examples

Release 1 beta 3 includes a small examples set for smoke testing and first
experiments with the packaged toolchain. The examples cover hello world,
arithmetic, strings, stems/preprocessing, simple algorithms, date/time, system
calls, and an object-oriented bank sketch.

Runnable tutorials and demonstrations have one source owner here:

- `basic/`: introductory language samples; intentionally invalid programs stay
  in `tests/demo` and are not shipped as tutorials.
- `cms/` and `native/{kv,sqlite}/`: Rexx and native ADDRESS environment examples.
- `llm/`: live provider demonstrations (require a configured endpoint/model).
- `rexxscript/`: compiler-exit playgrounds.

The compatible `crexx-demos` target builds the optional ADDRESS demonstrations;
`crexx-examples` builds the ordinary example collection. Level L SDK examples
remain in `langtools/examples`; experimental compiler examples remain under
`research/level-l/compilers`. Examples requiring external services are not
silently converted into unattended CI tests.

Reusable component demonstrations are grouped separately:

- `functions/` contains callable example modules that are not library
  contracts;
- `classes/` is the home for class and interface demonstrations;
- `exits/` contains standalone compiler-exit examples;
- `plugins/` is reserved for example plugin packages.

[Franz's Lotto examples](franz-lotto/README.md) provide a standalone cREXX ANSI
form, a NetRexx Swing window and an optional GTK provider kit. They preserve
compatibility with the released Beta 3 tools; native Windows qualification is
the next phase. The transport contains source and focused checks, with build/run
instructions in [its Windows handover](franz-lotto/WINDOWS-HANDOVER.md).

The KeyDB examples include `keydb_traversal.crexx` for active-key traversal and
`keydb_lookup_benchmark.crexx` for comparing cold and cached lookups.

The task examples include `ts_subtask_prime_count.crexx`, which splits a
CPU-bound prime-counting calculation across four subtasks and reduces their
scalar results in the controller.

These categories describe demonstration ownership, not a language level. A
source may use `options levelb` without becoming part of the supported Level B
or Level G library. Built demonstration artifacts are deployed under the same
`examples/` category path and are never placed in the normal `bin/` payload.

Run an example from an unpacked release package with:

```sh
bin/crexx examples/hello.crexx
```

After adding the package `bin/` directory to `PATH`, the same program can be
run as:

```sh
crexx examples/hello.crexx
```
