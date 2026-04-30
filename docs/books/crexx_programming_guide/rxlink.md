# rxlink - the \crexx{} Linker

`rxlink` combines one or more `.rxbin` modules into a linked image with a shared constant pool. The result is still a normal `003` format record stream and can be loaded by `rxvm`, `rxbvm`, or passed on to `rxcpack`.

## Command Line Options

\fontspec{IBM Plex Mono}
\begin{shaded}
  \small
  \obeylines \splice{rxlink -h | sed "s/&/\and/g"}
 \end{shaded}
 \fontspec{TeX Gyre Pagella}

## What It Produces

The linker writes:

1. one shared-pool record
2. one shared-backed module record for each selected module

This reduces duplication across modules while preserving the module boundaries that the virtual machine still uses for load and runtime link work.

## When To Use It

The simplest workflows can stop at `rxas` and run one or more `.rxbin`
files directly under `rxvm`. `rxlink` becomes useful when you want a
single deployable linked image:

- to package an application root together with the modules it needs
- to reduce duplicated constant-pool data before `rxcpack`
- to produce a tidier deployable artifact than a loose set of `.rxbin`
  files
- to remove source/file metadata from a deployment image

The linker does not try to replace all runtime loading. It links the
modules you give it, resolves what it can within that set, and leaves
other imports available for later runtime resolution if that is how the
program is designed.

## Module Selection

The linker can be driven from the command line or from a control file.

- `-r` selects one or more root modules directly.
- `INCLUDE` forces a module into the linked output even if it is not reached from the current roots.
- `OMIT` excludes a module from consideration.
- If no root is specified, `rxlink` selects modules containing `main`.
- If no selected module contains `main`, it falls back to the modules from the first input file.

After roots are chosen, `rxlink` follows exposed imports, `srcfproc`
interface-factory references, and interface relationships to pull in the
modules needed by those roots.

In normal application linking, explicit roots are usually enough.
`INCLUDE` and `OMIT` are mainly for advanced scenarios:

- preparing a linked image that is still expected to do some dynamic or
  late loading
- carrying an optional provider that is not reached by the normal root
  graph
- excluding an alternative provider while testing or packaging a chosen
  variant

## Selectors

Where a directive expects a module selector, `rxlink` accepts:

- the full module name
- the basename
- the source stem
- `input_path::member` for a specific member inside a multi-record input

This is especially useful when one input file contains multiple linked or
concatenated modules.

## Control Files

Control files are line-oriented and are useful when the link set is too
large or too deliberate to keep on one command line. Blank lines are
ignored, and lines beginning with `*` or `#` are treated as comments.

The supported directives are:

- `INPUT path`
- `ROOT selector`
- `INCLUDE selector`
- `OMIT selector`
- `OUTPUT path`
- `MAP path`
- `STRIP SOURCE`

Selectors can use a module name, basename, stem, or `input_path::member` form.

The most common pattern is:

1. list one or more `INPUT` files
2. choose a `ROOT`
3. optionally ask for `STRIP SOURCE`
4. choose an `OUTPUT`

### Minimal Application Control File

```text
* app.ctl
INPUT build/app.rxbin
INPUT build/library.rxbin
ROOT app
STRIP SOURCE
OUTPUT build/app_linked
```

This tells `rxlink` to start from module `app`, pull in the providers it
needs from the given inputs, strip source/file metadata, and write
`build/app_linked.rxbin`.

Run it with:

```sh
rxlink -c app.ctl
```

### Advanced Include Example

```text
# app_with_optional_provider.ctl
INPUT build/app.rxbin
INPUT build/providers.rxbin
ROOT app
INCLUDE providers::fallback_logger
OUTPUT build/app_with_optional_provider
```

Here `fallback_logger` is forced into the linked image even if the normal
root/import walk would not select it. This is an advanced packaging tool:
useful when you know a module will be located indirectly or later than
the normal static reachability analysis can see.

### Omit One Provider Variant

```text
* app_choose_provider.ctl
INPUT build/app.rxbin
INPUT build/provider_a.rxbin
INPUT build/provider_b.rxbin
ROOT app
OMIT provider_b
OUTPUT build/app_provider_a
```

This form is useful when you have alternative providers and want to make
the choice explicitly at packaging time.

### Add A Map File

```text
INPUT build/app.rxbin
INPUT build/library.rxbin
ROOT app
MAP build/app_linked.map
OUTPUT build/app_linked
```

The map file records which modules were selected and which imports, if
any, remain unresolved inside the current link set.

## Stripping Source Metadata

`rxlink` strips inline-body metadata from linked output by default. Inline
metadata is intended for library artifacts consumed by `rxc`; it is not needed
after a final linked image has been built. Use `rxlink -i` to preserve it for
diagnostic or tooling builds. The equivalent control-file form is:

```text
PRESERVE INLINE
```

`rxlink -s` strips source/file metadata from the linked output. The equivalent control-file form is:

```text
STRIP SOURCE
```

This removes the serialized source/file metadata records while preserving runtime metadata such as exposed symbols and interface/class contract data.

This option is intended for deployable `.rxbin` artifacts and for downstream `rxcpack` / wrapped images where source breadcrumbs are not needed.

## Example

```sh
rxlink -o app linked_root.rxbin helpers.rxbin
rxvm app
```

To produce a smaller deployable linked image:

```sh
rxlink -s -o app_small linked_root.rxbin helpers.rxbin
```

The equivalent control-file driven form is:

```text
INPUT linked_root.rxbin
INPUT helpers.rxbin
ROOT linked_root
STRIP SOURCE
OUTPUT app_small
```
