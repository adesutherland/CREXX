# Building and maintaining the documentation

The web pages, printed books, generated API tables and release notes have
different build routes. A normal CREXX product build does **not** regenerate
the four book PDFs. Preserve the book sources and their checked-in generation
inputs until their replacements have been reproduced and reviewed.

## What is source, and what is generated?

| Files | Role | Maintenance rule |
| --- | --- | --- |
| Book-root `.md` chapters; `structure.tex`; the four `crexx_*.tex` main files | Authored prose and book structure | Preserve; edit these to change the publication. |
| `texttools/*.crexx`, `generate-books.crexx`, `texttools/tests` | Maintained generator sources and focused regressions | Preserve. Compile bytecode and build publications in separate output storage. |
| `books/boilerplate/*.tex`, `.sty`, `.bib`; book-specific hand-written TeX; `operation/*.operation`; `.def`, `.rxas`, `.crexx` examples | Authored typesetting, semantics and executable examples | Preserve, including author, publisher and licence notices. A `.tex` suffix does not imply generated output. |
| Uppercase guide symlinks in book folders, such as `CREXX_LEVELB_AUTHORING.md` | Book inputs linked to canonical `ai-context` sources | Six are explicitly included by the current book structures. Preserve the links; edit the canonical guide. They are not redundant copies or generated output. |
| Library `levelb-*.md` and corresponding `lib/rxfnsb/rexx/*.md` | Similar but separately edited documentation | No maintained copier was found. Do not overwrite one with the other: many differ in heading, links or prose. |
| `books/crexx_library_reference/classlib-api.tex` | Generated class/interface/factory/method tables | Regenerate with `lib/classlib/genApiDoc.sh`; the `.crexx` declarations and RexxDoc comments remain the authored inputs. |
| `books/crexx_vm_spec/instruction_chapter.tex` | Checked-in generated instruction chapter | Its generator and SQLite data are retained. It is not a complete current instruction reference. |
| VM `svg/*.gv`, `.gv.svg`; reference-card instruction tables | Derived assets from instruction metadata | Preserve while the legacy generator dependencies remain unqualified. |
| Authored `.svg`, Mermaid `.mmd`, images and PDFs used as figures | Figures/source or build inputs, depending on the file | Do not classify by extension alone. Preserve unless the producing command and its inputs have been verified. |
| Each book's `tex/book/` | Working directory for generated Markdown, TeX, listings, auxiliary files and final PDF | Disposable build output. Keep it separate from the book-root source. |
| Historical QA logs and performance runs | Retired evidence | Recover from Git; they are separate from the authored books and generators. |

René's original [buildbooks.rexx](buildbooks.rexx) and
[buildbooks.crexx](buildbooks.crexx) are preserved. They contain his personal
checkout/tool paths and are examples of the original process, not portable
commands for this checkout. The publication notices in
[bookmeta.tex](books/boilerplate/bookmeta.tex) are also preserved.

## Web documentation

GitHub Pages is configured for **`master`, `/docs`**, using the
`jekyll-theme-minimal` theme in [_config.yml](_config.yml). It renders the
Markdown documentation linked from [index.md](index.md). It does not run
TextTools, RexxDoc or the instruction generator.

Checked on 1 October 2026: GitHub reports the site built successfully from
`ae1607b8e145174422cee7f3e73fbcc37a65226c`. Local changes on `develop`
are not published web content. The ordinary workflows do not contain a book
PDF generation job or a CMake target for these four publications.

## Printed books through cREXX

The maintained route is now the repository-owned [TextTools cREXX
port](texttools/README.md), based on René Jansen's 17 upstream scripts.
[generate-books.crexx](generate-books.crexx) stages the four books in a fresh
output tree, dereferences source-guide symlinks, excludes old `tex/book` outputs,
and calls the port directly. No ooRexx interpreter or external TextTools
checkout is required. The original wrappers and René's publication notices
remain unchanged.

The port preprocesses Markdown, extracts executable listings, runs Pandoc,
prepares TeX/assets, then invokes two XeLaTeX/index/Biber passes and xdvipdfmx.
`prepare` performs the conversion without typesetting; `check` checks the PDF
executables without creating output; `build` attempts the complete PDF route.
Shared boilerplate keeps the relative directory relationships used by the books.

