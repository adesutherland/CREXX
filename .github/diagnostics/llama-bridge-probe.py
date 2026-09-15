"""Bounded startup diagnosis using the archived bridge's permanent C probe."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time

p = argparse.ArgumentParser()
p.add_argument('--archive', type=Path, required=True)
p.add_argument('--sha256', required=True)
p.add_argument('--output-root', type=Path, required=True)
p.add_argument('--repeat', type=int, default=3)
p.add_argument('--capture-after', type=int, default=180)
a = p.parse_args()
assert sys.platform == 'darwin' and 1 <= a.repeat <= 5 and 1 <= a.capture_after < 1800
assert hashlib.sha256(a.archive.read_bytes()).hexdigest() == a.sha256
logs = a.output_root.resolve()
logs.mkdir(parents=True, exist_ok=True)
work = Path(tempfile.mkdtemp(prefix='crexx-bridge-probe-'))
subprocess.run(['/usr/bin/ditto', '-x', '-k', str(a.archive.resolve()), str(work / 'archive')], check=True)
manifests = list((work / 'archive').glob('*/bin/providers/rxllama.native.json'))
assert len(manifests) == 1
manifest = json.loads(manifests[0].read_text())
providers = manifests[0].parent
for entry in manifest['runtime_files']:
    name = entry['path']
    assert Path(name).name == name
    assert hashlib.sha256((providers / name).read_bytes()).hexdigest() == entry['sha256']
source = logs / 'bridge-probe.c'
source.write_text('''#include <stdio.h>
extern int rxllama_test_probe_cycle(void);
int main(void) {
    puts("BRIDGE_PROBE_BEGIN"); fflush(stdout);
    int result = rxllama_test_probe_cycle();
    printf("BRIDGE_PROBE_END: %d\\n", result);
    return result;
}
''')
clean = {k: v for k, v in os.environ.items()
         if not k.startswith(('CREXX_', 'GGML_', 'DYLD_', 'LLAMA_'))}
records = []
outcome = 'failed'
direct_source = logs / 'direct-ggml.c'
direct_source.write_text(r'''#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
extern void *ggml_backend_load(const char *);
extern size_t ggml_backend_reg_dev_count(void *);
extern void ggml_log_set(void (*)(int, const char *, void *), void *);
static void quiet(int level, const char *text, void *data) {}
int main(int argc, char **argv) {
    if (argc < 4) return 2;
    if (!strcmp(argv[1], "quiet")) ggml_log_set(quiet, NULL);
    int best = 0, best_score = 0;
    for (int i = 3; i < argc; ++i) {
        void *library = dlopen(argv[i], RTLD_NOW | RTLD_LOCAL);
        if (!library) { fprintf(stderr, "%s\n", dlerror()); return 3; }
        int (*score)(void) = (int (*)(void))dlsym(library, "ggml_backend_score");
        int value = score ? score() : 1;
        if (value > best_score) { best = i; best_score = value; }
        /* Match the permanent probe's extra retained first-CPU leases. */
        if (i == 3 && (!dlopen(argv[i], RTLD_NOW | RTLD_LOCAL) ||
                      !dlopen(argv[i], RTLD_NOW | RTLD_LOCAL))) return 4;
    }
    if (!best || !ggml_backend_load(argv[best])) return 5;
    puts("DIRECT_NATIVE_STAGE: Metal registration"); fflush(stdout);
    void *metal = ggml_backend_load(argv[2]);
    if (!metal || !ggml_backend_reg_dev_count(metal)) return 6;
    for (uint32_t i = 0; i < _dyld_image_count(); ++i) {
        const char *name = _dyld_get_image_name(i);
        printf("LOADED_IMAGE: %s\n", name);
        if (strstr(name, "libcrexx-llama") || strstr(name, "rxvm")) return 7;
    }
    puts("DIRECT_NATIVE_END: 0");
    return 0;
}
''')


def run(label, argv, marker=None):
    started = time.monotonic()
    record = dict(label=label, argv=list(map(str, argv)))
    records.append(record)
    with (logs / (label + '.log')).open('w') as log:
        with subprocess.Popen(record['argv'], cwd=work, env=clean, stdout=log, stderr=log) as child:
            try:
                child.wait(timeout=a.capture_after)
            except subprocess.TimeoutExpired:
                record['diagnostic_stop'] = True
                try:
                    with (logs / (label + '-sampler.log')).open('w') as sampler:
                        subprocess.run(['/usr/bin/sample', str(child.pid), '3', '1', '-file',
                                        str(logs / (label + '-stack.txt'))], stdout=sampler,
                                       stderr=sampler, timeout=30, check=False)
                finally:
                    child.kill(); child.wait()
            record.update(returncode=child.returncode,
                          elapsed_seconds=round(time.monotonic() - started, 3))
    assert not record.get('diagnostic_stop'), 'diagnostic stop, not a qualification pass'
    assert record['returncode'] == 0, (logs / (label + '.log')).read_text()[-2000:]
    if marker:
        assert marker in (logs / (label + '.log')).read_text()
    print('PASS: ' + label, flush=True)


try:
    executable = work / 'bridge-probe'
    run('compile', ['/usr/bin/cc', source, providers / 'libcrexx-llama.dylib',
                    '-Wl,-rpath,' + str(providers), '-o', executable])
    run('dependencies', ['/usr/bin/otool', '-L', executable], 'libcrexx-llama')
    direct_executable = work / 'direct-ggml'
    run('compile-direct', ['/usr/bin/cc', direct_source, providers / 'libcrexx-ggml.dylib',
                          providers / 'libcrexx-ggml-base.dylib',
                          '-Wl,-rpath,' + str(providers), '-o', direct_executable])
    cpu = [providers / entry['path'] for entry in
           json.loads((providers / 'rxllama.runtime.json').read_text())['backends']
           if entry['backend'].startswith('cpu')]
    failures = []
    controls = [
        ('native-quiet', [direct_executable, 'quiet', providers / 'libcrexx-ggml-metal.so', *cpu], 'DIRECT_NATIVE_END: 0'),
        ('native-default', [direct_executable, 'default', providers / 'libcrexx-ggml-metal.so', *cpu], 'DIRECT_NATIVE_END: 0'),
        ('python-direct', [sys.executable, '-B', Path(__file__).with_name('llama-ggml-replay.py').resolve(),
                           '--direct', providers], 'PASS: direct GGML registration'),
        ('bridge', [executable], 'BRIDGE_PROBE_END: 0')]
    for label, command, marker in controls:
        try:
            run(label, command, marker)
        except AssertionError as error:
            failures.append(label)
            print('FAIL: ' + label + ': ' + str(error), flush=True)
    assert not failures, 'Diagnostic failures: ' + ', '.join(failures)
    outcome = 'passed'
finally:
    (logs / 'summary.json').write_text(json.dumps(dict(outcome=outcome,
        scope='same-run native/Python GGML and archived bridge startup attribution; no VM, model or engine rebuild',
        archive=a.archive.name, archive_sha256=a.sha256, providers=manifest,
        source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),
        direct_source_sha256=hashlib.sha256(direct_source.read_bytes()).hexdigest(),
        capture_after=a.capture_after, commands=records), indent=2) + '\n')
