#!/usr/bin/env python3
"""Reject disposable tracked output; authored fixtures and vendored trees are inputs."""
from pathlib import Path
import subprocess
import sys

OUTPUT_ROOTS = ('docs/qa/', 'performance/evidence/', 'concurrency/evidence/')
OBSOLETE_ROOTS = ('bin/', 'demos/', 'experiments/', 'Utilities/', 'tools/docker_scripts/')

def fixture(path):
    parts = Path(path).parts
    return 'tests' in parts and any(part in ('fixtures', 'golden') for part in parts)

def violations(paths):
    return sorted(p for p in paths if p.startswith(OUTPUT_ROOTS + OBSOLETE_ROOTS)
                  or (p.endswith('.log') and not p.startswith('third_party/') and not fixture(p)))

def main():
    root = Path(__file__).resolve().parents[1]
    paths = subprocess.check_output(['git', 'ls-files', '-z'], cwd=root).decode().split('\0')
    # Pending tracked deletions are absent; untracked work belongs to its owner.
    errors = violations(p for p in paths if p and (root / p).exists())
    if errors:
        print('Disposable output or retired source owners are tracked:\n' + '\n'.join(errors))
        return 1
    print('PASS: tracked source hygiene; fixtures and upstream inputs preserved')
    return 0

if __name__ == '__main__':
    sys.exit(main())
