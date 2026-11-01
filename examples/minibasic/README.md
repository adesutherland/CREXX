# MiniBASIC Level L example

This directory demonstrates how cREXX can host a small language implemented
with a Level L grammar and a Level B reader/interpreter.

## Components

### 1. MiniBASIC grammar

[`grammar/minibasic.levell`](grammar/minibasic.levell) describes the language:

- numbered source lines;
- `REM` comments;
- `LET` and assignment statements;
- integer and string values;
- arithmetic and comparison expressions;
- `PRINT` and `INPUT`;
- `IF ... THEN` and `GOTO`;
- `END`.

The grammar is compiled by `rxlang` into the generated module
`minibasicgrammar.rxbin`. The generated module is a build artifact and should
not be committed as source.

### 2. Reader and interpreter

[`minibasic_reader.crexx`](minibasic_reader.crexx) is a Level B program. It:

1. reads a `.bas` file;
2. obtains the generated grammar through `minibasicgrammar..language()`;
3. parses the source through the `lllanguage` API;
4. builds a named parse-tree view;
5. executes the supported MiniBASIC statements.

The grammar and the reader are deliberately separate. Level L defines the
syntax; Level B supplies the application logic and interpreter behavior.

### 3. Sample program

[`basic1.bas`](basic1.bas) demonstrates a loop using `LET`, `IF`, `GOTO`,
`PRINT`, and `INPUT`.

[`strings.bas`](strings.bas) covers direct, assigned, copied, grouped, empty,
and doubled-quote string values using the same grammar.

## Manual build and run

Run these commands from the repository root with a configured Debug build:

```sh
cmake --build cmake-build-debug --target crexx rxlang rxfnsl rxbuild_legacy --parallel 4
task_bin="$PWD/cmake-build-debug/bin"
task_work="$PWD/cmake-build-debug/minibasic"
mkdir -p "$task_work"

"$task_bin/crexx" --library "$task_work/minibasicgrammar" \
  examples/minibasic/grammar/minibasic.levell
"$task_bin/crexx" --program "$task_work/minibasic_reader" \
  examples/minibasic/minibasic_reader.crexx \
  -i "$task_work" -l "$task_work/minibasicgrammar"

printf '5\n' | "$task_bin/rxvme" "$task_work/minibasic_reader.rxbin" \
  -a examples/minibasic/basic1.bas
```

Use your configured build directory and platform executable suffix as
appropriate. The driver generates the grammar with `rxlang` and performs
compilation, assembly, and linking. The linked reader includes the grammar and
Level L runtime. `INPUT` uses the Level B `pull()` facility.

The expected sample output includes `SUM = 55` and
`The number is non-negative: 5`, without string-literal quote delimiters.
Using `printf '%s\n' -5` instead exercises the negative branch.

The declarative plan
[`BUILD/build-minibasic.build`](BUILD/build-minibasic.build) performs the same
grammar/reader build and supplies input `5` for the sample:

```sh
"$task_bin/crexx" examples/minibasic/BUILD/build-minibasic.build
```

The driver routes ordinary `.build` files to `rxbuild-legacy`; invoking
`"$task_bin/rxbuild-legacy"` with the same plan is equivalent. Canonical
`rxbuild` accepts the separate graph language. These maintained MiniBASIC plans
explicitly place their products under `cmake-build-release/minibasic`, even
when launched by tools from another configured build.

### Scalar-string check

Run the focused string sample through the prepared reader:

```sh
"$task_bin/rxvme" "$task_work/minibasic_reader.rxbin" \
  -a examples/minibasic/strings.bas
```

The expected output is:

```text
direct "quote"
assigned "quote"
assigned "quote"
assigned "quote"
left  right
left  right
```

A final empty `PRINT` adds one blank line. Each `left  right` line contains two
spaces because an empty middle list item still has a separator on each side.
Literal delimiters are removed once during evaluation; doubled quotes become
one quote in the stored value. `PRINT` reads that value through the same
evaluator for literals, variables, and grouped expressions.

## Build a standalone application

The application plan in
[`BUILD/build-minibasic-application.build`](BUILD/build-minibasic-application.build)
uses `COMPILE APPLICATION`. It compiles the MiniBASIC compiler, translates
`basic1.bas` into generated cREXX source, and compiles that source to
`cmake-build-release/minibasic/application/basic1.rxbin`.

Run it from the repository root with the prepared tools:

```sh
"$task_bin/crexx" examples/minibasic/BUILD/build-minibasic-application.build
```

The resulting `basic1.rxbin` is the application image. Its runtime does not
load `minibasic_reader.rxbin` or `minibasicgrammar.rxbin`; those are build-time
inputs to the compiler and remain useful for the interpreter workflow.

The prime-number application is a more control-flow-oriented example. It
prints the prime numbers from 1 through 100 using arithmetic, comparisons, and
`GOTO` loops:

```sh
"$task_bin/crexx" examples/minibasic/BUILD/build-minibasic-primes.build
```

Its standalone image is
`cmake-build-release/minibasic/primes/prime.rxbin`.

The same plan also creates a native executable named `prime.exe` on Windows
or `prime` on Unix in the same directory.

It is created by the `CREATE EXE` phase from the generated application source;
the `.rxbin` remains independently runnable with `rxvme`.

## Development sequence

When extending MiniBASIC, work through the following stages:

1. **Extend the grammar.** Add the new tokens and parser rules in
   `grammar/minibasic.levell`.
2. **Regenerate the grammar module.** Run the `generate.cmake` command above
   and assemble the resulting `.rxas` file.
3. **Inspect the parse tree.** Confirm that the new construct produces the
   intended node names and child positions.
4. **Extend the reader.** Add execution logic to `minibasic_reader.crexx`.
   Keep syntax recognition in Level L and runtime behavior in Level B.
5. **Recompile, assemble, and link.** Link the reader with the regenerated
   grammar module.
6. **Add or update a sample.** Keep a small executable example for each new
   feature.
7. **Test valid and invalid input.** Check both normal execution and clear
   parser/interpreter diagnostics.

## Current limitations

The first interpreter slice does not yet implement:

- `FOR` / `NEXT` loops;
- arrays;
- real-number values;
- multiple statements on one source line;
- subroutines and `GOSUB` / `RETURN`;
- richer string operations;
- a separate runtime module shared by multiple MiniBASIC readers.

These should be added incrementally: grammar support first, parse-tree
inspection second, and execution support third.

## Suggested next steps

1. Add `FOR` / `NEXT`, using the existing line-number dispatch table as the
   basis for loop control.
2. Add explicit numeric and string runtime values so comparisons do not assume
   that every operand is numeric.
3. Improve `PRINT` formatting and string literal handling.
4. Add arrays after the scalar execution model is stable.
5. Move reusable interpreter state and expression helpers into a separate
   MiniBASIC runtime module if a second reader or compiler front end needs them.
6. Add focused automated tests for grammar admission, parse diagnostics, loop
   execution, input branches, and missing `GOTO` targets.
