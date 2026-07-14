# Array-formatting function example

This example contains the former `rxfnsb` `arrayformat` and `arraydump`
procedures. They are diagnostic presentation helpers, not Level B, Level G, or
Level C library contracts.

The source uses `options levelb` because Level B is its implementation language.
The procedures are exported from the example-only `arrayformatdemo` namespace
and built into `arrayformatdemo.rxbin`. The optimized safety test records the
existing behavior; it is not a conformance or performance suite.

The installed module and source live under:

```text
examples/functions/array-formatting/
```

From an unpacked package root, compile and run the safety program with:

```sh
bin/rxc -x -i bin -i examples/functions/array-formatting \
  -o array_formatting_example \
  examples/functions/array-formatting/array_formatting_example.crexx
bin/rxas -o array_formatting_example array_formatting_example
bin/rxvm array_formatting_example.rxbin \
  examples/functions/array-formatting/arrayformatdemo.rxbin \
  bin/library.rxbin
```

`pprint` in `examples/exits/pprint` demonstrates how a compiler exit can emit
code that uses this companion runtime namespace.
