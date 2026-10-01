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
import subprocess
import sys
import tempfile

REPO = Path(__file__).resolve().parents[3]
BOOKS = ("crexx_language_reference", "crexx_programming_guide",
         "crexx_vm_spec", "crexx_library_reference")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bin", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--pandoc", type=Path)
    args = parser.parse_args()
    product = args.bin.resolve()
    work = (args.work.resolve() if args.work else
            Path(tempfile.mkdtemp(prefix="crexx-texttools-tests-")))
    work.mkdir(parents=True, exist_ok=True)
    imports = work / "imports"
    imports.mkdir()
    receipts = []

    def run(label, command, expected=0, env=None):
        log = work / (label + ".log")
        with log.open("w") as stream:
            result = subprocess.run([str(x) for x in command], cwd=work,
                                    env=env, stdout=stream,
                                    stderr=subprocess.STDOUT, timeout=300)
        receipts.append({"label": label, "rc": result.returncode, "log": str(log)})
        good = result.returncode == expected if expected is not None else result.returncode != 0
        if not good:
            raise AssertionError(f"{label}: rc {result.returncode}; {log}\n{log.read_text()[-3000:]}")
        return log

    sources = {"texttools": REPO / "docs/texttools/texttools.crexx",
               "build": REPO / "docs/texttools/build.crexx",
               "regression": REPO / "docs/texttools/tests/regression.crexx",
               "generate-books": REPO / "docs/generate-books.crexx"}
    for name, source in sources.items():
        stem = imports / name if name == "texttools" else work / name
        run(name + "-compile", [product / "rxc", "-i", f"{imports};{product}",
                                "-o", stem, source])
        run(name + "-assemble", [product / "rxas", "-o", stem, stem])
        if name != "texttools":
            run(name + "-link", [product / "rxlink", "-o", work / (name + "-linked"),
                                    stem.with_suffix(".rxbin"), imports / "texttools.rxbin",
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
    for book in BOOKS:
        folder = source / "docs/books" / book
        folder.mkdir(parents=True)
        (folder / "MixedCase.md").write_text("# Example\n```rexx <!--Example.crexx-->\nsay 'ok'\n```\n")
        (folder / "structure.tex").write_text(r"\input{MixedCase}" + "\n")
        (folder / (book + ".tex")).write_text("fixture main file\n")
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
    calls = [json.loads(line) for line in commands.read_text().splitlines()]
    for book in BOOKS:
        built = work / "four books/docs/books" / book / "tex/book"
        received = [c["tool"] for c in calls if Path(c["cwd"]).resolve() == built.resolve()]
        assert received == ["pandoc", "pandoc", "xelatex", "makeindex", "biber",
                            "xelatex", "makeindex", "biber", "xdvipdfmx"], received
        assert (built / "Example.crexx").read_text() == "say 'ok'\n"
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
    run("existing-output", wrapper + ["build", source, work / "four books", BOOKS[0]], expected=2, env=env)
    run("inside-source", wrapper + ["build", source, source / "bad-output", BOOKS[0]], expected=2, env=env)
    alias = work / "source-alias"
    alias.symlink_to(source, target_is_directory=True)
    run("inside-source-alias", wrapper + ["build", source, alias / "bad-output", BOOKS[0]], expected=2, env=env)
    run("unknown-book", wrapper + ["build", source, work / "unknown", "unknown"], expected=2, env=env)
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
                paths = re.findall(r"\\lstinputlisting[^\n]*?\]\{([^}]+)\}", tex)
                for _name, _language, filename in tags:
                    assert (generated / filename).is_file(), filename
                    assert filename in paths, f"Pandoc listing path changed: {filename}"
                    extracted += 1
                assert r"\%includesource" not in tex
                assert r"\%splice\%" not in tex
            receipts.append({"book": book, "real_pandoc_chapters": len(chapters)})
            receipts.append({"book": book, "extracted_listings": extracted})
    (work / "results.json").write_text(json.dumps(receipts, indent=2) + "\n")
    print(f"TextTools checks passed. Receipts: {work / 'results.json'}")
    print("Mock typesetter checks do not qualify PDF output.")


if __name__ == "__main__":
    main()
