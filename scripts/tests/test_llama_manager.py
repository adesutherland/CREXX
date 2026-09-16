"""Public native Rexx command lifecycle; requires a prebuilt LLAMA_MANAGER tree."""
import ctypes
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('installer', ROOT / 'scripts/package-llama-installer.py')
installer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(installer)


@unittest.skipUnless(os.environ.get('LLAMA_MANAGER'), 'Set LLAMA_MANAGER to the compiled native tool tree')
class NativeManagerTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='llama native tests ')
        self.addCleanup(self.temp.cleanup)
        self.work = Path(self.temp.name).resolve()
        self.root = self.work / 'cREXX with spaces'
        (self.root / 'bin').mkdir(parents=True)
        (self.root / 'bin/core').write_bytes(b'untouched core')
        self.identity = dict(schema=1, commit='a' * 40, platform='windows-x64', toolchain='msvc')
        self.core = dict(self.identity, component='core', llama_enabled=False,
                         files={'bin/core': installer.digest(self.root / 'bin/core')})
        (self.root / 'core-package.json').write_text(json.dumps(self.core))
        (self.root / 'model.gguf').write_bytes(b'user model')
        self.original = {p.relative_to(self.root): p.read_bytes() for p in self.root.rglob('*') if p.is_file()}
        self.tool = Path(os.environ['LLAMA_MANAGER']).resolve()
        self.exe = 'crexx-llama' + ('.exe' if os.name == 'nt' else '')
        self.command = self.tool / 'bin' / self.exe
        self.stages = {name: self.stage(name) for name in ('vulkan', 'cuda')}

    def stage(self, backend, extra=None):
        plugin = self.work / ('plugin-' + backend)
        stage = self.work / ('stage-' + backend)
        files = {'bin/rxllama.rxplugin': backend.encode(), 'bin/providers/backend.dll': (backend + ' engine').encode()}
        if extra:
            files.update(extra)
        for name, data in files.items():
            path = plugin / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
        metadata = dict(self.identity, component='llama.rexx', backend=backend,
                        files={name: installer.digest(plugin / name) for name in files})
        (plugin / 'llama-package.json').write_text(json.dumps(metadata))
        stage.mkdir()
        installer.prepare(self.root, plugin, stage)
        shutil.copytree(self.tool, stage / 'tool')
        manager = json.loads((stage / 'tool/manager.json').read_text())
        manager['core_commit'] = self.identity['commit']
        (stage / 'tool/manager.json').write_text(json.dumps(manager))
        return stage

    def run_tool(self, action, backend=None, success=True, installed=False, extra=()):
        command = self.root / 'bin' / self.exe if installed else self.command
        args = [str(command), action]
        if backend:
            args.append(backend)
        args += ['--root', str(self.root)]
        if action in ('install', 'check', 'check-core'):
            args += ['--stage', str(self.stages[backend])]
        result = subprocess.run(args + list(extra), capture_output=True, text=True, timeout=180)
        self.assertEqual(result.returncode == 0, success, result.stdout + result.stderr)
        self.assertNotIn('PANIC:', result.stdout + result.stderr)
        return result.stdout

    def assert_core(self):
        for path, value in self.original.items():
            self.assertEqual((self.root / path).read_bytes(), value)

    def test_coexist_switch_reinstall_and_independent_removal(self):
        self.run_tool('install', 'vulkan')
        self.run_tool('install', 'cuda')
        self.assertIn('Active backend: vulkan', self.run_tool('status', installed=True))
        self.run_tool('use', 'cuda', installed=True)
        self.assertEqual((self.root / 'bin/rxllama.rxplugin').read_bytes(), b'cuda')
        self.run_tool('use', 'cuda', installed=True)
        self.run_tool('install', 'vulkan')
        self.assertIn('Active backend: cuda', self.run_tool('status', installed=True))
        self.run_tool('remove', 'vulkan')
        self.assertTrue((self.root / 'bin' / self.exe).is_file())
        self.run_tool('remove', 'cuda')
        self.assertFalse((self.root / 'bin' / self.exe).exists())
        self.assertFalse((self.root / 'bin/rxllama.rxplugin').exists())
        self.assert_core()

    def test_removing_active_does_not_select_other(self):
        self.run_tool('install', 'cuda')
        self.run_tool('install', 'vulkan')
        self.run_tool('remove', 'cuda')
        self.assertIn('Active backend: none', self.run_tool('status', installed=True))
        self.run_tool('use', 'vulkan', installed=True)
        self.assert_core()

    def test_active_reinstall_preserves_every_projected_file(self):
        # A real backend has many more files than the shared manager. Exercise
        # both directory depth and the shrinking manifest iteration count.
        for directory in ('plugin-vulkan', 'stage-vulkan'):
            shutil.rmtree(self.work / directory)
        self.stages['vulkan'] = self.stage('vulkan', {
            'bin/providers/extra-' + str(i) + '.dll': b'backend dependency'
            for i in range(12)
        })
        self.run_tool('install', 'vulkan')
        self.run_tool('install', 'vulkan')
        metadata = json.loads((self.stages['vulkan'] / 'installer.json').read_text())
        for name, expected in metadata['plugin_files'].items():
            self.assertEqual(installer.digest(self.root / name), expected, name)
        self.run_tool('remove', 'vulkan')
        self.assert_core()

    def test_installer_activation_option(self):
        self.run_tool('install', 'vulkan')
        self.run_tool('install', 'cuda', extra=['--activate'])
        self.assertIn('Active backend: cuda', self.run_tool('status', installed=True))

    def test_changed_core_and_payload_rejected(self):
        path = self.root / 'bin/core'
        path.write_bytes(b'changed')
        self.run_tool('install', 'vulkan', success=False)
        path.write_bytes(self.original[Path('bin/core')])
        (self.stages['vulkan'] / 'payload/bin/rxllama.rxplugin').write_bytes(b'changed')
        self.run_tool('install', 'vulkan', success=False)
        self.assertFalse((self.root / 'bin' / self.exe).exists())

    def test_unowned_file_preserved(self):
        path = self.root / 'bin/rxllama.rxplugin'
        path.write_bytes(b'mine')
        self.run_tool('install', 'vulkan', success=False)
        self.assertEqual(path.read_bytes(), b'mine')

    def test_unknown_backend_rejected(self):
        self.run_tool('install', 'vulkan')
        self.assertIn('Backend is not installed: cuda', self.run_tool('use', 'cuda', success=False, installed=True))
        self.assertIn('Active backend: vulkan', self.run_tool('status', installed=True))

    def test_tampered_store_rejected(self):
        self.run_tool('install', 'vulkan')
        self.run_tool('install', 'cuda')
        (self.root / '.llama-backends/cuda/payload/bin/rxllama.rxplugin').write_bytes(b'changed')
        self.run_tool('use', 'cuda', success=False, installed=True)
        self.assertEqual((self.root / 'bin/rxllama.rxplugin').read_bytes(), b'vulkan')

    def test_failed_publication_rolls_back(self):
        shutil.rmtree(self.work / 'plugin-cuda')
        shutil.rmtree(self.stages['cuda'])
        self.stages['cuda'] = self.stage('cuda', {'bin/blocked/library': b'new'})
        self.run_tool('install', 'vulkan')
        self.run_tool('install', 'cuda')
        (self.root / 'bin/blocked').write_bytes(b'user-owned')
        output = self.run_tool('use', 'cuda', success=False, installed=True)
        self.assertIn('Previous installation restored.', output)
        self.assertEqual((self.root / 'bin/rxllama.rxplugin').read_bytes(), b'vulkan')
        self.assertEqual((self.root / 'bin/blocked').read_bytes(), b'user-owned')
        self.assertFalse(list(self.root.glob('.llama-transaction-*')))
        self.assert_core()

    def hold(self, path, share=0):
        if os.name == 'nt':
            kernel = ctypes.WinDLL('kernel32', use_last_error=True)
            kernel.CreateFileW.argtypes = [ctypes.c_wchar_p, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_void_p]
            kernel.CreateFileW.restype = ctypes.c_void_p
            kernel.CloseHandle.argtypes = [ctypes.c_void_p]
            handle = kernel.CreateFileW(str(path), 0x80000000, share, None, 3, 0x80, None)
            self.assertNotEqual(handle, ctypes.c_void_p(-1).value)
            self.addCleanup(kernel.CloseHandle, handle)
        else:
            import fcntl
            stream = path.open('r+b')
            fcntl.flock(stream, fcntl.LOCK_EX | fcntl.LOCK_NB)
            self.addCleanup(stream.close)

    def test_in_use_files_refuse_switch(self):
        self.run_tool('install', 'vulkan')
        self.run_tool('install', 'cuda')
        self.hold(self.root / 'bin/rxllama.rxplugin', share=1)
        output = self.run_tool('use', 'cuda', success=False, installed=True)
        self.assertIn('in use or not writable', output)
        self.assertEqual((self.root / 'bin/rxllama.rxplugin').read_bytes(), b'vulkan')

    def test_concurrent_mutation_refused(self):
        lock = self.root / '.llama-operation.lock'
        lock.touch()
        self.hold(lock)
        output = self.run_tool('install', 'vulkan', success=False)
        self.assertIn('Another plugin operation', output)

    def test_interrupted_transaction_refused(self):
        (self.root / '.llama-transaction-interrupted').mkdir()
        self.run_tool('install', 'vulkan', success=False)
        self.assertFalse((self.root / 'bin/rxllama.rxplugin').exists())

    def test_unknown_management_schema_refused(self):
        self.run_tool('install', 'vulkan')
        path = self.root / '.llama-installer/manager.json'
        data = json.loads(path.read_text())
        data['schema'] = 2
        path.write_text(json.dumps(data))
        self.run_tool('install', 'cuda', success=False)
        self.assertEqual((self.root / 'bin/rxllama.rxplugin').read_bytes(), b'vulkan')

    def test_removal_protects_upgraded_core_ownership(self):
        self.run_tool('install', 'vulkan')
        self.core['files']['bin/rxllama.rxplugin'] = installer.digest(self.root / 'bin/rxllama.rxplugin')
        (self.root / 'core-package.json').write_text(json.dumps(self.core))
        self.run_tool('remove', 'vulkan', success=False)
        self.assertTrue((self.root / 'bin/rxllama.rxplugin').is_file())


if __name__ == '__main__':
    unittest.main()
