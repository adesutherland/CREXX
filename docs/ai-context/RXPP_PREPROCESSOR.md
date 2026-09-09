# RXPP Preprocessor

`rxpp` is the first-class CREXX preprocessor stage for `.rxpp` source files.
It lives in the root `preprocessor/` directory, not under `lib/plugins/`.

## Build Shape

The root `preprocessor/CMakeLists.txt` builds:

- `rxpp`: the executable preprocessor tool, staged in the build `bin/`
  directory.
- `rxpp-sh`: the DSLSH parser wrapper for `.rxpp` editor buffers when parser
  mode is enabled.
- `precomp_static`: the native helper linked into `rxpp`.
- `precomp`: the dynamic RXPA helper module, emitted as `rxprecomp.rxplugin`.
- `rxpp_support_files`: `maclib.rexx`, `macsys.rexx`, `mathlib.rexx`, and
  `syslib.rexx`, staged beside `rxpp` in `bin/` and installed to `bin/`.

The native `precomp` helper remains an RXPP implementation detail. Do not move
it back under `lib/plugins/` unless RXPP itself moves.

The CMake production path separates imports, work, linking, and packaging.
Declared library RXBINs, `rxcexits.rxbin`, and `rxprecomp.rxplugin` are copied
to `preprocessor/imports/compiler/`. `rxpp.crexx` is compiled with
`--no-exe-import` under `preprocessor/members/rxpp/`, so a previous self-image
cannot satisfy an import. The runtime image is written under
`preprocessor/linked/rxpp/`; `rxcpack` writes a temporary C file under
`preprocessor/generated/rxpp/` and that file is renamed atomically before the
native compiler consumes it. Do not merge those directories or reintroduce a
shared cleanup step: generated metadata and every intermediate have one owner.

## Pipeline Role

For `.rxpp` input the supported pipeline is:

```text
source.rxpp
  -> rxpp
generated.crexx
  -> rxc
generated.rxas
  -> rxas
generated.rxbin
  -> rxlink/rxvm
```

The `crexx` wrapper detects `.rxpp` input and invokes `rxpp` before compiling.
`rxc` does not run RXPP internally and ordinary `.crexx`, `.crx`, and `.rexx`
inputs do not pass through RXPP.

Source-tree builds should pass `-m ${CMAKE_SOURCE_DIR}/preprocessor/maclib.rexx`
when calling `rxpp` directly. Installed/wrapper paths use `bin/maclib.rexx`
beside the installed `rxpp` executable.

`rxpp-sh` is intentionally a native C wrapper rather than a Level B driver. It
owns the editor `CodeBuffer`, writes the active `.rxpp` buffer to a temp file,
runs RXPP, parses the generated CREXX through the compiler parser, and maps
diagnostics back to the authored buffer. The wrapper honors `RXPP_SH_RXPP` and
`RXPP_SH_MACLIB`; without them it uses build-tree paths when present and then
falls back to `rxpp`/`maclib.rexx` lookup.

The wrapper emits authoritative shallow RXPP tokens on the original editor
buffer, then overlays generated compiler diagnostics mapped through RXPP source
maps. It recognizes RXPP directives, local macro definitions and calls,
compile-time constants in directives, `{name}` macro variables, comments,
strings, ordinary identifiers, keywords, numbers, and operators. It does not yet
project generated CREXX semantic tokens or included-file macro definitions back
onto the authored RXPP buffer.

## Build metadata directives

Recent additions separate macro discovery from output placement:

- `##LOADMACRO ui` registers the `.rxpm` files in the `ui` directory relative
  to RXPP's selected system/macro-library directory (an absolute directory also
  works). Bodies load lazily on invocation. Later search roots override an
  earlier same-named package. Current package filenames must be lowercase,
  using letters, digits and underscores; each file contains one `##MACRO` body
  terminated by `##MEND`. The complete macro name is matched, not a prefix.
  The selected macro-library root remains authoritative when input and output
  live in separate directories; `##LOADMACRO` does not switch to the input
  source directory. `rxpp_loadmacro` retains a conflicting source-relative
  package to exercise that distinction.
