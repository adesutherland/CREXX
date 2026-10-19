#!/usr/bin/env python3
"""Keep the teaching module's binary-buffer code shape independently checked."""
from pathlib import Path
import subprocess
import sys
import tempfile

with tempfile.TemporaryDirectory(prefix='rxlang-tinyexpr-') as directory:
    output = Path(directory) / 'tinyexpr.rxas'
    result = subprocess.run([sys.argv[1], '-o', str(output), sys.argv[2]],
                            capture_output=True, timeout=60)
    assert result.returncode == 0, result.stderr.decode(errors='replace')
    assembly = output.read_text()
    for instruction in ('bgetu8', 'bseti64', 'bcmpb'):
        assert instruction in assembly, instruction
    assert 'bslice' not in assembly, 'unexpected buffer slice copying'
print('TINYEXPR_BINARY_SURFACE|PASS')
