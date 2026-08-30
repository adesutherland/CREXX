# Level L re2c Output-Adapter Evidence — 2026-08-25

## Scope

This is a bounded output-adapter experiment on the experimental `unicode`
branch. It tests whether vendored re2c can express a generated scanner in valid
cREXX and whether the existing CREXX binary/control-flow surface executes that
shape. It does not implement the authored Level L grammar, Unicode lexing, a
parser maker, or a production backend.

The experiment was run from a working tree based on commit `7db02a8f9`
(`docs: define Level L lexer syntax`). This evidence and the generated artifacts
are retained together in the branch commit containing this file.

## Environment

- Host: `Darwin 25.5.0 arm64`
- macOS: `26.5.2` (`25F84`)
- Generator: vendored `re2c 4.5.1 (debug)`, numeric version `040501`
- Product build: local `cmake-build-unicode-debug`, `CMAKE_BUILD_TYPE=Debug`
- Compiler path: the local worktree's `rxc`, followed by its `rxas` and
  `rxlink`
- Execution: the local worktree's `rxtvm` and `rxbvm`

## Reproduction

From the repository root, or from any other working directory using the script
path:

```sh
experiments/unicode/level-l-emitter/run.sh
```

The runner:

1. configures the local Debug build if necessary and builds the required
   product targets;
2. regenerates `tinyexpr.crexx` with vendored re2c and requires byte identity
   with the retained source;
3. compiles, assembles, and links the generated scanner;
4. compiles and assembles the differential test;
5. verifies selector and binary operations in generated RXAS; and
6. runs the same linked test with both VM implementations and requires
   identical output.

Retained summary:

```text
generated source: byte-identical
state dispatch: .jtable and jumpi
binary scan: bgetu8
token stores: bsetu16, bsetu32, bseti64
linked image: level_l_re2c_linked.rxbin
rxtvm: PASS
rxbvm: PASS
```

## Differential Coverage

The generated scanner is compared with `rxfnsl` TinyExpr for:

- empty and whitespace-only inputs;
- identifiers and decimal integers, including leading zeros;
- all TinyExpr punctuation in two representative expressions; and
- invalid input after a valid prefix, including status and byte offset.

Every token's kind, byte start, byte length, integer value, and token name is
compared. The test output on each VM is:

```text
PASS: Level L re2c emitter matches TinyExpr
```

The existing focused TinyExpr binary-surface and optimized/no-opt selector
tests were also run from the same local Debug tree:

```sh
CMAKE_BUILD_PARALLEL_LEVEL=10 CTEST_PARALLEL_LEVEL=10 \
ctest --test-dir cmake-build-unicode-debug --parallel 10 \
  -R '^(rxfnsl_tinyexpr_binary_surface_smoke|ts_tinyexpr_(noopt|opt)|tinyexpr_dispatch_compare_(noopt|opt))$' \
  --output-on-failure
```

Result: 6/6 passed in 195.76 seconds, including the automatically selected
`linked_opt_runtime_artifacts_build` fixture. The five directly selected tests
all passed; their combined execution after the fixture took approximately two
seconds of wall time.

## Retained File Hashes

```text
1ec54fca8a984a109e0e1964cae9dc6e53a97b1fda2519c24d26449c5c8005e6  crexx.syntax
73de45799770999431c224394ca5828304af2e5742898ae526f11b11dfbf1423  tinyexpr.re
29a9dda5d71eb4b7572eebce5571ff42dec9e8c56468169f3429fc23d55ea0f8  generated/tinyexpr.crexx
016223a16eb043bb1b021cdc90023f1d56296a730fbb49bf887dc56116a75d2d  test_tinyexpr.crexx
43aeb6a5e6dc0f1451650df06dc6473dc31fa545ef9c5186b9929ff2371bbf7b  evidence/result.txt
```

## Result And Boundary

The current cREXX target surface is sufficient for this generated DFA shape:
the state selector uses `.jtable`/`jumpi`, source bytes use `bgetu8`, and the
portable 16-byte token record uses fixed-width binary stores. No compiler,
assembler, linker, or VM change was needed.

This does not choose the eventual table representation. Portable `<at..type>`,
host-native `<packed..int>`, and hybrid portable-storage/packed-runtime layouts
remain candidates for post-PoC measurement. It also does not prove UTF-8
decoding, malformed UTF-8 handling, embedded NUL, streaming, captures, start
conditions, trailing context, Unicode properties, or authored Level L parsing.
