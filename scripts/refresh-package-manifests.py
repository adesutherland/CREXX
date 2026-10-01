"""Maintainer signing only: preserve identities/file sets, refresh final hashes.

Verify the original payload before signing. Refresh only after verifying all
signatures; never install this as a consumer repair or runtime bypass.
"""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def update(root, verify=False):
    root = Path(root).resolve()
    pending = []
    for name in ('core-package.json', 'llama-package.json', 'manager.json'):
        path = root / name
        if not path.exists():
            continue
        data = json.loads(path.read_text())
        for key in ('files', 'bootstrap_files'):
            for relative, expected in data.get(key, {}).items():
                local = PurePosixPath(relative)
                target = root / relative
                if (not relative or local.is_absolute() or '..' in local.parts or
                        '\\' in relative or ':' in relative or
                        not target.resolve().is_relative_to(root) or not target.is_file()):
                    raise ValueError('Missing/nonlocal package file: ' + relative)
                actual = digest(target)
                if verify and actual != expected:
                    raise ValueError('Package file changed: ' + relative)
                data[key][relative] = actual
        pending.append((path, data))
    # Validate every manifest before writing any of them.
    if not verify:
        for path, data in pending:
            path.write_text(json.dumps(data, indent=2) + '\n')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('root', type=Path)
    parser.add_argument('--verify', action='store_true')
    args = parser.parse_args()
    update(args.root, args.verify)
