"""Bounded smoke of a release payload; no trained model download or speed gate."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time


def sha(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def copy_runtime(source, destination):
    # CUDA libraries are large. The tests never alter them, so use independent
    # directory entries on the same filesystem; mutable manifests remain copies.
    if source.suffix != '.json':
        try:
            os.link(source, destination)
            return
        except OSError:
            pass
    shutil.copy2(source, destination)


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--build', type=Path, required=True)
    p.add_argument('--source', type=Path, required=True)
    p.add_argument('--helper', type=Path, required=True)
    p.add_argument('--payload', type=Path)
    p.add_argument('--output-root', type=Path, required=True)
    p.add_argument('--backends', default='cpu')
    a = p.parse_args()
    source, build, helper = a.source.resolve(), a.build.resolve(), a.helper.resolve()
    preferred_vm = (build / 'lib/plugins/llama/tests/release-smoke-default-vm.txt').read_text().strip()
    assert preferred_vm in ('rxbvm', 'rxtvm'), preferred_vm
    a.output_root.mkdir(parents=True, exist_ok=True)
    logs = Path(tempfile.mkdtemp(prefix='release-smoke-', dir=a.output_root.resolve()))
    work = Path(tempfile.mkdtemp(prefix='crexx-release-smoke-'))
    print('SMOKE_RECORD=' + str(logs), flush=True)
    started = time.monotonic()
    records = []
    suffix = '.exe' if os.name == 'nt' else ''
    clean = dict(os.environ)
    for key in ('CREXX_HOME', 'DYLD_LIBRARY_PATH', 'DYLD_FALLBACK_LIBRARY_PATH',
                'LD_LIBRARY_PATH', 'GGML_BACKEND_PATH', 'CREXX_PROVIDER_PATH',
                'CREXX_LLAMA_GLUE_PROBES'):
        clean.pop(key, None)
    execution = dict(clean)
    if os.name == 'nt':
        # Retain Windows itself, but remove CUDA, MSYS2 and Visual Studio paths
        # during consumer execution. Only native compilation needs those tools.
        system = Path(os.environ['SystemRoot'])
        execution['PATH'] = os.pathsep.join(map(str, (system / 'System32', system)))

    def run(label, argv, marker=None, expected=0, env=None, cwd=work, consumer=False):
        argv = list(map(str, argv))
        path = logs / (label + '.log')
        with path.open('w', encoding='utf-8') as log:
            log.write('argv=' + repr(argv) + '\n'); log.flush()
            result = subprocess.run(argv, cwd=cwd, env=env or (execution if consumer else clean), stdout=log,
                                    stderr=log, timeout=1800)
        text = path.read_text(encoding='utf-8', errors='replace')
        records.append({'label': label, 'argv': argv, 'returncode': result.returncode,
                        'consumer_environment': consumer})
        assert result.returncode == expected, (label, text[-3000:])
        assert not any(x in text for x in ('ERROR: AddressSanitizer', 'ERROR: LeakSanitizer', 'PANIC:')), (label, text[-3000:])
        if expected == 0:
            assert 'FAIL:' not in text, (label, text[-3000:])
        if marker:
            assert marker in text, (label, text[-3000:])
        print('PASS: ' + label, flush=True)

    outcome = 'failed'
    try:
        fixture_info = json.loads((source / 'tests/native-inference/fixtures/manifest.json').read_text())
        fixture = source / 'tests/native-inference/fixtures' / fixture_info['filename']
        assert fixture.stat().st_size == fixture_info['bytes'] < 5 * 1024 * 1024
        assert sha(fixture) == fixture_info['sha256']
        copied_fixture = work / 'fixture.gguf'
        shutil.copy2(fixture, copied_fixture)
        fixture = copied_fixture
        if a.payload:
            prefix = a.payload.resolve()
        else:
            prefix = work / 'payload'
            shutil.copytree(build / 'bin', prefix / 'bin', symlinks=True)
            run('stage-guides', ['cmake', '--install', build, '--prefix', prefix, '--component', 'llama-docs'])
        providers = prefix / 'bin/providers'
        manifest = json.loads((providers / 'rxllama.native.json').read_text())
        assert manifest['engine'] == fixture_info['upstream']
        assert manifest['provider'] == 'rxllama'
        for entry in manifest['runtime_files'] + manifest['link_libraries']:
            relative = Path(entry['path'])
            assert not relative.is_absolute() and '..' not in relative.parts
            assert sha(providers / relative) == entry['sha256'], entry['path']
        runtime = json.loads((providers / 'rxllama.runtime.json').read_text())
        names = [x['backend'] for x in runtime['backends']]
        for expected in a.backends.split(','):
            assert any(x == expected or x.startswith(expected + '-') for x in names), (expected, names)
        assert (prefix / ('bin/rxllama.rxplugin')).is_file()
        assert (prefix / ('bin/crexx-provider-package' + suffix)).is_file()
        assert (prefix / ('bin/rxbvm' + suffix)).is_file()
        assert (prefix / ('bin/rxvm' + suffix)).is_file()
        if os.name == 'nt':
            bootstrap = (build / 'lib/plugins/llama/tests/release-smoke-bootstrap-files.txt').read_text().splitlines()
            assert bootstrap, 'missing executable-adjacent bootstrap inventory'
            for name in bootstrap:
                assert Path(name).name == name and (prefix / 'bin' / name).is_file(), name
        entry = prefix / ('bin/rxvm' + suffix)
        selected = prefix / 'bin' / (preferred_vm + suffix)
        if os.name == 'nt':
            assert sha(entry) == sha(selected), 'rxvm differs from its selected VM'
        else:
            assert entry.is_symlink(), 'rxvm must retain its installed symlink'
            assert os.readlink(entry) == preferred_vm, 'rxvm must use a relative selected-VM link'
        assert not list(prefix.rglob('*.gguf')), 'fixture/model leaked into release'
        for name in ('README.md', 'installation.md', 'models.md', 'reference.md', 'qualification.md',
                     'examples/persistent_embeddings.crexx', 'examples/shared_embeddings.crexx',
                     'examples/persistent_generation.crexx', 'examples/shared_generation.crexx'):
            assert (prefix / 'share/crexx/llama' / name).is_file(), name
        (logs / 'payload-manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')

        # Origin-relative helper loads only the copied release runtime. Neither
        # this executable nor the fixture is added to the distributed payload.
        engine = work / 'engine'
        engine.mkdir()
        for entry in manifest['runtime_files']:
            target = engine / entry['path']; target.parent.mkdir(parents=True, exist_ok=True)
            copy_runtime(providers / entry['path'], target)
        copied_helper = engine / helper.name
        shutil.copy2(helper, copied_helper)
        run('engine', [copied_helper, fixture], 'PASS: release engine smoke', cwd=engine, consumer=True)
        broken = json.loads((engine / 'rxllama.runtime.json').read_text())
        for entry in broken['backends']:
            if entry['backend'].startswith('cpu'):
                entry['sha256'] = '0' * 64
        (engine / 'rxllama.runtime.json').write_text(json.dumps(broken))
        run('altered-backend-rejected', [copied_helper, fixture],
            'no usable verified CPU backend variant', expected=1, cwd=engine, consumer=True)
        (engine / 'rxllama.runtime.json').unlink()
        run('missing-manifest-rejected', [copied_helper, fixture], expected=1, cwd=engine, consumer=True)
        shutil.rmtree(engine)

        src = work / 'smoke.crexx'
        shutil.copy2(source / 'tests/native-inference/release_provider_smoke.crexx', src)
        marker = 'PASS: public llama release provider'
        for mode in ('opt', 'noopt'):
            program = work / mode
            run(mode + '-compile', [prefix / ('bin/rxc' + suffix), '--no-exe-import',
                *(['-n'] if mode == 'noopt' else []), '-i', prefix / 'bin', '-o', program, src], consumer=True)
            run(mode + '-assemble', [prefix / ('bin/rxas' + suffix), '-o', program, program], consumer=True)
            linked = work / (mode + '-linked')
            run(mode + '-link', [prefix / ('bin/rxlink' + suffix), '-o', linked, program,
                prefix / 'bin/library', prefix / 'bin/classlib', prefix / 'bin/rxfnsg'], consumer=True)
            # Exercise the user's public entry point and the alternate engine
            # where available, without running the preferred engine twice.
            for vm in ('rxvm', *[v for v in ('rxbvm', 'rxtvm') if v != preferred_vm]):
                binary = prefix / 'bin' / (vm + suffix)
                if binary.exists():
                    run(mode + '-' + vm, [binary, linked, '-a', fixture, fixture_info['sha256']], marker, consumer=True)
        native = work / 'native' / 'smoke'
        native.parent.mkdir()
        run('native-build', [prefix / ('bin/crexx' + suffix), '--program', native, src,
            '--jobs', '1', '--native'], 'PUBLISHED: native program',
            env=dict(clean, CREXX_HOME=str(prefix)))
        relocated = work / 'relocated'
        relocated.mkdir()
        executable = native.with_suffix(suffix) if suffix else native
        shutil.copy2(executable, relocated / executable.name)
        manifests = list(native.parent.glob('*.native.json'))
        assert any(m.name == 'rxllama.native.json' for m in manifests)
        for m in manifests:
            shutil.copy2(m, relocated / m.name)
            for entry in json.loads(m.read_text())['runtime_files']:
                target = relocated / entry['path']; target.parent.mkdir(parents=True, exist_ok=True)
                copy_runtime(native.parent / entry['path'], target)
        shutil.rmtree(native.parent)
        run('relocated-native', [relocated / executable.name, fixture, fixture_info['sha256']], marker, cwd=relocated, consumer=True)
        outcome = 'passed'
    finally:
        (logs / 'summary.json').write_text(json.dumps({'outcome': outcome,
            'elapsed_seconds': round(time.monotonic() - started, 3), 'commands': records,
            'work': str(work), 'build': str(build), 'required_backends': a.backends,
            'preferred_vm': preferred_vm,
            'model_downloads': 0, 'scope': 'package and fixture smoke; not full BGE/Smol or device qualification'}, indent=2) + '\n')
        if outcome == 'passed':
            shutil.rmtree(work)


if __name__ == '__main__':
    main()