- `##BUILDDIR path` is consumed by `bin/crexx.crexx`, which scans the first
  64 source lines and resolves it relative to the command's working directory.
  RXPP suppresses the directive in generated cREXX but does not itself move
  build products. Direct CMake recipes should keep their explicit `-o` paths.

RXPP also supports explicit external-module and link metadata. These directives
are suppressed from generated cREXX and recorded in the typed `.inc` manifest
beside the generated output:

```rexx
##EXTERNAL ../lib/CallCatalog.rxbin
##LINK CallCatalog_linked
```

`##NORUN` is a compile-only directive. It takes no argument, is suppressed
from generated cREXX, and records `norun|1` in the manifest. `crexx.exe` still
runs RXC and RXAS but does not start RXVME. It is the RXPP equivalent of the
driver's `--noexec` option. A link recipe already stops after RXLINK.

`##EXTERNAL` declares an existing RXBIN module that is added to the final
runtime or link stage.

External paths resolve relative to the input source directory, including when
the source is a bare filename in the working directory. Each `external|` record
holds one complete path; spaces in the source directory must survive manifest
reading and argv construction. Duplicate declarations match the complete,
case-preserved path. Resolving an external path must not change the selected
macro-library root used by a later `##LOADMACRO`. A missing module name is a
preprocessing error and stops the pipeline.

The presence of `##EXTERNAL` does not change the normal RXPP compilation
pipeline:

```text
source.rxpp
  -> rxpp
generated.crexx
  -> rxc
generated.rxas
  -> rxas
generated.rxbin
```

Without `##LINK`, `crexx.exe` invokes `rxvme` with the newly generated RXBIN
followed by the declared external modules:

```text
rxvme generated.rxbin external1.rxbin external2.rxbin ...
```

When `##LINK` is present, the preprocessing, compilation, and assembly stages
are unchanged. Only the final stage changes: `crexx.exe` invokes `rxlink`
instead of `rxvme`, linking the newly generated RXBIN together with the
declared external modules:

```text
rxlink generated.rxbin external1.rxbin external2.rxbin ...
  -> member.rxbin
```

The linked result is not automatically executed.

Compilation or assembly errors stop the recipe before linking. The driver
continues processing later command-line source members after a successful
recipe; `##NORUN` also applies to its own member without changing the command's
execution option for later members. `--nokeep` removes intermediates while
preserving the linked result, including when its member name matches the input.

`##LINK` accepts exactly one bare output member name. It must not contain a
directory separator, drive prefix, or `.rxbin` suffix. `##BUILDDIR` owns the
output directory, while `##LINK` supplies the output member name.

For example:

```rexx
##BUILDDIR ../temp
##EXTERNAL ../lib/CallCatalog.rxbin
##LINK CallCatalog_linked
```

produces:

```text
../temp/CallCatalog_linked.rxbin
```

The manifest uses typed records so compiler imports are never mistaken for
external runtime/link modules:

```text
import|data_CallCatalog
external|../lib/CallCatalog.rxbin
link|CallCatalog_linked
norun|1
```

---
The UI worked example uses `##LOADMACRO ui` with `ui_node.rxpm`,
`ui_command.rxpm` and `ui_launcher.rxpm`, staged below the selected macro-library
root. It retains one RXPP invocation per generated source file: these additions
do not introduce arbitrary multi-output generation. See
`preprocessor/tests/run_rxpp_loadmacro.cmake` and the Text Inspector generation
test for executable examples.

## Source Maps

The compiler applies the source-map prepass to imported generated source as
well as directly compiled source. `source_import_srcmap_factory` retains the
regression for an RXPP-mapped factory argument: map directives must not enter
the ordinary grammar or be mistaken for instructions before `ARG`. Attached
argument diagnostic nodes are skipped during formal-argument traversal, not
silently treated as parameters.

RXPP emits source maps by default for generated CREXX. The generated file's
leading options include `srcmap`, followed by raw source-map directives in the
`@` channel. `rxc` recognizes `options ... srcmap`, strips the raw directives
before normal tokenization, unescapes `@@` to a literal `@`, and remaps
diagnostics and source-step metadata through the source-map table.

