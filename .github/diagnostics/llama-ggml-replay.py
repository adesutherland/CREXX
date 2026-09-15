"""Diagnostic-only direct GGML registration, excluding every cREXX library."""
import argparse
import ctypes
import hashlib
import json
import os
from pathlib import Path
import stat
import subprocess
import sys
import tempfile
import time
import zipfile


def direct(directory):
    ggml = ctypes.CDLL(str(directory / 'libcrexx-ggml.dylib'))
    ggml.ggml_backend_load.argtypes = [ctypes.c_char_p]
    ggml.ggml_backend_load.restype = ctypes.c_void_p
    ggml.ggml_backend_reg_dev_count.argtypes = [ctypes.c_void_p]
    ggml.ggml_backend_reg_dev_count.restype = ctypes.c_size_t
    system = ctypes.CDLL(None)
    system._dyld_image_count.restype = ctypes.c_uint32
    system._dyld_get_image_name.argtypes = [ctypes.c_uint32]
    system._dyld_get_image_name.restype = ctypes.c_char_p

    def images():
        names = [system._dyld_get_image_name(i).decode()
                 for i in range(system._dyld_image_count())]
        print('LOADED_IMAGES: ' + json.dumps(names), flush=True)
        assert not any('libcrexx-llama' in name or 'rxvm' in name for name in names)

    images()
    print('DIRECT_STAGE: ggml_backend_load(metal)', flush=True)
    reg = ggml.ggml_backend_load(os.fsencode(directory / 'libcrexx-ggml-metal.so'))
    assert reg, 'direct Metal backend registration failed'
    count = ggml.ggml_backend_reg_dev_count(reg)
    print('DIRECT_DEVICES: ' + str(count), flush=True)
    assert count > 0, 'no Metal device; startup path was not exercised'
    images()
    print('PASS: direct GGML registration without cREXX bridge', flush=True)


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--archive', type=Path, required=True)
    p.add_argument('--sha256', required=True)
    p.add_argument('--output-root', type=Path, required=True)
    p.add_argument('--repeat', type=int, default=3)
    p.add_argument('--capture-after', type=int, default=180)
    a = p.parse_args()
    assert sys.platform == 'darwin' and 1 <= a.repeat <= 5
    assert 1 <= a.capture_after < 1800
    assert hashlib.sha256(a.archive.read_bytes()).hexdigest() == a.sha256
    logs = a.output_root.resolve()
    logs.mkdir(parents=True, exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix='crexx-direct-ggml-'))
    copied = []
    with zipfile.ZipFile(a.archive) as archive:
        manifests = [n for n in archive.namelist() if n.endswith('/providers/rxllama.native.json')]
        assert len(manifests) == 1
        base = manifests[0].rsplit('/', 1)[0]
        manifest = json.loads(archive.read(manifests[0]))
        for entry in manifest['runtime_files']:
            name = entry['path']
            assert Path(name).name == name
            if not name.startswith('libcrexx-ggml'):
                continue
            member = base + '/' + name
            seen = set()
            while stat.S_ISLNK(archive.getinfo(member).external_attr >> 16):
                assert member not in seen, 'cyclic library symlink'
                seen.add(member)
                target = archive.read(member).decode()
                assert Path(target).name == target and target.startswith('libcrexx-ggml')
                member = base + '/' + target
            data = archive.read(member)
            assert hashlib.sha256(data).hexdigest() == entry['sha256']
            (work / name).write_bytes(data)
            copied.append(entry)
    assert (work / 'libcrexx-ggml.dylib').is_file()
    assert (work / 'libcrexx-ggml-metal.so').is_file()
    clean = {k: v for k, v in os.environ.items()
             if not k.startswith(('DYLD_', 'CREXX_', 'GGML_', 'LLAMA_'))}
    records = []
    outcome = 'failed'
    try:
        for attempt in range(1, a.repeat + 1):
            label = 'direct-' + str(attempt)
            started = time.monotonic()
            record = dict(label=label)
            records.append(record)
            with (logs / (label + '.log')).open('w') as log:
                with subprocess.Popen([sys.executable, '-B', __file__, '--direct', str(work)],
                                      cwd=work, env=clean, stdout=log, stderr=log) as child:
                    try:
                        child.wait(timeout=a.capture_after)
                    except subprocess.TimeoutExpired:
                        record['diagnostic_stop'] = True
                        try:
                            with (logs / (label + '-sampler.log')).open('w') as sampler:
                                subprocess.run(['/usr/bin/sample', str(child.pid), '3', '1', '-file',
                                                str(logs / (label + '-stack.txt'))],
                                               stdout=sampler, stderr=sampler, timeout=30, check=False)
                        finally:
                            child.kill()
                            child.wait()
                    record['returncode'] = child.returncode
                    record['elapsed_seconds'] = round(time.monotonic() - started, 3)
            result = (logs / (label + '.log')).read_text(errors='replace')
            assert not record.get('diagnostic_stop'), 'diagnostic stop; not a qualification pass'
            assert record['returncode'] == 0 and 'PASS: direct GGML registration' in result, result[-2000:]
            print('PASS: ' + label, flush=True)
        outcome = 'passed'
    finally:
        (logs / 'summary.json').write_text(json.dumps(dict(outcome=outcome,
            scope='direct GGML registration attribution only; no bridge, VM, model or engine rebuild',
            archive=a.archive.name, archive_sha256=a.sha256, libraries=copied,
            work=str(work), capture_after=a.capture_after, commands=records), indent=2) + '\n')


if __name__ == '__main__':
    if len(sys.argv) == 3 and sys.argv[1] == '--direct':
        direct(Path(sys.argv[2]))
    else:
        main()
