import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

SCRIPTS = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('snapshot', SCRIPTS / 'sign-windows-snapshot.py')
snapshot = importlib.util.module_from_spec(spec)
spec.loader.exec_module(snapshot)


class SnapshotSigningTests(unittest.TestCase):
    def setUp(self):
        self.commit = 'a' * 40
        self.core = 'CREXX-dev-snapshot-windows-x64.zip'
        self.plugin = 'llama.rexx-dev-snapshot-windows-x64-vulkan.zip'
        self.state = dict(repo='owner/repo', tag='dev-snapshot', commit=self.commit,
                          release_id=10, asset=dict(name=self.core))
        self.dependency = dict(self.state, asset=dict(name=self.plugin))
        self.state['dependencies'] = [self.dependency]
        self.item = dict(id=1, name=f'llama-manager-{self.commit}-windows-x64', expired=False,
                         workflow_run=dict(id=7, head_sha=self.commit), digest='sha256:abc')
        self.run = dict(head_sha=self.commit, status='completed', conclusion='success',
                        path='.github/workflows/build.yml', repository=dict(full_name='owner/repo'),
                        head_repository=dict(full_name='owner/repo'))

    def test_default_pins_core_and_plugin_but_never_signed_derivatives(self):
        names = [self.core, self.plugin, self.plugin.replace('.zip', '-signed.zip')]
        with patch.object(snapshot.assets, 'release', return_value=dict(assets=[dict(name=n) for n in names])), \
             patch.object(snapshot.assets, 'capture', side_effect=[dict(self.state), self.dependency]), \
             patch.object(snapshot.assets, 'verify') as verify:
            result = snapshot.capture_inputs('owner/repo', 'dev-snapshot')
            self.assertEqual([s['asset']['name'] for s in result['dependencies']], [self.plugin])
            verify.assert_called_once_with(result)

    def test_no_plugin_fails_instead_of_silently_signing_only_core(self):
        with patch.object(snapshot.assets, 'release', return_value=dict(assets=[dict(name=self.core)])):
            with self.assertRaisesRegex(ValueError, 'no Windows llama'):
                snapshot.capture_inputs('owner/repo', 'dev-snapshot')

    def manager_api(self, path, *args):
        return [dict(artifacts=[self.item])] if '/artifacts?' in path else self.run

    def test_manager_requires_matching_successful_build_and_digest(self):
        with patch.object(snapshot.assets, 'api', side_effect=self.manager_api):
            self.assertEqual(snapshot.manager_artifact('owner/repo', self.commit), self.item)
            for changes in (dict(conclusion='failure'), dict(head_sha='b'*40),
                            dict(path='.github/workflows/unrelated.yml'),
                            dict(head_repository=dict(full_name='fork/repo'))):
                saved = copy.deepcopy(self.run)
                self.run.update(changes)
                with self.assertRaisesRegex(ValueError, 'No retained manager'):
                    snapshot.manager_artifact('owner/repo', self.commit)
                self.run = saved
            self.item['expired'] = True
            with self.assertRaisesRegex(ValueError, 'No retained manager'):
                snapshot.manager_artifact('owner/repo', self.commit)
            self.item['expired'] = False
            self.item['digest'] = None
            with self.assertRaisesRegex(ValueError, 'SHA-256'):
                snapshot.manager_artifact('owner/repo', self.commit)

    def test_corrupt_manager_archive_is_not_extracted(self):
        with tempfile.TemporaryDirectory() as temp, \
             patch.object(snapshot.subprocess, 'run'), \
             patch.object(snapshot.paired.package, 'extract') as extract:
            with self.assertRaisesRegex(ValueError, 'SHA-256'):
                snapshot.download_manager('owner/repo', self.item, Path(temp))
            extract.assert_not_called()

    def test_signing_publishes_complete_verified_set_only(self):
        with tempfile.TemporaryDirectory() as temp:
            work = Path(temp)
            names = [n.removesuffix('.zip') + suffix for n in (self.core, self.plugin)
                     for suffix in ('-signed.zip', '-signed-setup.exe')]
            def sign(command, **kwargs):
                self.assertIn(str(work / self.core), command)
                self.assertIn(str(work / self.plugin), command)
                output = work / 'signed'
                output.mkdir(exist_ok=True)
                for name in names:
                    (output / name).write_bytes(b'signed fixture')
                files = {name: snapshot.paired.package.digest(output / name) for name in names}
                (output / 'signed-delivery.json').write_text(json.dumps(dict(commit=self.commit, files=files)))
            with patch.object(snapshot.assets, 'download'), \
                 patch.object(snapshot.assets, 'verify_payload'), \
                 patch.object(snapshot.assets, 'verify'), \
                 patch.object(snapshot.paired, 'unpack', return_value=(work, {})), \
                 patch.object(snapshot, 'download_manager', return_value=work / 'manager'), \
                 patch.object(snapshot.subprocess, 'run', side_effect=sign), \
                 patch.object(snapshot.assets, 'publish') as publish:
                snapshot.sign_snapshot(self.state, self.item, work)
                self.assertEqual({p.name for p in publish.call_args.args[1]}, set(names))
                publish.reset_mock()
                names.pop()
                with self.assertRaisesRegex(ValueError, 'Incomplete'):
                    snapshot.sign_snapshot(self.state, self.item, work)
                publish.assert_not_called()


if __name__ == '__main__':
    unittest.main()
