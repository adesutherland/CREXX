"""Owned shared preparation for compiler value, byte-source and clock contracts."""
from contextlib import contextmanager
from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time



def source_bytes(source):
    if isinstance(source, bytes):
        return source
    if isinstance(source, str):
        return source.encode('utf-8')
    raise TypeError('Fixture source must be str or bytes')


def source_filename(filename):
    if (not filename or filename in ('.', '..') or '/' in filename or '\\' in filename):
        raise ValueError('Fixture source filename must be one path component')
    return filename


@dataclass(frozen=True)
class ProviderSource:
    filename: str
    source: object
    flags: tuple = ()

    def signature(self):
        return {'filename': source_filename(self.filename),
                'source': hashlib.sha256(source_bytes(self.source)).hexdigest(),
                'flags': list(self.flags)}


def checksum(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


@contextmanager
def locked(path):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('a+b') as stream:
        if os.name == 'nt':
            import msvcrt
            stream.seek(0)
            stream.write(b'\0')
            stream.flush()
            stream.seek(0)
            deadline = time.monotonic() + 1800
            while True:
                try:
                    msvcrt.locking(stream.fileno(), msvcrt.LK_NBLCK, 1)
                    break
                except OSError:
                    if time.monotonic() >= deadline:
                        raise TimeoutError('Classic fixture preparation lock did not become available')
                    time.sleep(0.1)
            try:
                yield
            finally:
                stream.seek(0)
                msvcrt.locking(stream.fileno(), msvcrt.LK_UNLCK, 1)
        else:
            import fcntl
            fcntl.flock(stream, fcntl.LOCK_EX)
            try:
                yield
            finally:
                fcntl.flock(stream, fcntl.LOCK_UN)


def run(argv, cwd, expected=0, input_file=None, stderr_empty=False):
    with tempfile.TemporaryFile() as output, tempfile.TemporaryFile() as error:
        source = open(input_file, 'rb') if input_file else None
        try:
            result = subprocess.run(list(map(str, argv)), cwd=cwd, stdin=source,
                                    stdout=output, stderr=error if stderr_empty else subprocess.STDOUT,
                                    timeout=900)
        finally:
            if source:
                source.close()
        output.seek(0)
        value = output.read()
        error.seek(0)
        diagnostic = error.read()
    matched_exit = result.returncode != 0 if expected is None else result.returncode == expected
    if not matched_exit or (stderr_empty and diagnostic):
        raise AssertionError(f'exit {result.returncode}, expected {expected}: {list(map(str, argv))}\n'
                             + (value + diagnostic)[-12000:].decode(errors='replace'))
    return value


class ContractFixture:
    def __init__(self, bindir, root, sources, prepare=False, fresh=False):
        self.bindir = Path(bindir).resolve()
        self.root = Path(root).resolve()
        if prepare and fresh:
            raise ValueError('Fresh execution and cached preparation are separate routes')
        self.prepare = prepare
        self.fresh = fresh
        suffix = '.exe' if os.name == 'nt' else ''
        self.tools = {name: self.bindir / (name + suffix) for name in ('rxc', 'rxas', 'rxlink', 'rxvm')}
        inputs = [Path(__file__), *map(Path, sources), *self.tools.values()]
        inputs.extend(self.bindir.glob('*.rxbin'))
        inputs.extend(self.bindir.glob('*.rxplugin'))
        for pattern in ('*.dll', '*.dylib', '*.so*'):
            inputs.extend(path for path in self.bindir.glob(pattern) if path.is_file())
        self.identity = hashlib.sha256(json.dumps(
            [(str(path.resolve()), checksum(path)) for path in sorted(set(inputs))],
            separators=(',', ':')).encode()).hexdigest()
        self.cache = self.root / 'prepared'
        self.stage = None
        self.manifest = None
        self.seen = set()

    @staticmethod
    def required_artifacts(manifest):
        required = set()
        for stem, signature in manifest['cases'].items():
            outputs = ['program', *(Path(provider['filename']).stem for provider in signature['providers'])]
            for mode in ('noopt', 'opt'):
                required.update(f'{mode}/{stem}/{output}{suffix}'
                                for output in outputs for suffix in ('.rxas', '.rxbin'))
        return required

    def source_inventory(self):
        return {str(path.relative_to(self.root)) for path in (self.root / 'sources').rglob('*')
                if path.is_file()}

    def reset_sources(self):
        directory = self.root / 'sources'
        if directory.exists():
            shutil.rmtree(directory)

    def valid(self):
        try:
            manifest = json.loads((self.cache / 'manifest.json').read_text())
            if manifest['identity'] != self.identity:
                return False
            if not all(isinstance(manifest.get(name), dict) for name in ('cases', 'artifacts', 'source_files')):
                return False
            if self.source_inventory() != set(manifest['source_files']):
                return False
            required = self.required_artifacts(manifest)
            if not required.issubset(manifest['artifacts']):
                return False
            if any(not (self.cache / name).is_file() or (self.cache / name).stat().st_size == 0 for name in required):
                return False
            if any(checksum(self.cache / name) != value for name, value in manifest['artifacts'].items()):
                return False
            if any(checksum(self.root / name) != value for name, value in manifest['source_files'].items()):
                return False
            self.manifest = manifest
            return True
        except (OSError, ValueError, KeyError, TypeError):
            return False

    def __enter__(self):
        self.lock = locked(self.root / 'prepare.lock')
        self.lock.__enter__()
        try:
            if self.fresh:
                self.reset_sources()
                self.reused = False
                self.stage = Path(tempfile.mkdtemp(prefix='.fresh-', dir=self.root))
                self.manifest = {'identity': self.identity, 'cases': {}, 'invalid': {}, 'source_files': {}, 'artifacts': {}}
            elif self.valid():
                self.reused = True
            elif not self.prepare:
                raise AssertionError('Missing/stale Classic preparation; run its CTest setup fixture')
            else:
                self.reused = False
                shutil.rmtree(self.cache, ignore_errors=True)
                self.reset_sources()
                self.stage = Path(tempfile.mkdtemp(prefix='.prepare-', dir=self.root))
                self.manifest = {'identity': self.identity, 'cases': {}, 'invalid': {}, 'source_files': {}}
            return self
        except BaseException:
            try:
                if self.stage and self.stage.exists():
                    shutil.rmtree(self.stage)
            finally:
                self.lock.__exit__(None, None, None)
            raise

    def source(self, stem, source, classic, filename=None):
        root = self.root / 'sources' / stem
        root.mkdir(parents=True, exist_ok=True)
        path = root / source_filename(filename or ('program.rexx' if classic else 'program.crexx'))
        content = source_bytes(source)
        if not path.exists() or path.read_bytes() != content:
            path.write_bytes(content)
        self.manifest['source_files'][str(path.relative_to(self.root))] = checksum(path)
        return path

    def case(self, stem, source, expected=None, classic=True, expected_exit=0,
             contains=(), noopt=False, linked=False, stderr_empty=False, context=None,
             filename=None, providers=()):
        filename = source_filename(filename or ('program.rexx' if classic else 'program.crexx'))
        providers = tuple(providers)
        provider_signatures = [provider.signature() for provider in providers]
        provider_stems = [Path(provider.filename).stem for provider in providers]
        owned_stems = [os.path.normcase(name) for name in provider_stems]
        if (len(set(owned_stems)) != len(owned_stems) or 'program' in owned_stems
                or any(os.path.normcase(provider.filename) == os.path.normcase(filename) for provider in providers)):
            raise ValueError('Auxiliary provider outputs must have distinct source/output owners')
        signature = {'source': hashlib.sha256(source_bytes(source)).hexdigest(),
                     'classic': classic, 'expected_exit': expected_exit,
                     'filename': filename, 'providers': provider_signatures}
        self.seen.add(stem)
        if self.fresh or (self.prepare and not self.reused):
            self.manifest['cases'][stem] = signature
            path = self.source(stem, source, classic, filename)
            provider_paths = [self.source(stem, provider.source, classic, provider.filename)
                              for provider in providers]
            modes = ('noopt' if noopt else 'opt',) if self.fresh else ('noopt', 'opt')
            for mode in modes:
                directory = self.stage / mode / stem
                directory.mkdir(parents=True)
                output = directory / 'program'
                mode_flags = ['-n'] if mode == 'noopt' else []
                run([self.tools['rxc'], *mode_flags,
                     '-i', self.bindir, '-o', output, path], directory, stderr_empty=True)
                run([self.tools['rxas'], '-o', str(output) + '.rxbin', output], directory, stderr_empty=True)
                for provider, provider_path, provider_stem in zip(providers, provider_paths, provider_stems):
                    provider_output = directory / provider_stem
                    run([self.tools['rxc'], *mode_flags, *provider.flags, '-i', self.bindir,
                         '-o', provider_output, provider_path], directory, stderr_empty=True)
                    run([self.tools['rxas'], '-o', str(provider_output) + '.rxbin', provider_output],
                        directory, stderr_empty=True)
            if not self.fresh:
                return
        if self.manifest['cases'].get(stem) != signature:
            raise AssertionError(f'Classic fixture source/mode mismatch: {stem}')
        if self.prepare:
            return
        artifact_root = self.stage if self.fresh else self.cache
        directory = artifact_root / ('noopt' if noopt else 'opt') / stem
        image = directory / ('program_image.rxbin' if linked else 'program.rxbin')
        image_name = str(image.relative_to(artifact_root))
        runtime_libraries = [self.bindir / (name + '.rxbin') for name in
                             (('library', 'classlib', 'rxfnsc') if classic else ('library', 'classlib'))]
        runtime_libraries.extend(directory / (name + '.rxbin') for name in provider_stems)
        linked_owned = (image.is_file() and self.manifest['artifacts'].get(image_name) == checksum(image))
        if linked and not linked_owned:
            image.unlink(missing_ok=True)
            # Only linked consumers need this image. Hold the preparation lock
            # and publish its bytes and manifest before executing a consumer.
            pending = directory / '.program_image.rxbin'
            pending.unlink(missing_ok=True)
            try:
                run([self.tools['rxlink'], '-o', pending, directory / 'program.rxbin', *runtime_libraries],
                    directory, stderr_empty=True)
                os.replace(pending, image)
                self.manifest['artifacts'][str(image.relative_to(artifact_root))] = checksum(image)
                if not self.fresh:
                    self.publish_manifest()
            except BaseException:
                pending.unlink(missing_ok=True)
                image.unlink(missing_ok=True)
                raise
        libraries = [] if linked else runtime_libraries
        raw = run([self.tools['rxvm'], image, *libraries], directory,
                  expected=expected_exit, stderr_empty=stderr_empty)
        if isinstance(expected, bytes):
            value = raw
        else:
            value = raw.decode('utf-8').replace('\r\n', '\n')
        if expected is not None and value != expected:
            raise AssertionError(f'Unexpected Classic output: {stem}: {context}\n{value}')
        if any(fragment not in value for fragment in contains):
            raise AssertionError(f'Unexpected Classic diagnostic: {stem}\n{value}')

    def invalid_source(self, stem, source, diagnostic_any, filename, expected_exit=2):
        signature = {'source': hashlib.sha256(source_bytes(source)).hexdigest(),
                     'filename': source_filename(filename), 'diagnostic_any': list(diagnostic_any),
                     'exit': expected_exit}
        if self.fresh or (self.prepare and not self.reused):
            path = self.source(stem, source, False, filename)
            output = run([self.tools['rxc'], '--diagnostics', 'raw', '-i', self.bindir,
                          '-o', self.stage / (stem + '_invalid'), path], self.stage,
                         expected=expected_exit).decode('utf-8', errors='replace')
            if not any(fragment in output for fragment in diagnostic_any):
                raise AssertionError(f'Missing source diagnostic for {filename}: {output}')
            self.manifest['invalid'][stem] = signature
        elif self.manifest['invalid'].get(stem) != signature:
            raise AssertionError(f'Source diagnostic preparation mismatch: {stem}')

    def invalid(self, stem, body, code):
        source = 'options levelc\n' + body + '\n'
        signature = {'source': hashlib.sha256(source.encode()).hexdigest(), 'code': code, 'exit': 2}
        if self.prepare and not self.reused:
            path = self.source(stem, source, True)
            output = run([self.tools['rxc'], '--diagnostics', 'raw', '-i', self.bindir,
                          '-o', self.stage / (stem + '_invalid'), path], self.stage, expected=2).decode()
            if 'RXC-LC-' + code not in output:
                raise AssertionError((body, output))
            self.manifest['invalid'][stem] = signature
        elif self.manifest['invalid'].get(stem) != signature:
            raise AssertionError(f'Classic source diagnostic preparation mismatch: {stem}')

    def publish_manifest(self):
        pending = self.cache / '.manifest.json'
        pending.write_text(json.dumps(self.manifest, sort_keys=True))
        os.replace(pending, self.cache / 'manifest.json')

    def __exit__(self, kind, value, traceback):
        try:
            if kind is None:
                if set(self.manifest['cases']) != self.seen:
                    raise AssertionError('Classic prepared case inventory differs from its contract')
                if self.stage and not self.fresh:
                    if self.source_inventory() != set(self.manifest['source_files']):
                        raise AssertionError('Undeclared source in contract fixture preparation')
                    required = self.required_artifacts(self.manifest)
                    if any(not (self.stage / name).is_file() or (self.stage / name).stat().st_size == 0 for name in required):
                        raise AssertionError('Incomplete contract fixture producer output')
                    self.manifest['artifacts'] = {str(path.relative_to(self.stage)): checksum(path)
                                                   for path in self.stage.rglob('*') if path.is_file()}
                    (self.stage / 'manifest.json').write_text(json.dumps(self.manifest, sort_keys=True))
                    os.replace(self.stage, self.cache)
        finally:
            if self.stage and self.stage.exists():
                shutil.rmtree(self.stage)
            self.lock.__exit__(kind, value, traceback)
