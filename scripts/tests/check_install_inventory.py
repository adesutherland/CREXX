#!/usr/bin/env python3
"""Install twice from a populated build tree; reject accidental output harvesting."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import uuid

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--build', required=True, type=Path)
parser.add_argument('--cmake', default='cmake')
args = parser.parse_args()
build = args.build.resolve()
name = 'test_obsolete_' + uuid.uuid4().hex
sentinels = [build/'bin'/f'{name}.rxbin', build/'bin/providers'/f'{name}.rxplugin']
try:
    for p in sentinels:
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text('not a product\n')
    with tempfile.TemporaryDirectory(prefix='crexx install inventory ') as temp:
        prefix = Path(temp)/'prefix with spaces'
        previous = None
        for attempt in (1, 2):
            result = subprocess.run([args.cmake, '--install', str(build), '--prefix', str(prefix)],
                                    capture_output=True, text=True)
            if result.returncode:
                raise RuntimeError('install failed:\n'+(result.stdout+result.stderr)[-5000:])
            inventory = {str(p.relative_to(prefix)) for p in prefix.rglob('*') if p.is_file()}
            assert not any(name in p for p in inventory), 'stale artifact installed'
            assert not any(Path(p).name.startswith(('test_', 'rxbvm_instrumented', 'rxtvm_instrumented'))
                           for p in inventory if p.startswith('bin/')), 'test harness installed'
            for command in ('crexx', 'rxc', 'rxc-c', 'rxas', 'rxlink', 'rxdas', 'rxvm', 'rxlang', 'rxbuild'):
                assert (prefix/'bin'/command).exists() or (prefix/'bin'/f'{command}.exe').exists(), command
            for module in ('library', 'classlib', 'classlib_native', 'rxfnsc', 'rxfnsg', 'rxfnsl', 'rxlanglib'):
                assert (prefix/'bin'/f'{module}.rxbin').is_file(), module
            for provider in ('rxcstream', 'rxconsole', 'rxfloat'):
                assert any((prefix/'bin/providers'/f'{provider}{ext}').is_file() for ext in ('.a','.lib')), provider
            assert (prefix/'bin/rxprecomp.rxplugin').is_file(), 'preprocessor provider'
            assert any((prefix/'bin'/f'rxprecomp_static{ext}').is_file() for ext in ('.a','.lib'))
            for alias, canonical in [('rx_rxmath.rxplugin', 'rxfloat.rxplugin')]:
                assert (prefix/'bin'/alias).read_bytes() == (prefix/'bin'/canonical).read_bytes()
            for ext in ('.a', '.lib'):
                canonical = prefix/'bin'/f'rxfloat_static{ext}'
                if canonical.exists():
                    # CMake may run ranlib on installed TARGET archives. The
                    # compatibility FILE copy must preserve the original input.
                    assert (prefix/'bin'/f'rx_rxmath_static{ext}').read_bytes() == (build/'bin'/f'rxfloat_static{ext}').read_bytes()
            for manifest in (prefix/'bin/providers').glob('*.native.json'):
                data = json.loads(manifest.read_text())
                for entry in data['link_libraries'] + data['runtime_files']:
                    assert hashlib.sha256((manifest.parent/entry['path']).read_bytes()).hexdigest() == entry['sha256']
            if previous is not None:
                assert inventory == previous, 'repeated install inventory changed'
            previous = inventory
        print(f'PASS: fresh/repeated install, spaces, {len(previous)} files, provider closure and stale-artifact exclusion')
finally:
    for p in sentinels:
        p.unlink(missing_ok=True)
