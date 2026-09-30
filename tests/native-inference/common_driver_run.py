#!/usr/bin/env python3
"""Common native/HTTP conformance, using checked-in random weights and no API keys."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time

p = argparse.ArgumentParser()
p.add_argument('--build', type=Path, required=True)
p.add_argument('--source', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
a = p.parse_args()
a.build = a.build.resolve(); a.source = a.source.resolve(); a.output = a.output.resolve()
a.output.mkdir(parents=True, exist_ok=True)
suffix = '.exe' if os.name == 'nt' else ''
bin_dir = a.build / 'bin'
commands = []
environment = dict(os.environ)
if os.name == 'nt':
    environment['PATH'] = str(bin_dir / 'providers') + os.pathsep + str(bin_dir) + os.pathsep + environment.get('PATH', '')
def run(label, argv, marker=None):
    started = time.monotonic()
    log = a.output / (label + '.log')
    with log.open('w') as out:
        result = subprocess.run([str(x) for x in argv], stdout=out, stderr=subprocess.STDOUT,
                                timeout=600, cwd=a.output, env=environment)
    elapsed = time.monotonic() - started
    commands.append({'label': label, 'argv': list(map(str, argv)), 'exit': result.returncode,
                     'elapsed_seconds': elapsed, 'log': str(log)})
    (a.output / 'commands.json').write_text(json.dumps(commands, indent=2) + '\n')
    content = log.read_text(errors='replace')
    if result.returncode or 'PANIC:' in content or 'FAIL:' in content or (marker and marker not in content):
        raise RuntimeError(f'{label}: exit {result.returncode}\n{content[-8000:]}')
    print(f'PASS: {label} ({elapsed:.2f}s)', flush=True)

# The compilation directory contains only core declarations: no llama import.
imports = a.output / 'core-imports'
imports.mkdir(exist_ok=True)
for name in ('library.rxbin', 'classlib.rxbin', 'rxfnsg.rxbin', 'rxcexits.rxbin', 'rxfs.rxplugin', 'rxjson.rxplugin'):
    source = bin_dir / name
    if source.exists(): shutil.copy2(source, imports / name)
fixture = a.source / 'tests/native-inference/fixtures/llama-text.gguf'
info = json.loads((fixture.parent / 'text-manifest.json').read_text())
assert hashlib.sha256(fixture.read_bytes()).hexdigest() == info['sha256']
runners = [bin_dir / ('rxbvm' + suffix)]
if (bin_dir / ('rxtvm' + suffix)).exists(): runners.append(bin_dir / ('rxtvm' + suffix))
for mode in ('noopt', 'opt'):
    for name, source in (
        ('generation', a.source / 'tests/native-inference/common_driver.crexx'),
        ('embedding', a.source / 'tests/native-inference/common_embedding.crexx'),
        ('missing', a.source / 'tests/native-inference/common_missing.crexx'),
        ('http', a.source / 'lib/rxfnsg/tests_functional/ts_llm_http_common.crexx'),
    ):
        program = a.output / (name + '-' + mode)
        run(name + '-' + mode + '-compile', [bin_dir / ('rxc' + suffix), '--no-exe-import',
            '-i', imports, *(['-n'] if mode == 'noopt' else []), '-o', program, source])
        run(name + '-' + mode + '-assemble', [bin_dir / ('rxas' + suffix),
            *(['-n'] if mode == 'noopt' else []), '-o', program, program])
        linked = a.output / (name + '-' + mode + '-linked')
        run(name + '-' + mode + '-link', [bin_dir / ('rxlink' + suffix), '-o', linked, program,
            imports / 'library.rxbin', imports / 'classlib.rxbin', imports / 'rxfnsg.rxbin'])
        for vm in runners:
            label = name + '-' + mode + '-' + vm.stem
            if name == 'generation':
                run(label, [vm, linked, '-a', fixture, info['sha256']], 'PASS: native common driver')
            elif name == 'embedding':
                run(label, [vm, linked, '-a', fixture, info['sha256'], '64', 'mean'], 'PASS: separate common embeddings')
            elif name == 'missing':
                run(label + '-absent', [vm, linked, '-a', a.output / 'absent.rxplugin', 'provider_unavailable'], 'PASS: application catches')
                run(label + '-incompatible', [vm, linked, '-a', bin_dir / 'rxfs.rxplugin', 'provider_incompatible'], 'PASS: application catches')
            else:
                run(label, [os.sys.executable, a.source / 'tests/native-inference/http_driver_fixture.py', vm, linked], 'PASS: all HTTP drivers')
run('fixture-bridge-generation', [a.build / 'lib/plugins/llama/tests' / ('rxllama_model_compatibility' + suffix),
    fixture, info['sha256'], 'generation', '64'], 'PASS: generic generation')
for mode, marker in (('embedding', 'PASS: generic embedding'), ('negative', 'PASS: rejected tokenizer')):
    run('fixture-bridge-' + mode, [a.build / 'lib/plugins/llama/tests' / ('rxllama_model_compatibility' + suffix),
        fixture, info['sha256'], mode, '64'], marker)
print('PASS: common LLM driver toolchain and optional-provider controls')
