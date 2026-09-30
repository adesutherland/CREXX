"""Run the permanent first-party probe against verified, prebuilt engine files."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import stat
import subprocess
import sys
import time
import zipfile


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--archive', type=Path, required=True)
    parser.add_argument('--sha256', required=True)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--llama-source', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    assert sys.platform in ('linux', 'darwin')
    assert digest(args.archive) == args.sha256
    source, headers, output = args.source.resolve(), args.llama_source.resolve(), args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    records = []
    outcome = 'failed'
    identities = {}
    for base, paths in ((source, ['lib/plugins/llama/bridge.cpp', 'lib/plugins/llama/bridge.h',
                         'lib/plugins/llama/package.hpp', 'lib/plugins/llama/generation.h',
                         'lib/plugins/llama/generation_utf8.h', 'binutils/rxsha256.c',
                         'tests/native-inference/bridge_lifecycle.cpp']),
                        (headers, ['include/llama.h', 'ggml/include/ggml.h',
                                   'ggml/include/ggml-backend.h', 'vendor/nlohmann/json.hpp'])):
        for name in paths:
            identities[str(base / name)] = digest(base / name)

    def run(label, argv):
        started = time.monotonic()
        with (output / (label + '.log')).open('w') as log:
            log.write('argv=' + repr(list(map(str, argv))) + '\n'); log.flush()
            result = subprocess.run(list(map(str, argv)), stdout=log, stderr=log, cwd=source)
        records.append(dict(label=label, returncode=result.returncode,
                            elapsed_seconds=round(time.monotonic() - started, 3)))
        if result.returncode:
            raise RuntimeError(label + ' failed; see ' + str(output / (label + '.log')))

    try:
        with zipfile.ZipFile(args.archive) as archive:
            names = [n for n in archive.namelist() if n.endswith('/providers/rxllama.native.json')]
            assert len(names) == 1
            base = names[0].rsplit('/', 1)[0]
            manifest_bytes = archive.read(names[0])
            manifest = json.loads(manifest_bytes)
            assert manifest['engine'] == '5266f24da75dc449bd56cbed7addb9c8e4a6a73e'
            for mode in ('debug', 'asan'):
                build = output / mode
                providers = build / 'providers'
                providers.mkdir(parents=True, exist_ok=True)
                ordinary = {}
                for entry in manifest['runtime_files']:
                    name = entry['path']
                    assert Path(name).name == name
                    member = base + '/' + name
                    seen = set()
                    while stat.S_ISLNK(archive.getinfo(member).external_attr >> 16):
                        assert member not in seen
                        seen.add(member)
                        target = archive.read(member).decode()
                        assert Path(target).name == target
                        member = base + '/' + target
                    data = archive.read(member)
                    assert hashlib.sha256(data).hexdigest() == entry['sha256']
                    (providers / name).write_bytes(data)
                    if name.startswith(('libcrexx-ggml', 'libcrexx-llama-engine')):
                        ordinary[name] = entry['sha256']
                (providers / 'rxllama.native.json').write_bytes(manifest_bytes)
                if mode == 'debug':
                    unique = {}
                    for name, sha in ordinary.items():
                        unique.setdefault(sha, providers / name)
                    run('ordinary-engine-symbols', ['nm', '-u', *unique.values()])
                    assert '__asan_' not in (output / 'ordinary-engine-symbols.log').read_text()
                run(mode + '-configure', ['cmake', '-S', Path(__file__).resolve().parent,
                    '-B', build, '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=Debug',
                    '-DCREXX_SOURCE=' + str(source), '-DLLAMA_SOURCE=' + str(headers),
                    '-DPROVIDER_DIRECTORY=' + str(providers),
                    '-DPROBE_ASAN=' + ('ON' if mode == 'asan' else 'OFF')])
                run(mode + '-graph', ['ninja', '-C', build, '-t', 'commands', 'rxllama_bridge_lifecycle'])
                graph = (output / (mode + '-graph.log')).read_text()
                assert str(headers / 'src') not in graph and str(headers / 'ggml/src') not in graph
                leaks = 'off' if sys.platform == 'darwin' else 'on'
                runner = [source / 'tools/asan-run.sh', '--build-dir', build, '--no-live-tail']
                run(mode + '-build', runner + ['--phase', 'build', '--build-target',
                    'rxllama_bridge_lifecycle', '--build-jobs', '2', '--build-leaks', leaks])
                run(mode + '-symbols', ['nm', '-u', providers /
                    ('libcrexx-llama.1.dylib' if sys.platform == 'darwin' else 'libcrexx-llama.so.1')])
                symbols = (output / (mode + '-symbols.log')).read_text()
                assert ('__asan_' in symbols) == (mode == 'asan')
                for name, sha in ordinary.items():
                    assert digest(providers / name) == sha, 'engine file changed: ' + name
                run(mode + '-ctest', runner + ['--phase', 'ctest', '--regex',
                    '^rxllama_backend_probe_cycle$', '--test-jobs', '1', '--leaks', leaks])
                (output / (mode + '-ordinary-engine.json')).write_text(json.dumps(ordinary, indent=2) + '\n')
        outcome = 'passed'
    finally:
        (output / 'summary.json').write_text(json.dumps(dict(outcome=outcome,
            archive=args.archive.name, archive_sha256=args.sha256, sources=identities,
            upstream_instrumented=False, upstream_built=False,
            scope='SAN-009 first-party probe; no model or real-device qualification',
            commands=records), indent=2) + '\n')
    print('PASS: same first-party probe in normal Debug and ASan with ordinary engine')


if __name__ == '__main__':
    main()
