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

Use the repository grammar generator:

```powershell
$work = "$PWD\cmake-build-release\<language>"
New-Item -ItemType Directory -Force $work | Out-Null

cmake `
  -DLEVEL_L="$PWD\cmake-build-release\bin\levell.exe" `
  -DSPEC="$PWD\examples\<language>\grammar\<language>.levell" `
  -DOUTPUT="$work\<language>grammar.rxas" `
  -DFORMAT=rxas `
  -P lib\rxfnsl\tools\generate.cmake

& .\cmake-build-release\bin\rxas.exe `
  -o "$work\<language>grammar" `
  "$work\<language>grammar.rxas"
```

This produces a generated grammar module:

```text
<language>grammar.rxbin
```

The generated `.rxas` and `.rxbin` files belong in the build directory, not in
the source tree.

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

Before implementing execution, run the reader in parse-tree mode and inspect:

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

Compile the Level B reader:

```powershell
$work = "$PWD\cmake-build-release\<language>"

& .\cmake-build-release\bin\rxc.exe `
  -i ".\cmake-build-release\bin;$work" `
  -o "$work\<language>_reader" `
  examples\<language>\<language>_reader.crexx

& .\cmake-build-release\bin\rxas.exe `
  -o "$work\<language>_reader" `
  "$work\<language>_reader.rxas"

Push-Location $work
& ..\bin\rxlink.exe -s `
  -o <language>_reader-linked `
  <language>_reader <language>grammar
Pop-Location
```

If the reader uses `pull()` or another runtime facility that is disabled by
`-x --no-exe-import`, omit those options as appropriate for the reader.

## Step 7: Run and test

Run the linked reader against a sample source file:

```powershell
& .\cmake-build-release\bin\rxtvm.exe `
  "$work\<language>_reader-linked.rxbin" `
  -a examples\<language>\<sample-source>
```

Test at least:

- one valid minimal program;
- one representative program using the main language features;
- malformed syntax and parser diagnostics;
- missing labels or runtime targets;
- input and branch behavior if supported;
- optimized and non-optimized builds where applicable.

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
