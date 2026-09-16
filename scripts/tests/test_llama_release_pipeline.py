import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import textwrap
import unittest


SCRIPTS = Path(__file__).resolve().parents[1]


def module(name):
    spec = importlib.util.spec_from_file_location(name, SCRIPTS / (name + '.py'))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


refresh = module('refresh-provider-manifests')
matrix = module('ci-release-matrix')
core_qa = module('ci-core-qa-matrix')


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


class PackageSigningTests(unittest.TestCase):
    def test_signer_preserves_selected_vm_copy_and_refreshes_core_manifest(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            payload = root / 'payload'
            (payload / 'bin').mkdir(parents=True)
            for name in ('rxvm.exe', 'rxbvm.exe'):
                (payload / 'bin' / name).write_bytes(b'MZ unsigned fixture')
            files = {'bin/' + name: refresh.digest(payload / 'bin' / name)
                     for name in ('rxvm.exe', 'rxbvm.exe')}
            (payload / 'core-package.json').write_text(json.dumps(dict(files=files, commit='pin')))
            mock = root / 'tools'
            mock.mkdir()
            scripts = {
                'file': '#!/bin/sh\necho PE32\n',
                'osslsigncode': '#!/bin/sh\ngrep -q SIGNED "$3"\n',
                'jsign': '#!/bin/bash\nfor last; do :; done\nprintf "SIGNED %s" "$RANDOM" >> "$last"\n'}
            for name, contents in scripts.items():
                path = mock / name
                path.write_text(contents)
                path.chmod(0o755)
            command = ['bash', '-c', 'set -e; source "$1"; sign_windows_payload "$2" unused unused unused',
                       'sign-test', str(SCRIPTS / 'windows-signing-common.sh'), str(payload)]
            # Model a signer that changes PE bytes with a distinct timestamp.
            # Manifests themselves are JSON, not PE; fake file must preserve that.
            (mock / 'file').write_text('#!/bin/sh\ncase "$2" in *.exe) echo PE32;; *) echo JSON;; esac\n')
            result = subprocess.run(command, env=dict(os.environ, PATH=str(mock) + os.pathsep + os.environ['PATH']),
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual((payload / 'bin/rxvm.exe').read_bytes(), (payload / 'bin/rxbvm.exe').read_bytes())
            module('refresh-package-manifests').update(payload, verify=True)

    def test_nested_manifests_refresh_without_changing_identity_or_members(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / 'bin').mkdir()
            binary = root / 'bin/vm.exe'
            binary.write_bytes(b'unsigned')
            info = dict(schema=1, component='core', commit='a'*40,
                        files={'bin/vm.exe': refresh.digest(binary)})
            manifest = root / 'core-package.json'
            manifest.write_text(json.dumps(info))
            package_refresh = module('refresh-package-manifests')
            package_refresh.update(root, verify=True)
            binary.write_bytes(b'signed')
            with self.assertRaisesRegex(ValueError, 'Package file changed'):
                package_refresh.update(root, verify=True)
            package_refresh.update(root)
            result = json.loads(manifest.read_text())
            self.assertEqual(result['commit'], info['commit'])
            self.assertEqual(set(result['files']), set(info['files']))
            self.assertEqual(result['files']['bin/vm.exe'], refresh.digest(binary))
            package_refresh.update(root, verify=True)
            previous = manifest.read_bytes()
            binary.unlink()
            with self.assertRaisesRegex(ValueError, 'Missing/nonlocal'):
                package_refresh.update(root)
            self.assertEqual(previous, manifest.read_bytes())

    def test_signing_rejects_mixed_source_and_overlapping_payloads(self):
        sign = module('sign-windows-packages')
        core = dict(component='core', llama_enabled=False, commit='a'*40,
                    platform='windows-x64', toolchain='msvc', files={'bin/vm.exe': 'x'})
        plugin = dict(component='llama.rexx', backend='cuda', commit='a'*40,
                      platform='windows-x64', toolchain='msvc', files={'bin/llama.dll': 'x'})
        sign.validate_pair(core, plugin)
        with self.assertRaisesRegex(ValueError, 'identity mismatch'):
            sign.validate_pair(core, dict(plugin, commit='b'*40))
        with self.assertRaisesRegex(ValueError, 'overlaps'):
            sign.validate_pair(core, dict(plugin, files=core['files']))

    def test_refresh_cannot_escape_root_or_bless_missing_bootstrap(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            manifest = root / 'manager.json'
            for names in ({'../escape': 'x'}, {'bin/missing.dll': 'x'}):
                manifest.write_text(json.dumps(dict(files={}, bootstrap_files=names)))
                with self.assertRaisesRegex(ValueError, 'Missing/nonlocal'):
                    module('refresh-package-manifests').update(root)


class MatrixTests(unittest.TestCase):
    def test_deep_subset_cannot_replace_ordinary_or_scheduled_full_gate(self):
        for event, ref in [('schedule', 'refs/heads/develop'),
                           ('workflow_dispatch', 'refs/heads/develop'),
                           ('push', 'refs/heads/temp/llama-release-qa')]:
            rows, full = core_qa.select('windows-msvc', event, ref)
            self.assertTrue(full)
            self.assertEqual(len(rows['include']), 5)
            self.assertEqual({r.get('toolchain') for r in rows['include'] if r['platform'] == 'windows'},
                             {'msvc', 'mingw'})

    def test_deep_candidate_retry_selects_only_requested_core(self):
        rows, full = core_qa.select('windows-msvc', 'workflow_dispatch',
                                   'refs/heads/temp/llama-release-combined')
        self.assertFalse(full)
        self.assertEqual([r['toolchain'] for r in rows['include']], ['msvc'])
        with self.assertRaises(ValueError):
            core_qa.select('unknown', 'workflow_dispatch', 'refs/heads/temp/llama-release-qa')

    def test_cuda_only_for_release_or_explicit_manual_selection(self):
        for ref in ('refs/heads/develop', 'refs/heads/temp/llama-release-combined'):
            for event in ('push', 'pull_request', 'schedule'):
                rows = matrix.select('all', event, ref)['include']
                self.assertEqual(len(rows), 4)
                self.assertFalse(any(r['cuda'] for r in rows))
            for lane, count in [('base', 4), ('cuda', 2), ('windows', 1), ('all', 6)]:
                self.assertEqual(len(matrix.select(lane, 'workflow_dispatch', ref)['include']), count)
        for tag in ('v1.0.0', 'v1.0.0-beta.3'):
            for event in ('create', 'workflow_dispatch'):
                rows = matrix.select('base', event, 'refs/tags/' + tag)['include']
                self.assertEqual(len(rows), 6)
                self.assertEqual(sum(r['cuda'] for r in rows), 2)
        with self.assertRaises(ValueError):
            matrix.select('unknown', 'workflow_dispatch', 'refs/heads/develop')

    def test_collectors_require_correct_plugin_sets(self):
        workflow = (SCRIPTS.parent / '.github/workflows/build.yml').read_text()
        release = workflow.split('      - name: Collect release assets')[1].split('      - name:')[0]
        snapshot = workflow.split('      - name: Collect dev snapshot assets')[1].split('      - name:')[0]
        for backend in ('linux-x64-cuda', 'windows-x64-cuda'):
            self.assertIn(backend, release)
            self.assertNotIn(backend, snapshot)
        self.assertIn('expected_plugin_assets', release)

    def test_actual_collectors_accept_complete_sets_and_reject_missing_files(self):
        workflow = (SCRIPTS.parent / '.github/workflows/build.yml').read_text()
        for full in (True, False):
            title = 'Collect release assets' if full else 'Collect dev snapshot assets'
            section = workflow.split('      - name: ' + title)[1].split('      - name:')[0]
            script = textwrap.dedent(section.split('run: |\n')[1])
            version = 'v1.0.0-beta.3' if full else 'dev-snapshot'
            rows = matrix.select('base', 'create' if full else 'push',
                                 'refs/tags/' + version if full else 'refs/heads/develop')
            names = ['CREXX-' + version + '-' + r['core_platform'] + '.zip'
                     for r in matrix.cores_for(rows)['include']]
            names += ['llama.rexx-' + version + '-' + r['core_platform'] + '-' + r['backend'] + '.zip'
                      for r in rows['include']]
            if not full:
                names += ['CREXX-dev-snapshot-linux-x64.deb', 'CREXX-dev-snapshot-windows-x64-unsigned-setup.exe']
            with tempfile.TemporaryDirectory() as temp:
                root = Path(temp)
                source = root / ('downloaded-release-assets' if full else 'downloaded-dev-snapshot-assets')
                source.mkdir()
                for name in names:
                    (source / name).touch()
                command = ['bash', '-c', script.replace('${{ runner.temp }}', temp)]
                env = dict(os.environ, GITHUB_REF_NAME=version)
                result = subprocess.run(command, env=env, capture_output=True, text=True)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                # Empty prior collector output so a missing input cannot pass
                # because an earlier invocation left a file behind.
                import shutil
                shutil.rmtree(root / ('release-assets' if full else 'dev-snapshot-assets'))
                missing = next(n for n in names if 'cuda' in n) if full else names[-1]
                (source / missing).unlink()
                result = subprocess.run(command, env=env, capture_output=True, text=True)
                self.assertNotEqual(result.returncode, 0)

    def test_delivery_names_unique_and_required_backends_present(self):
        rows = matrix.select('all', 'create', 'refs/tags/v1.0.0')['include']
        self.assertEqual(len({r['artifact_name'] for r in rows}), 6)
        for row in rows:
            self.assertIn('cpu', row['backends'].split(','))
            if row['id'] == 'macos-intel':
                self.assertEqual(row['backend'], 'cpu')
                self.assertEqual(row['backends'], 'cpu')
                self.assertIn('-DCREXX_LLAMA_METAL=OFF', row['cmake_args'])
                self.assertEqual(row['artifact_name'], 'llama.rexx-macos-x86_64-cpu')
            else:
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
