"""Real dependency scanning after copying roots from different directories."""
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

input_file, writer, proof = map(Path, sys.argv[1:])
proof.mkdir(parents=True, exist_ok=True)
source = input_file.read_text()
roots = re.findall(r'"([^"]+)"', source.split('set(runtime_files ', 1)[1])
assert len(roots) == 3
for mode in ('complete', 'missing'):
    output = proof / mode
    output.mkdir(exist_ok=True)
    text = source
    if mode == 'missing':
        # Only the first root is copied: its loader-relative dependency must
        # not be rescued from the original source directory.
        text = text.split('set(runtime_files ', 1)[0] + f'set(runtime_files "{roots[0]}")\n'
    config = proof / f'{mode}.cmake'
    config.write_text(text + f'set(output "{output.as_posix()}")\n')
    with (proof / f'{mode}.log').open('w') as log:
        result = subprocess.run(['cmake', '-DINPUT=' + str(config), '-P', str(writer)],
                                stdout=log, stderr=subprocess.STDOUT, timeout=120)
    log_text = (proof / f'{mode}.log').read_text(errors='replace')
    if mode == 'missing':
        assert result.returncode != 0 and 'Unresolved inference package dependencies' in log_text, log_text
    else:
        assert result.returncode == 0, log_text
        manifest = json.loads((output / 'copy_control.native.json').read_text())
        entries = {entry['path']: entry['sha256'] for entry in manifest['runtime_files']}
        for root in roots:
            original = Path(root)
            assert entries[original.name] == hashlib.sha256(original.read_bytes()).hexdigest()
        for name, expected in entries.items():
            assert expected == hashlib.sha256((output / name).read_bytes()).hexdigest()
    print('PASS: ' + mode + ' published package dependency control')
