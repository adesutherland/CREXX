"""A packaging retry cannot admit a failed/incomplete product qualification."""
import copy
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('source', Path(__file__).parents[1] / 'resolve-installer-qa-source.py')
source = importlib.util.module_from_spec(spec)
spec.loader.exec_module(source)


class SourceTests(unittest.TestCase):
    def setUp(self):
        self.run = dict(status='completed', conclusion='failure', path='.github/workflows/build.yml')
        self.jobs = [dict(name=name, status='completed', conclusion='success', steps=[])
                     for name in source.REQUIRED_JOBS]
        self.mac = next(j for j in self.jobs if j['name'] == 'Plugin macOS arm64 Metal')
        self.mac.update(conclusion='failure', steps=[
            dict(name=name, conclusion='success') for name in
            ('Smoke combined core and plugin downloads', 'Upload qualified optional plugin',
             'Notarize macOS ZIP asset')])
        self.mac['steps'].append(dict(name=source.MAC_INSTALLER_STEP, conclusion='failure'))

    def test_success_needs_no_exception(self):
        run = dict(self.run, conclusion='success')
        source.validate(run, [])

    def test_known_failure_requires_explicit_retry(self):
        with self.assertRaises(ValueError):
            source.validate(self.run, self.jobs)
        source.validate(self.run, self.jobs, True)

    def test_other_failure_is_rejected(self):
        for name in ('Core windows-x64', 'Plugin Linux x64 CUDA'):
            with self.subTest(name=name):
                jobs = copy.deepcopy(self.jobs)
                next(j for j in jobs if j['name'] == name)['conclusion'] = 'failure'
                with self.assertRaises(ValueError):
                    source.validate(self.run, jobs, True)

    def test_missing_smoke_or_extra_failed_step_is_rejected(self):
        for change in ('smoke', 'extra'):
            with self.subTest(change=change):
                jobs = copy.deepcopy(self.jobs)
                mac = next(j for j in jobs if j['name'] == self.mac['name'])
                if change == 'smoke':
                    mac['steps'][0]['conclusion'] = 'skipped'
                else:
                    mac['steps'].append(dict(name='Something else', conclusion='failure'))
                with self.assertRaises(ValueError):
                    source.validate(self.run, jobs, True)

    def test_incomplete_matrix_is_rejected(self):
        with self.assertRaises(ValueError):
            source.validate(self.run, self.jobs[:-1], True)
        self.mac['conclusion'] = 'skipped'
        with self.assertRaises(ValueError):
            source.validate(self.run, self.jobs, True)

    def test_nonterminal_or_wrong_workflow_is_rejected(self):
        for update in (dict(status='in_progress'), dict(path='other.yml'), dict(conclusion='cancelled')):
            with self.assertRaises(ValueError):
                source.validate(dict(self.run, **update), self.jobs, True)


if __name__ == '__main__':
    unittest.main()
