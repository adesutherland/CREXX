"""Explicit local model checks. Provisioned artifacts only; no downloads or timings gate."""
import argparse
import hashlib
import json
import subprocess
import time
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('--build', type=Path, required=True)
p.add_argument('--models', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
p.add_argument('--gemma', type=Path)
p.add_argument('--phase', choices=['all', 'models', 'examples'], default='all')
a = p.parse_args()
root = Path(__file__).resolve().parents[2]
build, models, output = a.build.resolve(), a.models.resolve(), a.output.resolve()
output.mkdir(parents=True, exist_ok=True)
bin = build / 'bin'
helpers = build / 'lib/plugins/llama/tests'
records = []
def run(label, argv, marker=None):
    log = output / (label + '.log')
    started = time.monotonic()
    with log.open('w') as stream:
        result = subprocess.run(list(map(str, argv)), stdout=stream, stderr=subprocess.STDOUT, timeout=1800, cwd=output)
    records.append({'label': label, 'argv': list(map(str, argv)), 'exit': result.returncode,
                    'elapsed_seconds': round(time.monotonic()-started, 3)})
    (output / 'commands.json').write_text(json.dumps(records, indent=2)+'\n')
    text = log.read_text(errors='replace')
    if result.returncode or 'PANIC:' in text or 'FAIL:' in text or (marker and marker not in text):
        raise RuntimeError(label+': '+text[-4000:])
    print('PASS: '+label, flush=True)
def compile(label, source):
    program = output / label
    run(label+'-compile', [bin/'rxc', '--no-exe-import', '-i', bin, '-o', program, source])
    run(label+'-assemble', [bin/'rxas', '-o', program, program])
    linked = output / (label+'-linked')
    run(label+'-link', [bin/'rxlink', '-o', linked, program, bin/'library', bin/'classlib', bin/'rxfnsg'])
    return linked
bge_hash = 'f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999'
smol_hash = '48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201'
if a.phase in ('all', 'models'):
    run('bge-preset-parity', [helpers/'rxllama_embedding_bridge', 'cpu', models/'bge-small-en-v1.5-f16.gguf', bge_hash], 'PASS:')
    run('smol-preset-parity', [helpers/'rxllama_generation_bridge', 'cpu', models/'smollm2-360m-instruct-q8_0.gguf', smol_hash], 'PASS: generation direct parity')
    run('stories-generation', [helpers/'rxllama_model_compatibility', models/'stories260K.gguf',
        '270cba1bd5109f42d03350f60406024560464db173c0e387d91f0426d3bd256d', 'generation', '64'], 'PASS: generic generation')
    embedding = compile('common-embedding', root/'tests/native-inference/common_embedding.crexx')
    for model, sha in [('bge-base-en-v1.5-f16.gguf', '88360fdf8521af0ac08d43818bd272da679ab97c685d9b273c48efd01a4187c2'),
                       ('bge-base-en-v1.5-q4_k_m.gguf', '74aebb552ea73b271d3b9c709923b4b7633b304fbc897a0498e52a180c3a9da9')]:
        # Direct control uses query: prefix and an empty document prefix. Common
        # consumer separately checks the BGE query prefix and owned space identity.
        for pooling in ('mean', 'cls'):
            run(model+'-'+pooling+'-parity', [helpers/'rxllama_model_compatibility', models/model, sha, 'embedding', '768', pooling], 'PASS: generic embedding')
        run(model+'-common', [bin/'rxbvm', embedding, '-a', models/model, sha, '768'], 'PASS: separate common embeddings')
if a.phase in ('all', 'examples'):
    fixture = root/'tests/native-inference/fixtures/llama-text.gguf'
    info = json.loads(fixture.with_name('text-manifest.json').read_text())
    for name, args, marker in [('common_generation', ['llama', fixture, 'Hello', info['sha256'], 'raw'], None),
                              ('common_embeddings', [fixture, info['sha256'], 'mean', '', ''], 'Rows: 2 Dimensions: 64 Values: 128')]:
        program = compile(name, root/'lib/plugins/llama/examples'/ (name+'.crexx'))
        run(name+'-example', [bin/'rxbvm', program, '-a', *args], marker)
    if a.gemma:
        program = compile('gemma4', root/'docs/qa/llm-interface-review-20260917/gemma4-common.crexx')
        run('gemma4-E4B-common', [bin/'rxbvm', program, '-a', a.gemma.resolve(),
            'a555b900214b477d8880e7832e0b8925e139b0159640036b09fe472b6f2097f2'], 'PASS: local Gemma 4 model')
print('PASS: local compatibility phase '+a.phase)
