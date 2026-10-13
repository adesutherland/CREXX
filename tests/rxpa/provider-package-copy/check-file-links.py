"""Verify concrete/target linker files and a plugin's discovered runtime DLL."""
import hashlib
import json
from pathlib import Path
import sys

manifest_path, archive, linker, plugin, runtime, provider = map(Path, sys.argv[1:])
manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
assert manifest['version'] == 1 and manifest['provider'] == 'rxcopy_provider'
assert provider.is_file(), 'static provider archive missing'
assert len(manifest['link_libraries']) == 2, 'LINK_FILES input was dropped'
for key, sources in [('link_libraries', (archive, linker)),
                     ('runtime_files', (plugin, runtime))]:
    entries = {e['path']: e['sha256'] for e in manifest[key]}
    assert len(entries) == len(manifest[key]), 'duplicate published file identity'
    for original in sources:
        expected = hashlib.sha256(original.read_bytes()).hexdigest()
        assert entries.get(original.name) == expected, f'{key} omitted {original.name}'
    for name, expected in entries.items():
        published = manifest_path.parent / name
        assert hashlib.sha256(published.read_bytes()).hexdigest() == expected, name
print('PASS: target/concrete linker files and rxplugin runtime dependency hashes')
