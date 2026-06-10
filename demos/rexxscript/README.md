# RexxScript Demo

This is a hand-run playground for the experimental `REXXSCRIPT` compiler exit.

From the repository root after building the toolchain:

```sh
cmake-build-debug/bin/crexx -nokeep demos/rexxscript/rexxscript_demo.crexx
```

The demo shows a host Level B program exposing simple variables to a RexxScript
snippet, capturing script `SAY` output, and reading the updated host variables
after the snippet completes.
