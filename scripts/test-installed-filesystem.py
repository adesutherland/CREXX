"""Run the standard rxfs contract through an extracted core's four tools."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--prefix', type=Path, required=True)
    parser.add_argument('--logs', type=Path, required=True)
    args = parser.parse_args()
    args.logs.mkdir(parents=True, exist_ok=True)
    binary = args.prefix.resolve() / 'bin'
    env = dict(os.environ)
    for name in ('CREXX_HOME', 'CREXX_PROVIDER_PATH', 'LD_LIBRARY_PATH', 'DYLD_LIBRARY_PATH'):
        env.pop(name, None)
    with tempfile.TemporaryDirectory(prefix='crexx-installed-rxfs-') as tmp:
        work = Path(tmp)
        source = work / 'contract.crexx'
        shutil.copy2(Path(__file__).resolve().parents[1] / 'lib/plugins/fs/rxfs_test.crexx', source)
        for mode in ('opt', 'noopt'):
            program, linked = work / mode, work / (mode + '-linked')
            commands = [('compile', [binary / 'rxc', '--no-exe-import', *(['-n'] if mode == 'noopt' else []),
                                     '-i', binary, '-o', program, source]),
                        ('assemble', [binary / 'rxas', '-o', program, program]),
                        ('link', [binary / 'rxlink', '-o', linked, program, binary / 'library'])]
            commands += [(vm, [binary / vm, linked, '-a', mode + '-' + vm])
                         for vm in ('rxvm', 'rxbvm') if (binary / vm).exists()]
            for name, command in commands:
                path = args.logs / (mode + '-' + name + '.log')
                with path.open('w') as log:
                    result = subprocess.run(list(map(str, command)), cwd=work, env=env,
                                            stdout=log, stderr=log, timeout=1800)
                output = path.read_text(errors='replace')
                if result.returncode or 'FAIL:' in output or 'PANIC:' in output:
                    raise RuntimeError(name + ': ' + output[-2000:])
                if name in ('rxvm', 'rxbvm') and 'PASS: rxfs public contract' not in output:
                    raise RuntimeError('Missing rxfs success marker')
    print('PASS: installed rxfs, both optimization modes and VM entries')


if __name__ == '__main__':
    main()
