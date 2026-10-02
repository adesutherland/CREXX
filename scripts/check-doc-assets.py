#!/usr/bin/env python3
"""Check fresh book output and publish only a complete, identified PDF set."""
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

BOOKS = {
    'crexx_language_reference': 'language-reference',
    'crexx_programming_guide': 'programming-guide',
    'crexx_vm_spec': 'vm-specification',
    'crexx_library_reference': 'library-reference',
}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def snapshot(repo):
    paths = subprocess.check_output([
        'git', '-C', str(repo), 'ls-files', '-z', 'docs/books', 'docs/instructions',
        'docs/texttools', 'docs/ai-context', 'docs/generate-books.crexx',
    ]).decode().split('\0')
    return {p: dict(sha256=digest(repo / p),
                    symlink=os.readlink(repo / p) if (repo / p).is_symlink() else None)
            for p in paths if p and (repo / p).is_file()}


def listing_bodies(path):
    lines = path.read_bytes().decode('utf-8').split('\n')
    i = 0
    serial = 0
    while i < len(lines):
        line = lines[i]
        if not line.startswith('```'):
            i += 1
            continue
        fence = re.match(r'`+', line).group()
        first = i + 1
        i = first
        while i < len(lines) and lines[i].strip() != fence:
            i += 1
        if i == len(lines):
            raise ValueError(f'Unclosed source fence: {path}:{first}')
        serial += 1
        yield f'{path.stem}-code-{serial}.txt', ''.join(s + '\n' for s in lines[first:i]).encode('utf-8')
        i += 1


def version_field(text, label):
    match = re.search(label, text)
    if not match:
        return None
    lines = text[match.end():].strip().splitlines()
    pieces = []
    for i, line in enumerate(lines):
        line = line.strip()
        # Publication data ends with its separate page number; it is not part
        # of a version that wrapped at one of the staged break opportunities.
        if i > 0 and i == len(lines) - 1 and re.fullmatch(r'(?:[ivxlcdm]+|[0-9]+)', line):
            break
        if not re.fullmatch(r'[A-Za-z0-9.+_-]+', line):
            break
        pieces.append(line)
    return ''.join(pieces)


def validate_pdf(path, version):
    from pypdf import PdfReader
    reader = PdfReader(path)
    if reader.is_encrypted or len(reader.pages) < 3:
        raise ValueError(f'Unreadable/incomplete book PDF: {path}')
    if version_field(reader.pages[0].extract_text(), r'Build\s+version:\s*') != version:
        raise ValueError(f'Cover version missing/mismatched: {path}')
    data = [page.extract_text() for page in reader.pages[1:5]]
    if not any('Publication Data' in text and version_field(
        text, r'Content is up to date with version\s*') == version for text in data):
        raise ValueError(f'Publication-data version missing/mismatched: {path}')
    return len(reader.pages)


def collect(repo, work, version, tag, commit):
    if not re.fullmatch(r'[A-Za-z0-9.+_-]+', tag):
        raise ValueError('Unsafe asset tag')
    if subprocess.check_output(['git', '-C', str(repo), 'rev-parse', 'HEAD']).decode().strip() != commit:
        raise ValueError('Source commit changed during document generation')
    before = json.loads((work / 'source-before.json').read_text())
    if before != snapshot(repo):
        raise ValueError('Authored inputs changed during document generation')
    assets = []
    listings = 0
    fatal = re.compile(r'Missing character:|There were undefined references|'
                       r'(?:Reference|Citation) .+ undefined|'
                       r'Rerun to get (?:cross-references|outlines) right|'
                       r'Please \(re\)run Biber|^! ', re.MULTILINE)
    for book, name in BOOKS.items():
        output = work / 'books/docs/books' / book / 'tex/book'
        log = (output / f'{book}.log').read_text(errors='replace')
        match = fatal.search(log)
        if match:
            raise ValueError(f'Unresolved typesetting diagnostic in {book}: {match.group()}')
        for source in sorted((repo / 'docs/books' / book).glob('*.md')):
            for listing, body in listing_bodies(source):
                if (output / listing).read_bytes() != body:
                    raise ValueError(f'Listing text differs from authored source: {book}/{listing}')
                listings += 1
        pdf = output / f'{book}.pdf'
        pages = validate_pdf(pdf, version)
        assets.append(dict(book=book, name=f'CREXX-{tag}-{name}.pdf', sha256=digest(pdf),
                           bytes=pdf.stat().st_size, pages=pages, source=pdf))
    # Nothing enters the release-asset folder before every book passes.
    target = work / 'release-assets'
    target.mkdir()
    for asset in assets:
        shutil.copyfile(asset.pop('source'), target / asset['name'])
    manifest = dict(schema=1, component='documentation', commit=commit,
                    version=version, font_profile='initial', listing_snapshots=listings,
                    authored_inputs=len(before),
                    authored_input_manifest_sha256=digest(work / 'source-before.json'),
                    product_tools={name: digest(work / 'product/bin' / name) for name in
                        ['rxc', 'rxas', 'rxlink', 'rxvm', 'rxvme', 'crexx', 'rxdas', 'rxdb', 'rxcpack', 'library.rxbin']},
                    workflow_run=os.environ.get('GITHUB_RUN_ID'),
                    dependency_versions=(work / 'logs/dependency-versions.log').read_text(),
                    assets=assets)
    (target / f'CREXX-{tag}-docs.json').write_text(json.dumps(manifest, indent=2) + '\n')
    print(f'Validated four PDFs and {listings} exact listing snapshots for {version}')


def verify_assets(directory, tag, commit, version):
    manifest = json.loads((directory / f'CREXX-{tag}-docs.json').read_text())
    if manifest['commit'] != commit or manifest['version'] != version:
        raise ValueError('Document assets do not match binary source/version')
    expected = {f'CREXX-{tag}-{name}.pdf' for name in BOOKS.values()}
    if {asset['name'] for asset in manifest['assets']} != expected or len(manifest['assets']) != 4:
        raise ValueError('Document asset set must contain exactly the four books')
    for asset in manifest['assets']:
        if digest(directory / asset['name']) != asset['sha256']:
            raise ValueError(f'Document asset hash mismatch: {asset["name"]}')
    print('Four document asset hashes, source revision and version verified')


def main():
    if sys.argv[1] == 'snapshot':
        repo, output = map(Path, sys.argv[2:])
        output.write_text(json.dumps(snapshot(repo), indent=2, sort_keys=True) + '\n')
    elif sys.argv[1] == 'collect':
        collect(Path(sys.argv[2]), Path(sys.argv[3]), *sys.argv[4:])
    elif sys.argv[1] == 'verify':
        verify_assets(Path(sys.argv[2]), *sys.argv[3:])
    else:
        raise ValueError('Expected snapshot or collect')


if __name__ == '__main__':
    main()