Follow the [build/run commands](texttools/README.md#build-and-run) to compile,
assemble and link the port outside the source tree. Arguments to the resulting
program are:

```text
generate-books check|prepare|build REPO OUTPUT [BOOK|all]
```

Use absolute paths and a fresh output directory whose parent already exists.
Native commands pass paths with spaces as single arguments. Capture stdout and
stderr to a temporary log. Each publication/chapter/tool is identified there,
and a failed external tool stops the pipeline. The build requires a newly
produced PDF header; inspect the PDF and complete log before publication.
The outer wrapper does not copy to CLOUDDRIVE, upload or open a viewer.

### Issue #712 and the original route

[#712](https://github.com/adesutherland/CREXX/issues/712) requests replacing the
ooRexx documentation build with cREXX, starting with the directory-traversing
outer script. Its `builddocs.rexx/.crexx` examples are actually named
`buildbooks.rexx/.crexx` in this checkout. The original scripts are preserved.
With only personal paths substituted, the original cREXX wrapper compiled but
launched all four fixture children from the first book's root. That cause has
not been diagnosed or repaired in the compiler/runtime.

The new route uses explicit native ADDRESS commands for working directories,
file discovery and child argv. The cREXX port extends that working outer-script
route through the inner TextTools conversion pipeline. Focused tool fixtures
verify per-book child directories, both typesetter passes and failure handling;
real Pandoc prepares all 244 current Markdown inputs and 1,422 extracted source
listings. These are conversion and orchestration results, not a real PDF or a
reason to close #712 before the remaining publication gaps are qualified.

### Portable directory discovery through ADDRESS CREXX

Directory listing is already available: `ls` (also named `dir`) returns one
entry name per output record, excluding `.` and `..`. `stat` identifies each
entry as a file, directory or other object. The
[native implementation](../interpreter/rxcrexxcmd.c) has POSIX and Windows
directory-enumeration paths; it does not invoke a platform shell's `ls` or
`dir` executable.

For example, from the repository root:

```rexx
options levelb
import rxfnsb
bookroot = 'docs/books'
i = .int
path = .string
kind = .string
entries = .string[]
info = .string[]
address crexx 'ls :bookroot' output entries
if rc <> 0 then return rc
do i = 1 to entries[0]
  path = bookroot || '/' || entries[i]
  call arraydrop info
  address crexx 'stat :path' output info
  if rc <> 0 then return rc
  parse var info[1] . ' type=' kind ' size=' .
  if kind = 'directory' then say entries[i]
end
```

This pattern was compiled and run on macOS against the actual book directory
and a fixture containing directories with spaces and an ordinary file. It
returns just the directories. This review did not execute the Windows path.
Listing order is not guaranteed. Book discovery must also distinguish the four
publications from support directories such as `boilerplate` and reference cards.

A possible convenience extension is `ls --directories`, returning only
directory names in the same output format. That flag is **not implemented**;
the existing `ls`/`stat` combination already supports directory discovery.

### Dependencies and remaining PDF gaps

The port is based on TextTools commit
[`0767adac303f80ade9af46b509f7ca33353c6065`](https://github.com/RexxLA/TextTools/tree/0767adac303f80ade9af46b509f7ca33353c6065).
All 17 scripts have named cREXX counterparts in [texttools.crexx](texttools/texttools.crexx).
The original external scripts remain useful comparison sources; they are no
longer a runtime dependency of the new route. [The port guide](texttools/README.md)
records the source mapping and repairs, including filename case, multi-index
arguments, fenced-source handling, Pandoc escapes and child return codes.

`prepare` requires Pandoc. Full PDF generation requires XeLaTeX, `makeindex`,
Biber, `xdvipdfmx`, Bash, `stdbuf`, Inkscape, and CREXX tools on `PATH` for
chapter splices. The TeX preamble/title pages also request Minion Pro, IBM Plex
Mono, Avenir Next, JuliaMono, TeX Gyre Pagella and Bodoni URW Light, plus TeX
packages such as `bashful`, `svg` and PSTricks. Executable discovery does not
establish font/package availability. cREXX has replaced the ooRexx requirement;
it does not replace Pandoc or TeX.

Full PDF generation remains **unverified**:

- macOS conversion with a temporary Pandoc 3.12 binary succeeds for all four
  books. XeLaTeX and related PDF tools are absent here. Mock typesetters verify
  command sequencing, directories and failure handling only.
- All 1,422 extracted fenced-source files exist, and their literal filenames
  appear in generated listing paths. The manual VM chapter
  [binary_memory_instructions.md](books/crexx_vm_spec/binary_memory_instructions.md)
  contains unresolved listing/splice names, but corresponding example sources
  are present in HEAD under `examples/`: `bcopy.rxas` corresponds to
  `binary_bcopy.rxas`, `fixedwidth.rxas` to `binary_fixed_width.rxas`,
  `textfields.rxas` to `binary_text_fields.rxas`, `move.rxas` to
  `binary_move.rxas`, and `compare.rxas` to `binary_compare.rxas`. These are
  filename/path wiring gaps, not evidence of deleted example source. The
  current VM `structure.tex` does not include this manual chapter; these
  references alone therefore do not establish a failure in the current PDF
  route. The generated instruction chapter's assets and live commands still
  need actual typesetting validation. Preserve authored material while
  resolving references.
- The native directory/file/process implementation has POSIX and Windows
  paths; the port has only been executed on macOS. Case-sensitive Linux and
  Windows execution, fonts, TeX packages and live chapter splices remain open.
- The VM's live splice spells `byOpcode`, matching
  [byOpcode.crexx](books/crexx_vm_spec/byOpcode.crexx). The checked-in VM chapter
  still contains `rxvme` splices; ensure that alias is available.

The original font choices, styles, publisher data and authored chapters have
not been redesigned to avoid these dependencies.

TeX can be obtained on macOS through
[MacTeX without GUI applications](https://formulae.brew.sh/cask/mactex-no-gui),
which supplies the full TeX Live distribution. The compact
[BasicTeX alternative](https://formulae.brew.sh/cask/basictex) requires additional
packages for these books. Neither is installed here. A macOS CoreText font
registry check finds Avenir Next, but not the other requested families listed
above; that check does not inspect TeX's own font tree. Exact layout validation
still requires the original fonts. The selected fonts for initial generation
are TeX Gyre Pagella for body/title serif text, TeX Gyre Heros for headings,
and TeX Gyre Cursor for code/terminal output. The
[TeX Gyre collection](https://ctan.org/pkg/tex-gyre) uses the GUST Font License
and is included in TeX Live. Apply this choice through an explicit font profile
for initial generation or a detached build copy, preserving René's authored
font defaults for handover. The substitutions are recorded here, but not yet applied by the
generator or validated in a PDF. They enable functional typesetting checks;
they would not establish the original layout. Handover must list every font
substitution and explain how René can select or restore his original typography.

### Listings, execution and syntax highlighting

Current highlighting is performed by LaTeX's `listings` package. The preamble
loads [netrexxformat.tex](books/boilerplate/netrexxformat.tex),
[assemblerformat.tex](books/boilerplate/assemblerformat.tex) and
[rxasformat.tex](books/boilerplate/rxasformat.tex), with hand-maintained language,
keyword, string and comment rules. The TextTools port emits `lstinputlisting`
commands with a language selected by the Markdown fence. It does not obtain
compiler/DSLSH token spans today.

Fenced examples are extracted and printed. Explicit `<!--splice--...-->` tags
and authored TeX `splice` commands run during typesetting; not every listing is
executed. Many listings are incomplete fragments or use other languages.

A token-based cREXX highlighter is a candidate for book fragments: classify
source text without requiring complete syntax, imports, semantic validation or
execution, and format the resulting spans for TeX while preserving every source
character and whitespace gap. Peter Jacob's
[Scanlex.crexx](../lib/classlib/Scanlex.crexx) already provides token types, text
and positions. It is a starting point rather than a complete cREXX lexer: its
identifier, comment and literal rules need comparison with the compiler, and
keyword classification and removal of diagnostic output remain necessary.
Truncated strings/comments and incomplete statements should remain printable;
other fence languages should retain their existing listing handlers.

Parser-based highlighting remains an alternative. CREXX already exposes
`rxc --syntaxhighlight` over DSLSH; THE is an editor consumer rather than a
requirement of that interface. A direct `parser_tester` check on the retained
`hello.crexx` example succeeds and returns comment, keyword and string tokens
without THE. See [the current protocol/integration reference](../compiler/docs/dslsh_integration.md).
Parser mode can return source structure when syntax errors exist, skipping
semantic validation for that request; incomplete fragments therefore do not
rule out this route. A lexical pass would avoid parser and semantic-analysis
work for simple listing styles. Actual latency and fragment coverage still
need measurement. No new lexical renderer or DSLSH plugin is implemented by
this port.

## Class API tables and RexxDoc

The maintained extraction entry point is
[genApiDoc.sh](../lib/classlib/genApiDoc.sh), using
[rexxApiDoc.crexx](../tools/rexxDoc/rexxApiDoc.crexx). It selects public
declarations from explicit source files and emits LaTeX for
`crexxalmanac.sty`. It can use an installed `rexxApiDoc` command, or compile
the extractor with the existing `cmake-build-debug/bin/crexx` product.

Generate a reviewable output without replacing the checked-in table:

```sh
log=$(mktemp /tmp/crexx-api-doc.XXXXXX)
sh lib/classlib/genApiDoc.sh /tmp/crexx-classlib-api.tex >"$log" 2>&1
ctest --test-dir cmake-build-debug --output-on-failure \
  -R '^(classlib_rexxdoc_coverage|rexx_api_doc_exposed_regression)$'
```

The coverage ratchet checks retained RexxDoc source markers; the extraction
regression checks exposed classes, tasks and private-member exclusion. These
checks do not typeset the book or prove every comment is rendered. The reusable
[rexxDoc.crexx](../tools/rexxDoc/rexxDoc.crexx) parser and its source comments are
also retained.

Review result: both tests passed; extracting the current source with the
existing Debug tools produced 74 class/interface blocks. That output differs
from the checked-in `classlib-api.tex`, which was deliberately not overwritten.
The existing tool binaries were reused; this was not a rebuild/qualification of
the current product revision.

## VM instruction chapter, diagrams and reference cards

[instruction_doc.rexx](books/crexx_vm_spec/instruction_doc.rexx) is a **NetRexx**
script, despite its `.rexx` suffix. Its documented command is
`nrc -exec instruction_doc.rexx`, from `docs/books/crexx_vm_spec`; it replaces
`instruction_chapter.tex`. It reads the retained SQLite database
`docs/instructions/instructionbase.sqb` and combines instruction categories,
operation prose and examples. Run it in the disposable build copy.

[svg/write_svgs.rexx](books/crexx_vm_spec/svg/write_svgs.rexx) is another
NetRexx-style generator, run from `docs/books/crexx_vm_spec/svg`; it queries
the same database and invokes Graphviz `dot` for diagrams. The database schema,
SQL inserts, input tables and pipeline fragments remain under `docs/instructions`.
They are a legacy maintenance route, not a single automated refresh from HEAD.

The current opcode source is `binutils/include/rxops.h`, filtered by
`rxop_is_source_mnemonic()` in `binutils/include/rxdefs.h`; `rxas -i` uses that
filter. The database omits `refsame` and newer channel/packed-access opcodes.
It also has 159 instruction mnemonics without a category, which the category
generator cannot include. Preserve the old data and generated chapter, but use
[the human RXAS reference](reference/rxas/README.md) for current semantics.
That human reference is not yet an input to the legacy chapter generator.

The live opcode/mnemonic appendices use René's cREXX
[byOpcode.crexx](books/crexx_vm_spec/byOpcode.crexx) and
[byMnemonic.crexx](books/crexx_vm_spec/byMnemonic.crexx), each running `rxas -i`.
The publication year uses
[TexYear.crexx](books/boilerplate/TexYear.crexx); the convention page runs
[hello.crexx](books/boilerplate/hello.crexx). These executable examples are retained.

The separate RXAS reference card uses
[makefile.refcard](books/crexx_reference_cards/makefile.refcard) and
[safe.pipe](books/crexx_reference_cards/safe.pipe). The makefile typesets existing
tables with XeLaTeX. The pipe fragments explain creation of `instr.tex`,
`opcodes.tex` and `mnemonic.tex`, but depend on an `instructions.txt` input not
present in that directory and are not invoked by the makefile. NetRexx/pipeline
support, font availability and the old interactive TeX warning described in
its README remain unqualified. The old `md2tex.sh` files contain commented
personal commands and are not an active complete build script.

## GitHub release notes

Release source is `docs/releases/<tag>.md`. The **Prepare release notes** step
in [.github/workflows/build.yml](../.github/workflows/build.yml) currently copies
that file into the GitHub Release body without rebasing relative links.

Use tag-pinned absolute links in the published body and verify the resulting
links. The workflow currently does not rebase relative source links. Release
publication is separate from the book-generation route in issue #712.
