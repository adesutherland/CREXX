import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import zipfile


path = Path(__file__).resolve().parents[1] / 'fetch-signed-windows-qa.py'
spec = importlib.util.spec_from_file_location('signed_qa', path)
qa = importlib.util.module_from_spec(spec)
spec.loader.exec_module(qa)


class SignedTransportTests(unittest.TestCase):
    def exercise(self, wrong_commit=False, corrupt_input=False, published=False):
        with tempfile.TemporaryDirectory() as temporary:
            work = Path(temporary)
            commit = 'a' * 40
            original = work / 'retained/core/original.zip'
            original.parent.mkdir(parents=True)
            original.write_bytes(b'original qualified archive')
            (work / 'proof').mkdir()
            zipped = io.BytesIO()
            with zipfile.ZipFile(zipped, 'w') as archive:
                archive.writestr('CREXX-windows-x64/core-package.json', '{}')
            blobs = {'CREXX-candidate-signed.zip': zipped.getvalue(), 'CREXX-candidate-signed-setup.exe': b'signed fixture'}
            record = dict(commit=commit, platform='windows-x64',
                inputs={'/signer/original.zip': hashlib.sha256(original.read_bytes()).hexdigest()},
                files={name: hashlib.sha256(data).hexdigest() for name, data in blobs.items()})
            blobs['signed-delivery.json'] = json.dumps(record).encode()
            names = list(blobs)
            info = dict(id=7, draft=not published, target_commitish='b'*40 if wrong_commit else commit,
                tag_name='qa-llama-signing-' + commit,
                assets=[dict(id=i, name=name, digest='sha256:' + hashlib.sha256(blobs[name]).hexdigest())
                        for i, name in enumerate(names)])
            if corrupt_input:
                original.write_bytes(b'changed')

            def download(command, stdout, check):
                self.assertEqual(command[:2], ['gh', 'api'])
                stdout.write(blobs[names[int(command[2].rsplit('/', 1)[1])]])

            with patch.dict(os.environ, GITHUB_REPOSITORY='owner/repo'), \
                    patch('sys.argv', ['fetch', '--release-id', '7', '--commit', commit, '--work', str(work)]), \
                    patch.object(qa.subprocess, 'check_output', return_value=json.dumps(info).encode()), \
                    patch.object(qa.subprocess, 'run', side_effect=download):
                qa.main()
            self.assertTrue((work / 'core/CREXX-windows-x64/core-package.json').exists())

    def test_matching_private_artifacts_are_staged(self):
        self.exercise()

    def test_wrong_commit_or_public_release_is_rejected(self):
        for changes in (dict(wrong_commit=True), dict(published=True)):
            with self.subTest(changes=changes), self.assertRaisesRegex(RuntimeError, 'private draft'):
                self.exercise(**changes)

    def test_signed_artifact_requires_the_exact_qualified_unsigned_input(self):
        with self.assertRaisesRegex(RuntimeError, 'differs from qualified'):
            self.exercise(corrupt_input=True)


if __name__ == '__main__':
    unittest.main()
