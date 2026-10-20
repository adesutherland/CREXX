"""Disposable NetRexx compiler/runtime orchestration shared by package checks."""
from pathlib import Path
import os
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]


class Toolchain:
    def __init__(self, sdk, java_home=None):
        self.sdk = sdk.expanduser().resolve(strict=True)
        self.compiler = self.sdk / 'lib/NetRexxF.jar'
        self.runtime = self.sdk / 'runlib/NetRexxR.jar'
        self.java = ((java_home.expanduser().resolve(strict=True) / ('bin/java.exe' if os.name == 'nt' else 'bin/java'))
                     if java_home else Path(shutil.which('java') or 'java').resolve(strict=True))
        assert self.compiler.is_file() and self.runtime.is_file(), 'Expected a complete NetRexx SDK'

    def command(self, args, directory, timeout=600):
        log = directory / 'command.log'
        with log.open('wb') as output:
            completed = subprocess.run(list(map(str, args)), cwd=directory, stdout=output,
                                       stderr=subprocess.STDOUT, timeout=timeout)
        text = log.read_text(encoding='utf-8', errors='replace')
        if completed.returncode or 'Warning' in text or 'FAIL' in text:
            raise RuntimeError(f'Command failed ({completed.returncode}): {args[0]}\n' + text[:6500])
        return text

    def compile(self, directory, sources):
        self.command([self.java, '-cp', self.compiler, 'org.netrexx.process.NetRexxC',
                      *sources, '-ecj', '-utf8', '-sourcedir', '-nologo', '-verbose0'], directory)

    def run(self, directory, main, *arguments):
        # Never put the compiler JAR or checkout on the consumer's classpath.
        return self.command([self.java, '-cp', str(directory) + os.pathsep + str(self.runtime),
                             main, *arguments], directory)

    def copy_library(self, directory):
        shutil.copytree(ROOT / 'netrexx/rexxcsv', directory / 'rexxcsv')
        return [p.relative_to(directory) for p in sorted((directory / 'rexxcsv').glob('*.nrx'))]
