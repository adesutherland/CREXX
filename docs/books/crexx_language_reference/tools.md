# Tools {#tools}

The public \crexx{} toolchain is intentionally split into small tools. The
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
- `-o output_file`: RXAS output file
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
- `-o output_file`: RXBIN output file
- `-n`: disable optimisation

The assembler reads RXAS text and writes RXBIN bytecode.

## Linker: `rxlink`

```bash
rxlink [options] input_file [input_file ...]
```

Common options:

- `-h`: help
- `-o output_file`: linked RXBIN output
- `-c control_file`: linker control file
- `-r root_member`: root module selector; repeatable
- `-m map_file`: write a link map
- `-l location`: working location
- `-s`: strip source/TRACE debug metadata
- `-i`: preserve inline-body metadata
- `-d`: debug/verbose mode

`rxlink` combines modules into a linked image with one shared constant pool.
Use it for deployable bytecode and native packaging.

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
