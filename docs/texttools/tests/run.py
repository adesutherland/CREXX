#!/usr/bin/env python3
"""Compile/link the cREXX port and check conversion/orchestration in temp trees.

The external tool fixtures are deliberately fake: they qualify argv, directories
and failure handling, not PDF typesetting. --pandoc adds real book preparation.
No source file is changed and no publication/view option reaches a real service.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

REPO = Path(__file__).resolve().parents[3]
BOOKS = ("crexx_language_reference", "crexx_programming_guide",
         "crexx_vm_spec", "crexx_library_reference")


def staged_version(preamble):
    line = next(line for line in preamble.splitlines()
                if line.startswith("\\newcommand{\\BookBuildVersion}"))
    encoded = line.removeprefix(
        "\\newcommand{\\BookBuildVersion}{\\texttt{").removesuffix("}}")
    return encoded.replace("\\allowbreak{}", "").replace("\\_", "_")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bin", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--pandoc", type=Path)
    parser.add_argument("--footnote-pdf", action="store_true",
                        help="Prepare one real book and verify a XeLaTeX footnote PDF destination")
    parser.add_argument("--listings-legacy-catalog", type=Path,
                        help="Exercise the CMake fallback with a listings 1.9 catalog and native TeX")
    parser.add_argument("--layout-pdf", action="store_true",
                        help="Typeset the repaired literal help and VM path in one real PDF fixture")
    parser.add_argument("--layout-legacy-catalog", type=Path,
                        help="Select an older listings catalog for the real layout PDF fixture")
    args = parser.parse_args()
    product = args.bin.resolve()
    work = (args.work.resolve() if args.work else
            Path(tempfile.mkdtemp(prefix="crexx-texttools-tests-")))
    work.mkdir(parents=True, exist_ok=True)
    imports = work / "imports"
    imports.mkdir()
    receipts = []

    def run(label, command, expected=0, env=None, cwd=None):
        log = work / (label + ".log")
        with log.open("w") as stream:
            result = subprocess.run([str(x) for x in command], cwd=cwd or work,
                                    env=env, stdout=stream,
                                    stderr=subprocess.STDOUT, timeout=300)
        receipts.append({"label": label, "rc": result.returncode, "log": str(log)})
        good = result.returncode == expected if expected is not None else result.returncode != 0
        if not good:
            raise AssertionError(f"{label}: rc {result.returncode}; {log}\n{log.read_text()[-3000:]}")
        return log

    sources = {"bookhighlight": REPO / "docs/texttools/bookhighlight.crexx",
               "texttools": REPO / "docs/texttools/texttools.crexx",
               "build": REPO / "docs/texttools/build.crexx",
               "regression": REPO / "docs/texttools/tests/regression.crexx",
               "generate-books": REPO / "docs/generate-books.crexx"}
    for name, source in sources.items():
        stem = imports / name if name in ("bookhighlight", "texttools") else work / name
        run(name + "-compile", [product / "rxc", "-i", f"{imports};{product}",
                                "-o", stem, source])
        run(name + "-assemble", [product / "rxas", "-o", stem, stem])
        if name not in ("bookhighlight", "texttools"):
            run(name + "-link", [product / "rxlink", "-o", work / (name + "-linked"),
                                    stem.with_suffix(".rxbin"), imports / "texttools.rxbin",
                                    imports / "bookhighlight.rxbin",
                                    product / "library.rxbin"])
    run("regression", [product / "rxvm", work / "regression-linked.rxbin", "-a", work / "unit"])

    fixtures = work / "tools"
    fixtures.mkdir()
    stub = fixtures / "fixture-tool"
    stub.write_text(f"#!{sys.executable}\n" + r'''
import json, os, pathlib, sys
name = pathlib.Path(sys.argv[0]).name
with open(os.environ['TT_COMMANDS'], 'a') as log:
    log.write(json.dumps({'tool': name, 'argv': sys.argv[1:], 'cwd': os.getcwd()})+'\n')
if name == os.environ.get('TT_FAIL'): sys.exit(7)
if name == 'pandoc':
    pathlib.Path(sys.argv[sys.argv.index('-o')+1]).write_text(
        '\\%includesource=Example.crexx:rexx\\%\n')
if name == 'xelatex':
    pathlib.Path(sys.argv[-1]).with_suffix('.xdv').write_bytes(b'fixture xdv')
    pathlib.Path(sys.argv[-1]).with_suffix('.log').write_text('fixture converged\n')
if name == 'makeindex':
    rejected = 1 if os.environ.get('TT_INDEX_REJECT') else 0
    pathlib.Path(sys.argv[-1]).with_suffix('.ilg').write_text(
        f'done (1 entries accepted, {rejected} rejected).\n')
if name == 'xdvipdfmx' and not os.environ.get('TT_NO_PDF'):
    pathlib.Path(sys.argv[-1]).with_suffix('.pdf').write_bytes(b'%PDF-1.4\nFIXTURE ONLY\n')
''')
    stub.chmod(0o755)
    for name in ("pandoc", "xelatex", "makeindex", "biber", "xdvipdfmx",
                 "stdbuf", "inkscape", "scp", "open"):
        (fixtures / name).symlink_to(stub)
    env = os.environ.copy()
    env["PATH"] = str(fixtures) + os.pathsep + str(product) + os.pathsep + env.get("PATH", "")
    commands = work / "commands.jsonl"
    env["TT_COMMANDS"] = str(commands)
    source = work / "source with spaces"
    guard_dir = source / "docs/texttools"
    guard_dir.mkdir(parents=True)
    (guard_dir / "splice-guard.tex").write_bytes(
        (REPO / "docs/texttools/splice-guard.tex").read_bytes())
    (guard_dir / "glyph-fallback.tex").write_bytes(
        (REPO / "docs/texttools/glyph-fallback.tex").read_bytes())
    (guard_dir / "listings-cmake-compat.tex").write_bytes(
        (REPO / "docs/texttools/listings-cmake-compat.tex").read_bytes())
    boilerplate = source / "docs/books/boilerplate"
    boilerplate.mkdir(parents=True)
    (boilerplate / "preamble.tex").write_text(
        "\\usepackage{hyperref}\n\\usepackage{setspace}\n"
        "\\usepackage{fontspec}\n\\usepackage{longtable}\n"
        "\\usepackage{fancyvrb}\n"
        "\\usepackage{bashful}\n\\usepackage{listings}\n"
        "\\usepackage{longtable}\n\\usepackage{longtable}\n"
        "\\setmainfont[Mapping=tex-text]{Minion Pro}\n"
        "\\newfontfamily\\headingfont{Avenir Next}\n"
        "\\newfontfamily\\codefont{IBM Plex Mono}\n")
    (boilerplate / "bookmeta.tex").write_text(
        "Content is up to date with version \\emph{\\splice{rxc -v}}\n")
    for book in BOOKS:
        folder = source / "docs/books" / book
        folder.mkdir(parents=True)
        (folder / "MixedCase.md").write_text("# Example\n```rexx <!--Example.crexx-->\nsay 'ok'\n```\n")
        (folder / "structure.tex").write_text(r"\input{MixedCase}" + "\n")
        if book == "crexx_vm_spec":
            (folder / "instruction_chapter.tex").write_text(
                r"\IfFileExists{example.rxas}{\obeylines \begin{terminaloutput} \splice{rxvme example} \end{terminaloutput}}{}" + "\n" + r"\includesvg{../../svg/cnop.gv}" + "\n")
        (folder / (book + ".tex")).write_text(
            "\\title{\\fontspec{Bodoni URW\n    Light}Title}\n"
            "\\date{\\null\\hfill \\today}\n")
        old = folder / "tex/book"
        old.mkdir(parents=True)
        (old / (book + ".pdf")).write_bytes(b"old PDF must not be copied")
    (source / "docs/instructions").mkdir()
    canonical = source / "canonical.md"
    canonical.write_text("# Linked source\n")
    for book in BOOKS:
        (source / "docs/books" / book / "LinkedGuide.md").symlink_to(canonical)
    before = {str(p): hashlib.sha256(p.read_bytes()).hexdigest()
              for p in source.rglob("*") if p.is_file()}
    wrapper = [product / "rxvm", work / "generate-books-linked.rxbin", "-a"]
    run("check", wrapper + ["check", source, work / "check output", "all"], env=env)
    assert not (work / "check output").exists()
    run("four-books", wrapper + ["build", source, work / "four books", "all"], env=env)
    staged = work / "four books/docs/books"
    preamble = (staged / "boilerplate/preamble.tex").read_text()
    assert preamble.index("\\usepackage{setspace}") < preamble.index("\\usepackage{longtable}")
    assert preamble.index("\\usepackage{longtable}") < preamble.index("\\usepackage{hyperref}")
    assert preamble.count("\\usepackage{longtable}") == 3
    assert "\\usepackage{fvextra}" in preamble
    assert "\\tracinglostchars=3" in preamble
    assert "\\lstset{indexstyle=\\BookIndex}" in preamble
    assert "\\input{../../../boilerplate/splice-guard}" in preamble
    assert "\\input{../../../boilerplate/glyph-fallback}" in preamble
    assert (staged / "boilerplate/splice-guard.tex").read_bytes() == (
        guard_dir / "splice-guard.tex").read_bytes()
    assert (staged / "boilerplate/glyph-fallback.tex").read_bytes() == (
        guard_dir / "glyph-fallback.tex").read_bytes()
    assert (staged / "boilerplate/listings-cmake-compat.tex").read_bytes() == (
        guard_dir / "listings-cmake-compat.tex").read_bytes()
    assert "\\@ifundefined{lstlang@cmake$}" in preamble
    assert "\\input{../../../boilerplate/listings-cmake-compat}" in preamble
    assert "texgyrepagella-regular.otf" in preamble
    assert "texgyreheros-regular.otf" in preamble
    assert "\\setsansfont[" in preamble
    assert "\\spliceliteral{rxvme example}" in (
        staged / "crexx_vm_spec/instruction_chapter.tex").read_text()
    assert "\\begin{bookterminaloutput}" in (
        staged / "crexx_vm_spec/instruction_chapter.tex").read_text()
    assert "\\includesvg[width=\\linewidth]{../../svg/cnop.gv}" in (
        staged / "crexx_vm_spec/instruction_chapter.tex").read_text()
    assert "JuliaMono-Regular.ttf" in preamble and "RawFeature=-calt" in preamble
    assert "Minion Pro" not in preamble
    assert "texgyrepagella-regular.otf" in (
        staged / BOOKS[0] / (BOOKS[0] + ".tex")).read_text()
    assert "\\BookBuildVersion" not in preamble, "Legacy invocation changed its preamble"
    assert "\\splice{rxc -v}" in (staged / "boilerplate/bookmeta.tex").read_text()
    for book in BOOKS:
        assert "\\date{\\null\\hfill \\today}" in (staged / book / (book + ".tex")).read_text()
    development_version = "crexx-1.0.0-beta.3+dev-snapshot.g0123456789ab"
    run("versioned-all-prepare", wrapper + ["prepare", source,
        work / "versioned all", "all", "initial", development_version], env=env)
    versioned = work / "versioned all/docs/books"
    versioned_preamble = (versioned / "boilerplate/preamble.tex").read_text()
    assert "\\newcommand{\\BookBuildVersion}" in versioned_preamble
    assert staged_version(versioned_preamble) == development_version
    versioned_meta = (versioned / "boilerplate/bookmeta.tex").read_text()
    assert "\\emph{\\BookBuildVersion}" in versioned_meta
    assert "\\splice{rxc -v}" not in versioned_meta
    for book in BOOKS:
        title = (versioned / book / (book + ".tex")).read_text()
        assert "Build version: \\BookBuildVersion" in title, book
        assert "\\date{\\null\\hfill \\today" in title, book
    run("versioned-tag-prepare", wrapper + ["prepare", source,
        work / "versioned tag", BOOKS[0], "initial", "crexx-1.0.0-beta.3"], env=env)
    assert staged_version((
        work / "versioned tag/docs/books/boilerplate/preamble.tex").read_text()
        ) == "crexx-1.0.0-beta.3"
    run("versioned-underscore-prepare", wrapper + ["prepare", source,
        work / "versioned underscore", BOOKS[0], "initial", "crexx-1.0.0+dev_snapshot.g0123456789ab"], env=env)
    underscore_preamble = (
        work / "versioned underscore/docs/books/boilerplate/preamble.tex").read_text()
    assert "dev\\_\\allowbreak{}snapshot" in underscore_preamble
    assert staged_version(underscore_preamble) == "crexx-1.0.0+dev_snapshot.g0123456789ab"
    for index, invalid in enumerate(("", "bad version", "bad%version", "bad&version",
                                     "bad#version", "bad\\version", "-bad", "a" * 97)):
        bad_output = work / f"invalid-version-{index}"
        run(f"invalid-version-{index}", wrapper + ["prepare", source,
            bad_output, BOOKS[0], "initial", invalid], expected=2, env=env)
        assert not bad_output.exists(), invalid
    calls = [json.loads(line) for line in commands.read_text().splitlines()]
    for book in BOOKS:
        built = work / "four books/docs/books" / book / "tex/book"
        received = [c["tool"] for c in calls if Path(c["cwd"]).resolve() == built.resolve()]
        assert received == ["pandoc", "pandoc", "xelatex", "makeindex", "biber",
                            "xelatex", "makeindex", "biber", "xelatex", "xdvipdfmx"], received
        assert (built / "Example.crexx").read_text() == "say 'ok'\n"
        assert (built / "MixedCase-code-1.txt").read_text() == "say 'ok'\n"
        assert "\\begin{Verbatim}" in (built / "MixedCase-code-1.txt.highlight.tex").read_text()
        assert not (built.parent.parent / "LinkedGuide.md").is_symlink()
    assert not any(c["tool"] in ("scp", "open") for c in calls)
    for tool in ("pandoc", "xelatex", "makeindex", "biber", "xdvipdfmx"):
        failed_env = dict(env, TT_FAIL=tool)
        start = len(commands.read_text().splitlines())
        run("failed-" + tool, wrapper + ["build", source, work / ("failed-" + tool), BOOKS[0]],
            expected=None, env=failed_env)
        failed_calls = [json.loads(line) for line in commands.read_text().splitlines()[start:]]
        assert failed_calls[-1]["tool"] == tool, "Pipeline continued after failure"
    run("missing-pdf", wrapper + ["build", source, work / "missing-pdf", BOOKS[0]],
        expected=None, env=dict(env, TT_NO_PDF="1"))
    run("rejected-index", wrapper + ["build", source, work / "rejected-index", BOOKS[0]],
        expected=None, env=dict(env, TT_INDEX_REJECT="1"))
    run("original-fonts", wrapper + ["prepare", source, work / "original-fonts",
                                      BOOKS[0], "original"], env=env)
    original_staged = work / "original-fonts/docs/books"
    assert "Minion Pro" in (original_staged / "boilerplate/preamble.tex").read_text()
    assert "Bodoni URW\n    Light" in (
        original_staged / BOOKS[0] / (BOOKS[0] + ".tex")).read_text()
    assert "\\usepackage{fvextra}" in (
        original_staged / "boilerplate/preamble.tex").read_text()
    original_preamble = (original_staged / "boilerplate/preamble.tex").read_text()
    assert original_preamble.index("\\usepackage{setspace}") < original_preamble.index(
        "\\usepackage{longtable}") < original_preamble.index("\\usepackage{hyperref}")
    assert original_preamble.count("\\usepackage{longtable}") == 3
    assert "\\input{../../../boilerplate/glyph-fallback}" not in (
        original_staged / "boilerplate/preamble.tex").read_text()
    run("original-fonts-versioned", wrapper + ["prepare", source,
        work / "original-fonts-versioned", BOOKS[0], "original",
        "crexx-1.0.0-beta.3"], env=env)
    original_versioned = work / "original-fonts-versioned/docs/books"
    assert "Minion Pro" in (original_versioned / "boilerplate/preamble.tex").read_text()
    assert "\\newcommand{\\BookBuildVersion}" in (
        original_versioned / "boilerplate/preamble.tex").read_text()
    assert "Build version: \\BookBuildVersion" in (
        original_versioned / BOOKS[0] / (BOOKS[0] + ".tex")).read_text()
    run("existing-output", wrapper + ["build", source, work / "four books", BOOKS[0]], expected=2, env=env)
    run("inside-source", wrapper + ["build", source, source / "bad-output", BOOKS[0]], expected=2, env=env)
    alias = work / "source-alias"
    alias.symlink_to(source, target_is_directory=True)
    run("inside-source-alias", wrapper + ["build", source, alias / "bad-output", BOOKS[0]], expected=2, env=env)
    run("unknown-book", wrapper + ["build", source, work / "unknown", "unknown"], expected=2, env=env)
    run("unknown-font-profile", wrapper + ["prepare", source,
                                          work / "unknown-font-profile",
                                          BOOKS[0], "unknown"], expected=2, env=env)
    empty_env = dict(env, PATH=str(product))
    run("missing-tools", wrapper + ["check", source, work / "missing-tools", "all"], expected=1, env=empty_env)
    assert not (work / "missing-tools").exists()
    empty_repo = work / 'empty repo'
    empty_repo.mkdir()
    run("missing-input", wrapper + ["prepare", empty_repo, work / "missing-input", "all"], expected=1, env=env)
    assert not (work / "missing-input").exists()
    after = {str(p): hashlib.sha256(p.read_bytes()).hexdigest()
             for p in source.rglob("*") if p.is_file()}
    assert before == after, "Source fixture changed"

    # The upstream options remain explicit; their fixtures never publish/view.
    destination = work / "cloud destination"
    destination.mkdir()
    run("explicit-options", [product / "rxvm", work / "build-linked.rxbin", "-a",
                              "build", source / "docs/books" / BOOKS[0],
                              work / "option build", BOOKS[0], "-copy", "-crexx", "-show"],
        env=dict(env, CLOUDDRIVE=str(destination)))
    assert (destination / (BOOKS[0] + ".pdf")).is_file()
    run("stale-pdf", [product / "rxvm", work / "build-linked.rxbin", "-a",
                      "build", source / "docs/books" / BOOKS[0],
                      work / "option build", BOOKS[0]],
        expected=None, env=dict(env, TT_NO_PDF="1"))
    assert not (work / "option build" / (BOOKS[0] + ".pdf")).exists()

    # Optional real Markdown -> TeX check; no typesetter is involved here.
    if args.pandoc:
        authored_layout = {
            path: hashlib.sha256((REPO / path).read_bytes()).hexdigest()
            for path in ("docs/books/crexx_programming_guide/rxcpack.md",
                         "docs/ai-context/CREXX_ARCHITECTURE.md")}
        real_tools = work / "real-tools"
        real_tools.mkdir()
        (real_tools / "pandoc").symlink_to(args.pandoc.resolve())
        real_env = dict(env, PATH=str(real_tools) + os.pathsep + os.environ.get("PATH", ""))
        run("actual-books-prepare", wrapper + ["prepare", REPO, work / "actual books", "all"], env=real_env)
        for book in BOOKS:
            folder = work / "actual books/docs/books" / book
            chapters = sorted(p.stem for p in folder.glob("*.md") if p.is_file())
            assert all((folder / "tex/book" / (c + ".tex")).is_file() for c in chapters)
            extracted = 0
            for chapter in chapters:
                generated = folder / "tex/book"
                tags = re.findall(r"%includesource=(.*?):(.*?):(.*?)%", (generated / (chapter + ".md")).read_text())
                tex = (generated / (chapter + ".tex")).read_text()
                for _name, language, filename in tags:
                    assert (generated / filename).is_file(), filename
                    if language.lower() in ("rexx", "crexx"):
                        assert (generated / (filename + ".highlight.tex")).is_file()
                        assert "\\input{" + filename + ".highlight.tex}" in tex, (
                            f"Pandoc lexical listing path changed: {filename}")
                    elif language.lower() == "text":
                        assert "\\VerbatimInput[" in tex and (
                            "{\\detokenize{" + filename + "}}" in tex), (
                            f"Pandoc literal text path changed: {filename}")
                    else:
                        assert "\\lstinputlisting[" in tex and (
                            "{\\detokenize{" + filename + "}}" in tex), (
                            f"Pandoc listing path changed: {filename}")
                    extracted += 1
                assert r"\%includesource" not in tex
                assert r"\%splice\%" not in tex
            receipts.append({"book": book, "real_pandoc_chapters": len(chapters)})
            receipts.append({"book": book, "extracted_listings": extracted})
        program_tex = work / "actual books/docs/books/crexx_programming_guide/tex/book"
        rxcpack = (program_tex / "rxcpack.tex").read_text()
        assert rxcpack.count("\\begin{bookterminaloutput}") == 1
        assert rxcpack.count("\\end{bookterminaloutput}") == 1
        assert rxcpack.count("\\spliceliteral{rxcpack -h}") == 1
        assert "\\obeylines" not in rxcpack and "sed 's/" not in rxcpack
        architecture = (work / "actual books/docs/books/crexx_vm_spec/tex/book/"
                        "CREXX_ARCHITECTURE.tex").read_text()
        vm_path_token = ("\\texttt{performance/\\allowbreak{}UNICODE-"
                         "\\allowbreak{}CERT-\\allowbreak{}01-"
                         "\\allowbreak{}WORKLIST.md}")
        assert vm_path_token in architecture
        assert "\\nolinkurl{performance/UNICODE-CERT-01-WORKLIST.md}" not in architecture
        assert "\\texttt{performance/UNICODE-CERT-01-WORKLIST.md}" not in architecture
        assert authored_layout == {
            path: hashlib.sha256((REPO / path).read_bytes()).hexdigest()
            for path in authored_layout}, "Authored layout source changed during preparation"
        # The exact directive guard must fail if a future Pandoc/source edit
        # leaves the old raw splice in a slightly different context.
        probe_source = work / "layout-guard.crexx"
        probe_source.write_text("options levelb\nimport texttools\n"
                                "arg args = .string[]\n"
                                "return texttools..preprocessTEX(args[1], args[2])\n")
        run("layout-guard-compile", [product / "rxc", "-i", f"{imports};{product}",
                                     "-o", work / "layout-guard", probe_source])
        run("layout-guard-assemble", [product / "rxas", "-o", work / "layout-guard",
                                      work / "layout-guard"])
        run("layout-guard-link", [product / "rxlink", "-o", work / "layout-guard-linked",
                                  work / "layout-guard.rxbin", imports / "texttools.rxbin",
                                  imports / "bookhighlight.rxbin", product / "library.rxbin"])
        malformed = work / "malformed-rxcpack/rxcpack.texin"
        malformed.parent.mkdir()
        valid_input = (program_tex / "rxcpack.texin").read_text()
        old = "\\obeylines \\splice{rxcpack -h | sed "
        assert old in valid_input
        malformed.write_text(valid_input.replace(old, " " + old, 1))
        bad_log = run("layout-guard-malformed", [
            product / "rxvm", work / "layout-guard-linked.rxbin", "-a",
            malformed, work / "malformed-rxcpack/output.tex"],
            expected=None, env=real_env).read_text()
        assert "Unexpected rxcpack help splice context" in bad_log
        assert not (work / "malformed-rxcpack/output.tex").exists()
    if args.footnote_pdf:
        real_env = dict(os.environ, PATH=str(product) + os.pathsep + os.environ.get("PATH", ""))
        source_preamble = REPO / "docs/books/boilerplate/preamble.tex"
        source_bytes = source_preamble.read_bytes()
        actual = work / "actual footnote book"
        run("footnote-book-prepare", wrapper + [
            "prepare", REPO, actual, BOOKS[1], "initial"], env=real_env)
        assert source_preamble.read_bytes() == source_bytes
        staged_preamble = (actual / "docs/books/boilerplate/preamble.tex").read_text()
        assert staged_preamble.index("\\usepackage{setspace}") < staged_preamble.index(
            "\\usepackage{longtable}") < staged_preamble.index("\\usepackage{hyperref}")
        assert staged_preamble.count("\\usepackage{longtable}") == (
            source_bytes.decode().count("\\usepackage{longtable}"))
        fixture = actual / "docs/books" / BOOKS[1] / "tex/book"
        (fixture / "footnote-fixture.tex").write_text(r"""\input{../../../boilerplate/preamble}
