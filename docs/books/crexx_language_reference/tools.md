# Tools {#tools}

The public cRexx toolchain is intentionally split into small tools. The
Programming Guide contains workflow examples; this page is a compact reference.

## Compiler: `rxc`

```bash
rxc [options] source_file
```

Common options:

- `-h`: help
- `-c`: copyright and licence details
- `-v`: version
- `-d[level]`: debug/verbose compiler diagnostics
- `-l location`: working location
- `-s source`: source import root; repeatable
- `-i import`: binary import root; repeatable
- `--level level`: default source level when the source omits one
- `--import ns`: inject a file-level import; repeatable
- `--import-rxas`: allow `.rxas` import scanning in binary roots
- `-o output_stem`: RXAS output stem or `.rxas` file
- `-n`: disable optimisation
- `-x`: disable compiler exits; explicit certified-exit statements such as
  `TRACE`, `PARSE`, and `ADDRESS` are rejected instead of rewritten
- `--parser`: parser-mode service
- `--port port`: parser-mode port

Source imports and binary imports are searched separately. If the initial source
name has no extension, `rxc` falls back to `.crexx`. `.crexx` and `.crx` default
to Level G when the source has no `options` clause; `.rexx` defaults to Level C.
An arbitrary extension on the initial source, such as `.the`, is treated as a
source extension for that compile and also defaults to Level G.

Source roots contain `.crexx`, `.crx`, `.rexx`, and any arbitrary extension used
by the initial source file for this compile. Binary roots contain `.rxbin`,
optional `.rxas`, and `.rxplugin`.
The source file's directory is not an implicit binary root, so pass `-i .` or
another `-i` path when a local build-output `.rxbin` is an intended import.

The `.rxpp` preprocessor input extension is reserved for preprocessing and is
not part of source-import discovery. The `crexx` driver writes preprocessed
output as `.crexx` before invoking `rxc`.

`rxc` output is always RXAS text. `-o` names an output stem/path; if the value
already ends in `.rxas`, that path is used as-is, otherwise `.rxas` is appended.
This means dotted stems such as `build/foo.debug` produce
`build/foo.debug.rxas`. If the output name contains a directory separator it is
written there directly; otherwise `-l location`, when supplied, provides the
working output directory.

## Assembler: `rxas`

```bash
rxas [options] source_file
```

Common options:

- `-h`: help
- `-c`: copyright and licence details
- `-v`: version
- `-a`: architecture details
- `-i`: print instruction information
- `-d`: debug/verbose mode
- `-l location`: working location
- `-o output_stem`: RXBIN output stem or `.rxbin` file
- `-n`: disable optimisation

The assembler reads RXAS text and writes RXBIN bytecode.
`rxas -o` follows the same fixed-artifact rule as `rxc`: a value ending in
`.rxbin` is used as-is, and any other stem/path gets `.rxbin` appended.

## Linker: `rxlink`

```bash
rxlink [options] input_file [input_file ...]
```

Common options:

- `-h`: help
- `-o output_stem`: linked RXBIN output stem or `.rxbin` file
- `-c control_file`: linker control file
- `-r root_member`: root module selector; repeatable
- `-m map_file`: write a link map
- `-l location`: working location
- `-s`: strip source/TRACE debug metadata
- `-i`: preserve inline-body metadata
- `-d`: debug/verbose mode

`rxlink` combines modules into a linked image with one shared constant pool.
Use it for deployable bytecode and native packaging.
Linked output is also a fixed `.rxbin` artifact: `rxlink -o app` writes
`app.rxbin`, while `rxlink -o app.rxbin` writes exactly `app.rxbin`.

## Virtual Machines

```bash
rxvm [options] binary_file [binary_file_2 ...] -a args ...
```

Common options:

- `-h`: help
- `-p plugin`: load a plugin
- `-c`: copyright and licence details
- `-d`: debug mode
- `-l location`: working location
- `-v`: version

The VM executables include:

- `rxvm`: threaded interpreter
- `rxbvm`: bytecode-dispatch interpreter
- `rxvme`: threaded interpreter with the standard library image
- `rxbvme`: bytecode-dispatch interpreter with the standard library image

## Disassembler: `rxdas`

```bash
rxdas [options] binary_file
```

Common options:

- `-h`: help
- `-c`: copyright and licence details
- `-v`: version
- `-p`: print all constant-pool details
- `-l location`: working location
- `-o output_file`: output file; stdout by default

## Packager: `rxcpack`

```bash
rxcpack [options] input_file_1 [input_file_2 ...]
```

Common options:

- `-h`: help
- `-c`: copyright and licence details
- `-v`: version
- `-o output_file`: generated C output

`rxcpack` serializes RXBIN images as C data for native executable builds.
Release-style native builds should link first, then package.

## Driver: `crexx`

```bash
crexx in_file_specification... [--option]...
```

The driver wraps the usual compile, assemble, execute, link, and native package
steps. Headerless top-level scripts are compiled with `--level levelb --import
rxfnsb`; explicit modules should still declare their own `options` and
imports.

Common driver options:

- `-help`: print driver help
- `-version`: print version information
- `-exec` / `-noexec`: enable or skip execution after building
- `-native` / `-nonative`: generate or skip a native executable package
- `-compile` / `-nocompile`: enable or skip compile/assemble work
- `-optimize` / `-nooptimize`: enable or disable optimisation
- `-keep` / `-nokeep`: keep or remove intermediate files
- `-decimal` / `-nodecimal`: enable or disable decimal plugin support
- `-s path` / `-source path`: add a source import root
- `-i path`: add a binary import root
- `-import-rxas`: allow RXAS import scanning in binary roots
- `-linkmap path`: write a linker map
- `-link-keep-source` / `-link-strip-source`: preserve or strip source and
  TRACE debug metadata from linked images
- `-link-keep-inline` / `-link-strip-inline`: preserve or strip inline-body
  metadata from linked images
- `-args`: stop driver option parsing; remaining arguments are passed to the
  executed program

If the input filename ends in `.rxpp`, the driver runs RXPP first and then
compiles the generated `.crexx` source. `CREXX_HOME`, when set, is used as the
root for locating the installed `bin` directory and standard library image;
otherwise the driver derives that root from its load path.

The driver does not have its own `-o` option. It derives the compile and
assemble output stem from each input path, preserving the input directory and
turning dots in the basename into underscores after removing the source suffix.

Native packaging links the program with the standard library image, runs
`rxcpack`, and invokes a platform C compiler. The lower-level `crxc.rexx`
helper exposes the same basic flow for explicit scripted builds:

```bash
crxc.rexx crexx_home execName[.ext]
```
