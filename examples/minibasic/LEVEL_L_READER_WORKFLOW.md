# Generic Level L Grammar and Reader Workflow

This document is a reusable pattern for adding a small language to cREXX.
MiniBASIC is the example in this directory, but the same workflow applies to
other languages such as a FORTRAN IV or MiniBASIC-style language.

## Step 1: Define the grammar

Create a Level L grammar file:

```text
examples/<language>/grammar/<language>.levell
```

The grammar normally contains:

- token declarations;
- lexer rules;
- parser rules;
- optional token classification or transformation stages;
- optional parser diagnostics and recovery rules.

Keep the grammar responsible for recognizing structure. Do not put application
execution logic into the grammar.

## Step 2: Generate the grammar module

Use the current `crexx` driver with prepared tools from one configured build.
The [MiniBASIC README](README.md#manual-build-and-run) gives the preparation
commands. Replace `<language>` in this template with your language name:

```sh
task_bin="$PWD/cmake-build-debug/bin"
task_work="$PWD/cmake-build-debug/<language>"
mkdir -p "$task_work"
"$task_bin/crexx" --library "$task_work/<language>grammar" \
  examples/<language>/grammar/<language>.levell
```

This produces a generated grammar module:

```text
<language>grammar.rxbin
```

The driver invokes `rxlang`, compiles its generated source, and assembles the
grammar module. Generated source, `.rxas`, and `.rxbin` files belong in the
build directory, not in the source tree. When calling the lower-level CMake
generator directly, its current path is `langtools/cmake/generate.cmake`.

## Step 3: Create the reader

Create a Level B reader:

```text
examples/<language>/<language>_reader.crexx
```

The reader should normally:

1. declare `options levelb`;
2. import the Level L runtime modules;
3. import the generated grammar module;
4. accept a source-file argument;
5. read the source with `readbinary`;
6. create a language request using the generated grammar;
7. parse the source;
8. report parser diagnostics;
9. create an `lltree` view;
10. inspect, execute, or translate the resulting tree.

The essential reader call is:

```rexx
request = .lllanguage..request(<language>grammar..language(),source)
status = request.parse()
```

The reader is not written in Level L. Level L defines the grammar; the reader
is an ordinary Level B program that consumes the generated grammar module.

## Step 4: Inspect the parse tree

Before implementing execution, inspect the reader's resulting named tree:

- node names;
- child order;
- token text;
- line and source offsets;
- optional and repeated constructs.

The exact tree shape is determined by the grammar. The reader must use the
observed tree shape rather than assumptions based only on the source syntax.

## Step 5: Add execution or translation

After parsing works, add language-specific behavior to the reader or to a
separate runtime module.

Typical execution steps are:

1. Build symbol tables or runtime state.
2. Build a dispatch table for labels, line numbers, or procedures.
3. Evaluate literals, variables, and expressions.
4. Execute statements in tree order.
5. Implement branches, loops, calls, and returns.
6. Report unsupported constructs with a language-specific diagnostic.

Keep this logic separate from the grammar. A grammar recognizes a program; the
reader gives that program meaning.

## Step 6: Compile, assemble, and link

Build the Level B reader with the same driver and generated grammar:

```sh
"$task_bin/crexx" --program "$task_work/<language>_reader" \
  examples/<language>/<language>_reader.crexx \
  -i "$task_work" -l "$task_work/<language>grammar"
```

The driver compiles, assembles, and links the reader with the grammar and
required runtime libraries. `--program` chooses the output and builds without
running it. Use your platform's executable suffix as appropriate.

## Step 7: Run and test

Run the linked reader against a sample source file:

```sh
"$task_bin/rxvme" "$task_work/<language>_reader.rxbin" \
  -a examples/<language>/<sample-source>
```

Test at least:

- one valid minimal program;
- one representative program using the main language features;
- malformed syntax and parser diagnostics;
- missing labels or runtime targets;
- input and branch behavior if supported;
- optimized and non-optimized builds where applicable.

If you provide an ordinary `.build` plan, run it through `crexx` or
`rxbuild-legacy`. Canonical `rxbuild` accepts the separate graph language:

```sh
"$task_bin/crexx" examples/<language>/BUILD/<plan>.build
```

## Recommended source layout

```text
examples/<language>/
  README.md
  <language>_reader.crexx
  <sample-source>
  grammar/
    <language>.levell
```

The README should describe the language slice, build commands, current
limitations, and future extensions. Generated grammar and reader artifacts
should remain under `cmake-build-*`.
