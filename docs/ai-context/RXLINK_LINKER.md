# cREXX Linker Architecture (`rxlink`)

The `rxlink` tool combines one or more assembled `.rxbin` modules into a linked image that keeps module boundaries while deduplicating compatible constant-pool leaves into one shared pool.

## Purpose

Use `rxlink` when you want to:

- bundle a root module with the providers it needs
- turn a loose module set into one deployable linked image
- shrink downstream `rxcpack` / wrapped artifacts by removing duplicated pool entries
- optionally strip source/file metadata from deployable images

This is now the normal native-packaging route for the shipped drivers too:
`crexx`, `crxc`, `rxpp`, and related wrapped tools link a deployable image
first and then pass that linked image to `rxcpack`.

`rxlink` is not a replacement for the VM loader. The output still contains multiple module records, and `rxvm` still performs the final runtime link/load work.

## Output Format

The linker writes a `003`-format record stream:

1. one `RXBIN_RECORD_POOL_SHARED` record containing the shared constant pool
2. one `RXBIN_RECORD_MODULE_SHARED` record per selected module

Each shared-backed module keeps its own instruction stream, header values, and module name/description, but borrows constants from the shared pool.

## Selection Model

Inputs are read as a record stream, so one input file may contain multiple module records. Module selection then happens in this order:

1. apply `OMIT`
2. apply `INCLUDE`
3. apply explicit `ROOT`
4. if no roots were chosen, select modules containing `main`
5. if there is still no root, select modules from the first input file
6. walk imports, `srcfproc` interface references, and interface relationships to pull in required providers
7. reject duplicate selected exports

Selectors match by:

- full module name
- basename
- filename stem with trailing `.rxas` removed
- `input_path::member`

## What The Linker Reads From Metadata

`rxlink` does not need all metadata equally:

- `proc_head` is used to find procedures and detect `main`
- `expose_head` is used to discover imports and exports
- `meta_head` is scanned for `META_INTERFACE` and `META_IMPLEMENTS` so interface definitions and implementations pull each other in
- the instruction stream is scanned for `srcfproc` selector strings so modules referenced only through runtime interface-factory lookup are still retained

The linker preserves the metadata chain in output because the VM and tooling still consume it at runtime.

## Constant-Pool Rewriting

Leaf constants are deduplicated across selected modules when their serialized bytes match:

- `STRING_CONST`
- `BINARY_CONST`
- `DECIMAL_CONST`
- `FLOAT_CONST`

Structured constants are rewritten into the shared pool with all referenced offsets updated:

- procedures
- exposed register/procedure entries
- metadata entries
- instruction operands that point into the pool

## Strip Support

Current conservative strip support is source-only:

- CLI: `-s`
- control file: `STRIP SOURCE`

This removes:

- `META_SRC`
- `META_FILE`

It intentionally does not remove runtime contract metadata such as:

- `META_CLASS`
- `META_ATTR`
- `META_INTERFACE`
- `META_IMPLEMENTS`
- `META_MEMBER`

That keeps interface/class dispatch and metadata-aware tooling behaviour stable while still removing the source text/file path payload that most affects linked image size.

## Control Files

Supported directives are:

- `INPUT path`
- `ROOT selector`
- `INCLUDE selector`
- `OMIT selector`
- `OUTPUT path`
- `MAP path`
- `STRIP SOURCE`

## Testing Guidance

When changing `rxlink`, keep three layers of coverage in mind:

1. format tests: shared-pool/shared-module record layout
2. behavioural tests: linked images run correctly in both `rxvm` and `rxbvm`
3. toolchain tests: `rxdas` can still disassemble linked images, including stripped ones

For broader confidence, the runtime `_opt` path is now wired through linked
images in normal `ctest` coverage. For a focused rerun, use:

- `ctest -L linked_opt --output-on-failure`
- `cmake --build <build-dir> --target linked_opt_sweep`

Be conservative with stripping. If a proposed change removes more than `META_SRC` / `META_FILE`, verify both runtime contract lookup and metadata introspection in `rxvm` before assuming it is safe.