\begin{document}
Body marker\footnote{Retained readable footnote text.}
\begin{longtable}{p{0.2\linewidth}p{0.6\linewidth}}
One & Table one\footnote{First retained table note.} \\
Two & Table two\footnote{Second retained table note.} \\
Three & Table three\footnote{Third retained table note.} \\
Four & Table four\footnote{Fourth retained table note.} \\
\end{longtable}
\end{document}
""")
        run("footnote-xelatex", ["xelatex", "-no-pdf", "-halt-on-error",
                                  "-interaction=nonstopmode", "footnote-fixture.tex"],
            env=real_env, cwd=fixture)
        run("footnote-xdvipdfmx", ["xdvipdfmx", "-o", "footnote-fixture.pdf",
                                    "footnote-fixture.xdv"], env=real_env, cwd=fixture)
        dests = run("footnote-destinations", ["pdfinfo", "-dests",
                    "footnote-fixture.pdf"], env=real_env, cwd=fixture).read_text()
        for index in range(1, 6):
            assert f'"Hfootnote.{index}"' in dests, (
                f"Footnote {index} has no PDF destination")
        receipts.append({"real_footnote_pdf": str(fixture / "footnote-fixture.pdf"),
                         "named_destinations": [f"Hfootnote.{i}" for i in range(1, 6)]})
    if args.layout_pdf:
        if not args.pandoc:
            raise ValueError("--layout-pdf also requires --pandoc")
        import pdfplumber
        real_env = dict(os.environ, PATH=str(product) + os.pathsep +
                        os.environ.get("PATH", ""))
        staged = work / "actual books/docs/books"
        tree = work / "layout-pdf"
        boilerplate = tree / "docs/books/boilerplate"
        shutil.copytree(staged / "boilerplate", boilerplate)
        fixture = tree / "docs/books/crexx_programming_guide/tex/book"
        fixture.mkdir(parents=True)
        rxcpack = (staged / "crexx_programming_guide/tex/book/rxcpack.tex").read_text()
        architecture = (staged / "crexx_vm_spec/tex/book/"
                        "CREXX_ARCHITECTURE.tex").read_text()
        vm_path_token = ("\\texttt{performance/\\allowbreak{}UNICODE-"
                         "\\allowbreak{}CERT-\\allowbreak{}01-"
                         "\\allowbreak{}WORKLIST.md}")
        vm_path_line = next(line for line in architecture.splitlines()
                            if vm_path_token in line)
        (fixture / "rxcpack.tex").write_text(rxcpack)
        version = "crexx-1.0.0-beta.3+dev-snapshot.gdf9bf84d7b65"
        help_text = run("layout-actual-rxcpack-help", [product / "rxcpack", "-h"],
                        env=real_env).read_text()
        assert help_text.count("Version : ") == 1
        help_text = re.sub(r"(?m)^Version : .*$", "Version : " + version, help_text)
        (fixture / "rxcpack-help.txt").write_text(help_text)
        tool_dir = fixture / "tools"
        tool_dir.mkdir()
        shim = tool_dir / "rxcpack"
        shim.write_text("#!/bin/sh\n"
                        "test \"$1\" = \"-h\" || exit 2\n"
                        "cat ./rxcpack-help.txt\n")
        shim.chmod(0o755)
        fixture_tex = (
            "\\input{../../../boilerplate/preamble}\n"
            "\\begin{document}\n\\large\n"
            "\\input{rxcpack.tex}\n"
            "\\section*{VM architecture path}\n"
            "\\begin{itemize}\n\\tightlist\n\\item\n"
            + vm_path_line + "\n"
            "\\end{itemize}\n"
            "\\section*{Narrow path stress}\n"
            "\\begin{minipage}{0.38\\linewidth}\n"
            "\\begin{itemize}\n\\item " + vm_path_token + "\n"
            "\\end{itemize}\n\\end{minipage}\n"
            "\\end{document}\n")
        assert fixture_tex.count(vm_path_token) == 2
        (fixture / "layout-fixture.tex").write_text(fixture_tex)
        baseline_path = "\\nolinkurl{performance/UNICODE-CERT-01-WORKLIST.md}"
        (fixture / "layout-baseline.tex").write_text(
            fixture_tex.replace(vm_path_token, baseline_path, 1))
        tex_env = dict(real_env, PATH=str(tool_dir) + os.pathsep + real_env["PATH"])
        if args.layout_legacy_catalog:
            old_catalog = args.layout_legacy_catalog.resolve()
            assert (old_catalog / "lstlang2.sty").is_file(), old_catalog
            tex_env["TEXINPUTS"] = str(old_catalog) + os.pathsep + (
                real_env.get("TEXINPUTS", ""))
            resolved_catalog = run("layout-catalog-lookup",
                ["kpsewhich", "lstlang2.sty"], env=tex_env, cwd=fixture
                ).read_text().strip()
            assert Path(resolved_catalog).resolve() == old_catalog / "lstlang2.sty"
        baseline_log = run("layout-baseline-xelatex", [
            "xelatex", "-no-pdf", "-shell-escape", "-halt-on-error",
            "-interaction=nonstopmode", "layout-baseline.tex"],
            env=tex_env, cwd=fixture).read_text()
        baseline_overfull = [float(width) for width in re.findall(
            r"Overfull \\hbox \(([\d.]+)pt too wide\) in paragraph",
            baseline_log)]
        assert any(width > 50 for width in baseline_overfull), baseline_overfull
        run("layout-baseline-xdvipdfmx", ["xdvipdfmx", "-o",
            "layout-baseline.pdf", "layout-baseline.xdv"], env=tex_env, cwd=fixture)
        with pdfplumber.open(fixture / "layout-baseline.pdf") as baseline_pages:
            baseline_outside = [ch for page in baseline_pages.pages for ch in page.chars
                                if ch["x0"] < -0.01 or ch["x1"] > page.width + 0.01
                                or ch["top"] < -0.01 or ch["bottom"] > page.height + 0.01]
        assert baseline_outside, "Old URL path failed to reproduce the physical clip"
        log = run("layout-xelatex", ["xelatex", "-no-pdf", "-shell-escape", "-halt-on-error",
                                     "-interaction=nonstopmode", "layout-fixture.tex"],
                  env=tex_env, cwd=fixture).read_text()
        repaired_overfull = [float(width) for width in re.findall(
            r"Overfull \\hbox \(([\d.]+)pt too wide\) in paragraph", log)]
        assert all(width < 20 for width in repaired_overfull), repaired_overfull
        assert "Missing character:" not in log
        run("layout-xdvipdfmx", ["xdvipdfmx", "-o", "layout-fixture.pdf",
                                  "layout-fixture.xdv"], env=tex_env, cwd=fixture)
        pdf = fixture / "layout-fixture.pdf"
        with pdfplumber.open(pdf) as pages:
            outside = [(i + 1, ch["text"], ch["x0"], ch["x1"], ch["top"], ch["bottom"])
                       for i, page in enumerate(pages.pages) for ch in page.chars
                       if ch["x0"] < -0.01 or ch["x1"] > page.width + 0.01
                       or ch["top"] < -0.01 or ch["bottom"] > page.height + 0.01]
            extracted = "\n".join(page.extract_text(layout=False) or ""
                                  for page in pages.pages)
            page_count = len(pages.pages)
        assert not outside, outside[:10]
        compact = re.sub(r"\s+", " ", extracted)
        for line in help_text.splitlines():
            assert re.sub(r"\s+", " ", line) in compact, line
        assert ("performance/UNICODE-CERT-01-WORKLIST.md" in
                re.sub(r"\s+", "", extracted)), "VM path text changed across wrap"
        assert "\\&" not in compact and "Copyright & License Details" in compact
        page_pattern = fixture / "layout-page-%d.png"
        run("layout-raster", ["gs", "-q", "-dNOPAUSE", "-dBATCH",
                              "-sDEVICE=png16m", "-r120", "-o", page_pattern, pdf],
            env=tex_env, cwd=fixture)
        pages = sorted(fixture.glob("layout-page-*.png"))
        assert len(pages) == page_count, pages
        receipts.append({"layout_pdf": str(pdf), "pages": page_count,
                         "physical_outside_characters": len(outside),
                         "old_url_baseline_pdf": str(fixture / "layout-baseline.pdf"),
                         "old_url_baseline_outside_characters": len(baseline_outside),
                         "old_url_baseline_overfull_pt": baseline_overfull,
                         "repaired_overfull_pt": repaired_overfull,
                         "version": version,
                         "literal_help_sha256": hashlib.sha256(help_text.encode()).hexdigest(),
                         "pdf_sha256": hashlib.sha256(pdf.read_bytes()).hexdigest(),
                         "raster_pages": [str(page) for page in pages],
                         "listings_catalog": str(args.layout_legacy_catalog.resolve())
                             if args.layout_legacy_catalog else "installed default"})
    if args.listings_legacy_catalog:
        if not args.pandoc:
            raise ValueError("--listings-legacy-catalog also requires --pandoc")
        old_catalog = args.listings_legacy_catalog.resolve()
        assert (old_catalog / "lstlang2.sty").is_file(), old_catalog
        source_preamble = REPO / "docs/books/boilerplate/preamble.tex"
        source_bytes = source_preamble.read_bytes()
        real_tools = work / "cmake-real-tools"
        real_tools.mkdir()
        (real_tools / "pandoc").symlink_to(args.pandoc.resolve())
        real_env = dict(os.environ, PATH=str(real_tools) + os.pathsep +
                        str(product) + os.pathsep + os.environ.get("PATH", ""))
        actual = work / "cmake-actual-book"
        run("cmake-book-prepare", wrapper + [
            "prepare", REPO, actual, BOOKS[1], "initial"], env=real_env)
        assert source_preamble.read_bytes() == source_bytes
        staged = actual / "docs/books"
        staged_boilerplate = staged / "boilerplate"
        assert (staged_boilerplate / "listings-cmake-compat.tex").read_bytes() == (
            REPO / "docs/texttools/listings-cmake-compat.tex").read_bytes()
        source_md = (REPO / "docs/books" / BOOKS[1] / "rxpa.md").read_text()
        fence = chr(96) * 3
        authored_cmake = source_md.split(fence + "cmake\n", 1)[1].split(
            "\n" + fence, 1)[0] + "\n"
        snapshot = staged / BOOKS[1] / "tex/book/rxpa-code-1.txt"
        assert snapshot.read_text() == authored_cmake
        probe = ('# CMake comment: keep % & ' + chr(92) + ' exactly\n'
                 'set(_message "double # inside quoted string")\n'
                 "set(_single 'single # inside quoted string')\n"
                 "add_library(sample STATIC sample.c)\n")
        outputs = {}
        guard_open = "\\makeatletter\n\\@ifundefined{lstlang@cmake$}{\n"
        for name in ("legacy-1.9", "native-1.11",
                     "native-baseline-1.11", "registered-1.9"):
            tree = work / ("cmake-" + name)
            boilerplate = tree / "docs/books/boilerplate"
            shutil.copytree(staged_boilerplate, boilerplate)
            if name == "native-baseline-1.11":
                preamble_path = boilerplate / "preamble.tex"
                preamble = preamble_path.read_text()
                start = preamble.index(guard_open)
                end = preamble.index("\\makeatother", start) + len("\\makeatother")
                preamble_path.write_text(preamble[:start] + preamble[end:])
            if name == "registered-1.9":
                preamble_path = boilerplate / "preamble.tex"
                preamble = preamble_path.read_text()
                registered = (
                    "\\lstdefinelanguage{CMake}{morekeywords={RegisteredSentinel},"
                    "morecomment=[l]\\#,morestring=[b]\"}\n"
                    "\\makeatletter\n"
                    "\\edef\\BookCMakeBefore{\\expandafter\\meaning"
                    "\\csname lstlang@cmake$\\endcsname}\n"
                    "\\makeatother\n")
                checked = (
                    "\\makeatletter\n"
                    "\\edef\\BookCMakeAfter{\\expandafter\\meaning"
                    "\\csname lstlang@cmake$\\endcsname}\n"
                    "\\ifx\\BookCMakeBefore\\BookCMakeAfter\n"
                    "\\typeout{CMAKE-REGISTERED-PRESERVED}\n"
                    "\\else\\errmessage{CMake handler changed}\\fi\n"
                    "\\makeatother\n")
                start = preamble.index(guard_open)
                end = preamble.index("\\makeatother", start) + len("\\makeatother")
                preamble_path.write_text(
                    preamble[:start] + registered + preamble[start:end] +
                    "\n" + checked + preamble[end:])
            fixture = tree / "docs/books" / BOOKS[1] / "tex/book"
            fixture.mkdir(parents=True)
            (fixture / "rxpa-code-1.txt").write_bytes(snapshot.read_bytes())
            (fixture / "cmake-lexical-probe.txt").write_text(probe)
            (fixture / "cmake-compat-fixture.tex").write_text(
                "\\input{../../../boilerplate/preamble}\n"
                "\\begin{document}\n"
                "\\lstset{basicstyle=\\ttfamily\\small,"
                "keywordstyle=\\color{nrblue},commentstyle=\\color{nrgreen},"
                "stringstyle=\\color{nrorange}}\n"
                "\\lstinputlisting[language=cmake,label=rxpa-code-1.txt,"
                "caption=rxpa-code-1.txt]{\\detokenize{rxpa-code-1.txt}}\n"
                "\\lstinputlisting[language=cmake,label=cmake-lexical-probe.txt,"
                "caption=cmake-lexical-probe.txt]"
                "{\\detokenize{cmake-lexical-probe.txt}}\n"
                "\\end{document}\n")
            tex_env = dict(real_env)
            if name in ("legacy-1.9", "registered-1.9"):
                tex_env["TEXINPUTS"] = str(old_catalog) + os.pathsep + (
                    real_env.get("TEXINPUTS", ""))
            log = run("cmake-" + name + "-xelatex", [
                "xelatex", "-no-pdf", "-halt-on-error",
                "-interaction=nonstopmode", "cmake-compat-fixture.tex"],
                env=tex_env, cwd=fixture).read_text()
            loaded_fallback = "(../../../boilerplate/listings-cmake-compat.tex)" in log
            assert loaded_fallback == (name == "legacy-1.9"), name
            if name == "registered-1.9":
                assert "CMAKE-REGISTERED-PRESERVED" in log
            run("cmake-" + name + "-xdvipdfmx", [
                "xdvipdfmx", "-o", "cmake-compat-fixture.pdf",
                "cmake-compat-fixture.xdv"], env=tex_env, cwd=fixture)
            text_log = run("cmake-" + name + "-text", [
                "gs", "-q", "-dNOPAUSE", "-dBATCH", "-sDEVICE=txtwrite",
                "-sOutputFile=-", "cmake-compat-fixture.pdf"],
                env=tex_env, cwd=fixture).read_text()
            for line in (authored_cmake + probe).splitlines():
                assert line in text_log, (name, line)
            page = fixture / "cmake-compat-page-1.png"
            run("cmake-" + name + "-raster", [
                "gs", "-q", "-dNOPAUSE", "-dBATCH", "-sDEVICE=png16m",
                "-r100", "-o", page, "cmake-compat-fixture.pdf"],
                env=tex_env, cwd=fixture)
            outputs[name] = {"pdf": str(fixture / "cmake-compat-fixture.pdf"),
                             "text_sha256": hashlib.sha256(
                                 text_log.encode()).hexdigest(),
                             "page_sha256": hashlib.sha256(
                                 page.read_bytes()).hexdigest()}
        comparable = [outputs[k] for k in (
            "legacy-1.9", "native-1.11", "native-baseline-1.11")]
        assert len({v["text_sha256"] for v in comparable}) == 1, outputs
        assert len({v["page_sha256"] for v in comparable}) == 1, outputs
        receipts.append({"cmake_listings_real_fixture": outputs,
                         "legacy_catalog": str(old_catalog),
                         "authored_cmake_sha256": hashlib.sha256(
                             authored_cmake.encode()).hexdigest()})
    (work / "results.json").write_text(json.dumps(receipts, indent=2) + "\n")
    print(f"TextTools checks passed. Receipts: {work / 'results.json'}")
    print("Mock typesetter checks do not qualify PDF output.")


if __name__ == "__main__":
    main()
