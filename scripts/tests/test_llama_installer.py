"""Focused installer controls; no engine build or real model download."""
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('installer', ROOT / 'scripts/package-llama-installer.py')
installer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(installer)


@unittest.skipUnless(sys.platform == 'darwin', 'Mac lifecycle controls; Windows INST-AC coverage awaits CI-D04 coexistence decision')
class InstallerTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='llama installer tests ')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.core = self.root / 'cREXX with spaces'
        self.plugin = self.root / 'plugin'
        self.stage = self.root / 'stage'
        self.stage.mkdir()
        windows = os.name == 'nt'
        identity = dict(schema=1, commit='a' * 40,
                        platform='windows-x64' if windows else 'macos-arm64',
                        toolchain='msvc' if windows else 'clang')
        for path, data in [(self.core / 'bin/core', b'core'),
                           (self.plugin / 'bin/plugin', b'plugin'),
                           (self.plugin / 'bin/providers/backend', b'backend')]:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
        self.base = dict(identity, component='core', llama_enabled=False,
                         files={'bin/core': installer.digest(self.core / 'bin/core')})
        self.addon = dict(identity, component='llama.rexx',
                          backend='vulkan' if windows else 'metal',
                          files={p.relative_to(self.plugin).as_posix(): installer.digest(p)
                                 for p in self.plugin.rglob('*') if p.is_file()})
        self.manifests()
        installer.prepare(self.core, self.plugin, self.stage)
        self.extension = 'ps1' if windows else 'sh'
        shutil.copy2(ROOT / 'packaging/llama' / ('manage.' + self.extension), self.stage)
        self.original = {p.relative_to(self.core): p.read_bytes() for p in self.core.rglob('*') if p.is_file()}

    def manifests(self):
        (self.core / 'core-package.json').write_text(json.dumps(self.base))
        (self.plugin / 'llama-package.json').write_text(json.dumps(self.addon))

    def run_helper(self, action, expected=0, stage=None, env=None):
        helper = (stage or self.stage) / ('manage.' + self.extension)
        if os.name == 'nt':
            args = ['powershell.exe', '-NoProfile', '-NonInteractive', '-ExecutionPolicy', 'Bypass',
                    '-File', str(helper), '-Action', action, '-Root', str(self.core)]
        else:
            args = ['/bin/sh', str(helper), action, str(self.core)]
        result = subprocess.run(args, env=env, text=True, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, timeout=120)
        self.assertEqual(result.returncode == 0, expected == 0, result.stdout)
        return result.stdout

    def check_core(self):
        for name, value in self.original.items():
            self.assertEqual((self.core / name).read_bytes(), value)

    def test_install_reinstall_remove_preserves_core_and_models(self):
        (self.core / 'my-model.gguf').write_bytes(b'user data')
        self.run_helper('check')
        self.run_helper('install')
        self.run_helper('install')
        self.assertEqual((self.core / 'bin/plugin').read_bytes(), b'plugin')
        self.run_helper('remove', stage=self.core / '.llama-installer')
        self.assertFalse((self.core / 'bin/plugin').exists())
        self.assertEqual((self.core / 'my-model.gguf').read_bytes(), b'user data')
        self.check_core()

    def test_altered_core_rejected_before_copy(self):
        (self.core / 'bin/core').write_bytes(b'changed')
        self.run_helper('install', expected=1)
        self.assertFalse((self.core / 'bin/plugin').exists())

    def test_missing_core_rejected(self):
        (self.core / 'core-package.json').unlink()
        self.run_helper('install', expected=1)
        self.assertFalse((self.core / 'bin/plugin').exists())

    def test_changed_payload_rejected(self):
        (self.stage / 'payload/bin/plugin').write_bytes(b'changed')
        self.run_helper('install', expected=1)
        self.assertFalse((self.core / 'bin/plugin').exists())

    def test_unowned_file_preserved(self):
        (self.core / 'bin/plugin').write_bytes(b'mine')
        self.run_helper('install', expected=1)
        self.assertEqual((self.core / 'bin/plugin').read_bytes(), b'mine')

    def test_changed_installed_plugin_not_removed(self):
        self.run_helper('install')
        (self.core / 'bin/plugin').write_bytes(b'changed')
        self.run_helper('remove', expected=1, stage=self.core / '.llama-installer')
        self.assertEqual((self.core / 'bin/plugin').read_bytes(), b'changed')
        self.check_core()

    def test_mismatched_identity_rejected_at_packaging(self):
        self.addon['commit'] = 'b' * 40
        self.manifests()
        with self.assertRaisesRegex(ValueError, 'identity mismatch'):
            installer.prepare(self.core, self.plugin, self.root / 'other')

    def test_core_overlap_rejected_at_packaging(self):
        shutil.copy2(self.core / 'bin/core', self.plugin / 'bin/core')
        self.addon['files']['bin/core'] = self.base['files']['bin/core']
        self.manifests()
        with self.assertRaisesRegex(ValueError, 'overlaps'):
            installer.prepare(self.core, self.plugin, self.root / 'other')

    @unittest.skipIf(os.name == 'nt', 'Mac shell rollback control')
    def test_partial_copy_failure_restores_previous_plugin(self):
        self.run_helper('install')
        mock = self.root / 'mock'
        mock.mkdir()
        # Fail only during publication, not backup or rollback; no production
        # failure-injection flag is needed.
        cp = mock / 'cp'
        cp.write_text('#!/bin/sh\ncase "$*" in *payload/bin/providers/backend*) exit 1;; esac\nexec /bin/cp "$@"\n')
        cp.chmod(0o755)
        self.run_helper('install', expected=1, env=dict(os.environ, PATH=str(mock) + ':' + os.environ['PATH']))
        self.assertEqual((self.core / 'bin/plugin').read_bytes(), b'plugin')
        self.assertEqual((self.core / 'bin/providers/backend').read_bytes(), b'backend')
        self.run_helper('remove', stage=self.core / '.llama-installer')
        self.check_core()

    @unittest.skipIf(os.name == 'nt', 'Mac symlink control')
    def test_destination_symlink_rejected(self):
        elsewhere = self.root / 'elsewhere'
        elsewhere.mkdir()
        (self.core / 'bin/providers').symlink_to(elsewhere, target_is_directory=True)
        self.run_helper('install', expected=1)
        self.assertEqual(list(elsewhere.iterdir()), [])

    @unittest.skipIf(os.name == 'nt', 'Mac discovery control')
    def test_discovery_and_ambiguous_locations(self):
        env = dict(os.environ, CREXX_HOME=str(self.core), REXX_HOME=str(self.core))
        command = ['/bin/sh', str(self.stage / 'manage.sh'), 'check']
        result = subprocess.run(command, env=env, capture_output=True, text=True, timeout=120)
        self.assertEqual(result.returncode, 0, result.stderr)
        other = self.root / 'other core'
        shutil.copytree(self.core, other)
        env['REXX_HOME'] = str(other)
        result = subprocess.run(command, env=env, capture_output=True, text=True, timeout=120)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('No unique', result.stderr)

    def test_overlapping_installer_operations_rejected(self):
        if os.name == 'nt':
            self.skipTest('Windows named mutex checked in hosted lifecycle')
        (self.core / '.llama-install-lock').mkdir()
        self.run_helper('install', expected=1)
        self.assertFalse((self.core / 'bin/plugin').exists())


if __name__ == '__main__':
    unittest.main()
