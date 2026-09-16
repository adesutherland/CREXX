import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch
import zipfile


def module(name):
    path = Path(__file__).resolve().parents[1] / (name + '.py')
    spec = importlib.util.spec_from_file_location(name, path)
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


core = module('package-core-release')
plugin = module('package-llama-release')


class SplitPackageTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.base = self.root / 'CREXX-platform'
        (self.base / 'bin').mkdir(parents=True)
        (self.base / 'bin/rxbvm').write_bytes(b'qualified executable')
        self.manifest = dict(schema=1, component='core', commit='a' * 40,
            platform='windows-x64', toolchain='msvc', llama_enabled=False,
            files={'bin/rxbvm': core.digest(self.base / 'bin/rxbvm')})
        (self.base / 'core-package.json').write_text(json.dumps(self.manifest))

    def verify(self, **changes):
        values = {k:self.manifest[k] for k in ('commit', 'platform', 'toolchain')}
        values.update(changes)
        return plugin.verify_core(self.base, **values)

    def test_matching_core_passes_but_other_commit_or_toolchain_does_not(self):
        self.verify()
        for changes in [dict(commit='b' * 40), dict(toolchain='mingw'), dict(platform='linux-x64')]:
            with self.subTest(changes=changes), self.assertRaisesRegex(ValueError, 'identity mismatch'):
                self.verify(**changes)

    def test_changed_qualified_binary_is_rejected(self):
        (self.base / 'bin/rxbvm').write_bytes(b'other executable')
        with self.assertRaisesRegex(ValueError, 'Core file changed'):
            self.verify()

    def test_plugin_cannot_replace_core_with_different_bytes(self):
        source = self.root / 'runtime.dll'
        source.write_bytes(b'changed bytes')
        with self.assertRaisesRegex(ValueError, 'replace a qualified core'):
            plugin.add_file(source, Path('bin/rxbvm'), self.root/'plugin', self.base)
        self.verify()

    def test_shared_identical_runtime_is_not_duplicated(self):
        target = self.root / 'plugin'
        plugin.add_file(self.base/'bin/rxbvm', Path('bin/rxbvm'), target, self.base)
        self.assertFalse(target.exists())
        self.verify()

    def test_msvc_runtime_preflight_accepts_shared_bytes_and_rejects_missing_or_changed(self):
        sdk = self.root / 'redist'
        sdk.mkdir()
        runtime = sdk / 'concrt140.dll'
        runtime.write_bytes(b'qualified CRT')
        qualified = self.base / 'bin' / runtime.name
        qualified.write_bytes(runtime.read_bytes())
        build = self.root / 'build'
        inventory = build / 'lib/plugins/llama/tests/release-smoke-msvc-runtime-files.txt'
        inventory.parent.mkdir(parents=True)
        inventory.write_text(str(runtime) + '\n')
        plugin.verify_msvc_runtime(build, self.base)
        runtime.write_bytes(b'different CRT')
        with self.assertRaisesRegex(ValueError, 'runtime differs'):
            plugin.verify_msvc_runtime(build, self.base)
        qualified.unlink()
        with self.assertRaisesRegex(ValueError, 'runtime differs'):
            plugin.verify_msvc_runtime(build, self.base)
        inventory.write_text('')
        with self.assertRaisesRegex(ValueError, 'inventory is empty'):
            plugin.verify_msvc_runtime(build, self.base)

    def test_selected_msvc_redist_uses_actual_crt_name_and_rejects_ambiguity(self):
        redist = self.root / '14.51' / 'x64'
        crt = redist / 'Microsoft.VC145.CRT'
        crt.mkdir(parents=True)
        (crt / 'concrt140.dll').write_bytes(b'active runtime')
        helper = Path(__file__).resolve().parents[2] / 'cmake/CrexxMSVCRuntime.cmake'
        script = self.root / 'selection.cmake'
        result = self.root / 'selected.txt'
        script.write_text(f'include("{helper.as_posix()}")\n'
            f'crexx_msvc_runtime_files(files "{redist.parent.as_posix()}" x64)\n'
            f'file(WRITE "{result.as_posix()}" "${{files}}")\n')
        selected = subprocess.run(['cmake', '-P', str(script)], capture_output=True, text=True)
        self.assertEqual(selected.returncode, 0, selected.stderr)
        self.assertEqual(result.read_text(), (crt / 'concrt140.dll').as_posix())
        (redist / 'Microsoft.VC143.CRT').mkdir()
        rejected = subprocess.run(['cmake', '-P', str(script)], capture_output=True, text=True)
        self.assertNotEqual(rejected.returncode, 0)
        self.assertIn('Expected one MSVC CRT directory', rejected.stderr)

    @unittest.skipIf(os.name == 'nt', 'Unix executable and symlink control')
    def test_zip_preserves_public_vm_link_and_execute_permission(self):
        (self.base/'bin/rxbvm').chmod(0o755)
        (self.base/'bin/rxvm').symlink_to('rxbvm')
        archive = self.root/'core.zip'
        core.archive(self.base, archive)
        core.extract(archive, self.root/'extracted')
        binary = self.root/'extracted/CREXX-platform/bin/rxvm'
        self.assertEqual(os.readlink(binary), 'rxbvm')
        self.assertTrue(os.access(binary, os.X_OK))
        self.assertEqual(binary.read_bytes(), b'qualified executable')

    def test_nonlocal_archive_entries_are_rejected(self):
        archive = self.root/'bad.zip'
        with zipfile.ZipFile(archive, 'w') as z:
            z.writestr('../escape', 'invalid')
        with self.assertRaisesRegex(ValueError, 'Nonlocal archive'):
            core.extract(archive, self.root/'extracted')

    def test_windows_restricted_path_handles_environment_key_casing(self):
        for system_key, path_key in [('SYSTEMROOT', 'PATH'), ('SystemRoot', 'Path')]:
            env = {system_key: 'C:\\Windows', path_key: 'compiler-and-sdk-path', 'TEMP': 'temporary'}
            runtime = core.windows_execution_environment(env)
            self.assertEqual(runtime['PATH'], 'C:\\Windows\\System32;C:\\Windows')
            self.assertEqual(runtime['TEMP'], 'temporary')
            self.assertEqual([k for k in runtime if k.upper() == 'PATH'], ['PATH'])
            self.assertEqual(env[path_key], 'compiler-and-sdk-path')

    def stage_provider(self):
        renamed = self.root / 'CREXX-windows-x64'
        self.base.rename(renamed)
        self.base = renamed
        archive = self.root / 'core.zip'
        core.archive(self.base, archive)
        build = self.root / 'build'
        providers = build / 'bin/providers'
        providers.mkdir(parents=True)
        entries = []
        for backend in ('cpu', 'vulkan'):
            file = providers / (backend + '.dll')
            file.write_bytes((backend + ' unsigned library').encode())
            entries.append(dict(path=file.name, sha256=core.digest(file), backend=backend))
        identity = dict(version=1, provider='rxllama', platform='Windows', arch='AMD64', engine='pin')
        runtime = providers / 'rxllama.runtime.json'
        runtime.write_text(json.dumps(dict(identity, backends=entries)))
        (providers / 'rxllama.native.json').write_text(json.dumps(dict(identity,
            link_libraries=entries[:1], runtime_files=entries + [dict(path=runtime.name, sha256=core.digest(runtime))])))
        for name in ('rxllama.rxplugin', 'crexx-provider-package.exe'):
            (build / 'bin' / name).write_bytes(b'plugin input')
        inventory = build / 'lib/plugins/llama/tests/release-smoke-bootstrap-files.txt'
        inventory.parent.mkdir(parents=True)
        inventory.write_text('')
        self.output = self.root / 'output'
        self.arguments = ['package-llama-release.py', '--core-archive', str(archive),
            '--build', str(build), '--output', str(self.output), '--commit', 'a' * 40,
            '--platform', 'windows-x64', '--toolchain', 'msvc', '--backend', 'vulkan']
        with patch('sys.argv', self.arguments + ['--stage-only']), patch.object(plugin.subprocess, 'run') as install:
            plugin.main()
            self.assertIn('llama-docs', install.call_args.args[0])
        self.providers = self.output / 'plugin-stage/CREXX-windows-x64/bin/providers'

    def test_signed_plugin_finalization_verifies_both_actual_archives(self):
        self.stage_provider()
        file = self.providers / 'vulkan.dll'
        file.write_bytes(file.read_bytes() + b' verified signature')
        module('refresh-provider-manifests').refresh(self.providers)
        with patch('sys.argv', self.arguments + ['--finalize-only']):
            plugin.main()
        combined = self.output / 'combined/CREXX-windows-x64'
        self.assertEqual((combined / 'bin/rxbvm').read_bytes(), b'qualified executable')
        self.assertEqual((combined / 'bin/providers/vulkan.dll').read_bytes(), file.read_bytes())
        with zipfile.ZipFile(next((self.output / 'assets').glob('*.zip'))) as archive:
            self.assertNotIn('CREXX-windows-x64/bin/rxbvm', archive.namelist())

    def test_finalization_rejects_changed_provider_without_refreshed_hashes(self):
        self.stage_provider()
        (self.providers / 'vulkan.dll').write_bytes(b'changed after staging')
        with patch('sys.argv', self.arguments + ['--finalize-only']), self.assertRaisesRegex(ValueError, 'Provider hash mismatch'):
            plugin.main()

    def test_cpu_only_package_rejects_gpu_backend_then_accepts_cpu(self):
        self.stage_provider()
        with self.assertRaisesRegex(ValueError, 'Unexpected GPU'):
            plugin.verify_provider(self.providers, 'cpu')
        runtime = self.providers / 'rxllama.runtime.json'
        data = json.loads(runtime.read_text())
        data['backends'] = [e for e in data['backends'] if e['backend'] == 'cpu']
        runtime.write_text(json.dumps(data))
        module('refresh-provider-manifests').refresh(self.providers)
        plugin.verify_provider(self.providers, 'cpu')

    def test_finalization_rejects_accidental_core_overlap_after_staging(self):
        self.stage_provider()
        (self.providers.parent / 'rxbvm').write_bytes(b'core overwrite')
        with patch('sys.argv', self.arguments + ['--finalize-only']), self.assertRaisesRegex(ValueError, 'overlaps a core file'):
            plugin.main()


if __name__ == '__main__':
    unittest.main()
