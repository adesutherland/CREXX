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

## Source Maps

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
