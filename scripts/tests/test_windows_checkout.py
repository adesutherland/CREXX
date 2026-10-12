"""Reject tracked paths which prevent Git for Windows from checking out the tree."""
from pathlib import Path
import re
import subprocess
import unittest


ROOT = Path(__file__).resolve().parents[2]
DEVICE = re.compile(r"^(CON|PRN|AUX|NUL|COM[1-9¹²³]|LPT[1-9¹²³])(?:\.|$)", re.I)


def invalid_windows_path(path):
    return any(
        DEVICE.match(part) or part.endswith((".", " "))
        or any(char in '<>:"\\|?*' or ord(char) < 32 for char in part)
        for part in path.split("/")
    )


class CheckoutPaths(unittest.TestCase):
    def test_device_names_with_extensions_and_directory_components(self):
        for path in ("nul.txt", "evidence/NUL.txt", "con/data", "aux", "COM1.log",
                     "Lpt9.bin", "COM¹.txt", "file.", "file ", "a:b", "a\\b"):
            with self.subTest(path=path):
                self.assertTrue(invalid_windows_path(path))

    def test_safe_fixture_names(self):
        for path in ("nul-byte.txt", "input-nul.txt", "nullable.levell",
                     "evidence/café.txt", "com10.txt"):
            with self.subTest(path=path):
                self.assertFalse(invalid_windows_path(path))

    def test_tracked_paths_are_portable(self):
        paths = subprocess.check_output(
            ["git", "ls-files", "-z"], cwd=ROOT
        ).decode("utf-8").split("\0")
        invalid = sorted(path for path in paths if path and invalid_windows_path(path))
        self.assertEqual(invalid, [], "Rename these Windows-incompatible tracked paths")


if __name__ == "__main__":
    unittest.main()
