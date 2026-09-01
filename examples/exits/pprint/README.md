# `pprint` compiler-exit example

`pprint` demonstrates a compiler exit that parses a comma-separated argument
list and emits a call to an example-only runtime namespace. The source program
imports that companion namespace explicitly so the dependency is visible:

```rexx
import arrayformatdemo
pprint lines,3,9
```

is replaced at compile time by a call to the `arraydump` procedure supplied by
`examples/functions/array-formatting`. This is demonstration syntax, not a
Level B or Level G language statement. It is deliberately absent from the
default `rxcexits.rxbin` bundle.

The build creates the standalone optimized bundle `pprint_example.rxbin` in
the matching example-artifact directory. Select it explicitly with
`RXCP_EXIT_MODULE=pprint_example` and place both example artifact directories
on the compiler import path.

From an unpacked package root:

```sh
RXCP_EXIT_MODULE=pprint_example bin/rxc \
  -i bin \
  -i examples/exits/pprint \
  -i examples/functions/array-formatting \
  -o pprint_example_program examples/exits/pprint/pprint_example.crexx
bin/rxas -o pprint_example_program pprint_example_program
bin/rxvm pprint_example_program.rxbin \
  examples/functions/array-formatting/arrayformatdemo.rxbin \
  bin/library.rxbin
```
