#!/usr/bin/env python3
"""Small cold/no-op bootstrap proof for the new shared comment dependency."""
import argparse
import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path
from _scratch import scratch

p = argparse.ArgumentParser()
p.add_argument('tools', type=Path)
p.add_argument('work', type=Path)
p.add_argument('evidence', type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[2]
tools, work, out = a.tools.resolve(), scratch(), scratch()
assert not work.exists(), 'Use a fresh owned work directory'
out.mkdir(parents=True, exist_ok=True)
source, build, blocked = work / 'source', work / 'build', work / 'no-re2c'
shutil.copytree(root / 'lib/rxfnsl', source / 'lib/rxfnsl')
(source / 'cmake').mkdir()
shutil.copy2(root / 'cmake/CrexxImportRoots.cmake', source / 'cmake/CrexxImportRoots.cmake')
blocked.mkdir()
(blocked / 're2c').write_text('#!/bin/sh\nexit 97\n')
(blocked / 're2c').chmod(0o755)
env = dict(os.environ, PATH=str(blocked) + os.pathsep + os.environ['PATH'])
records = []


def run(name, argv):
    argv = list(map(str, argv))
    result = subprocess.run(argv, capture_output=True, env=env, timeout=1800)
    data = result.stdout + result.stderr
    (out / (name + '.log')).write_bytes(data)
    assert result.returncode == 0 and b'Warning in ' not in data, (name, data[-2500:])
    assert b're2c' not in data.lower(), (name, 'unexpected re2c build invocation')
    records.append(dict(name=name, argv=argv, status=result.returncode,
                        sha256=hashlib.sha256(data).hexdigest()))


def snapshot():
    return {str(path.relative_to(build)): (path.stat().st_mtime_ns, hashlib.sha256(path.read_bytes()).hexdigest())
            for path in build.rglob('*') if path.suffix in ['.rxbin', '.rxas', '.stamp']}


run('configure', ['cmake', '-S', source / 'langtools/bootstrap', '-B', build,
                 '-DCREXX_HOST_BIN=' + str(tools), '-DCMAKE_BUILD_TYPE=Debug'])
command = ['cmake', '--build', build, '--target', 'rxfnsl', 'levell_unicode_lexer', '--parallel', '4', '--verbose']
run('cold', command)
before = snapshot()
run('noop', command)
assert snapshot() == before, 'No-op bootstrap rewrote generated artifacts'
assert not (source / 'lib/rxfnsg').exists() and not (source / 're2c').exists()
first, second = build / 'langtools/bootstrap/L1/llsyntax.rxas', build / 'langtools/bootstrap/L2/llsyntax.rxas'
assert first.read_bytes() == second.read_bytes()
assert (build / 'rexx/llcomments.rxbin').is_file()
(out / 'receipt.json').write_text(json.dumps(dict(records=records,
    level_g_present=False, re2c_invocations=0, no_op_unchanged=True,
    self_description_sha256=hashlib.sha256(second.read_bytes()).hexdigest(),
    generated={key: value[1] for key, value in before.items()}), indent=2) + '\n')
print('bootstrap_cases=3; no_level_g=true; re2c_invocations=0; failures=0')
