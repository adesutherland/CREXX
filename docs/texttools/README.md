# TextTools for initial cREXX document generation

This is a Level B port of René Jansen's 17 scripts in
[RexxLA/TextTools at `0767adac303f80ade9af46b509f7ca33353c6065`](https://github.com/RexxLA/TextTools/tree/0767adac303f80ade9af46b509f7ca33353c6065).
The original author is René Jansen. The inspected upstream tree has no licence
file; this port does not assign a new licence to his original work. The original
CREXX `docs/buildbooks.rexx` and `.crexx` wrappers remain unchanged. The inspected
upstream originals are retained outside this checkout in the documentation-review
receipts; this directory contains the maintained port, not duplicate originals.

[texttools.crexx](texttools.crexx) exposes the original script names from one
namespace. [build.crexx](build.crexx) is a command entry point;
[generate-books.crexx](../generate-books.crexx) stages and processes one or all
four CREXX books outside the source tree. Neither needs ooRexx or an external
TextTools checkout. Directory enumeration, file copying and process launch use
the existing native `ADDRESS CREXX` commands.

## Build and run

From the repository root, use an existing built/installed product containing
`rxc`, `rxas`, `rxlink`, `rxvm` and `library.rxbin`. For this checkout:

```sh
product="$PWD/cmake-build-debug/bin"
port=$(mktemp -d /tmp/crexx-texttools.XXXXXX)
mkdir "$port/imports"
"$product/rxc" -i "$product" -o "$port/imports/bookhighlight" docs/texttools/bookhighlight.crexx
"$product/rxas" -o "$port/imports/bookhighlight" "$port/imports/bookhighlight"
"$product/rxc" -i "$port/imports;$product" -o "$port/imports/texttools" docs/texttools/texttools.crexx
"$product/rxas" -o "$port/imports/texttools" "$port/imports/texttools"
"$product/rxc" -i "$port/imports;$product" -o "$port/generate-books" docs/generate-books.crexx
"$product/rxas" -o "$port/generate-books" "$port/generate-books"
"$product/rxlink" -o "$port/generate-books-linked" \
  "$port/generate-books.rxbin" "$port/imports/texttools.rxbin" \
  "$port/imports/bookhighlight.rxbin" "$product/library.rxbin"
```

Keep the import directory dedicated to the library: an old linked program may
also contain `texttools` metadata and compete during binary import discovery.
Generated `.rxas`, `.rxbin`, logs and publications belong in temporary/build
storage, not beside these source files.

Put the product and external tools on `PATH`. For the macOS user-local TeX Live
2026 installation used in this qualification, prepend
`$HOME/.local/share/crexx-doc-tools/texlive/2026/bin/universal-darwin` and the
current `$product` directory; that selects the current embedded `rxvme` for
the VM book rather than an older installed copy. `prepare` needs Pandoc and writes
preprocessed chapters, extracted listings, copied assets and TeX. It does not
run TeX or chapter splices:

```sh
export PATH="$product:$PATH"
log=$(mktemp /tmp/crexx-book-prepare.XXXXXX)
"$product/rxvm" "$port/generate-books-linked.rxbin" -a \
  prepare "$PWD" "$port/books" all >"$log" 2>&1
```

`check` checks all executable dependencies for PDF generation and creates no
output directory. `build` runs the complete tool sequence. The optional fifth
argument is a font profile: `initial` (the default) or `original`. The initial
profile substitutes TeX Gyre Pagella and Heros plus JuliaMono in detached TeX inputs;
the original profile keeps René's authored font requests. Both load `fvextra`
in the detached preamble for lexical listings. Use a fresh output
path for each invocation; its parent directory must exist. Paths with spaces
are passed as single native command arguments. The Windows native paths have
not been executed in this review.

The outer driver accepts an optional sixth argument, `VERSION`, after the font
profile: `check|prepare|build REPO OUTPUT [BOOK|all] [initial|original] [VERSION]`.
Pass the exact `display_version` supplied to the matching binary workflow,
such as `crexx-1.0.0-beta.3` or
`crexx-1.0.0-beta.3+dev-snapshot.g0123456789ab`. It must be 1–96 ASCII
characters, start with a letter or digit, and otherwise contain only letters,
digits, `.`, `+`, `-` and `_`. Invalid values fail before staging; underscores
are TeX-escaped and long values receive invisible line-break opportunities.
The supplied version appears on every selected book's detached cover and
publication-data page. Omit it to retain the original cover and the authored
`rxc -v` publication-data splice. The checked-in title, preamble and
publication-data templates are never rewritten by this option.

```sh
"$product/rxvm" "$port/generate-books-linked.rxbin" -a \
  check "$PWD" "$port/pdf-books" all
log=$(mktemp /tmp/crexx-book-build.XXXXXX)
"$product/rxvm" "$port/generate-books-linked.rxbin" -a \
  build "$PWD" "$port/pdf-books" all initial >"$log" 2>&1
# To stamp PDFs with the exact matching binary-workflow version:
versioned_log=$(mktemp /tmp/crexx-book-versioned.XXXXXX)
"$product/rxvm" "$port/generate-books-linked.rxbin" -a \
  build "$PWD" "$port/versioned-pdf-books" all initial \
  crexx-1.0.0-beta.3+dev-snapshot.g0123456789ab >"$versioned_log" 2>&1
```

To rebuild a single **already staged** publication with the smaller driver,
compile and link `build.crexx` exactly as above in place of
`generate-books.crexx`. Its arguments are
`prepare|build SOURCE WORK TITLE [initial|original]`, followed by optional
build-only flags. Pass the same profile used by the outer driver; the smaller
driver defaults to `original` for René's direct-call behavior. Its source tree
must already contain the detached `preparetypesetting` preamble, splice guard,
font profile and Unicode fallback prepared by `generate-books`. Shared
boilerplate must retain the relative directory relationships used by the book's
main TeX file. Its explicit build-only options preserve the original behavior:

- `-copy`: copy the resulting PDF into the `CLOUDDRIVE` directory.
- `-crexx`: run `scp` to the original `netrexx@rexxla.org:files/crexx/docs` target.
- `-show`: open the PDF on macOS; other platforms retain the original no-op.

The outer wrapper never requests these options. They are not needed for
generation. The port stops on nonzero Pandoc, XeLaTeX, makeindex, Biber or
xdvipdfmx results, rejected index entries, or unconverged outlines and
cross-references. It requires a freshly produced PDF header. A header check
does not validate PDF content or layout; inspect the complete log and PDF.

## Source mapping

Every upstream `.rexx` script has a same-named callable in `texttools.crexx`:

| Upstream script/callable | cREXX contract |
| --- | --- |
| `build` | `build(source, work, title, mode, profile)`; mode is `prepare` or `build`, profile is `initial` or `original`. |
| `builddocument` | `builddocument(work, title)`; two XeLaTeX/index/Biber passes, bounded final XeLaTeX convergence, then xdvipdfmx. |
| `copyassets` | `copyassets(source, work)`; authored root files and asset directories. |
| `erasefiles` | `erasefiles(work, chapter)`; remove only that chapter's generated MD/TeX. |
| `getMarkdownFilenames` | `getMarkdownFilenames(source)` returns a sorted string array of case-preserved basenames. |
| `newer` | `newer(original, generated)` compares explicit complete file paths using native `stat`. |
| `preprocessMD` | `preprocessMD(source, output, work, chapter)`; markers and fenced-source extraction. |
| `preprocessTEX` | `preprocessTEX(source, output, linkmap='')`; Pandoc TeX marker transforms and optional checked RXPP book-link mapping. |
| `writeSourceFile` | `writeSourceFile(lines, first, last, output)`; explicit bounded source lines. |
| `joinLinesMD` | `joinLinesMD(source, output)`; retained optional helper, still unused by `build`. |
| `includeasm`, `includelisting` | One-line assembly/source listing transforms. |
| `replaceHyperlink`, `replaceSplice` | One-line link/page-reference and terminal-command transforms. |
| `replacecites`, `replaceindices`, `replacemultiindices` | Citation, single-index and multi-index transforms. |

`copytree` is an additional native staging helper. It follows source symlinks;
when requested by the outer wrapper it excludes old `tex/book` output trees.
Callers must keep source and destination separate. BIF file IO uses absolute
paths because native ADDRESS working directories do not change the process
directory used by those BIFs.

`preparetypesetting` adds `fvextra`, the checked splice guard and (for `initial`)
the scoped Unicode fallback to a staged preamble. `applyfontprofile`
translates only detached `.tex`/`.sty` files for the `initial` profile; the
`original` profile leaves font requests intact. The initial profile uses
TeX Gyre OTF and JuliaMono TTF filenames with explicit bold/italic faces
because this user-local XeTeX does not resolve family names through macOS font
discovery. JuliaMono uses `RawFeature=-calt` to keep source operators literal;
the glyph wrappers retain its native box drawing and call GNU Unifont BMP/Upper
only for absent characters. See [the complete substitution
table](../BUILDING-DOCS.md#dependencies-and-qualification) and install
René's original families before using `original` for a PDF build.

The initial profile wraps long API names in a fixed-width name cell only in
the detached library structure. Escaped underscores and the ten long
constructors without underscores gain legal breaks; their printed identifiers,
the checked-in `classlib-api.tex` and the authored almanac style stay intact.
The detached class table receives only those break commands. A few other long
generated prose terms and paths receive exact,
book-local break hints at visible separators. There is no general rewrite of
inline TeX or program listings.

For `rexx` and `crexx` fences, `preprocessMD` also creates a TeX colour block
from the exact render snapshot through `bookhighlight..render`. `preprocessTEX`
inputs that block with the original listing caption and label. The lexer does
not compile, resolve imports or execute snippets. Plain `text` fences use
`VerbatimInput` for literal Unicode/layout fidelity; `bat` maps to listings'
`[WinXP]command.com` dialect. Other programming-language fences continue
through their authored `listings` handlers. RXPP's pre-compilation flowchart
declares a `text` fence so its box-drawing geometry prints literally. An explicit `splice` is
still the only route that executes an example during typesetting.

The language book leaves RXPP's authored GitHub TOC links intact. Its
[`rxpp-book-links.tsv`](../books/crexx_language_reference/rxpp-book-links.tsv)
records the 48 links whose GitHub slugs differ from Pandoc labels, with the
selected heading title for review. Conversion checks each
mapped target, scopes every Pandoc RXPP label and all 62 local TOC references
under `rxpp-book-`, and leaves generated listing labels alone. In the detached
book TeX only, it promotes the first RXPP heading to a chapter and makes its
local contents a section, so later chapters and running heads stay navigable;
the Markdown/Web headings and anchors remain as authored. When RXPP
headings change, regenerate the Pandoc chapter, compare each TOC link with its
intended heading, then update the map and its provenance.

## Changes from upstream

- Typed arguments/arrays replace classic argument parsing and ooRexx stems;
  filename enumeration uses native `ls`/`stat` rather than Bash redirection.
- Preserve filename and command case. Basenames can contain spaces and dots.
- Extract exact fenced source without an injected Rexx comment. Anonymous names
  are stable `CHAPTER-code-N.txt`; four-backtick fences can contain three
  backticks. Reject unterminated fences and listing filenames outside the work
  directory, rather than hanging or writing beyond that directory.
- Every listing has a stable render snapshot. Reused named examples keep the
  original last-definition executable behavior, but no longer overwrite an
  earlier printed snippet. The current library inputs reuse six names with
  different bodies; all those bodies are retained in the generated snapshots.
- Pass the current line into the multi-index transform; upstream omitted it.
- Keep listing markers in separate paragraphs and disable Pandoc line wrapping,
  so long filenames remain intact and adjoining prose is not lost by a line
  transformation.
- Decode Pandoc's escaped underscores in listing paths/labels and pass paths
  through `\detokenize` while keeping TeX text escaping for captions. Command splices use raw LaTeX blocks so adjacent commands
  stay separate and shell punctuation/case is preserved.
- Disable Pandoc YAML metadata blocks: these book chapters use `---` as prose
  separators, including `rxpp.md`, which otherwise fails conversion.
- Always regenerate chapters. Upstream's two-MD timestamp comparison misses
  helper, Pandoc and missing-TeX changes. `newer` remains available to callers.
- Do not copy original Markdown over preprocessed Markdown or stale chapter TeX
  over transformed TeX. Copy asset bytes without extension-based loss.
- Stop at each failed tool. The detached `splice-guard.tex` also turns a failed
  authored `\splice` command into a TeX error, including an explicit `exit` or
  failed pipeline. Literal terminal stdout preserves line breaks and special
  characters; its paginating container clears caption float state left by
  earlier listings. XeLaTeX uses noninteractive, halt-on-error mode;
  retain the authored font choices, styles and two-pass sequence in source.
- `replaceSpliceNoTerminal` is absent from the inspected upstream source. The
  unused `ntsplice` tag reports an error instead of pretending to implement it.

## Validation and remaining limits

Run the retained checks with Python as the test harness; the implementation and
transformation regressions run in cREXX:

```sh
python3 docs/texttools/tests/run.py --bin "$product"
python3 docs/texttools/tests/run.py --bin "$product" --pandoc /absolute/path/to/pandoc
```

The harness compiles, assembles, links and executes the module/drivers. It checks
transformations, filenames, fences, byte-preserving assets, symlink staging,
four-book tool argv/CWD, explicit options, preflight, freshness and failed/missing
outputs. It also checks all four detached cover stamps, publication-data stamping,
the legacy invocation and invalid version rejection. Fake typesetters test
orchestration and never qualify real PDF output.
The optional Pandoc run prepares the actual books and checks every extracted
listing's path in the generated TeX.

Real PDF generation needs the TeX tools, packages, fonts and chapter-command
dependencies described in [BUILDING-DOCS.md](../BUILDING-DOCS.md). macOS execution
does not qualify Linux/Windows. The inactive manual VM chapter's five stale
example paths were repaired to existing `examples/binary_*.rxas` files; it is
still excluded from the active book structure. The checked-in generated
instruction chapter remains a partial historical view of current opcodes.
See the generation guide for authored inputs, generated material and output.

Checked on 1 October 2026: the focused compile/assemble/link and cREXX
regression harness passes with the Debug product from this checkout. Pandoc
3.11 prepares 244 Markdown inputs (40 language, 31 programming, 10 VM and
163 library). All 1,422 extracted listings have exact authored snapshots and
matching paths in generated TeX. A separate one-VM corpus correctness batch
scanned, reconstructed and rendered all 1,422 fences (196,037 input bytes),
writing span/TeX evidence in 7.98 seconds. This includes evidence I/O and
measures neither isolated lexer speed nor end-to-end book-build speed.

The four `initial`-profile real PDFs built on macOS with TeX Live 2026 and
passed independent full-page bounds, representative visual, log, font,
reference, index and listing-fidelity review. Their page counts and SHA-256
hashes are in [BUILDING-DOCS.md](../BUILDING-DOCS.md#dependencies-and-qualification).
The active VM instruction chapter is a partial historical opcode view and the
separate manual binary-memory chapter is excluded, as documented there.
Bounded authored repairs are listed in the generation review; the original
wrappers, source font defaults and René's notices remain for his handover.
PDF layout with the selectable `original` font profile and non-macOS execution
remain unverified.
