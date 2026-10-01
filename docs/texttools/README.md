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
"$product/rxc" -i "$product" -o "$port/imports/texttools" docs/texttools/texttools.crexx
"$product/rxas" -o "$port/imports/texttools" "$port/imports/texttools"
"$product/rxc" -i "$port/imports;$product" -o "$port/generate-books" docs/generate-books.crexx
"$product/rxas" -o "$port/generate-books" "$port/generate-books"
"$product/rxlink" -o "$port/generate-books-linked" \
  "$port/generate-books.rxbin" "$port/imports/texttools.rxbin" "$product/library.rxbin"
```

Keep the import directory dedicated to the library: an old linked program may
also contain `texttools` metadata and compete during binary import discovery.
Generated `.rxas`, `.rxbin`, logs and publications belong in temporary/build
storage, not beside these source files.

Put the product and external tools on `PATH`. `prepare` needs Pandoc and writes
preprocessed chapters, extracted listings, copied assets and TeX. It does not
run TeX or chapter splices:

```sh
export PATH="$product:$PATH"
log=$(mktemp /tmp/crexx-book-prepare.XXXXXX)
"$product/rxvm" "$port/generate-books-linked.rxbin" -a \
  prepare "$PWD" "$port/books" all >"$log" 2>&1
```

`check` checks all executable dependencies for PDF generation and creates no
output directory. `build` runs the complete tool sequence. Use a fresh output
path for each invocation; its parent directory must exist. Paths with spaces
are passed as single native command arguments. The Windows native paths have
not been executed in this review.

```sh
"$product/rxvm" "$port/generate-books-linked.rxbin" -a \
  check "$PWD" "$port/pdf-books" crexx_language_reference
log=$(mktemp /tmp/crexx-book-build.XXXXXX)
"$product/rxvm" "$port/generate-books-linked.rxbin" -a \
  build "$PWD" "$port/pdf-books" crexx_language_reference >"$log" 2>&1
```

To build a single already-staged publication with the smaller driver, compile
and link `build.crexx` exactly as above in place of `generate-books.crexx`. Its
arguments are `prepare|build SOURCE WORK TITLE`, with absolute paths. Shared
boilerplate must retain the relative directory relationships used by the book's
main TeX file. Its explicit build-only options preserve the original behavior:

- `-copy`: copy the resulting PDF into the `CLOUDDRIVE` directory.
- `-crexx`: run `scp` to the original `netrexx@rexxla.org:files/crexx/docs` target.
- `-show`: open the PDF on macOS; other platforms retain the original no-op.

The outer wrapper never requests these options. They are not needed for
generation. The port stops on nonzero Pandoc, XeLaTeX, makeindex, Biber or
xdvipdfmx results and requires a freshly produced PDF header. A header check
does not validate PDF content or layout; inspect the complete log and PDF.

## Source mapping

Every upstream `.rexx` script has a same-named callable in `texttools.crexx`:

| Upstream script/callable | cREXX contract |
| --- | --- |
| `build` | `build(source, work, title, mode)`; mode is `prepare` or `build`. |
| `builddocument` | `builddocument(work, title)`; two XeLaTeX/index/Biber passes, then xdvipdfmx. |
| `copyassets` | `copyassets(source, work)`; authored root files and asset directories. |
| `erasefiles` | `erasefiles(work, chapter)`; remove only that chapter's generated MD/TeX. |
| `getMarkdownFilenames` | `getMarkdownFilenames(source)` returns a sorted string array of case-preserved basenames. |
| `newer` | `newer(original, generated)` compares explicit complete file paths using native `stat`. |
| `preprocessMD` | `preprocessMD(source, output, work, chapter)`; markers and fenced-source extraction. |
| `preprocessTEX` | `preprocessTEX(source, output)`; Pandoc TeX marker transforms. |
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
- Decode Pandoc's escaped underscores in listing paths while keeping TeX text
  escaping for captions. Command splices use raw LaTeX blocks so adjacent commands
  stay separate and shell punctuation/case is preserved.
- Disable Pandoc YAML metadata blocks: these book chapters use `---` as prose
  separators, including `rxpp.md`, which otherwise fails conversion.
- Always regenerate chapters. Upstream's two-MD timestamp comparison misses
  helper, Pandoc and missing-TeX changes. `newer` remains available to callers.
- Do not copy original Markdown over preprocessed Markdown or stale chapter TeX
  over transformed TeX. Copy asset bytes without extension-based loss.
- Stop at each failed tool. XeLaTeX uses noninteractive, halt-on-error mode;
  retain the original fonts, styles and two-pass sequence.
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
outputs. Fake typesetters test orchestration and never qualify real PDF output.
The optional Pandoc run prepares the actual books and checks every extracted
listing's path in the generated TeX.

Real PDF generation still needs the TeX tools, packages, fonts and chapter-command
dependencies described in [BUILDING-DOCS.md](../BUILDING-DOCS.md). macOS execution
does not qualify Linux/Windows. The original VM instruction chapter/manual listing
inputs have separate unresolved references; the port does not silently invent or
remove them. See the generation guide for the distinction between authored inputs,
legacy generated material and disposable output.

Checked on 1 October 2026: the focused harness passes with the existing Debug
product rebuilt from this checkout. Real Pandoc 3.12 prepares 244 Markdown inputs
(40 language, 31 programming, 10 VM and 163 library) and all 1,422 extracted
listings have matching literal paths in the generated TeX. The book inputs and
original wrappers retain their pre-port hashes. Complete PDF typesetting and
non-macOS execution remain unverified.

This is the checkpoint for initial cREXX document generation. Remaining work
includes actual typesetting, reference repairs and fragment-tolerant lexical
highlighting. The initial free-font selection is TeX Gyre Pagella for body text,
Heros for headings and Cursor for code; it is recorded, not yet implemented.
Preserve René's original font defaults and document the substitutions and
restoration route in the eventual handover.
