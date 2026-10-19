"""Narrow regressions for safe transient ownership and permanent-input classification."""
import importlib.util
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]

def load(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module

workspace = load('qa_workspace', 'tools/qa_workspace.py')
hygiene = load('source_hygiene', 'scripts/check-source-hygiene.py')

class CleanupTests(unittest.TestCase):
    def test_only_owned_workspace_removed(self):
        with tempfile.TemporaryDirectory() as parent:
            sibling = Path(parent) / 'active-work'
            sibling.mkdir()
            (sibling / 'keep').write_text('active')
            run = workspace.create(parent)
            (run / 'diagnostic.log').write_text('failed command')
            workspace.cleanup(run)
            self.assertFalse(run.exists())
            self.assertEqual((sibling / 'keep').read_text(), 'active')
            with self.assertRaises((ValueError, FileNotFoundError)):
                workspace.cleanup(sibling)

    def test_symlink_and_moved_marker_rejected(self):
        with tempfile.TemporaryDirectory() as parent:
            run = workspace.create(parent)
            alias = Path(parent) / 'crexx-qa-link'
            try:
                alias.symlink_to(run, target_is_directory=True)
            except OSError:
                self.skipTest('host does not permit symlinks')
            with self.assertRaises(ValueError):
                workspace.cleanup(alias)
            alias.unlink()
            moved = Path(parent) / 'crexx-qa-renamed'
            run.rename(moved)
            with self.assertRaises(ValueError):
                workspace.cleanup(moved)
            self.assertTrue((moved / workspace.MARKER).exists())

    def test_failure_and_interruption_leave_inspectable_work(self):
        # A killed/failing producer cannot make another process erase its evidence.
        with tempfile.TemporaryDirectory() as parent:
            for exit_code in (1, 130, 143):
                program = ('import sys; sys.path.insert(0,sys.argv[1]); '
                           'import qa_workspace; p=qa_workspace.create(sys.argv[2]); '
                           '(p/"failure.log").write_text("diagnostic"); print(p); '
                           'sys.exit(int(sys.argv[3]))')
                result = subprocess.run([sys.executable, '-c', program, str(ROOT/'tools'),
                                         parent, str(exit_code)], capture_output=True, text=True)
                self.assertEqual(result.returncode, exit_code)
                run = Path(result.stdout.strip())
                self.assertEqual((run/'failure.log').read_text(), 'diagnostic')
                workspace.cleanup(run)
                self.assertFalse(run.exists())

    def test_terminated_producer_leaves_inspectable_work(self):
        with tempfile.TemporaryDirectory() as parent:
            program = ('import sys,time; sys.path.insert(0,sys.argv[1]); '
                       'import qa_workspace; p=qa_workspace.create(sys.argv[2]); '
                       '(p/"failure.log").write_text("diagnostic"); '
                       'print(p,flush=True); time.sleep(60)')
            child = subprocess.Popen([sys.executable, '-c', program, str(ROOT/'tools'), parent],
                                     stdout=subprocess.PIPE, text=True)
            try:
                run = Path(child.stdout.readline().strip())
                child.terminate()
                child.wait(timeout=10)
                self.assertNotEqual(child.returncode, 0)
                self.assertEqual((run/'failure.log').read_text(), 'diagnostic')
                workspace.cleanup(run)
                self.assertFalse(run.exists())
            finally:
                if child.poll() is None:
                    child.kill()
                    child.wait()
                child.stdout.close()

    def test_hygiene_preserves_real_inputs(self):
        fixtures = ['compiler/tests/golden/run/empty.out', 'tests/invalid.rxas',
                    'third_party/re2c/test/parser.log', 'tests/fixtures/invalid.log',
                    'compiler/tests/golden/diagnostic.log', 'tests/native-inference/fixtures/model.gguf',
                    'lib/rxfnsl/unicode/data/UnicodeData.txt', 'docs/books/figure.pdf']
        self.assertEqual(hygiene.violations(fixtures), [])
        bad = ['docs/qa/new-run/input.crexx', 'performance/evidence/new/results.csv',
               'concurrency/evidence/session.md', 'compiler/build.log']
        self.assertEqual(hygiene.violations(bad), sorted(bad))

if __name__ == '__main__':
    unittest.main()
