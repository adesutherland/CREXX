import importlib.util
import json
import os
from pathlib import Path
import tempfile
import unittest
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


if __name__ == '__main__':
    unittest.main()
