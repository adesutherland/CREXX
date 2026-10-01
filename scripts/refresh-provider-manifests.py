"""Build/signing step only: rehash the unchanged declared provider file set.

Run after verifying signatures on a trusted staged payload. This is deliberately
not installed as a user command: runtime verification must never repair hashes.
"""
import argparse
import hashlib
import json
from pathlib import Path


def digest(path):
    value = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            value.update(chunk)
    return value.hexdigest()


def refresh(directory):
    for native_path in sorted(directory.glob('*.native.json')):
        native = json.loads(native_path.read_text())
        runtime_path = directory / (native['provider'] + '.runtime.json')
        if not runtime_path.exists():
            raise RuntimeError('Missing provider runtime manifest: ' + str(runtime_path))
        runtime = json.loads(runtime_path.read_text())
        for key in ('version', 'provider', 'platform', 'arch', 'engine'):
            if runtime[key] != native[key]:
                raise RuntimeError('Inconsistent provider identity: ' + key)
        def rehash(entries):
            for entry in entries:
                path = directory / entry['path']
                if path.resolve().parent != directory.resolve() or not path.is_file():
                    raise RuntimeError('Missing/nonlocal declared provider file: ' + str(path))
                entry['sha256'] = digest(path)
        # Verify all declared paths before mutating either manifest.
        rehash(native['link_libraries'] + native['runtime_files'])
        rehash(runtime['backends'])
        runtime_path.write_text(json.dumps(runtime, separators=(',', ':')) + '\n')
        rehash(native['runtime_files'])
        native_path.write_text(json.dumps(native, separators=(',', ':')) + '\n')
        print('Refreshed signed provider: ' + str(native_path))


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('directory', type=Path)
    refresh(p.parse_args().directory)
