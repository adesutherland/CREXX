#!/usr/bin/env python3
"""Own transient QA captures, never an archive or a caller's build directory."""
import argparse
import json
from pathlib import Path
import shutil
import tempfile

MARKER = '.crexx-qa-workspace.json'

def create(parent=None):
    directory = Path(tempfile.mkdtemp(prefix='crexx-qa-', dir=parent)).resolve()
    (directory / MARKER).write_text(json.dumps({'version': 1, 'path': str(directory)}))
    return directory

def cleanup(directory):
    directory = Path(directory).absolute()
    if directory.is_symlink() or directory.resolve() != directory:
        raise ValueError('cleanup requires the original workspace path')
    marker = directory / MARKER
    if marker.is_symlink() or not directory.name.startswith('crexx-qa-'):
        raise ValueError('not an owned QA workspace')
    if json.loads(marker.read_text()) != {'version': 1, 'path': str(directory)}:
        raise ValueError('workspace identity does not match')
    shutil.rmtree(directory)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    commands.add_parser('create').add_argument('--parent', type=Path)
    commands.add_parser('cleanup').add_argument('directory', type=Path)
    args = parser.parse_args()
    if args.command == 'create':
        print(create(args.parent))
    else:
        cleanup(args.directory)

if __name__ == '__main__':
    main()
