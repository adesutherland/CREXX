# CREXX Examples

Release 1 beta 3 includes a small examples set for smoke testing and first
experiments with the packaged toolchain. The examples cover hello world,
arithmetic, strings, stems/preprocessing, simple algorithms, date/time, system
calls, and an object-oriented bank sketch.

Reusable component demonstrations are grouped separately:

- `functions/` contains callable example modules that are not library
  contracts;
- `classes/` is the home for class and interface demonstrations;
- `exits/` contains standalone compiler-exit examples;
- `plugins/` is reserved for example plugin packages.

The KeyDB examples include `keydb_traversal.crexx` for active-key traversal and
`keydb_lookup_benchmark.crexx` for comparing cold and cached lookups.

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