Use `##CFLAG nosrcmap` only when deliberately inspecting or preserving legacy
plain generated CREXX.

Important source-map rules:

- `@` is reserved across the whole generated file when `srcmap` is enabled,
  including strings and comments.
- Literal `@` must be emitted as `@@`.
- `@"file"` sets the original file.
- `@Nl"text"` sets the original line and optional line text.
- `@Nc` sets the original source-column base.
- `@N+M{ ... @}` maps generated text to a source span.
- Nested mappings are legal; `rxc` chooses the narrowest enclosing span. RXPP
  emits an outer span for a macro call and narrower spans for substituted fixed
  arguments where it can track their source columns.
- Malformed directives and unbalanced spans are compiler diagnostics with
  `SRCMAP_MALFORMED` or `SRCMAP_UNBALANCED`.

Explicit no-srcmap RXPP output remains ordinary CREXX. It must not escape
literal `@` or emit raw source-map markers.

RXPP keeps source provenance in arrays beside `source[]`:

- `source_origin_file[]`
- `source_origin_line[]`
- `source_origin_text[]`

Any RXPP helper that inserts into `source[]` must keep these arrays aligned with
the inserted lines. `insert_source` copies provenance from the directive line
that caused the generated helper line. `##INCLUDE` and `##USE` override that
default with the included file path and included source line number. Script
macros generated through RexxScript map each emitted line to the whole
script-macro invocation because RXPP does not yet receive token-level
provenance from the script engine.

RXPP refuses input that already contains `options ... srcmap`. That is treated
as generated output that should go directly to `rxc`, not through RXPP again.

## Diagnostics

RXPP warnings and errors use the same shared message catalogs as `rxc`:

- `messages/diagnostics.en_GB.msg`
- `messages/diagnostics.en_US.msg`
- `messages/diagnostics.de_DE.msg`
- `messages/diagnostics.nl_NL.msg`

New RXPP diagnostics should use a stable `RXPP_*` key and call `rxpp_diag`
with named parameters instead of formatting English text at the call site. The
helper in `preprocessor/rxpp.crexx` owns:

- `CREXX_DIAGNOSTICS=raw|localized`
- `CREXX_DIAGNOSTIC_LOCALE`
- `CREXX_MESSAGE_PATH`
- fallback to `en_GB` when an override locale does not define the key

Raw mode follows the compiler shape:

```text
RXPP_SOURCE_MISSING file="demo.rxpp"
```

Localized mode follows the compiler shape:

```text
RXPP_SOURCE_MISSING: Source file is missing: demo.rxpp.
```

Do not reuse a diagnostic key for different messages. The catalog key is the
stable identity; translations depend on one key mapping to one template.

## Focused Tests

Use these focused tests before broader CTest runs:

```sh
ctest --test-dir cmake-build-release -R 'rxc_srcmap|rxpp_(smoke|srcmap|sh_srcmap|sh_default_lookup|diagnostics|diagnostic_catalogs)' --output-on-failure
```

`rxc_srcmap` covers direct compiler source-map preprocessing, positive mapping,
literal `@` escaping, malformed directives, unbalanced spans, and nested-span
precedence. `rxpp_smoke` covers automatic srcmap output plus explicit
`##CFLAG nosrcmap` compatibility. `rxpp_srcmap` covers reviewed RXPP srcmap
output, compile-through stripping/remapping, include-file origin, nested
argument spans, script-macro output spans, diagnostic deduplication after
remapping, and the double-processing guard.
`rxpp_sh_srcmap` checks the prototype DSLSH wrapper path that preprocesses an
editor `.rxpp` buffer and maps a generated compiler diagnostic back to the
original macro argument span, including overlay onto the authored RXPP token.
`rxpp_sh_default_lookup` checks the same wrapper path without environment
overrides, covering the build-tree RXPP/maclib discovery.
`rxpp_sh_tokens` checks RXPP directive, macro identifier, macro variable, and
macro constant token emission on the authored buffer.
`rxpp_diagnostics` checks raw/localized RXPP diagnostic rendering.
`rxpp_diagnostic_catalogs` checks RXPP-emitted `RXPP_*` keys against the shared
catalogs and complete German/Dutch translations.
