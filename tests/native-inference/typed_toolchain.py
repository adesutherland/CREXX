"""Explicit STEP-04 typed four-tool checks; no performance/sanitizer aggregate.
Usage: typed_toolchain.py BUILD SOURCE MODELS FRESH_OUTPUT cpu,required-gpu
"""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import sys

build, source, models, work = map(lambda p: Path(p).resolve(), sys.argv[1:5])
modes = sys.argv[5].split(',')
assert modes and all(x in ('cpu', 'required-gpu', 'auto') for x in modes)
work.mkdir(parents=True, exist_ok=False)
env = dict(os.environ)
env.pop('CREXX_LLAMA_GLUE_PROBES', None)
suffix = '.exe' if os.name == 'nt' else ''
model = models / 'bge-small-en-v1.5-f16.gguf'
digest = 'f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999'
assert hashlib.sha256(model.read_bytes()).hexdigest() == digest
number = 0
runs = []


def run(label, argv, marker=None):
    global number
    number += 1
    argv = list(map(str, argv))
    result = subprocess.run(argv, cwd=work, env=env, capture_output=True, text=True, timeout=300)
    output = result.stdout + result.stderr
    (work / f'{number:02}-{label}.log').write_text(f'argv={argv!r}\nrc={result.returncode}\n{output}')
    assert result.returncode == 0 and not any(x in output for x in ('FAIL:', 'PANIC:', 'ERROR:')), (label, output[-3500:])
    if marker:
        assert marker in output, (label, output[-3500:])
        runs.append(label)
    print('PASS:', label, flush=True)


cases = (
    ('typed_configuration', source / 'tests/native-inference/typed_configuration.crexx', 'PASS: typed llama configuration', False),
    ('typed_embeddings', source / 'tests/native-inference/typed_embeddings.crexx', 'PASS: typed llama embeddings', True),
    ('persistent_embeddings', source / 'lib/plugins/llama/examples/persistent_embeddings.crexx', 'PASS: persistent typed embedding example', True),
    ('shared_embeddings', source / 'lib/plugins/llama/examples/shared_embeddings.crexx', 'PASS: four typed embedding workers', True),
)
for opt in ('opt', 'noopt'):
    for name, consumer, marker, uses_model in cases:
        label = opt + '-' + name
        program = work / label
        run(label + '-compile', [build / ('bin/rxc' + suffix), '--no-exe-import', *(['-n'] if opt == 'noopt' else []), '-i', build / 'bin', '-o', program, consumer])
        run(label + '-assemble', [build / ('bin/rxas' + suffix), '-o', program, program])
        linked = str(program) + '-linked'
        run(label + '-link', [build / ('bin/rxlink' + suffix), '-o', linked, program, build / 'bin/library', build / 'bin/classlib', build / 'bin/rxfnsg'])
        for mode in modes if uses_model else ['no-model']:
            for engine in ('rxbvm', 'rxtvm'):
                vm = build / ('bin/' + engine + suffix)
                if engine == 'rxtvm' and not vm.exists():
                    continue
                args = ['-a', mode, model, digest] if uses_model else []
                run(label + '-' + mode + '-' + engine, [vm, linked, *args], marker)
(work / 'results.json').write_text(json.dumps({'runs': runs, 'model_sha256': digest, 'sources': {str(p.relative_to(source)): hashlib.sha256(p.read_bytes()).hexdigest() for _, p, _, _ in cases}}, indent=2) + '\n')
print('PASS: typed four-tool matrix', len(runs), 'consumer executions')
