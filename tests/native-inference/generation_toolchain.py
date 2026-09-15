"""Explicit STEP-05 minimum four-tool correctness; no timing/sanitizer aggregate.
Usage: generation_toolchain.py BUILD SOURCE MODELS FRESH_OUTPUT cpu,required-gpu [--closeout]
"""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import sys

build, source, models, work = map(lambda p: Path(p).resolve(), sys.argv[1:5])
closeout = len(sys.argv) > 6 and sys.argv[6] == "--closeout"
modes = sys.argv[5].split(',')
assert modes and all(x in ('cpu', 'required-gpu', 'auto') for x in modes)
work.mkdir(parents=True, exist_ok=False)
env = dict(os.environ)
env.pop('CREXX_LLAMA_GLUE_PROBES', None)
suffix = '.exe' if os.name == 'nt' else ''
model = models / 'smollm2-360m-instruct-q8_0.gguf'
digest = '48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201'
assert hashlib.sha256(model.read_bytes()).hexdigest() == digest
mode_bge = models / 'bge-small-en-v1.5-f16.gguf'
digest_bge = 'f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999'
if closeout: assert hashlib.sha256(mode_bge.read_bytes()).hexdigest() == digest_bge
number = 0
runs = []


def run(label, argv, marker=None):
    global number
    number += 1
    argv = list(map(str, argv))
    result = subprocess.run(argv, cwd=work, env=env, capture_output=True, text=True, timeout=1800)
    output = result.stdout + result.stderr
    (work / f'{number:02}-{label}.log').write_text(f'argv={argv!r}\nrc={result.returncode}\n{output}')
    assert result.returncode == 0 and not any(x in output for x in ('FAIL:', 'PANIC:', 'ERROR:')), (label, output[-3500:])
    if marker:
        assert marker in output, (label, output[-3500:])
        runs.append(label)
    print('PASS:', label, flush=True)


cases = (
    ('typed_generation', source / 'tests/native-inference/typed_generation.crexx', 'PASS: typed llama generation', True),
    ('generation_overhead', source / 'tests/native-inference/generation_overhead.crexx', 'PASS: NI-S5 fixed generation work', True),
)
if closeout:
    cases = (
        ('provider_acceptance', source / 'tests/native-inference/provider_acceptance.crexx', 'PASS: rxllama persistent embedding and generation acceptance', True),
        ('persistent_generation', source / 'lib/plugins/llama/examples/persistent_generation.crexx', 'PASS: persistent typed generation example', True),
        ('shared_generation', source / 'lib/plugins/llama/examples/shared_generation.crexx', 'PASS: four typed generation workers', True),
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
                args = ['-a', mode, model, digest]
                if name == 'provider_acceptance': args = ['-a', mode_bge, digest_bge, model, digest, mode]
                if name == 'generation_overhead': args += ['4', '1']
                run(label + '-' + mode + '-' + engine, [vm, linked, *args], marker)
(work / 'results.json').write_text(json.dumps({'runs': runs, 'model_sha256': digest, 'sources': {str(p.relative_to(source)): hashlib.sha256(p.read_bytes()).hexdigest() for _, p, _, _ in cases}}, indent=2) + '\n')
print('PASS: generation four-tool matrix', len(runs), 'consumer executions')
