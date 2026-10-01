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
are not published web content. The ordinary Build CREXX workflow now invokes the Linux book job described
below; this does not change the master-only web publication route. There is no
CMake target for these four publications.

## Printed books through cREXX

The maintained route is now the repository-owned [TextTools cREXX
port](texttools/README.md), based on René Jansen's 17 upstream scripts.
[generate-books.crexx](generate-books.crexx) stages the four books in a fresh
output tree, dereferences source-guide symlinks, excludes old `tex/book` outputs,
and calls the port directly. No ooRexx interpreter or external TextTools
checkout is required. The original wrappers and René's publication notices
remain unchanged.

The port preprocesses Markdown, extracts executable listings, runs Pandoc,
prepares TeX/assets, then invokes two XeLaTeX/index/Biber passes, bounded
XeLaTeX outline/cross-reference convergence, and xdvipdfmx.
`prepare` performs the conversion without typesetting; `check` checks the PDF
executables without creating output; `build` attempts the complete PDF route.
Shared boilerplate keeps the relative directory relationships used by the books.

Follow the [build/run commands](texttools/README.md#build-and-run) to compile,
assemble and link the port outside the source tree. Arguments to the resulting
program are:

```text
generate-books check|prepare|build REPO OUTPUT [BOOK|all] [initial|original] [VERSION]
```

Use absolute paths and a fresh output directory whose parent already exists.
The default `initial` profile selects free TeX Gyre and JuliaMono fonts in the detached
build copy. Pass `original` as the fifth argument to retain René's authored
font choices. Both profiles add `fvextra` to the detached preamble for lexical
listings; neither edits the source preamble. Native commands pass paths with
spaces as single arguments. Capture stdout and
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
verify per-book child directories, typesetter passes and failure handling;
real Pandoc prepares all 244 current Markdown inputs and 1,422 extracted source
listings. Those fixture results qualify conversion and orchestration. The
real initial-profile PDF and page results below qualify the printed route;
the original wrapper's working-directory defect remains separate.

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

### Dependencies and qualification

The port is based on TextTools commit
[`0767adac303f80ade9af46b509f7ca33353c6065`](https://github.com/RexxLA/TextTools/tree/0767adac303f80ade9af46b509f7ca33353c6065).
All 17 scripts have named cREXX counterparts in [texttools.crexx](texttools/texttools.crexx).
The original external scripts remain useful comparison sources; they are no
longer a runtime dependency of the new route. [The port guide](texttools/README.md)
records the source mapping and repairs, including filename case, multi-index
arguments, fenced-source handling, Pandoc escapes and child return codes.

`prepare` requires Pandoc. Full PDF generation requires XeLaTeX, `makeindex`,
Biber, `xdvipdfmx`, Bash, `stdbuf`, Inkscape, `sed`, and CREXX tools on `PATH`
for chapter splices (`crexx`, `rxc`, `rxas`, `rxlink`, `rxdas`, `rxdb`,
`rxcpack`, `rxvme`). The original TeX preamble/title pages also request Minion Pro, IBM Plex
Mono, Avenir Next, JuliaMono, TeX Gyre Pagella and Bodoni URW Light, plus TeX
packages such as `bashful`, `svg` and PSTricks. Executable discovery does not
establish font/package availability. cREXX has replaced the ooRexx requirement;
it does not replace Pandoc or TeX.

The detached preamble loads `setspace` and the first existing `longtable`
declaration before `hyperref`, so ordinary and table footnotes retain their
clickable PDF destinations. The authored preamble and package count are
preserved.

On 1 October 2026 the `initial` profile built all four beta 3 PDFs on macOS
ARM64 with Pandoc 3.11, TeX Live 2026 and fresh release-channel product
tools. The edition is stamped `crexx-1.0.0-beta.3`, from documentation source
`54eb6aa8bec320a8538377dbd2686c53cc0e7282`. Independent review checked every page for text outside its physical
boundary, representative rendered pages, logs, font embedding, indices,
references, and all 1,422 literal fenced snapshots (997 Rexx/cREXX colour
blocks and 425 retained other handlers). No source file changed during
generation. All 1,716 internal PDF links resolve, including all 39 ordinary/table
footnotes. The qualified PDFs are:

| Book | Pages | SHA-256 |
| --- | ---: | --- |
| Language reference | 430 | `ca0b6bb7658b0e836f1f2161947cb6327cde9ce0096636daaaddf0e4a655a488` |
| Programming guide | 260 | `efb026c8857da4bef9c9dd38ad4c55d1d276a1739cf75bfb05d8d38d4278176d` |
| VM specification | 361 | `91162253be742aa907b8d579a234fc7ca847defed09f208cb8fc888379407cb4` |
| Library reference | 546 | `b16821867b6e82b4680bac5a3cc95bd6a9bc3217936a6c0a45c3bc1480e88239` |

The current VM structure includes its checked-in generated instruction
chapter, whose live splices and 92 present guarded inputs were typeset.
That chapter is a partial historical view of today's opcode set. The separate
manual [binary_memory_instructions.md](books/crexx_vm_spec/binary_memory_instructions.md)
chapter remains excluded from the VM structure; its five stale example paths
were repaired to existing `examples/binary_*.rxas` files, but it was not
printed. The active VM splice uses the current product `rxvme` with embedded
core bytecode, and `byOpcode` matches
[byOpcode.crexx](books/crexx_vm_spec/byOpcode.crexx).

The native directory/file/process code has POSIX and Windows paths, but this
generation was executed only on macOS. Case-sensitive Linux and Windows book
builds and PDF layout under the selectable `original` font profile remain
unqualified. A matching original font installation would also be needed to
compare René's intended typography.

The original font choices, styles, publisher data and authored chapters remain
in source. The generator applies these substitutions only to detached TeX
inputs under `OUTPUT`, including the copied instruction chapter and generated
splices:

| Original active request | Initial profile |
| --- | --- |
| Minion Pro, Bodoni URW Light | TeX Gyre Pagella |
| Avenir Next, IBM Plex Sans, IBM Plex Sans Condensed | TeX Gyre Heros |
| IBM Plex Mono, JuliaMono | JuliaMono 0.63.2 |

The implementation uses TeX Gyre OTF and JuliaMono TTF filenames through TeX
Live's kpathsea lookup because this user-local XeTeX does not resolve the family
names through macOS font discovery. The fontspec declarations give explicit
bold and italic files where those faces are requested. Every JuliaMono request
sets `RawFeature=-calt` so source operators stay literal. GNU Unifont BMP/Upper
18.0.01 supplies a scoped monochrome fallback only for codepoints absent from
the selected face; JuliaMono's box drawing remains native. Authored characters
stay unchanged. Emoji sequences print
as monochrome component glyphs, without colour emoji ligatures. U+200D and
U+FE0F are default-ignorable joiner/presentation controls and have no visible
glyph in this profile. To restore René's
font choices, select `original` and install the original families on the
typesetting host. To change the substitutions, edit `applyfontprofile` in
`texttools/texttools.crexx` and the scoped `texttools/glyph-fallback.tex`; the authored TeX remains the reference. Original
font availability and layout equivalence have not been qualified here.

For this macOS qualification, TeX Live 2026 is installed in
`$HOME/.local/share/crexx-doc-tools/texlive/2026`, with the executable directory
`bin/universal-darwin`. Its selected collections include basic, latex,
latexrecommended, latexextra, fontsrecommended, xetex, pstricks, bibtexextra,
fontutils and langenglish. Pandoc 3.11, Inkscape 1.4.4 and Ghostscript 10.08.0
come from Homebrew; this macOS host supplies `/usr/bin/stdbuf`. On a macOS host
without these tools, install them with `brew install pandoc ghostscript` and
`brew install --cask inkscape`; provide `stdbuf` from the host or coreutils.
The TeX Live net installer can reproduce this user-local collection selection
with a custom profile (use the matching TeX Live 2026 installer/archive if the
current network installer has advanced to another release):

```sh
tex_install_work=$(mktemp -d /tmp/crexx-texlive.XXXXXX)
tex_install_root="$HOME/.local/share/crexx-doc-tools/texlive"
curl -fL https://mirror.ctan.org/systems/texlive/tlnet/install-tl-unx.tar.gz \
  -o "$tex_install_work/install-tl-unx.tar.gz"
tar -xzf "$tex_install_work/install-tl-unx.tar.gz" -C "$tex_install_work"
cat > "$tex_install_work/texlive.profile" <<EOF
selected_scheme scheme-custom
binary_universal-darwin 1
collection-basic 1
collection-latex 1
collection-latexrecommended 1
collection-latexextra 1
collection-fontsrecommended 1
collection-xetex 1
collection-pstricks 1
collection-bibtexextra 1
collection-fontutils 1
collection-langenglish 1
TEXDIR $tex_install_root/2026
TEXMFLOCAL $tex_install_root/texmf-local
TEXMFSYSCONFIG $tex_install_root/2026/texmf-config
TEXMFSYSVAR $tex_install_root/2026/texmf-var
TEXMFCONFIG $tex_install_root/2026/user-texmf-config
TEXMFVAR $tex_install_root/2026/user-texmf-var
TEXMFHOME $tex_install_root/2026/texmf-home
instopt_adjustpath 0
instopt_adjustrepo 0
tlpdbopt_install_docfiles 0
tlpdbopt_install_srcfiles 0
tlpdbopt_create_formats 1
EOF
tex_installer=$(find "$tex_install_work" -maxdepth 2 -name install-tl -type f -print -quit)
perl "$tex_installer" -profile "$tex_install_work/texlive.profile"
```

This is the collection/profile route used for this review; the retained
installation log and profile are in the documentation-review evidence. Put the
TeX binary directory and the current CREXX product
binary directory before older installed commands on `PATH`; the active VM
chapter uses `rxvme` with embedded core bytecode. Exact layout validation
of the original font profile requires those original fonts. The selected fonts for initial generation
are TeX Gyre Pagella for body/title serif text, TeX Gyre Heros for headings,
and JuliaMono 0.63.2 for code/terminal output. The
[TeX Gyre collection](https://ctan.org/pkg/tex-gyre) uses the GUST Font License
and is included in TeX Live; JuliaMono uses SIL OFL 1.1. Functional typesetting with the substitutes
does not establish equivalence to the original typography.

Some preserved PDF figures contain embedded Arial/Trebuchet fonts or standard
Helvetica/Courier resources. Those belong to the authored assets and add no
host font request or substitution by the initial text profile.

Install the [GNU Unifont 18.0.01 OTF files](https://unifoundry.com/unifont/)
into this TeX Live tree before using `initial` on another host:

```sh
texmf_local=$(kpsewhich -var-value=TEXMFLOCAL)
fallback_dir="$texmf_local/fonts/opentype/public/crexx-doc-fallback"
mkdir -p "$fallback_dir"
curl -fL https://unifoundry.com/pub/unifont/unifont-18.0.01/font-builds/unifont-18.0.01.otf -o "$fallback_dir/unifont-18.0.01.otf"
curl -fL https://unifoundry.com/pub/unifont/unifont-18.0.01/font-builds/unifont_upper-18.0.01.otf -o "$fallback_dir/unifont_upper-18.0.01.otf"
mktexlsr "$texmf_local"
kpsewhich unifont-18.0.01.otf
kpsewhich unifont_upper-18.0.01.otf
shasum -a 256 "$fallback_dir"/unifont*.otf
```

For the qualified inputs, the expected SHA-256 values are
`88d0a14d4aa9a96419720b39ba5da921a59560d76d4ec7d523acc06ed85cd3c2`
for the BMP OTF and
`472201e45a050bff4c7658208b8c73e34ebce3764b287a5a46c7774038ef9391`
for the Upper OTF. The binaries are host dependencies, not repository files.

Install the four selected [JuliaMono 0.63.2 TTF faces](https://github.com/cormullion/juliamono/releases/tag/v0.63.2)
and retain their SIL OFL licence in the same TeX-local tree:

```sh
julia_stage=$(mktemp -d /tmp/crexx-juliamono.XXXXXX)
julia_dir="$(kpsewhich -var-value=TEXMFLOCAL)/fonts/truetype/public/crexx-doc-juliamono"
mkdir -p "$julia_dir"
curl -fL https://github.com/cormullion/juliamono/releases/download/v0.63.2/JuliaMono-ttf.tar.gz \
  -o "$julia_stage/JuliaMono-ttf.tar.gz"
tar -xzf "$julia_stage/JuliaMono-ttf.tar.gz" -C "$julia_stage"
install -m 644 "$julia_stage"/JuliaMono-{Regular,RegularItalic,Bold,BoldItalic}.ttf "$julia_dir"
install -m 644 "$julia_stage/LICENSE" "$julia_dir/LICENSE"
mktexlsr "$(kpsewhich -var-value=TEXMFLOCAL)"
kpsewhich JuliaMono-Regular.ttf
shasum -a 256 "$julia_dir"/JuliaMono-{Regular,RegularItalic,Bold,BoldItalic}.ttf
```

The selected archive SHA-256 is
`be6517295198ec5c92bdbaad42f4f6f8d83f921d80512b79f54fe036add95c0c`;
the four file hashes and source are retained in the generation review's font
receipt. No font binaries are committed to this repository.

### Listings, execution and syntax highlighting

Other-language highlighting is performed by LaTeX's `listings` package. The preamble
loads [netrexxformat.tex](books/boilerplate/netrexxformat.tex),
[assemblerformat.tex](books/boilerplate/assemblerformat.tex) and
[rxasformat.tex](books/boilerplate/rxasformat.tex), with hand-maintained language,
keyword, string and comment rules. For `rexx` and `crexx` fences, the port
instead emits a separate `fvextra` Verbatim colour block from the exact render
snapshot, with the original listing caption and label. The lexical pass does
not invoke the compiler or obtain DSLSH token spans. Plain `text` fences use literal `VerbatimInput` so Unicode diagrams keep their
line structure; `bat` selects the installed `command.com` listings dialect.
Other programming-language fences retain `lstinputlisting`.

Fenced examples are extracted and printed. Explicit `<!--splice--...-->` tags
and authored TeX `splice` commands run during typesetting; not every listing is
executed. Many listings are incomplete fragments or use other languages.

The book-specific [bookhighlight.crexx](texttools/bookhighlight.crexx) uses
Peter Jacob's [Scanlex.crexx](../lib/classlib/Scanlex.crexx) as its token and
character-scanning starting point, with rules checked against the compiler
scanners. It classifies fragments without requiring complete syntax, imports,
semantic validation or execution. The generated colour block preserves every
source character and whitespace gap; incomplete strings/comments remain
printable. `Scanlex` and its JSON API are unchanged. The dedicated lexical
fixtures and real listing-set timing remain part of the generation review.

Parser-based highlighting remains an alternative. CREXX already exposes
`rxc --syntaxhighlight` over DSLSH; THE is an editor consumer rather than a
requirement of that interface. A direct `parser_tester` check on the retained
`hello.crexx` example succeeds and returns comment, keyword and string tokens
without THE. See [the current protocol/integration reference](../compiler/docs/dslsh_integration.md).
Parser mode can return source structure when syntax errors exist, skipping
semantic validation for that request; incomplete fragments therefore do not
rule out this route. A lexical pass would avoid parser and semantic-analysis
work for simple listing styles. Actual latency and fragment coverage still
need measurement. The book lexer does not change the DSLSH interface.

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

## Linux CI, cover versions and release assets

[Build CREXX](../.github/workflows/build.yml) invokes the reusable
[Linux book job](../.github/workflows/build-docs.yml) on normal develop/master
pushes, pull requests and future versioned tags. It checks out the exact source
SHA selected by version metadata and builds the actual embedded `rxvme` plus the
required compiler, assembler, linker, VM and helper tools. Generation and lexical
highlighting remain the repository's cREXX scripts; shell/Python helpers install
CI dependencies, launch the route and validate the complete PDF asset set.

Ubuntu 24.04 supplies XeLaTeX and the selected TeX Live packages, Biber,
makeindex, xdvipdfmx, Inkscape, Ghostscript and Poppler. Pandoc 3.11, JuliaMono
0.63.2 and GNU Unifont 18.0.01 downloads have pinned SHA-256 checksums in
[install-doc-tools-linux.sh](../scripts/install-doc-tools-linux.sh); downloaded
archives are cached. Ubuntu additionally needs the explicit
[`fonts-texgyre` package](https://packages.ubuntu.com/en/noble/fonts-texgyre) for
the Pagella/Heros OpenType faces; `texlive-fonts-recommended` alone does not
supply them. The installer checks all eight serif/sans faces through kpathsea.
Ubuntu's [`texlive-science` package](https://packages.ubuntu.com/noble/texlive-science)
supplies `siunitx`, required by the included PSTricks calculation packages;
the installer checks this file before building the tools or books.
On GitHub runners it replaces the observed slow Azure HTTP archive mirror with
Canonical's HTTPS archive; local developer mirror settings remain intact.
Package/tool versions are retained with each build. This
Linux distribution TeX installation differs from the qualified macOS TeX Live
2026 installation; its first real four-book result needs independent review.
Ubuntu's older `listings` catalogue lacks CMake. Detached output uses the
upstream CMake definition bundled with the port only when no native handler
exists; authored listings and newer installations' handlers remain intact.
Windows generation and original typography remain unqualified.

An optional sixth driver argument supplies the exact version token:

```sh
"$product/rxvm" "$port/generate-books-linked.rxbin" -a \
  build "$PWD" "$port/pdf-books" all initial \
  'crexx-1.0.0-beta.3+dev-snapshot.g0123456789ab'
```

The stamp is applied to staged cover and publication-data pages only. It is the
workflow's explicit `display_version`, not an older compiler's cached Git
suffix. Existing calls without the sixth argument preserve the authored date
and version-splice behavior. The CLI guide documents accepted version tokens.

The job validates all four PDFs, cover/publication-data stamps, final typesetting
logs, all extracted listing snapshots against authored Markdown, and unchanged
source hashes. Only a complete successful set is uploaded as
`documentation-release-asset`. The PDFs are `CREXX-TAG-language-reference.pdf`,
`CREXX-TAG-programming-guide.pdf`, `CREXX-TAG-vm-specification.pdf` and
`CREXX-TAG-library-reference.pdf`. `TAG` is `dev-snapshot` or the actual versioned
tag (including its `v`). `CREXX-TAG-docs.json` records source commit/version,
font profile, tool versions, page counts and PDF SHA-256 hashes. PRs retain the
assets in Actions without publishing a release. Logs, prepared TeX and literal
listing snapshots are retained as `linux-documentation-evidence` for 14 days.
No generated PDFs or historical CI runs enter HEAD.

As approved by Adrian, ordinary binary publication proceeds if document
generation fails. The workflow summary and release body report that no matching
PDFs are available. A refreshed development snapshot removes old PDF/manifest
assets rather than presenting documents from another commit. The existing
binary checks and latest-develop publication guard remain required.
[Deep Build QA](../.github/workflows/deep-build.yml) invokes the identical Linux
route with document failures fatal; it cannot record a qualified develop marker
unless the four books pass. GitHub runs scheduled workflows from its default
branch, currently `master`. Adrian authorised deploying the reusable job and
required Deep gate there on 1 October 2026; commit `7f33356264bd` activates that
scheduled route. The scheduled workflow resolves and checks out the actual
develop commit, including its generation scripts. This is an overnight
assurance requirement, not a reason to dispatch the entire Deep matrix for
ordinary documentation edits.

Current status: CI wiring and focused failure/asset checks pass independent review;
real hosted Linux PDFs and their first asset publication remain open in
[the authoritative plan](planning/document-generation.md#linux-ci-and-versioned-publication-follow-up--1-october-2026).
