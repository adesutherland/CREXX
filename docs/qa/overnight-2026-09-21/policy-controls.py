#!/usr/bin/env python3
"""Darwin local negative controls for the RXVECTOR-02 manifest assertion.

Build temporary copies of the real provider with one deliberately invalid policy
or hook. The repository sources and production provider artifacts are untouched.
Usage: python3 policy-controls.py SOURCE_DIR DEBUG_BUILD_DIR OUTPUT_DIR
"""
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

source, build, output = map(lambda p: Path(p).resolve(), sys.argv[1:])
output.mkdir(parents=True, exist_ok=True)
provider = source / 'lib/plugins/vector'
header = (provider / 'rxvector_owner.h').read_text()
variants = {
    'valid': header,
    'stateless-marked-session-affine': header.replace(
        'return RXPA_PROCEDURE_CAP_PROCESS_REENTRANT;',
        'return RXPA_PROCEDURE_CAP_SESSION_AFFINE;', 1),
    'owner-marked-reentrant': header.replace(
        'return RXPA_PROCEDURE_CAP_SESSION_AFFINE;',
        'return RXPA_PROCEDURE_CAP_PROCESS_REENTRANT;', 1),
    'missing-session-destroy': header.replace(
        'RXPA_PLUGIN_SESSION_WITH_HOST(rxvi_create_old,rxvi_destroy,',
        'RXPA_PLUGIN_SESSION_WITH_HOST(rxvi_create_old,NULL,', 1),
    'missing-host-factory': header.replace(
        'rxvi_caps,rxvi_create)', 'rxvi_caps,NULL)', 1),
}
results = []
with tempfile.TemporaryDirectory(prefix='crexx-vector-policy-') as scratch:
    for name, text in variants.items():
        folder = Path(scratch) / name
        folder.mkdir()
        for path in provider.iterdir():
            if path.suffix in ('.c', '.h'):
                shutil.copy2(path, folder / path.name)
        (folder / 'rxvector_owner.h').write_text(text)
        command = ['/usr/bin/cc', '-DBUILD_DLL', '-DPLUGIN_ID=rxvector',
                   '-Dvector_EXPORTS', '-I'+str(build/'generated'),
                   '-I'+str(source), '-I'+str(source/'rxpa'), '-g',
                   '-std=gnu99', '-arch', 'arm64', '-fPIC', '-bundle',
                   str(folder/'rxvector.c'), str(folder/'rxvector_index.c'),
                   '-lm', '-o', str(folder/'rxvector.rxplugin')]
        with (output/(name+'-build.log')).open('w') as log:
            subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True)
        command = [str(build/'interpreter/test_rxpa_concurrency'), 'bundled',
                   str(folder), 'rxvector.rxplugin', 'vector']
        with (output/(name+'.log')).open('w') as log:
            result = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT)
        diagnostic = (output/(name+'.log')).read_text()
        expected = 0 if name == 'valid' else 1
        assert result.returncode == expected, (name, result.returncode, diagnostic)
        if expected:
            assert 'RXPA vector manifest qualification failed' in diagnostic
        results.append({'control': name, 'returncode': result.returncode,
                        'expected': expected, 'passed': True})
(output/'results.json').write_text(json.dumps(results, indent=2)+'\n')
print(json.dumps(results, indent=2))
