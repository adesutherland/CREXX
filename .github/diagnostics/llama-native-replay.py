"""Diagnostic-only replay of a released-layout native consumer; no engine rebuild."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time

p = argparse.ArgumentParser()
p.add_argument('--archive', type=Path, required=True)
p.add_argument('--sha256', required=True)
p.add_argument('--source', type=Path, required=True)
p.add_argument('--output-root', type=Path, required=True)
p.add_argument('--repeat', type=int, default=5)
p.add_argument('--capture-after', type=int, default=180)
a = p.parse_args()
assert sys.platform == 'darwin' and 1 <= a.repeat <= 5 and 1 <= a.capture_after < 1800
assert hashlib.sha256(a.archive.read_bytes()).hexdigest() == a.sha256
logs = a.output_root.resolve()
logs.mkdir(parents=True, exist_ok=True)
work = Path(tempfile.mkdtemp(prefix='crexx-native-replay-'))
subprocess.run(['/usr/bin/ditto', '-x', '-k', str(a.archive.resolve()), str(work / 'payload')], check=True)
drivers = list((work / 'payload').glob('*/bin/crexx'))
assert len(drivers) == 1
prefix = drivers[0].parent.parent
source = a.source.resolve()
fixture_info = json.loads((source / 'tests/native-inference/fixtures/manifest.json').read_text())
fixture = work / 'fixture.gguf'
shutil.copy2(source / 'tests/native-inference/fixtures' / fixture_info['filename'], fixture)
assert fixture.stat().st_size == fixture_info['bytes'] < 5 * 1024 * 1024
assert hashlib.sha256(fixture.read_bytes()).hexdigest() == fixture_info['sha256']
clean = dict(os.environ)
for key in ('CREXX_HOME', 'DYLD_LIBRARY_PATH', 'DYLD_FALLBACK_LIBRARY_PATH',
            'LD_LIBRARY_PATH', 'GGML_BACKEND_PATH', 'CREXX_PROVIDER_PATH', 'CREXX_LLAMA_GLUE_PROBES'):
    clean.pop(key, None)
records = []
outcome = 'failed'

def run(label, argv, env, marker):
    argv = list(map(str, argv))
    started = time.monotonic()
    with (logs / (label + '.log')).open('w') as log:
        log.write('argv=' + repr(argv) + '\n'); log.flush()
        with subprocess.Popen(argv, cwd=work, env=env, stdout=log, stderr=log) as child:
            try:
                child.wait(timeout=a.capture_after)
            except subprocess.TimeoutExpired:
                try:
                    with (logs / (label + '-sampler.log')).open('w') as sampler:
                        subprocess.run(['/usr/bin/sample', str(child.pid), '3', '1', '-file',
                                        str(logs / (label + '-stack.txt'))], stdout=sampler,
                                       stderr=sampler, timeout=30, check=False)
                finally:
                    child.kill(); child.wait()
                raise RuntimeError(label + ': diagnostic stop after stack capture, not a qualification pass')
        records.append(dict(label=label, argv=argv, returncode=child.returncode,
                            elapsed_seconds=round(time.monotonic() - started, 3)))
    text = (logs / (label + '.log')).read_text(errors='replace')
    assert child.returncode == 0 and marker in text, text[-3000:]
    assert not any(x in text for x in ('FAIL:', 'PANIC:', 'ERROR: AddressSanitizer', 'ERROR: LeakSanitizer'))
    print('PASS: ' + label, flush=True)

try:
    native = work / 'native' / 'smoke'
    native.parent.mkdir()
    run('native-build', [drivers[0], '--program', native,
        source / 'tests/native-inference/release_provider_smoke.crexx', '--jobs', '1', '--native'],
        dict(clean, CREXX_HOME=str(prefix)), 'PUBLISHED: native program')
    relocated = work / 'relocated'
    relocated.mkdir()
    shutil.copy2(native, relocated / native.name)
    manifests = list(native.parent.glob('*.native.json'))
    assert any(m.name == 'rxllama.native.json' for m in manifests)
    for manifest in manifests:
        shutil.copy2(manifest, relocated / manifest.name)
        for entry in json.loads(manifest.read_text())['runtime_files']:
            relative = Path(entry['path'])
            assert not relative.is_absolute() and '..' not in relative.parts
            target = relocated / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(native.parent / relative, target)
    shutil.rmtree(native.parent)
    for attempt in range(a.repeat):
        run('relocated-' + str(attempt + 1), [relocated / native.name, fixture, fixture_info['sha256']],
            clean, 'PASS: public llama release provider')
    outcome = 'passed'
finally:
    (logs / 'summary.json').write_text(json.dumps(dict(outcome=outcome,
        scope='isolated native stall diagnosis; no full-package or performance qualification',
        archive=a.archive.name, archive_sha256=a.sha256, work=str(work),
        capture_after=a.capture_after, requested_repetitions=a.repeat, commands=records), indent=2) + '\n')
