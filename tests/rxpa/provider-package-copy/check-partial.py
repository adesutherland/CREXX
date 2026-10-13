"""Partial-flat DLL closure: identical copies pass, changed/missing bytes fail."""
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import time

input_file, writer, proof = map(Path, sys.argv[1:])
proof.mkdir(parents=True, exist_ok=True)
source = input_file.read_text(encoding='utf-8')
roots = re.findall(r'"([^"]+)"', source.split('set(runtime_files ', 1)[1].split(')\n', 1)[0])
branch = Path(re.search(r'set\(branch "([^"]+)"', source).group(1))
start = time.monotonic()
for mode in ('complete', 'different', 'missing'):
    output = proof / mode
    output.mkdir(exist_ok=True)
    text = source
    held = None
    if mode == 'different':
        shadow = proof / 'shadow' / Path(roots[1]).name
        shadow.parent.mkdir(exist_ok=True)
        shadow.write_bytes(Path(roots[1]).read_bytes() + b'collision control')
        text = text.replace(roots[1], shadow.as_posix())
    if mode == 'missing':
        held = branch.with_name(branch.name + '.held')
        branch.rename(held)
    config = proof / (mode + '.cmake')
    config.write_text(text + f'set(output "{output.as_posix()}")\n', encoding='utf-8')
    try:
        with (proof / (mode + '.log')).open('w') as log:
            result = subprocess.run(['cmake', '-DINPUT=' + str(config), '-P', str(writer)],
                stdout=log, stderr=subprocess.STDOUT, timeout=120)
    finally:
        if held is not None: held.rename(branch)
    log_text = (proof / (mode + '.log')).read_text(errors='replace')
    if mode == 'complete':
        assert result.returncode == 0, log_text
        manifest = json.loads((output / 'partial_control.native.json').read_text())
        entries = {e['path']: e['sha256'] for e in manifest['runtime_files']}
        assert len(entries) == len(manifest['runtime_files']), 'duplicate runtime identity'
        for original in [*map(Path, roots), branch]:
            assert entries[original.name] == hashlib.sha256(original.read_bytes()).hexdigest()
        for name, expected in entries.items():
            assert hashlib.sha256((output / name).read_bytes()).hexdigest() == expected
    else:
        marker = 'Conflicting provider dependency bytes' if mode == 'different' else 'Unresolved inference package dependencies'
        assert result.returncode != 0 and marker in log_text, log_text
    print('PASS: partial package ' + mode)
print('Partial package controls seconds=' + str(round(time.monotonic() - start, 3)))
