# RXPP And Source Mapping Working Design

Status: working note for beta 3 issues
[#610](https://github.com/adesutherland/CREXX/issues/610) and
[#611](https://github.com/adesutherland/CREXX/issues/611). This is not an
implementation contract until reviewed and approved.

## Purpose

RXPP should be treated as a first-class CREXX toolchain stage, not as invisible
text rewriting. Generated source must carry enough source mapping for `rxc`
diagnostics, DSLSH parser mode, TRACE/debug source-step metadata, and future
debugger views to report against the user's original source.

The beta 3 goal is deliberately narrow:

- define RXPP as a visible preprocessor stage used by the CREXX wrapper for
  `.rxpp` inputs;
- define a parser-friendly inline source-map syntax for generated CREXX;
- teach `rxc` to consume that syntax when `options srcmap` is present;
- update RXPP so ordinary no-macro files pass through cleanly, mapped generated
  files cannot be accidentally preprocessed twice, and source spans are emitted
  for macro-generated text;
- specify a DSLSH wrapper that orchestrates RXPP plus `rxc` for `.rxpp`
  syntax-highlighting use.

## Directory Shape

RXPP should move out of `lib/plugins/precomp/` into a root-level directory so it
is visible as a toolchain component beside the compiler, assembler, linker, and
VM.

Proposed layout:

```text
rxpp/
  CMakeLists.txt
  src/
    rxpp.crexx
    native/
      precomp.c
  lib/
    maclib.rexx
    macsys.rexx
    syslib.rexx
  docs/
    README.md
    user-guide.md
    internals.md
    srcmap.md
  tests/
    CMakeLists.txt
    smoke/
    srcmap/
    dslsh-wrapper/
  examples/
```

The old `lib/plugins/precomp/` path should either be removed in the same
migration or kept only as a temporary compatibility stub that points to `rxpp/`.
The install/package shape should expose `rxpp` as a normal tool in `bin/`.

Top-level documentation changes should include:

- `docs/ai-context/CREXX_ARCHITECTURE.md`: RXPP as a first-class source stage.
- `docs/books/crexx_language_reference/tools.md`: wrapper behavior for `.rxpp`
  input and direct `rxpp` invocation.
- RXPP user/internal docs under `rxpp/docs/`.
- `compiler/docs/dslsh_integration.md`: `.rxpp` parser-mode wrapper
  responsibilities and limits.
- release notes / known limitations when beta 3 scope is finalized.

## Toolchain Role

For beta 3, RXPP remains a separate tool. `rxc` does not run macro expansion
internally.

The supported pipeline is:

```text
source.rxpp
  -> rxpp
generated.crexx with options srcmap and inline source-map markers
  -> rxc
generated.rxas
  -> rxas
generated.rxbin
  -> rxlink/rxvm
```

The CREXX wrapper should run RXPP automatically for `.rxpp` input. It should not
run RXPP for ordinary `.crexx`, `.crx`, or `.rexx` files unless explicitly
requested.

## `options srcmap`

Generated files that contain inline source-map markers must begin with, or
otherwise have their leading options clause include:

```rexx
options levelb srcmap
```

`srcmap` means:

- the source is generated/preprocessed source with a raw pre-lexical source-map
  channel;
- `rxc` must run the source-map prepass before the full parse;
- the marker introducer is reserved in the whole source stream;
- literal marker introducers must be escaped;
- RXPP should treat a file whose leading options already include `srcmap` as
  already processed unless a future force option is explicitly requested.

This keeps double processing as a belt-and-braces failure mode even though the
normal wrapper is expected to run RXPP only for `.rxpp` inputs.

## Source-Map Syntax

Use `@` as the raw marker introducer. It is rare in current CREXX source and is
not currently a Level B or Level C symbol character. The syntax is active only
when `options srcmap` is present.

Because this is a raw pre-lexical channel, the `@` escape rule applies across
the whole source stream, including strings and comments:

```text
@@
```

emits one literal `@` into the cleaned source passed to the normal compiler.
Any unescaped `@` that is not a valid source-map directive is a source-map
prepass error.

Initial directives:

```text
@"path"
```

Sets the current original source file. The file name ends at the closing double
quote.

```text
@3l
@+1l
@-1l
@3l"answer = SQUARE(totl + 1)"
@+1l"next source line"
```

Sets or moves the current original source line. The optional quoted text records
the original line text for the current file/line in the source-map table. This
lets `rxc` produce diagnostics and `.srcstep` metadata without loading the
original source file. When the text is omitted, `rxc` should still report the
mapped file, line, and column; it may use a previously recorded line-text entry
for that file/line, but it must not assume the original source file is available
on disk.

Quoted payload rules for `@"path"` and `@...l"text"`:

- payloads are double-quoted and end at the first unescaped `"`;
- `""` decodes to one literal `"`;
- `@@` decodes to one literal `@`;
- backslash has no special meaning, so Windows paths do not need extra escaping;
- physical newlines are not allowed inside the payload;
- line text is the original logical source line without the line terminator.

Examples:

```text
@"C:\work\demo.rxpp"
@3l"say ""email@@example.com"""
```

```text
@10c
@+4c
@-2c
```

Sets or moves the current original source column base.

```text
@10+16{ ... @}
@+7+4{ ... @}
```

Opens and closes a mapped span.

- `@10+16{` maps the enclosed generated text to the current source file, current
  source line, absolute source column 10, length 16.
- `@+7+4{` maps the enclosed generated text to the current source file, current
  source line, current source column base plus 7, length 4.
- `@}` closes the most recent open source-map span.

Both absolute and relative column forms should be supported from the start. The
working hypothesis to test is that relative columns may make RXPP emission
simpler because RXPP normally knows the start position of the source construct
it is expanding and can express nested token spans as offsets from that anchor.

## Worked Example

Original `demo.rxpp`:

```rexx
##define SQUARE(x) {(x) * (x)}

answer = SQUARE(totl + 1)
```

Generated output:

```rexx
options levelb srcmap
@"demo.rxpp"
@3l"answer = SQUARE(totl + 1)"
@10c
answer = @+0+16{(@+7+4{totl@} + 1) * (@+7+4{totl@} + 1)@}
```

After the source-map prepass, `rxc` compiles:

```rexx
options levelb srcmap

answer = (totl + 1) * (totl + 1)
```

The outer map points the whole generated expression back to
`SQUARE(totl + 1)`. The inner maps point both generated `totl` tokens back to
the single original `totl` token.

If a diagnostic lands on the second generated `totl`, `rxc` should report the
original source location:

```text
demo.rxpp:3:17: unknown variable "totl"
answer = SQUARE(totl + 1)
                ^^^^
```

If a diagnostic lands on generated punctuation introduced by the macro body, the
outer map gives a truthful fallback:

```text
demo.rxpp:3:10: diagnostic in expansion of SQUARE(totl + 1)
answer = SQUARE(totl + 1)
         ^^^^^^^^^^^^^^^^
```

## `rxc` Responsibilities

`rxc` should:

1. Recognize `srcmap` in the leading options scan and the full options grammar.
2. Run a raw source-map prepass before the normal Level B / Level C parser when
   `srcmap` is present.
3. Strip source-map directives and unescape `@@` before normal tokenization.
4. Build a generated-offset to original-source-span table while stripping.
5. Preserve enough generated-buffer offsets to map later token, AST, source
   tree, and diagnostic locations back to original source spans.
6. Prefer the narrowest enclosing map when mappings are nested.
7. Fall back to the generated file location when no map applies.
8. Report unbalanced spans, malformed directives, and unescaped marker
   introducers as clear source-map prepass errors.
9. Carry mapped source locations into `.srcstep` metadata where practical, using
   the existing source-step model rather than inventing a second provenance
   vocabulary.
10. Cache original source line text by mapped file/line where available so
    diagnostics and `.srcstep` can show the original line, not just the
    generated line.
11. Avoid requiring the original source file at compile time. If line text is
    missing, mapped diagnostics should still report file, line, and column, but
    may omit the source-line/caret display.

The first implementation can map diagnostics and source steps by line/column
span. Richer secondary provenance, such as macro definition locations in
addition to macro call sites, can be deferred.

## RXPP Responsibilities

RXPP should:

1. Move to the root `rxpp/` directory and build/install as a first-class tool.
2. Emit `options srcmap` in generated output when source-map markers are
   present.
3. Treat input whose leading options already include `srcmap` as already
   processed unless a future explicit force mode is added.
4. Process files with no macros cleanly: no unwanted imports, headers,
   rewrites, or semantic changes. The output may add or normalize the options
   clause needed to mark the file as processed and source-mapped.
5. Escape literal `@` as `@@` in all generated text when `srcmap` mode is used.
6. Maintain current original source file, line, and source-column base while
   reading input.
7. Emit source-map spans around generated text where RXPP knows the source
   origin.
8. Use relative column spans where they simplify emission, and absolute spans
   where they are clearer.
9. Emit `@"file"` and line directives when included files or generated chunks
   change origin.
10. Keep current macro expansion visible and inspectable; generated CREXX should
    remain a useful debugging artifact after source-map stripping.

RXPP does not need to emit generated columns. `rxc` infers generated spans from
where `@...{` and `@}` appear in the stripped stream.

## RexxScript Touchpoints

RexxScript appears in two different places and should not be confused with the
RXPP source-map syntax.

1. RXPP script macros may use RexxScript to generate output lines. RXPP should
   still emit normal `@` source-map spans around those generated lines. The
   primary anchor should be the RXPP macro call site or script-macro invocation
   that caused the generated text. A later extension may add secondary notes
   that point to the script macro definition.
2. The `REXXSCRIPT` compiler exit runs inside `rxc` after parsing. It should use
   the existing compiler-exit source mapping path rather than emitting `@`
   markers. Generated replacement statements should report against the authored
   `REXXSCRIPT` statement or the most specific token span supplied by the exit
   protocol.
3. Standalone RexxScript runtime errors are outside the first RXPP source-map
   slice. They should remain documented as runtime status strings until a
   RexxScript-specific source-name/source-line API is designed.

## DSLSH Integration Options

Current `rxc --syntaxhighlight` projects one `CodeBuffer`. It can highlight
generated CREXX after RXPP has produced it, but it cannot by itself make an
editor buffer containing `.rxpp` source behave as though the generated CREXX
were the original text.

The exact DSLSH answer is still pending. The design should leave room for at
least these options:

1. `rxc`-only generated view. `rxc --syntaxhighlight` runs on generated CREXX
   with `options srcmap`, colors mapped macro-generated spans as macro text, and
   still links inner exact spans to ordinary identifiers/symbols. This is the
   smallest compiler-side extension and is useful when users inspect generated
   output directly.
2. DSLSH library helpers. DSLSH itself may grow helper APIs for generated-source
   scenarios: macro span coloring, mapped diagnostics, paired original/generated
   buffers, or source-map-aware overlays. This would keep the editor-facing
   logic out of CREXX-specific wrapper code.
3. A `.rxpp` wrapper. A Level B wrapper accepts an editor `.rxpp` buffer, runs
   RXPP, runs `rxc --syntaxhighlight` on the generated CREXX, and maps
   diagnostics back to the original `.rxpp` buffer.

Any wrapper or helper approach should avoid stale RXPP output when the source
file, included files, macro library, RXPP binary, compiler binary, or relevant
options change. It should also provide a way to inspect the generated CREXX
when debugging source-map behavior.

The open question is which layer should own original-buffer coordinates. Full
semantic highlighting of the original `.rxpp` source may require an RXPP-aware
source model because RXPP does not produce a formal source AST today.

Acceptance proof, once the direction is selected:

- a prototype can process a `.rxpp` macro case, run RXPP and `rxc` parser mode
  or equivalent DSLSH helper logic, and map at least one compiler diagnostic
  back to the original `.rxpp` line and column;
- the same generated CREXX can still be opened directly and highlighted by
  ordinary `rxc --syntaxhighlight`.

## Tests

Focused beta 3 tests should include:

- `rxc` accepts `options srcmap`, strips markers, and compiles the cleaned
  source.
- `@@` in generated source becomes a literal `@`.
- malformed `@` directives fail with a source-map prepass diagnostic.
- nested spans choose the narrowest matching original span.
- a macro-expanded token diagnostic reports the `.rxpp` source file, line, and
  column.
- generated punctuation inside a macro expansion falls back to the enclosing
  source span.
- `.srcstep` output for mapped code references the original source location
  where practical.
- RXPP no-macro input produces equivalent CREXX and is marked as processed.
- RXPP refuses or safely skips already processed `options srcmap` input.
- a DSLSH integration prototype maps at least one generated diagnostic to the
  original `.rxpp` buffer, or documents that the first slice is generated-view
  macro coloring only.
- the Rexx source-map algorithm demo in
  [`610-611-srcmap-demo.rexx`](610-611-srcmap-demo.rexx) is kept aligned with
  the syntax while the contract is being finalized.

## Demo Output

The current Rexx design demo emits one macro-expanded line, strips the source
map markers, records nested spans, and remaps two synthetic diagnostics:

```text
RXPP emits:
options levelb srcmap
@"demo.rxpp"
@3l"answer = SQUARE(totl + 1)"
@10c
answer = @+0+16{(@+7+4{totl@} + 1) * (@+7+4{totl@} + 1)@}

rxc compiles cleaned generated source:
answer = (totl + 1) * (totl + 1)

rxc captured maps:
 generated col 11 len 4 -> source col 17 len 4
 generated col 24 len 4 -> source col 17 len 4
 generated col 10 len 23 -> source col 10 len 16

generated diagnostic:
answer = (totl + 1) * (totl + 1)
                       ^ unknown variable totl
remapped diagnostic:
demo.rxpp:3:17: unknown variable totl
answer = SQUARE(totl + 1)
                ^^^^

generated diagnostic:
answer = (totl + 1) * (totl + 1)
                    ^ diagnostic on generated operator
remapped diagnostic:
demo.rxpp:3:10: diagnostic on generated operator
answer = SQUARE(totl + 1)
         ^^^^^^^^^^^^^^^^
```

## Implementation Slices

After design approval, the likely follow-on issues are:

1. Move RXPP to root `rxpp/`, preserve build/package behavior, and update
   documentation paths.
2. Add `options srcmap` and the raw `@` source-map prepass to `rxc`.
3. Wire mapped locations into diagnostics, source tree state, and `.srcstep`
   emission.
4. Update RXPP to emit `options srcmap`, escape literal `@`, emit spans for the
   first macro cases, pass no-macro inputs cleanly, and guard against double
   processing.
5. Add focused RXPP/rxc source-map tests.
6. Choose the DSLSH integration direction, then build the selected prototype
   and document its limits.
7. Review RexxScript compiler-exit mappings against the same diagnostic
   expectations.

## Open Questions And Risks

- Column units need an explicit implementation decision. The source-map syntax
  uses one-based columns. Current compiler internals often derive columns from
  byte pointers, while DSLSH uses UTF-8 character offsets for editor positions.
  Beta 3 may initially limit exact column guarantees to ASCII-compatible source
  and document non-ASCII column behavior until the broader UTF/text ownership
  work settles the shared rule.
- Multi-line original source spans are not in the v1 syntax. A span may enclose
  multi-line generated text, but its original anchor is one source line. That is
  probably enough for RXPP's current line-oriented model; multi-line source
  anchors can be a later extension.
- Macro definition provenance is not represented. The primary diagnostic target
  is the macro call site. A later design can add secondary notes for the macro
  definition if needed.
- `options srcmap` reserves `@` in the entire raw generated file. That is
  simple for `rxc`, but RXPP must reliably escape literal `@` everywhere.
- File paths in `@"path"` should be normalized enough for reproducible
  diagnostics without hiding useful user paths. Exact path policy still needs to
  align with package/build expectations.
