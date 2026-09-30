"""Native QA regression for pwsh -> Python -> Windows PowerShell module paths."""
import importlib.util
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch


@unittest.skipUnless(sys.platform == 'win32', 'Requires native Windows Authenticode')
class WindowsSignatureTests(unittest.TestCase):
    def test_module_shadowing_and_signed_unsigned_controls(self):
        source = os.environ.get('WINDOWS_SIGNED_CONTROL')
        self.assertTrue(source, 'Set WINDOWS_SIGNED_CONTROL to a retained signed setup')
        spec = importlib.util.spec_from_file_location(
            'windows_installer_qa',
            Path(__file__).resolve().parents[1] / 'test-llama-windows-installer.py')
        qa = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(qa)
        with tempfile.TemporaryDirectory(prefix='Windows signature control ') as tmp:
            root = Path(tmp)
            module = root / 'modules/Microsoft.PowerShell.Security'
            module.mkdir(parents=True)
            (module / 'Microsoft.PowerShell.Security.psd1').write_text(
                "@{ RootModule='shadow.psm1'; ModuleVersion='1.0'; "
                "FunctionsToExport=@('Get-AuthenticodeSignature') }")
            (module / 'shadow.psm1').write_text("throw 'INCOMPATIBLE_MODULE_CONTROL'")
            payload = root / 'payload'
            payload.mkdir()
            shutil.copyfile(source, payload / 'signed.exe')
            with patch.dict(os.environ, {'PSModulePath': str(module.parent)}):
                broken = subprocess.run(
                    ['powershell.exe', '-NoProfile', '-NonInteractive', '-Command',
                     '$ErrorActionPreference="Stop"; '
                     'Import-Module Microsoft.PowerShell.Security -ErrorAction Stop'],
                    text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                    timeout=60)
                self.assertNotEqual(broken.returncode, 0, broken.stdout)
                self.assertIn('INCOMPATIBLE_MODULE_CONTROL', broken.stdout)
                print('PASS: inherited module shadowing reproduces the failure', flush=True)
                qa.verify_signatures(payload)
                (payload / 'unsigned.exe').write_bytes(b'MZ unsigned negative control')
                with self.assertRaises(subprocess.CalledProcessError):
                    qa.verify_signatures(payload)
                print('PASS: clean child accepts signed and rejects unsigned payload', flush=True)


if __name__ == '__main__':
    unittest.main()
