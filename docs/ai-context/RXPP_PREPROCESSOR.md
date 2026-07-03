# RXPP Preprocessor

`rxpp` is the first-class CREXX preprocessor stage for `.rxpp` source files.
It lives in the root `preprocessor/` directory, not under `lib/plugins/`.

## Build Shape

The root `preprocessor/CMakeLists.txt` builds:

- `rxpp`: the executable preprocessor tool, staged in the build `bin/`
  directory.
- `precomp_static`: the native helper linked into `rxpp`.
- `precomp`: the dynamic RXPA helper module, emitted as `rxprecomp.rxplugin`.
- `rxpp_support_files`: `maclib.rexx`, `macsys.rexx`, `mathlib.rexx`, and
  `syslib.rexx`, staged beside `rxpp` in `bin/` and installed to `bin/`.

The native `precomp` helper remains an RXPP implementation detail. Do not move
it back under `lib/plugins/` unless RXPP itself moves.

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

## Source Maps

RXPP source-map emission is opt-in with:

```rexx
##CFLAG srcmap
```

In that mode RXPP emits generated CREXX whose leading options include
`srcmap`, then writes raw source-map directives in the `@` channel. `rxc`
recognizes `options ... srcmap`, strips the raw directives before normal
tokenization, unescapes `@@` to a literal `@`, and remaps diagnostics and
source-step metadata through the source-map table.

Important source-map rules:

- `@` is reserved across the whole generated file when `srcmap` is enabled,
  including strings and comments.
- Literal `@` must be emitted as `@@`.
- `@"file"` sets the original file.
- `@Nl"text"` sets the original line and optional line text.
- `@Nc` sets the original source-column base.
- `@N+M{ ... @}` maps generated text to a source span.
- Nested mappings are legal; `rxc` chooses the narrowest enclosing span.
- Malformed directives and unbalanced spans are compiler diagnostics with
  `SRCMAP_MALFORMED` or `SRCMAP_UNBALANCED`.

No-srcmap RXPP output remains ordinary CREXX. It must not escape literal `@` or
emit raw source-map markers.

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
ctest --test-dir cmake-build-release -R 'rxc_srcmap|rxpp_(smoke|srcmap|diagnostics|diagnostic_catalogs)' --output-on-failure
```

`rxc_srcmap` covers direct compiler source-map preprocessing, positive mapping,
literal `@` escaping, malformed directives, unbalanced spans, and nested-span
precedence. `rxpp_smoke` covers no-srcmap compatibility. `rxpp_srcmap` covers
`##CFLAG srcmap` output, compile-through stripping/remapping, include-file
origin, script-macro output spans, and the double-processing guard.
`rxpp_diagnostics` checks raw/localized RXPP diagnostic rendering.
`rxpp_diagnostic_catalogs` checks RXPP-emitted `RXPP_*` keys against the shared
catalogs and complete German/Dutch translations.
