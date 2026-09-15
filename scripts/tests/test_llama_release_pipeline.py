import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest


SCRIPTS = Path(__file__).resolve().parents[1]


def module(name):
    spec = importlib.util.spec_from_file_location(name, SCRIPTS / (name + '.py'))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


refresh = module('refresh-provider-manifests')
matrix = module('ci-release-matrix')


class ManifestSigningTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.library = self.root / 'cpu.dll'
        self.library.write_bytes(b'unsigned PE payload')
        common = dict(version=1, provider='rxllama', platform='Windows', arch='AMD64', engine='pin')
        entry = dict(path='cpu.dll', sha256=refresh.digest(self.library))
        self.runtime = self.root / 'rxllama.runtime.json'
        self.runtime.write_text(json.dumps(dict(common, backends=[dict(entry, backend='cpu')])))
        self.native = self.root / 'rxllama.native.json'
        self.native.write_text(json.dumps(dict(common, link_libraries=[entry], runtime_files=[entry,
            dict(path=self.runtime.name, sha256=refresh.digest(self.runtime))])))

    def test_signing_invalidates_hashes_and_refresh_covers_manifest_dependency(self):
        before = self.native.read_bytes()
        self.library.write_bytes(self.library.read_bytes() + b' Authenticode signature')
        self.assertNotEqual(json.loads(before)['runtime_files'][0]['sha256'], refresh.digest(self.library))
        refresh.refresh(self.root)
        after = json.loads(self.native.read_text())
        for entry in after['link_libraries'] + after['runtime_files']:
            self.assertEqual(entry['sha256'], refresh.digest(self.root / entry['path']))
        self.assertEqual(json.loads(self.runtime.read_text())['backends'][0]['sha256'], refresh.digest(self.library))
        self.assertEqual(after['engine'], 'pin')
        self.assertEqual(len(after['runtime_files']), 2)

    def test_missing_file_is_not_removed_or_blessed(self):
        before = self.native.read_bytes(), self.runtime.read_bytes()
        self.library.unlink()
        with self.assertRaisesRegex(RuntimeError, 'Missing/nonlocal'):
            refresh.refresh(self.root)
        self.assertEqual(before, (self.native.read_bytes(), self.runtime.read_bytes()))

    def test_unknown_file_is_not_added(self):
        (self.root / 'unlisted.dll').write_bytes(b'unlisted')
        refresh.refresh(self.root)
        self.assertNotIn('unlisted.dll', self.native.read_text())


class MatrixTests(unittest.TestCase):
    def test_diagnostic_selection_only_on_candidate_manual_runs(self):
        candidate = 'refs/heads/temp/llama-release-qa'
        self.assertEqual(len(matrix.select('windows', 'workflow_dispatch', candidate)['include']), 1)
        self.assertEqual(len(matrix.select('base', 'workflow_dispatch', candidate)['include']), 4)
        self.assertEqual(len(matrix.select('cuda', 'workflow_dispatch', candidate)['include']), 2)
        for event, ref in [('push', 'refs/heads/develop'), ('create', 'refs/tags/v1.0.0'),
                           ('workflow_dispatch', 'refs/heads/develop'), ('push', candidate)]:
            self.assertEqual(len(matrix.select('windows', event, ref)['include']), 6)

    def test_delivery_names_unique_and_required_backends_present(self):
        rows = matrix.select('all', 'push', 'refs/heads/develop')['include']
        self.assertEqual(len({r['artifact_name'] for r in rows}), 6)
        for row in rows:
            self.assertIn('cpu', row['backends'].split(','))
            self.assertEqual(len(row['backends'].split(',')), 2)
            if row['platform'] == 'windows' and row['cuda']:
                self.assertEqual(row['toolchain'], 'msvc')

    def test_core_matrix_is_shared_and_has_no_gpu_build_flags(self):
        providers = matrix.select('all', 'push', 'refs/heads/develop')
        cores = matrix.cores_for(providers)['include']
        self.assertEqual(len(cores), 4)
        self.assertEqual(len({c['core_platform'] for c in cores}), 4)
        self.assertEqual(len([c for c in cores if c['platform'] == 'windows']), 1)
        for core in cores:
            self.assertFalse(core['cuda'])
            self.assertNotIn('CREXX_LLAMA_', core['cmake_args'])
            self.assertNotEqual(core['toolchain'], 'mingw')
        for row in providers['include']:
            if row['platform'] == 'windows':
                self.assertEqual(row['toolchain'], 'msvc')
                self.assertEqual(row['core_platform'], 'windows-x64')
        selected = matrix.cores_for(matrix.select('windows-cuda', 'workflow_dispatch',
                                                'refs/heads/temp/llama-release-qa'))['include']
        self.assertEqual(len(selected), 1)
        self.assertEqual(selected[0]['artifact_name'], 'CREXX-windows-x64')


if __name__ == '__main__':
    unittest.main()
