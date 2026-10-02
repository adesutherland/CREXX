import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('doc_assets', REPO / 'scripts/check-doc-assets.py')
doc_assets = importlib.util.module_from_spec(spec)
spec.loader.exec_module(doc_assets)
SHA = '0123456789abcdef0123456789abcdef01234567'
VERSION = 'crexx-1.0.0-beta.3+dev-snapshot.g0123456789ab'


def run_block(path, step):
    text = path.read_text()
    start = text.index('      - name: ' + step + '\n')
    end = text.find('\n      - name:', start + 1)
    block = text[start:end if end >= 0 else len(text)]
    return '\n'.join(line[10:] for line in block.split('        run: |\n', 1)[1].splitlines())


class DocumentPublicationTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)

    def documents(self, folder, tag='dev-snapshot'):
        assets = []
        for name in doc_assets.BOOKS.values():
            path = folder / f'CREXX-{tag}-{name}.pdf'
            path.write_bytes(b'PDF payload fixture')
            assets.append(dict(name=path.name, sha256=doc_assets.digest(path)))
        path = folder / f'CREXX-{tag}-docs.json'
        path.write_text(json.dumps(dict(commit=SHA, version=VERSION, assets=assets)))
        return path

    def test_asset_identity_and_hashes_reject_other_source_version_and_partial_sets(self):
        manifest = self.documents(self.root)
        doc_assets.verify_assets(self.root, 'dev-snapshot', SHA, VERSION)
        for sha, version in [('f' * 40, VERSION), (SHA, 'crexx-1.0.0-beta.2')]:
            with self.assertRaisesRegex(ValueError, 'source/version'):
                doc_assets.verify_assets(self.root, 'dev-snapshot', sha, version)
        data = json.loads(manifest.read_text())
        data['assets'].pop()
        manifest.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError, 'exactly the four'):
            doc_assets.verify_assets(self.root, 'dev-snapshot', SHA, VERSION)
        self.documents(self.root)
        (self.root / 'CREXX-dev-snapshot-language-reference.pdf').write_bytes(b'wrong book')
        with self.assertRaisesRegex(ValueError, 'hash mismatch'):
            doc_assets.verify_assets(self.root, 'dev-snapshot', SHA, VERSION)

    def collect_snapshot(self, ready, change=None):
        folder = self.root / 'downloaded-dev-snapshot-assets'
        folder.mkdir()
        binaries = [
            'CREXX-dev-snapshot-linux-x64.zip', 'CREXX-dev-snapshot-linux-x64.deb',
            'CREXX-dev-snapshot-windows-x64.zip', 'CREXX-dev-snapshot-windows-x64-unsigned-setup.exe',
            'CREXX-dev-snapshot-macos-arm64.zip', 'CREXX-dev-snapshot-macos-x86_64.zip',
            'llama.rexx-dev-snapshot-linux-x64-vulkan.zip',
            'llama.rexx-dev-snapshot-windows-x64-vulkan.zip',
            'llama.rexx-dev-snapshot-macos-arm64-metal.zip',
            'llama.rexx-dev-snapshot-macos-x86_64-cpu.zip',
            'llama.rexx-dev-snapshot-windows-x64-vulkan-unsigned-setup.exe',
        ] + [f'CREXX-sdk-{SHA}-{p}.zip' for p in ['windows-x64', 'macos-arm64', 'macos-x86_64']]
        for name in binaries:
            (folder / name).write_bytes(b'binary fixture')
        self.documents(folder)
        if change:
            change(folder)
        block = run_block(REPO / '.github/workflows/build.yml', 'Collect dev snapshot assets')
        substitutions = {'runner.temp': str(self.root), 'needs.documents.outputs.docs_ready': str(ready).lower(),
                         'needs.version.outputs.display_version': VERSION}
        block = re.sub(r'\$\{\{\s*(.*?)\s*\}\}', lambda m: substitutions[m[1]], block)
        result = subprocess.run(['bash', '-c', block], cwd=REPO, capture_output=True, text=True,
                                env={**os.environ, 'GITHUB_SHA': SHA, 'GITHUB_OUTPUT': str(self.root / 'outputs')})
        return result, self.root / 'dev-snapshot-assets'

    def test_successful_documents_join_existing_binary_set(self):
        result, assets = self.collect_snapshot(True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(len(list(assets.iterdir())), 19)

    def test_failed_or_partial_documents_do_not_block_binaries_or_leave_old_books(self):
        result, assets = self.collect_snapshot(False, lambda folder:
            (folder / 'CREXX-dev-snapshot-language-reference.pdf').unlink())
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(len(list(assets.iterdir())), 14)
        self.assertEqual(list(assets.glob('*.pdf')), [])
        self.assertFalse((assets / 'CREXX-dev-snapshot-docs.json').exists())

    def test_binary_checks_remain_required_when_docs_fail(self):
        result, _ = self.collect_snapshot(False, lambda folder:
            (folder / 'CREXX-dev-snapshot-linux-x64.zip').unlink())
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Missing dev snapshot asset', result.stderr)

    def test_claimed_success_with_missing_book_still_publishes_binaries_without_docs(self):
        result, assets = self.collect_snapshot(True, lambda folder:
            (folder / 'CREXX-dev-snapshot-language-reference.pdf').unlink())
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(len(list(assets.iterdir())), 14)
        self.assertEqual(list(assets.glob('*.pdf')), [])
        self.assertIn('docs_ready=false', (self.root / 'outputs').read_text())

    def test_deep_document_failure_is_fatal_and_ordinary_failure_is_reported(self):
        block = run_block(REPO / '.github/workflows/build-docs.yml', 'Report document result')
        for ready, required, expected in [(True, True, 0), (True, False, 0), (False, False, 0), (False, True, 1)]:
            with self.subTest(ready=ready, required=required):
                summary = self.root / f'{ready}-{required}.md'
                result = subprocess.run(['bash', '-c', block], capture_output=True, text=True,
                    env={**os.environ, 'DOCS_READY': str(ready).lower(), 'REQUIRED': str(required).lower(),
                         'GITHUB_STEP_SUMMARY': str(summary)})
                self.assertEqual(result.returncode, expected)
                if not ready:
                    self.assertIn('Document generation failed', summary.read_text())
        deep = (REPO / '.github/workflows/deep-build.yml').read_text()
        self.assertIn('required: true', deep)
        self.assertIn("needs.documents.result == 'success'", deep)
        self.assertIn("needs.documents.outputs.docs_ready == 'true'", deep)

    def test_literal_listing_extraction_preserves_tabs_empty_and_truncated_text(self):
        source = self.root / 'sample.md'
        source.write_text('# Example\n```crexx\n\tsay "partial\n\n```\n````text\n```\n````\n```rexx\n```\n')
        self.assertEqual(list(doc_assets.listing_bodies(source)), [
            ('sample-code-1.txt', b'\tsay "partial\n\n'),
            ('sample-code-2.txt', b'```\n'), ('sample-code-3.txt', b'')])

    def test_version_fields_are_exact_and_reconstruct_wrapped_versions(self):
        cover = 'CREXX\nBuild version: crexx-1.0.0-beta.3\nTHE REXX LANGUAGE ASSOCIATION\nISBN 978'
        self.assertEqual(doc_assets.version_field(cover, r'Build\s+version:\s*'), 'crexx-1.0.0-beta.3')
        self.assertNotEqual(doc_assets.version_field(cover, r'Build\s+version:\s*'), 'crexx-1.0.0')
        data = 'Publication Data\nContent is up to date with version crexx-1.0.0-beta.3+dev-\nsnapshot.g0123456789ab\nii'
        self.assertEqual(doc_assets.version_field(data, r'Content is up to date with version\s*'), VERSION)

    def test_source_snapshot_records_symlink_target_even_when_referent_bytes_match(self):
        subprocess.run(['git', 'init', '-q', str(self.root)], check=True)
        folder = self.root / 'docs/books'
        folder.mkdir(parents=True)
        for name in ['a.md', 'b.md']:
            (folder / name).write_text('same source bytes\n')
        link = folder / 'guide.md'
        link.symlink_to('a.md')
        subprocess.run(['git', '-C', str(self.root), 'add', 'docs'], check=True)
        before = doc_assets.snapshot(self.root)
        link.unlink()
        link.symlink_to('b.md')
        after = doc_assets.snapshot(self.root)
        self.assertNotEqual(before, after)
        self.assertEqual(before['docs/books/guide.md']['sha256'], after['docs/books/guide.md']['sha256'])

    def test_snapshot_prunes_previous_documents_only_when_they_are_not_current(self):
        text = (REPO / '.github/workflows/build.yml').read_text()
        loop = re.search(r'          for stale_optional_asset in .*?\n          done', text, re.DOTALL).group()
        names = [f'CREXX-dev-snapshot-{name}.pdf' for name in doc_assets.BOOKS.values()] + ['CREXX-dev-snapshot-docs.json']
        for ready in [False, True]:
            deletes = self.root / f'deletes-{ready}'
            expected = ' '.join(names) if ready else 'no-current-documents'
            prelude = '''set -euo pipefail
asset_is_expected() { [[ " $EXPECTED " == *" $1 "* ]]; }
asset_api_path() { echo "$1"; }
gh() { echo "${@: -1}" >> "$DELETES"; }
'''
            result = subprocess.run(['bash', '-c', prelude + loop], capture_output=True, text=True,
                env={**os.environ, 'EXPECTED': expected, 'DELETES': str(deletes)})
            self.assertEqual(result.returncode, 0, result.stderr)
            deleted = set(deletes.read_text().splitlines())
            self.assertEqual(deleted.intersection(names), set() if ready else set(names))


if __name__ == '__main__':
    unittest.main()
